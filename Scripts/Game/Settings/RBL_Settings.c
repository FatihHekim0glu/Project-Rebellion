// ============================================================================
// PROJECT REBELLION - Settings Data Structure
// Defines all configurable game options
// ============================================================================

// Difficulty presets
enum ERBLDifficulty
{
	EASY,
	NORMAL,
	HARD,
	HARDCORE
}

// Settings categories
enum ERBLSettingsCategory
{
	GAMEPLAY,
	DISPLAY,
	AUDIO,
	CONTROLS
}

// ============================================================================
// DIFFICULTY PRESETS - Configures multiple values at once
// ============================================================================
class RBL_DifficultyPresets
{
	// Starting money by difficulty
	static int GetStartingMoney(ERBLDifficulty difficulty)
	{
		switch (difficulty)
		{
			case ERBLDifficulty.EASY: return 1000;
			case ERBLDifficulty.NORMAL: return 500;
			case ERBLDifficulty.HARD: return 250;
			case ERBLDifficulty.HARDCORE: return 100;
		}
		return 500;
	}
	
	// Starting HR by difficulty
	static int GetStartingHR(ERBLDifficulty difficulty)
	{
		switch (difficulty)
		{
			case ERBLDifficulty.EASY: return 20;
			case ERBLDifficulty.NORMAL: return 10;
			case ERBLDifficulty.HARD: return 5;
			case ERBLDifficulty.HARDCORE: return 2;
		}
		return 10;
	}
	
	// Enemy aggression multiplier
	static float GetAggressionMultiplier(ERBLDifficulty difficulty)
	{
		switch (difficulty)
		{
			case ERBLDifficulty.EASY: return 0.5;
			case ERBLDifficulty.NORMAL: return 1.0;
			case ERBLDifficulty.HARD: return 1.5;
			case ERBLDifficulty.HARDCORE: return 2.0;
		}
		return 1.0;
	}
	
	// QRF response time multiplier (lower = faster response)
	static float GetQRFResponseMultiplier(ERBLDifficulty difficulty)
	{
		switch (difficulty)
		{
			case ERBLDifficulty.EASY: return 1.5;
			case ERBLDifficulty.NORMAL: return 1.0;
			case ERBLDifficulty.HARD: return 0.75;
			case ERBLDifficulty.HARDCORE: return 0.5;
		}
		return 1.0;
	}
	
	// Income multiplier for player
	static float GetIncomeMultiplier(ERBLDifficulty difficulty)
	{
		switch (difficulty)
		{
			case ERBLDifficulty.EASY: return 1.5;
			case ERBLDifficulty.NORMAL: return 1.0;
			case ERBLDifficulty.HARD: return 0.75;
			case ERBLDifficulty.HARDCORE: return 0.5;
		}
		return 1.0;
	}
	
	// Detection speed multiplier (higher = detected faster)
	static float GetDetectionMultiplier(ERBLDifficulty difficulty)
	{
		switch (difficulty)
		{
			case ERBLDifficulty.EASY: return 0.5;
			case ERBLDifficulty.NORMAL: return 1.0;
			case ERBLDifficulty.HARD: return 1.5;
			case ERBLDifficulty.HARDCORE: return 2.0;
		}
		return 1.0;
	}
	
	// Max player deaths before defeat
	static int GetMaxPlayerDeaths(ERBLDifficulty difficulty)
	{
		switch (difficulty)
		{
			case ERBLDifficulty.EASY: return -1; // Unlimited
			case ERBLDifficulty.NORMAL: return 50;
			case ERBLDifficulty.HARD: return 25;
			case ERBLDifficulty.HARDCORE: return 1; // Permadeath
		}
		return 50;
	}
	
	// Get difficulty name
	static string GetDifficultyName(ERBLDifficulty difficulty)
	{
		switch (difficulty)
		{
			case ERBLDifficulty.EASY: return "EASY";
			case ERBLDifficulty.NORMAL: return "NORMAL";
			case ERBLDifficulty.HARD: return "HARD";
			case ERBLDifficulty.HARDCORE: return "HARDCORE";
		}
		return "NORMAL";
	}
	
	// Get difficulty description
	static string GetDifficultyDescription(ERBLDifficulty difficulty)
	{
		switch (difficulty)
		{
			case ERBLDifficulty.EASY: return "Relaxed experience. More resources, slower enemy response.";
			case ERBLDifficulty.NORMAL: return "Balanced challenge. Standard resource rates and AI.";
			case ERBLDifficulty.HARD: return "Tough challenge. Reduced income, aggressive enemies.";
			case ERBLDifficulty.HARDCORE: return "Permadeath. One life, minimal resources, brutal AI.";
		}
		return "";
	}
}

// ============================================================================
// SETTINGS DATA - Holds all current settings values
// ============================================================================
class RBL_SettingsData
{
	// Version for migration
	int m_iVersion = 1;
	
	// ---- GAMEPLAY ----
	ERBLDifficulty m_eDifficulty = ERBLDifficulty.NORMAL;
	bool m_bAutoSaveEnabled = true;
	float m_fAutoSaveInterval = 300.0;           // 5 minutes
	bool m_bSaveOnZoneCapture = true;
	bool m_bSaveOnMissionComplete = true;
	bool m_bUndercoverSystemEnabled = true;
	bool m_bFriendlyFireEnabled = false;
	bool m_bShowTutorialHints = true;
	
	// ---- DISPLAY ----
	bool m_bHUDEnabled = true;
	float m_fHUDOpacity = 1.0;                   // 0.0 - 1.0
	float m_fUIScale = 1.0;                      // 0.75 - 1.5
	bool m_bShowMapMarkers = true;
	bool m_bShowZoneNames = true;
	bool m_bShowMinimap = true;
	bool m_bShowNotifications = true;
	float m_fNotificationDuration = 4.0;         // seconds
	bool m_bShowKeybindHints = true;
	bool m_bShowDamageNumbers = true;
	bool m_bShowCaptureBar = true;
	bool m_bShowZoneInfo = false;
	bool m_bShowUndercoverStatus = false;
	
	// ---- AUDIO ----
	float m_fMasterVolume = 1.0;                 // 0.0 - 1.0
	float m_fMusicVolume = 0.7;                  // 0.0 - 1.0
	float m_fSFXVolume = 1.0;                    // 0.0 - 1.0
	float m_fUIVolume = 0.8;                     // 0.0 - 1.0
	bool m_bMuteWhenUnfocused = true;
	
	// ---- CONTROLS ----
	float m_fMouseSensitivity = 1.0;             // 0.1 - 3.0
	bool m_bInvertY = false;
	bool m_bToggleADS = false;
	bool m_bToggleSprint = false;
	
	// Apply default values
	void ApplyDefaults()
	{
		m_eDifficulty = ERBLDifficulty.NORMAL;
		m_bAutoSaveEnabled = true;
		m_fAutoSaveInterval = 300.0;
		m_bSaveOnZoneCapture = true;
		m_bSaveOnMissionComplete = true;
		m_bUndercoverSystemEnabled = true;
		m_bFriendlyFireEnabled = false;
		m_bShowTutorialHints = true;
		
		m_bHUDEnabled = true;
		m_fHUDOpacity = 1.0;
		m_fUIScale = 1.0;
		m_bShowMapMarkers = true;
		m_bShowZoneNames = true;
		m_bShowMinimap = true;
		m_bShowNotifications = true;
		m_fNotificationDuration = 4.0;
		m_bShowKeybindHints = false;
		m_bShowDamageNumbers = true;
		m_bShowCaptureBar = true;
		m_bShowZoneInfo = false;
		m_bShowUndercoverStatus = false;
		
		m_fMasterVolume = 1.0;
		m_fMusicVolume = 0.7;
		m_fSFXVolume = 1.0;
		m_fUIVolume = 0.8;
		m_bMuteWhenUnfocused = true;
		
		m_fMouseSensitivity = 1.0;
		m_bInvertY = false;
		m_bToggleADS = false;
		m_bToggleSprint = false;
	}
	
	// Copy from another settings instance
	void CopyFrom(RBL_SettingsData other)
	{
		if (!other)
			return;
		
		m_iVersion = other.m_iVersion;
		m_eDifficulty = other.m_eDifficulty;
		m_bAutoSaveEnabled = other.m_bAutoSaveEnabled;
		m_fAutoSaveInterval = other.m_fAutoSaveInterval;
		m_bSaveOnZoneCapture = other.m_bSaveOnZoneCapture;
		m_bSaveOnMissionComplete = other.m_bSaveOnMissionComplete;
		m_bUndercoverSystemEnabled = other.m_bUndercoverSystemEnabled;
		m_bFriendlyFireEnabled = other.m_bFriendlyFireEnabled;
		m_bShowTutorialHints = other.m_bShowTutorialHints;
		
		m_bHUDEnabled = other.m_bHUDEnabled;
		m_fHUDOpacity = other.m_fHUDOpacity;
		m_fUIScale = other.m_fUIScale;
		m_bShowMapMarkers = other.m_bShowMapMarkers;
		m_bShowZoneNames = other.m_bShowZoneNames;
		m_bShowMinimap = other.m_bShowMinimap;
		m_bShowNotifications = other.m_bShowNotifications;
		m_fNotificationDuration = other.m_fNotificationDuration;
		m_bShowKeybindHints = other.m_bShowKeybindHints;
		m_bShowDamageNumbers = other.m_bShowDamageNumbers;
		m_bShowCaptureBar = other.m_bShowCaptureBar;
		m_bShowZoneInfo = other.m_bShowZoneInfo;
		m_bShowUndercoverStatus = other.m_bShowUndercoverStatus;
		
		m_fMasterVolume = other.m_fMasterVolume;
		m_fMusicVolume = other.m_fMusicVolume;
		m_fSFXVolume = other.m_fSFXVolume;
		m_fUIVolume = other.m_fUIVolume;
		m_bMuteWhenUnfocused = other.m_bMuteWhenUnfocused;
		
		m_fMouseSensitivity = other.m_fMouseSensitivity;
		m_bInvertY = other.m_bInvertY;
		m_bToggleADS = other.m_bToggleADS;
		m_bToggleSprint = other.m_bToggleSprint;
	}
	
	// Check equality
	bool Equals(RBL_SettingsData other)
	{
		if (!other)
			return false;
		
		return m_eDifficulty == other.m_eDifficulty &&
			   m_bAutoSaveEnabled == other.m_bAutoSaveEnabled &&
			   m_fAutoSaveInterval == other.m_fAutoSaveInterval &&
			   m_bUndercoverSystemEnabled == other.m_bUndercoverSystemEnabled &&
			   m_bHUDEnabled == other.m_bHUDEnabled &&
			   Math.AbsFloat(m_fHUDOpacity - other.m_fHUDOpacity) < 0.01 &&
			   Math.AbsFloat(m_fUIScale - other.m_fUIScale) < 0.01;
	}
}

// ============================================================================
// SETTINGS ITEM - UI representation of a single setting
// ============================================================================
enum ERBLSettingType
{
	TOGGLE,           // On/Off
	SLIDER,           // Float range
	DROPDOWN,         // List of options
	INTEGER,          // Integer range
	ACTION            // Button (apply, reset, etc.)
}

class RBL_SettingItem
{
	string m_sID;
	string m_sName;
	string m_sDescription;
	ERBLSettingsCategory m_eCategory;
	ERBLSettingType m_eType;
	
	// Value bounds for sliders/integers
	float m_fMinValue;
	float m_fMaxValue;
	float m_fStepValue;
	
	// Options for dropdowns
	ref array<string> m_aOptions;
	
	void RBL_SettingItem(string id, string name, string desc, ERBLSettingsCategory category, ERBLSettingType type)
	{
		m_sID = id;
		m_sName = name;
		m_sDescription = desc;
		m_eCategory = category;
		m_eType = type;
		m_fMinValue = 0;
		m_fMaxValue = 1;
		m_fStepValue = 0.1;
		m_aOptions = new array<string>();
	}
	
	void SetRange(float min, float max, float step)
	{
		m_fMinValue = min;
		m_fMaxValue = max;
		m_fStepValue = step;
	}
	
	void SetOptions(array<string> options)
	{
		m_aOptions.Clear();
		for (int i = 0; i < options.Count(); i++)
		{
			m_aOptions.Insert(options[i]);
		}
	}
}

// ============================================================================
// SETTINGS CONSTANTS
// ============================================================================
class RBL_SettingsConstants
{
	// Settings file
	static const string SETTINGS_FOLDER = "$profile:Rebellion/";
	static const string SETTINGS_FILENAME = "settings.rblcfg";
	static const int SETTINGS_VERSION = 1;
	
	// Value constraints
	static const float MIN_AUTOSAVE_INTERVAL = 60.0;    // 1 minute
	static const float MAX_AUTOSAVE_INTERVAL = 1800.0;  // 30 minutes
	static const float MIN_UI_SCALE = 0.75;
	static const float MAX_UI_SCALE = 1.5;
	static const float MIN_OPACITY = 0.25;
	static const float MAX_OPACITY = 1.0;
	static const float MIN_SENSITIVITY = 0.1;
	static const float MAX_SENSITIVITY = 3.0;
	static const float MIN_NOTIFICATION_DURATION = 1.0;
	static const float MAX_NOTIFICATION_DURATION = 10.0;
}


