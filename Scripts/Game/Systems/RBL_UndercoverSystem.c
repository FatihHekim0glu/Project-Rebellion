// ============================================================================
// PROJECT REBELLION - Undercover Detection System
// Determines if players are disguised/hidden from enemy detection
// ============================================================================

class RBL_UndercoverSystem
{
	protected static RBL_UndercoverSystem s_Instance;
	
	// ========================================================================
	// CONFIGURATION
	// ========================================================================
	
	protected const float CHECK_INTERVAL = 1.0;           // Check every second
	protected const float DETECTION_RADIUS = 50.0;        // Enemy awareness range
	protected const float CLOSE_INSPECTION_RADIUS = 10.0; // Close scrutiny range
	protected const float SUSPICION_BUILD_RATE = 0.15;    // Per second when suspicious
	protected const float SUSPICION_DECAY_RATE = 0.05;    // Per second when safe
	protected const float COMPROMISE_THRESHOLD = 1.0;     // Suspicion level to blow cover
	
	// ========================================================================
	// ILLEGAL ITEMS
	// Items that will compromise undercover status if visible
	// ========================================================================
	
	protected ref array<string> m_aIllegalWeapons;
	protected ref array<string> m_aIllegalClothing;
	protected ref array<string> m_aIllegalVehicles;
	
	// Player tracking
	protected ref map<int, ref RBL_PlayerCoverState> m_mPlayerStates;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_UndercoverSystem GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_UndercoverSystem();
		return s_Instance;
	}
	
	void RBL_UndercoverSystem()
	{
		m_mPlayerStates = new map<int, ref RBL_PlayerCoverState>();
		
		InitializeIllegalItems();
	}
	
	protected void InitializeIllegalItems()
	{
		m_aIllegalWeapons = {};
		m_aIllegalClothing = {};
		m_aIllegalVehicles = {};
		
		// Weapons that will blow cover immediately if drawn/visible
		m_aIllegalWeapons.Insert("{WEAP001}AK74");
		m_aIllegalWeapons.Insert("{WEAP002}AKM");
		m_aIllegalWeapons.Insert("{WEAP003}RPG7");
		m_aIllegalWeapons.Insert("{WEAP004}PKM");
		m_aIllegalWeapons.Insert("{WEAP005}SVD");
		m_aIllegalWeapons.Insert("{WEAP006}M16A2");
		m_aIllegalWeapons.Insert("{WEAP007}M249");
		m_aIllegalWeapons.Insert("{WEAP008}M72_LAW");
		m_aIllegalWeapons.Insert("{WEAP009}Grenade_Frag");
		// Pistols are NOT illegal - civilians can carry concealed
		
		// Military clothing that marks you as combatant
		m_aIllegalClothing.Insert("{CLOTH001}Uniform_FIA");
		m_aIllegalClothing.Insert("{CLOTH002}Uniform_USSR");
		m_aIllegalClothing.Insert("{CLOTH003}Uniform_US");
		m_aIllegalClothing.Insert("{CLOTH004}Vest_Tactical");
		m_aIllegalClothing.Insert("{CLOTH005}Vest_PlateCarrier");
		m_aIllegalClothing.Insert("{CLOTH006}Helmet_Combat");
		m_aIllegalClothing.Insert("{CLOTH007}Helmet_SSh68");
		m_aIllegalClothing.Insert("{CLOTH008}Backpack_Military");
		// Civilian clothes are safe
		
		// Military vehicles are immediately suspicious
		m_aIllegalVehicles.Insert("{VEH001}BTR70");
		m_aIllegalVehicles.Insert("{VEH002}BMP1");
		m_aIllegalVehicles.Insert("{VEH003}UAZ_Armed");
		m_aIllegalVehicles.Insert("{VEH004}Ural_Military");
		m_aIllegalVehicles.Insert("{VEH005}M113");
		m_aIllegalVehicles.Insert("{VEH006}HMMWV_Armed");
		// Civilian cars/trucks are safe
	}
	
	// ========================================================================
	// MAIN UPDATE
	// ========================================================================
	
	void Update(float timeSlice)
	{
		// Get all player-controlled characters
		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		
		foreach (int playerID : playerIDs)
		{
			IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!playerEntity)
				continue;
			
			ChimeraCharacter character = ChimeraCharacter.Cast(playerEntity);
			if (!character || !character.IsAlive())
				continue;
			
			// Get or create state
			RBL_PlayerCoverState state = GetOrCreatePlayerState(playerID);
			
			// Update undercover check
			UpdatePlayerCoverStatus(character, state, timeSlice);
		}
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
	// CORE DETECTION LOGIC
	// ========================================================================
	
	protected void UpdatePlayerCoverStatus(ChimeraCharacter player, RBL_PlayerCoverState state, float timeSlice)
	{
		// Step 1: Check immediate disqualifiers
		ERBLCoverStatus immediateStatus = CheckImmediateCompromise(player);
		if (immediateStatus == ERBLCoverStatus.HOSTILE)
		{
			// Instant compromise - in combat or holding weapon
			state.SetStatus(ERBLCoverStatus.HOSTILE);
			state.m_fSuspicionLevel = 1.0;
			return;
		}
		
		// Step 2: Check gear legality
		float gearThreat = CalculateGearThreat(player);
		
		// Step 3: Check vehicle
		float vehicleThreat = CalculateVehicleThreat(player);
		
		// Step 4: Check nearby enemies
		float proximityThreat = CalculateProximityThreat(player);
		
		// Step 5: Combine threats
		float totalThreat = Math.Max(gearThreat, Math.Max(vehicleThreat, proximityThreat));
		
		// Step 6: Update suspicion
		if (totalThreat > 0)
		{
			// Suspicion builds
			state.m_fSuspicionLevel += totalThreat * SUSPICION_BUILD_RATE * timeSlice;
		}
		else
		{
			// Suspicion decays
			state.m_fSuspicionLevel -= SUSPICION_DECAY_RATE * timeSlice;
		}
		
		state.m_fSuspicionLevel = Math.Clamp(state.m_fSuspicionLevel, 0, 1.0);
		
		// Step 7: Determine status from suspicion
		if (state.m_fSuspicionLevel >= COMPROMISE_THRESHOLD)
		{
			state.SetStatus(ERBLCoverStatus.COMPROMISED);
			OnCoverBlown(player, state);
		}
		else if (state.m_fSuspicionLevel > 0.5)
		{
			state.SetStatus(ERBLCoverStatus.SUSPICIOUS);
		}
		else
		{
			state.SetStatus(ERBLCoverStatus.HIDDEN);
		}
	}
	
	// ========================================================================
	// THREAT CALCULATORS
	// ========================================================================
	
	protected ERBLCoverStatus CheckImmediateCompromise(ChimeraCharacter player)
	{
		// Check if player is currently in combat (firing weapon)
		SCR_CharacterControllerComponent charCtrl = SCR_CharacterControllerComponent.Cast(
			player.FindComponent(SCR_CharacterControllerComponent)
		);
		
		if (charCtrl)
		{
			// Check if weapon is raised/aiming
			if (charCtrl.IsWeaponRaised())
				return ERBLCoverStatus.HOSTILE;
		}
		
		// Check if currently engaged by enemy
		DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(
			player.FindComponent(DamageManagerComponent)
		);
		
		if (dmgMgr)
		{
			// Recently took damage = compromised
			// Would check m_fLastDamageTime if exposed
		}
		
		return ERBLCoverStatus.HIDDEN;
	}
	
	protected float CalculateGearThreat(ChimeraCharacter player)
	{
		float threat = 0;
		
		// Get inventory
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(
			player.FindComponent(SCR_InventoryStorageManagerComponent)
		);
		
		if (!inventory)
			return 0;
		
		// Check equipped weapon (most important)
		BaseWeaponManagerComponent weaponMgr = BaseWeaponManagerComponent.Cast(
			player.FindComponent(BaseWeaponManagerComponent)
		);
		
		if (weaponMgr)
		{
			BaseWeaponComponent currentWeapon = weaponMgr.GetCurrentWeapon();
			if (currentWeapon)
			{
				string weaponPrefab = GetPrefabName(currentWeapon.GetOwner());
				
				// Check if weapon is illegal
				if (IsIllegalWeapon(weaponPrefab))
				{
					// Weapon in hands = major threat
					threat = Math.Max(threat, 0.8);
				}
			}
		}
		
		// Check visible clothing
		threat = Math.Max(threat, CheckClothingThreat(player, inventory));
		
		return threat;
	}
	
	protected float CheckClothingThreat(ChimeraCharacter player, SCR_InventoryStorageManagerComponent inventory)
	{
		float threat = 0;
		
		// Get all equipped items
		array<IEntity> equippedItems = {};
		inventory.GetAllItems(equippedItems);
		
		foreach (IEntity item : equippedItems)
		{
			string prefabName = GetPrefabName(item);
			
			// Check clothing slots
			if (IsIllegalClothing(prefabName))
			{
				// Military uniform = high threat
				// Combat vest = medium threat
				// Helmet = medium threat
				if (prefabName.Contains("Uniform"))
					threat = Math.Max(threat, 0.7);
				else if (prefabName.Contains("Vest") || prefabName.Contains("Helmet"))
					threat = Math.Max(threat, 0.5);
				else
					threat = Math.Max(threat, 0.3);
			}
		}
		
		return threat;
	}
	
	protected float CalculateVehicleThreat(ChimeraCharacter player)
	{
		// Check if in vehicle
		CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(
			player.FindComponent(CompartmentAccessComponent)
		);
		
		if (!compAccess || !compAccess.IsInCompartment())
			return 0;
		
		// Get vehicle entity
		IEntity vehicle = compAccess.GetCompartment().GetOwner();
		if (!vehicle)
			return 0;
		
		string vehiclePrefab = GetPrefabName(vehicle);
		
		if (IsIllegalVehicle(vehiclePrefab))
		{
			// Military vehicle = instant compromise when enemy sees
			return 1.0;
		}
		
		return 0;
	}
	
	protected float CalculateProximityThreat(ChimeraCharacter player)
	{
		float threat = 0;
		vector playerPos = player.GetOrigin();
		
		// Query nearby entities
		array<IEntity> nearbyEntities = {};
		GetGame().GetWorld().QueryEntitiesBySphere(
			playerPos,
			DETECTION_RADIUS,
			null,
			FilterEnemyUnits,
			EQueryEntitiesFlags.DYNAMIC
		);
		
		foreach (IEntity entity : nearbyEntities)
		{
			ChimeraCharacter enemy = ChimeraCharacter.Cast(entity);
			if (!enemy || !enemy.IsAlive())
				continue;
			
			// Check if actually enemy faction
			FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(
				enemy.FindComponent(FactionAffiliationComponent)
			);
			
			if (!factionComp)
				continue;
			
			Faction enemyFaction = factionComp.GetAffiliatedFaction();
			if (!enemyFaction || !IsEnemyFaction(enemyFaction))
				continue;
			
			// Calculate threat based on distance
			float distance = vector.Distance(playerPos, enemy.GetOrigin());
			
			// Check line of sight
			if (!HasLineOfSight(enemy, player))
				continue;
			
			if (distance < CLOSE_INSPECTION_RADIUS)
			{
				// Close inspection - high threat
				threat = Math.Max(threat, 0.6);
				
				// Check if enemy is looking at player
				if (IsLookingAt(enemy, player))
					threat = Math.Max(threat, 0.9);
			}
			else
			{
				// General proximity - low threat
				float proximityFactor = 1.0 - (distance / DETECTION_RADIUS);
				threat = Math.Max(threat, proximityFactor * 0.3);
			}
		}
		
		return threat;
	}
	
	// ========================================================================
	// UTILITY FUNCTIONS
	// ========================================================================
	
	protected bool IsIllegalWeapon(string prefabName)
	{
		foreach (string illegal : m_aIllegalWeapons)
		{
			if (prefabName.Contains(illegal))
				return true;
		}
		return false;
	}
	
	protected bool IsIllegalClothing(string prefabName)
	{
		foreach (string illegal : m_aIllegalClothing)
		{
			if (prefabName.Contains(illegal))
				return true;
		}
		return false;
	}
	
	protected bool IsIllegalVehicle(string prefabName)
	{
		foreach (string illegal : m_aIllegalVehicles)
		{
			if (prefabName.Contains(illegal))
				return true;
		}
		return false;
	}
	
	protected string GetPrefabName(IEntity entity)
	{
		if (!entity)
			return "";
		
		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return "";
		
		return prefabData.GetPrefabName();
	}
	
	protected bool FilterEnemyUnits(IEntity entity)
	{
		return ChimeraCharacter.Cast(entity) != null;
	}
	
	protected bool IsEnemyFaction(Faction faction)
	{
		string factionKey = faction.GetFactionKey();
		return (factionKey == "USSR" || factionKey == "US");
	}
	
	protected bool HasLineOfSight(IEntity from, IEntity to)
	{
		vector fromPos = from.GetOrigin();
		fromPos[1] += 1.6; // Eye height
		
		vector toPos = to.GetOrigin();
		toPos[1] += 1.0; // Center mass
		
		TraceParam trace = new TraceParam();
		trace.Start = fromPos;
		trace.End = toPos;
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.Exclude = from;
		
		float result = GetGame().GetWorld().TraceMove(trace, null);
		
		// If trace hits target or goes full distance, LOS exists
		return (result >= 0.99 || trace.TraceEnt == to);
	}
	
	protected bool IsLookingAt(ChimeraCharacter observer, ChimeraCharacter target)
	{
		vector observerPos = observer.GetOrigin();
		vector targetPos = target.GetOrigin();
		
		// Get observer's facing direction
		vector observerDir = observer.GetYawPitchRoll();
		float observerYaw = observerDir[0];
		
		// Calculate direction to target
		vector toTarget = targetPos - observerPos;
		toTarget.Normalize();
		
		float angleToTarget = Math.Atan2(toTarget[0], toTarget[2]) * Math.RAD2DEG;
		
		// Check if target is within 60 degree cone
		float angleDiff = Math.AbsFloat(observerYaw - angleToTarget);
		if (angleDiff > 180)
			angleDiff = 360 - angleDiff;
		
		return angleDiff < 30; // 30 degrees each side = 60 degree cone
	}
	
	// ========================================================================
	// COVER BLOWN EVENT
	// ========================================================================
	
	protected void OnCoverBlown(ChimeraCharacter player, RBL_PlayerCoverState state)
	{
		if (state.m_bCoverBlownNotified)
			return;
		
		state.m_bCoverBlownNotified = true;
		
		PrintFormat("[RBL] Player cover blown!");
		
		// Alert nearby enemies
		AlertNearbyEnemies(player, DETECTION_RADIUS * 2);
		
		// Notify player (UI hook)
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(player);
		RpcDo_NotifyCoverBlown(playerID);
	}
	
	protected void AlertNearbyEnemies(ChimeraCharacter player, float radius)
	{
		vector playerPos = player.GetOrigin();
		
		array<IEntity> nearbyEntities = {};
		GetGame().GetWorld().QueryEntitiesBySphere(
			playerPos,
			radius,
			null,
			FilterEnemyUnits,
			EQueryEntitiesFlags.DYNAMIC
		);
		
		foreach (IEntity entity : nearbyEntities)
		{
			ChimeraCharacter enemy = ChimeraCharacter.Cast(entity);
			if (!enemy || !enemy.IsAlive())
				continue;
			
			// Get AI controller and set target
			AIControlComponent aiCtrl = AIControlComponent.Cast(
				enemy.FindComponent(AIControlComponent)
			);
			
			if (aiCtrl)
			{
				// Set player as known target
				AIAgent agent = aiCtrl.GetAIAgent();
				if (agent)
				{
					// Would use agent.AddTargetInfo or similar
					// For now, this is a placeholder for combat engagement
				}
			}
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_NotifyCoverBlown(int playerID)
	{
		// Client-side notification
		// Would trigger UI warning
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
	
	void ResetPlayerCover(int playerID)
	{
		RBL_PlayerCoverState state;
		if (m_mPlayerStates.Find(playerID, state))
		{
			state.m_fSuspicionLevel = 0;
			state.m_eCurrentStatus = ERBLCoverStatus.HIDDEN;
			state.m_bCoverBlownNotified = false;
		}
	}
	
	// Add custom illegal items (for modders)
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
}

// ============================================================================
// PLAYER COVER STATE
// Tracks individual player's undercover status
// ============================================================================
class RBL_PlayerCoverState
{
	ERBLCoverStatus m_eCurrentStatus;
	ERBLCoverStatus m_ePreviousStatus;
	float m_fSuspicionLevel;
	float m_fTimeSinceStatusChange;
	bool m_bCoverBlownNotified;
	
	void RBL_PlayerCoverState()
	{
		m_eCurrentStatus = ERBLCoverStatus.HIDDEN;
		m_ePreviousStatus = ERBLCoverStatus.HIDDEN;
		m_fSuspicionLevel = 0;
		m_fTimeSinceStatusChange = 0;
		m_bCoverBlownNotified = false;
	}
	
	void SetStatus(ERBLCoverStatus newStatus)
	{
		if (m_eCurrentStatus != newStatus)
		{
			m_ePreviousStatus = m_eCurrentStatus;
			m_eCurrentStatus = newStatus;
			m_fTimeSinceStatusChange = 0;
			
			// Reset notification flag when recovering cover
			if (newStatus == ERBLCoverStatus.HIDDEN)
				m_bCoverBlownNotified = false;
		}
	}
}

