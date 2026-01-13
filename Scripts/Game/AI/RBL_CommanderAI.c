// ============================================================================
// PROJECT REBELLION - Commander AI
// Strategic layer AI that manages enemy faction responses
// Now supports both entity-based and virtual zones
// ============================================================================

class RBL_CommanderAI
{
	protected static ref RBL_CommanderAI s_Instance;
	
	protected const float DECISION_INTERVAL = 30.0;
	protected const float QRF_COOLDOWN = 180.0;
	protected const int MAX_CONCURRENT_QRFS = 3;
	
	protected ERBLFactionKey m_eControlledFaction;
	protected int m_iFactionResources;
	protected float m_fTimeSinceLastDecision;
	protected float m_fTimeSinceLastQRF;
	
	protected ref array<ref RBL_QRFOperation> m_aActiveQRFs;
	
	static RBL_CommanderAI GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_CommanderAI();
		return s_Instance;
	}
	
	void RBL_CommanderAI()
	{
		m_aActiveQRFs = new array<ref RBL_QRFOperation>();
		m_eControlledFaction = ERBLFactionKey.USSR;
		m_iFactionResources = RBL_Config.AI_STARTING_RESOURCES;
		m_fTimeSinceLastDecision = 0;
		m_fTimeSinceLastQRF = RBL_Config.QRF_COOLDOWN_SECONDS;
	}
	
	void Update(float timeSlice)
	{
		m_fTimeSinceLastDecision += timeSlice;
		m_fTimeSinceLastQRF += timeSlice;
		
		UpdateActiveQRFs(timeSlice);
		
		if (m_fTimeSinceLastDecision >= DECISION_INTERVAL)
		{
			m_fTimeSinceLastDecision = 0;
			MakeStrategicDecision();
		}
	}
	
	protected void MakeStrategicDecision()
	{
		// Check virtual zones for threats (primary zone type used in game)
		array<ref RBL_VirtualZone> threatenedVirtualZones = GetThreatenedVirtualZones();
		for (int i = 0; i < threatenedVirtualZones.Count(); i++)
		{
			ConsiderQRFResponseVirtual(threatenedVirtualZones[i]);
		}
		
		// Also check entity zones for threats (for maps using placed entities)
		array<RBL_CampaignZone> threatenedEntityZones = GetThreatenedEntityZones();
		for (int i = 0; i < threatenedEntityZones.Count(); i++)
		{
			ConsiderQRFResponseEntity(threatenedEntityZones[i]);
		}
		
		// Consider offensive operations
		if (m_iFactionResources > 500)
		{
			RBL_VirtualZone targetZone = FindRecaptureTargetVirtual();
			if (targetZone)
				ConsiderOffensiveOperationVirtual(targetZone);
		}
		
		RegenerateResources();
	}
	
	// ============================================================================
	// VIRTUAL ZONE SUPPORT (Config-based zones)
	// ============================================================================
	
	bool ConsiderQRFResponseVirtual(RBL_VirtualZone targetZone)
	{
		if (!targetZone)
			return false;
		
		if (m_fTimeSinceLastQRF < RBL_Config.QRF_COOLDOWN_SECONDS)
			return false;
		
		if (m_aActiveQRFs.Count() >= RBL_Config.QRF_MAX_CONCURRENT)
			return false;
		
		if (targetZone.GetOwnerFaction() != m_eControlledFaction)
			return false;
		
		int threatLevel = CalculateThreatLevelVirtual(targetZone);
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		int aggression = 50;
		int warLevel = 1;
		if (campaignMgr)
		{
			aggression = campaignMgr.GetAggression();
			warLevel = campaignMgr.GetWarLevel();
		}
		
		int responseThreshold = 100 - aggression;
		if (threatLevel < responseThreshold)
			return false;
		
		RBL_VirtualZone sourceBase = FindNearestFriendlyBaseVirtual(targetZone);
		if (!sourceBase)
			return false;
		
		ERBLQRFType qrfType = DetermineQRFTypeVirtual(targetZone, sourceBase, threatLevel, warLevel);
		int cost = GetQRFCost(qrfType);
		
		if (m_iFactionResources < cost)
		{
			qrfType = DowngradeQRFType(qrfType);
			cost = GetQRFCost(qrfType);
			
			if (m_iFactionResources < cost)
				return false;
		}
		
		return LaunchQRFVirtual(qrfType, sourceBase, targetZone, cost);
	}
	
	protected ERBLQRFType DetermineQRFTypeVirtual(RBL_VirtualZone target, RBL_VirtualZone source, int threatLevel, int warLevel)
	{
		float distance = target.GetDistanceTo(source);
		int zoneValue = target.GetStrategicValue();
		
		bool isCritical = zoneValue >= 500;
		bool isFar = distance >= RBL_Config.DISTANCE_MEDIUM;
		bool isMedium = distance < RBL_Config.DISTANCE_MEDIUM;
		
		if (isCritical && warLevel >= 5)
			return ERBLQRFType.SPECOPS;
		
		if (isCritical && warLevel >= 3)
			return ERBLQRFType.MECHANIZED;
		
		if (isFar && warLevel >= 6)
			return ERBLQRFType.HELICOPTER;
		
		if (threatLevel > 70 && isMedium)
			return ERBLQRFType.CONVOY;
		
		if (warLevel >= 4 && threatLevel > 50)
			return ERBLQRFType.MECHANIZED;
		
		if (threatLevel > 30)
			return ERBLQRFType.INFANTRY;
		
		return ERBLQRFType.PATROL;
	}
	
	protected int CalculateThreatLevelVirtual(RBL_VirtualZone zone)
	{
		int threat = 0;
		
		if (zone.IsUnderAttack())
			threat += 50;
		
		threat += zone.GetStrategicValue() / 20;
		
		int maxGarrison = zone.GetMaxGarrison();
		if (maxGarrison > 0)
		{
			float garrisonRatio = zone.GetCurrentGarrison() / (float)maxGarrison;
			threat += Math.Round((1.0 - garrisonRatio) * 30);
		}
		
		// Check if zone is being captured
		RBL_CaptureManager capMgr = RBL_CaptureManager.GetInstance();
		if (capMgr && capMgr.IsZoneBeingCaptured(zone.GetZoneID()))
			threat += 40;
		
		return Math.Clamp(threat, 0, 100);
	}
	
	protected array<ref RBL_VirtualZone> GetThreatenedVirtualZones()
	{
		array<ref RBL_VirtualZone> result = new array<ref RBL_VirtualZone>();
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return result;
		
		array<ref RBL_VirtualZone> allZones = zoneMgr.GetAllVirtualZones();
		RBL_CaptureManager capMgr = RBL_CaptureManager.GetInstance();
		
		for (int i = 0; i < allZones.Count(); i++)
		{
			RBL_VirtualZone zone = allZones[i];
			if (zone.GetOwnerFaction() != m_eControlledFaction)
				continue;
			
			// Check if under attack or being captured
			bool isThreatened = zone.IsUnderAttack();
			if (capMgr && capMgr.IsZoneBeingCaptured(zone.GetZoneID()))
				isThreatened = true;
			
			if (isThreatened)
				result.Insert(zone);
		}
		
		return result;
	}
	
	protected RBL_VirtualZone FindNearestFriendlyBaseVirtual(RBL_VirtualZone targetZone)
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return null;
		
		RBL_VirtualZone nearest = null;
		float nearestDist = 999999.0;
		
		array<ref RBL_VirtualZone> allZones = zoneMgr.GetAllVirtualZones();
		for (int i = 0; i < allZones.Count(); i++)
		{
			RBL_VirtualZone zone = allZones[i];
			
			if (zone.GetOwnerFaction() != m_eControlledFaction)
				continue;
			
			if (zone.GetZoneID() == targetZone.GetZoneID())
				continue;
			
			ERBLZoneType type = zone.GetZoneType();
			bool isBase = (type == ERBLZoneType.Airbase || type == ERBLZoneType.Outpost || type == ERBLZoneType.HQ);
			
			if (!isBase)
				continue;
			
			float dist = targetZone.GetDistanceTo(zone);
			if (dist < nearestDist)
			{
				nearestDist = dist;
				nearest = zone;
			}
		}
		
		return nearest;
	}
	
	protected RBL_VirtualZone FindRecaptureTargetVirtual()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return null;
		
		RBL_VirtualZone bestTarget = null;
		int highestPriority = 0;
		
		array<ref RBL_VirtualZone> allZones = zoneMgr.GetAllVirtualZones();
		for (int i = 0; i < allZones.Count(); i++)
		{
			RBL_VirtualZone zone = allZones[i];
			
			if (zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				continue;
			
			int priority = zone.GetStrategicValue() - zone.GetCurrentGarrison() * 10;
			
			if (priority > highestPriority)
			{
				highestPriority = priority;
				bestTarget = zone;
			}
		}
		
		return bestTarget;
	}
	
	protected bool LaunchQRFVirtual(ERBLQRFType type, RBL_VirtualZone source, RBL_VirtualZone target, int cost)
	{
		m_iFactionResources -= cost;
		m_fTimeSinceLastQRF = 0;
		
		RBL_QRFOperation qrf = new RBL_QRFOperation();
		qrf.InitializeFromVirtual(type, source, target, m_eControlledFaction);
		
		m_aActiveQRFs.Insert(qrf);
		
		PrintFormat("[RBL_AI] QRF Launched! Type: %1, From: %2, To: %3, Cost: %4",
			typename.EnumToString(ERBLQRFType, type),
			source.GetZoneID(),
			target.GetZoneID(),
			cost
		);
		
		return true;
	}
	
	protected void ConsiderOffensiveOperationVirtual(RBL_VirtualZone target)
	{
		if (m_aActiveQRFs.Count() >= RBL_Config.QRF_MAX_CONCURRENT)
			return;
		
		RBL_VirtualZone source = FindNearestFriendlyBaseVirtual(target);
		if (!source)
			return;
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		int warLevel = 1;
		if (campaignMgr)
			warLevel = campaignMgr.GetWarLevel();
		
		ERBLQRFType attackType = ERBLQRFType.CONVOY;
		if (warLevel >= 6)
			attackType = ERBLQRFType.MECHANIZED;
		
		int cost = GetQRFCost(attackType) * 2;
		
		if (m_iFactionResources >= cost)
			LaunchQRFVirtual(attackType, source, target, cost);
	}
	
	// ============================================================================
	// ENTITY ZONE SUPPORT (Placed in editor)
	// ============================================================================
	
	bool ConsiderQRFResponseEntity(RBL_CampaignZone targetZone)
	{
		if (!targetZone)
			return false;
		
		if (m_fTimeSinceLastQRF < RBL_Config.QRF_COOLDOWN_SECONDS)
			return false;
		
		if (m_aActiveQRFs.Count() >= RBL_Config.QRF_MAX_CONCURRENT)
			return false;
		
		if (targetZone.GetOwnerFaction() != m_eControlledFaction)
			return false;
		
		int threatLevel = CalculateThreatLevelEntity(targetZone);
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		int aggression = 50;
		int warLevel = 1;
		if (campaignMgr)
		{
			aggression = campaignMgr.GetAggression();
			warLevel = campaignMgr.GetWarLevel();
		}
		
		int responseThreshold = 100 - aggression;
		if (threatLevel < responseThreshold)
			return false;
		
		RBL_CampaignZone sourceBase = FindNearestFriendlyBaseEntity(targetZone);
		if (!sourceBase)
			return false;
		
		ERBLQRFType qrfType = DetermineQRFTypeEntity(targetZone, sourceBase, threatLevel, warLevel);
		int cost = GetQRFCost(qrfType);
		
		if (m_iFactionResources < cost)
		{
			qrfType = DowngradeQRFType(qrfType);
			cost = GetQRFCost(qrfType);
			
			if (m_iFactionResources < cost)
				return false;
		}
		
		return LaunchQRFEntity(qrfType, sourceBase, targetZone, cost);
	}
	
	protected ERBLQRFType DetermineQRFTypeEntity(RBL_CampaignZone target, RBL_CampaignZone source, int threatLevel, int warLevel)
	{
		float distance = target.GetDistanceTo(source);
		int zoneValue = target.GetStrategicValue();
		
		bool isCritical = zoneValue >= 500;
		bool isFar = distance >= RBL_Config.DISTANCE_MEDIUM;
		bool isMedium = distance < RBL_Config.DISTANCE_MEDIUM;
		
		if (isCritical && warLevel >= 5)
			return ERBLQRFType.SPECOPS;
		
		if (isCritical && warLevel >= 3)
			return ERBLQRFType.MECHANIZED;
		
		if (isFar && warLevel >= 6)
			return ERBLQRFType.HELICOPTER;
		
		if (threatLevel > 70 && isMedium)
			return ERBLQRFType.CONVOY;
		
		if (warLevel >= 4 && threatLevel > 50)
			return ERBLQRFType.MECHANIZED;
		
		if (threatLevel > 30)
			return ERBLQRFType.INFANTRY;
		
		return ERBLQRFType.PATROL;
	}
	
	protected int CalculateThreatLevelEntity(RBL_CampaignZone zone)
	{
		int threat = 0;
		
		if (zone.IsUnderAttack())
			threat += 50;
		
		threat += zone.GetStrategicValue() / 20;
		
		int maxGarrison = zone.GetMaxGarrison();
		if (maxGarrison > 0)
		{
			float garrisonRatio = zone.GetCurrentGarrison() / (float)maxGarrison;
			threat += Math.Round((1.0 - garrisonRatio) * 30);
		}
		
		return Math.Clamp(threat, 0, 100);
	}
	
	protected array<RBL_CampaignZone> GetThreatenedEntityZones()
	{
		array<RBL_CampaignZone> result = new array<RBL_CampaignZone>();
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return result;
		
		array<RBL_CampaignZone> allZones = zoneMgr.GetAllZones();
		for (int i = 0; i < allZones.Count(); i++)
		{
			RBL_CampaignZone zone = allZones[i];
			if (zone.GetOwnerFaction() == m_eControlledFaction && zone.IsUnderAttack())
				result.Insert(zone);
		}
		
		return result;
	}
	
	protected RBL_CampaignZone FindNearestFriendlyBaseEntity(RBL_CampaignZone targetZone)
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return null;
		
		RBL_CampaignZone nearest = null;
		float nearestDist = 999999.0;
		
		array<RBL_CampaignZone> allZones = zoneMgr.GetAllZones();
		for (int i = 0; i < allZones.Count(); i++)
		{
			RBL_CampaignZone zone = allZones[i];
			
			if (zone.GetOwnerFaction() != m_eControlledFaction)
				continue;
			
			if (zone == targetZone)
				continue;
			
			ERBLZoneType type = zone.GetZoneType();
			bool isBase = (type == ERBLZoneType.Airbase || type == ERBLZoneType.Outpost || type == ERBLZoneType.HQ);
			
			if (!isBase)
				continue;
			
			float dist = targetZone.GetDistanceTo(zone);
			if (dist < nearestDist)
			{
				nearestDist = dist;
				nearest = zone;
			}
		}
		
		return nearest;
	}
	
	protected bool LaunchQRFEntity(ERBLQRFType type, RBL_CampaignZone source, RBL_CampaignZone target, int cost)
	{
		m_iFactionResources -= cost;
		m_fTimeSinceLastQRF = 0;
		
		RBL_QRFOperation qrf = new RBL_QRFOperation();
		qrf.InitializeFromEntity(type, source, target, m_eControlledFaction);
		
		m_aActiveQRFs.Insert(qrf);
		
		PrintFormat("[RBL_AI] QRF Launched! Type: %1, From: %2, To: %3, Cost: %4",
			typename.EnumToString(ERBLQRFType, type),
			source.GetZoneID(),
			target.GetZoneID(),
			cost
		);
		
		return true;
	}
	
	// ============================================================================
	// SHARED METHODS
	// ============================================================================
	
	protected ERBLQRFType DowngradeQRFType(ERBLQRFType current)
	{
		switch (current)
		{
			case ERBLQRFType.SPECOPS: return ERBLQRFType.MECHANIZED;
			case ERBLQRFType.HELICOPTER: return ERBLQRFType.CONVOY;
			case ERBLQRFType.MECHANIZED: return ERBLQRFType.CONVOY;
			case ERBLQRFType.CONVOY: return ERBLQRFType.INFANTRY;
			case ERBLQRFType.INFANTRY: return ERBLQRFType.PATROL;
		}
		return ERBLQRFType.PATROL;
	}
	
	protected int GetQRFCost(ERBLQRFType type)
	{
		switch (type)
		{
			case ERBLQRFType.PATROL: return RBL_Config.QRF_COST_PATROL;
			case ERBLQRFType.CONVOY: return RBL_Config.QRF_COST_CONVOY;
			case ERBLQRFType.INFANTRY: return RBL_Config.QRF_COST_INFANTRY;
			case ERBLQRFType.MECHANIZED: return RBL_Config.QRF_COST_MECHANIZED;
			case ERBLQRFType.HELICOPTER: return RBL_Config.QRF_COST_HELICOPTER;
			case ERBLQRFType.SPECOPS: return RBL_Config.QRF_COST_SPECOPS;
		}
		return RBL_Config.QRF_COST_PATROL;
	}
	
	protected void UpdateActiveQRFs(float timeSlice)
	{
		for (int i = m_aActiveQRFs.Count() - 1; i >= 0; i--)
		{
			RBL_QRFOperation qrf = m_aActiveQRFs[i];
			qrf.Update(timeSlice);
			
			if (qrf.IsComplete())
			{
				m_aActiveQRFs.Remove(i);
			}
		}
	}
	
	protected void RegenerateResources()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		int income = 0;
		
		// Income from entity zones
		array<RBL_CampaignZone> entityZones = zoneMgr.GetAllZones();
		for (int i = 0; i < entityZones.Count(); i++)
		{
			RBL_CampaignZone zone = entityZones[i];
			if (zone.GetOwnerFaction() == m_eControlledFaction)
				income += zone.CalculateResourceIncome() / 10;
		}
		
		// Income from virtual zones
		array<ref RBL_VirtualZone> virtualZones = zoneMgr.GetAllVirtualZones();
		for (int i = 0; i < virtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = virtualZones[i];
			if (zone.GetOwnerFaction() == m_eControlledFaction)
				income += zone.CalculateResourceIncome() / 10;
		}
		
		m_iFactionResources += income;
		m_iFactionResources = Math.Min(m_iFactionResources, RBL_Config.AI_MAX_RESOURCES);
	}
	
	array<ref RBL_ActiveMissionData> GetActiveMissionData()
	{
		array<ref RBL_ActiveMissionData> result = new array<ref RBL_ActiveMissionData>();
		
		for (int i = 0; i < m_aActiveQRFs.Count(); i++)
		{
			RBL_QRFOperation qrf = m_aActiveQRFs[i];
			RBL_ActiveMissionData data = new RBL_ActiveMissionData();
			data.MissionID = qrf.GetOperationID();
			data.MissionType = qrf.GetQRFType();
			data.TargetZoneID = qrf.GetTargetZoneID();
			data.SourceZoneID = qrf.GetSourceZoneID();
			data.CurrentPosition = qrf.GetCurrentPosition();
			data.TimeStarted = qrf.GetTimeStarted();
			result.Insert(data);
		}
		
		return result;
	}
	
	int GetFactionResources() { return m_iFactionResources; }
	int GetActiveQRFCount() { return m_aActiveQRFs.Count(); }
	ERBLFactionKey GetControlledFaction() { return m_eControlledFaction; }
}

// ============================================================================
// QRF Operation - Supports both zone types via ID/position
// ============================================================================
class RBL_QRFOperation
{
	protected string m_sOperationID;
	protected ERBLQRFType m_eType;
	protected ERBLFactionKey m_eFaction;
	protected string m_sTargetZoneID;
	protected string m_sSourceZoneID;
	protected vector m_vTargetPosition;
	protected vector m_vCurrentPosition;
	protected float m_fTimeStarted;
	protected bool m_bComplete;
	protected bool m_bSuccessful;
	
	void RBL_QRFOperation()
	{
		m_sOperationID = "QRF_" + Math.RandomInt(0, 99999).ToString();
		m_bComplete = false;
		m_bSuccessful = false;
	}
	
	void InitializeFromVirtual(ERBLQRFType type, RBL_VirtualZone source, RBL_VirtualZone target, ERBLFactionKey faction)
	{
		m_eType = type;
		m_eFaction = faction;
		m_sTargetZoneID = target.GetZoneID();
		m_sSourceZoneID = source.GetZoneID();
		m_vTargetPosition = target.GetZonePosition();
		m_vCurrentPosition = source.GetZonePosition();
		m_fTimeStarted = 0;
	}
	
	void InitializeFromEntity(ERBLQRFType type, RBL_CampaignZone source, RBL_CampaignZone target, ERBLFactionKey faction)
	{
		m_eType = type;
		m_eFaction = faction;
		m_sTargetZoneID = target.GetZoneID();
		m_sSourceZoneID = source.GetZoneID();
		m_vTargetPosition = target.GetZonePosition();
		m_vCurrentPosition = source.GetZonePosition();
		m_fTimeStarted = 0;
	}
	
	void Update(float timeSlice)
	{
		if (m_bComplete)
			return;
		
		m_fTimeStarted += timeSlice;
		
		// Move towards target
		vector direction = m_vTargetPosition - m_vCurrentPosition;
		float dist = direction.Length();
		
		if (dist > 10)
		{
			direction.Normalize();
			float speed = 50.0; // meters per second
			m_vCurrentPosition = m_vCurrentPosition + direction * speed * timeSlice;
		}
		else
		{
			// Arrived at target
			m_bComplete = true;
			m_bSuccessful = true;
			PrintFormat("[RBL_AI] QRF %1 arrived at target %2", m_sOperationID, m_sTargetZoneID);
		}
		
		// Timeout after 5 minutes
		if (m_fTimeStarted > 300)
		{
			m_bComplete = true;
			m_bSuccessful = false;
			PrintFormat("[RBL_AI] QRF %1 timed out", m_sOperationID);
		}
	}
	
	string GetOperationID() { return m_sOperationID; }
	ERBLQRFType GetQRFType() { return m_eType; }
	string GetTargetZoneID() { return m_sTargetZoneID; }
	string GetSourceZoneID() { return m_sSourceZoneID; }
	vector GetCurrentPosition() { return m_vCurrentPosition; }
	vector GetTargetPosition() { return m_vTargetPosition; }
	float GetTimeStarted() { return m_fTimeStarted; }
	bool IsComplete() { return m_bComplete; }
	bool WasSuccessful() { return m_bSuccessful; }
}
