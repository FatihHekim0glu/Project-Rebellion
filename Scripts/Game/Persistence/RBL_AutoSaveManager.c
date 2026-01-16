// ============================================================================
// PROJECT REBELLION - AutoSave Manager
// Handles automatic saving at intervals and key events
// ============================================================================

class RBL_AutoSaveManager
{
	protected static ref RBL_AutoSaveManager s_Instance;
	
	// Configuration
	protected bool m_bAutoSaveEnabled;
	protected float m_fAutoSaveInterval;
	protected bool m_bSaveOnZoneCapture;
	protected bool m_bSaveOnMissionComplete;
	protected bool m_bSaveOnPlayerJoin;
	protected int m_iMaxAutoSaves;
	
	// State
	protected float m_fTimeSinceLastSave;
	protected bool m_bIsSaving;
	protected int m_iAutoSaveCount;
	protected float m_fLastSaveTime;
	
	// Callbacks
	protected ref ScriptInvoker m_OnAutoSaveStart;
	protected ref ScriptInvoker m_OnAutoSaveComplete;
	protected ref ScriptInvoker m_OnAutoSaveFailed;
	
	// Defaults
	protected const float DEFAULT_INTERVAL = 300.0; // 5 minutes
	protected const int DEFAULT_MAX_AUTOSAVES = 3;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_AutoSaveManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_AutoSaveManager();
		return s_Instance;
	}
	
	void RBL_AutoSaveManager()
	{
		m_bAutoSaveEnabled = true;
		m_fAutoSaveInterval = DEFAULT_INTERVAL;
		m_bSaveOnZoneCapture = true;
		m_bSaveOnMissionComplete = true;
		m_bSaveOnPlayerJoin = false;
		m_iMaxAutoSaves = DEFAULT_MAX_AUTOSAVES;
		
		m_fTimeSinceLastSave = 0;
		m_bIsSaving = false;
		m_iAutoSaveCount = 0;
		m_fLastSaveTime = 0;
		
		m_OnAutoSaveStart = new ScriptInvoker();
		m_OnAutoSaveComplete = new ScriptInvoker();
		m_OnAutoSaveFailed = new ScriptInvoker();
		
		// Register event listeners
		RegisterEventListeners();
	}
	
	// ========================================================================
	// UPDATE
	// ========================================================================
	
	void Update(float timeSlice)
	{
		if (!m_bAutoSaveEnabled || m_bIsSaving)
			return;
		
		m_fTimeSinceLastSave += timeSlice;
		
		// Check if interval elapsed
		if (m_fTimeSinceLastSave >= m_fAutoSaveInterval)
		{
			TriggerAutoSave("Interval");
		}
	}
	
	// ========================================================================
	// AUTOSAVE TRIGGERS
	// ========================================================================
	
	// Manual trigger
	void TriggerAutoSave(string reason)
	{
		if (m_bIsSaving)
		{
			PrintFormat("[RBL_AutoSave] Already saving, skipping trigger: %1", reason);
			return;
		}
		
		PrintFormat("[RBL_AutoSave] Triggering autosave: %1", reason);
		
		m_bIsSaving = true;
		m_OnAutoSaveStart.Invoke(reason);
		
		// Collect save data
		RBL_SaveData saveData = CollectFullSaveData();
		if (!saveData)
		{
			FailAutoSave("Failed to collect save data");
			return;
		}
		
		// Get autosave filename
		string filename = GetAutoSaveFilename();
		
		// Serialize
		string json = RBL_SaveSerializer.SerializeToJson(saveData);
		if (json.IsEmpty())
		{
			FailAutoSave("Serialization failed");
			return;
		}
		
		// Write to file
		RBL_SaveFileManager fileMgr = RBL_SaveFileManager.GetInstance();
		bool success = fileMgr.WriteToFile(filename, json);
		
		if (success)
		{
			CompleteAutoSave(filename);
		}
		else
		{
			FailAutoSave("File write failed");
		}
	}
	
	// Event-triggered saves
	void OnZoneCaptured(string zoneID)
	{
		if (!m_bSaveOnZoneCapture)
			return;
		
		// Delay slightly to ensure zone state is updated
		GetGame().GetCallqueue().CallLater(DelayedTrigger, 1000, false, "Zone captured: " + zoneID);
	}
	
	void OnMissionCompleted(string missionID)
	{
		if (!m_bSaveOnMissionComplete)
			return;
		
		GetGame().GetCallqueue().CallLater(DelayedTrigger, 1000, false, "Mission complete: " + missionID);
	}
	
	void OnPlayerJoined(int playerId)
	{
		if (!m_bSaveOnPlayerJoin)
			return;
		
		GetGame().GetCallqueue().CallLater(DelayedTrigger, 2000, false, "Player joined");
	}
	
	protected void DelayedTrigger(string reason)
	{
		TriggerAutoSave(reason);
	}
	
	// ========================================================================
	// SAVE COMPLETION
	// ========================================================================
	
	protected void CompleteAutoSave(string filename)
	{
		m_bIsSaving = false;
		m_fTimeSinceLastSave = 0;
		m_iAutoSaveCount++;
		m_fLastSaveTime = GetGame().GetWorld().GetWorldTime();
		
		PrintFormat("[RBL_AutoSave] AutoSave complete: %1", filename);
		
		m_OnAutoSaveComplete.Invoke(filename);
		
		// Show notification
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ShowNotification("Game Auto-Saved", RBL_UIColors.COLOR_TEXT_SECONDARY, 2.0);
		}
		
		// Rotate old autosaves
		RotateAutoSaves();
	}
	
	protected void FailAutoSave(string reason)
	{
		m_bIsSaving = false;
		
		PrintFormat("[RBL_AutoSave] AutoSave FAILED: %1", reason);
		
		m_OnAutoSaveFailed.Invoke(reason);
		
		// Show notification
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ShowNotification("AutoSave Failed", RBL_UIColors.COLOR_ACCENT_RED, 3.0);
		}
	}
	
	// ========================================================================
	// DATA COLLECTION
	// ========================================================================
	
	protected RBL_SaveData CollectFullSaveData()
	{
		RBL_SaveData saveData = new RBL_SaveData();
		
		// Header
		saveData.m_sMagic = RBL_SAVE_MAGIC;
		saveData.m_iVersion = RBL_SAVE_VERSION;
		saveData.m_sSaveTime = GetCurrentTimestamp();
		saveData.m_sPlayerId = GetHostPlayerUID();
		saveData.m_sWorldName = GetWorldName();
		
		// Campaign state
		saveData.m_Campaign = RBL_CampaignPersistence.GetInstance().CollectCampaignState();
		
		// Zones
		saveData.m_aZones = RBL_ZonePersistence.GetInstance().CollectAllZoneStates();
		
		// Economy
		saveData.m_Economy = RBL_EconomyPersistence.GetInstance().CollectEconomyState();
		
		// Players
		saveData.m_aPlayers = RBL_PlayerPersistence.GetInstance().CollectAllPlayerStates();
		
		// Commander AI
		saveData.m_Commander = RBL_CampaignPersistence.GetInstance().CollectCommanderState();
		
		// Missions
		saveData.m_aMissions = RBL_CampaignPersistence.GetInstance().CollectActiveMissions();
		
		return saveData;
	}
	
	// ========================================================================
	// AUTOSAVE ROTATION
	// ========================================================================
	
	protected void RotateAutoSaves()
	{
		// Keep only m_iMaxAutoSaves autosave files
		// Delete oldest when limit exceeded
		
		array<string> autoSaves = GetAutoSaveFiles();
		if (autoSaves.Count() <= m_iMaxAutoSaves)
			return;
		
		// Sort by modification time (oldest first)
		// For simplicity, autosaves are numbered, so sort by number
		SortAutoSaves(autoSaves);
		
		// Delete excess
		RBL_SaveFileManager fileMgr = RBL_SaveFileManager.GetInstance();
		while (autoSaves.Count() > m_iMaxAutoSaves)
		{
			string oldestSave = autoSaves[0];
			fileMgr.DeleteFile(oldestSave);
			autoSaves.Remove(0);
			PrintFormat("[RBL_AutoSave] Deleted old autosave: %1", oldestSave);
		}
	}
	
	protected array<string> GetAutoSaveFiles()
	{
		array<string> autoSaves = new array<string>();
		
		array<string> allSaves = RBL_SaveFileManager.GetInstance().GetSaveFileList();
		
		for (int i = 0; i < allSaves.Count(); i++)
		{
			if (allSaves[i].Contains("autosave"))
			{
				autoSaves.Insert(allSaves[i]);
			}
		}
		
		return autoSaves;
	}
	
	protected void SortAutoSaves(array<string> saves)
	{
		// Simple sort by name (autosave_1, autosave_2, etc)
		for (int i = 0; i < saves.Count() - 1; i++)
		{
			for (int j = 0; j < saves.Count() - i - 1; j++)
			{
				if (saves[j] > saves[j + 1])
				{
					string temp = saves[j];
					saves[j] = saves[j + 1];
					saves[j + 1] = temp;
				}
			}
		}
	}
	
	// ========================================================================
	// QUICKSAVE / QUICKLOAD
	// ========================================================================
	
	// Quicksave (manual, separate from autosave)
	bool QuickSave()
	{
		if (m_bIsSaving)
		{
			PrintFormat("[RBL_AutoSave] Cannot quicksave while saving");
			return false;
		}
		
		PrintFormat("[RBL_AutoSave] Quicksave triggered");
		
		RBL_SaveData saveData = CollectFullSaveData();
		if (!saveData)
			return false;
		
		string json = RBL_SaveSerializer.SerializeToJson(saveData);
		if (json.IsEmpty())
			return false;
		
		bool success = RBL_SaveFileManager.GetInstance().WriteQuicksave(json);
		
		if (success)
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
				uiMgr.ShowNotification("Game Quicksaved", RBL_UIColors.COLOR_ACCENT_GREEN, 2.0);
		}
		
		return success;
	}
	
	// Quickload
	bool QuickLoad()
	{
		if (!RBL_SaveFileManager.GetInstance().QuicksaveExists())
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
				uiMgr.ShowNotification("No Quicksave Found", RBL_UIColors.COLOR_ACCENT_RED, 2.0);
			return false;
		}
		
		string json = RBL_SaveFileManager.GetInstance().ReadQuicksave();
		if (json.IsEmpty())
			return false;
		
		RBL_SaveData saveData = RBL_SaveSerializer.DeserializeFromJson(json);
		if (!saveData)
			return false;
		
		return RBL_SaveRestorer.GetInstance().StartRestore(saveData);
	}
	
	// ========================================================================
	// EVENT LISTENERS
	// ========================================================================
	
	protected void RegisterEventListeners()
	{
		// Register for game events
		// These would hook into the actual game event system
		
		// Zone capture events
		// RBL_ZoneManager.GetOnZoneCaptured().Insert(OnZoneCaptured);
		
		// Mission events
		// RBL_MissionManager.GetOnMissionComplete().Insert(OnMissionCompleted);
		
		// Player events
		// GetGame().GetOnPlayerConnected().Insert(OnPlayerJoined);
	}
	
	// ========================================================================
	// UTILITIES
	// ========================================================================
	
	protected string GetAutoSaveFilename()
	{
		// Rotating autosave names
		int slot = 0;
		if (m_iMaxAutoSaves > 0)
			slot = m_iAutoSaveCount - (m_iAutoSaveCount / m_iMaxAutoSaves) * m_iMaxAutoSaves;
		return "autosave_" + slot.ToString();
	}
	
	protected string GetCurrentTimestamp()
	{
		int year, month, day, hour, minute, second;
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, second);
		
		return string.Format("%1-%2-%3 %4:%5:%6",
			year, PadZero(month), PadZero(day),
			PadZero(hour), PadZero(minute), PadZero(second));
	}
	
	protected string PadZero(int value)
	{
		if (value < 10)
			return "0" + value.ToString();
		return value.ToString();
	}
	
	protected string GetHostPlayerUID()
	{
		int hostId = RBL_NetworkUtils.GetLocalPlayerID();
		if (hostId < 0)
			return "";
		return hostId.ToString();
	}
	
	protected string GetWorldName()
	{
		return "";
	}
	
	// ========================================================================
	// CONFIGURATION
	// ========================================================================
	
	void SetAutoSaveEnabled(bool enabled) { m_bAutoSaveEnabled = enabled; }
	bool IsAutoSaveEnabled() { return m_bAutoSaveEnabled; }
	
	void SetAutoSaveInterval(float seconds) { m_fAutoSaveInterval = Math.Max(60, seconds); }
	float GetAutoSaveInterval() { return m_fAutoSaveInterval; }
	
	void SetSaveOnZoneCapture(bool save) { m_bSaveOnZoneCapture = save; }
	void SetSaveOnMissionComplete(bool save) { m_bSaveOnMissionComplete = save; }
	void SetSaveOnPlayerJoin(bool save) { m_bSaveOnPlayerJoin = save; }
	
	void SetMaxAutoSaves(int max) { m_iMaxAutoSaves = Math.Max(1, max); }
	int GetMaxAutoSaves() { return m_iMaxAutoSaves; }
	
	// State getters
	bool IsSaving() { return m_bIsSaving; }
	float GetTimeSinceLastSave() { return m_fTimeSinceLastSave; }
	int GetAutoSaveCount() { return m_iAutoSaveCount; }
	
	// Event accessors
	ScriptInvoker GetOnAutoSaveStart() { return m_OnAutoSaveStart; }
	ScriptInvoker GetOnAutoSaveComplete() { return m_OnAutoSaveComplete; }
	ScriptInvoker GetOnAutoSaveFailed() { return m_OnAutoSaveFailed; }
}

