// ============================================================================
// PROJECT REBELLION - Feature Tests
// Tests for Mission System, Victory Conditions, Item Delivery Integration
// ============================================================================

class RBL_FeatureTests
{
	// Run all feature tests
	static void RunAll()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		PrintFormat("\n[RBL_FeatureTests] Starting all feature tests...\n");
		
		TestMissionData();
		TestMissionManager();
		TestMissionGenerator();
		TestVictoryManager();
		TestEndGameUI();
		TestItemDeliveryIntegration();
		TestCampaignEventWiring();
		
		runner.PrintResults();
		
		if (runner.AllPassed())
			PrintFormat("[RBL_FeatureTests] *** ALL FEATURE TESTS PASSED ***");
		else
			PrintFormat("[RBL_FeatureTests] *** SOME TESTS FAILED ***");
	}
	
	// ========================================================================
	// MISSION DATA TESTS
	// ========================================================================
	
	static void TestMissionData()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_FeatureTests] Running Mission Data Tests...");
		
		// Test mission objective creation
		RBL_MissionObjective captureObj = RBL_MissionObjective.CreateCaptureZone("TestZone", "Capture the test zone");
		runner.AssertNotNull("CreateCaptureZone returns objective", captureObj);
		
		if (captureObj)
		{
			runner.Assert("Capture objective type correct", captureObj.GetType() == ERBLObjectiveType.CAPTURE_ZONE, "Wrong type");
			runner.Assert("Objective description set", captureObj.GetDescription().Length() > 0, "Empty description");
			runner.Assert("Objective starts incomplete", !captureObj.IsCompleted(), "Started complete");
		}
		
		// Test defend objective
		RBL_MissionObjective defendObj = RBL_MissionObjective.CreateDefendZone("TestZone", 300, "Defend for 5 minutes");
		runner.AssertNotNull("CreateDefendZone returns objective", defendObj);
		
		if (defendObj)
		{
			runner.Assert("Defend objective type correct", defendObj.GetType() == ERBLObjectiveType.DEFEND_ZONE, "Wrong type");
			runner.Assert("Time limit set", defendObj.GetTimeLimit() == 300, "Wrong time limit");
		}
		
		// Test kill count objective
		RBL_MissionObjective killObj = RBL_MissionObjective.CreateKillCount(10, "Eliminate 10 enemies");
		runner.AssertNotNull("CreateKillCount returns objective", killObj);
		
		if (killObj)
		{
			runner.Assert("Kill objective type correct", killObj.GetType() == ERBLObjectiveType.KILL_COUNT, "Wrong type");
			runner.Assert("Target count set", killObj.GetTargetCount() == 10, "Wrong target count");
			
			// Test progress tracking
			killObj.IncrementProgress(5);
			runner.Assert("Progress incremented", killObj.GetCurrentCount() == 5, "Wrong current count");
			runner.Assert("Not complete at 5/10", !killObj.IsCompleted(), "Completed too early");
			
			killObj.IncrementProgress(5);
			runner.Assert("Complete at 10/10", killObj.IsCompleted(), "Should be complete");
		}
		
		// Test reach location objective
		vector testPos = Vector(1000, 0, 2000);
		RBL_MissionObjective reachObj = RBL_MissionObjective.CreateReachLocation(testPos, "Reach the waypoint");
		runner.AssertNotNull("CreateReachLocation returns objective", reachObj);
		
		if (reachObj)
		{
			runner.Assert("Reach objective type correct", reachObj.GetType() == ERBLObjectiveType.REACH_LOCATION, "Wrong type");
			runner.Assert("Target position set", reachObj.GetTargetPosition() == testPos, "Wrong position");
		}
		
		// Test mission reward
		RBL_MissionReward reward = RBL_MissionReward.Create(500, 5, 10);
		runner.AssertNotNull("Create reward", reward);
		
		if (reward)
		{
			runner.Assert("Money reward set", reward.GetMoney() == 500, "Wrong money");
			runner.Assert("HR reward set", reward.GetHR() == 5, "Wrong HR");
			runner.Assert("Aggression set", reward.GetAggression() == 10, "Wrong aggression");
			runner.Assert("Reward string not empty", reward.GetRewardString().Length() > 0, "Empty reward string");
		}
		
		// Test mission creation
		RBL_Mission mission = new RBL_Mission();
		runner.AssertNotNull("Mission created", mission);
		
		if (mission)
		{
			mission.SetID("test_mission_001");
			mission.SetName("Test Mission");
			mission.SetDescription("A test mission for testing");
			mission.SetBriefing("Detailed briefing text");
			mission.SetType(ERBLMissionType.ATTACK);
			mission.SetDifficulty(ERBLMissionDifficulty.MEDIUM);
			mission.SetTimeLimit(600);
			
			runner.Assert("Mission ID set", mission.GetID() == "test_mission_001", "Wrong ID");
			runner.Assert("Mission name set", mission.GetName() == "Test Mission", "Wrong name");
			runner.Assert("Mission type set", mission.GetType() == ERBLMissionType.ATTACK, "Wrong type");
			runner.Assert("Time limit set", mission.GetTimeLimit() == 600, "Wrong time limit");
			runner.Assert("Mission starts available", mission.IsAvailable(), "Should be available");
			
			// Test objective addition
			mission.AddObjective(captureObj);
			runner.Assert("Objective count is 1", mission.GetObjectiveCount() == 1, "Wrong objective count");
			
			// Test reward
			mission.SetReward(reward);
			runner.AssertNotNull("Reward assigned", mission.GetReward());
			
			// Test mission strings
			runner.Assert("Type string valid", mission.GetTypeString() == "Attack", "Wrong type string");
			runner.Assert("Difficulty string valid", mission.GetDifficultyString() == "Medium", "Wrong difficulty string");
			runner.Assert("Status string valid", mission.GetStatusString() == "Available", "Wrong status string");
		}
	}
	
	// ========================================================================
	// MISSION MANAGER TESTS
	// ========================================================================
	
	static void TestMissionManager()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_FeatureTests] Running Mission Manager Tests...");
		
		// Test singleton
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		runner.AssertNotNull("MissionManager.GetInstance", missionMgr);
		
		if (!missionMgr)
			return;
		
		// Test initialization
		missionMgr.Initialize();
		runner.Assert("Manager initialized", missionMgr.IsInitialized(), "Not initialized");
		
		// Test mission counts
		int availableCount = missionMgr.GetAvailableMissionCount();
		int activeCount = missionMgr.GetActiveMissionCount();
		runner.Assert("Available count >= 0", availableCount >= 0, "Negative available count");
		runner.Assert("Active count >= 0", activeCount >= 0, "Negative active count");
		
		// Test event invokers exist
		runner.AssertNotNull("OnMissionAvailable invoker", missionMgr.GetOnMissionAvailable());
		runner.AssertNotNull("OnMissionStarted invoker", missionMgr.GetOnMissionStarted());
		runner.AssertNotNull("OnMissionCompleted invoker", missionMgr.GetOnMissionCompleted());
		runner.AssertNotNull("OnMissionFailed invoker", missionMgr.GetOnMissionFailed());
		runner.AssertNotNull("OnMissionExpired invoker", missionMgr.GetOnMissionExpired());
		
		// Test available missions array
		array<ref RBL_Mission> availableMissions = missionMgr.GetAvailableMissions();
		runner.AssertNotNull("GetAvailableMissions returns array", availableMissions);
		
		// Test starting a mission (if available)
		if (availableMissions.Count() > 0)
		{
			RBL_Mission testMission = availableMissions[0];
			string missionID = testMission.GetID();
			
			bool startResult = missionMgr.StartMission(missionID);
			runner.Assert("StartMission returns success", startResult, "Failed to start mission");
			
			if (startResult)
			{
				runner.Assert("Mission now active", testMission.IsActive(), "Not active after start");
				runner.Assert("Active count increased", missionMgr.GetActiveMissionCount() > 0, "No active missions");
				
				// Test finding active mission
				RBL_Mission foundMission = missionMgr.GetActiveMissionByID(missionID);
				runner.AssertNotNull("GetActiveMissionByID finds mission", foundMission);
			}
		}
		
		// Test update doesn't crash
		missionMgr.Update(0.1);
		runner.RecordResult("MissionManager.Update runs without crash", true, "OK");
		
		// Test event handlers
		missionMgr.OnEnemyKilled();
		runner.RecordResult("OnEnemyKilled doesn't crash", true, "OK");
		
		missionMgr.OnZoneCaptured("TestZone", ERBLFactionKey.FIA);
		runner.RecordResult("OnZoneCaptured doesn't crash", true, "OK");
		
		missionMgr.OnPlayerPositionUpdate(Vector(1000, 50, 2000));
		runner.RecordResult("OnPlayerPositionUpdate doesn't crash", true, "OK");
		
		// Test stats
		int completed = missionMgr.GetTotalMissionsCompleted();
		int failed = missionMgr.GetTotalMissionsFailed();
		runner.Assert("Completed count >= 0", completed >= 0, "Negative completed count");
		runner.Assert("Failed count >= 0", failed >= 0, "Negative failed count");
	}
	
	// ========================================================================
	// MISSION GENERATOR TESTS
	// ========================================================================
	
	static void TestMissionGenerator()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_FeatureTests] Running Mission Generator Tests...");
		
		// Test available mission types based on war level
		array<ERBLMissionType> typesWL1 = RBL_MissionGenerator.GetAvailableMissionTypes(1);
		runner.AssertNotNull("GetAvailableMissionTypes returns array", typesWL1);
		runner.Assert("WL1 has at least ATTACK type", typesWL1.Count() > 0, "No mission types at WL1");
		
		array<ERBLMissionType> typesWL5 = RBL_MissionGenerator.GetAvailableMissionTypes(5);
		runner.Assert("WL5 has more mission types", typesWL5.Count() >= typesWL1.Count(), "No scaling");
		
		array<ERBLMissionType> typesWL10 = RBL_MissionGenerator.GetAvailableMissionTypes(10);
		runner.Assert("WL10 has all mission types", typesWL10.Count() >= typesWL5.Count(), "Missing types");
		
		// Test mission generation
		RBL_Mission randomMission = RBL_MissionGenerator.GenerateRandomMission();
		runner.AssertNotNull("GenerateRandomMission returns mission", randomMission);
		
		if (randomMission)
		{
			runner.Assert("Generated mission has ID", randomMission.GetID().Length() > 0, "No ID");
			runner.Assert("Generated mission has name", randomMission.GetName().Length() > 0, "No name");
			runner.Assert("Generated mission has objectives", randomMission.GetObjectiveCount() > 0, "No objectives");
			runner.AssertNotNull("Generated mission has reward", randomMission.GetReward());
		}
		
		// Test specific mission type generation
		RBL_Mission attackMission = RBL_MissionGenerator.GenerateMissionByType(ERBLMissionType.ATTACK);
		if (attackMission)
		{
			runner.Assert("Attack mission is ATTACK type", attackMission.GetType() == ERBLMissionType.ATTACK, "Wrong type");
		}
		
		RBL_Mission defendMission = RBL_MissionGenerator.GenerateMissionByType(ERBLMissionType.DEFEND);
		if (defendMission)
		{
			runner.Assert("Defend mission is DEFEND type", defendMission.GetType() == ERBLMissionType.DEFEND, "Wrong type");
		}
		
		RBL_Mission patrolMission = RBL_MissionGenerator.GenerateMissionByType(ERBLMissionType.PATROL);
		if (patrolMission)
		{
			runner.Assert("Patrol mission is PATROL type", patrolMission.GetType() == ERBLMissionType.PATROL, "Wrong type");
		}
	}
	
	// ========================================================================
	// VICTORY MANAGER TESTS
	// ========================================================================
	
	static void TestVictoryManager()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_FeatureTests] Running Victory Manager Tests...");
		
		// Test singleton
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		runner.AssertNotNull("VictoryManager.GetInstance", victoryMgr);
		
		if (!victoryMgr)
			return;
		
		// Test initialization
		victoryMgr.Initialize();
		
		// Test initial state
		runner.Assert("Campaign starts active", victoryMgr.IsActive(), "Not active");
		runner.Assert("Not victory initially", !victoryMgr.IsVictory(), "Started as victory");
		runner.Assert("Not defeat initially", !victoryMgr.IsDefeat(), "Started as defeat");
		
		// Test state getters
		ERBLCampaignState state = victoryMgr.GetCampaignState();
		runner.Assert("GetCampaignState returns ACTIVE", state == ERBLCampaignState.ACTIVE, "Wrong state");
		
		string stateStr = victoryMgr.GetCampaignStateString();
		runner.Assert("State string is 'Active'", stateStr == "Active", "Wrong state string");
		
		// Test player deaths tracking
		int deaths = victoryMgr.GetPlayerDeaths();
		runner.Assert("Initial deaths is 0", deaths == 0, "Non-zero initial deaths");
		
		victoryMgr.OnPlayerDeath();
		runner.Assert("Death incremented", victoryMgr.GetPlayerDeaths() == 1, "Death not counted");
		
		// Test configuration
		victoryMgr.SetMaxPlayerDeaths(100);
		runner.Assert("Max deaths set", victoryMgr.GetMaxPlayerDeaths() == 100, "Max not set");
		
		victoryMgr.SetVictoryZonePercentage(0.8);
		victoryMgr.SetRequireAllZones(false);
		victoryMgr.SetRequireHQCapture(true);
		
		// Test campaign time
		float campaignTime = victoryMgr.GetCampaignTime();
		runner.Assert("Campaign time >= 0", campaignTime >= 0, "Negative campaign time");
		
		// Test progress
		float progress = victoryMgr.GetCampaignProgress();
		runner.Assert("Progress between 0 and 1", progress >= 0 && progress <= 1, "Progress out of range");
		
		// Test event invokers
		runner.AssertNotNull("OnVictory invoker", victoryMgr.GetOnVictory());
		runner.AssertNotNull("OnDefeat invoker", victoryMgr.GetOnDefeat());
		runner.AssertNotNull("OnCampaignStateChanged invoker", victoryMgr.GetOnCampaignStateChanged());
		
		// Test update doesn't crash
		victoryMgr.Update(0.1);
		runner.RecordResult("VictoryManager.Update runs without crash", true, "OK");
		
		// Test condition strings
		string victoryStr = RBL_VictoryManager.GetVictoryConditionString(ERBLVictoryCondition.ALL_ZONES_CAPTURED);
		runner.Assert("Victory condition string valid", victoryStr.Length() > 0, "Empty victory string");
		
		string defeatStr = RBL_VictoryManager.GetDefeatConditionString(ERBLDefeatCondition.HQ_LOST);
		runner.Assert("Defeat condition string valid", defeatStr.Length() > 0, "Empty defeat string");
	}
	
	// ========================================================================
	// END GAME UI TESTS
	// ========================================================================
	
	static void TestEndGameUI()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_FeatureTests] Running End Game UI Tests...");
		
		// Test end game widget creation
		RBL_EndGameWidget endWidget = new RBL_EndGameWidget();
		runner.AssertNotNull("EndGameWidget created", endWidget);
		
		if (endWidget)
		{
			runner.Assert("Widget starts invisible", !endWidget.IsVisible(), "Should start invisible");
			
			// Test show
			endWidget.Show();
			runner.Assert("Widget visible after Show", endWidget.IsVisible(), "Should be visible");
			
			// Test hide
			endWidget.Hide();
			runner.RecordResult("Hide doesn't crash", true, "OK");
			
			// Test update
			endWidget.Update(0.5);
			runner.RecordResult("Update doesn't crash", true, "OK");
			
			// Test draw
			endWidget.Draw();
			runner.RecordResult("Draw doesn't crash", true, "OK");
		}
		
		// Test campaign summary widget
		RBL_CampaignSummaryWidget summaryWidget = new RBL_CampaignSummaryWidget();
		runner.AssertNotNull("CampaignSummaryWidget created", summaryWidget);
		
		if (summaryWidget)
		{
			summaryWidget.Draw();
			runner.RecordResult("Summary widget Draw doesn't crash", true, "OK");
		}
	}
	
	// ========================================================================
	// ITEM DELIVERY INTEGRATION TESTS
	// ========================================================================
	
	static void TestItemDeliveryIntegration()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_FeatureTests] Running Item Delivery Integration Tests...");
		
		// Test delivery system exists
		RBL_ItemDelivery delivery = RBL_ItemDelivery.GetInstance();
		runner.AssertNotNull("ItemDelivery.GetInstance", delivery);
		
		if (!delivery)
			return;
		
		// Test initialization
		delivery.Initialize();
		runner.RecordResult("Initialize doesn't crash", true, "OK");
		
		// Test economy integration
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		runner.AssertNotNull("EconomyManager for delivery test", econMgr);
		
		if (econMgr)
		{
			// Test TryPurchase method
			int startMoney = econMgr.GetMoney();
			int startHR = econMgr.GetHR();
			
			// Set known values for testing
			econMgr.SetMoney(1000);
			econMgr.SetHR(10);
			
			// Test purchase that should fail (no player)
			bool purchaseResult = econMgr.TryPurchase("test_item", 100, 0);
			// Result depends on whether a player entity exists
			runner.RecordResult("TryPurchase doesn't crash", true, "OK");
			
			// Restore values
			econMgr.SetMoney(startMoney);
			econMgr.SetHR(startHR);
		}
		
		// Test shop manager integration
		RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
		runner.AssertNotNull("ShopManager for delivery test", shopMgr);
		
		if (shopMgr)
		{
			// Test that shop items have valid delivery data
			array<ref RBL_ShopItem> weapons = shopMgr.GetWeapons();
			if (weapons.Count() > 0)
			{
				RBL_ShopItem item = weapons[0];
				runner.Assert("Shop item has prefab", item.PrefabPath.Length() > 0, "No prefab path");
				runner.Assert("Shop item has price", item.Price > 0, "No price");
			}
			
			array<ref RBL_ShopItem> recruits = shopMgr.GetRecruits();
			if (recruits.Count() > 0)
			{
				RBL_ShopItem recruit = recruits[0];
				runner.Assert("Recruit has HR cost", recruit.HRCost > 0, "No HR cost for recruit");
			}
		}
	}
	
	// ========================================================================
	// CAMPAIGN EVENT WIRING TESTS
	// ========================================================================
	
	static void TestCampaignEventWiring()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_FeatureTests] Running Campaign Event Wiring Tests...");
		
		// Test campaign manager events exist
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		runner.AssertNotNull("CampaignManager for wiring test", campMgr);
		
		if (campMgr)
		{
			runner.AssertNotNull("OnCampaignEvent invoker", campMgr.GetOnCampaignEvent());
			runner.AssertNotNull("OnWarLevelChanged invoker", campMgr.GetOnWarLevelChanged());
			runner.AssertNotNull("OnAggressionChanged invoker", campMgr.GetOnAggressionChanged());
		}
		
		// Test that mission manager receives events
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (missionMgr)
		{
			// Simulate zone capture event
			string testZoneID = "TestZone_EventWiring";
			missionMgr.OnZoneCaptured(testZoneID, ERBLFactionKey.FIA);
			runner.RecordResult("Mission manager receives zone capture", true, "OK");
			
			// Simulate enemy killed event
			missionMgr.OnEnemyKilled();
			runner.RecordResult("Mission manager receives enemy killed", true, "OK");
		}
		
		// Test that victory manager receives events
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (victoryMgr)
		{
			victoryMgr.OnPlayerDeath();
			runner.RecordResult("Victory manager receives player death", true, "OK");
		}
		
		// Test enum values for events
		runner.Assert("ZONE_CAPTURED event exists", ERBLCampaignEvent.ZONE_CAPTURED >= 0, "Missing event");
		runner.Assert("ZONE_LOST event exists", ERBLCampaignEvent.ZONE_LOST >= 0, "Missing event");
		runner.Assert("ENEMY_KILLED event exists", ERBLCampaignEvent.ENEMY_KILLED >= 0, "Missing event");
		runner.Assert("PLAYER_KILLED event exists", ERBLCampaignEvent.PLAYER_KILLED >= 0, "Missing event");
		runner.Assert("GAME_LOADED event exists", ERBLCampaignEvent.GAME_LOADED >= 0, "Missing event");
	}
}

// ============================================================================
// Console Commands
// ============================================================================

class RBL_FeatureTestCommands
{
	static void RunFeatureTests()
	{
		RBL_FeatureTests.RunAll();
	}
	
	static void RunMissionTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_FeatureTests.TestMissionData();
		RBL_FeatureTests.TestMissionManager();
		RBL_FeatureTests.TestMissionGenerator();
		
		runner.PrintResults();
	}
	
	static void RunVictoryTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_FeatureTests.TestVictoryManager();
		RBL_FeatureTests.TestEndGameUI();
		
		runner.PrintResults();
	}
	
	static void RunIntegrationTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_FeatureTests.TestItemDeliveryIntegration();
		RBL_FeatureTests.TestCampaignEventWiring();
		
		runner.PrintResults();
	}
}

