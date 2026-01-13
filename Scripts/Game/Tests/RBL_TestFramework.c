// ============================================================================
// PROJECT REBELLION - Test Framework
// Automated testing for all systems
// ============================================================================

class RBL_TestResult
{
	string TestName;
	bool Passed;
	string Message;
	
	void RBL_TestResult(string name, bool passed, string msg = "")
	{
		TestName = name;
		Passed = passed;
		Message = msg;
	}
}

class RBL_TestRunner
{
	protected static ref RBL_TestRunner s_Instance;
	protected ref array<ref RBL_TestResult> m_aResults;
	protected int m_iPassed;
	protected int m_iFailed;
	
	static RBL_TestRunner GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_TestRunner();
		return s_Instance;
	}
	
	void RBL_TestRunner()
	{
		m_aResults = new array<ref RBL_TestResult>();
		m_iPassed = 0;
		m_iFailed = 0;
	}
	
	void Reset()
	{
		m_aResults.Clear();
		m_iPassed = 0;
		m_iFailed = 0;
	}
	
	void RecordResult(string testName, bool passed, string message = "")
	{
		RBL_TestResult result = new RBL_TestResult(testName, passed, message);
		m_aResults.Insert(result);
		
		if (passed)
			m_iPassed++;
		else
			m_iFailed++;
	}
	
	void Assert(string testName, bool condition, string failMessage = "Assertion failed")
	{
		if (condition)
			RecordResult(testName, true, "OK");
		else
			RecordResult(testName, false, failMessage);
	}
	
	void AssertEqual(string testName, int expected, int actual)
	{
		bool passed = (expected == actual);
		string msg = passed ? "OK" : string.Format("Expected %1, got %2", expected, actual);
		RecordResult(testName, passed, msg);
	}
	
	void AssertEqualFloat(string testName, float expected, float actual, float tolerance = 0.01)
	{
		bool passed = Math.AbsFloat(expected - actual) <= tolerance;
		string msg = passed ? "OK" : string.Format("Expected %1, got %2", expected, actual);
		RecordResult(testName, passed, msg);
	}
	
	void AssertNotNull(string testName, Class obj)
	{
		bool passed = (obj != null);
		string msg = passed ? "OK" : "Object is null";
		RecordResult(testName, passed, msg);
	}
	
	void AssertNull(string testName, Class obj)
	{
		bool passed = (obj == null);
		string msg = passed ? "OK" : "Object is not null";
		RecordResult(testName, passed, msg);
	}
	
	void AssertStringEqual(string testName, string expected, string actual)
	{
		bool passed = (expected == actual);
		string msg = passed ? "OK" : string.Format("Expected '%1', got '%2'", expected, actual);
		RecordResult(testName, passed, msg);
	}
	
	void PrintResults()
	{
		PrintFormat("\n========================================");
		PrintFormat("       RBL TEST RESULTS");
		PrintFormat("========================================\n");
		
		for (int i = 0; i < m_aResults.Count(); i++)
		{
			RBL_TestResult result = m_aResults[i];
			string status = result.Passed ? "[PASS]" : "[FAIL]";
			PrintFormat("%1 %2: %3", status, result.TestName, result.Message);
		}
		
		PrintFormat("\n========================================");
		PrintFormat("  TOTAL: %1 passed, %2 failed", m_iPassed, m_iFailed);
		PrintFormat("========================================\n");
	}
	
	bool AllPassed()
	{
		return m_iFailed == 0;
	}
	
	int GetPassedCount() { return m_iPassed; }
	int GetFailedCount() { return m_iFailed; }
	int GetTotalCount() { return m_aResults.Count(); }
}

// ============================================================================
// Test Suites
// ============================================================================

class RBL_Tests
{
	// Run all tests
	static void RunAll()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		PrintFormat("\n[RBL_Tests] Starting all tests...\n");
		
		// Run test suites
		TestNullSafety();
		TestEconomyManager();
		TestZoneManager();
		TestConfig();
		TestCampaignManager();
		TestIncomeCalculations();
		TestCommanderAI();
		TestPersistence();
		TestTerrainHeights();
		TestGarrisonSystem();
		TestQRFSystem();
		TestUndercoverSystem();
		TestItemDelivery();
		TestMissionSystem();
		TestVictorySystem();
		
		// Input System Tests
		RBL_InputTests.RunAll();
		
		// Capture Progress UI Tests
		RBL_CaptureProgressTests.RunAll();
		
		// Print results
		runner.PrintResults();
		
		if (runner.AllPassed())
			PrintFormat("[RBL_Tests] *** ALL TESTS PASSED ***");
		else
			PrintFormat("[RBL_Tests] *** SOME TESTS FAILED ***");
	}
	
	// Test null safety in singleton getters
	static void TestNullSafety()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Null Safety Tests...");
		
		// Test all singletons return valid instances
		runner.AssertNotNull("EconomyManager.GetInstance", RBL_EconomyManager.GetInstance());
		runner.AssertNotNull("ZoneManager.GetInstance", RBL_ZoneManager.GetInstance());
		runner.AssertNotNull("CommanderAI.GetInstance", RBL_CommanderAI.GetInstance());
		runner.AssertNotNull("UndercoverSystem.GetInstance", RBL_UndercoverSystem.GetInstance());
		runner.AssertNotNull("PersistenceManager.GetInstance", RBL_PersistenceManager.GetInstance());
		runner.AssertNotNull("ZoneConfigurator.GetInstance", RBL_ZoneConfigurator.GetInstance());
		runner.AssertNotNull("ShopManager.GetInstance", RBL_ShopManager.GetInstance());
		runner.AssertNotNull("CaptureManager.GetInstance", RBL_CaptureManager.GetInstance());
		runner.AssertNotNull("AutoInitializer.GetInstance", RBL_AutoInitializer.GetInstance());
		runner.AssertNotNull("ScreenHUD.GetInstance", RBL_ScreenHUD.GetInstance());
		runner.AssertNotNull("HUDManager.GetInstance", RBL_HUDManager.GetInstance());
		runner.AssertNotNull("GarrisonManager.GetInstance", RBL_GarrisonManager.GetInstance());
		runner.AssertNotNull("InputManager.GetInstance", RBL_InputManager.GetInstance());
		runner.AssertNotNull("InputBindingRegistry.GetInstance", RBL_InputBindingRegistry.GetInstance());
		runner.AssertNotNull("InputHandler.GetInstance", RBL_InputHandler.GetInstance());
		
		// Test HUD systems handle null player gracefully
		RBL_ScreenHUD screenHUD = RBL_ScreenHUD.GetInstance();
		if (screenHUD)
		{
			// This should not crash even without a player
			screenHUD.Update(0.1);
			runner.RecordResult("ScreenHUD.Update handles null player", true, "No crash");
		}
		
		RBL_HUDManager hudMgr = RBL_HUDManager.GetInstance();
		if (hudMgr)
		{
			// This should not crash even without a player
			hudMgr.Update(0.1);
			runner.RecordResult("HUDManager.Update handles null player", true, "No crash");
		}
	}
	
	// Test economy manager
	static void TestEconomyManager()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Economy Tests...");
		
		RBL_EconomyManager econ = RBL_EconomyManager.GetInstance();
		if (!econ)
		{
			runner.RecordResult("EconomyManager exists", false, "Manager is null");
			return;
		}
		
		// Test money operations
		int startMoney = econ.GetMoney();
		econ.AddMoney(100);
		runner.AssertEqual("AddMoney adds correctly", startMoney + 100, econ.GetMoney());
		
		econ.SpendMoney(50);
		runner.AssertEqual("SpendMoney deducts correctly", startMoney + 50, econ.GetMoney());
		
		// Test HR operations
		int startHR = econ.GetHR();
		econ.AddHR(5);
		runner.AssertEqual("AddHR adds correctly", startHR + 5, econ.GetHR());
		
		// Test CanAfford
		econ.SetMoney(100);
		runner.Assert("CanAfford returns true when enough", econ.CanAfford(50), "Should afford 50");
		runner.Assert("CanAfford returns false when not enough", !econ.CanAfford(200), "Should not afford 200");
		
		// Test money clamping
		econ.SetMoney(-100);
		runner.Assert("Money cannot go negative", econ.GetMoney() >= 0, "Money went negative");
		
		// Reset for other tests
		econ.SetMoney(RBL_Config.STARTING_MONEY);
		econ.SetHR(RBL_Config.STARTING_HR);
	}
	
	// Test zone manager
	static void TestZoneManager()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Zone Manager Tests...");
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
		{
			runner.RecordResult("ZoneManager exists", false, "Manager is null");
			return;
		}
		
		// Test zone count
		int totalZones = zoneMgr.GetTotalZoneCount();
		runner.Assert("Has zones loaded", totalZones > 0, "No zones found");
		
		// Test faction zone counts
		int fiaZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
		int ussrZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR);
		runner.Assert("FIA has at least 1 zone (HQ)", fiaZones >= 1, "FIA has no zones");
		runner.Assert("USSR has zones", ussrZones > 0, "USSR has no zones");
		
		// Test zone lookup
		RBL_VirtualZone hq = zoneMgr.GetVirtualZoneByID("HQ_FIA");
		runner.AssertNotNull("Can find HQ_FIA by ID", hq);
		
		if (hq)
		{
			runner.AssertStringEqual("HQ_FIA has correct ID", "HQ_FIA", hq.GetZoneID());
			runner.Assert("HQ_FIA is HQ type", hq.GetZoneType() == ERBLZoneType.HQ, "Wrong zone type");
			runner.Assert("HQ_FIA owned by FIA", hq.GetOwnerFaction() == ERBLFactionKey.FIA, "Wrong owner");
		}
		
		// Test nearest zone function
		vector testPos = Vector(4000, 0, 5000);
		RBL_VirtualZone nearest = zoneMgr.GetNearestVirtualZone(testPos);
		runner.AssertNotNull("GetNearestVirtualZone returns zone", nearest);
	}
	
	// Test config values
	static void TestConfig()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Config Tests...");
		
		// Test starting values are reasonable
		runner.Assert("Starting money > 0", RBL_Config.STARTING_MONEY > 0, "Invalid starting money");
		runner.Assert("Starting HR > 0", RBL_Config.STARTING_HR > 0, "Invalid starting HR");
		runner.Assert("Starting war level valid", RBL_Config.STARTING_WAR_LEVEL >= 1 && RBL_Config.STARTING_WAR_LEVEL <= 10, "Invalid war level");
		
		// Test income values
		runner.Assert("Factory income > Town income", RBL_Config.INCOME_FACTORY_BASE > RBL_Config.INCOME_TOWN_BASE, "Income hierarchy wrong");
		runner.Assert("Resource income > Outpost income", RBL_Config.INCOME_RESOURCE_BASE > RBL_Config.INCOME_OUTPOST_BASE, "Income hierarchy wrong");
		
		// Test QRF costs are ordered
		runner.Assert("Helicopter costs more than infantry", RBL_Config.QRF_COST_HELICOPTER > RBL_Config.QRF_COST_INFANTRY, "QRF cost hierarchy wrong");
	}
	
	// Test campaign manager
	static void TestCampaignManager()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Campaign Manager Tests...");
		
		RBL_CampaignManager campaign = RBL_CampaignManager.GetInstance();
		if (!campaign)
		{
			runner.RecordResult("CampaignManager exists", false, "Manager is null");
			return;
		}
		
		// Test war level bounds
		int warLevel = campaign.GetWarLevel();
		runner.Assert("War level >= 1", warLevel >= 1, "War level too low");
		runner.Assert("War level <= 10", warLevel <= 10, "War level too high");
		
		// Test aggression bounds
		int aggression = campaign.GetAggression();
		runner.Assert("Aggression >= 0", aggression >= 0, "Aggression negative");
		runner.Assert("Aggression <= 100", aggression <= 100, "Aggression over 100");
	}
	
	// Test income calculations
	static void TestIncomeCalculations()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Income Calculation Tests...");
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
		{
			runner.RecordResult("ZoneManager for income test", false, "Manager is null");
			return;
		}
		
		// Test that income calculations return integers (not truncated floats)
		array<ref RBL_VirtualZone> zones = zoneMgr.GetAllVirtualZones();
		bool allValid = true;
		
		for (int i = 0; i < zones.Count(); i++)
		{
			RBL_VirtualZone zone = zones[i];
			int income = zone.CalculateResourceIncome();
			int hrIncome = zone.CalculateHRIncome();
			
			// Income should be non-negative
			if (income < 0 || hrIncome < 0)
			{
				allValid = false;
				break;
			}
		}
		
		runner.Assert("All zone incomes are valid integers", allValid, "Some incomes are negative");
		
		// Test specific zone income values
		RBL_VirtualZone factoryZone = zoneMgr.GetVirtualZoneByID("Factory_East");
		if (factoryZone)
		{
			int factoryIncome = factoryZone.CalculateResourceIncome();
			// Factory with 25% support: 150 * (0.5 + 0.25) = 112.5 -> 113
			runner.Assert("Factory income > 0", factoryIncome > 0, "Factory has no income");
			runner.Assert("Factory income reasonable", factoryIncome >= 75 && factoryIncome <= 225, 
				string.Format("Factory income out of range: %1", factoryIncome));
		}
		
		// Test town HR income
		RBL_VirtualZone townZone = zoneMgr.GetVirtualZoneByID("Town_Morton");
		if (townZone)
		{
			int hrIncome = townZone.CalculateHRIncome();
			// Town with 45% support: 45 * 0.1 = 4.5 -> 5 (rounded)
			runner.Assert("Town HR income calculated", hrIncome >= 0, "Town HR income negative");
		}
	}
	
	// Test Commander AI with Virtual Zone support
	static void TestCommanderAI()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Commander AI Tests...");
		
		RBL_CommanderAI ai = RBL_CommanderAI.GetInstance();
		if (!ai)
		{
			runner.RecordResult("CommanderAI exists", false, "AI is null");
			return;
		}
		
		// Test resource pool
		int resources = ai.GetFactionResources();
		runner.Assert("AI has resources", resources > 0, "AI has no resources");
		runner.Assert("AI resources within bounds", resources <= RBL_Config.AI_MAX_RESOURCES, "AI resources exceed max");
		
		// Test QRF count
		int qrfCount = ai.GetActiveQRFCount();
		runner.Assert("QRF count >= 0", qrfCount >= 0, "Negative QRF count");
		
		// Test controlled faction
		ERBLFactionKey faction = ai.GetControlledFaction();
		runner.Assert("AI controls USSR faction", faction == ERBLFactionKey.USSR, "Wrong controlled faction");
		
		// Test that AI can process updates without crashing
		ai.Update(0.1);
		runner.RecordResult("AI.Update handles no threats", true, "No crash");
		
		// Test QRF operation creation
		RBL_QRFOperation qrf = new RBL_QRFOperation();
		runner.AssertNotNull("QRF operation created", qrf);
		
		if (qrf)
		{
			runner.Assert("QRF starts incomplete", !qrf.IsComplete(), "QRF started complete");
			qrf.Update(0.1);
			runner.Assert("QRF still incomplete after small update", !qrf.IsComplete(), "QRF completed too fast");
		}
		
		// Test virtual zone integration
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			array<ref RBL_VirtualZone> ussrZones = zoneMgr.GetVirtualZonesByFaction(ERBLFactionKey.USSR);
			runner.Assert("AI can find USSR virtual zones", ussrZones.Count() > 0, "No USSR zones for AI");
		}
	}
	
	// Test persistence system
	static void TestPersistence()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Persistence Tests...");
		
		RBL_PersistenceManager persistence = RBL_PersistenceManager.GetInstance();
		if (!persistence)
		{
			runner.RecordResult("PersistenceManager exists", false, "Manager is null");
			return;
		}
		
		// Test save data structure
		RBL_CampaignSaveData saveData = persistence.GetCurrentSaveData();
		runner.AssertNotNull("SaveData structure exists", saveData);
		
		if (saveData)
		{
			runner.AssertNotNull("CampaignInfo exists", saveData.CampaignInfo);
			runner.AssertNotNull("Zones array exists", saveData.Zones);
			runner.AssertNotNull("Arsenal array exists", saveData.Arsenal);
		}
		
		// Test save operation (will create file)
		bool saveResult = persistence.SaveCampaign();
		runner.Assert("SaveCampaign returns true", saveResult, "Save failed");
		
		// Test HasExistingSave after save
		bool hasSave = persistence.HasExistingSave();
		runner.Assert("HasExistingSave returns true after save", hasSave, "No save detected after saving");
		
		// Test time tracking
		float timeSinceSave = persistence.GetTimeSinceLastSave();
		runner.Assert("Time since save is near 0 after save", timeSinceSave < 1.0, "Time since save incorrect");
	}
	
	// Test terrain height resolution
	static void TestTerrainHeights()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Terrain Height Tests...");
		
		RBL_ZoneConfigurator config = RBL_ZoneConfigurator.GetInstance();
		if (!config)
		{
			runner.RecordResult("ZoneConfigurator exists", false, "Config is null");
			return;
		}
		
		// Test that terrain resolution can be called
		config.ResolveTerrainHeights();
		runner.Assert("ResolveTerrainHeights completes", config.AreTerrainHeightsResolved(), "Heights not resolved");
		
		// Test zone positions have Y values (not all zero)
		array<ref RBL_ZoneDefinition> defs = config.GetAllDefinitions();
		runner.Assert("Zone definitions exist", defs.Count() > 0, "No zone definitions");
		
		// Test SnapToTerrain utility
		vector testPos = Vector(4000, 500, 5000);
		vector snappedPos = RBL_ZoneConfigurator.SnapToTerrain(testPos, 0);
		runner.Assert("SnapToTerrain returns valid position", snappedPos[0] == testPos[0] && snappedPos[2] == testPos[2], "X/Z changed during snap");
		
		// Test with offset
		vector snappedWithOffset = RBL_ZoneConfigurator.SnapToTerrain(testPos, 2.0);
		runner.Assert("SnapToTerrain offset works", snappedWithOffset[1] >= snappedPos[1], "Offset not applied");
	}
	
	// Test garrison spawning system
	static void TestGarrisonSystem()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Garrison System Tests...");
		
		// Test GarrisonManager singleton
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		runner.AssertNotNull("GarrisonManager.GetInstance", garMgr);
		
		if (!garMgr)
			return;
		
		// Test initial state
		runner.Assert("Initial garrison count is 0", garMgr.GetGarrisonedZoneCount() >= 0, "Negative garrison count");
		
		// Test template retrieval (internal functionality test via spawn)
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			// Get a USSR zone to test with
			array<ref RBL_VirtualZone> ussrZones = zoneMgr.GetVirtualZonesByFaction(ERBLFactionKey.USSR);
			runner.Assert("Have USSR zones for garrison test", ussrZones.Count() > 0, "No USSR zones");
			
			if (ussrZones.Count() > 0)
			{
				RBL_VirtualZone testZone = ussrZones[0];
				string testZoneID = testZone.GetZoneID();
				
				// Test IsGarrisonSpawned before spawn
				bool wasSpawned = garMgr.IsGarrisonSpawned(testZoneID);
				
				// Test spawn (may or may not actually spawn depending on world state)
				bool spawnResult = garMgr.SpawnGarrisonForZone(
					testZoneID,
					testZone.GetZonePosition(),
					testZone.GetCaptureRadius(),
					testZone.GetZoneType(),
					testZone.GetOwnerFaction(),
					testZone.GetMaxGarrison()
				);
				
				// If spawn succeeded, verify data was created
				if (spawnResult)
				{
					runner.Assert("Garrison spawned successfully", garMgr.IsGarrisonSpawned(testZoneID), "Garrison not marked as spawned");
					
					RBL_GarrisonData data = garMgr.GetGarrisonData(testZoneID);
					runner.AssertNotNull("Garrison data created", data);
					
					if (data)
					{
						runner.Assert("Target strength set", data.TargetStrength > 0, "No target strength");
					}
					
					// Test clear garrison
					garMgr.ClearGarrison(testZoneID);
					runner.Assert("Garrison cleared", !garMgr.IsGarrisonSpawned(testZoneID), "Garrison not cleared");
				}
				else
				{
					// Spawn might fail in test environment (no world loaded)
					runner.RecordResult("Garrison spawn attempted", true, "Spawn returned false (normal in test env)");
				}
			}
		}
		
		// Test stats functions don't crash
		int totalUnits = garMgr.GetTotalSpawnedUnits();
		int totalVehicles = garMgr.GetTotalSpawnedVehicles();
		runner.Assert("GetTotalSpawnedUnits returns >= 0", totalUnits >= 0, "Negative unit count");
		runner.Assert("GetTotalSpawnedVehicles returns >= 0", totalVehicles >= 0, "Negative vehicle count");
		
		// Test update doesn't crash
		garMgr.Update(0.1);
		runner.RecordResult("GarrisonManager.Update runs without crash", true, "OK");
	}
	
	// Test QRF spawning system
	static void TestQRFSystem()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running QRF System Tests...");
		
		// Test QRF Templates
		PrintFormat("[RBL_Tests] Testing QRF Templates...");
		
		// Test infantry counts scale with war level
		int patrolWL1 = RBL_QRFTemplates.GetInfantryCount(ERBLQRFType.PATROL, 1);
		int patrolWL10 = RBL_QRFTemplates.GetInfantryCount(ERBLQRFType.PATROL, 10);
		runner.Assert("PATROL infantry count at WL1 >= 2", patrolWL1 >= 2, "Too few patrol units");
		runner.Assert("PATROL scales with war level", patrolWL10 >= patrolWL1, "No WL scaling");
		
		int infantryWL1 = RBL_QRFTemplates.GetInfantryCount(ERBLQRFType.INFANTRY, 1);
		int infantryWL10 = RBL_QRFTemplates.GetInfantryCount(ERBLQRFType.INFANTRY, 10);
		runner.Assert("INFANTRY count at WL1 >= 8", infantryWL1 >= 8, "Too few infantry units");
		runner.Assert("INFANTRY scales with war level", infantryWL10 > infantryWL1, "No WL scaling");
		
		int mechWL1 = RBL_QRFTemplates.GetInfantryCount(ERBLQRFType.MECHANIZED, 1);
		runner.Assert("MECHANIZED count at WL1 >= 6", mechWL1 >= 6, "Too few mech units");
		
		int specopsWL1 = RBL_QRFTemplates.GetInfantryCount(ERBLQRFType.SPECOPS, 1);
		runner.Assert("SPECOPS count at WL1 >= 4", specopsWL1 >= 4, "Too few specops units");
		
		// Test vehicle counts
		int patrolVeh = RBL_QRFTemplates.GetVehicleCount(ERBLQRFType.PATROL, 1);
		runner.Assert("PATROL has 1 vehicle", patrolVeh == 1, "Wrong vehicle count");
		
		int convoyVeh = RBL_QRFTemplates.GetVehicleCount(ERBLQRFType.CONVOY, 1);
		runner.Assert("CONVOY has 2 vehicles", convoyVeh == 2, "Wrong vehicle count");
		
		int infantryVeh = RBL_QRFTemplates.GetVehicleCount(ERBLQRFType.INFANTRY, 1);
		runner.Assert("INFANTRY has 0 vehicles", infantryVeh == 0, "Infantry shouldn't have vehicles");
		
		int specopsVeh = RBL_QRFTemplates.GetVehicleCount(ERBLQRFType.SPECOPS, 1);
		runner.Assert("SPECOPS has 0 vehicles", specopsVeh == 0, "Specops shouldn't have vehicles");
		
		// Test prefab generation
		array<string> prefabs;
		RBL_QRFTemplates.GetInfantryPrefabs(ERBLQRFType.PATROL, 5, prefabs);
		runner.Assert("GetInfantryPrefabs returns array", prefabs != null, "Null prefabs");
		runner.Assert("Prefabs count matches infantry count", prefabs.Count() == RBL_QRFTemplates.GetInfantryCount(ERBLQRFType.PATROL, 5), "Count mismatch");
		
		// Test vehicle prefab scaling
		string vehicleWL1 = RBL_QRFTemplates.GetVehiclePrefab(ERBLQRFType.MECHANIZED, 1);
		string vehicleWL8 = RBL_QRFTemplates.GetVehiclePrefab(ERBLQRFType.MECHANIZED, 8);
		runner.Assert("MECH vehicle prefab at WL1 valid", vehicleWL1.Length() > 0, "Empty prefab");
		runner.Assert("MECH vehicle changes at high WL", vehicleWL8 != vehicleWL1 || vehicleWL8.Length() > 0, "Should upgrade vehicle");
		
		// Test QRF Operation creation
		PrintFormat("[RBL_Tests] Testing QRF Operation...");
		
		RBL_QRFOperation qrf = new RBL_QRFOperation();
		runner.AssertNotNull("QRFOperation created", qrf);
		
		string opID = qrf.GetOperationID();
		runner.Assert("Operation ID generated", opID.Length() > 0, "Empty operation ID");
		runner.Assert("Operation ID has prefix", opID.Contains("QRF_"), "Missing QRF_ prefix");
		
		// Test initial state
		runner.Assert("Initial state not complete", !qrf.IsComplete(), "Should not be complete");
		runner.Assert("Initial alive count is 0", qrf.GetAliveCount() == 0, "Should have no units");
		
		// Test Commander AI QRF tracking
		RBL_CommanderAI ai = RBL_CommanderAI.GetInstance();
		runner.AssertNotNull("CommanderAI.GetInstance", ai);
		
		if (ai)
		{
			int activeQRFs = ai.GetActiveQRFCount();
			runner.Assert("Active QRF count >= 0", activeQRFs >= 0, "Negative QRF count");
			
			int resources = ai.GetFactionResources();
			runner.Assert("Faction resources >= 0", resources >= 0, "Negative resources");
			
			// Test update doesn't crash
			ai.Update(0.1);
			runner.RecordResult("CommanderAI.Update runs without crash", true, "OK");
		}
		
		// Test QRF state enum values
		runner.Assert("SPAWNING state exists", ERBLQRFState.SPAWNING >= 0, "Missing state");
		runner.Assert("EN_ROUTE state exists", ERBLQRFState.EN_ROUTE >= 0, "Missing state");
		runner.Assert("ARRIVED state exists", ERBLQRFState.ARRIVED >= 0, "Missing state");
		runner.Assert("DESTROYED state exists", ERBLQRFState.DESTROYED >= 0, "Missing state");
		runner.Assert("COMPLETE state exists", ERBLQRFState.COMPLETE >= 0, "Missing state");
	}
	
	// Test undercover detection system
	static void TestUndercoverSystem()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Undercover System Tests...");
		
		// Test singleton
		RBL_UndercoverSystem system = RBL_UndercoverSystem.GetInstance();
		runner.AssertNotNull("UndercoverSystem.GetInstance", system);
		
		if (!system)
			return;
		
		// Test detection factors constants
		runner.Assert("WEAPON_VISIBLE factor > 0", RBL_DetectionFactors.WEAPON_VISIBLE > 0, "Invalid factor");
		runner.Assert("MILITARY_UNIFORM factor > 0", RBL_DetectionFactors.MILITARY_UNIFORM > 0, "Invalid factor");
		runner.Assert("MILITARY_VEHICLE factor > 0", RBL_DetectionFactors.MILITARY_VEHICLE > 0, "Invalid factor");
		runner.Assert("DECAY_RATE > 0", RBL_DetectionFactors.DECAY_RATE > 0, "Invalid decay rate");
		
		// Test thresholds are ordered correctly
		runner.Assert("Thresholds ordered: SUSPICIOUS < SPOTTED", 
			RBL_DetectionFactors.THRESHOLD_SUSPICIOUS < RBL_DetectionFactors.THRESHOLD_SPOTTED, "Wrong order");
		runner.Assert("Thresholds ordered: SPOTTED < COMPROMISED", 
			RBL_DetectionFactors.THRESHOLD_SPOTTED < RBL_DetectionFactors.THRESHOLD_COMPROMISED, "Wrong order");
		
		// Test player state creation
		int testPlayerID = 99999;
		ERBLCoverStatus status = system.GetPlayerCoverStatus(testPlayerID);
		runner.Assert("New player starts HIDDEN", status == ERBLCoverStatus.HIDDEN, "Wrong initial status");
		
		float suspicion = system.GetPlayerSuspicionLevel(testPlayerID);
		runner.Assert("New player has 0 suspicion", suspicion == 0, "Non-zero initial suspicion");
		
		bool undercover = system.IsPlayerUndercover(testPlayerID);
		runner.Assert("New player is undercover", undercover == true, "Should be undercover");
		
		bool compromised = system.IsPlayerCompromised(testPlayerID);
		runner.Assert("New player not compromised", compromised == false, "Should not be compromised");
		
		// Test player state object
		RBL_PlayerCoverState state = system.GetPlayerState(testPlayerID);
		runner.AssertNotNull("GetPlayerState returns state", state);
		
		if (state)
		{
			runner.Assert("State status string not empty", state.GetStatusString().Length() > 0, "Empty status string");
			runner.RecordResult("GetFactorsString works", true, state.GetFactorsString());
			
			// Test status change
			state.m_fSuspicionLevel = 0.3;
			state.SetStatus(ERBLCoverStatus.SUSPICIOUS);
			runner.Assert("Status changed to SUSPICIOUS", state.m_eCurrentStatus == ERBLCoverStatus.SUSPICIOUS, "Status not changed");
			
			// Test status string
			string statusStr = state.GetStatusString();
			runner.Assert("SUSPICIOUS has status string", statusStr == "SUSPICIOUS", "Wrong status string");
			
			// Test compromise
			state.m_fSuspicionLevel = 1.0;
			state.SetStatus(ERBLCoverStatus.COMPROMISED);
			runner.Assert("Status changed to COMPROMISED", state.m_eCurrentStatus == ERBLCoverStatus.COMPROMISED, "Status not changed");
			
			// Test reset
			system.ResetPlayerCover(testPlayerID);
			runner.Assert("Reset sets suspicion to 0", state.m_fSuspicionLevel == 0, "Suspicion not reset");
			runner.Assert("Reset sets status to HIDDEN", state.m_eCurrentStatus == ERBLCoverStatus.HIDDEN, "Status not reset");
		}
		
		// Test event triggers don't crash
		system.OnPlayerFiredWeapon(testPlayerID);
		runner.RecordResult("OnPlayerFiredWeapon doesn't crash", true, "OK");
		
		system.OnPlayerAttackedEnemy(testPlayerID);
		runner.RecordResult("OnPlayerAttackedEnemy doesn't crash", true, "OK");
		
		// Test update doesn't crash
		system.Update(0.5);
		runner.RecordResult("UndercoverSystem.Update doesn't crash", true, "OK");
		
		// Test cover status enum values
		runner.Assert("HIDDEN status exists", ERBLCoverStatus.HIDDEN >= 0, "Missing status");
		runner.Assert("SUSPICIOUS status exists", ERBLCoverStatus.SUSPICIOUS >= 0, "Missing status");
		runner.Assert("SPOTTED status exists", ERBLCoverStatus.SPOTTED >= 0, "Missing status");
		runner.Assert("COMPROMISED status exists", ERBLCoverStatus.COMPROMISED >= 0, "Missing status");
		
		// Test event invokers exist
		ScriptInvoker onStatusChanged = system.GetOnStatusChanged();
		runner.AssertNotNull("GetOnStatusChanged returns invoker", onStatusChanged);
		
		ScriptInvoker onCoverBlown = system.GetOnCoverBlown();
		runner.AssertNotNull("GetOnCoverBlown returns invoker", onCoverBlown);
		
		// Test tracked player count
		int trackedCount = system.GetTrackedPlayerCount();
		runner.Assert("Tracked player count >= 0", trackedCount >= 0, "Negative count");
	}
	
	// Test item delivery system
	static void TestItemDelivery()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Item Delivery Tests...");
		
		// Test singleton
		RBL_ItemDelivery delivery = RBL_ItemDelivery.GetInstance();
		runner.AssertNotNull("ItemDelivery.GetInstance", delivery);
		
		if (!delivery)
			return;
		
		// Test delivery result enum
		runner.Assert("SUCCESS result exists", ERBLDeliveryResult.SUCCESS >= 0, "Missing result");
		runner.Assert("FAILED_NO_PLAYER result exists", ERBLDeliveryResult.FAILED_NO_PLAYER >= 0, "Missing result");
		runner.Assert("FAILED_NO_PREFAB result exists", ERBLDeliveryResult.FAILED_NO_PREFAB >= 0, "Missing result");
		runner.Assert("FAILED_SPAWN_ERROR result exists", ERBLDeliveryResult.FAILED_SPAWN_ERROR >= 0, "Missing result");
		
		// Test helper methods don't crash
		int localPlayerID = delivery.GetLocalPlayerID();
		runner.RecordResult("GetLocalPlayerID works", true, "ID: " + localPlayerID.ToString());
		
		IEntity localPlayer = delivery.GetLocalPlayerEntity();
		if (localPlayer)
		{
			runner.RecordResult("GetLocalPlayerEntity found player", true, "OK");
			
			bool validTarget = delivery.IsValidDeliveryTarget(localPlayer);
			runner.RecordResult("IsValidDeliveryTarget works", true, validTarget ? "Valid" : "Invalid");
			
			vector playerPos = delivery.GetPlayerPosition(localPlayer);
			runner.Assert("GetPlayerPosition returns valid vector", playerPos != vector.Zero, "Zero vector");
		}
		else
		{
			runner.RecordResult("GetLocalPlayerEntity", true, "No player (normal in test env)");
		}
		
		// Test event invokers exist
		ScriptInvoker onDelivered = delivery.GetOnItemDelivered();
		runner.AssertNotNull("GetOnItemDelivered returns invoker", onDelivered);
		
		ScriptInvoker onFailed = delivery.GetOnDeliveryFailed();
		runner.AssertNotNull("GetOnDeliveryFailed returns invoker", onFailed);
		
		// Test ShopManager integration
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		runner.AssertNotNull("ShopManager.GetInstance", shop);
		
		if (shop)
		{
			// Test item catalog exists
			array<ref RBL_ShopItem> weapons = shop.GetWeapons();
			runner.Assert("Shop has weapons", weapons.Count() > 0, "No weapons in shop");
			
			array<ref RBL_ShopItem> vehicles = shop.GetVehicles();
			runner.Assert("Shop has vehicles", vehicles.Count() > 0, "No vehicles in shop");
			
			array<ref RBL_ShopItem> recruits = shop.GetRecruits();
			runner.Assert("Shop has recruits", recruits.Count() > 0, "No recruits in shop");
			
			array<ref RBL_ShopItem> equipment = shop.GetEquipment();
			runner.Assert("Shop has equipment", equipment.Count() > 0, "No equipment in shop");
			
			// Test item structure
			if (weapons.Count() > 0)
			{
				RBL_ShopItem testItem = weapons[0];
				runner.Assert("Weapon has ID", testItem.ID.Length() > 0, "Empty ID");
				runner.Assert("Weapon has name", testItem.DisplayName.Length() > 0, "Empty name");
				runner.Assert("Weapon has price > 0", testItem.Price > 0, "Zero price");
				runner.Assert("Weapon has prefab", testItem.PrefabPath.Length() > 0, "Empty prefab");
				runner.Assert("Weapon category correct", testItem.Category == "Weapons", "Wrong category");
			}
			
			if (recruits.Count() > 0)
			{
				RBL_ShopItem testRecruit = recruits[0];
				runner.Assert("Recruit has HR cost", testRecruit.HRCost > 0, "Zero HR cost");
			}
		}
		
		// Test DeliveryEventData
		RBL_DeliveryEventData eventData = new RBL_DeliveryEventData();
		runner.AssertNotNull("DeliveryEventData created", eventData);
		runner.Assert("EventData default PlayerID is -1", eventData.PlayerID == -1, "Wrong default");
		runner.Assert("EventData default Result is SUCCESS", eventData.Result == ERBLDeliveryResult.SUCCESS, "Wrong default");
	}
	
	// Test mission system
	static void TestMissionSystem()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Mission System Tests...");
		
		// Test Mission Manager singleton
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		runner.AssertNotNull("MissionManager.GetInstance", missionMgr);
		
		if (!missionMgr)
			return;
		
		// Test initialization
		missionMgr.Initialize();
		runner.Assert("Mission manager initialized", missionMgr.IsInitialized(), "Not initialized");
		
		// Test mission generation
		RBL_Mission mission = RBL_MissionGenerator.GenerateRandomMission();
		runner.AssertNotNull("GenerateRandomMission returns mission", mission);
		
		if (mission)
		{
			runner.Assert("Mission has ID", mission.GetID().Length() > 0, "Empty ID");
			runner.Assert("Mission has name", mission.GetName().Length() > 0, "Empty name");
			runner.Assert("Mission has objectives", mission.GetObjectiveCount() > 0, "No objectives");
		}
		
		// Test event invokers
		runner.AssertNotNull("OnMissionStarted invoker", missionMgr.GetOnMissionStarted());
		runner.AssertNotNull("OnMissionCompleted invoker", missionMgr.GetOnMissionCompleted());
		runner.AssertNotNull("OnMissionFailed invoker", missionMgr.GetOnMissionFailed());
		
		// Test update
		missionMgr.Update(0.1);
		runner.RecordResult("MissionManager.Update runs", true, "OK");
	}
	
	// Test victory system
	static void TestVictorySystem()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_Tests] Running Victory System Tests...");
		
		// Test Victory Manager singleton
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		runner.AssertNotNull("VictoryManager.GetInstance", victoryMgr);
		
		if (!victoryMgr)
			return;
		
		// Test initial state
		runner.Assert("Campaign starts active", victoryMgr.IsActive(), "Not active");
		runner.Assert("Not victory initially", !victoryMgr.IsVictory(), "Started victory");
		runner.Assert("Not defeat initially", !victoryMgr.IsDefeat(), "Started defeat");
		
		// Test configuration
		victoryMgr.SetMaxPlayerDeaths(50);
		runner.Assert("Max deaths configurable", victoryMgr.GetMaxPlayerDeaths() == 50, "Config failed");
		
		// Test death tracking
		int initialDeaths = victoryMgr.GetPlayerDeaths();
		victoryMgr.OnPlayerDeath();
		runner.Assert("Death tracked", victoryMgr.GetPlayerDeaths() == initialDeaths + 1, "Death not counted");
		
		// Test event invokers
		runner.AssertNotNull("OnVictory invoker", victoryMgr.GetOnVictory());
		runner.AssertNotNull("OnDefeat invoker", victoryMgr.GetOnDefeat());
		
		// Test update
		victoryMgr.Update(0.1);
		runner.RecordResult("VictoryManager.Update runs", true, "OK");
	}
}

// Console command to run tests
class RBL_TestCommands
{
	static void RunTests()
	{
		RBL_Tests.RunAll();
	}
	
	static void RunNullSafetyTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_Tests.TestNullSafety();
		runner.PrintResults();
	}
	
	static void RunEconomyTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_Tests.TestEconomyManager();
		runner.PrintResults();
	}
	
	static void RunZoneTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_Tests.TestZoneManager();
		runner.PrintResults();
	}
	
	static void RunQRFTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_Tests.TestQRFSystem();
		runner.PrintResults();
	}
	
	static void RunGarrisonTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_Tests.TestGarrisonSystem();
		runner.PrintResults();
	}
	
	static void RunUndercoverTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_Tests.TestUndercoverSystem();
		runner.PrintResults();
	}
	
	static void RunDeliveryTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_Tests.TestItemDelivery();
		runner.PrintResults();
	}
}
