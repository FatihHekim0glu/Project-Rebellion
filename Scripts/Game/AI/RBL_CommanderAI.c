// ============================================================================
// PROJECT REBELLION - Commander AI
// Strategic layer AI that manages enemy faction responses (QRF, patrols, attacks)
// Does NOT cheat - relies on "Known Information" system
// ============================================================================

class RBL_CommanderAI
{
	protected static RBL_CommanderAI s_Instance;
	
	// ========================================================================
	// CONFIGURATION
	// ========================================================================
	
	protected const float DECISION_INTERVAL = 30.0;       // Think every 30s
	protected const float QRF_COOLDOWN = 180.0;           // 3 min between QRFs
	protected const int MAX_CONCURRENT_QRFS = 3;          // Max simultaneous QRFs
	protected const float INTEL_DECAY_RATE = 0.01;        // Intel degrades over time
	protected const int UNLOCK_THRESHOLD = 25;            // Items needed for unlimited
	
	// Resource costs per QRF type (subtracted from faction reserves)
	protected const int COST_QRF_PATROL = 50;
	protected const int COST_QRF_CONVOY = 150;
	protected const int COST_QRF_INFANTRY = 100;
	protected const int COST_QRF_MECHANIZED = 300;
	protected const int COST_QRF_HELICOPTER = 500;
	protected const int COST_QRF_SPECOPS = 400;
	
	// ========================================================================
	// STATE
	// ========================================================================
	
	protected ERBLFactionKey m_eControlledFaction;        // Which faction this AI controls
	protected int m_iFactionResources;                    // Resource pool for spawning
	protected float m_fTimeSinceLastDecision;
	protected float m_fTimeSinceLastQRF;
	
	// Known information (simulated fog of war)
	protected ref map<string, float> m_mZoneIntelAge;     // How old is our intel on each zone
	protected ref map<string, ERBLFactionKey> m_mKnownZoneOwners;
	protected ref map<string, int> m_mKnownGarrisons;
	
	// Active operations
	protected ref array<ref RBL_QRFOperation> m_aActiveQRFs;
	protected ref array<ref RBL_PatrolRoute> m_aActivePatrols;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_CommanderAI GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_CommanderAI();
		return s_Instance;
	}
	
	void RBL_CommanderAI()
	{
		m_mZoneIntelAge = new map<string, float>();
		m_mKnownZoneOwners = new map<string, ERBLFactionKey>();
		m_mKnownGarrisons = new map<string, int>();
		m_aActiveQRFs = new array<ref RBL_QRFOperation>();
		m_aActivePatrols = new array<ref RBL_PatrolRoute>();
		
		m_eControlledFaction = ERBLFactionKey.USSR;
		m_iFactionResources = 1000;
		m_fTimeSinceLastDecision = 0;
		m_fTimeSinceLastQRF = QRF_COOLDOWN;
	}
	
	// ========================================================================
	// MAIN LOOP
	// ========================================================================
	
	void Update(float timeSlice)
	{
		if (!Replication.IsServer())
			return;
		
		m_fTimeSinceLastDecision += timeSlice;
		m_fTimeSinceLastQRF += timeSlice;
		
		// Update active operations
		UpdateActiveQRFs(timeSlice);
		UpdatePatrols(timeSlice);
		
		// Decay intel
		UpdateIntelDecay(timeSlice);
		
		// Strategic decision making
		if (m_fTimeSinceLastDecision >= DECISION_INTERVAL)
		{
			m_fTimeSinceLastDecision = 0;
			MakeStrategicDecision();
		}
	}
	
	// ========================================================================
	// STRATEGIC DECISION MAKING
	// ========================================================================
	
	protected void MakeStrategicDecision()
	{
		// Priority 1: Respond to zones under attack
		array<RBL_CampaignZone> threatenedZones = GetThreatenedZones();
		foreach (RBL_CampaignZone zone : threatenedZones)
		{
			ConsiderQRFResponse(zone);
		}
		
		// Priority 2: Recapture lost zones (if resources permit)
		if (m_iFactionResources > 500)
		{
			RBL_CampaignZone targetZone = FindRecaptureTarget();
			if (targetZone)
				ConsiderOffensiveOperation(targetZone);
		}
		
		// Priority 3: Reinforce weak garrisons
		RBL_CampaignZone weakZone = FindWeakestGarrison();
		if (weakZone)
			ConsiderReinforcement(weakZone);
		
		// Regenerate resources based on controlled zones
		RegenerateResources();
	}
	
	// ========================================================================
	// QRF DECISION ALGORITHM
	// The core logic for deciding whether/how to send a Quick Reaction Force
	// ========================================================================
	
	bool ConsiderQRFResponse(RBL_CampaignZone targetZone)
	{
		if (!targetZone)
			return false;
		
		// Check cooldown
		if (m_fTimeSinceLastQRF < QRF_COOLDOWN)
		{
			PrintFormat("[RBL_AI] QRF on cooldown. Time remaining: %1s", QRF_COOLDOWN - m_fTimeSinceLastQRF);
			return false;
		}
		
		// Check concurrent limit
		if (m_aActiveQRFs.Count() >= MAX_CONCURRENT_QRFS)
		{
			PrintFormat("[RBL_AI] Max concurrent QRFs reached (%1)", MAX_CONCURRENT_QRFS);
			return false;
		}
		
		// Check if we own the target zone
		if (targetZone.GetOwnerFaction() != m_eControlledFaction)
			return false;
		
		// Calculate threat level
		int threatLevel = CalculateThreatLevel(targetZone);
		
		// Get campaign aggression and war level
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		int aggression = campaignMgr ? campaignMgr.GetAggression() : 50;
		int warLevel = campaignMgr ? campaignMgr.GetWarLevel() : 1;
		
		// Decision: Should we respond?
		// Higher aggression = lower threshold to respond
		int responseThreshold = 100 - aggression;
		if (threatLevel < responseThreshold)
		{
			PrintFormat("[RBL_AI] Threat level %1 below threshold %2. No QRF.", threatLevel, responseThreshold);
			return false;
		}
		
		// Find source base for QRF
		RBL_CampaignZone sourceBase = FindNearestFriendlyBase(targetZone);
		if (!sourceBase)
		{
			PrintFormat("[RBL_AI] No viable source base for QRF.");
			return false;
		}
		
		// Determine QRF type based on threat, distance, and war level
		ERBLQRFType qrfType = DetermineQRFType(targetZone, sourceBase, threatLevel, warLevel);
		
		// Check if we can afford it
		int cost = GetQRFCost(qrfType);
		if (m_iFactionResources < cost)
		{
			// Try to downgrade
			qrfType = DowngradeQRFType(qrfType);
			cost = GetQRFCost(qrfType);
			
			if (m_iFactionResources < cost)
			{
				PrintFormat("[RBL_AI] Insufficient resources for QRF. Have: %1, Need: %2", m_iFactionResources, cost);
				return false;
			}
		}
		
		// Execute QRF
		return LaunchQRF(qrfType, sourceBase, targetZone, cost);
	}
	
	// ========================================================================
	// QRF TYPE DETERMINATION
	// ========================================================================
	
	protected ERBLQRFType DetermineQRFType(RBL_CampaignZone target, RBL_CampaignZone source, int threatLevel, int warLevel)
	{
		float distance = target.GetDistanceTo(source);
		int zoneValue = target.GetStrategicValue();
		
		// Distance thresholds
		bool isClose = distance < 1000;     // < 1km
		bool isMedium = distance < 3000;    // < 3km
		bool isFar = distance >= 3000;      // 3km+
		
		// High-value target modifiers
		bool isHighValue = zoneValue >= 300;
		bool isCritical = zoneValue >= 500;
		
		// ----------------------------------------------------------------
		// DECISION MATRIX
		// ----------------------------------------------------------------
		
		// Critical targets (HQ, Airbase) always get strong response
		if (isCritical && warLevel >= 5)
			return ERBLQRFType.SPECOPS;
		
		if (isCritical && warLevel >= 3)
			return ERBLQRFType.MECHANIZED;
		
		// Far targets need air mobility (if unlocked)
		if (isFar && warLevel >= 6)
			return ERBLQRFType.HELICOPTER;
		
		// High threat + medium distance = convoy
		if (threatLevel > 70 && isMedium)
			return ERBLQRFType.CONVOY;
		
		// Mechanized available at war level 4+
		if (warLevel >= 4 && threatLevel > 50)
			return ERBLQRFType.MECHANIZED;
		
		// Medium threat = infantry response
		if (threatLevel > 30)
			return ERBLQRFType.INFANTRY;
		
		// Low threat = patrol
		return ERBLQRFType.PATROL;
	}
	
	protected ERBLQRFType DowngradeQRFType(ERBLQRFType current)
	{
		switch (current)
		{
			case ERBLQRFType.SPECOPS:
				return ERBLQRFType.MECHANIZED;
			case ERBLQRFType.HELICOPTER:
				return ERBLQRFType.CONVOY;
			case ERBLQRFType.MECHANIZED:
				return ERBLQRFType.CONVOY;
			case ERBLQRFType.CONVOY:
				return ERBLQRFType.INFANTRY;
			case ERBLQRFType.INFANTRY:
				return ERBLQRFType.PATROL;
			default:
				return ERBLQRFType.PATROL;
		}
	}
	
	protected int GetQRFCost(ERBLQRFType type)
	{
		switch (type)
		{
			case ERBLQRFType.PATROL:
				return COST_QRF_PATROL;
			case ERBLQRFType.CONVOY:
				return COST_QRF_CONVOY;
			case ERBLQRFType.INFANTRY:
				return COST_QRF_INFANTRY;
			case ERBLQRFType.MECHANIZED:
				return COST_QRF_MECHANIZED;
			case ERBLQRFType.HELICOPTER:
				return COST_QRF_HELICOPTER;
			case ERBLQRFType.SPECOPS:
				return COST_QRF_SPECOPS;
		}
		return COST_QRF_PATROL;
	}
	
	// ========================================================================
	// QRF EXECUTION
	// ========================================================================
	
	protected bool LaunchQRF(ERBLQRFType type, RBL_CampaignZone source, RBL_CampaignZone target, int cost)
	{
		// Deduct resources
		m_iFactionResources -= cost;
		m_fTimeSinceLastQRF = 0;
		
		// Create QRF operation
		RBL_QRFOperation qrf = new RBL_QRFOperation();
		qrf.Initialize(type, source, target, m_eControlledFaction);
		
		// Get unit composition based on type and war level
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		int warLevel = campaignMgr ? campaignMgr.GetWarLevel() : 1;
		
		array<string> unitPrefabs = GetQRFComposition(type, warLevel);
		qrf.SetUnitComposition(unitPrefabs);
		
		// Spawn at source base
		qrf.SpawnUnits();
		
		// Issue movement order (A* pathfinding handled by AI)
		qrf.SetMoveOrder(target.GetZonePosition());
		
		// Track operation
		m_aActiveQRFs.Insert(qrf);
		
		PrintFormat("[RBL_AI] QRF Launched! Type: %1, From: %2, To: %3, Cost: %4",
			typename.EnumToString(ERBLQRFType, type),
			source.GetZoneID(),
			target.GetZoneID(),
			cost
		);
		
		// Notify campaign
		if (campaignMgr)
			campaignMgr.OnCampaignEvent(ERBLCampaignEvent.QRF_DISPATCHED, target);
		
		return true;
	}
	
	protected array<string> GetQRFComposition(ERBLQRFType type, int warLevel)
	{
		array<string> units = {};
		
		// Base infantry prefabs (would be actual resource paths)
		string rifleman = "{ABC123}Prefabs/Characters/OPFOR/Rifleman.et";
		string grenadier = "{ABC124}Prefabs/Characters/OPFOR/Grenadier.et";
		string medic = "{ABC125}Prefabs/Characters/OPFOR/Medic.et";
		string mg = "{ABC126}Prefabs/Characters/OPFOR/Machinegunner.et";
		string at = "{ABC127}Prefabs/Characters/OPFOR/AT_Specialist.et";
		string sniper = "{ABC128}Prefabs/Characters/OPFOR/Marksman.et";
		string officer = "{ABC129}Prefabs/Characters/OPFOR/Officer.et";
		string specops = "{ABC130}Prefabs/Characters/OPFOR/SpecOps.et";
		
		// Vehicle prefabs
		string truck = "{VEH001}Prefabs/Vehicles/OPFOR/Truck_Transport.et";
		string apc = "{VEH002}Prefabs/Vehicles/OPFOR/BTR70.et";
		string ifv = "{VEH003}Prefabs/Vehicles/OPFOR/BMP1.et";
		string heli = "{VEH004}Prefabs/Vehicles/OPFOR/Mi8_Transport.et";
		
		switch (type)
		{
			case ERBLQRFType.PATROL:
				units.Insert(rifleman);
				units.Insert(rifleman);
				units.Insert(grenadier);
				break;
				
			case ERBLQRFType.INFANTRY:
				units.Insert(officer);
				units.Insert(rifleman);
				units.Insert(rifleman);
				units.Insert(grenadier);
				units.Insert(mg);
				units.Insert(medic);
				if (warLevel >= 3)
					units.Insert(at);
				break;
				
			case ERBLQRFType.CONVOY:
				units.Insert(truck);
				units.Insert(officer);
				for (int i = 0; i < 6; i++)
					units.Insert(rifleman);
				units.Insert(mg);
				units.Insert(medic);
				if (warLevel >= 4)
				{
					units.Insert(apc);
					units.Insert(at);
				}
				break;
				
			case ERBLQRFType.MECHANIZED:
				units.Insert(apc);
				units.Insert(officer);
				for (int i = 0; i < 4; i++)
					units.Insert(rifleman);
				units.Insert(mg);
				units.Insert(at);
				units.Insert(medic);
				if (warLevel >= 6)
					units.Insert(ifv);
				break;
				
			case ERBLQRFType.HELICOPTER:
				units.Insert(heli);
				units.Insert(officer);
				for (int i = 0; i < 8; i++)
					units.Insert(rifleman);
				units.Insert(mg);
				units.Insert(mg);
				units.Insert(medic);
				units.Insert(medic);
				break;
				
			case ERBLQRFType.SPECOPS:
				for (int i = 0; i < 6; i++)
					units.Insert(specops);
				units.Insert(sniper);
				units.Insert(sniper);
				break;
		}
		
		return units;
	}
	
	// ========================================================================
	// THREAT ASSESSMENT
	// ========================================================================
	
	protected int CalculateThreatLevel(RBL_CampaignZone zone)
	{
		int threat = 0;
		
		// Base threat from being under attack
		if (zone.IsUnderAttack())
			threat += 50;
		
		// Zone value contributes to threat priority
		threat += zone.GetStrategicValue() / 20;
		
		// Low garrison = higher threat
		float garrisonRatio = zone.GetCurrentGarrison() / (float)Math.Max(zone.GetMaxGarrison(), 1);
		threat += (int)((1.0 - garrisonRatio) * 30);
		
		// Recent intel increases accuracy
		string zoneID = zone.GetZoneID();
		float intelAge = 0;
		if (m_mZoneIntelAge.Find(zoneID, intelAge))
		{
			// Fresh intel (< 60s) = accurate threat assessment
			// Old intel (> 300s) = reduced confidence
			if (intelAge < 60)
				threat += 10;
			else if (intelAge > 300)
				threat -= 20;
		}
		
		return Math.Clamp(threat, 0, 100);
	}
	
	// ========================================================================
	// ZONE FINDING UTILITIES
	// ========================================================================
	
	protected array<RBL_CampaignZone> GetThreatenedZones()
	{
		array<RBL_CampaignZone> result = {};
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return result;
		
		array<RBL_CampaignZone> allZones = zoneMgr.GetAllZones();
		foreach (RBL_CampaignZone zone : allZones)
		{
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
		float nearestDist = float.MAX;
		
		array<RBL_CampaignZone> allZones = zoneMgr.GetAllZones();
		foreach (RBL_CampaignZone zone : allZones)
		{
			// Must be our zone
			if (zone.GetOwnerFaction() != m_eControlledFaction)
				continue;
			
			// Must be a base type (not the target itself)
			if (zone == targetZone)
				continue;
			
			ERBLZoneType type = zone.GetZoneType();
			bool isBase = (type == ERBLZoneType.Airbase || 
			               type == ERBLZoneType.Outpost || 
			               type == ERBLZoneType.HQ);
			
			if (!isBase)
				continue;
			
			// Check distance
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
		foreach (RBL_CampaignZone zone : allZones)
		{
			// Look for zones owned by enemy (player faction)
			if (zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				continue;
			
			// Calculate priority based on value and garrison
			int priority = zone.GetStrategicValue();
			priority -= zone.GetCurrentGarrison() * 10; // Strong garrisons are less attractive
			
			if (priority > highestPriority)
			{
				highestPriority = priority;
				bestTarget = zone;
			}
		}
		
		return bestTarget;
	}
	
	protected RBL_CampaignZone FindWeakestGarrison()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return null;
		
		RBL_CampaignZone weakest = null;
		float lowestRatio = 1.0;
		
		array<RBL_CampaignZone> allZones = zoneMgr.GetAllZones();
		foreach (RBL_CampaignZone zone : allZones)
		{
			if (zone.GetOwnerFaction() != m_eControlledFaction)
				continue;
			
			float ratio = zone.GetCurrentGarrison() / (float)Math.Max(zone.GetMaxGarrison(), 1);
			if (ratio < lowestRatio && ratio < 0.5) // Below 50% strength
			{
				lowestRatio = ratio;
				weakest = zone;
			}
		}
		
		return weakest;
	}
	
	// ========================================================================
	// INTEL SYSTEM
	// ========================================================================
	
	void UpdateIntel(string zoneID, ERBLFactionKey observedOwner, int observedGarrison)
	{
		m_mZoneIntelAge.Set(zoneID, 0); // Reset age
		m_mKnownZoneOwners.Set(zoneID, observedOwner);
		m_mKnownGarrisons.Set(zoneID, observedGarrison);
	}
	
	protected void UpdateIntelDecay(float timeSlice)
	{
		array<string> keys = {};
		m_mZoneIntelAge.GetKeyArray(keys);
		
		foreach (string key : keys)
		{
			float age = m_mZoneIntelAge.Get(key);
			m_mZoneIntelAge.Set(key, age + timeSlice);
		}
	}
	
	// ========================================================================
	// OPERATION MANAGEMENT
	// ========================================================================
	
	protected void UpdateActiveQRFs(float timeSlice)
	{
		for (int i = m_aActiveQRFs.Count() - 1; i >= 0; i--)
		{
			RBL_QRFOperation qrf = m_aActiveQRFs[i];
			qrf.Update(timeSlice);
			
			if (qrf.IsComplete())
			{
				HandleQRFCompletion(qrf);
				m_aActiveQRFs.Remove(i);
			}
		}
	}
	
	protected void HandleQRFCompletion(RBL_QRFOperation qrf)
	{
		if (qrf.WasSuccessful())
		{
			PrintFormat("[RBL_AI] QRF to %1 succeeded.", qrf.GetTargetZoneID());
		}
		else
		{
			PrintFormat("[RBL_AI] QRF to %1 failed/destroyed.", qrf.GetTargetZoneID());
			
			RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
			if (campaignMgr)
				campaignMgr.OnCampaignEvent(ERBLCampaignEvent.QRF_DESTROYED, null);
		}
	}
	
	protected void UpdatePatrols(float timeSlice)
	{
		// Patrol management - simplified
		foreach (RBL_PatrolRoute patrol : m_aActivePatrols)
		{
			patrol.Update(timeSlice);
		}
	}
	
	protected void ConsiderOffensiveOperation(RBL_CampaignZone target)
	{
		// Offensive operations use same QRF system but with stronger composition
		// and no cooldown restrictions
		if (m_aActiveQRFs.Count() >= MAX_CONCURRENT_QRFS)
			return;
		
		RBL_CampaignZone source = FindNearestFriendlyBase(target);
		if (!source)
			return;
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		int warLevel = campaignMgr ? campaignMgr.GetWarLevel() : 1;
		
		ERBLQRFType attackType = (warLevel >= 6) ? ERBLQRFType.MECHANIZED : ERBLQRFType.CONVOY;
		int cost = GetQRFCost(attackType) * 2; // Offensive ops cost more
		
		if (m_iFactionResources >= cost)
			LaunchQRF(attackType, source, target, cost);
	}
	
	protected void ConsiderReinforcement(RBL_CampaignZone zone)
	{
		// Send small patrol to reinforce weak garrison
		if (m_iFactionResources < COST_QRF_PATROL * 2)
			return;
		
		RBL_CampaignZone source = FindNearestFriendlyBase(zone);
		if (source)
			LaunchQRF(ERBLQRFType.PATROL, source, zone, COST_QRF_PATROL);
	}
	
	// ========================================================================
	// RESOURCE MANAGEMENT
	// ========================================================================
	
	protected void RegenerateResources()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		int income = 0;
		array<RBL_CampaignZone> allZones = zoneMgr.GetAllZones();
		
		foreach (RBL_CampaignZone zone : allZones)
		{
			if (zone.GetOwnerFaction() == m_eControlledFaction)
				income += zone.CalculateResourceIncome() / 10; // Scaled down for tick rate
		}
		
		m_iFactionResources += income;
		
		// Cap resources
		m_iFactionResources = Math.Min(m_iFactionResources, 10000);
	}
	
	// ========================================================================
	// SERIALIZATION
	// ========================================================================
	
	array<ref RBL_ActiveMissionData> GetActiveMissionData()
	{
		array<ref RBL_ActiveMissionData> result = {};
		
		foreach (RBL_QRFOperation qrf : m_aActiveQRFs)
		{
			RBL_ActiveMissionData data = new RBL_ActiveMissionData();
			data.MissionID = qrf.GetOperationID();
			data.MissionType = qrf.GetQRFType();
			data.TargetZoneID = qrf.GetTargetZoneID();
			data.SourceZoneID = qrf.GetSourceZoneID();
			data.CurrentPosition = qrf.GetCurrentPosition();
			data.AssignedUnits = qrf.GetAssignedUnitPrefabs();
			data.TimeStarted = qrf.GetTimeStarted();
			result.Insert(data);
		}
		
		return result;
	}
	
	// ========================================================================
	// GETTERS
	// ========================================================================
	
	int GetFactionResources() { return m_iFactionResources; }
	int GetActiveQRFCount() { return m_aActiveQRFs.Count(); }
}

// ============================================================================
// QRF OPERATION CLASS
// Tracks individual QRF missions from spawn to completion
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
	
	protected ref array<IEntity> m_aSpawnedUnits;
	protected ref array<string> m_aUnitPrefabs;
	protected AIGroup m_Group;
	
	void RBL_QRFOperation()
	{
		m_aSpawnedUnits = new array<IEntity>();
		m_aUnitPrefabs = new array<string>();
		m_sOperationID = "QRF_" + System.GetTickCount().ToString();
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
		m_fTimeStarted = GetGame().GetWorld().GetWorldTime();
	}
	
	void SetUnitComposition(array<string> prefabs)
	{
		m_aUnitPrefabs.Clear();
		foreach (string prefab : prefabs)
			m_aUnitPrefabs.Insert(prefab);
	}
	
	void SpawnUnits()
	{
		// Create AI group
		m_Group = GetGame().GetAIWorld().CreateGroup();
		
		foreach (string prefabName : m_aUnitPrefabs)
		{
			Resource prefab = Resource.Load(prefabName);
			if (!prefab || !prefab.IsValid())
				continue;
			
			EntitySpawnParams params = new EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			
			// Spread spawn positions
			vector offset = Vector(
				Math.RandomFloat(-5, 5),
				0,
				Math.RandomFloat(-5, 5)
			);
			params.Transform[3] = m_vCurrentPosition + offset;
			
			IEntity unit = GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld(), params);
			if (unit)
			{
				m_aSpawnedUnits.Insert(unit);
				
				// Add to group
				AIControlComponent aiCtrl = AIControlComponent.Cast(unit.FindComponent(AIControlComponent));
				if (aiCtrl && m_Group)
					aiCtrl.JoinGroup(m_Group);
			}
		}
	}
	
	void SetMoveOrder(vector destination)
	{
		m_vTargetPosition = destination;
		
		if (m_Group)
		{
			// Issue waypoint through AI system
			// Actual implementation would use SCR_AIWaypoint
			AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntity(AIWaypoint));
			if (wp)
			{
				wp.SetOrigin(destination);
				m_Group.AddWaypoint(wp);
			}
		}
	}
	
	void Update(float timeSlice)
	{
		if (m_bComplete)
			return;
		
		// Update position tracking
		if (m_aSpawnedUnits.Count() > 0 && m_aSpawnedUnits[0])
			m_vCurrentPosition = m_aSpawnedUnits[0].GetOrigin();
		
		// Check completion conditions
		CheckCompletion();
	}
	
	protected void CheckCompletion()
	{
		// Count alive units
		int aliveCount = 0;
		foreach (IEntity unit : m_aSpawnedUnits)
		{
			if (!unit)
				continue;
			
			ChimeraCharacter character = ChimeraCharacter.Cast(unit);
			if (character && character.IsAlive())
				aliveCount++;
		}
		
		// All units dead = failed
		if (aliveCount == 0)
		{
			m_bComplete = true;
			m_bSuccessful = false;
			return;
		}
		
		// Check if reached target
		float distToTarget = vector.Distance(m_vCurrentPosition, m_vTargetPosition);
		if (distToTarget < 50)
		{
			m_bComplete = true;
			m_bSuccessful = true;
			return;
		}
	}
	
	// Getters
	string GetOperationID() { return m_sOperationID; }
	ERBLQRFType GetQRFType() { return m_eType; }
	string GetTargetZoneID() { return m_sTargetZoneID; }
	string GetSourceZoneID() { return m_sSourceZoneID; }
	vector GetCurrentPosition() { return m_vCurrentPosition; }
	array<string> GetAssignedUnitPrefabs() { return m_aUnitPrefabs; }
	float GetTimeStarted() { return m_fTimeStarted; }
	bool IsComplete() { return m_bComplete; }
	bool WasSuccessful() { return m_bSuccessful; }
}

// ============================================================================
// PATROL ROUTE CLASS (Simplified)
// ============================================================================
class RBL_PatrolRoute
{
	protected ref array<vector> m_aWaypoints;
	protected int m_iCurrentWaypoint;
	protected ref array<IEntity> m_aUnits;
	
	void RBL_PatrolRoute()
	{
		m_aWaypoints = new array<vector>();
		m_aUnits = new array<IEntity>();
		m_iCurrentWaypoint = 0;
	}
	
	void Update(float timeSlice)
	{
		// Patrol logic
	}
}

