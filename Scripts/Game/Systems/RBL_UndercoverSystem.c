// ============================================================================
// PROJECT REBELLION - Undercover Detection System
// Determines if players are disguised/hidden from enemy detection
// ============================================================================

class RBL_UndercoverSystem
{
	protected static RBL_UndercoverSystem s_Instance;
	
	protected const float CHECK_INTERVAL = 1.0;
	protected const float DETECTION_RADIUS = 50.0;
	protected const float CLOSE_INSPECTION_RADIUS = 10.0;
	protected const float SUSPICION_BUILD_RATE = 0.15;
	protected const float SUSPICION_DECAY_RATE = 0.05;
	protected const float COMPROMISE_THRESHOLD = 1.0;
	
	protected ref array<string> m_aIllegalWeapons;
	protected ref array<string> m_aIllegalClothing;
	protected ref array<string> m_aIllegalVehicles;
	
	protected ref map<int, ref RBL_PlayerCoverState> m_mPlayerStates;
	
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
		m_aIllegalWeapons = new array<string>();
		m_aIllegalClothing = new array<string>();
		m_aIllegalVehicles = new array<string>();
		
		m_aIllegalWeapons.Insert("{WEAP001}AK74");
		m_aIllegalWeapons.Insert("{WEAP002}AKM");
		m_aIllegalWeapons.Insert("{WEAP003}RPG7");
		m_aIllegalWeapons.Insert("{WEAP004}PKM");
		m_aIllegalWeapons.Insert("{WEAP005}SVD");
		m_aIllegalWeapons.Insert("{WEAP006}M16A2");
		m_aIllegalWeapons.Insert("{WEAP007}M249");
		m_aIllegalWeapons.Insert("{WEAP008}M72_LAW");
		m_aIllegalWeapons.Insert("{WEAP009}Grenade_Frag");
		
		m_aIllegalClothing.Insert("{CLOTH001}Uniform_FIA");
		m_aIllegalClothing.Insert("{CLOTH002}Uniform_USSR");
		m_aIllegalClothing.Insert("{CLOTH003}Uniform_US");
		m_aIllegalClothing.Insert("{CLOTH004}Vest_Tactical");
		m_aIllegalClothing.Insert("{CLOTH005}Vest_PlateCarrier");
		m_aIllegalClothing.Insert("{CLOTH006}Helmet_Combat");
		m_aIllegalClothing.Insert("{CLOTH007}Helmet_SSh68");
		m_aIllegalClothing.Insert("{CLOTH008}Backpack_Military");
		
		m_aIllegalVehicles.Insert("{VEH001}BTR70");
		m_aIllegalVehicles.Insert("{VEH002}BMP1");
		m_aIllegalVehicles.Insert("{VEH003}UAZ_Armed");
		m_aIllegalVehicles.Insert("{VEH004}Ural_Military");
		m_aIllegalVehicles.Insert("{VEH005}M113");
		m_aIllegalVehicles.Insert("{VEH006}HMMWV_Armed");
	}
	
	void Update(float timeSlice)
	{
		array<int> playerIDs = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		
		for (int i = 0; i < playerIDs.Count(); i++)
		{
			int playerID = playerIDs[i];
			IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!playerEntity)
				continue;
			
			ChimeraCharacter character = ChimeraCharacter.Cast(playerEntity);
			if (!character || !character.IsAlive())
				continue;
			
			RBL_PlayerCoverState state = GetOrCreatePlayerState(playerID);
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
	
	protected void UpdatePlayerCoverStatus(ChimeraCharacter player, RBL_PlayerCoverState state, float timeSlice)
	{
		ERBLCoverStatus immediateStatus = CheckImmediateCompromise(player);
		if (immediateStatus == ERBLCoverStatus.HOSTILE)
		{
			state.SetStatus(ERBLCoverStatus.HOSTILE);
			state.m_fSuspicionLevel = 1.0;
			return;
		}
		
		float gearThreat = CalculateGearThreat(player);
		float vehicleThreat = CalculateVehicleThreat(player);
		float proximityThreat = CalculateProximityThreat(player);
		
		float totalThreat = gearThreat;
		if (vehicleThreat > totalThreat)
			totalThreat = vehicleThreat;
		if (proximityThreat > totalThreat)
			totalThreat = proximityThreat;
		
		if (totalThreat > 0)
		{
			state.m_fSuspicionLevel += totalThreat * SUSPICION_BUILD_RATE * timeSlice;
		}
		else
		{
			state.m_fSuspicionLevel -= SUSPICION_DECAY_RATE * timeSlice;
		}
		
		state.m_fSuspicionLevel = Math.Clamp(state.m_fSuspicionLevel, 0, 1.0);
		
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
	
	protected ERBLCoverStatus CheckImmediateCompromise(ChimeraCharacter player)
	{
		SCR_CharacterControllerComponent charCtrl = SCR_CharacterControllerComponent.Cast(
			player.FindComponent(SCR_CharacterControllerComponent)
		);
		
		if (charCtrl)
		{
			if (charCtrl.IsWeaponRaised())
				return ERBLCoverStatus.HOSTILE;
		}
		
		return ERBLCoverStatus.HIDDEN;
	}
	
	protected float CalculateGearThreat(ChimeraCharacter player)
	{
		float threat = 0;
		
		BaseWeaponManagerComponent weaponMgr = BaseWeaponManagerComponent.Cast(
			player.FindComponent(BaseWeaponManagerComponent)
		);
		
		if (weaponMgr)
		{
			BaseWeaponComponent currentWeapon = weaponMgr.GetCurrentWeapon();
			if (currentWeapon)
			{
				string weaponPrefab = GetPrefabName(currentWeapon.GetOwner());
				if (IsIllegalWeapon(weaponPrefab))
				{
					threat = 0.8;
				}
			}
		}
		
		return threat;
	}
	
	protected float CalculateVehicleThreat(ChimeraCharacter player)
	{
		CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(
			player.FindComponent(CompartmentAccessComponent)
		);
		
		if (!compAccess || !compAccess.IsInCompartment())
			return 0;
		
		IEntity vehicle = compAccess.GetCompartment().GetOwner();
		if (!vehicle)
			return 0;
		
		string vehiclePrefab = GetPrefabName(vehicle);
		
		if (IsIllegalVehicle(vehiclePrefab))
		{
			return 1.0;
		}
		
		return 0;
	}
	
	protected float CalculateProximityThreat(ChimeraCharacter player)
	{
		float threat = 0;
		// Placeholder - actual implementation would query nearby enemies
		return threat;
	}
	
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
	
	protected string GetPrefabName(IEntity entity)
	{
		if (!entity)
			return "";
		
		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return "";
		
		return prefabData.GetPrefabName();
	}
	
	protected void OnCoverBlown(ChimeraCharacter player, RBL_PlayerCoverState state)
	{
		if (state.m_bCoverBlownNotified)
			return;
		
		state.m_bCoverBlownNotified = true;
		
		PrintFormat("[RBL] Player cover blown!");
	}
	
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
			
			if (newStatus == ERBLCoverStatus.HIDDEN)
				m_bCoverBlownNotified = false;
		}
	}
}
