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
		TestCommanderAI();
		
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
	
	// Test Commander AI
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
}
