// ============================================================================
// PROJECT REBELLION - Victory Manager
// Handles win/loss conditions and campaign end states
// ============================================================================

enum ERBLVictoryCondition
{
	NONE = 0,
	ALL_ZONES_CAPTURED,
	HQ_CAPTURED,
	ENEMY_ELIMINATED,
	TIME_LIMIT_VICTORY
}

enum ERBLDefeatCondition
{
	NONE = 0,
	HQ_LOST,
	ALL_ZONES_LOST,
	PLAYER_DEATHS_EXCEEDED,
	TIME_LIMIT_DEFEAT,
	RESOURCES_DEPLETED
}

enum ERBLCampaignState
{
	ACTIVE = 0,
	VICTORY,
	DEFEAT,
	PAUSED
}

class RBL_VictoryManager
{
	protected static ref RBL_VictoryManager s_Instance;
	
	// Campaign state
	protected ERBLCampaignState m_eCampaignState;
	protected ERBLVictoryCondition m_eVictoryCondition;
	protected ERBLDefeatCondition m_eDefeatCondition;
	
	// Tracking
	protected int m_iPlayerDeaths;
	protected int m_iMaxPlayerDeaths;
	protected float m_fCampaignTimeLimit;
	protected float m_fCampaignTime;
	protected bool m_bInitialized;
	
	// Win condition configuration
	protected bool m_bRequireAllZones;
	protected bool m_bRequireHQCapture;
	protected float m_fVictoryZonePercentage;
	
	// Events
	protected ref ScriptInvoker m_OnVictory;
	protected ref ScriptInvoker m_OnDefeat;
	protected ref ScriptInvoker m_OnCampaignStateChanged;
	
	static RBL_VictoryManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_VictoryManager();
		return s_Instance;
	}
	
	void RBL_VictoryManager()
	{
		m_eCampaignState = ERBLCampaignState.ACTIVE;
		m_eVictoryCondition = ERBLVictoryCondition.NONE;
		m_eDefeatCondition = ERBLDefeatCondition.NONE;
		
		m_iPlayerDeaths = 0;
		m_iMaxPlayerDeaths = 50;
		m_fCampaignTimeLimit = 0;
		m_fCampaignTime = 0;
		m_bInitialized = false;
		
		m_bRequireAllZones = true;
		m_bRequireHQCapture = true;
		m_fVictoryZonePercentage = 0.75;
		
		m_OnVictory = new ScriptInvoker();
		m_OnDefeat = new ScriptInvoker();
		m_OnCampaignStateChanged = new ScriptInvoker();
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		// Subscribe to campaign events
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (campMgr)
		{
			campMgr.GetOnCampaignEvent().Insert(OnCampaignEvent);
		}
		
		m_bInitialized = true;
		PrintFormat("[RBL_VictoryMgr] Victory Manager initialized");
	}
	
	// ========================================================================
	// UPDATE
	// ========================================================================
	
	void Update(float timeSlice)
	{
		if (m_eCampaignState != ERBLCampaignState.ACTIVE)
			return;
		
		m_fCampaignTime += timeSlice;
		
		// Check time limit
		if (m_fCampaignTimeLimit > 0 && m_fCampaignTime >= m_fCampaignTimeLimit)
		{
			CheckTimeLimitCondition();
		}
		
		// Periodic win condition check
		CheckVictoryConditions();
		CheckDefeatConditions();
	}
	
	// ========================================================================
	// VICTORY CONDITIONS
	// ========================================================================
	
	protected void CheckVictoryConditions()
	{
		if (m_eCampaignState != ERBLCampaignState.ACTIVE)
			return;
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		int totalZones = zoneMgr.GetTotalZoneCount();
		int fiaZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
		int enemyZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR);
		
		// Victory: All zones captured
		if (m_bRequireAllZones && fiaZones >= totalZones)
		{
			TriggerVictory(ERBLVictoryCondition.ALL_ZONES_CAPTURED);
			return;
		}
		
		// Victory: Percentage of zones captured
		if (!m_bRequireAllZones && totalZones > 0)
		{
			float captureRatio = fiaZones / totalZones;
			if (captureRatio >= m_fVictoryZonePercentage)
			{
				TriggerVictory(ERBLVictoryCondition.ALL_ZONES_CAPTURED);
				return;
			}
		}
		
		// Victory: Enemy HQ captured
		if (m_bRequireHQCapture)
		{
			RBL_CampaignZone enemyHQ = zoneMgr.GetEnemyHQ();
			if (enemyHQ && enemyHQ.GetOwnerFaction() == ERBLFactionKey.FIA)
			{
				TriggerVictory(ERBLVictoryCondition.HQ_CAPTURED);
				return;
			}
		}
		
		// Victory: All enemies eliminated
		if (enemyZones == 0)
		{
			TriggerVictory(ERBLVictoryCondition.ENEMY_ELIMINATED);
			return;
		}
	}
	
	protected void CheckDefeatConditions()
	{
		if (m_eCampaignState != ERBLCampaignState.ACTIVE)
			return;
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		
		// Defeat: Player HQ lost
		if (zoneMgr)
		{
			RBL_CampaignZone playerHQ = zoneMgr.GetPlayerHQ();
			if (playerHQ && playerHQ.GetOwnerFaction() != ERBLFactionKey.FIA)
			{
				TriggerDefeat(ERBLDefeatCondition.HQ_LOST);
				return;
			}
			
			// Defeat: All zones lost
			int fiaZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
			if (fiaZones == 0)
			{
				TriggerDefeat(ERBLDefeatCondition.ALL_ZONES_LOST);
				return;
			}
		}
		
		// Defeat: Too many player deaths
		if (m_iMaxPlayerDeaths > 0 && m_iPlayerDeaths >= m_iMaxPlayerDeaths)
		{
			TriggerDefeat(ERBLDefeatCondition.PLAYER_DEATHS_EXCEEDED);
			return;
		}
		
		// Defeat: Resources depleted
		if (econMgr)
		{
			int money = econMgr.GetMoney();
			int hr = econMgr.GetHR();
			
			// Only check if we have no zones (can't recover)
			if (zoneMgr)
			{
				int fiaZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
				if (fiaZones == 0 && money <= 0 && hr <= 0)
				{
					TriggerDefeat(ERBLDefeatCondition.RESOURCES_DEPLETED);
					return;
				}
			}
		}
	}
	
	protected void CheckTimeLimitCondition()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		int fiaZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
		int enemyZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR);
		
		// If FIA has more zones, victory; otherwise defeat
		if (fiaZones > enemyZones)
			TriggerVictory(ERBLVictoryCondition.TIME_LIMIT_VICTORY);
		else
			TriggerDefeat(ERBLDefeatCondition.TIME_LIMIT_DEFEAT);
	}
	
	// ========================================================================
	// STATE CHANGES
	// ========================================================================
	
	protected void TriggerVictory(ERBLVictoryCondition condition)
	{
		if (m_eCampaignState != ERBLCampaignState.ACTIVE)
			return;
		
		m_eCampaignState = ERBLCampaignState.VICTORY;
		m_eVictoryCondition = condition;
		
		PrintFormat("[RBL_VictoryMgr] === VICTORY ===");
		PrintFormat("[RBL_VictoryMgr] Condition: %1", GetVictoryConditionString(condition));
		PrintFormat("[RBL_VictoryMgr] Campaign Time: %1 seconds", m_fCampaignTime);
		
		// Notify
		m_OnVictory.Invoke(condition);
		m_OnCampaignStateChanged.Invoke(m_eCampaignState);
		
		// Show notification
		RBL_Notifications.ShowNotification("VICTORY!", RBL_UIColors.COLOR_ACCENT_GREEN, 10.0);
		
		// End campaign in campaign manager
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (campMgr)
			campMgr.EndCampaign(true);
	}
	
	protected void TriggerDefeat(ERBLDefeatCondition condition)
	{
		if (m_eCampaignState != ERBLCampaignState.ACTIVE)
			return;
		
		m_eCampaignState = ERBLCampaignState.DEFEAT;
		m_eDefeatCondition = condition;
		
		PrintFormat("[RBL_VictoryMgr] === DEFEAT ===");
		PrintFormat("[RBL_VictoryMgr] Condition: %1", GetDefeatConditionString(condition));
		PrintFormat("[RBL_VictoryMgr] Campaign Time: %1 seconds", m_fCampaignTime);
		
		// Notify
		m_OnDefeat.Invoke(condition);
		m_OnCampaignStateChanged.Invoke(m_eCampaignState);
		
		// Show notification
		RBL_Notifications.ShowNotification("DEFEAT", RBL_UIColors.COLOR_ACCENT_RED, 10.0);
		
		// End campaign in campaign manager
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (campMgr)
			campMgr.EndCampaign(false);
	}
	
	// ========================================================================
	// EVENT HANDLERS
	// ========================================================================
	
	protected void OnCampaignEvent(ERBLCampaignEvent eventType, RBL_CampaignZone relatedZone)
	{
		switch (eventType)
		{
			case ERBLCampaignEvent.PLAYER_KILLED:
				OnPlayerDeath();
				break;
			case ERBLCampaignEvent.ZONE_CAPTURED:
			case ERBLCampaignEvent.ZONE_LOST:
				// Triggers are checked in Update()
				break;
		}
	}
	
	void OnPlayerDeath()
	{
		m_iPlayerDeaths++;
		PrintFormat("[RBL_VictoryMgr] Player death recorded: %1/%2", m_iPlayerDeaths, m_iMaxPlayerDeaths);
	}
	
	// ========================================================================
	// CONFIGURATION
	// ========================================================================
	
	void SetVictoryZonePercentage(float percentage)
	{
		m_fVictoryZonePercentage = Math.Clamp(percentage, 0.1, 1.0);
	}
	
	void SetMaxPlayerDeaths(int maxDeaths)
	{
		m_iMaxPlayerDeaths = maxDeaths;
	}
	
	void SetCampaignTimeLimit(float seconds)
	{
		m_fCampaignTimeLimit = seconds;
	}
	
	void SetRequireAllZones(bool require)
	{
		m_bRequireAllZones = require;
	}
	
	void SetRequireHQCapture(bool require)
	{
		m_bRequireHQCapture = require;
	}
	
	// ========================================================================
	// GETTERS
	// ========================================================================
	
	ERBLCampaignState GetCampaignState() { return m_eCampaignState; }
	ERBLVictoryCondition GetVictoryCondition() { return m_eVictoryCondition; }
	ERBLDefeatCondition GetDefeatCondition() { return m_eDefeatCondition; }
	int GetPlayerDeaths() { return m_iPlayerDeaths; }
	int GetMaxPlayerDeaths() { return m_iMaxPlayerDeaths; }
	float GetCampaignTime() { return m_fCampaignTime; }
	float GetCampaignTimeLimit() { return m_fCampaignTimeLimit; }
	bool IsVictory() { return m_eCampaignState == ERBLCampaignState.VICTORY; }
	bool IsDefeat() { return m_eCampaignState == ERBLCampaignState.DEFEAT; }
	bool IsActive() { return m_eCampaignState == ERBLCampaignState.ACTIVE; }
	
	ScriptInvoker GetOnVictory() { return m_OnVictory; }
	ScriptInvoker GetOnDefeat() { return m_OnDefeat; }
	ScriptInvoker GetOnCampaignStateChanged() { return m_OnCampaignStateChanged; }
	
	float GetTimeRemaining()
	{
		if (m_fCampaignTimeLimit <= 0)
			return -1;
		return Math.Max(0, m_fCampaignTimeLimit - m_fCampaignTime);
	}
	
	float GetCampaignProgress()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return 0;
		
		int totalZones = zoneMgr.GetTotalZoneCount();
		if (totalZones == 0)
			return 0;
		
		int fiaZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
		return fiaZones / totalZones;
	}
	
	// ========================================================================
	// STRING HELPERS
	// ========================================================================
	
	static string GetVictoryConditionString(ERBLVictoryCondition condition)
	{
		switch (condition)
		{
			case ERBLVictoryCondition.ALL_ZONES_CAPTURED: return "All Zones Captured";
			case ERBLVictoryCondition.HQ_CAPTURED: return "Enemy HQ Captured";
			case ERBLVictoryCondition.ENEMY_ELIMINATED: return "Enemy Eliminated";
			case ERBLVictoryCondition.TIME_LIMIT_VICTORY: return "Time Limit Victory";
		}
		return "Unknown";
	}
	
	static string GetDefeatConditionString(ERBLDefeatCondition condition)
	{
		switch (condition)
		{
			case ERBLDefeatCondition.HQ_LOST: return "HQ Lost";
			case ERBLDefeatCondition.ALL_ZONES_LOST: return "All Zones Lost";
			case ERBLDefeatCondition.PLAYER_DEATHS_EXCEEDED: return "Too Many Casualties";
			case ERBLDefeatCondition.TIME_LIMIT_DEFEAT: return "Time Limit Defeat";
			case ERBLDefeatCondition.RESOURCES_DEPLETED: return "Resources Depleted";
		}
		return "Unknown";
	}
	
	string GetCampaignStateString()
	{
		switch (m_eCampaignState)
		{
			case ERBLCampaignState.ACTIVE: return "Active";
			case ERBLCampaignState.VICTORY: return "Victory";
			case ERBLCampaignState.DEFEAT: return "Defeat";
			case ERBLCampaignState.PAUSED: return "Paused";
		}
		return "Unknown";
	}
	
	// ========================================================================
	// DEBUG
	// ========================================================================
	
	void PrintStatus()
	{
		PrintFormat("[RBL_VictoryMgr] === VICTORY MANAGER STATUS ===");
		PrintFormat("Campaign State: %1", GetCampaignStateString());
		PrintFormat("Campaign Time: %1 seconds", m_fCampaignTime);
		PrintFormat("Player Deaths: %1/%2", m_iPlayerDeaths, m_iMaxPlayerDeaths);
		PrintFormat("Progress: %1%%", Math.Round(GetCampaignProgress() * 100));
		
		if (m_fCampaignTimeLimit > 0)
			PrintFormat("Time Remaining: %1 seconds", GetTimeRemaining());
		
		if (m_eCampaignState == ERBLCampaignState.VICTORY)
			PrintFormat("Victory Condition: %1", GetVictoryConditionString(m_eVictoryCondition));
		else if (m_eCampaignState == ERBLCampaignState.DEFEAT)
			PrintFormat("Defeat Condition: %1", GetDefeatConditionString(m_eDefeatCondition));
	}
}

// ============================================================================
// DEBUG COMMANDS
// ============================================================================
class RBL_VictoryCommands
{
	static void PrintStatus()
	{
		RBL_VictoryManager mgr = RBL_VictoryManager.GetInstance();
		if (mgr)
			mgr.PrintStatus();
	}
	
	static void TriggerVictory()
	{
		RBL_VictoryManager mgr = RBL_VictoryManager.GetInstance();
		if (mgr)
		{
			// Force victory for testing
			PrintFormat("[RBL] Triggering test victory...");
			RBL_Notifications.ShowNotification("VICTORY! (DEBUG)", RBL_UIColors.COLOR_ACCENT_GREEN, 5.0);
		}
	}
	
	static void TriggerDefeat()
	{
		RBL_VictoryManager mgr = RBL_VictoryManager.GetInstance();
		if (mgr)
		{
			// Force defeat for testing
			PrintFormat("[RBL] Triggering test defeat...");
			RBL_Notifications.ShowNotification("DEFEAT (DEBUG)", RBL_UIColors.COLOR_ACCENT_RED, 5.0);
		}
	}
}

