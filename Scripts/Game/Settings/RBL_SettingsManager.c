// ============================================================================
// PROJECT REBELLION - Settings Manager
// Singleton manager for game settings
// ============================================================================

class RBL_SettingsManager
{
	protected static ref RBL_SettingsManager s_Instance;
	
	// Current settings
	protected ref RBL_SettingsData m_CurrentSettings;
	protected ref RBL_SettingsData m_PendingSettings;
	protected ref RBL_SettingsData m_DefaultSettings;
	
	// Settings catalog (for UI)
	protected ref array<ref RBL_SettingItem> m_aSettingItems;
	
	// State
	protected bool m_bInitialized;
	protected bool m_bHasUnsavedChanges;
	protected bool m_bSettingsLoaded;
	
	// Events
	protected ref ScriptInvoker m_OnSettingsChanged;
	protected ref ScriptInvoker m_OnSettingsSaved;
	protected ref ScriptInvoker m_OnSettingsReset;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_SettingsManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_SettingsManager();
		return s_Instance;
	}
	
	void RBL_SettingsManager()
	{
		m_CurrentSettings = new RBL_SettingsData();
		m_PendingSettings = new RBL_SettingsData();
		m_DefaultSettings = new RBL_SettingsData();
		m_aSettingItems = new array<ref RBL_SettingItem>();
		
		m_bInitialized = false;
		m_bHasUnsavedChanges = false;
		m_bSettingsLoaded = false;
		
		m_OnSettingsChanged = new ScriptInvoker();
		m_OnSettingsSaved = new ScriptInvoker();
		m_OnSettingsReset = new ScriptInvoker();
		
		// Set defaults
		m_DefaultSettings.ApplyDefaults();
		m_CurrentSettings.ApplyDefaults();
		m_PendingSettings.ApplyDefaults();
		
		PrintFormat("[RBL_Settings] Settings Manager created");
	}
	
	// ========================================================================
	// INITIALIZATION
	// ========================================================================
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		PrintFormat("[RBL_Settings] Initializing Settings Manager...");
		
		// Build settings catalog
		BuildSettingsCatalog();
		
		// Try to load saved settings
		if (RBL_SettingsPersistence.GetInstance().LoadSettings(m_CurrentSettings))
		{
			m_bSettingsLoaded = true;
			m_PendingSettings.CopyFrom(m_CurrentSettings);
			PrintFormat("[RBL_Settings] Settings loaded from file");
		}
		else
		{
			PrintFormat("[RBL_Settings] No saved settings, using defaults");
		}
		
		// Apply loaded settings to game systems
		ApplyAllSettings();
		
		m_bInitialized = true;
		PrintFormat("[RBL_Settings] Settings Manager initialized");
	}
	
	// ========================================================================
	// SETTINGS CATALOG - Define all settings for UI
	// ========================================================================
	
	protected void BuildSettingsCatalog()
	{
		m_aSettingItems.Clear();
		
		// ---- GAMEPLAY SETTINGS ----
		
		// Difficulty
		RBL_SettingItem difficulty = new RBL_SettingItem(
			"difficulty", "Difficulty", 
			"Game difficulty preset affecting resources and AI behavior",
			ERBLSettingsCategory.GAMEPLAY, ERBLSettingType.DROPDOWN
		);
		array<string> diffOptions = {"EASY", "NORMAL", "HARD", "HARDCORE"};
		difficulty.SetOptions(diffOptions);
		m_aSettingItems.Insert(difficulty);
		
		// Autosave enabled
		m_aSettingItems.Insert(new RBL_SettingItem(
			"autosave_enabled", "Auto-Save Enabled",
			"Automatically save game progress at intervals",
			ERBLSettingsCategory.GAMEPLAY, ERBLSettingType.TOGGLE
		));
		
		// Autosave interval
		RBL_SettingItem autosaveInterval = new RBL_SettingItem(
			"autosave_interval", "Auto-Save Interval",
			"Time between automatic saves (minutes)",
			ERBLSettingsCategory.GAMEPLAY, ERBLSettingType.SLIDER
		);
		autosaveInterval.SetRange(1, 30, 1);
		m_aSettingItems.Insert(autosaveInterval);
		
		// Save on zone capture
		m_aSettingItems.Insert(new RBL_SettingItem(
			"save_on_capture", "Save on Zone Capture",
			"Automatically save when capturing a zone",
			ERBLSettingsCategory.GAMEPLAY, ERBLSettingType.TOGGLE
		));
		
		// Undercover system
		m_aSettingItems.Insert(new RBL_SettingItem(
			"undercover_enabled", "Undercover System",
			"Enable the undercover/detection mechanics",
			ERBLSettingsCategory.GAMEPLAY, ERBLSettingType.TOGGLE
		));
		
		// Friendly fire
		m_aSettingItems.Insert(new RBL_SettingItem(
			"friendly_fire", "Friendly Fire",
			"Allow damage to friendly units",
			ERBLSettingsCategory.GAMEPLAY, ERBLSettingType.TOGGLE
		));
		
		// Tutorial hints
		m_aSettingItems.Insert(new RBL_SettingItem(
			"tutorial_hints", "Tutorial Hints",
			"Show helpful gameplay hints",
			ERBLSettingsCategory.GAMEPLAY, ERBLSettingType.TOGGLE
		));
		
		// ---- DISPLAY SETTINGS ----
		
		// HUD enabled
		m_aSettingItems.Insert(new RBL_SettingItem(
			"hud_enabled", "Show HUD",
			"Display the heads-up display",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.TOGGLE
		));
		
		// HUD opacity
		RBL_SettingItem hudOpacity = new RBL_SettingItem(
			"hud_opacity", "HUD Opacity",
			"Transparency of HUD elements",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.SLIDER
		);
		hudOpacity.SetRange(0.25, 1.0, 0.05);
		m_aSettingItems.Insert(hudOpacity);
		
		// UI scale
		RBL_SettingItem uiScale = new RBL_SettingItem(
			"ui_scale", "UI Scale",
			"Size of user interface elements",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.SLIDER
		);
		uiScale.SetRange(0.75, 1.5, 0.05);
		m_aSettingItems.Insert(uiScale);
		
		// Map markers
		m_aSettingItems.Insert(new RBL_SettingItem(
			"map_markers", "Show Map Markers",
			"Display zone markers on the map",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.TOGGLE
		));
		
		// Zone names
		m_aSettingItems.Insert(new RBL_SettingItem(
			"zone_names", "Show Zone Names",
			"Display zone names in the HUD",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.TOGGLE
		));
		
		// Zone info panel
		m_aSettingItems.Insert(new RBL_SettingItem(
			"zone_info", "Show Zone Info",
			"Display detailed zone information panel",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.TOGGLE
		));
		
		// Notifications
		m_aSettingItems.Insert(new RBL_SettingItem(
			"notifications", "Show Notifications",
			"Display popup notifications",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.TOGGLE
		));
		
		// Notification duration
		RBL_SettingItem notifDuration = new RBL_SettingItem(
			"notification_duration", "Notification Duration",
			"How long notifications stay on screen (seconds)",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.SLIDER
		);
		notifDuration.SetRange(1, 10, 0.5);
		m_aSettingItems.Insert(notifDuration);
		
		// Keybind hints
		m_aSettingItems.Insert(new RBL_SettingItem(
			"keybind_hints", "Show Keybind Hints",
			"Display keybind reminders",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.TOGGLE
		));
		
		// Undercover status
		m_aSettingItems.Insert(new RBL_SettingItem(
			"undercover_status", "Show Undercover Status",
			"Display undercover/suspicion indicator",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.TOGGLE
		));
		
		// Capture bar
		m_aSettingItems.Insert(new RBL_SettingItem(
			"capture_bar", "Show Capture Bar",
			"Display zone capture progress",
			ERBLSettingsCategory.DISPLAY, ERBLSettingType.TOGGLE
		));
		
		// ---- AUDIO SETTINGS ----
		
		// Master volume
		RBL_SettingItem masterVol = new RBL_SettingItem(
			"master_volume", "Master Volume",
			"Overall audio volume",
			ERBLSettingsCategory.AUDIO, ERBLSettingType.SLIDER
		);
		masterVol.SetRange(0, 1, 0.05);
		m_aSettingItems.Insert(masterVol);
		
		// Music volume
		RBL_SettingItem musicVol = new RBL_SettingItem(
			"music_volume", "Music Volume",
			"Background music volume",
			ERBLSettingsCategory.AUDIO, ERBLSettingType.SLIDER
		);
		musicVol.SetRange(0, 1, 0.05);
		m_aSettingItems.Insert(musicVol);
		
		// SFX volume
		RBL_SettingItem sfxVol = new RBL_SettingItem(
			"sfx_volume", "Effects Volume",
			"Sound effects volume",
			ERBLSettingsCategory.AUDIO, ERBLSettingType.SLIDER
		);
		sfxVol.SetRange(0, 1, 0.05);
		m_aSettingItems.Insert(sfxVol);
		
		// UI volume
		RBL_SettingItem uiVol = new RBL_SettingItem(
			"ui_volume", "UI Volume",
			"Menu and notification sounds",
			ERBLSettingsCategory.AUDIO, ERBLSettingType.SLIDER
		);
		uiVol.SetRange(0, 1, 0.05);
		m_aSettingItems.Insert(uiVol);
		
		// Mute when unfocused
		m_aSettingItems.Insert(new RBL_SettingItem(
			"mute_unfocused", "Mute When Unfocused",
			"Mute audio when game is not active",
			ERBLSettingsCategory.AUDIO, ERBLSettingType.TOGGLE
		));
		
		// ---- CONTROLS SETTINGS ----
		
		// Mouse sensitivity
		RBL_SettingItem sensitivity = new RBL_SettingItem(
			"mouse_sensitivity", "Mouse Sensitivity",
			"Camera turn speed with mouse",
			ERBLSettingsCategory.CONTROLS, ERBLSettingType.SLIDER
		);
		sensitivity.SetRange(0.1, 3.0, 0.1);
		m_aSettingItems.Insert(sensitivity);
		
		// Invert Y
		m_aSettingItems.Insert(new RBL_SettingItem(
			"invert_y", "Invert Y-Axis",
			"Invert vertical camera controls",
			ERBLSettingsCategory.CONTROLS, ERBLSettingType.TOGGLE
		));
		
		// Toggle ADS
		m_aSettingItems.Insert(new RBL_SettingItem(
			"toggle_ads", "Toggle ADS",
			"Toggle aim-down-sights instead of hold",
			ERBLSettingsCategory.CONTROLS, ERBLSettingType.TOGGLE
		));
		
		// Toggle Sprint
		m_aSettingItems.Insert(new RBL_SettingItem(
			"toggle_sprint", "Toggle Sprint",
			"Toggle sprint instead of hold",
			ERBLSettingsCategory.CONTROLS, ERBLSettingType.TOGGLE
		));
		
		PrintFormat("[RBL_Settings] Built catalog with %1 settings", m_aSettingItems.Count());
	}
	
	// ========================================================================
	// SETTING VALUES - Getters and Setters
	// ========================================================================
	
	// Get current settings (read-only)
	RBL_SettingsData GetCurrentSettings()
	{
		return m_CurrentSettings;
	}
	
	// Get pending settings (for modification in UI)
	RBL_SettingsData GetPendingSettings()
	{
		return m_PendingSettings;
	}
	
	// Get specific setting value (for UI binding)
	float GetSettingFloat(string settingID)
	{
		switch (settingID)
		{
			case "autosave_interval": return m_PendingSettings.m_fAutoSaveInterval / 60.0; // Convert to minutes
			case "hud_opacity": return m_PendingSettings.m_fHUDOpacity;
			case "ui_scale": return m_PendingSettings.m_fUIScale;
			case "notification_duration": return m_PendingSettings.m_fNotificationDuration;
			case "master_volume": return m_PendingSettings.m_fMasterVolume;
			case "music_volume": return m_PendingSettings.m_fMusicVolume;
			case "sfx_volume": return m_PendingSettings.m_fSFXVolume;
			case "ui_volume": return m_PendingSettings.m_fUIVolume;
			case "mouse_sensitivity": return m_PendingSettings.m_fMouseSensitivity;
		}
		return 0;
	}
	
	bool GetSettingBool(string settingID)
	{
		switch (settingID)
		{
			case "autosave_enabled": return m_PendingSettings.m_bAutoSaveEnabled;
			case "save_on_capture": return m_PendingSettings.m_bSaveOnZoneCapture;
			case "undercover_enabled": return m_PendingSettings.m_bUndercoverSystemEnabled;
			case "friendly_fire": return m_PendingSettings.m_bFriendlyFireEnabled;
			case "tutorial_hints": return m_PendingSettings.m_bShowTutorialHints;
			case "hud_enabled": return m_PendingSettings.m_bHUDEnabled;
			case "map_markers": return m_PendingSettings.m_bShowMapMarkers;
			case "zone_names": return m_PendingSettings.m_bShowZoneNames;
			case "zone_info": return m_PendingSettings.m_bShowZoneInfo;
			case "notifications": return m_PendingSettings.m_bShowNotifications;
			case "keybind_hints": return m_PendingSettings.m_bShowKeybindHints;
			case "capture_bar": return m_PendingSettings.m_bShowCaptureBar;
			case "undercover_status": return m_PendingSettings.m_bShowUndercoverStatus;
			case "mute_unfocused": return m_PendingSettings.m_bMuteWhenUnfocused;
			case "invert_y": return m_PendingSettings.m_bInvertY;
			case "toggle_ads": return m_PendingSettings.m_bToggleADS;
			case "toggle_sprint": return m_PendingSettings.m_bToggleSprint;
		}
		return false;
	}
	
	int GetSettingInt(string settingID)
	{
		switch (settingID)
		{
			case "difficulty": return m_PendingSettings.m_eDifficulty;
		}
		return 0;
	}
	
	// Set setting value
	void SetSettingFloat(string settingID, float value)
	{
		switch (settingID)
		{
			case "autosave_interval": 
				m_PendingSettings.m_fAutoSaveInterval = Math.Clamp(value * 60.0, 
					RBL_SettingsConstants.MIN_AUTOSAVE_INTERVAL, 
					RBL_SettingsConstants.MAX_AUTOSAVE_INTERVAL);
				break;
			case "hud_opacity": 
				m_PendingSettings.m_fHUDOpacity = Math.Clamp(value, 
					RBL_SettingsConstants.MIN_OPACITY, 
					RBL_SettingsConstants.MAX_OPACITY);
				break;
			case "ui_scale": 
				m_PendingSettings.m_fUIScale = Math.Clamp(value, 
					RBL_SettingsConstants.MIN_UI_SCALE, 
					RBL_SettingsConstants.MAX_UI_SCALE);
				break;
			case "notification_duration": 
				m_PendingSettings.m_fNotificationDuration = Math.Clamp(value, 
					RBL_SettingsConstants.MIN_NOTIFICATION_DURATION, 
					RBL_SettingsConstants.MAX_NOTIFICATION_DURATION);
				break;
			case "master_volume": 
				m_PendingSettings.m_fMasterVolume = Math.Clamp(value, 0, 1);
				break;
			case "music_volume": 
				m_PendingSettings.m_fMusicVolume = Math.Clamp(value, 0, 1);
				break;
			case "sfx_volume": 
				m_PendingSettings.m_fSFXVolume = Math.Clamp(value, 0, 1);
				break;
			case "ui_volume": 
				m_PendingSettings.m_fUIVolume = Math.Clamp(value, 0, 1);
				break;
			case "mouse_sensitivity": 
				m_PendingSettings.m_fMouseSensitivity = Math.Clamp(value, 
					RBL_SettingsConstants.MIN_SENSITIVITY, 
					RBL_SettingsConstants.MAX_SENSITIVITY);
				break;
		}
		m_bHasUnsavedChanges = true;
	}
	
	void SetSettingBool(string settingID, bool value)
	{
		switch (settingID)
		{
			case "autosave_enabled": m_PendingSettings.m_bAutoSaveEnabled = value; break;
			case "save_on_capture": m_PendingSettings.m_bSaveOnZoneCapture = value; break;
			case "undercover_enabled": m_PendingSettings.m_bUndercoverSystemEnabled = value; break;
			case "friendly_fire": m_PendingSettings.m_bFriendlyFireEnabled = value; break;
			case "tutorial_hints": m_PendingSettings.m_bShowTutorialHints = value; break;
			case "hud_enabled": m_PendingSettings.m_bHUDEnabled = value; break;
			case "map_markers": m_PendingSettings.m_bShowMapMarkers = value; break;
			case "zone_names": m_PendingSettings.m_bShowZoneNames = value; break;
			case "zone_info": m_PendingSettings.m_bShowZoneInfo = value; break;
			case "notifications": m_PendingSettings.m_bShowNotifications = value; break;
			case "keybind_hints": m_PendingSettings.m_bShowKeybindHints = value; break;
			case "capture_bar": m_PendingSettings.m_bShowCaptureBar = value; break;
			case "undercover_status": m_PendingSettings.m_bShowUndercoverStatus = value; break;
			case "mute_unfocused": m_PendingSettings.m_bMuteWhenUnfocused = value; break;
			case "invert_y": m_PendingSettings.m_bInvertY = value; break;
			case "toggle_ads": m_PendingSettings.m_bToggleADS = value; break;
			case "toggle_sprint": m_PendingSettings.m_bToggleSprint = value; break;
		}
		m_bHasUnsavedChanges = true;
	}
	
	void SetSettingInt(string settingID, int value)
	{
		switch (settingID)
		{
			case "difficulty": 
				m_PendingSettings.m_eDifficulty = Math.Clamp(value, 0, 3);
				break;
		}
		m_bHasUnsavedChanges = true;
	}
	
	void SetDifficulty(ERBLDifficulty difficulty)
	{
		m_PendingSettings.m_eDifficulty = difficulty;
		m_bHasUnsavedChanges = true;
	}
	
	// ========================================================================
	// APPLY / SAVE / RESET
	// ========================================================================
	
	// Apply pending settings (save to current and propagate to systems)
	void ApplySettings()
	{
		PrintFormat("[RBL_Settings] Applying pending settings...");
		
		// Copy pending to current
		m_CurrentSettings.CopyFrom(m_PendingSettings);
		
		// Apply to all game systems
		ApplyAllSettings();
		
		// Save to file
		RBL_SettingsPersistence.GetInstance().SaveSettings(m_CurrentSettings);
		
		m_bHasUnsavedChanges = false;
		
		m_OnSettingsChanged.Invoke();
		m_OnSettingsSaved.Invoke();
		
		PrintFormat("[RBL_Settings] Settings applied and saved");
	}
	
	// Discard pending changes
	void DiscardChanges()
	{
		m_PendingSettings.CopyFrom(m_CurrentSettings);
		m_bHasUnsavedChanges = false;
		PrintFormat("[RBL_Settings] Pending changes discarded");
	}
	
	// Reset to defaults
	void ResetToDefaults()
	{
		PrintFormat("[RBL_Settings] Resetting to defaults...");
		
		m_PendingSettings.CopyFrom(m_DefaultSettings);
		m_bHasUnsavedChanges = true;
		
		m_OnSettingsReset.Invoke();
	}
	
	// Reset and apply defaults
	void ResetAndApplyDefaults()
	{
		ResetToDefaults();
		ApplySettings();
	}
	
	// ========================================================================
	// APPLY TO GAME SYSTEMS
	// ========================================================================
	
	protected void ApplyAllSettings()
	{
		ApplyGameplaySettings();
		ApplyDisplaySettings();
		ApplyAudioSettings();
		ApplyControlSettings();
	}
	
	void ApplyDisplaySettingsToUI()
	{
		ApplyDisplaySettings();
	}
	
	protected void ApplyGameplaySettings()
	{
		// Apply autosave settings
		RBL_AutoSaveManager autoSave = RBL_AutoSaveManager.GetInstance();
		if (autoSave)
		{
			autoSave.SetAutoSaveEnabled(m_CurrentSettings.m_bAutoSaveEnabled);
			autoSave.SetAutoSaveInterval(m_CurrentSettings.m_fAutoSaveInterval);
			autoSave.SetSaveOnZoneCapture(m_CurrentSettings.m_bSaveOnZoneCapture);
			autoSave.SetSaveOnMissionComplete(m_CurrentSettings.m_bSaveOnMissionComplete);
		}
		
		// Apply difficulty to Victory Manager
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (victoryMgr)
		{
			int maxDeaths = RBL_DifficultyPresets.GetMaxPlayerDeaths(m_CurrentSettings.m_eDifficulty);
			victoryMgr.SetMaxPlayerDeaths(maxDeaths);
		}
		
		// Apply undercover system toggle
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
		{
			undercover.SetEnabled(m_CurrentSettings.m_bUndercoverSystemEnabled);
		}
		
		PrintFormat("[RBL_Settings] Applied gameplay settings");
	}
	
	protected void ApplyDisplaySettings()
	{
		// Apply HUD settings
		RBL_HUDManager hudMgr = RBL_HUDManager.GetInstance();
		if (hudMgr)
		{
			if (m_CurrentSettings.m_bHUDEnabled)
				hudMgr.Show();
			else
				hudMgr.Hide();
		}
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.SetVisible(m_CurrentSettings.m_bHUDEnabled);
			
			RBL_ZoneInfoWidgetImpl zoneInfo = uiMgr.GetZoneInfo();
			if (zoneInfo)
			{
				if (m_CurrentSettings.m_bShowZoneInfo)
					zoneInfo.Show();
				else
					zoneInfo.Hide();
			}
			
			RBL_UndercoverWidgetImpl undercover = uiMgr.GetUndercoverIndicator();
			if (undercover)
			{
				if (m_CurrentSettings.m_bShowUndercoverStatus)
					undercover.Show();
				else
					undercover.Hide();
			}
			
			RBL_CaptureBarWidgetImpl captureBar = uiMgr.GetCaptureBar();
			if (captureBar)
			{
				if (m_CurrentSettings.m_bShowCaptureBar)
					captureBar.Show();
				else
					captureBar.Hide();
			}
			
			RBL_NotificationManagerImpl notifications = uiMgr.GetNotifications();
			if (notifications)
			{
				if (m_CurrentSettings.m_bShowNotifications)
					notifications.Show();
				else
					notifications.Hide();
			}
			
			RBL_KeybindHintsWidgetImpl keybindHints = uiMgr.GetKeybindHints();
			if (keybindHints)
			{
				if (m_CurrentSettings.m_bShowKeybindHints)
					keybindHints.Show();
				else
					keybindHints.Hide();
			}
		}
		
		PrintFormat("[RBL_Settings] Applied display settings");
	}
	
	protected void ApplyAudioSettings()
	{
		// Audio settings would be applied to audio system when implemented
		PrintFormat("[RBL_Settings] Applied audio settings (placeholder)");
	}
	
	protected void ApplyControlSettings()
	{
		// Control settings would be applied to input system when implemented
		PrintFormat("[RBL_Settings] Applied control settings (placeholder)");
	}
	
	// ========================================================================
	// QUERIES
	// ========================================================================
	
	bool HasUnsavedChanges() { return m_bHasUnsavedChanges; }
	bool IsInitialized() { return m_bInitialized; }
	bool AreSettingsLoaded() { return m_bSettingsLoaded; }
	
	array<ref RBL_SettingItem> GetSettingsByCategory(ERBLSettingsCategory category)
	{
		array<ref RBL_SettingItem> result = new array<ref RBL_SettingItem>();
		
		for (int i = 0; i < m_aSettingItems.Count(); i++)
		{
			if (m_aSettingItems[i].m_eCategory == category)
				result.Insert(m_aSettingItems[i]);
		}
		
		return result;
	}
	
	RBL_SettingItem GetSettingByID(string settingID)
	{
		for (int i = 0; i < m_aSettingItems.Count(); i++)
		{
			if (m_aSettingItems[i].m_sID == settingID)
				return m_aSettingItems[i];
		}
		return null;
	}
	
	int GetTotalSettingCount() { return m_aSettingItems.Count(); }
	
	// Get category count
	int GetSettingCountByCategory(ERBLSettingsCategory category)
	{
		int count = 0;
		for (int i = 0; i < m_aSettingItems.Count(); i++)
		{
			if (m_aSettingItems[i].m_eCategory == category)
				count++;
		}
		return count;
	}
	
	// Get current difficulty
	ERBLDifficulty GetDifficulty()
	{
		return m_CurrentSettings.m_eDifficulty;
	}
	
	// Get difficulty multipliers for other systems
	float GetAggressionMultiplier()
	{
		return RBL_DifficultyPresets.GetAggressionMultiplier(m_CurrentSettings.m_eDifficulty);
	}
	
	float GetIncomeMultiplier()
	{
		return RBL_DifficultyPresets.GetIncomeMultiplier(m_CurrentSettings.m_eDifficulty);
	}
	
	float GetDetectionMultiplier()
	{
		return RBL_DifficultyPresets.GetDetectionMultiplier(m_CurrentSettings.m_eDifficulty);
	}
	
	float GetQRFResponseMultiplier()
	{
		return RBL_DifficultyPresets.GetQRFResponseMultiplier(m_CurrentSettings.m_eDifficulty);
	}
	
	// ========================================================================
	// EVENT ACCESSORS
	// ========================================================================
	
	ScriptInvoker GetOnSettingsChanged() { return m_OnSettingsChanged; }
	ScriptInvoker GetOnSettingsSaved() { return m_OnSettingsSaved; }
	ScriptInvoker GetOnSettingsReset() { return m_OnSettingsReset; }
}


