// ============================================================================
// PROJECT REBELLION - Mission Manager
// Core system for managing missions
// Server-authoritative mission state with client sync
// ============================================================================

class RBL_MissionManager
{
	protected static ref RBL_MissionManager s_Instance;
	
	// Mission pools
	protected ref array<ref RBL_Mission> m_aAvailableMissions;
	protected ref array<ref RBL_Mission> m_aActiveMissions;
	protected ref array<ref RBL_Mission> m_aCompletedMissions;
	protected ref array<ref RBL_Mission> m_aFailedMissions;
	
	// Configuration
	protected const int MAX_ACTIVE_MISSIONS = 3;
	protected const int MAX_AVAILABLE_MISSIONS = 5;
	protected const float MISSION_REFRESH_INTERVAL = 300.0;
	
	// State
	protected bool m_bInitialized;
	protected float m_fTimeSinceRefresh;
	protected int m_iTotalMissionsCompleted;
	protected int m_iTotalMissionsFailed;
	
	// Events
	protected ref ScriptInvoker m_OnMissionAvailable;
	protected ref ScriptInvoker m_OnMissionStarted;
	protected ref ScriptInvoker m_OnMissionCompleted;
	protected ref ScriptInvoker m_OnMissionFailed;
	protected ref ScriptInvoker m_OnMissionExpired;
	
	static RBL_MissionManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_MissionManager();
		return s_Instance;
	}
	
	void RBL_MissionManager()
	{
		m_aAvailableMissions = new array<ref RBL_Mission>();
		m_aActiveMissions = new array<ref RBL_Mission>();
		m_aCompletedMissions = new array<ref RBL_Mission>();
		m_aFailedMissions = new array<ref RBL_Mission>();
		
		m_OnMissionAvailable = new ScriptInvoker();
		m_OnMissionStarted = new ScriptInvoker();
		m_OnMissionCompleted = new ScriptInvoker();
		m_OnMissionFailed = new ScriptInvoker();
		m_OnMissionExpired = new ScriptInvoker();
		
		m_bInitialized = false;
		m_fTimeSinceRefresh = 0;
		m_iTotalMissionsCompleted = 0;
		m_iTotalMissionsFailed = 0;
	}
	
	// ========================================================================
	// NETWORK HELPERS
	// ========================================================================
	
	protected bool IsServer()
	{
		return RBL_NetworkUtils.IsSinglePlayer() || RBL_NetworkUtils.IsServer();
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		// Only server generates missions
		if (IsServer())
			RefreshAvailableMissions();
		
		m_bInitialized = true;
		PrintFormat("[RBL_MissionMgr] Mission Manager initialized (Server: %1)", IsServer());
	}
	
	// ========================================================================
	// UPDATE LOOP
	// ========================================================================
	
	void Update(float timeSlice)
	{
		if (!m_bInitialized)
			return;
		
		// Mission processing is server-authoritative
		if (!IsServer())
			return;
		
		// Update active missions
		for (int i = m_aActiveMissions.Count() - 1; i >= 0; i--)
		{
			RBL_Mission mission = m_aActiveMissions[i];
			if (!mission)
				continue;
			
			mission.Update(timeSlice);
			
			// Check for completion/failure
			if (mission.IsCompleted())
			{
				OnMissionComplete(mission);
			}
			else if (mission.IsFailed())
			{
				OnMissionFail(mission);
			}
		}
		
		// Refresh available missions periodically
		m_fTimeSinceRefresh += timeSlice;
		if (m_fTimeSinceRefresh >= MISSION_REFRESH_INTERVAL)
		{
			m_fTimeSinceRefresh = 0;
			RefreshAvailableMissions();
		}
	}
	
	// ========================================================================
	// MISSION MANAGEMENT
	// ========================================================================
	
	bool StartMission(string missionID)
	{
		// Only server can start missions
		if (!IsServer())
		{
			PrintFormat("[RBL_MissionMgr] StartMission blocked - not server");
			return false;
		}
		
		// Find mission in available pool
		RBL_Mission mission = GetAvailableMissionByID(missionID);
		if (!mission)
		{
			PrintFormat("[RBL_MissionMgr] Mission not found: %1", missionID);
			return false;
		}
		
		// Check if at max active missions
		if (m_aActiveMissions.Count() >= MAX_ACTIVE_MISSIONS)
		{
			PrintFormat("[RBL_MissionMgr] Cannot start mission - max active missions reached");
			RBL_Notifications.ErrorMessage("Maximum active missions reached");
			return false;
		}
		
		// Remove from available
		m_aAvailableMissions.RemoveItem(mission);
		
		// Start and add to active
		mission.Start();
		m_aActiveMissions.Insert(mission);
		
		// Notify locally
		m_OnMissionStarted.Invoke(mission);
		
		// Broadcast to all clients
		RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
		if (netMgr)
			netMgr.BroadcastMissionStarted(mission.GetID(), mission.GetName());
		else
			RBL_Notifications.MissionReceived(mission.GetName());
		
		PrintFormat("[RBL_MissionMgr] Mission started: %1", mission.GetName());
		return true;
	}
	
	void AbandonMission(string missionID)
	{
		RBL_Mission mission = GetActiveMissionByID(missionID);
		if (!mission)
			return;
		
		mission.Fail();
		OnMissionFail(mission);
	}
	
	void RestoreActiveMission(RBL_Mission mission)
	{
		if (!mission)
			return;
		
		// Only server can restore missions
		if (!IsServer())
			return;
		
		// Check if at max active missions
		if (m_aActiveMissions.Count() >= MAX_ACTIVE_MISSIONS)
		{
			PrintFormat("[RBL_MissionMgr] Cannot restore mission - max active missions reached");
			return;
		}
		
		// Add to active missions (mission is already started)
		m_aActiveMissions.Insert(mission);
		
		// Notify locally
		m_OnMissionStarted.Invoke(mission);
		
		PrintFormat("[RBL_MissionMgr] Mission restored: %1", mission.GetName());
	}
	
	protected void OnMissionComplete(RBL_Mission mission)
	{
		if (!mission)
			return;
		
		if (!IsServer())
			return;
		
		// Remove from active
		m_aActiveMissions.RemoveItem(mission);
		m_aCompletedMissions.Insert(mission);
		
		m_iTotalMissionsCompleted++;
		
		// Apply rewards
		ApplyMissionReward(mission);
		
		// Notify locally
		m_OnMissionCompleted.Invoke(mission);
		
		// Broadcast to all clients
		RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
		if (netMgr)
			netMgr.BroadcastMissionCompleted(mission.GetID(), mission.GetName());
		else
			RBL_Notifications.MissionComplete(mission.GetName());
		
		PrintFormat("[RBL_MissionMgr] Mission completed: %1", mission.GetName());
		
		// Possibly generate new mission
		if (m_aAvailableMissions.Count() < MAX_AVAILABLE_MISSIONS)
			GenerateNewMission();
	}
	
	protected void OnMissionFail(RBL_Mission mission)
	{
		if (!mission)
			return;
		
		if (!IsServer())
			return;
		
		// Remove from active
		m_aActiveMissions.RemoveItem(mission);
		m_aFailedMissions.Insert(mission);
		
		m_iTotalMissionsFailed++;
		
		// Notify locally
		m_OnMissionFailed.Invoke(mission);
		
		// Broadcast to all clients
		RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
		if (netMgr)
			netMgr.BroadcastMissionFailed(mission.GetID(), mission.GetName());
		else
			RBL_Notifications.MissionFailed(mission.GetName());
		
		PrintFormat("[RBL_MissionMgr] Mission failed: %1", mission.GetName());
	}
	
	protected void ApplyMissionReward(RBL_Mission mission)
	{
		RBL_MissionReward reward = mission.GetReward();
		if (!reward)
			return;
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
			return;
		
		if (reward.GetMoney() > 0)
		{
			econMgr.AddMoney(reward.GetMoney());
			RBL_Notifications.MoneyReceived(reward.GetMoney(), "Mission");
		}
		
		if (reward.GetHR() > 0)
		{
			econMgr.AddHR(reward.GetHR());
			RBL_Notifications.HRReceived(reward.GetHR(), "Mission");
		}
		
		// Apply aggression
		if (reward.GetAggression() != 0)
		{
			RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
			if (campMgr)
				campMgr.AddAggression(reward.GetAggression());
		}
		
		PrintFormat("[RBL_MissionMgr] Reward applied: %1", reward.GetRewardString());
	}
	
	// ========================================================================
	// MISSION GENERATION
	// ========================================================================
	
	protected void RefreshAvailableMissions()
	{
		// Remove expired missions
		for (int i = m_aAvailableMissions.Count() - 1; i >= 0; i--)
		{
			RBL_Mission mission = m_aAvailableMissions[i];
			if (mission && mission.GetStatus() == ERBLMissionStatus.EXPIRED)
			{
				m_aAvailableMissions.Remove(i);
				m_OnMissionExpired.Invoke(mission);
			}
		}
		
		// Generate new missions up to max
		while (m_aAvailableMissions.Count() < MAX_AVAILABLE_MISSIONS)
		{
			GenerateNewMission();
		}
		
		PrintFormat("[RBL_MissionMgr] Available missions refreshed: %1 available", m_aAvailableMissions.Count());
	}
	
	protected void GenerateNewMission()
	{
		RBL_Mission mission = RBL_MissionGenerator.GenerateRandomMission();
		if (mission)
		{
			m_aAvailableMissions.Insert(mission);
			m_OnMissionAvailable.Invoke(mission);
		}
	}
	
	// ========================================================================
	// EVENT HANDLERS (called by other systems)
	// ========================================================================
	
	void OnZoneCaptured(string zoneID, ERBLFactionKey byFaction)
	{
		for (int i = 0; i < m_aActiveMissions.Count(); i++)
		{
			RBL_Mission mission = m_aActiveMissions[i];
			if (mission)
				mission.OnZoneCaptured(zoneID, byFaction);
		}
	}
	
	void OnEnemyKilled()
	{
		for (int i = 0; i < m_aActiveMissions.Count(); i++)
		{
			RBL_Mission mission = m_aActiveMissions[i];
			if (mission)
				mission.OnEnemyKilled();
		}
	}
	
	void OnPlayerPositionUpdate(vector position)
	{
		for (int i = 0; i < m_aActiveMissions.Count(); i++)
		{
			RBL_Mission mission = m_aActiveMissions[i];
			if (mission)
				mission.OnPlayerReachedLocation(position);
		}
	}
	
	// ========================================================================
	// GETTERS
	// ========================================================================
	
	RBL_Mission GetAvailableMissionByID(string missionID)
	{
		for (int i = 0; i < m_aAvailableMissions.Count(); i++)
		{
			if (m_aAvailableMissions[i] && m_aAvailableMissions[i].GetID() == missionID)
				return m_aAvailableMissions[i];
		}
		return null;
	}
	
	RBL_Mission GetActiveMissionByID(string missionID)
	{
		for (int i = 0; i < m_aActiveMissions.Count(); i++)
		{
			if (m_aActiveMissions[i] && m_aActiveMissions[i].GetID() == missionID)
				return m_aActiveMissions[i];
		}
		return null;
	}
	
	array<ref RBL_Mission> GetAvailableMissions() { return m_aAvailableMissions; }
	array<ref RBL_Mission> GetActiveMissions() { return m_aActiveMissions; }
	array<ref RBL_Mission> GetCompletedMissions() { return m_aCompletedMissions; }
	array<ref RBL_Mission> GetFailedMissions() { return m_aFailedMissions; }
	
	int GetAvailableMissionCount() { return m_aAvailableMissions.Count(); }
	int GetActiveMissionCount() { return m_aActiveMissions.Count(); }
	int GetTotalMissionsCompleted() { return m_iTotalMissionsCompleted; }
	int GetTotalMissionsFailed() { return m_iTotalMissionsFailed; }
	bool CanStartMoreMissions() { return m_aActiveMissions.Count() < MAX_ACTIVE_MISSIONS; }
	bool IsInitialized() { return m_bInitialized; }
	
	ScriptInvoker GetOnMissionAvailable() { return m_OnMissionAvailable; }
	ScriptInvoker GetOnMissionStarted() { return m_OnMissionStarted; }
	ScriptInvoker GetOnMissionCompleted() { return m_OnMissionCompleted; }
	ScriptInvoker GetOnMissionFailed() { return m_OnMissionFailed; }
	ScriptInvoker GetOnMissionExpired() { return m_OnMissionExpired; }
	
	// ========================================================================
	// DEBUG
	// ========================================================================
	
	void PrintStatus()
	{
		PrintFormat("[RBL_MissionMgr] === MISSION STATUS ===");
		PrintFormat("Available: %1", m_aAvailableMissions.Count());
		PrintFormat("Active: %1", m_aActiveMissions.Count());
		PrintFormat("Completed: %1", m_iTotalMissionsCompleted);
		PrintFormat("Failed: %1", m_iTotalMissionsFailed);
		
		PrintFormat("\n--- Active Missions ---");
		for (int i = 0; i < m_aActiveMissions.Count(); i++)
		{
			RBL_Mission m = m_aActiveMissions[i];
			if (m)
				PrintFormat("  %1: %2 (%3)", m.GetID(), m.GetName(), m.GetStatusString());
		}
		
		PrintFormat("\n--- Available Missions ---");
		for (int i = 0; i < m_aAvailableMissions.Count(); i++)
		{
			RBL_Mission m = m_aAvailableMissions[i];
			if (m)
				PrintFormat("  %1: %2 (%3)", m.GetID(), m.GetName(), m.GetDifficultyString());
		}
	}
}

// ============================================================================
// DEBUG COMMANDS
// ============================================================================
class RBL_MissionCommands
{
	static void PrintStatus()
	{
		RBL_MissionManager mgr = RBL_MissionManager.GetInstance();
		if (mgr)
			mgr.PrintStatus();
	}
	
	static void StartMission(string missionID)
	{
		RBL_MissionManager mgr = RBL_MissionManager.GetInstance();
		if (mgr)
			mgr.StartMission(missionID);
	}
	
	static void ListMissions()
	{
		RBL_MissionManager mgr = RBL_MissionManager.GetInstance();
		if (!mgr)
			return;
		
		PrintFormat("\n=== AVAILABLE MISSIONS ===");
		array<ref RBL_Mission> available = mgr.GetAvailableMissions();
		for (int i = 0; i < available.Count(); i++)
		{
			RBL_Mission m = available[i];
			if (m)
			{
				PrintFormat("%1. [%2] %3", i + 1, m.GetID(), m.GetName());
				PrintFormat("   Type: %1 | Difficulty: %2", m.GetTypeString(), m.GetDifficultyString());
				PrintFormat("   Reward: %1", m.GetReward().GetRewardString());
			}
		}
	}
	
	static void RefreshMissions()
	{
		RBL_MissionManager mgr = RBL_MissionManager.GetInstance();
		if (mgr)
		{
			mgr.Initialize();
			PrintFormat("[RBL] Missions refreshed");
		}
	}
}

