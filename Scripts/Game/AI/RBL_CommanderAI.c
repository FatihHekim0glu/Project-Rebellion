// ============================================================================
// PROJECT REBELLION - Commander AI
// Strategic layer AI that manages enemy faction responses
// ============================================================================

class RBL_CommanderAI
{
	protected static ref RBL_CommanderAI s_Instance;
	
	protected const float DECISION_INTERVAL = 30.0;
	protected const float QRF_COOLDOWN = 180.0;
	protected const int MAX_CONCURRENT_QRFS = 3;
	
	protected const int COST_QRF_PATROL = 50;
	protected const int COST_QRF_CONVOY = 150;
	protected const int COST_QRF_INFANTRY = 100;
	protected const int COST_QRF_MECHANIZED = 300;
	protected const int COST_QRF_HELICOPTER = 500;
	protected const int COST_QRF_SPECOPS = 400;
	
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
		m_iFactionResources = 1000;
		m_fTimeSinceLastDecision = 0;
		m_fTimeSinceLastQRF = QRF_COOLDOWN;
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
		array<RBL_CampaignZone> threatenedZones = GetThreatenedZones();
		for (int i = 0; i < threatenedZones.Count(); i++)
		{
			ConsiderQRFResponse(threatenedZones[i]);
		}
		
		if (m_iFactionResources > 500)
		{
			RBL_CampaignZone targetZone = FindRecaptureTarget();
			if (targetZone)
				ConsiderOffensiveOperation(targetZone);
		}
		
		RegenerateResources();
	}
	
	bool ConsiderQRFResponse(RBL_CampaignZone targetZone)
	{
		if (!targetZone)
			return false;
		
		if (m_fTimeSinceLastQRF < QRF_COOLDOWN)
			return false;
		
		if (m_aActiveQRFs.Count() >= MAX_CONCURRENT_QRFS)
			return false;
		
		if (targetZone.GetOwnerFaction() != m_eControlledFaction)
			return false;
		
		int threatLevel = CalculateThreatLevel(targetZone);
		
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
		
		RBL_CampaignZone sourceBase = FindNearestFriendlyBase(targetZone);
		if (!sourceBase)
			return false;
		
		ERBLQRFType qrfType = DetermineQRFType(targetZone, sourceBase, threatLevel, warLevel);
		int cost = GetQRFCost(qrfType);
		
		if (m_iFactionResources < cost)
		{
			qrfType = DowngradeQRFType(qrfType);
			cost = GetQRFCost(qrfType);
			
			if (m_iFactionResources < cost)
				return false;
		}
		
		return LaunchQRF(qrfType, sourceBase, targetZone, cost);
	}
	
	protected ERBLQRFType DetermineQRFType(RBL_CampaignZone target, RBL_CampaignZone source, int threatLevel, int warLevel)
	{
		float distance = target.GetDistanceTo(source);
		int zoneValue = target.GetStrategicValue();
		
		bool isCritical = zoneValue >= 500;
		bool isFar = distance >= 3000;
		bool isMedium = distance < 3000;
		
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
			case ERBLQRFType.PATROL: return COST_QRF_PATROL;
			case ERBLQRFType.CONVOY: return COST_QRF_CONVOY;
			case ERBLQRFType.INFANTRY: return COST_QRF_INFANTRY;
			case ERBLQRFType.MECHANIZED: return COST_QRF_MECHANIZED;
			case ERBLQRFType.HELICOPTER: return COST_QRF_HELICOPTER;
			case ERBLQRFType.SPECOPS: return COST_QRF_SPECOPS;
		}
		return COST_QRF_PATROL;
	}
	
	protected bool LaunchQRF(ERBLQRFType type, RBL_CampaignZone source, RBL_CampaignZone target, int cost)
	{
		m_iFactionResources -= cost;
		m_fTimeSinceLastQRF = 0;
		
		RBL_QRFOperation qrf = new RBL_QRFOperation();
		qrf.Initialize(type, source, target, m_eControlledFaction);
		
		m_aActiveQRFs.Insert(qrf);
		
		PrintFormat("[RBL_AI] QRF Launched! Type: %1, From: %2, To: %3, Cost: %4",
			typename.EnumToString(ERBLQRFType, type),
			source.GetZoneID(),
			target.GetZoneID(),
			cost
		);
		
		return true;
	}
	
	protected int CalculateThreatLevel(RBL_CampaignZone zone)
	{
		int threat = 0;
		
		if (zone.IsUnderAttack())
			threat += 50;
		
		threat += zone.GetStrategicValue() / 20;
		
		int maxGarrison = zone.GetMaxGarrison();
		if (maxGarrison > 0)
		{
			float garrisonRatio = zone.GetCurrentGarrison() / maxGarrison;
			threat += (1.0 - garrisonRatio) * 30;
		}
		
		return Math.Clamp(threat, 0, 100);
	}
	
	protected array<RBL_CampaignZone> GetThreatenedZones()
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
	
	protected RBL_CampaignZone FindNearestFriendlyBase(RBL_CampaignZone targetZone)
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
	
	protected RBL_CampaignZone FindRecaptureTarget()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return null;
		
		RBL_CampaignZone bestTarget = null;
		int highestPriority = 0;
		
		array<RBL_CampaignZone> allZones = zoneMgr.GetAllZones();
		for (int i = 0; i < allZones.Count(); i++)
		{
			RBL_CampaignZone zone = allZones[i];
			
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
	
	protected void ConsiderOffensiveOperation(RBL_CampaignZone target)
	{
		if (m_aActiveQRFs.Count() >= MAX_CONCURRENT_QRFS)
			return;
		
		RBL_CampaignZone source = FindNearestFriendlyBase(target);
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
			LaunchQRF(attackType, source, target, cost);
	}
	
	protected void RegenerateResources()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		int income = 0;
		array<RBL_CampaignZone> allZones = zoneMgr.GetAllZones();
		
		for (int i = 0; i < allZones.Count(); i++)
		{
			RBL_CampaignZone zone = allZones[i];
			if (zone.GetOwnerFaction() == m_eControlledFaction)
				income += zone.CalculateResourceIncome() / 10;
		}
		
		m_iFactionResources += income;
		m_iFactionResources = Math.Min(m_iFactionResources, 10000);
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
}

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
	
	void Initialize(ERBLQRFType type, RBL_CampaignZone source, RBL_CampaignZone target, ERBLFactionKey faction)
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
		
		// Simplified - just mark complete after some time
		m_fTimeStarted += timeSlice;
		if (m_fTimeStarted > 300)
		{
			m_bComplete = true;
			m_bSuccessful = true;
		}
	}
	
	string GetOperationID() { return m_sOperationID; }
	ERBLQRFType GetQRFType() { return m_eType; }
	string GetTargetZoneID() { return m_sTargetZoneID; }
	string GetSourceZoneID() { return m_sSourceZoneID; }
	vector GetCurrentPosition() { return m_vCurrentPosition; }
	float GetTimeStarted() { return m_fTimeStarted; }
	bool IsComplete() { return m_bComplete; }
	bool WasSuccessful() { return m_bSuccessful; }
}
