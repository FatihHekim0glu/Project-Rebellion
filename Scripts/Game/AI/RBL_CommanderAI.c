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
// QRF TEMPLATES - Unit compositions for each QRF type
// ============================================================================
class RBL_QRFTemplates
{
	// USSR infantry prefabs
	static const string PREFAB_USSR_RIFLEMAN = "{6D62B5D93627D}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_Rifleman.et";
	static const string PREFAB_USSR_MG = "{DCB41E1B7E697DE8}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_MG.et";
	static const string PREFAB_USSR_AT = "{2A79433F26F22D2D}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_AT.et";
	static const string PREFAB_USSR_MEDIC = "{6B5F3AE10AA0C35E}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_Medic.et";
	static const string PREFAB_USSR_OFFICER = "{E8C5ED3082B4D2A1}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_Officer.et";
	static const string PREFAB_USSR_SNIPER = "{DF25789F0CD75F22}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_Sniper.et";
	
	// USSR vehicle prefabs
	static const string PREFAB_UAZ = "{5E74787BB083789B}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469.et";
	static const string PREFAB_UAZ_MG = "{C38D37EE2C0E0888}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469_MG.et";
	static const string PREFAB_URAL = "{91B01E6C0D20E1D1}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320.et";
	static const string PREFAB_BTR70 = "{D85A504DF4E2C128}Prefabs/Vehicles/Wheeled/BTR70/BTR70.et";
	static const string PREFAB_BMP1 = "{BB0E7CE42F0F3E19}Prefabs/Vehicles/Tracked/BMP1/BMP1.et";
	static const string PREFAB_MI8 = "{3D2E5D6E8C5F9A1B}Prefabs/Vehicles/Helicopters/Mi8/Mi8MT.et";
	
	// Get infantry count for QRF type
	static int GetInfantryCount(ERBLQRFType type, int warLevel)
	{
		int base = 0;
		int scaling = 0;
		
		switch (type)
		{
			case ERBLQRFType.PATROL:
				base = 2;
				scaling = warLevel / 3;
				return Math.ClampInt(base + scaling, 2, 4);
				
			case ERBLQRFType.CONVOY:
				base = 4;
				scaling = warLevel / 2;
				return Math.ClampInt(base + scaling, 4, 8);
				
			case ERBLQRFType.INFANTRY:
				base = 8;
				scaling = warLevel;
				return Math.ClampInt(base + scaling, 8, 16);
				
			case ERBLQRFType.MECHANIZED:
				base = 6;
				scaling = warLevel / 2;
				return Math.ClampInt(base + scaling, 6, 10);
				
			case ERBLQRFType.HELICOPTER:
				base = 6;
				scaling = warLevel / 3;
				return Math.ClampInt(base + scaling, 6, 10);
				
			case ERBLQRFType.SPECOPS:
				base = 4;
				scaling = warLevel / 4;
				return Math.ClampInt(base + scaling, 4, 6);
		}
		return 4;
	}
	
	// Get vehicle count for QRF type
	static int GetVehicleCount(ERBLQRFType type, int warLevel)
	{
		switch (type)
		{
			case ERBLQRFType.PATROL: return 1;
			case ERBLQRFType.CONVOY: return 2;
			case ERBLQRFType.INFANTRY: return 0;
			case ERBLQRFType.MECHANIZED: return 1;
			case ERBLQRFType.HELICOPTER: return 1;
			case ERBLQRFType.SPECOPS: return 0;
		}
		return 0;
	}
	
	// Get infantry prefabs for composition
	static void GetInfantryPrefabs(ERBLQRFType type, int warLevel, out array<string> prefabs)
	{
		prefabs = new array<string>();
		int count = GetInfantryCount(type, warLevel);
		
		// SPECOPS gets elite units
		if (type == ERBLQRFType.SPECOPS)
		{
			prefabs.Insert(PREFAB_USSR_OFFICER);
			prefabs.Insert(PREFAB_USSR_SNIPER);
			for (int i = 2; i < count; i++)
			{
				if (i % 2 == 0)
					prefabs.Insert(PREFAB_USSR_AT);
				else
					prefabs.Insert(PREFAB_USSR_MG);
			}
			return;
		}
		
		// Standard composition
		prefabs.Insert(PREFAB_USSR_OFFICER); // Squad leader
		
		for (int i = 1; i < count; i++)
		{
			int role = i % 5;
			switch (role)
			{
				case 0: prefabs.Insert(PREFAB_USSR_RIFLEMAN); break;
				case 1: prefabs.Insert(PREFAB_USSR_RIFLEMAN); break;
				case 2: prefabs.Insert(PREFAB_USSR_MG); break;
				case 3: prefabs.Insert(PREFAB_USSR_MEDIC); break;
				case 4: prefabs.Insert(PREFAB_USSR_AT); break;
			}
		}
		
		// Add sniper at high war levels
		if (warLevel >= 7 && count >= 6)
			prefabs[count - 1] = PREFAB_USSR_SNIPER;
	}
	
	// Get vehicle prefab for type
	static string GetVehiclePrefab(ERBLQRFType type, int warLevel)
	{
		switch (type)
		{
			case ERBLQRFType.PATROL:
				return (warLevel >= 5) ? PREFAB_UAZ_MG : PREFAB_UAZ;
				
			case ERBLQRFType.CONVOY:
				return PREFAB_URAL;
				
			case ERBLQRFType.MECHANIZED:
				return (warLevel >= 6) ? PREFAB_BMP1 : PREFAB_BTR70;
				
			case ERBLQRFType.HELICOPTER:
				return PREFAB_MI8;
		}
		return PREFAB_UAZ;
	}
}

// ============================================================================
// QRF OPERATION STATE
// ============================================================================
// ============================================================================
// QRF Operation - Now spawns and tracks actual units
// ERBLQRFState enum moved to RBL_Enums.c for centralization
// ============================================================================
class RBL_QRFOperation
{
	protected string m_sOperationID;
	protected ERBLQRFType m_eType;
	protected ERBLFactionKey m_eFaction;
	protected ERBLQRFState m_eState;
	protected string m_sTargetZoneID;
	protected string m_sSourceZoneID;
	protected vector m_vTargetPosition;
	protected vector m_vSourcePosition;
	protected float m_fTimeStarted;
	protected float m_fTimeSinceUpdate;
	protected int m_iWarLevel;
	
	// Spawned entities
	protected ref array<IEntity> m_aSpawnedUnits;
	protected ref array<IEntity> m_aSpawnedVehicles;
	protected AIGroup m_AIGroup;
	
	protected const float UPDATE_INTERVAL = 2.0;
	protected const float ARRIVAL_DISTANCE = 100.0;
	protected const float TIMEOUT_SECONDS = 600.0;
	
	void RBL_QRFOperation()
	{
		m_sOperationID = "QRF_" + Math.RandomInt(10000, 99999).ToString();
		m_aSpawnedUnits = new array<IEntity>();
		m_aSpawnedVehicles = new array<IEntity>();
		m_eState = ERBLQRFState.SPAWNING;
		m_fTimeStarted = 0;
		m_fTimeSinceUpdate = 0;
		m_iWarLevel = 1;
	}
	
	void InitializeFromVirtual(ERBLQRFType type, RBL_VirtualZone source, RBL_VirtualZone target, ERBLFactionKey faction)
	{
		m_eType = type;
		m_eFaction = faction;
		m_sTargetZoneID = target.GetZoneID();
		m_sSourceZoneID = source.GetZoneID();
		m_vTargetPosition = target.GetZonePosition();
		m_vSourcePosition = source.GetZonePosition();
		
		RBL_CampaignManager campaign = RBL_CampaignManager.GetInstance();
		if (campaign)
			m_iWarLevel = campaign.GetWarLevel();
		
		PrintFormat("[RBL_QRF] Initializing %1 from %2 to %3 (WL%4)", 
			typename.EnumToString(ERBLQRFType, type), m_sSourceZoneID, m_sTargetZoneID, m_iWarLevel);
		
		SpawnUnits();
	}
	
	void InitializeFromEntity(ERBLQRFType type, RBL_CampaignZone source, RBL_CampaignZone target, ERBLFactionKey faction)
	{
		m_eType = type;
		m_eFaction = faction;
		m_sTargetZoneID = target.GetZoneID();
		m_sSourceZoneID = source.GetZoneID();
		m_vTargetPosition = target.GetZonePosition();
		m_vSourcePosition = source.GetZonePosition();
		
		RBL_CampaignManager campaign = RBL_CampaignManager.GetInstance();
		if (campaign)
			m_iWarLevel = campaign.GetWarLevel();
		
		PrintFormat("[RBL_QRF] Initializing %1 from %2 to %3 (WL%4)", 
			typename.EnumToString(ERBLQRFType, type), m_sSourceZoneID, m_sTargetZoneID, m_iWarLevel);
		
		SpawnUnits();
	}
	
	// ========================================================================
	// UNIT SPAWNING
	// ========================================================================
	protected void SpawnUnits()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
		{
			PrintFormat("[RBL_QRF] ERROR: No world for spawning");
			m_eState = ERBLQRFState.DESTROYED;
			return;
		}
		
		// Get unit composition
		array<string> infantryPrefabs;
		RBL_QRFTemplates.GetInfantryPrefabs(m_eType, m_iWarLevel, infantryPrefabs);
		int vehicleCount = RBL_QRFTemplates.GetVehicleCount(m_eType, m_iWarLevel);
		
		PrintFormat("[RBL_QRF] Spawning %1: %2 infantry, %3 vehicles", 
			m_sOperationID, infantryPrefabs.Count(), vehicleCount);
		
		// Generate spawn positions around source
		array<vector> spawnPositions = GenerateSpawnPositions(m_vSourcePosition, 20.0, infantryPrefabs.Count());
		
		// Spawn infantry
		for (int i = 0; i < infantryPrefabs.Count(); i++)
		{
			if (i >= spawnPositions.Count())
				break;
				
			IEntity unit = SpawnEntity(infantryPrefabs[i], spawnPositions[i]);
			if (unit)
			{
				m_aSpawnedUnits.Insert(unit);
				AssignToAIGroup(unit);
			}
		}
		
		// Spawn vehicles
		for (int i = 0; i < vehicleCount; i++)
		{
			string vehiclePrefab = RBL_QRFTemplates.GetVehiclePrefab(m_eType, m_iWarLevel);
			vector vehiclePos = m_vSourcePosition + Vector(Math.RandomFloat(-30, 30), 0, Math.RandomFloat(-30, 30));
			vehiclePos[1] = world.GetSurfaceY(vehiclePos[0], vehiclePos[2]);
			
			IEntity vehicle = SpawnEntity(vehiclePrefab, vehiclePos);
			if (vehicle)
				m_aSpawnedVehicles.Insert(vehicle);
		}
		
		// Set up waypoint to target
		if (m_aSpawnedUnits.Count() > 0)
		{
			CreateMoveWaypoint();
			m_eState = ERBLQRFState.EN_ROUTE;
			PrintFormat("[RBL_QRF] %1 deployed: %2 units, %3 vehicles", 
				m_sOperationID, m_aSpawnedUnits.Count(), m_aSpawnedVehicles.Count());
		}
		else
		{
			m_eState = ERBLQRFState.DESTROYED;
			PrintFormat("[RBL_QRF] %1 failed to spawn any units", m_sOperationID);
		}
	}
	
	protected IEntity SpawnEntity(string prefab, vector position)
	{
		if (prefab.IsEmpty())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return null;
		
		// Adjust to terrain
		position[1] = world.GetSurfaceY(position[0], position[2]);
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position;
		
		// Random rotation
		float yaw = Math.RandomFloat(0, 360);
		vector angles = Vector(0, yaw, 0);
		Math3D.AnglesToMatrix(angles, params.Transform);
		params.Transform[3] = position;
		
		Resource resource = Resource.Load(prefab);
		if (!resource.IsValid())
		{
			PrintFormat("[RBL_QRF] Invalid prefab: %1", prefab);
			return null;
		}
		
		return GetGame().SpawnEntityPrefab(resource, world, params);
	}
	
	protected array<vector> GenerateSpawnPositions(vector center, float radius, int count)
	{
		array<vector> positions = new array<vector>();
		BaseWorld world = GetGame().GetWorld();
		
		for (int i = 0; i < count; i++)
		{
			float angle = (i / (float)count) * Math.PI2;
			float dist = radius * 0.5 + Math.RandomFloat(0, radius * 0.5);
			
			float x = center[0] + Math.Cos(angle) * dist;
			float z = center[2] + Math.Sin(angle) * dist;
			float y = center[1];
			
			if (world)
				y = world.GetSurfaceY(x, z);
			
			positions.Insert(Vector(x, y, z));
		}
		
		return positions;
	}
	
	// ========================================================================
	// AI GROUP & MOVEMENT
	// ========================================================================
	protected void AssignToAIGroup(IEntity unit)
	{
		if (!unit)
			return;
		
		AIControlComponent aiControl = AIControlComponent.Cast(unit.FindComponent(AIControlComponent));
		if (!aiControl)
			return;
		
		AIAgent agent = aiControl.GetAIAgent();
		if (!agent)
			return;
		
		// Create group if needed
		if (!m_AIGroup)
		{
			AIWorld aiWorld = GetGame().GetAIWorld();
			if (aiWorld)
				m_AIGroup = aiWorld.CreateGroup();
		}
		
		if (m_AIGroup)
			m_AIGroup.AddAgent(agent);
	}
	
	protected void CreateMoveWaypoint()
	{
		if (!m_AIGroup)
			return;
		
		// Create waypoint at target position
		AIWaypoint wp = CreateWaypointEntity(m_vTargetPosition);
		if (wp)
		{
			m_AIGroup.AddWaypoint(wp);
			PrintFormat("[RBL_QRF] Waypoint created at target %1", m_sTargetZoneID);
		}
	}
	
	protected AIWaypoint CreateWaypointEntity(vector position)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return null;
		
		// Adjust to terrain
		position[1] = world.GetSurfaceY(position[0], position[2]);
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position;
		
		// Use move waypoint prefab
		string wpPrefab = "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
		Resource resource = Resource.Load(wpPrefab);
		if (!resource.IsValid())
		{
			PrintFormat("[RBL_QRF] Failed to load waypoint prefab");
			return null;
		}
		
		IEntity wpEntity = GetGame().SpawnEntityPrefab(resource, world, params);
		return AIWaypoint.Cast(wpEntity);
	}
	
	// ========================================================================
	// UPDATE & TRACKING
	// ========================================================================
	void Update(float timeSlice)
	{
		if (m_eState == ERBLQRFState.COMPLETE || m_eState == ERBLQRFState.DESTROYED || m_eState == ERBLQRFState.TIMEOUT)
			return;
		
		m_fTimeStarted += timeSlice;
		m_fTimeSinceUpdate += timeSlice;
		
		// Periodic update
		if (m_fTimeSinceUpdate >= UPDATE_INTERVAL)
		{
			m_fTimeSinceUpdate = 0;
			UpdateUnitStatus();
		}
		
		// Check timeout
		if (m_fTimeStarted > TIMEOUT_SECONDS)
		{
			PrintFormat("[RBL_QRF] %1 timed out after %2s", m_sOperationID, m_fTimeStarted);
			m_eState = ERBLQRFState.TIMEOUT;
			Cleanup();
		}
	}
	
	protected void UpdateUnitStatus()
	{
		// Count alive units
		int alive = CountAliveUnits();
		
		if (alive == 0)
		{
			PrintFormat("[RBL_QRF] %1 destroyed - all units KIA", m_sOperationID);
			m_eState = ERBLQRFState.DESTROYED;
			Cleanup();
			return;
		}
		
		// Check if arrived at target
		vector avgPosition = GetAverageUnitPosition();
		float distToTarget = vector.Distance(avgPosition, m_vTargetPosition);
		
		if (distToTarget <= ARRIVAL_DISTANCE)
		{
			PrintFormat("[RBL_QRF] %1 ARRIVED at %2 with %3 units", m_sOperationID, m_sTargetZoneID, alive);
			m_eState = ERBLQRFState.ARRIVED;
			OnArrival();
		}
	}
	
	protected void OnArrival()
	{
		// Reinforce the garrison at target zone
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
		{
			RBL_GarrisonData garData = garMgr.GetGarrisonData(m_sTargetZoneID);
			if (garData)
			{
				// Transfer units to garrison
				for (int i = 0; i < m_aSpawnedUnits.Count(); i++)
				{
					IEntity unit = m_aSpawnedUnits[i];
					if (unit && IsUnitAlive(unit))
						garData.SpawnedUnits.Insert(unit);
				}
				for (int i = 0; i < m_aSpawnedVehicles.Count(); i++)
				{
					IEntity veh = m_aSpawnedVehicles[i];
					if (veh)
						garData.SpawnedVehicles.Insert(veh);
				}
				
				PrintFormat("[RBL_QRF] Units transferred to garrison at %1", m_sTargetZoneID);
			}
		}
		
		// Clear our arrays (don't delete entities - they're now part of garrison)
		m_aSpawnedUnits.Clear();
		m_aSpawnedVehicles.Clear();
		m_eState = ERBLQRFState.COMPLETE;
	}
	
	int CountAliveUnits()
	{
		int alive = 0;
		for (int i = m_aSpawnedUnits.Count() - 1; i >= 0; i--)
		{
			IEntity unit = m_aSpawnedUnits[i];
			if (!unit)
			{
				m_aSpawnedUnits.Remove(i);
				continue;
			}
			
			if (IsUnitAlive(unit))
				alive++;
			else
				m_aSpawnedUnits.Remove(i);
		}
		return alive;
	}
	
	protected bool IsUnitAlive(IEntity unit)
	{
		if (!unit)
			return false;
		
		DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(unit.FindComponent(DamageManagerComponent));
		if (dmgMgr && dmgMgr.GetState() == EDamageState.DESTROYED)
			return false;
		
		return true;
	}
	
	vector GetAverageUnitPosition()
	{
		if (m_aSpawnedUnits.Count() == 0)
			return m_vTargetPosition;
		
		vector sum = Vector(0, 0, 0);
		int count = 0;
		
		for (int i = 0; i < m_aSpawnedUnits.Count(); i++)
		{
			IEntity unit = m_aSpawnedUnits[i];
			if (unit && IsUnitAlive(unit))
			{
				sum = sum + unit.GetOrigin();
				count++;
			}
		}
		
		if (count == 0)
			return m_vTargetPosition;
		
		return sum * (1.0 / count);
	}
	
	// ========================================================================
	// CLEANUP
	// ========================================================================
	void Cleanup()
	{
		// Delete all spawned units
		for (int i = m_aSpawnedUnits.Count() - 1; i >= 0; i--)
		{
			IEntity unit = m_aSpawnedUnits[i];
			if (unit)
				SCR_EntityHelper.DeleteEntityAndChildren(unit);
		}
		m_aSpawnedUnits.Clear();
		
		// Delete vehicles
		for (int i = m_aSpawnedVehicles.Count() - 1; i >= 0; i--)
		{
			IEntity veh = m_aSpawnedVehicles[i];
			if (veh)
				SCR_EntityHelper.DeleteEntityAndChildren(veh);
		}
		m_aSpawnedVehicles.Clear();
		
		// Delete AI group
		if (m_AIGroup)
		{
			m_AIGroup.Delete();
			m_AIGroup = null;
		}
	}
	
	// ========================================================================
	// GETTERS
	// ========================================================================
	string GetOperationID() { return m_sOperationID; }
	ERBLQRFType GetQRFType() { return m_eType; }
	ERBLQRFState GetState() { return m_eState; }
	string GetTargetZoneID() { return m_sTargetZoneID; }
	string GetSourceZoneID() { return m_sSourceZoneID; }
	vector GetCurrentPosition() { return GetAverageUnitPosition(); }
	vector GetTargetPosition() { return m_vTargetPosition; }
	float GetTimeStarted() { return m_fTimeStarted; }
	int GetAliveCount() { return CountAliveUnits(); }
	int GetTotalSpawned() { return m_aSpawnedUnits.Count() + m_aSpawnedVehicles.Count(); }
	
	bool IsComplete() 
	{ 
		return m_eState == ERBLQRFState.COMPLETE || 
			   m_eState == ERBLQRFState.DESTROYED || 
			   m_eState == ERBLQRFState.TIMEOUT; 
	}
	
	bool WasSuccessful() 
	{ 
		return m_eState == ERBLQRFState.ARRIVED || m_eState == ERBLQRFState.COMPLETE; 
	}
}
