// ============================================================================
// PROJECT REBELLION - Settings System Tests
// Comprehensive automated tests for all settings functionality
// ============================================================================

class RBL_SettingsTests
{
	// ========================================================================
	// TEST RUNNER
	// ========================================================================
	
	static void RunAll()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("\n[RBL_SettingsTests] Starting settings tests...\n");
		
		TestSettingsDataStructure();
		TestDifficultyPresets();
		TestSettingsManager();
		TestSettingsCatalog();
		TestSettingValues();
		TestSettingsPersistence();
		TestSettingsUI();
		TestSettingsIntegration();
		
		PrintFormat("[RBL_SettingsTests] Settings tests complete");
	}
	
	// ========================================================================
	// DATA STRUCTURE TESTS
	// ========================================================================
	
	static void TestSettingsDataStructure()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		PrintFormat("[RBL_SettingsTests] Testing data structure...");
		
		// Test creation
		RBL_SettingsData settings = new RBL_SettingsData();
		runner.AssertNotNull("SettingsData created", settings);
		
		// Test default values
		settings.ApplyDefaults();
		runner.Assert("Default difficulty is NORMAL", settings.m_eDifficulty == ERBLDifficulty.NORMAL, 
			"Wrong default difficulty");
		runner.Assert("Default autosave enabled", settings.m_bAutoSaveEnabled == true, 
			"Autosave should be on by default");
		runner.Assert("Default HUD enabled", settings.m_bHUDEnabled == true, 
			"HUD should be on by default");
		runner.AssertEqualFloat("Default HUD opacity is 1.0", 1.0, settings.m_fHUDOpacity);
		runner.AssertEqualFloat("Default UI scale is 1.0", 1.0, settings.m_fUIScale);
		runner.AssertEqualFloat("Default master volume is 1.0", 1.0, settings.m_fMasterVolume);
		
		// Test copy
		RBL_SettingsData copy = new RBL_SettingsData();
		settings.m_eDifficulty = ERBLDifficulty.HARD;
		settings.m_fAutoSaveInterval = 600.0;
		settings.m_fHUDOpacity = 0.75;
		
		copy.CopyFrom(settings);
		runner.Assert("Copy preserves difficulty", copy.m_eDifficulty == ERBLDifficulty.HARD, 
			"Difficulty not copied");
		runner.AssertEqualFloat("Copy preserves autosave interval", 600.0, copy.m_fAutoSaveInterval);
		runner.AssertEqualFloat("Copy preserves HUD opacity", 0.75, copy.m_fHUDOpacity);
		
		// Test equality
		runner.Assert("Copy equals original", settings.Equals(copy), "Copies should be equal");
		
		copy.m_eDifficulty = ERBLDifficulty.EASY;
		runner.Assert("Modified copy not equal", !settings.Equals(copy), "Modified should not be equal");
	}
	
	// ========================================================================
	// DIFFICULTY PRESET TESTS
	// ========================================================================
	
	static void TestDifficultyPresets()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		PrintFormat("[RBL_SettingsTests] Testing difficulty presets...");
		
		// Test EASY
		int easyMoney = RBL_DifficultyPresets.GetStartingMoney(ERBLDifficulty.EASY);
		int easyHR = RBL_DifficultyPresets.GetStartingHR(ERBLDifficulty.EASY);
		float easyAggression = RBL_DifficultyPresets.GetAggressionMultiplier(ERBLDifficulty.EASY);
		int easyDeaths = RBL_DifficultyPresets.GetMaxPlayerDeaths(ERBLDifficulty.EASY);
		
		runner.Assert("EASY has high starting money", easyMoney >= 1000, "EASY should have 1000+ money");
		runner.Assert("EASY has high starting HR", easyHR >= 15, "EASY should have 15+ HR");
		runner.Assert("EASY has low aggression", easyAggression < 1.0, "EASY should have < 1.0 aggression");
		runner.Assert("EASY has unlimited deaths", easyDeaths == -1, "EASY should have unlimited deaths");
		
		// Test NORMAL
		int normalMoney = RBL_DifficultyPresets.GetStartingMoney(ERBLDifficulty.NORMAL);
		float normalAggression = RBL_DifficultyPresets.GetAggressionMultiplier(ERBLDifficulty.NORMAL);
		
		runner.Assert("NORMAL has moderate starting money", normalMoney == 500, "NORMAL should have 500 money");
		runner.AssertEqualFloat("NORMAL has 1.0 aggression", 1.0, normalAggression);
		
		// Test HARD
		int hardMoney = RBL_DifficultyPresets.GetStartingMoney(ERBLDifficulty.HARD);
		float hardIncome = RBL_DifficultyPresets.GetIncomeMultiplier(ERBLDifficulty.HARD);
		
		runner.Assert("HARD has less starting money", hardMoney < normalMoney, "HARD should have less money");
		runner.Assert("HARD has reduced income", hardIncome < 1.0, "HARD should have reduced income");
		
		// Test HARDCORE
		int hardcoreMoney = RBL_DifficultyPresets.GetStartingMoney(ERBLDifficulty.HARDCORE);
		int hardcoreDeaths = RBL_DifficultyPresets.GetMaxPlayerDeaths(ERBLDifficulty.HARDCORE);
		float hardcoreDetection = RBL_DifficultyPresets.GetDetectionMultiplier(ERBLDifficulty.HARDCORE);
		
		runner.Assert("HARDCORE has minimal starting money", hardcoreMoney <= 100, "HARDCORE should have <= 100 money");
		runner.Assert("HARDCORE has permadeath", hardcoreDeaths == 1, "HARDCORE should have 1 life");
		runner.Assert("HARDCORE has high detection", hardcoreDetection >= 2.0, "HARDCORE should have 2.0+ detection");
		
		// Test difficulty progression (harder = less resources)
		runner.Assert("Money decreases with difficulty", 
			easyMoney > normalMoney && normalMoney > hardMoney && hardMoney > hardcoreMoney,
			"Money should decrease with difficulty");
		
		// Test difficulty names
		runner.AssertStringEqual("EASY name", "EASY", RBL_DifficultyPresets.GetDifficultyName(ERBLDifficulty.EASY));
		runner.AssertStringEqual("NORMAL name", "NORMAL", RBL_DifficultyPresets.GetDifficultyName(ERBLDifficulty.NORMAL));
		runner.AssertStringEqual("HARD name", "HARD", RBL_DifficultyPresets.GetDifficultyName(ERBLDifficulty.HARD));
		runner.AssertStringEqual("HARDCORE name", "HARDCORE", RBL_DifficultyPresets.GetDifficultyName(ERBLDifficulty.HARDCORE));
		
		// Test descriptions exist
		string easyDesc = RBL_DifficultyPresets.GetDifficultyDescription(ERBLDifficulty.EASY);
		runner.Assert("EASY has description", easyDesc.Length() > 0, "EASY should have description");
	}
	
	// ========================================================================
	// SETTINGS MANAGER TESTS
	// ========================================================================
	
	static void TestSettingsManager()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		PrintFormat("[RBL_SettingsTests] Testing settings manager...");
		
		// Test singleton
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		runner.AssertNotNull("SettingsManager singleton exists", mgr);
		
		// Test double call returns same instance
		RBL_SettingsManager mgr2 = RBL_SettingsManager.GetInstance();
		runner.Assert("Singleton returns same instance", mgr == mgr2, "Should be same instance");
		
		// Test initialization
		mgr.Initialize();
		runner.Assert("Manager is initialized", mgr.IsInitialized(), "Should be initialized");
		
		// Test settings data
		RBL_SettingsData current = mgr.GetCurrentSettings();
		runner.AssertNotNull("GetCurrentSettings returns data", current);
		
		RBL_SettingsData pending = mgr.GetPendingSettings();
		runner.AssertNotNull("GetPendingSettings returns data", pending);
		
		// Test no unsaved changes initially
		mgr.DiscardChanges();
		runner.Assert("No unsaved changes after discard", !mgr.HasUnsavedChanges(), 
			"Should have no unsaved changes");
	}
	
	// ========================================================================
	// SETTINGS CATALOG TESTS
	// ========================================================================
	
	static void TestSettingsCatalog()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		PrintFormat("[RBL_SettingsTests] Testing settings catalog...");
		
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		mgr.Initialize();
		
		// Test total setting count
		int total = mgr.GetTotalSettingCount();
		runner.Assert("Has settings in catalog", total > 0, "Should have settings");
		runner.Assert("Has expected number of settings", total >= 20, 
			string.Format("Expected 20+ settings, got %1", total));
		
		// Test category counts
		int gameplayCount = mgr.GetSettingCountByCategory(ERBLSettingsCategory.GAMEPLAY);
		int displayCount = mgr.GetSettingCountByCategory(ERBLSettingsCategory.DISPLAY);
		int audioCount = mgr.GetSettingCountByCategory(ERBLSettingsCategory.AUDIO);
		int controlCount = mgr.GetSettingCountByCategory(ERBLSettingsCategory.CONTROLS);
		
		runner.Assert("Has gameplay settings", gameplayCount > 0, "Should have gameplay settings");
		runner.Assert("Has display settings", displayCount > 0, "Should have display settings");
		runner.Assert("Has audio settings", audioCount > 0, "Should have audio settings");
		runner.Assert("Has control settings", controlCount > 0, "Should have control settings");
		
		// Test specific settings exist
		RBL_SettingItem diffSetting = mgr.GetSettingByID("difficulty");
		runner.AssertNotNull("Difficulty setting exists", diffSetting);
		if (diffSetting)
		{
			runner.Assert("Difficulty is dropdown type", diffSetting.m_eType == ERBLSettingType.DROPDOWN,
				"Difficulty should be dropdown");
			runner.Assert("Difficulty has options", diffSetting.m_aOptions.Count() == 4,
				"Difficulty should have 4 options");
		}
		
		RBL_SettingItem autosaveSetting = mgr.GetSettingByID("autosave_enabled");
		runner.AssertNotNull("Autosave setting exists", autosaveSetting);
		if (autosaveSetting)
		{
			runner.Assert("Autosave is toggle type", autosaveSetting.m_eType == ERBLSettingType.TOGGLE,
				"Autosave should be toggle");
		}
		
		RBL_SettingItem opacitySetting = mgr.GetSettingByID("hud_opacity");
		runner.AssertNotNull("HUD opacity setting exists", opacitySetting);
		if (opacitySetting)
		{
			runner.Assert("Opacity is slider type", opacitySetting.m_eType == ERBLSettingType.SLIDER,
				"Opacity should be slider");
			runner.Assert("Opacity min is 0.25", opacitySetting.m_fMinValue >= 0.25,
				"Opacity min should be >= 0.25");
			runner.Assert("Opacity max is 1.0", opacitySetting.m_fMaxValue == 1.0,
				"Opacity max should be 1.0");
		}
	}
	
	// ========================================================================
	// SETTING VALUE TESTS
	// ========================================================================
	
	static void TestSettingValues()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		PrintFormat("[RBL_SettingsTests] Testing setting values...");
		
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		mgr.Initialize();
		mgr.DiscardChanges();
		
		// Test bool getter/setter
		bool origAutosave = mgr.GetSettingBool("autosave_enabled");
		mgr.SetSettingBool("autosave_enabled", !origAutosave);
		runner.Assert("Bool setting changed", mgr.GetSettingBool("autosave_enabled") != origAutosave,
			"Bool should toggle");
		runner.Assert("Has unsaved changes after bool change", mgr.HasUnsavedChanges(),
			"Should have unsaved changes");
		
		// Test float getter/setter
		mgr.SetSettingFloat("hud_opacity", 0.5);
		float opacity = mgr.GetSettingFloat("hud_opacity");
		runner.AssertEqualFloat("Float setting changed", 0.5, opacity);
		
		// Test float clamping
		mgr.SetSettingFloat("hud_opacity", 2.0);
		float clampedOpacity = mgr.GetSettingFloat("hud_opacity");
		runner.Assert("Float clamped to max", clampedOpacity <= 1.0, "Should clamp to 1.0");
		
		mgr.SetSettingFloat("hud_opacity", 0.0);
		float clampedOpacityMin = mgr.GetSettingFloat("hud_opacity");
		runner.Assert("Float clamped to min", clampedOpacityMin >= 0.25, "Should clamp to >= 0.25");
		
		// Test int getter/setter
		mgr.SetSettingInt("difficulty", 2);
		int diff = mgr.GetSettingInt("difficulty");
		runner.AssertEqual("Int setting changed", 2, diff);
		
		// Test int clamping
		mgr.SetSettingInt("difficulty", 10);
		int clampedDiff = mgr.GetSettingInt("difficulty");
		runner.Assert("Int clamped to max", clampedDiff <= 3, "Should clamp to 3");
		
		// Test discard changes
		mgr.DiscardChanges();
		runner.Assert("No unsaved changes after discard", !mgr.HasUnsavedChanges(),
			"Should have no unsaved changes");
		
		// Test reset to defaults
		mgr.ResetToDefaults();
		float defaultOpacity = mgr.GetSettingFloat("hud_opacity");
		runner.AssertEqualFloat("Reset restores default opacity", 1.0, defaultOpacity);
		
		mgr.DiscardChanges();
	}
	
	// ========================================================================
	// PERSISTENCE TESTS
	// ========================================================================
	
	static void TestSettingsPersistence()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		PrintFormat("[RBL_SettingsTests] Testing settings persistence...");
		
		// Test singleton
		RBL_SettingsPersistence persistence = RBL_SettingsPersistence.GetInstance();
		runner.AssertNotNull("SettingsPersistence singleton exists", persistence);
		
		// Test file path
		string filePath = persistence.GetSettingsFilePath();
		runner.Assert("File path not empty", filePath.Length() > 0, "Should have file path");
		runner.Assert("File path has correct extension", filePath.Contains(".rblcfg"),
			"Should have .rblcfg extension");
		
		// Test serialization round-trip
		RBL_SettingsData original = new RBL_SettingsData();
		original.ApplyDefaults();
		original.m_eDifficulty = ERBLDifficulty.HARD;
		original.m_fAutoSaveInterval = 600.0;
		original.m_bUndercoverSystemEnabled = false;
		original.m_fHUDOpacity = 0.8;
		original.m_fMasterVolume = 0.5;
		
		// Save
		bool saveResult = persistence.SaveSettings(original);
		runner.Assert("Settings saved successfully", saveResult, "Save should succeed");
		
		// Load into new object
		RBL_SettingsData loaded = new RBL_SettingsData();
		bool loadResult = persistence.LoadSettings(loaded);
		runner.Assert("Settings loaded successfully", loadResult, "Load should succeed");
		
		// Verify values
		runner.Assert("Loaded difficulty matches", loaded.m_eDifficulty == ERBLDifficulty.HARD,
			"Difficulty should match");
		runner.AssertEqualFloat("Loaded autosave interval matches", 600.0, loaded.m_fAutoSaveInterval);
		runner.Assert("Loaded undercover setting matches", !loaded.m_bUndercoverSystemEnabled,
			"Undercover should be false");
		runner.AssertEqualFloat("Loaded HUD opacity matches", 0.8, loaded.m_fHUDOpacity);
		runner.AssertEqualFloat("Loaded master volume matches", 0.5, loaded.m_fMasterVolume);
		
		// Test settings file exists
		runner.Assert("Settings file exists after save", persistence.SettingsFileExists(),
			"File should exist");
		
		// Test backup
		bool backupResult = persistence.CreateBackup();
		runner.Assert("Backup created", backupResult, "Backup should succeed");
	}
	
	// ========================================================================
	// UI TESTS
	// ========================================================================
	
	static void TestSettingsUI()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		PrintFormat("[RBL_SettingsTests] Testing settings UI...");
		
		// Test widget creation
		RBL_SettingsMenuWidget widget = new RBL_SettingsMenuWidget();
		runner.AssertNotNull("Settings widget created", widget);
		
		// Test initial state
		runner.Assert("Widget starts closed", !widget.IsVisible(), "Should start closed");
		
		// Test category count
		runner.AssertEqual("Widget has 4 categories", 4, widget.GetCategoryCount());
		
		// Test selection indices
		runner.Assert("Initial category index is 0", widget.GetSelectedCategoryIndex() == 0,
			"Should start at category 0");
		runner.Assert("Initial item index is 0", widget.GetSelectedItemIndex() == 0,
			"Should start at item 0");
		
		// Test open/close
		widget.Open();
		runner.Assert("Widget opens", widget.IsVisible(), "Should be visible after open");
		
		// Test navigation (simulate)
		widget.SelectNextCategory();
		runner.Assert("Category advanced", widget.GetSelectedCategoryIndex() == 1,
			"Should advance to category 1");
		
		widget.SelectPreviousCategory();
		runner.Assert("Category back", widget.GetSelectedCategoryIndex() == 0,
			"Should return to category 0");
		
		widget.SelectNextItem();
		runner.Assert("Item advanced", widget.GetSelectedItemIndex() == 1,
			"Should advance to item 1");
		
		widget.SelectPreviousItem();
		runner.Assert("Item back", widget.GetSelectedItemIndex() == 0,
			"Should return to item 0");
		
		// Test category wrap-around
		widget.SelectPreviousCategory();
		runner.Assert("Category wraps to end", widget.GetSelectedCategoryIndex() == 3,
			"Should wrap to category 3");
		
		// Test editing state
		runner.Assert("Not editing initially", !widget.IsEditingValue(),
			"Should not be editing");
		runner.Assert("Not showing confirmation initially", !widget.IsShowingConfirmation(),
			"Should not show confirmation");
		
		// Test close with no unsaved changes
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		mgr.DiscardChanges();
		
		widget.ForceClose();
		runner.RecordResult("Widget closes without error", true, "OK");
	}
	
	// ========================================================================
	// INTEGRATION TESTS
	// ========================================================================
	
	static void TestSettingsIntegration()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		PrintFormat("[RBL_SettingsTests] Testing settings integration...");
		
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		mgr.Initialize();
		
		// Test difficulty multiplier getters
		mgr.SetSettingInt("difficulty", ERBLDifficulty.EASY);
		float easyAggression = mgr.GetAggressionMultiplier();
		
		mgr.SetSettingInt("difficulty", ERBLDifficulty.HARD);
		float hardAggression = mgr.GetAggressionMultiplier();
		
		runner.Assert("Aggression multiplier changes with difficulty", 
			hardAggression > easyAggression, "HARD should have higher aggression");
		
		// Test income multiplier
		mgr.SetSettingInt("difficulty", ERBLDifficulty.EASY);
		float easyIncome = mgr.GetIncomeMultiplier();
		
		mgr.SetSettingInt("difficulty", ERBLDifficulty.HARDCORE);
		float hardcoreIncome = mgr.GetIncomeMultiplier();
		
		runner.Assert("Income multiplier changes with difficulty",
			easyIncome > hardcoreIncome, "EASY should have higher income");
		
		// Test detection multiplier
		mgr.SetSettingInt("difficulty", ERBLDifficulty.EASY);
		float easyDetection = mgr.GetDetectionMultiplier();
		
		mgr.SetSettingInt("difficulty", ERBLDifficulty.HARDCORE);
		float hardcoreDetection = mgr.GetDetectionMultiplier();
		
		runner.Assert("Detection multiplier changes with difficulty",
			hardcoreDetection > easyDetection, "HARDCORE should have higher detection");
		
		// Test event invokers exist
		ScriptInvoker onChanged = mgr.GetOnSettingsChanged();
		runner.AssertNotNull("OnSettingsChanged invoker exists", onChanged);
		
		ScriptInvoker onSaved = mgr.GetOnSettingsSaved();
		runner.AssertNotNull("OnSettingsSaved invoker exists", onSaved);
		
		ScriptInvoker onReset = mgr.GetOnSettingsReset();
		runner.AssertNotNull("OnSettingsReset invoker exists", onReset);
		
		// Test apply doesn't crash
		mgr.ResetToDefaults();
		mgr.ApplySettings();
		runner.RecordResult("ApplySettings completes without crash", true, "OK");
		
		// Reset for other tests
		mgr.DiscardChanges();
	}
}

// ============================================================================
// CONSOLE COMMAND TO RUN SETTINGS TESTS
// ============================================================================
class RBL_SettingsTestCommands
{
	static void RunSettingsTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_SettingsTests.RunAll();
		runner.PrintResults();
	}
	
	static void TestDifficulty()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_SettingsTests.TestDifficultyPresets();
		runner.PrintResults();
	}
	
	static void TestPersistence()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_SettingsTests.TestSettingsPersistence();
		runner.PrintResults();
	}
	
	static void TestUI()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_SettingsTests.TestSettingsUI();
		runner.PrintResults();
	}
}


