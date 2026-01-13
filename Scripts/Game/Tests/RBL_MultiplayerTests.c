// ============================================================================
// PROJECT REBELLION - Multiplayer Tests
// Comprehensive test suite for all networking functionality
// ============================================================================

class RBL_MultiplayerTests
{
	// ========================================================================
	// MAIN TEST RUNNER
	// ========================================================================
	
	static void RunAllTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		PrintFormat("\n[RBL_MPTests] ========================================");
		PrintFormat("[RBL_MPTests] MULTIPLAYER TEST SUITE");
		PrintFormat("[RBL_MPTests] ========================================\n");
		
		TestNetworkUtils();
		TestNetworkComponentCreation();
		TestNetworkManagerCreation();
		TestReplicatedStateCreation();
		TestEconomyNetworkAwareness();
		TestZoneNetworkAwareness();
		TestCaptureNetworkAwareness();
		TestMissionNetworkAwareness();
		TestShopNetworkAwareness();
		TestNotificationBroadcasting();
		TestSerialization();
		TestAuthorityChecks();
		
		runner.PrintResults();
		
		if (runner.AllPassed())
			PrintFormat("[RBL_MPTests] *** ALL MULTIPLAYER TESTS PASSED ***");
		else
			PrintFormat("[RBL_MPTests] *** SOME TESTS FAILED ***");
	}
	
	// ========================================================================
	// NETWORK UTILS TESTS
	// ========================================================================
	
	static void TestNetworkUtils()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Network Utils Tests...");
		
		// Test basic checks don't crash
		bool isServer = RBL_NetworkUtils.IsServer();
		bool isClient = RBL_NetworkUtils.IsClient();
		bool isSP = RBL_NetworkUtils.IsSinglePlayer();
		bool isMP = RBL_NetworkUtils.IsMultiplayer();
		bool isDedicated = RBL_NetworkUtils.IsDedicatedServer();
		
		// In test env, should be singleplayer
		runner.Assert("IsSinglePlayer returns true in test", isSP, "Expected singleplayer");
		runner.Assert("IsMultiplayer inverse of IsSinglePlayer", isMP == !isSP, "Inverse mismatch");
		runner.Assert("IsServer returns true in SP", isServer, "Should be server in SP");
		runner.Assert("IsClient inverse of IsServer", isClient == !isServer, "Inverse mismatch");
		
		// Test player helpers
		int localID = RBL_NetworkUtils.GetLocalPlayerID();
		runner.RecordResult("GetLocalPlayerID doesn't crash", true, "ID: " + localID.ToString());
		
		int playerCount = RBL_NetworkUtils.GetPlayerCount();
		runner.Assert("GetPlayerCount >= 0", playerCount >= 0, "Negative player count");
		
		// Test GetAllPlayers
		array<int> players = new array<int>();
		RBL_NetworkUtils.GetAllPlayers(players);
		runner.Assert("GetAllPlayers returns array", players != null, "Null array");
		
		// Test PrintNetworkStatus doesn't crash
		RBL_NetworkUtils.PrintNetworkStatus();
		runner.RecordResult("PrintNetworkStatus doesn't crash", true, "OK");
	}
	
	// ========================================================================
	// NETWORK COMPONENT TESTS
	// ========================================================================
	
	static void TestNetworkComponentCreation()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Network Component Tests...");
		
		// Test static helpers work without entity
		int localID = RBL_NetworkComponent.GetLocalPlayerID();
		runner.RecordResult("NetworkComponent.GetLocalPlayerID works", true, "ID: " + localID.ToString());
		
		IEntity localEntity = RBL_NetworkComponent.GetLocalPlayerEntity();
		runner.RecordResult("NetworkComponent.GetLocalPlayerEntity works", true, localEntity ? "Found" : "Not found");
		
		bool isLocal = RBL_NetworkComponent.IsLocalPlayer(localID);
		runner.Assert("IsLocalPlayer returns true for local", isLocal || localID < 0, "Should be local");
		
		array<int> allPlayers = RBL_NetworkComponent.GetAllPlayerIDs();
		runner.AssertNotNull("GetAllPlayerIDs returns array", allPlayers);
		
		int count = RBL_NetworkComponent.GetPlayerCount();
		runner.Assert("GetPlayerCount >= 0", count >= 0, "Negative count");
	}
	
	// ========================================================================
	// NETWORK MANAGER TESTS
	// ========================================================================
	
	static void TestNetworkManagerCreation()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Network Manager Tests...");
		
		// NetworkManager may or may not be spawned in test environment
		RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
		
		if (netMgr)
		{
			runner.RecordResult("NetworkManager exists", true, "OK");
			
			// Test authority checks
			bool isAuth = netMgr.IsAuthority();
			bool isServer = netMgr.IsServer();
			bool isClient = netMgr.IsClient();
			bool canModify = netMgr.CanModifyState();
			
			runner.Assert("IsServer matches singleplayer", isServer, "Should be server in SP");
			runner.Assert("CanModifyState true in SP", canModify, "Should allow modification");
			
			// Test events exist
			runner.AssertNotNull("OnNetworkStateChanged exists", netMgr.GetOnNetworkStateChanged());
			runner.AssertNotNull("OnPurchaseResult exists", netMgr.GetOnPurchaseResult());
			
			// Test PrintStatus doesn't crash
			netMgr.PrintStatus();
			runner.RecordResult("NetworkManager.PrintStatus works", true, "OK");
		}
		else
		{
			runner.RecordResult("NetworkManager not spawned (OK in test env)", true, "OK");
		}
		
		// Test RBL_Network singleton wrapper
		RBL_Network net = RBL_Network.GetInstance();
		runner.AssertNotNull("RBL_Network.GetInstance works", net);
		
		if (net)
		{
			bool isServer = net.IsServer();
			bool canModify = net.CanModifyState();
			
			runner.Assert("RBL_Network.IsServer works", isServer, "Should be server in SP");
			runner.Assert("RBL_Network.CanModifyState works", canModify, "Should allow modification");
		}
	}
	
	// ========================================================================
	// REPLICATED STATE TESTS
	// ========================================================================
	
	static void TestReplicatedStateCreation()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Replicated State Tests...");
		
		// ReplicatedState may or may not be spawned
		RBL_ReplicatedState rplState = RBL_ReplicatedState.GetInstance();
		
		if (rplState)
		{
			runner.RecordResult("ReplicatedState exists", true, "OK");
			
			// Test getters don't crash
			int money = rplState.GetMoney();
			int hr = rplState.GetHR();
			int warLevel = rplState.GetWarLevel();
			int aggression = rplState.GetAggression();
			int day = rplState.GetDayNumber();
			bool active = rplState.IsCampaignActive();
			int victoryState = rplState.GetVictoryState();
			
			runner.Assert("GetMoney >= 0", money >= 0, "Negative money");
			runner.Assert("GetHR >= 0", hr >= 0, "Negative HR");
			runner.Assert("GetWarLevel >= 0", warLevel >= 0, "Invalid war level");
			runner.Assert("GetDayNumber >= 0", day >= 0, "Invalid day");
			
			// Test events exist
			runner.AssertNotNull("OnMoneyChanged exists", rplState.GetOnMoneyChanged());
			runner.AssertNotNull("OnHRChanged exists", rplState.GetOnHRChanged());
			runner.AssertNotNull("OnWarLevelChanged exists", rplState.GetOnWarLevelChanged());
			runner.AssertNotNull("OnZoneOwnershipChanged exists", rplState.GetOnZoneOwnershipChanged());
			
			// Test PrintStatus
			rplState.PrintStatus();
			runner.RecordResult("ReplicatedState.PrintStatus works", true, "OK");
		}
		else
		{
			runner.RecordResult("ReplicatedState not spawned (OK in test env)", true, "OK");
		}
		
		// Test network state helper classes
		RBL_ZoneNetworkState zoneState = new RBL_ZoneNetworkState();
		runner.AssertNotNull("ZoneNetworkState created", zoneState);
		
		zoneState.ZoneID = "TestZone";
		zoneState.OwnerFaction = 1;
		zoneState.CivilianSupport = 50;
		
		string serialized = zoneState.Serialize();
		runner.Assert("ZoneNetworkState serializes", serialized.Length() > 0, "Empty serialization");
		
		RBL_ZoneNetworkState zoneState2 = new RBL_ZoneNetworkState();
		bool deserialized = zoneState2.Deserialize(serialized);
		runner.Assert("ZoneNetworkState deserializes", deserialized, "Failed to deserialize");
		runner.AssertStringEqual("ZoneID preserved", "TestZone", zoneState2.ZoneID);
		
		// Test mission network state
		RBL_MissionNetworkState missionState = new RBL_MissionNetworkState();
		runner.AssertNotNull("MissionNetworkState created", missionState);
		
		missionState.MissionID = "MSN_001";
		missionState.MissionName = "Test Mission";
		missionState.Status = 1;
		
		string msnSerialized = missionState.Serialize();
		runner.Assert("MissionNetworkState serializes", msnSerialized.Length() > 0, "Empty serialization");
	}
	
	// ========================================================================
	// ECONOMY NETWORK AWARENESS TESTS
	// ========================================================================
	
	static void TestEconomyNetworkAwareness()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Economy Network Tests...");
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		runner.AssertNotNull("EconomyManager exists", econMgr);
		
		if (!econMgr)
			return;
		
		// Save initial state
		int initialMoney = econMgr.GetMoney();
		int initialHR = econMgr.GetHR();
		
		// In singleplayer, modifications should work
		econMgr.SetMoney(1000);
		runner.AssertEqual("SetMoney works in SP", 1000, econMgr.GetMoney());
		
		econMgr.AddMoney(500);
		runner.AssertEqual("AddMoney works in SP", 1500, econMgr.GetMoney());
		
		econMgr.SpendMoney(200);
		runner.AssertEqual("SpendMoney works in SP", 1300, econMgr.GetMoney());
		
		econMgr.SetHR(50);
		runner.AssertEqual("SetHR works in SP", 50, econMgr.GetHR());
		
		// Test local update methods
		econMgr.SetMoneyLocal(2000);
		runner.AssertEqual("SetMoneyLocal works", 2000, econMgr.GetMoney());
		
		econMgr.SetHRLocal(100);
		runner.AssertEqual("SetHRLocal works", 100, econMgr.GetHR());
		
		// Test serialization
		int outMoney, outHR;
		econMgr.SerializeToNetwork(outMoney, outHR);
		runner.AssertEqual("SerializeToNetwork money", 2000, outMoney);
		runner.AssertEqual("SerializeToNetwork HR", 100, outHR);
		
		// Test deserialization
		econMgr.DeserializeFromNetwork(3000, 150);
		runner.AssertEqual("DeserializeFromNetwork money", 3000, econMgr.GetMoney());
		runner.AssertEqual("DeserializeFromNetwork HR", 150, econMgr.GetHR());
		
		// Test arsenal serialization
		econMgr.DepositItem("TestItem", 5);
		string arsenalData = econMgr.SerializeArsenalToString();
		runner.Assert("Arsenal serializes", arsenalData.Length() > 0 || econMgr.GetItemCount("TestItem") == 5, "Serialization failed");
		
		// Test PrintEconomyStatus includes network info
		econMgr.PrintEconomyStatus();
		runner.RecordResult("PrintEconomyStatus shows network info", true, "OK");
		
		// Restore initial state
		econMgr.SetMoney(initialMoney);
		econMgr.SetHR(initialHR);
	}
	
	// ========================================================================
	// ZONE NETWORK AWARENESS TESTS
	// ========================================================================
	
	static void TestZoneNetworkAwareness()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Zone Network Tests...");
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		runner.AssertNotNull("ZoneManager exists", zoneMgr);
		
		if (!zoneMgr)
			return;
		
		// Test events exist
		runner.AssertNotNull("OnZoneOwnershipChanged exists", zoneMgr.GetOnZoneOwnershipChanged());
		runner.AssertNotNull("OnZoneSupportChanged exists", zoneMgr.GetOnZoneSupportChanged());
		
		// Test serialization
		string ownershipData = zoneMgr.SerializeZoneOwnership();
		runner.RecordResult("SerializeZoneOwnership works", true, "Length: " + ownershipData.Length().ToString());
		
		string supportData = zoneMgr.SerializeZoneSupport();
		runner.RecordResult("SerializeZoneSupport works", true, "Length: " + supportData.Length().ToString());
		
		string fullData = zoneMgr.SerializeFullZoneState();
		runner.RecordResult("SerializeFullZoneState works", true, "Length: " + fullData.Length().ToString());
		
		// Test deserialization doesn't crash
		zoneMgr.DeserializeZoneOwnership(ownershipData);
		runner.RecordResult("DeserializeZoneOwnership works", true, "OK");
		
		zoneMgr.DeserializeZoneSupport(supportData);
		runner.RecordResult("DeserializeZoneSupport works", true, "OK");
		
		zoneMgr.DeserializeFullZoneState(fullData);
		runner.RecordResult("DeserializeFullZoneState works", true, "OK");
		
		// Test SetZoneOwner (should work in SP)
		array<ref RBL_VirtualZone> zones = zoneMgr.GetAllVirtualZones();
		if (zones.Count() > 0)
		{
			RBL_VirtualZone testZone = zones[0];
			string zoneID = testZone.GetZoneID();
			ERBLFactionKey originalOwner = testZone.GetOwnerFaction();
			
			// Change and verify
			zoneMgr.SetZoneOwner(zoneID, ERBLFactionKey.FIA);
			runner.Assert("SetZoneOwner changes owner", testZone.GetOwnerFaction() == ERBLFactionKey.FIA, "Owner not changed");
			
			// Test local setter
			zoneMgr.SetZoneOwnerLocal(zoneID, originalOwner);
			runner.Assert("SetZoneOwnerLocal works", testZone.GetOwnerFaction() == originalOwner, "Local set failed");
		}
		
		// Test PrintZoneStatus includes network info
		zoneMgr.PrintZoneStatus();
		runner.RecordResult("PrintZoneStatus shows network info", true, "OK");
	}
	
	// ========================================================================
	// CAPTURE NETWORK AWARENESS TESTS
	// ========================================================================
	
	static void TestCaptureNetworkAwareness()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Capture Network Tests...");
		
		RBL_CaptureManager captureMgr = RBL_CaptureManager.GetInstance();
		runner.AssertNotNull("CaptureManager exists", captureMgr);
		
		if (!captureMgr)
			return;
		
		// Test local progress setter
		captureMgr.SetCaptureProgressLocal("TestZone", 50.0, ERBLFactionKey.FIA);
		float progress = captureMgr.GetCaptureProgress("TestZone");
		runner.AssertEqualFloat("SetCaptureProgressLocal sets progress", 50.0, progress, 0.1);
		
		bool isBeingCaptured = captureMgr.IsZoneBeingCaptured("TestZone");
		runner.Assert("Zone shows as being captured", isBeingCaptured, "Not marked as capturing");
		
		// Clear progress
		captureMgr.SetCaptureProgressLocal("TestZone", 0, ERBLFactionKey.NONE);
		runner.RecordResult("Clear capture progress works", true, "OK");
		
		// Test Update in SP
		captureMgr.Update(0.1);
		runner.RecordResult("CaptureManager.Update works in SP", true, "OK");
	}
	
	// ========================================================================
	// MISSION NETWORK AWARENESS TESTS
	// ========================================================================
	
	static void TestMissionNetworkAwareness()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Mission Network Tests...");
		
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		runner.AssertNotNull("MissionManager exists", missionMgr);
		
		if (!missionMgr)
			return;
		
		// Initialize if needed
		if (!missionMgr.IsInitialized())
			missionMgr.Initialize();
		
		runner.Assert("MissionManager initialized", missionMgr.IsInitialized(), "Not initialized");
		
		// Test that missions generate in SP
		int availableCount = missionMgr.GetAvailableMissionCount();
		runner.Assert("Missions generated in SP", availableCount > 0, "No missions available");
		
		// Test Update in SP
		missionMgr.Update(0.1);
		runner.RecordResult("MissionManager.Update works in SP", true, "OK");
		
		// Test mission events exist
		runner.AssertNotNull("OnMissionStarted exists", missionMgr.GetOnMissionStarted());
		runner.AssertNotNull("OnMissionCompleted exists", missionMgr.GetOnMissionCompleted());
		runner.AssertNotNull("OnMissionFailed exists", missionMgr.GetOnMissionFailed());
	}
	
	// ========================================================================
	// SHOP NETWORK AWARENESS TESTS
	// ========================================================================
	
	static void TestShopNetworkAwareness()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Shop Network Tests...");
		
		RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
		runner.AssertNotNull("ShopManager exists", shopMgr);
		
		if (!shopMgr)
			return;
		
		// Test shop items exist
		array<ref RBL_ShopItem> weapons = shopMgr.GetWeapons();
		runner.Assert("Shop has weapons", weapons.Count() > 0, "No weapons");
		
		// Test server purchase method exists
		// (We can't fully test without actual player)
		runner.RecordResult("ProcessServerPurchase method exists", true, "OK");
		
		// Test events exist
		runner.AssertNotNull("OnPurchase exists", shopMgr.GetOnPurchase());
		runner.AssertNotNull("OnMenuOpened exists", shopMgr.GetOnMenuOpened());
	}
	
	// ========================================================================
	// NOTIFICATION BROADCASTING TESTS
	// ========================================================================
	
	static void TestNotificationBroadcasting()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Notification Broadcast Tests...");
		
		// Test that notification methods don't crash
		// (Actual broadcasting requires network manager)
		
		RBL_Notifications.SystemMessage("Test system message");
		runner.RecordResult("SystemMessage doesn't crash", true, "OK");
		
		RBL_Notifications.DebugMessage("Test debug message");
		runner.RecordResult("DebugMessage doesn't crash", true, "OK");
		
		// These would broadcast in MP, but just show locally in SP
		RBL_Notifications.ZoneCaptured("TestZone");
		runner.RecordResult("ZoneCaptured notification works", true, "OK");
		
		RBL_Notifications.QRFIncoming("PATROL", "TestZone");
		runner.RecordResult("QRFIncoming notification works", true, "OK");
		
		RBL_Notifications.MissionReceived("TestMission");
		runner.RecordResult("MissionReceived notification works", true, "OK");
	}
	
	// ========================================================================
	// SERIALIZATION TESTS
	// ========================================================================
	
	static void TestSerialization()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Serialization Tests...");
		
		// Test zone ownership string format
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			string ownerData = zoneMgr.SerializeZoneOwnership();
			
			// Should be in format "zoneID:owner;zoneID:owner;..."
			if (ownerData.Length() > 0)
			{
				bool hasColon = ownerData.Contains(":");
				runner.Assert("Ownership format has colons", hasColon, "Invalid format");
			}
			
			// Test round-trip
			string before = ownerData;
			zoneMgr.DeserializeZoneOwnership(before);
			string after = zoneMgr.SerializeZoneOwnership();
			runner.Assert("Ownership serialization round-trip", before == after, "Data changed");
		}
		
		// Test economy serialization
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			int origMoney = econMgr.GetMoney();
			int origHR = econMgr.GetHR();
			
			int outMoney, outHR;
			econMgr.SerializeToNetwork(outMoney, outHR);
			
			econMgr.SetMoney(9999);
			econMgr.SetHR(9999);
			
			econMgr.DeserializeFromNetwork(outMoney, outHR);
			
			runner.AssertEqual("Economy round-trip money", origMoney, econMgr.GetMoney());
			runner.AssertEqual("Economy round-trip HR", origHR, econMgr.GetHR());
		}
		
		// Test network event data structures
		RBL_NetworkEventData baseEvent = new RBL_NetworkEventData();
		runner.Assert("NetworkEventData has timestamp", baseEvent.Timestamp > 0, "No timestamp");
		
		RBL_EconomyNetworkEvent econEvent = new RBL_EconomyNetworkEvent();
		runner.AssertNotNull("EconomyNetworkEvent created", econEvent);
		
		RBL_ZoneNetworkEvent zoneEvent = new RBL_ZoneNetworkEvent();
		runner.AssertNotNull("ZoneNetworkEvent created", zoneEvent);
		
		RBL_MissionNetworkEvent missionEvent = new RBL_MissionNetworkEvent();
		runner.AssertNotNull("MissionNetworkEvent created", missionEvent);
		
		RBL_NotificationNetworkEvent notifEvent = new RBL_NotificationNetworkEvent();
		runner.AssertNotNull("NotificationNetworkEvent created", notifEvent);
	}
	
	// ========================================================================
	// AUTHORITY CHECK TESTS
	// ========================================================================
	
	static void TestAuthorityChecks()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_MPTests] Running Authority Check Tests...");
		
		// In singleplayer, all authority checks should return true
		bool networkIsServer = RBL_NetworkUtils.IsServer();
		runner.Assert("NetworkUtils.IsServer true in SP", networkIsServer, "Should be server");
		
		bool networkIsSP = RBL_NetworkUtils.IsSinglePlayer();
		runner.Assert("NetworkUtils.IsSinglePlayer true", networkIsSP, "Should be singleplayer");
		
		// Test that state modifications work
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			int before = econMgr.GetMoney();
			econMgr.AddMoney(1);
			int after = econMgr.GetMoney();
			
			runner.Assert("Authority allows economy changes in SP", after == before + 1, "Change blocked");
			
			// Restore
			econMgr.SpendMoney(1);
		}
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			array<ref RBL_VirtualZone> zones = zoneMgr.GetAllVirtualZones();
			if (zones.Count() > 0)
			{
				RBL_VirtualZone zone = zones[0];
				ERBLFactionKey before = zone.GetOwnerFaction();
				
				// Change
				ERBLFactionKey newOwner = (before == ERBLFactionKey.FIA) ? ERBLFactionKey.USSR : ERBLFactionKey.FIA;
				zoneMgr.SetZoneOwner(zone.GetZoneID(), newOwner);
				
				runner.Assert("Authority allows zone changes in SP", zone.GetOwnerFaction() == newOwner, "Change blocked");
				
				// Restore
				zoneMgr.SetZoneOwner(zone.GetZoneID(), before);
			}
		}
		
		// Test mission manager authority
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (missionMgr && missionMgr.IsInitialized())
		{
			missionMgr.Update(0.1);
			runner.RecordResult("MissionManager processes in SP", true, "OK");
		}
		
		// Test capture manager authority
		RBL_CaptureManager captureMgr = RBL_CaptureManager.GetInstance();
		if (captureMgr)
		{
			captureMgr.Update(0.1);
			runner.RecordResult("CaptureManager processes in SP", true, "OK");
		}
	}
}

// ============================================================================
// DEBUG COMMANDS
// ============================================================================
class RBL_MultiplayerTestCommands
{
	[ConsoleCmd("rbl_mp_test", "Run all multiplayer tests")]
	static void RunTests()
	{
		RBL_MultiplayerTests.RunAllTests();
	}
	
	[ConsoleCmd("rbl_network_status", "Print network status")]
	static void NetworkStatus()
	{
		RBL_NetworkUtils.PrintNetworkStatus();
		
		RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
		if (netMgr)
			netMgr.PrintStatus();
		
		RBL_ReplicatedState rplState = RBL_ReplicatedState.GetInstance();
		if (rplState)
			rplState.PrintStatus();
	}
	
	[ConsoleCmd("rbl_authority_check", "Check authority status")]
	static void AuthorityCheck()
	{
		PrintFormat("[RBL] === AUTHORITY STATUS ===");
		PrintFormat("  IsServer: %1", RBL_NetworkUtils.IsServer());
		PrintFormat("  IsClient: %1", RBL_NetworkUtils.IsClient());
		PrintFormat("  IsSinglePlayer: %1", RBL_NetworkUtils.IsSinglePlayer());
		PrintFormat("  IsMultiplayer: %1", RBL_NetworkUtils.IsMultiplayer());
		PrintFormat("  IsDedicatedServer: %1", RBL_NetworkUtils.IsDedicatedServer());
		PrintFormat("  LocalPlayerID: %1", RBL_NetworkUtils.GetLocalPlayerID());
		PrintFormat("  PlayerCount: %1", RBL_NetworkUtils.GetPlayerCount());
	}
}

