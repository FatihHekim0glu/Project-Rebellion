// ============================================================================
// PROJECT REBELLION - Settings Persistence
// Save/Load settings to file
// ============================================================================

class RBL_SettingsPersistence
{
	protected static ref RBL_SettingsPersistence s_Instance;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_SettingsPersistence GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_SettingsPersistence();
		return s_Instance;
	}
	
	void RBL_SettingsPersistence()
	{
		EnsureSettingsDirectory();
	}
	
	// ========================================================================
	// SAVE SETTINGS
	// ========================================================================
	
	bool SaveSettings(RBL_SettingsData settings)
	{
		if (!settings)
		{
			PrintFormat("[RBL_SettingsPersistence] Cannot save: null settings");
			return false;
		}
		
		// Serialize to JSON
		string json = SerializeSettings(settings);
		if (json.IsEmpty())
		{
			PrintFormat("[RBL_SettingsPersistence] Serialization failed");
			return false;
		}
		
		// Write to file
		string filepath = GetSettingsFilePath();
		
		FileHandle file = FileIO.OpenFile(filepath, FileMode.WRITE);
		if (!file)
		{
			PrintFormat("[RBL_SettingsPersistence] Failed to open file for writing: %1", filepath);
			return false;
		}
		
		file.Write(json);
		file.Close();
		
		PrintFormat("[RBL_SettingsPersistence] Settings saved: %1 bytes", json.Length());
		return true;
	}
	
	// ========================================================================
	// LOAD SETTINGS
	// ========================================================================
	
	bool LoadSettings(RBL_SettingsData outSettings)
	{
		if (!outSettings)
		{
			PrintFormat("[RBL_SettingsPersistence] Cannot load: null output");
			return false;
		}
		
		string filepath = GetSettingsFilePath();
		
		if (!FileIO.FileExists(filepath))
		{
			PrintFormat("[RBL_SettingsPersistence] No settings file found");
			return false;
		}
		
		// Read file content
		FileHandle file = FileIO.OpenFile(filepath, FileMode.READ);
		if (!file)
		{
			PrintFormat("[RBL_SettingsPersistence] Failed to open file for reading");
			return false;
		}
		
		string content = "";
		string line;
		while (file.ReadLine(line) >= 0)
		{
			content += line;
		}
		file.Close();
		
		if (content.IsEmpty())
		{
			PrintFormat("[RBL_SettingsPersistence] Settings file is empty");
			return false;
		}
		
		// Deserialize
		if (!DeserializeSettings(content, outSettings))
		{
			PrintFormat("[RBL_SettingsPersistence] Deserialization failed");
			return false;
		}
		
		PrintFormat("[RBL_SettingsPersistence] Settings loaded successfully");
		return true;
	}
	
	// ========================================================================
	// DELETE SETTINGS
	// ========================================================================
	
	bool DeleteSettings()
	{
		string filepath = GetSettingsFilePath();
		
		if (!FileIO.FileExists(filepath))
			return true;
		
		bool result = FileIO.DeleteFile(filepath);
		
		if (result)
			PrintFormat("[RBL_SettingsPersistence] Settings file deleted");
		else
			PrintFormat("[RBL_SettingsPersistence] Failed to delete settings file");
		
		return result;
	}
	
	// Check if settings file exists
	bool SettingsFileExists()
	{
		return FileIO.FileExists(GetSettingsFilePath());
	}
	
	// ========================================================================
	// SERIALIZATION
	// ========================================================================
	
	protected string SerializeSettings(RBL_SettingsData settings)
	{
		// Manual JSON construction for Enfusion compatibility - minified
		string json = "{";
		json += "\"version\":" + settings.m_iVersion.ToString() + ",";
		json += "\"gameplay\":{";
		json += "\"difficulty\":" + ((int)settings.m_eDifficulty).ToString() + ",";
		json += "\"autoSaveEnabled\":" + BoolToString(settings.m_bAutoSaveEnabled) + ",";
		json += "\"autoSaveInterval\":" + settings.m_fAutoSaveInterval.ToString() + ",";
		json += "\"saveOnZoneCapture\":" + BoolToString(settings.m_bSaveOnZoneCapture) + ",";
		json += "\"saveOnMissionComplete\":" + BoolToString(settings.m_bSaveOnMissionComplete) + ",";
		json += "\"undercoverSystemEnabled\":" + BoolToString(settings.m_bUndercoverSystemEnabled) + ",";
		json += "\"friendlyFireEnabled\":" + BoolToString(settings.m_bFriendlyFireEnabled) + ",";
		json += "\"showTutorialHints\":" + BoolToString(settings.m_bShowTutorialHints);
		json += "},";
		json += "\"display\":{";
		json += "\"hudEnabled\":" + BoolToString(settings.m_bHUDEnabled) + ",";
		json += "\"hudOpacity\":" + settings.m_fHUDOpacity.ToString() + ",";
		json += "\"uiScale\":" + settings.m_fUIScale.ToString() + ",";
		json += "\"showMapMarkers\":" + BoolToString(settings.m_bShowMapMarkers) + ",";
		json += "\"showZoneNames\":" + BoolToString(settings.m_bShowZoneNames) + ",";
		json += "\"showZoneInfo\":" + BoolToString(settings.m_bShowZoneInfo) + ",";
		json += "\"showUndercoverStatus\":" + BoolToString(settings.m_bShowUndercoverStatus) + ",";
		json += "\"showMinimap\":" + BoolToString(settings.m_bShowMinimap) + ",";
		json += "\"showNotifications\":" + BoolToString(settings.m_bShowNotifications) + ",";
		json += "\"notificationDuration\":" + settings.m_fNotificationDuration.ToString() + ",";
		json += "\"showKeybindHints\":" + BoolToString(settings.m_bShowKeybindHints) + ",";
		json += "\"showDamageNumbers\":" + BoolToString(settings.m_bShowDamageNumbers) + ",";
		json += "\"showCaptureBar\":" + BoolToString(settings.m_bShowCaptureBar);
		json += "},";
		json += "\"audio\":{";
		json += "\"masterVolume\":" + settings.m_fMasterVolume.ToString() + ",";
		json += "\"musicVolume\":" + settings.m_fMusicVolume.ToString() + ",";
		json += "\"sfxVolume\":" + settings.m_fSFXVolume.ToString() + ",";
		json += "\"uiVolume\":" + settings.m_fUIVolume.ToString() + ",";
		json += "\"muteWhenUnfocused\":" + BoolToString(settings.m_bMuteWhenUnfocused);
		json += "},";
		json += "\"controls\":{";
		json += "\"mouseSensitivity\":" + settings.m_fMouseSensitivity.ToString() + ",";
		json += "\"invertY\":" + BoolToString(settings.m_bInvertY) + ",";
		json += "\"toggleADS\":" + BoolToString(settings.m_bToggleADS) + ",";
		json += "\"toggleSprint\":" + BoolToString(settings.m_bToggleSprint);
		json += "}";
		json += "}";
		return json;
	}
	
	protected bool DeserializeSettings(string json, RBL_SettingsData outSettings)
	{
		// Start with defaults
		outSettings.ApplyDefaults();
		
		// Parse version
		outSettings.m_iVersion = ParseIntValue(json, "version", 1);
		
		// Migrate if needed
		if (outSettings.m_iVersion < RBL_SettingsConstants.SETTINGS_VERSION)
		{
			MigrateSettings(outSettings, outSettings.m_iVersion);
		}
		
		// Parse gameplay
		outSettings.m_eDifficulty = ParseIntValue(json, "difficulty", 1);
		outSettings.m_bAutoSaveEnabled = ParseBoolValue(json, "autoSaveEnabled", true);
		outSettings.m_fAutoSaveInterval = ParseFloatValue(json, "autoSaveInterval", 300.0);
		outSettings.m_bSaveOnZoneCapture = ParseBoolValue(json, "saveOnZoneCapture", true);
		outSettings.m_bSaveOnMissionComplete = ParseBoolValue(json, "saveOnMissionComplete", true);
		outSettings.m_bUndercoverSystemEnabled = ParseBoolValue(json, "undercoverSystemEnabled", true);
		outSettings.m_bFriendlyFireEnabled = ParseBoolValue(json, "friendlyFireEnabled", false);
		outSettings.m_bShowTutorialHints = ParseBoolValue(json, "showTutorialHints", true);
		
		// Parse display
		outSettings.m_bHUDEnabled = ParseBoolValue(json, "hudEnabled", true);
		outSettings.m_fHUDOpacity = ParseFloatValue(json, "hudOpacity", 1.0);
		outSettings.m_fUIScale = ParseFloatValue(json, "uiScale", 1.0);
		outSettings.m_bShowMapMarkers = ParseBoolValue(json, "showMapMarkers", true);
		outSettings.m_bShowZoneNames = ParseBoolValue(json, "showZoneNames", true);
		outSettings.m_bShowZoneInfo = ParseBoolValue(json, "showZoneInfo", false);
		outSettings.m_bShowUndercoverStatus = ParseBoolValue(json, "showUndercoverStatus", false);
		outSettings.m_bShowMinimap = ParseBoolValue(json, "showMinimap", true);
		outSettings.m_bShowNotifications = ParseBoolValue(json, "showNotifications", true);
		outSettings.m_fNotificationDuration = ParseFloatValue(json, "notificationDuration", 4.0);
		outSettings.m_bShowKeybindHints = ParseBoolValue(json, "showKeybindHints", false);
		outSettings.m_bShowDamageNumbers = ParseBoolValue(json, "showDamageNumbers", true);
		outSettings.m_bShowCaptureBar = ParseBoolValue(json, "showCaptureBar", true);
		
		// Parse audio
		outSettings.m_fMasterVolume = ParseFloatValue(json, "masterVolume", 1.0);
		outSettings.m_fMusicVolume = ParseFloatValue(json, "musicVolume", 0.7);
		outSettings.m_fSFXVolume = ParseFloatValue(json, "sfxVolume", 1.0);
		outSettings.m_fUIVolume = ParseFloatValue(json, "uiVolume", 0.8);
		outSettings.m_bMuteWhenUnfocused = ParseBoolValue(json, "muteWhenUnfocused", true);
		
		// Parse controls
		outSettings.m_fMouseSensitivity = ParseFloatValue(json, "mouseSensitivity", 1.0);
		outSettings.m_bInvertY = ParseBoolValue(json, "invertY", false);
		outSettings.m_bToggleADS = ParseBoolValue(json, "toggleADS", false);
		outSettings.m_bToggleSprint = ParseBoolValue(json, "toggleSprint", false);
		
		// Validate and clamp values
		ValidateSettings(outSettings);
		
		return true;
	}
	
	// ========================================================================
	// VERSION MIGRATION
	// ========================================================================
	
	protected void MigrateSettings(RBL_SettingsData settings, int fromVersion)
	{
		PrintFormat("[RBL_SettingsPersistence] Migrating settings from v%1 to v%2", 
			fromVersion, RBL_SettingsConstants.SETTINGS_VERSION);
		
		// Add migration logic here as versions increase
		// Example:
		// if (fromVersion < 2)
		// {
		//     // Settings added in v2 get defaults
		//     settings.m_bNewSetting = true;
		// }
		
		settings.m_iVersion = RBL_SettingsConstants.SETTINGS_VERSION;
	}
	
	// ========================================================================
	// VALIDATION
	// ========================================================================
	
	protected void ValidateSettings(RBL_SettingsData settings)
	{
		// Clamp all values to valid ranges
		settings.m_eDifficulty = Math.Clamp(settings.m_eDifficulty, 0, 3);
		
		settings.m_fAutoSaveInterval = Math.Clamp(settings.m_fAutoSaveInterval, 
			RBL_SettingsConstants.MIN_AUTOSAVE_INTERVAL, 
			RBL_SettingsConstants.MAX_AUTOSAVE_INTERVAL);
		
		settings.m_fHUDOpacity = Math.Clamp(settings.m_fHUDOpacity, 
			RBL_SettingsConstants.MIN_OPACITY, 
			RBL_SettingsConstants.MAX_OPACITY);
		
		settings.m_fUIScale = Math.Clamp(settings.m_fUIScale, 
			RBL_SettingsConstants.MIN_UI_SCALE, 
			RBL_SettingsConstants.MAX_UI_SCALE);
		
		settings.m_fNotificationDuration = Math.Clamp(settings.m_fNotificationDuration, 
			RBL_SettingsConstants.MIN_NOTIFICATION_DURATION, 
			RBL_SettingsConstants.MAX_NOTIFICATION_DURATION);
		
		settings.m_fMasterVolume = Math.Clamp(settings.m_fMasterVolume, 0, 1);
		settings.m_fMusicVolume = Math.Clamp(settings.m_fMusicVolume, 0, 1);
		settings.m_fSFXVolume = Math.Clamp(settings.m_fSFXVolume, 0, 1);
		settings.m_fUIVolume = Math.Clamp(settings.m_fUIVolume, 0, 1);
		
		settings.m_fMouseSensitivity = Math.Clamp(settings.m_fMouseSensitivity, 
			RBL_SettingsConstants.MIN_SENSITIVITY, 
			RBL_SettingsConstants.MAX_SENSITIVITY);
	}
	
	// ========================================================================
	// PARSING HELPERS
	// ========================================================================
	
	protected string BoolToString(bool value)
	{
		if (value)
			return "true";
		return "false";
	}
	
	protected int ParseIntValue(string json, string key, int defaultValue)
	{
		string searchKey = "\"" + key + "\":";
		int keyPos = json.IndexOf(searchKey);
		if (keyPos < 0)
			return defaultValue;
		
		int valueStart = keyPos + searchKey.Length();
		
		// Skip whitespace
		while (valueStart < json.Length() && json.Get(valueStart) == " ")
			valueStart++;
		
		// Find end of value
		int valueEnd = valueStart;
		while (valueEnd < json.Length() && IsDigitChar(json.Get(valueEnd)))
			valueEnd++;
		
		if (valueEnd == valueStart)
			return defaultValue;
		
		string valueStr = json.Substring(valueStart, valueEnd - valueStart);
		return valueStr.ToInt();
	}
	
	protected float ParseFloatValue(string json, string key, float defaultValue)
	{
		string searchKey = "\"" + key + "\":";
		int keyPos = json.IndexOf(searchKey);
		if (keyPos < 0)
			return defaultValue;
		
		int valueStart = keyPos + searchKey.Length();
		
		// Skip whitespace
		while (valueStart < json.Length() && json.Get(valueStart) == " ")
			valueStart++;
		
		// Find end of value (including decimals)
		int valueEnd = valueStart;
		while (valueEnd < json.Length() && IsFloatChar(json.Get(valueEnd)))
			valueEnd++;
		
		if (valueEnd == valueStart)
			return defaultValue;
		
		string valueStr = json.Substring(valueStart, valueEnd - valueStart);
		return valueStr.ToFloat();
	}
	
	protected bool ParseBoolValue(string json, string key, bool defaultValue)
	{
		string searchKey = "\"" + key + "\":";
		int keyPos = json.IndexOf(searchKey);
		if (keyPos < 0)
			return defaultValue;
		
		int valueStart = keyPos + searchKey.Length();
		
		// Skip whitespace
		while (valueStart < json.Length() && json.Get(valueStart) == " ")
			valueStart++;
		
		// Check for true/false
		if (json.Length() >= valueStart + 4)
		{
			string sub = json.Substring(valueStart, 4);
			if (sub == "true")
				return true;
		}
		
		if (json.Length() >= valueStart + 5)
		{
			string sub = json.Substring(valueStart, 5);
			if (sub == "false")
				return false;
		}
		
		return defaultValue;
	}
	
	protected bool IsDigitChar(string c)
	{
		return c == "0" || c == "1" || c == "2" || c == "3" || c == "4" || 
			   c == "5" || c == "6" || c == "7" || c == "8" || c == "9";
	}
	
	protected bool IsFloatChar(string c)
	{
		return IsDigitChar(c) || c == "." || c == "-";
	}
	
	protected bool IsDigit(string char)
	{
		return char == "0" || char == "1" || char == "2" || char == "3" || char == "4" ||
			   char == "5" || char == "6" || char == "7" || char == "8" || char == "9";
	}
	
	// ========================================================================
	// FILE PATH UTILITIES
	// ========================================================================
	
	protected void EnsureSettingsDirectory()
	{
		FileIO.MakeDirectory(RBL_SettingsConstants.SETTINGS_FOLDER);
	}
	
	string GetSettingsFilePath()
	{
		return RBL_SettingsConstants.SETTINGS_FOLDER + RBL_SettingsConstants.SETTINGS_FILENAME;
	}
	
	// ========================================================================
	// BACKUP
	// ========================================================================
	
	bool CreateBackup()
	{
		string sourcePath = GetSettingsFilePath();
		string backupPath = sourcePath + ".backup";
		
		if (!FileIO.FileExists(sourcePath))
			return false;
		
		// Read source
		FileHandle sourceFile = FileIO.OpenFile(sourcePath, FileMode.READ);
		if (!sourceFile)
			return false;
		
		string content = "";
		string line;
		while (sourceFile.ReadLine(line) >= 0)
		{
			content += line + "\n";
		}
		sourceFile.Close();
		
		// Write backup
		FileHandle backupFile = FileIO.OpenFile(backupPath, FileMode.WRITE);
		if (!backupFile)
			return false;
		
		backupFile.Write(content);
		backupFile.Close();
		
		PrintFormat("[RBL_SettingsPersistence] Backup created");
		return true;
	}
	
	bool RestoreFromBackup()
	{
		string sourcePath = GetSettingsFilePath();
		string backupPath = sourcePath + ".backup";
		
		if (!FileIO.FileExists(backupPath))
		{
			PrintFormat("[RBL_SettingsPersistence] No backup file found");
			return false;
		}
		
		// Read backup
		FileHandle backupFile = FileIO.OpenFile(backupPath, FileMode.READ);
		if (!backupFile)
			return false;
		
		string content = "";
		string line;
		while (backupFile.ReadLine(line) >= 0)
		{
			content += line + "\n";
		}
		backupFile.Close();
		
		// Write to main file
		FileHandle mainFile = FileIO.OpenFile(sourcePath, FileMode.WRITE);
		if (!mainFile)
			return false;
		
		mainFile.Write(content);
		mainFile.Close();
		
		PrintFormat("[RBL_SettingsPersistence] Settings restored from backup");
		return true;
	}
}


