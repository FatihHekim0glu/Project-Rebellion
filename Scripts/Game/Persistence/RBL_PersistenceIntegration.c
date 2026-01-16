// ============================================================================
// PROJECT REBELLION - Persistence Integration
// Integrates persistence system with GameMode and other systems
// ============================================================================

class RBL_PersistenceIntegration
{
	protected static ref RBL_PersistenceIntegration s_Instance;
	
	// State
	protected bool m_bInitialized;
	protected bool m_bLoadOnStart;
	protected int m_iAutoLoadSlot;
	
	// Managers
	protected ref RBL_SaveFileManager m_FileManager;
	protected ref RBL_SaveSlotManager m_SlotManager;
	protected ref RBL_SaveRestorer m_Restorer;
	protected ref RBL_AutoSaveManager m_AutoSaveManager;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_PersistenceIntegration GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_PersistenceIntegration();
		return s_Instance;
	}
	
	void RBL_PersistenceIntegration()
	{
		m_bInitialized = false;
		m_bLoadOnStart = false;
		m_iAutoLoadSlot = -1;
	}
	
	// ========================================================================
	// INITIALIZATION
	// ========================================================================
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		PrintFormat("[RBL_Persistence] Initializing persistence system...");
		
		// Get manager instances
		m_FileManager = RBL_SaveFileManager.GetInstance();
		m_SlotManager = RBL_SaveSlotManager.GetInstance();
		m_Restorer = RBL_SaveRestorer.GetInstance();
		m_AutoSaveManager = RBL_AutoSaveManager.GetInstance();
		
		// Register event callbacks
		RegisterEventCallbacks();
		
		// Configure autosave
		ConfigureAutoSave();
		
		m_bInitialized = true;
		PrintFormat("[RBL_Persistence] Persistence system initialized");
		
		// Check for auto-load
		if (m_bLoadOnStart)
		{
			AutoLoad();
		}
	}
	
	// ========================================================================
	// UPDATE
	// ========================================================================
	
	void Update(float timeSlice)
	{
		if (!m_bInitialized)
			return;
		
		// Update autosave timer
		if (m_AutoSaveManager)
			m_AutoSaveManager.Update(timeSlice);
	}
	
	// ========================================================================
	// SAVE OPERATIONS
	// ========================================================================
	
	// Save to current slot
	bool SaveGame()
	{
		return SaveToSlot(m_SlotManager.GetCurrentSlot());
	}
	
	// Save to specific slot
	bool SaveToSlot(int slotIndex)
	{
		PrintFormat("[RBL_Persistence] Saving to slot %1...", slotIndex);
		
		// Collect save data
		RBL_SaveData saveData = CollectFullSaveData();
		if (!saveData)
		{
			PrintFormat("[RBL_Persistence] Failed to collect save data");
			return false;
		}
		
		// Save through slot manager
		bool result = m_SlotManager.SaveToSlot(slotIndex, saveData);
		
		if (result)
		{
			PrintFormat("[RBL_Persistence] Save complete");
			RBL_Notifications.GameSaved();
		}
		else
		{
			PrintFormat("[RBL_Persistence] Save failed");
		}
		
		return result;
	}
	
	// Quick save
	bool QuickSave()
	{
		return m_AutoSaveManager.QuickSave();
	}
	
	// ========================================================================
	// LOAD OPERATIONS
	// ========================================================================
	
	// Load from current slot
	bool LoadGame()
	{
		return LoadFromSlot(m_SlotManager.GetCurrentSlot());
	}
	
	// Load from specific slot
	bool LoadFromSlot(int slotIndex)
	{
		PrintFormat("[RBL_Persistence] Loading from slot %1...", slotIndex);
		return m_Restorer.StartRestoreFromSlot(slotIndex);
	}
	
	// Quick load
	bool QuickLoad()
	{
		return m_AutoSaveManager.QuickLoad();
	}
	
	// Continue game (load most recent)
	bool ContinueGame()
	{
		int lastSlot = m_SlotManager.GetLastUsedSlot();
		
		if (lastSlot < 0)
		{
			// Try autosave
			if (m_FileManager.AutosaveExists())
			{
				PrintFormat("[RBL_Persistence] Continuing from autosave");
				return m_Restorer.StartRestoreFromFile("autosave");
			}
			
			// Try quicksave
			if (m_FileManager.QuicksaveExists())
			{
				PrintFormat("[RBL_Persistence] Continuing from quicksave");
				return QuickLoad();
			}
			
			PrintFormat("[RBL_Persistence] No save found to continue");
			return false;
		}
		
		PrintFormat("[RBL_Persistence] Continuing from slot %1", lastSlot);
		return LoadFromSlot(lastSlot);
	}
	
	// Auto load on startup
	protected void AutoLoad()
	{
		if (m_iAutoLoadSlot >= 0)
		{
			PrintFormat("[RBL_Persistence] Auto-loading slot %1", m_iAutoLoadSlot);
			GetGame().GetCallqueue().CallLater(DelayedAutoLoad, 3000, false);
		}
		else
		{
			// Try to continue
			GetGame().GetCallqueue().CallLater(DelayedContinue, 3000, false);
		}
	}
	
	protected void DelayedAutoLoad()
	{
		LoadFromSlot(m_iAutoLoadSlot);
	}
	
	protected void DelayedContinue()
	{
		if (m_FileManager.AutosaveExists())
		{
			m_Restorer.StartRestoreFromFile("autosave");
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
		
		// Campaign
		saveData.m_Campaign = RBL_CampaignPersistence.GetInstance().CollectCampaignState();
		
		// Zones
		saveData.m_aZones = RBL_ZonePersistence.GetInstance().CollectAllZoneStates();
		
		// Economy
		saveData.m_Economy = RBL_EconomyPersistence.GetInstance().CollectEconomyState();
		
		// Players
		saveData.m_aPlayers = RBL_PlayerPersistence.GetInstance().CollectAllPlayerStates();
		
		// Commander
		saveData.m_Commander = RBL_CampaignPersistence.GetInstance().CollectCommanderState();
		
		// Missions
		saveData.m_aMissions = RBL_CampaignPersistence.GetInstance().CollectActiveMissions();
		
		return saveData;
	}
	
	// ========================================================================
	// EVENT CALLBACKS
	// ========================================================================
	
	protected void RegisterEventCallbacks()
	{
		// Register for restore events
		m_Restorer.GetOnRestoreComplete().Insert(OnRestoreComplete);
		m_Restorer.GetOnRestoreFailed().Insert(OnRestoreFailed);
		
		// Register for autosave events
		m_AutoSaveManager.GetOnAutoSaveComplete().Insert(OnAutoSaveComplete);
		m_AutoSaveManager.GetOnAutoSaveFailed().Insert(OnAutoSaveFailed);
	}
	
	protected void OnRestoreComplete(RBL_RestoreResult result)
	{
		PrintFormat("[RBL_Persistence] Restore complete: %1", result.GetSummary());
		
		// Notify campaign manager
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
			campaignMgr.OnGameLoaded();
	}
	
	protected void OnRestoreFailed(string reason)
	{
		PrintFormat("[RBL_Persistence] Restore failed: %1", reason);
	}
	
	protected void OnAutoSaveComplete(string filename)
	{
		PrintFormat("[RBL_Persistence] AutoSave complete: %1", filename);
	}
	
	protected void OnAutoSaveFailed(string reason)
	{
		PrintFormat("[RBL_Persistence] AutoSave failed: %1", reason);
	}
	
	// ========================================================================
	// CONFIGURATION
	// ========================================================================
	
	protected void ConfigureAutoSave()
	{
		// Default configuration
		m_AutoSaveManager.SetAutoSaveEnabled(true);
		m_AutoSaveManager.SetAutoSaveInterval(300); // 5 minutes
		m_AutoSaveManager.SetSaveOnZoneCapture(true);
		m_AutoSaveManager.SetSaveOnMissionComplete(true);
		m_AutoSaveManager.SetMaxAutoSaves(3);
	}
	
	void SetLoadOnStart(bool load, int slot)
	{
		m_bLoadOnStart = load;
		m_iAutoLoadSlot = slot;
	}
	
	// ========================================================================
	// UTILITIES
	// ========================================================================
	
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
		int playerId = RBL_NetworkUtils.GetLocalPlayerID();
		if (playerId < 0)
			return "";
		return playerId.ToString();
	}
	
	protected string GetWorldName()
	{
		return "";
	}
	
	// ========================================================================
	// ACCESSORS
	// ========================================================================
	
	bool IsInitialized() { return m_bInitialized; }
	RBL_SaveSlotManager GetSlotManager() { return m_SlotManager; }
	RBL_SaveFileManager GetFileManager() { return m_FileManager; }
	RBL_AutoSaveManager GetAutoSaveManager() { return m_AutoSaveManager; }
}

// ============================================================================
// CONSOLE COMMANDS
// ============================================================================
class RBL_SaveCommands
{
	static void Save()
	{
		RBL_PersistenceIntegration.GetInstance().SaveGame();
	}
	
	static void SaveSlot(int slot)
	{
		RBL_PersistenceIntegration.GetInstance().SaveToSlot(slot);
	}
	
	static void Load()
	{
		RBL_PersistenceIntegration.GetInstance().LoadGame();
	}
	
	static void LoadSlot(int slot)
	{
		RBL_PersistenceIntegration.GetInstance().LoadFromSlot(slot);
	}
	
	static void Continue()
	{
		RBL_PersistenceIntegration.GetInstance().ContinueGame();
	}
	
	static void SlotInfo()
	{
		RBL_SaveSlotManager slotMgr = RBL_SaveSlotManager.GetInstance();
		int maxSlots = slotMgr.GetMaxSlots();
		
		PrintFormat("[RBL] Save Slots:");
		for (int i = 0; i < maxSlots; i++)
		{
			PrintFormat("[RBL]   %1: %2", i, slotMgr.GetSlotDisplayText(i));
		}
	}
	
	static void ToggleAutoSave()
	{
		RBL_AutoSaveManager autoSave = RBL_AutoSaveManager.GetInstance();
		autoSave.SetAutoSaveEnabled(!autoSave.IsAutoSaveEnabled());
		string enabledStr = "Disabled";
		if (autoSave.IsAutoSaveEnabled())
			enabledStr = "Enabled";
		PrintFormat("[RBL] AutoSave: %1", enabledStr);
	}
	
	static void SetAutoSaveInterval(int seconds)
	{
		RBL_AutoSaveManager.GetInstance().SetAutoSaveInterval(seconds);
		PrintFormat("[RBL] AutoSave interval: %1 seconds", seconds);
	}
}

// ============================================================================
// GAMEMODE INTEGRATION
// ============================================================================

// Extension for RBL_GameMode to include persistence
class RBL_GameModePersistence
{
	static void InitializePersistence()
	{
		RBL_PersistenceIntegration persist = RBL_PersistenceIntegration.GetInstance();
		persist.Initialize();
	}
	
	static void UpdatePersistence(float timeSlice)
	{
		RBL_PersistenceIntegration persist = RBL_PersistenceIntegration.GetInstance();
		persist.Update(timeSlice);
	}
	
	static void OnGameModeEnd()
	{
		// Trigger final save on game end
		RBL_AutoSaveManager autoSave = RBL_AutoSaveManager.GetInstance();
		autoSave.TriggerAutoSave("Game End");
	}
}

// ============================================================================
// KEYBIND HANDLERS
// ============================================================================
class RBL_PersistenceKeybinds
{
	static void HandleQuickSave()
	{
		RBL_PersistenceIntegration.GetInstance().QuickSave();
	}
	
	static void HandleQuickLoad()
	{
		RBL_PersistenceIntegration.GetInstance().QuickLoad();
	}
}

