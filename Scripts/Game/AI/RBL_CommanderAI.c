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
	
	protected const float DECISION_INTERVAL = 30.0;
	protected const float QRF_COOLDOWN = 180.0;
	protected const int MAX_CONCURRENT_QRFS = 3;
	protected const float INTEL_DECAY_RATE = 0.01;
	protected const int UNLOCK_THRESHOLD = 25;
	
	protected const int COST_QRF_PATROL = 50;
	protected const int COST_QRF_CONVOY = 150;
	protected const int COST_QRF_INFANTRY = 100;
	protected const int COST_QRF_MECHANIZED = 300;
	protected const int COST_QRF_HELICOPTER = 500;
	protected const int COST_QRF_SPECOPS = 400;
	
	// ========================================================================
	// STATE
	// ========================================================================
	
	protected ERBLFactionKey m_eControlledFaction;
	protected int m_iFactionResources;
	protected float m_fTimeSinceLastDecision;
	protected float m_fTimeSinceLastQRF;
	
	protected ref map<string, float> m_mZoneIntelAge;
	protected ref map<string, ERBLFactionKey> m_mKnownZoneOwners;
	protected ref map<string, int> m_mKnownGarrisons;
	
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
		
		UpdateActiveQRFs(timeSlice);
		UpdatePatrols(timeSlice);
		UpdateIntelDecay(timeSlice);
		
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
		
		RBL_CampaignZone weakZone = FindWeakestGarrison();
		if (weakZone)
			ConsiderReinforcement(weakZone);
		
		RegenerateResources();
	}
	
	// ========================================================================
	// QRF DECISION ALGORITHM
	// ========================================================================
	
	bool ConsiderQRFResponse(RBL_CampaignZone targetZone)
	{
		if (!targetZone)
			return false;
		
		if (m_fTimeSinceLastQRF < QRF_COOLDOWN)
		{
			PrintFormat("[RBL_AI] QRF on cooldown. Time remaining: %1s", QRF_COOLDOWN - m_fTimeSinceLastQRF);
			return false;
		}
		
		if (m_aActiveQRFs.Count() >= MAX_CONCURRENT_QRFS)
		{
			PrintFormat("[RBL_AI] Max concurrent QRFs reached (%1)", MAX_CONCURRENT_QRFS);
			return false;
		}
		
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
		{
			PrintFormat("[RBL_AI] Threat level %1 below threshold %2. No QRF.", threatLevel, responseThreshold);
			return false;
		}
		
		RBL_CampaignZone sourceBase = FindNearestFriendlyBase(targetZone);
		if (!sourceBase)
		{
			PrintFormat("[RBL_AI] No viable source base for QRF.");
			return false;
		}
		
		ERBLQRFType qrfType = DetermineQRFType(targetZone, sourceBase, threatLevel, warLevel);
		
		int cost = GetQRFCost(qrfType);
		if (m_iFactionResources < cost)
		{
			qrfType = DowngradeQRFType(qrfType);
			cost = GetQRFCost(qrfType);
			
			if (m_iFactionResources < cost)
			{
				PrintFormat("[RBL_AI] Insufficient resources for QRF. Have: %1, Need: %2", m_iFactionResources, cost);
				return false;
			}
		}
		
		return LaunchQRF(qrfType, sourceBase, targetZone, cost);
	}
	
	// ========================================================================
	// QRF TYPE DETERMINATION
	// ========================================================================
	
	protected ERBLQRFType DetermineQRFType(RBL_CampaignZone target, RBL_CampaignZone source, int threatLevel, int warLevel)
	{
		float distance = target.GetDistanceTo(source);
		int zoneValue = target.GetStrategicValue();
		
		bool isClose = distance < 1000;
		bool isMedium = distance < 3000;
		bool isFar = distance >= 3000;
		
		bool isHighValue = zoneValue >= 300;
		bool isCritical = zoneValue >= 500;
		
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
		}
		return ERBLQRFType.PATROL;
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
		m_iFactionResources -= cost;
		m_fTimeSinceLastQRF = 0;
		
		RBL_QRFOperation qrf = new RBL_QRFOperation();
		qrf.Initialize(type, source, target, m_eControlledFaction);
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		int warLevel = 1;
		if (campaignMgr)
			warLevel = campaignMgr.GetWarLevel();
		
		array<string> unitPrefabs = GetQRFComposition(type, warLevel);
		qrf.SetUnitComposition(unitPrefabs);
		
		qrf.SpawnUnits();
		qrf.SetMoveOrder(target.GetZonePosition());
		
		m_aActiveQRFs.Insert(qrf);
		
		PrintFormat("[RBL_AI] QRF Launched! Type: %1, From: %2, To: %3, Cost: %4",
			typename.EnumToString(ERBLQRFType, type),
			source.GetZoneID(),
			target.GetZoneID(),
			cost
		);
		
		if (campaignMgr)
			campaignMgr.OnCampaignEvent(ERBLCampaignEvent.QRF_DISPATCHED, target);
		
		return true;
	}
	
	protected array<string> GetQRFComposition(ERBLQRFType type, int warLevel)
	{
		array<string> units = new array<string>();
		
		string rifleman = "{ABC123}Prefabs/Characters/OPFOR/Rifleman.et";
		string grenadier = "{ABC124}Prefabs/Characters/OPFOR/Grenadier.et";
		string medic = "{ABC125}Prefabs/Characters/OPFOR/Medic.et";
		string mg = "{ABC126}Prefabs/Characters/OPFOR/Machinegunner.et";
		string at = "{ABC127}Prefabs/Characters/OPFOR/AT_Specialist.et";
		string sniper = "{ABC128}Prefabs/Characters/OPFOR/Marksman.et";
		string officer = "{ABC129}Prefabs/Characters/OPFOR/Officer.et";
		string specops = "{ABC130}Prefabs/Characters/OPFOR/SpecOps.et";
		
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
				for (int j = 0; j < 4; j++)
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
				for (int k = 0; k < 8; k++)
					units.Insert(rifleman);
				units.Insert(mg);
				units.Insert(mg);
				units.Insert(medic);
				units.Insert(medic);
				break;
				
			case ERBLQRFType.SPECOPS:
				for (int l = 0; l < 6; l++)
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
		
		if (zone.IsUnderAttack())
			threat += 50;
		
		threat += zone.GetStrategicValue() / 20;
		
		float garrisonRatio = zone.GetCurrentGarrison() / Math.Max(zone.GetMaxGarrison(), 1);
		threat += (1.0 - garrisonRatio) * 30;
		
		string zoneID = zone.GetZoneID();
		float intelAge = 0;
		if (m_mZoneIntelAge.Find(zoneID, intelAge))
		{
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
			
			int priority = zone.GetStrategicValue();
			priority -= zone.GetCurrentGarrison() * 10;
			
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
		for (int i = 0; i < allZones.Count(); i++)
		{
			RBL_CampaignZone zone = allZones[i];
			
			if (zone.GetOwnerFaction() != m_eControlledFaction)
				continue;
			
			float ratio = zone.GetCurrentGarrison() / Math.Max(zone.GetMaxGarrison(), 1);
			if (ratio < lowestRatio && ratio < 0.5)
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
		m_mZoneIntelAge.Set(zoneID, 0);
		m_mKnownZoneOwners.Set(zoneID, observedOwner);
		m_mKnownGarrisons.Set(zoneID, observedGarrison);
	}
	
	protected void UpdateIntelDecay(float timeSlice)
	{
		array<string> keys = new array<string>();
		m_mZoneIntelAge.GetKeyArray(keys);
		
		for (int i = 0; i < keys.Count(); i++)
		{
			string key = keys[i];
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
		for (int i = 0; i < m_aActivePatrols.Count(); i++)
		{
			m_aActivePatrols[i].Update(timeSlice);
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
	
	protected void ConsiderReinforcement(RBL_CampaignZone zone)
	{
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
		
		for (int i = 0; i < allZones.Count(); i++)
		{
			RBL_CampaignZone zone = allZones[i];
			if (zone.GetOwnerFaction() == m_eControlledFaction)
				income += zone.CalculateResourceIncome() / 10;
		}
		
		m_iFactionResources += income;
		m_iFactionResources = Math.Min(m_iFactionResources, 10000);
	}
	
	// ========================================================================
	// SERIALIZATION
	// ========================================================================
	
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
		if (GetGame() && GetGame().GetWorld())
			m_fTimeStarted = GetGame().GetWorld().GetWorldTime();
	}
	
	void SetUnitComposition(array<string> prefabs)
	{
		m_aUnitPrefabs.Clear();
		for (int i = 0; i < prefabs.Count(); i++)
			m_aUnitPrefabs.Insert(prefabs[i]);
	}
	
	void SpawnUnits()
	{
		if (!GetGame() || !GetGame().GetAIWorld())
			return;
			
		m_Group = GetGame().GetAIWorld().CreateGroup();
		
		for (int i = 0; i < m_aUnitPrefabs.Count(); i++)
		{
			string prefabName = m_aUnitPrefabs[i];
			Resource prefab = Resource.Load(prefabName);
			if (!prefab || !prefab.IsValid())
				continue;
			
			EntitySpawnParams params = new EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			
			vector offset = Vector(Math.RandomFloat(-5, 5), 0, Math.RandomFloat(-5, 5));
			params.Transform[3] = m_vCurrentPosition + offset;
			
			IEntity unit = GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld(), params);
			if (unit)
			{
				m_aSpawnedUnits.Insert(unit);
				
				AIControlComponent aiCtrl = AIControlComponent.Cast(unit.FindComponent(AIControlComponent));
				if (aiCtrl && m_Group)
					aiCtrl.JoinGroup(m_Group);
			}
		}
	}
	
	void SetMoveOrder(vector destination)
	{
		m_vTargetPosition = destination;
		
		if (m_Group && GetGame())
		{
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
		
		if (m_aSpawnedUnits.Count() > 0 && m_aSpawnedUnits[0])
			m_vCurrentPosition = m_aSpawnedUnits[0].GetOrigin();
		
		CheckCompletion();
	}
	
	protected void CheckCompletion()
	{
		int aliveCount = 0;
		for (int i = 0; i < m_aSpawnedUnits.Count(); i++)
		{
			IEntity unit = m_aSpawnedUnits[i];
			if (!unit)
				continue;
			
			ChimeraCharacter character = ChimeraCharacter.Cast(unit);
			if (character && character.IsAlive())
				aliveCount++;
		}
		
		if (aliveCount == 0)
		{
			m_bComplete = true;
			m_bSuccessful = false;
			return;
		}
		
		float distToTarget = vector.Distance(m_vCurrentPosition, m_vTargetPosition);
		if (distToTarget < 50)
		{
			m_bComplete = true;
			m_bSuccessful = true;
			return;
		}
	}
	
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
// PATROL ROUTE CLASS
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
	}
}
