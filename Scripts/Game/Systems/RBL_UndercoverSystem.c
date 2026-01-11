// ============================================================================
// PROJECT REBELLION - Undercover Detection System
// ============================================================================

class RBL_UndercoverSystem
{
	protected static ref RBL_UndercoverSystem s_Instance;
	
	protected const float CHECK_INTERVAL = 1.0;
	protected const float DETECTION_RADIUS = 50.0;
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
		
		m_aIllegalWeapons.Insert("AK74");
		m_aIllegalWeapons.Insert("AKM");
		m_aIllegalWeapons.Insert("RPG7");
		m_aIllegalWeapons.Insert("PKM");
		m_aIllegalWeapons.Insert("M16A2");
		m_aIllegalWeapons.Insert("M249");
		
		m_aIllegalClothing.Insert("Uniform_FIA");
		m_aIllegalClothing.Insert("Uniform_USSR");
		m_aIllegalClothing.Insert("Uniform_US");
		m_aIllegalClothing.Insert("Vest_Tactical");
		m_aIllegalClothing.Insert("Helmet_Combat");
		
		m_aIllegalVehicles.Insert("BTR70");
		m_aIllegalVehicles.Insert("BMP1");
		m_aIllegalVehicles.Insert("UAZ_Armed");
		m_aIllegalVehicles.Insert("M113");
	}
	
	void Update(float timeSlice)
	{
		// Simplified update - actual implementation would check all players
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
