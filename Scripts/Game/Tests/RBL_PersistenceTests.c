// ============================================================================
// PROJECT REBELLION - Persistence Tests
// Automated test suite for save/load system
// ============================================================================

class RBL_PersistenceTests
{
	protected static int s_iTestsPassed;
	protected static int s_iTestsFailed;
	protected static ref array<string> s_aFailedTests;
	
	// ========================================================================
	// TEST RUNNER
	// ========================================================================
	
	static void RunAllTests()
	{
		s_iTestsPassed = 0;
		s_iTestsFailed = 0;
		s_aFailedTests = new array<string>();
		
		PrintFormat("[RBL_PersistenceTests] ========================================");
		PrintFormat("[RBL_PersistenceTests] STARTING PERSISTENCE TEST SUITE");
		PrintFormat("[RBL_PersistenceTests] ========================================");
		
		// Data structure tests
		Test_SaveDataStructures();
		Test_CampaignSaveData();
		Test_ZoneSaveData();
		Test_EconomySaveData();
		Test_PlayerSaveData();
		
		// Serialization tests
		Test_JsonSerialization();
		Test_SerializeDeserializeCycle();
		
		// File operations tests
		Test_FileOperations();
		Test_SaveSlots();
		
		// Persistence layer tests
		Test_ZonePersistence();
		Test_EconomyPersistence();
		Test_CampaignPersistence();
		
		// Integration tests
		Test_FullSaveLoad();
		Test_AutoSave();
		
		// Print results
		PrintFormat("[RBL_PersistenceTests] ========================================");
		PrintFormat("[RBL_PersistenceTests] TEST RESULTS");
		PrintFormat("[RBL_PersistenceTests] Passed: %1", s_iTestsPassed);
		PrintFormat("[RBL_PersistenceTests] Failed: %1", s_iTestsFailed);
		
		if (s_iTestsFailed > 0)
		{
			PrintFormat("[RBL_PersistenceTests] FAILED TESTS:");
			for (int i = 0; i < s_aFailedTests.Count(); i++)
			{
				PrintFormat("[RBL_PersistenceTests]   - %1", s_aFailedTests[i]);
			}
		}
		
		PrintFormat("[RBL_PersistenceTests] ========================================");
	}
	
	// ========================================================================
	// ASSERTIONS
	// ========================================================================
	
	protected static void Assert(bool condition, string testName)
	{
		if (condition)
		{
			s_iTestsPassed++;
			PrintFormat("[RBL_PersistenceTests] PASS: %1", testName);
		}
		else
		{
			s_iTestsFailed++;
			s_aFailedTests.Insert(testName);
			PrintFormat("[RBL_PersistenceTests] FAIL: %1", testName);
		}
	}
	
	protected static void AssertEqual(int actual, int expected, string testName)
	{
		Assert(actual == expected, testName + " (expected " + expected.ToString() + ", got " + actual.ToString() + ")");
	}
	
	protected static void AssertEqualFloat(float actual, float expected, float tolerance, string testName)
	{
		Assert(Math.AbsFloat(actual - expected) <= tolerance, testName);
	}
	
	protected static void AssertNotNull(Class obj, string testName)
	{
		Assert(obj != null, testName);
	}
	
	protected static void AssertStringEqual(string actual, string expected, string testName)
	{
		Assert(actual == expected, testName);
	}
	
	protected static void AssertTrue(bool value, string testName)
	{
		Assert(value, testName);
	}
	
	protected static void AssertFalse(bool value, string testName)
	{
		Assert(!value, testName);
	}
	
	// ========================================================================
	// DATA STRUCTURE TESTS
	// ========================================================================
	
	static void Test_SaveDataStructures()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Save Data Structures ---");
		
		// RBL_SaveData
		RBL_SaveData saveData = new RBL_SaveData();
		AssertNotNull(saveData, "RBL_SaveData creation");
		AssertStringEqual(saveData.m_sMagic, RBL_SAVE_MAGIC, "SaveData magic header");
		AssertEqual(saveData.m_iVersion, RBL_SAVE_VERSION, "SaveData version");
		AssertTrue(saveData.IsValid(), "SaveData is valid");
		AssertFalse(saveData.NeedsMigration(), "SaveData doesn't need migration");
		
		// Test invalid data
		RBL_SaveData invalidData = new RBL_SaveData();
		invalidData.m_sMagic = "INVALID";
		AssertFalse(invalidData.IsValid(), "Invalid magic rejected");
	}
	
	static void Test_CampaignSaveData()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Campaign Save Data ---");
		
		RBL_CampaignSaveData campaign = new RBL_CampaignSaveData();
		AssertNotNull(campaign, "CampaignSaveData creation");
		AssertEqual(campaign.m_iWarLevel, 1, "Default war level");
		AssertEqual(campaign.m_iAggression, 0, "Default aggression");
		AssertEqual(campaign.m_iCivilianSupport, 50, "Default civilian support");
		AssertEqualFloat(campaign.m_fTotalPlayTime, 0, 0.01, "Default playtime");
	}
	
	static void Test_ZoneSaveData()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Zone Save Data ---");
		
		RBL_ZoneSaveData zone = new RBL_ZoneSaveData();
		AssertNotNull(zone, "ZoneSaveData creation");
		AssertTrue(zone.m_sZoneID.IsEmpty(), "Default zone ID empty");
		AssertEqual(zone.m_iOwnerFaction, 0, "Default owner faction");
		AssertEqual(zone.m_iSupportLevel, 50, "Default support level");
		AssertFalse(zone.m_bIsUnderAttack, "Default not under attack");
		AssertNotNull(zone.m_aGarrisonUnitTypes, "Garrison types array exists");
	}
	
	static void Test_EconomySaveData()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Economy Save Data ---");
		
		RBL_EconomySaveData economy = new RBL_EconomySaveData();
		AssertNotNull(economy, "EconomySaveData creation");
		AssertEqual(economy.m_iFIAMoney, 500, "Default money");
		AssertEqual(economy.m_iFIAHumanResources, 10, "Default HR");
		AssertNotNull(economy.m_aArsenalItems, "Arsenal items array exists");
		AssertNotNull(economy.m_aVehiclePool, "Vehicle pool array exists");
	}
	
	static void Test_PlayerSaveData()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Player Save Data ---");
		
		RBL_PlayerSaveData player = new RBL_PlayerSaveData();
		AssertNotNull(player, "PlayerSaveData creation");
		AssertTrue(player.m_sPlayerUID.IsEmpty(), "Default UID empty");
		AssertEqual(player.m_iMoney, 0, "Default player money");
		AssertEqual(player.m_iKills, 0, "Default kills");
		AssertNotNull(player.m_aUnlocks, "Unlocks array exists");
		AssertNotNull(player.m_aInventoryItems, "Inventory array exists");
	}
	
	// ========================================================================
	// SERIALIZATION TESTS
	// ========================================================================
	
	static void Test_JsonSerialization()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing JSON Serialization ---");
		
		// Create test data
		RBL_SaveData saveData = CreateTestSaveData();
		AssertNotNull(saveData, "Test save data created");
		
		// Serialize
		string json = RBL_SaveSerializer.SerializeToJson(saveData);
		AssertFalse(json.IsEmpty(), "Serialization produces output");
		AssertTrue(json.Contains("RBL_SAVE"), "JSON contains magic");
		AssertTrue(json.Contains("campaign"), "JSON contains campaign");
		AssertTrue(json.Contains("zones"), "JSON contains zones");
	}
	
	static void Test_SerializeDeserializeCycle()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Serialize/Deserialize Cycle ---");
		
		// Create original data
		RBL_SaveData original = CreateTestSaveData();
		original.m_Campaign.m_iWarLevel = 5;
		original.m_Campaign.m_iAggression = 75;
		original.m_Economy.m_iFIAMoney = 12345;
		
		// Add zone
		RBL_ZoneSaveData testZone = new RBL_ZoneSaveData();
		testZone.m_sZoneID = "test_zone_1";
		testZone.m_iOwnerFaction = 1;
		testZone.m_iGarrisonStrength = 10;
		original.m_aZones.Insert(testZone);
		
		// Serialize
		string json = RBL_SaveSerializer.SerializeToJson(original);
		AssertFalse(json.IsEmpty(), "Serialize produced output");
		
		// Deserialize
		RBL_SaveData restored = RBL_SaveSerializer.DeserializeFromJson(json);
		AssertNotNull(restored, "Deserialize returned data");
		
		// Verify data integrity
		AssertTrue(restored.IsValid(), "Restored data is valid");
		AssertEqual(restored.m_Campaign.m_iWarLevel, 5, "War level preserved");
		AssertEqual(restored.m_Campaign.m_iAggression, 75, "Aggression preserved");
		AssertEqual(restored.m_Economy.m_iFIAMoney, 12345, "Money preserved");
		AssertEqual(restored.m_aZones.Count(), 1, "Zone count preserved");
		
		if (restored.m_aZones.Count() > 0)
		{
			AssertStringEqual(restored.m_aZones[0].m_sZoneID, "test_zone_1", "Zone ID preserved");
			AssertEqual(restored.m_aZones[0].m_iOwnerFaction, 1, "Zone owner preserved");
		}
	}
	
	// ========================================================================
	// FILE OPERATION TESTS
	// ========================================================================
	
	static void Test_FileOperations()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing File Operations ---");
		
		RBL_SaveFileManager fileMgr = RBL_SaveFileManager.GetInstance();
		AssertNotNull(fileMgr, "SaveFileManager exists");
		
		// Test write and read
		string testContent = "{\"test\": \"data\"}";
		string testFilename = "test_persistence";
		
		bool writeResult = fileMgr.WriteToFile(testFilename, testContent);
		AssertTrue(writeResult, "File write succeeded");
		
		AssertTrue(fileMgr.SaveExists(testFilename), "File exists after write");
		
		string readContent = fileMgr.ReadFromFile(testFilename);
		AssertFalse(readContent.IsEmpty(), "File read returned content");
		AssertTrue(readContent.Contains("test"), "Read content matches written");
		
		// Test delete
		bool deleteResult = fileMgr.DeleteFile(testFilename);
		AssertTrue(deleteResult, "File delete succeeded");
		AssertFalse(fileMgr.SaveExists(testFilename), "File gone after delete");
	}
	
	static void Test_SaveSlots()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Save Slots ---");
		
		RBL_SaveSlotManager slotMgr = RBL_SaveSlotManager.GetInstance();
		AssertNotNull(slotMgr, "SaveSlotManager exists");
		
		// Test slot count
		int maxSlots = slotMgr.GetMaxSlots();
		AssertTrue(maxSlots > 0, "Max slots is positive");
		
		// Test slot selection
		slotMgr.SelectSlot(0);
		AssertEqual(slotMgr.GetCurrentSlot(), 0, "Slot selection works");
		
		slotMgr.SelectSlot(3);
		AssertEqual(slotMgr.GetCurrentSlot(), 3, "Slot change works");
		
		// Test slot info
		RBL_SaveSlotInfo info = slotMgr.GetSlotInfo(0);
		AssertNotNull(info, "Slot info exists");
		AssertEqual(info.m_iSlotIndex, 0, "Slot index correct");
	}
	
	// ========================================================================
	// PERSISTENCE LAYER TESTS
	// ========================================================================
	
	static void Test_ZonePersistence()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Zone Persistence ---");
		
		RBL_ZonePersistence zonePersist = RBL_ZonePersistence.GetInstance();
		AssertNotNull(zonePersist, "ZonePersistence exists");
		
		// Test validation
		RBL_ZoneSaveData validZone = new RBL_ZoneSaveData();
		validZone.m_sZoneID = "test_zone";
		validZone.m_iOwnerFaction = 0;
		validZone.m_iSupportLevel = 50;
		validZone.m_fCaptureProgress = 0.5;
		AssertTrue(zonePersist.ValidateZoneData(validZone), "Valid zone passes validation");
		
		RBL_ZoneSaveData invalidZone = new RBL_ZoneSaveData();
		invalidZone.m_iSupportLevel = 150; // Invalid
		AssertFalse(zonePersist.ValidateZoneData(invalidZone), "Invalid zone fails validation");
	}
	
	static void Test_EconomyPersistence()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Economy Persistence ---");
		
		RBL_EconomyPersistence econPersist = RBL_EconomyPersistence.GetInstance();
		AssertNotNull(econPersist, "EconomyPersistence exists");
		
		// Test validation
		RBL_EconomySaveData validEcon = new RBL_EconomySaveData();
		AssertTrue(econPersist.ValidateEconomyData(validEcon), "Valid economy passes validation");
		
		RBL_EconomySaveData invalidEcon = new RBL_EconomySaveData();
		invalidEcon.m_iFIAMoney = -100; // Invalid
		AssertFalse(econPersist.ValidateEconomyData(invalidEcon), "Invalid economy fails validation");
	}
	
	static void Test_CampaignPersistence()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Campaign Persistence ---");
		
		RBL_CampaignPersistence campaignPersist = RBL_CampaignPersistence.GetInstance();
		AssertNotNull(campaignPersist, "CampaignPersistence exists");
		
		// Test validation
		RBL_CampaignSaveData validCampaign = new RBL_CampaignSaveData();
		AssertTrue(campaignPersist.ValidateCampaignData(validCampaign), "Valid campaign passes validation");
		
		RBL_CampaignSaveData invalidCampaign = new RBL_CampaignSaveData();
		invalidCampaign.m_iWarLevel = 15; // Invalid (max is 10)
		AssertFalse(campaignPersist.ValidateCampaignData(invalidCampaign), "Invalid campaign fails validation");
	}
	
	// ========================================================================
	// INTEGRATION TESTS
	// ========================================================================
	
	static void Test_FullSaveLoad()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing Full Save/Load ---");
		
		// Create test data
		RBL_SaveData saveData = CreateTestSaveData();
		saveData.m_Campaign.m_iWarLevel = 7;
		saveData.m_Economy.m_iFIAMoney = 9999;
		
		// Serialize
		string json = RBL_SaveSerializer.SerializeToJson(saveData);
		
		// Write
		RBL_SaveFileManager fileMgr = RBL_SaveFileManager.GetInstance();
		bool writeOk = fileMgr.WriteToFile("test_full_save", json);
		AssertTrue(writeOk, "Full save write succeeded");
		
		// Read
		string loadedJson = fileMgr.ReadFromFile("test_full_save");
		AssertFalse(loadedJson.IsEmpty(), "Full save read succeeded");
		
		// Deserialize
		RBL_SaveData loaded = RBL_SaveSerializer.DeserializeFromJson(loadedJson);
		AssertNotNull(loaded, "Full save deserialize succeeded");
		AssertEqual(loaded.m_Campaign.m_iWarLevel, 7, "Full save preserves war level");
		AssertEqual(loaded.m_Economy.m_iFIAMoney, 9999, "Full save preserves money");
		
		// Cleanup
		fileMgr.DeleteFile("test_full_save");
	}
	
	static void Test_AutoSave()
	{
		PrintFormat("[RBL_PersistenceTests] --- Testing AutoSave ---");
		
		RBL_AutoSaveManager autoSave = RBL_AutoSaveManager.GetInstance();
		AssertNotNull(autoSave, "AutoSaveManager exists");
		
		// Test configuration
		autoSave.SetAutoSaveEnabled(true);
		AssertTrue(autoSave.IsAutoSaveEnabled(), "AutoSave enabled");
		
		autoSave.SetAutoSaveInterval(120);
		AssertEqualFloat(autoSave.GetAutoSaveInterval(), 120, 0.01, "Interval set correctly");
		
		autoSave.SetMaxAutoSaves(5);
		AssertEqual(autoSave.GetMaxAutoSaves(), 5, "Max autosaves set correctly");
		
		// Test state
		AssertFalse(autoSave.IsSaving(), "Not saving initially");
	}
	
	// ========================================================================
	// TEST DATA FACTORIES
	// ========================================================================
	
	protected static RBL_SaveData CreateTestSaveData()
	{
		RBL_SaveData saveData = new RBL_SaveData();
		
		saveData.m_sSaveTime = "2025-01-13 12:00:00";
		saveData.m_sPlayerId = "test_player";
		saveData.m_sWorldName = "TestWorld";
		
		return saveData;
	}
}

// ============================================================================
// TEST CONSOLE COMMANDS
// ============================================================================
class RBL_PersistenceTestCommands
{
	[ConsoleCmd("rbl_persist_test", "Run all persistence tests")]
	static void RunTests()
	{
		RBL_PersistenceTests.RunAllTests();
	}
	
	[ConsoleCmd("rbl_save_test", "Test save to slot 9")]
	static void TestSave()
	{
		RBL_AutoSaveManager autoSave = RBL_AutoSaveManager.GetInstance();
		autoSave.TriggerAutoSave("Test Command");
	}
	
	[ConsoleCmd("rbl_quicksave", "Quicksave")]
	static void QuickSave()
	{
		RBL_AutoSaveManager.GetInstance().QuickSave();
	}
	
	[ConsoleCmd("rbl_quickload", "Quickload")]
	static void QuickLoad()
	{
		RBL_AutoSaveManager.GetInstance().QuickLoad();
	}
	
	[ConsoleCmd("rbl_list_saves", "List all save files")]
	static void ListSaves()
	{
		array<string> saves = RBL_SaveFileManager.GetInstance().GetSaveFileList();
		PrintFormat("[RBL] Found %1 save files:", saves.Count());
		for (int i = 0; i < saves.Count(); i++)
		{
			PrintFormat("[RBL]   %1", saves[i]);
		}
	}
}

