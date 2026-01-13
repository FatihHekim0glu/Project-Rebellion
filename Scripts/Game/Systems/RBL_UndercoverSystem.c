// ============================================================================
// PROJECT REBELLION - Undercover Detection System
// Handles player stealth, suspicion buildup, and enemy awareness
// ============================================================================

// Detection factor weights
class RBL_DetectionFactors
{
	// Suspicion build rates (per second when factor is active)
	static const float WEAPON_VISIBLE = 0.30;
	static const float MILITARY_UNIFORM = 0.20;
	static const float MILITARY_VEHICLE = 0.15;
	static const float RUNNING_NEAR_ENEMY = 0.10;
	static const float RESTRICTED_ZONE = 0.20;
	static const float ENEMY_VERY_CLOSE = 0.25;
	
	// Suspicion decay rate (per second when safe)
	static const float DECAY_RATE = 0.08;
	
	// Detection distances
	static const float ENEMY_DETECTION_RANGE = 50.0;
	static const float ENEMY_CLOSE_RANGE = 15.0;
	static const float RESTRICTED_ZONE_RANGE = 100.0;
	
	// Status thresholds (0-1 scale)
	static const float THRESHOLD_SUSPICIOUS = 0.25;
	static const float THRESHOLD_SPOTTED = 0.50;
	static const float THRESHOLD_COMPROMISED = 0.75;
}

class RBL_UndercoverSystem
{
	protected static ref RBL_UndercoverSystem s_Instance;
	
	protected const float CHECK_INTERVAL = 0.5;
	protected float m_fTimeSinceCheck;
	
	protected ref array<string> m_aIllegalWeapons;
	protected ref array<string> m_aIllegalClothing;
	protected ref array<string> m_aIllegalVehicles;
	protected ref array<string> m_aRestrictedZoneTypes;
	
	protected ref map<int, ref RBL_PlayerCoverState> m_mPlayerStates;
	
	// Events
	protected ref ScriptInvoker m_OnStatusChanged;
	protected ref ScriptInvoker m_OnCoverBlown;
	
	static RBL_UndercoverSystem GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_UndercoverSystem();
		return s_Instance;
	}
	
	void RBL_UndercoverSystem()
	{
		m_mPlayerStates = new map<int, ref RBL_PlayerCoverState>();
		m_OnStatusChanged = new ScriptInvoker();
		m_OnCoverBlown = new ScriptInvoker();
		m_fTimeSinceCheck = 0;
		
		InitializeIllegalItems();
		InitializeRestrictedZones();
		
		PrintFormat("[RBL_Undercover] System initialized");
	}
	
	protected void InitializeIllegalItems()
	{
		m_aIllegalWeapons = new array<string>();
		m_aIllegalClothing = new array<string>();
		m_aIllegalVehicles = new array<string>();
		
		// Illegal weapons (military rifles, MGs, launchers)
		m_aIllegalWeapons.Insert("AK74");
		m_aIllegalWeapons.Insert("AKM");
		m_aIllegalWeapons.Insert("AK");
		m_aIllegalWeapons.Insert("RPG");
		m_aIllegalWeapons.Insert("PKM");
		m_aIllegalWeapons.Insert("M16");
		m_aIllegalWeapons.Insert("M249");
		m_aIllegalWeapons.Insert("M4");
		m_aIllegalWeapons.Insert("SVD");
		m_aIllegalWeapons.Insert("Mosin");
		m_aIllegalWeapons.Insert("SKS");
		
		// Illegal clothing (military uniforms)
		m_aIllegalClothing.Insert("USSR");
		m_aIllegalClothing.Insert("US_Army");
		m_aIllegalClothing.Insert("FIA");
		m_aIllegalClothing.Insert("Uniform_");
		m_aIllegalClothing.Insert("Vest_Carrier");
		m_aIllegalClothing.Insert("Vest_Plate");
		m_aIllegalClothing.Insert("Helmet_");
		
		// Illegal vehicles (military)
		m_aIllegalVehicles.Insert("BTR");
		m_aIllegalVehicles.Insert("BMP");
		m_aIllegalVehicles.Insert("M113");
		m_aIllegalVehicles.Insert("HMMWV");
		m_aIllegalVehicles.Insert("UAZ469_MG");
		m_aIllegalVehicles.Insert("Ural4320_Covered");
	}
	
	protected void InitializeRestrictedZones()
	{
		m_aRestrictedZoneTypes = new array<string>();
		m_aRestrictedZoneTypes.Insert("HQ");
		m_aRestrictedZoneTypes.Insert("Airbase");
		m_aRestrictedZoneTypes.Insert("Outpost");
	}
	
	// ========================================================================
	// MAIN UPDATE LOOP
	// ========================================================================
	void Update(float timeSlice)
	{
		m_fTimeSinceCheck += timeSlice;
		
		if (m_fTimeSinceCheck < CHECK_INTERVAL)
			return;
		
		m_fTimeSinceCheck = 0;
		
		// Check all players
		array<int> playerIDs = GetAllPlayerIDs();
		
		for (int i = 0; i < playerIDs.Count(); i++)
		{
			int playerID = playerIDs[i];
			IEntity playerEntity = GetPlayerEntity(playerID);
			
			if (!playerEntity)
				continue;
			
			CheckPlayerDetection(playerID, playerEntity, CHECK_INTERVAL);
		}
	}
	
	protected array<int> GetAllPlayerIDs()
	{
		array<int> ids = new array<int>();
		
		// Get player manager
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return ids;
		
		// Get all player IDs
		array<int> allPlayers = new array<int>();
		pm.GetPlayers(allPlayers);
		
		return allPlayers;
	}
	
	protected IEntity GetPlayerEntity(int playerID)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return null;
		
		return pm.GetPlayerControlledEntity(playerID);
	}
	
	// ========================================================================
	// DETECTION CHECKS
	// ========================================================================
	protected void CheckPlayerDetection(int playerID, IEntity playerEntity, float deltaTime)
	{
		RBL_PlayerCoverState state = GetOrCreatePlayerState(playerID);
		vector playerPos = playerEntity.GetOrigin();
		
		// Calculate suspicion change this frame
		float suspicionDelta = 0;
		bool instantCompromise = false;
		string compromiseReason = "";
		
		// Check each detection factor
		bool hasIllegalWeapon = CheckWeaponVisibility(playerEntity);
		bool hasMilitaryClothing = CheckClothing(playerEntity);
		bool inMilitaryVehicle = CheckVehicle(playerEntity);
		bool nearEnemy = false;
		bool veryCloseToEnemy = false;
		bool inRestrictedZone = false;
		bool isRunning = CheckIfRunning(playerEntity);
		
		// Enemy proximity check
		float nearestEnemyDist = GetNearestEnemyDistance(playerPos);
		nearEnemy = (nearestEnemyDist <= RBL_DetectionFactors.ENEMY_DETECTION_RANGE);
		veryCloseToEnemy = (nearestEnemyDist <= RBL_DetectionFactors.ENEMY_CLOSE_RANGE);
		
		// Restricted zone check
		inRestrictedZone = CheckRestrictedZone(playerPos);
		
		// Build suspicion based on factors
		if (nearEnemy)
		{
			if (hasIllegalWeapon)
				suspicionDelta += RBL_DetectionFactors.WEAPON_VISIBLE * deltaTime;
			
			if (hasMilitaryClothing)
				suspicionDelta += RBL_DetectionFactors.MILITARY_UNIFORM * deltaTime;
			
			if (inMilitaryVehicle)
				suspicionDelta += RBL_DetectionFactors.MILITARY_VEHICLE * deltaTime;
			
			if (isRunning)
				suspicionDelta += RBL_DetectionFactors.RUNNING_NEAR_ENEMY * deltaTime;
			
			if (veryCloseToEnemy)
				suspicionDelta += RBL_DetectionFactors.ENEMY_VERY_CLOSE * deltaTime;
		}
		
		if (inRestrictedZone)
			suspicionDelta += RBL_DetectionFactors.RESTRICTED_ZONE * deltaTime;
		
		// Check for instant compromise triggers
		if (state.m_bJustFiredWeapon && nearEnemy)
		{
			instantCompromise = true;
			compromiseReason = "Fired weapon near enemies";
		}
		
		if (state.m_bJustAttackedEnemy)
		{
			instantCompromise = true;
			compromiseReason = "Attacked enemy soldier";
		}
		
		// Apply suspicion change
		if (instantCompromise)
		{
			state.m_fSuspicionLevel = 1.0;
			state.m_sCompromiseReason = compromiseReason;
		}
		else if (suspicionDelta > 0)
		{
			state.m_fSuspicionLevel = Math.Clamp(state.m_fSuspicionLevel + suspicionDelta, 0, 1);
		}
		else
		{
			// Decay suspicion when safe
			float decay = RBL_DetectionFactors.DECAY_RATE * deltaTime;
			state.m_fSuspicionLevel = Math.Clamp(state.m_fSuspicionLevel - decay, 0, 1);
		}
		
		// Reset frame flags
		state.m_bJustFiredWeapon = false;
		state.m_bJustAttackedEnemy = false;
		
		// Update detection factors for debug/HUD
		state.m_bHasIllegalWeapon = hasIllegalWeapon;
		state.m_bHasMilitaryClothing = hasMilitaryClothing;
		state.m_bInMilitaryVehicle = inMilitaryVehicle;
		state.m_bNearEnemy = nearEnemy;
		state.m_bInRestrictedZone = inRestrictedZone;
		state.m_fNearestEnemyDistance = nearestEnemyDist;
		
		// Update status based on suspicion level
		UpdatePlayerStatus(playerID, state);
	}
	
	// ========================================================================
	// INDIVIDUAL CHECKS
	// ========================================================================
	protected bool CheckWeaponVisibility(IEntity playerEntity)
	{
		if (!playerEntity)
			return false;
		
		// Get weapon manager
		BaseWeaponManagerComponent weaponMgr = BaseWeaponManagerComponent.Cast(
			playerEntity.FindComponent(BaseWeaponManagerComponent));
		
		if (!weaponMgr)
			return false;
		
		// Check current weapon
		BaseWeaponComponent currentWeapon = weaponMgr.GetCurrentWeapon();
		if (!currentWeapon)
			return false;
		
		IEntity weaponEntity = currentWeapon.GetOwner();
		if (!weaponEntity)
			return false;
		
		string prefabName = weaponEntity.GetPrefabData().GetPrefabName();
		return IsIllegalWeapon(prefabName);
	}
	
	protected bool CheckClothing(IEntity playerEntity)
	{
		if (!playerEntity)
			return false;
		
		// Get character entity prefab name
		string prefabName = playerEntity.GetPrefabData().GetPrefabName();
		
		// Check if wearing military clothing
		return IsIllegalClothing(prefabName);
	}
	
	protected bool CheckVehicle(IEntity playerEntity)
	{
		if (!playerEntity)
			return false;
		
		// Check if player is in a vehicle
		CompartmentAccessComponent compartment = CompartmentAccessComponent.Cast(
			playerEntity.FindComponent(CompartmentAccessComponent));
		
		if (!compartment)
			return false;
		
		BaseCompartmentSlot slot = compartment.GetCompartment();
		if (!slot)
			return false;
		
		IEntity vehicle = slot.GetOwner();
		if (!vehicle)
			return false;
		
		string prefabName = vehicle.GetPrefabData().GetPrefabName();
		return IsIllegalVehicle(prefabName);
	}
	
	protected bool CheckIfRunning(IEntity playerEntity)
	{
		if (!playerEntity)
			return false;
		
		CharacterControllerComponent charCtrl = CharacterControllerComponent.Cast(
			playerEntity.FindComponent(CharacterControllerComponent));
		
		if (!charCtrl)
			return false;
		
		// Check if sprinting
		return charCtrl.IsSprinting();
	}
	
	protected float GetNearestEnemyDistance(vector playerPos)
	{
		float nearestDist = 99999.0;
		
		// Query nearby entities
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return nearestDist;
		
		// Get all characters in range
		array<IEntity> nearbyEntities = new array<IEntity>();
		GetGame().GetWorld().QueryEntitiesBySphere(
			playerPos,
			RBL_DetectionFactors.ENEMY_DETECTION_RANGE * 2,
			QueryEnemyCallback,
			FilterEnemyCallback,
			EQueryEntitiesFlags.DYNAMIC
		);
		
		// Find nearest USSR/enemy soldier
		// This is handled by the callback storing results
		return m_fQueryNearestEnemy;
	}
	
	// Query result storage
	protected float m_fQueryNearestEnemy = 99999.0;
	protected vector m_vQueryOrigin;
	
	protected bool QueryEnemyCallback(IEntity entity)
	{
		if (!entity)
			return true;
		
		// Check if it's an AI character
		AIControlComponent aiControl = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
		if (!aiControl)
			return true;
		
		// Check faction
		FactionAffiliationComponent faction = FactionAffiliationComponent.Cast(
			entity.FindComponent(FactionAffiliationComponent));
		
		if (!faction)
			return true;
		
		Faction entityFaction = faction.GetAffiliatedFaction();
		if (!entityFaction)
			return true;
		
		string factionKey = entityFaction.GetFactionKey();
		
		// Check if USSR or enemy faction
		if (factionKey == "USSR" || factionKey == "OPFOR")
		{
			float dist = vector.Distance(m_vQueryOrigin, entity.GetOrigin());
			if (dist < m_fQueryNearestEnemy)
				m_fQueryNearestEnemy = dist;
		}
		
		return true;
	}
	
	protected bool FilterEnemyCallback(IEntity entity)
	{
		return entity != null;
	}
	
	protected bool CheckRestrictedZone(vector playerPos)
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return false;
		
		// Check virtual zones
		array<ref RBL_VirtualZone> zones = zoneMgr.GetAllVirtualZones();
		for (int i = 0; i < zones.Count(); i++)
		{
			RBL_VirtualZone zone = zones[i];
			if (!zone)
				continue;
			
			// Only enemy-controlled zones are restricted
			if (zone.GetOwnerFaction() == ERBLFactionKey.FIA)
				continue;
			
			// Check if zone type is restricted
			ERBLZoneType zoneType = zone.GetZoneType();
			if (zoneType == ERBLZoneType.HQ || zoneType == ERBLZoneType.Airbase || zoneType == ERBLZoneType.Outpost)
			{
				float dist = vector.Distance(playerPos, zone.GetZonePosition());
				if (dist <= zone.GetCaptureRadius())
					return true;
			}
		}
		
		return false;
	}
	
	// ========================================================================
	// STATUS MANAGEMENT
	// ========================================================================
	protected void UpdatePlayerStatus(int playerID, RBL_PlayerCoverState state)
	{
		ERBLCoverStatus oldStatus = state.m_eCurrentStatus;
		ERBLCoverStatus newStatus;
		
		// Determine status from suspicion level
		if (state.m_fSuspicionLevel >= RBL_DetectionFactors.THRESHOLD_COMPROMISED)
			newStatus = ERBLCoverStatus.COMPROMISED;
		else if (state.m_fSuspicionLevel >= RBL_DetectionFactors.THRESHOLD_SPOTTED)
			newStatus = ERBLCoverStatus.SPOTTED;
		else if (state.m_fSuspicionLevel >= RBL_DetectionFactors.THRESHOLD_SUSPICIOUS)
			newStatus = ERBLCoverStatus.SUSPICIOUS;
		else
			newStatus = ERBLCoverStatus.HIDDEN;
		
		// Check for status change
		if (newStatus != oldStatus)
		{
			state.SetStatus(newStatus);
			
			// Notify
			m_OnStatusChanged.Invoke(playerID, oldStatus, newStatus);
			
			// Log status changes
			string statusName = typename.EnumToString(ERBLCoverStatus, newStatus);
			PrintFormat("[RBL_Undercover] Player %1 status: %2 (%.0f%% suspicion)", 
				playerID, statusName, state.m_fSuspicionLevel * 100);
			
			// Special handling for compromise
			if (newStatus == ERBLCoverStatus.COMPROMISED && !state.m_bCoverBlownNotified)
			{
				state.m_bCoverBlownNotified = true;
				m_OnCoverBlown.Invoke(playerID, state.m_sCompromiseReason);
				PrintFormat("[RBL_Undercover] *** COVER BLOWN: %1 ***", state.m_sCompromiseReason);
			}
		}
	}
	
	// ========================================================================
	// EVENT TRIGGERS (Call from game events)
	// ========================================================================
	void OnPlayerFiredWeapon(int playerID)
	{
		RBL_PlayerCoverState state = GetOrCreatePlayerState(playerID);
		state.m_bJustFiredWeapon = true;
	}
	
	void OnPlayerAttackedEnemy(int playerID)
	{
		RBL_PlayerCoverState state = GetOrCreatePlayerState(playerID);
		state.m_bJustAttackedEnemy = true;
	}
	
	void OnPlayerEnteredVehicle(int playerID, IEntity vehicle)
	{
		// Handled in regular check
	}
	
	// ========================================================================
	// HELPER METHODS
	// ========================================================================
	protected bool IsIllegalWeapon(string prefabName)
	{
		for (int i = 0; i < m_aIllegalWeapons.Count(); i++)
		{
			if (prefabName.Contains(m_aIllegalWeapons[i]))
				return true;
		}
		return false;
	}
	
	protected bool IsIllegalClothing(string prefabName)
	{
		for (int i = 0; i < m_aIllegalClothing.Count(); i++)
		{
			if (prefabName.Contains(m_aIllegalClothing[i]))
				return true;
		}
		return false;
	}
	
	protected bool IsIllegalVehicle(string prefabName)
	{
		for (int i = 0; i < m_aIllegalVehicles.Count(); i++)
		{
			if (prefabName.Contains(m_aIllegalVehicles[i]))
				return true;
		}
		return false;
	}
	
	protected RBL_PlayerCoverState GetOrCreatePlayerState(int playerID)
	{
		RBL_PlayerCoverState state;
		if (!m_mPlayerStates.Find(playerID, state))
		{
			state = new RBL_PlayerCoverState();
			m_mPlayerStates.Set(playerID, state);
		}
		return state;
	}
	
	// ========================================================================
	// PUBLIC API
	// ========================================================================
	ERBLCoverStatus GetPlayerCoverStatus(int playerID)
	{
		RBL_PlayerCoverState state;
		if (m_mPlayerStates.Find(playerID, state))
			return state.m_eCurrentStatus;
		return ERBLCoverStatus.HIDDEN;
	}
	
	float GetPlayerSuspicionLevel(int playerID)
	{
		RBL_PlayerCoverState state;
		if (m_mPlayerStates.Find(playerID, state))
			return state.m_fSuspicionLevel;
		return 0;
	}
	
	bool IsPlayerUndercover(int playerID)
	{
		ERBLCoverStatus status = GetPlayerCoverStatus(playerID);
		return (status == ERBLCoverStatus.HIDDEN || status == ERBLCoverStatus.SUSPICIOUS);
	}
	
	bool IsPlayerCompromised(int playerID)
	{
		return GetPlayerCoverStatus(playerID) == ERBLCoverStatus.COMPROMISED;
	}
	
	RBL_PlayerCoverState GetPlayerState(int playerID)
	{
		RBL_PlayerCoverState state;
		m_mPlayerStates.Find(playerID, state);
		return state;
	}
	
	RBL_PlayerCoverState GetPlayerState(IEntity playerEntity)
	{
		if (!playerEntity)
			return null;
		
		int playerID = GetPlayerIDFromEntity(playerEntity);
		if (playerID < 0)
			return null;
		
		return GetPlayerState(playerID);
	}
	
	protected int GetPlayerIDFromEntity(IEntity entity)
	{
		if (!entity)
			return -1;
		
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return -1;
		
		array<int> playerIds = new array<int>();
		pm.GetPlayers(playerIds);
		
		for (int i = 0; i < playerIds.Count(); i++)
		{
			IEntity playerEntity = pm.GetPlayerControlledEntity(playerIds[i]);
			if (playerEntity == entity)
				return playerIds[i];
		}
		
		return -1;
	}
	
	void ResetPlayerCover(int playerID)
	{
		RBL_PlayerCoverState state;
		if (m_mPlayerStates.Find(playerID, state))
		{
			state.m_fSuspicionLevel = 0;
			state.m_eCurrentStatus = ERBLCoverStatus.HIDDEN;
			state.m_bCoverBlownNotified = false;
			state.m_sCompromiseReason = "";
		}
	}
	
	// Item list management
	void AddIllegalWeapon(string prefabName)
	{
		if (m_aIllegalWeapons.Find(prefabName) == -1)
			m_aIllegalWeapons.Insert(prefabName);
	}
	
	void AddIllegalClothing(string prefabName)
	{
		if (m_aIllegalClothing.Find(prefabName) == -1)
			m_aIllegalClothing.Insert(prefabName);
	}
	
	void AddIllegalVehicle(string prefabName)
	{
		if (m_aIllegalVehicles.Find(prefabName) == -1)
			m_aIllegalVehicles.Insert(prefabName);
	}
	
	// Event accessors
	ScriptInvoker GetOnStatusChanged() { return m_OnStatusChanged; }
	ScriptInvoker GetOnCoverBlown() { return m_OnCoverBlown; }
	
	// Stats
	int GetTrackedPlayerCount() { return m_mPlayerStates.Count(); }
}

// ============================================================================
// PLAYER COVER STATE
// ============================================================================
class RBL_PlayerCoverState
{
	ERBLCoverStatus m_eCurrentStatus;
	ERBLCoverStatus m_ePreviousStatus;
	float m_fSuspicionLevel;
	float m_fTimeSinceStatusChange;
	bool m_bCoverBlownNotified;
	string m_sCompromiseReason;
	
	// Current frame detection flags
	bool m_bJustFiredWeapon;
	bool m_bJustAttackedEnemy;
	
	// Current detection factors (for HUD display)
	bool m_bHasIllegalWeapon;
	bool m_bHasMilitaryClothing;
	bool m_bInMilitaryVehicle;
	bool m_bNearEnemy;
	bool m_bInRestrictedZone;
	float m_fNearestEnemyDistance;
	int m_iNearbyEnemyCount;
	
	void RBL_PlayerCoverState()
	{
		m_eCurrentStatus = ERBLCoverStatus.HIDDEN;
		m_ePreviousStatus = ERBLCoverStatus.HIDDEN;
		m_fSuspicionLevel = 0;
		m_fTimeSinceStatusChange = 0;
		m_bCoverBlownNotified = false;
		m_sCompromiseReason = "";
		
		m_bJustFiredWeapon = false;
		m_bJustAttackedEnemy = false;
		m_bHasIllegalWeapon = false;
		m_bHasMilitaryClothing = false;
		m_bInMilitaryVehicle = false;
		m_bNearEnemy = false;
		m_bInRestrictedZone = false;
		m_fNearestEnemyDistance = 99999;
		m_iNearbyEnemyCount = 0;
	}
	
	// Getters for UI
	ERBLCoverStatus GetStatus() { return m_eCurrentStatus; }
	float GetSuspicion() { return m_fSuspicionLevel * 100.0; }
	float GetSuspicionNormalized() { return m_fSuspicionLevel; }
	int GetNearbyEnemyCount() { return m_iNearbyEnemyCount; }
	float GetClosestEnemyDistance() { return m_fNearestEnemyDistance; }
	bool HasIllegalWeapon() { return m_bHasIllegalWeapon; }
	bool HasMilitaryClothing() { return m_bHasMilitaryClothing; }
	bool IsInMilitaryVehicle() { return m_bInMilitaryVehicle; }
	bool IsNearEnemy() { return m_bNearEnemy; }
	bool IsInRestrictedZone() { return m_bInRestrictedZone; }
	
	void SetStatus(ERBLCoverStatus newStatus)
	{
		if (m_eCurrentStatus != newStatus)
		{
			m_ePreviousStatus = m_eCurrentStatus;
			m_eCurrentStatus = newStatus;
			m_fTimeSinceStatusChange = 0;
			
			if (newStatus == ERBLCoverStatus.HIDDEN)
			{
				m_bCoverBlownNotified = false;
				m_sCompromiseReason = "";
			}
		}
	}
	
	// Get status as string for display
	string GetStatusString()
	{
		switch (m_eCurrentStatus)
		{
			case ERBLCoverStatus.HIDDEN: return "HIDDEN";
			case ERBLCoverStatus.SUSPICIOUS: return "SUSPICIOUS";
			case ERBLCoverStatus.SPOTTED: return "SPOTTED";
			case ERBLCoverStatus.COMPROMISED: return "COMPROMISED";
		}
		return "UNKNOWN";
	}
	
	// Get detection factors as string
	string GetFactorsString()
	{
		string factors = "";
		if (m_bHasIllegalWeapon) factors += "Weapon ";
		if (m_bHasMilitaryClothing) factors += "Uniform ";
		if (m_bInMilitaryVehicle) factors += "Vehicle ";
		if (m_bNearEnemy) factors += "Enemy ";
		if (m_bInRestrictedZone) factors += "Restricted ";
		
		if (factors.IsEmpty())
			return "None";
		return factors;
	}
}

// ============================================================================
// DEBUG COMMANDS
// ============================================================================
class RBL_UndercoverCommands
{
	static void PrintStatus()
	{
		RBL_UndercoverSystem system = RBL_UndercoverSystem.GetInstance();
		if (!system)
			return;
		
		PrintFormat("[RBL_Undercover] === UNDERCOVER STATUS ===");
		PrintFormat("Tracked players: %1", system.GetTrackedPlayerCount());
		
		// Get local player status
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			int playerID = pc.GetPlayerId();
			RBL_PlayerCoverState state = system.GetPlayerState(playerID);
			if (state)
			{
				PrintFormat("Your status: %1", state.GetStatusString());
				PrintFormat("Suspicion: %.0f%%", state.m_fSuspicionLevel * 100);
				PrintFormat("Factors: %1", state.GetFactorsString());
				PrintFormat("Nearest enemy: %.0fm", state.m_fNearestEnemyDistance);
			}
		}
	}
	
	static void ResetCover()
	{
		RBL_UndercoverSystem system = RBL_UndercoverSystem.GetInstance();
		PlayerController pc = GetGame().GetPlayerController();
		
		if (system && pc)
		{
			system.ResetPlayerCover(pc.GetPlayerId());
			PrintFormat("[RBL_Undercover] Cover reset for local player");
		}
	}
	
	static void SetSuspicion(float level)
	{
		RBL_UndercoverSystem system = RBL_UndercoverSystem.GetInstance();
		PlayerController pc = GetGame().GetPlayerController();
		
		if (system && pc)
		{
			RBL_PlayerCoverState state = system.GetPlayerState(pc.GetPlayerId());
			if (state)
			{
				state.m_fSuspicionLevel = Math.Clamp(level, 0, 1);
				PrintFormat("[RBL_Undercover] Suspicion set to %.0f%%", level * 100);
			}
		}
	}
}
