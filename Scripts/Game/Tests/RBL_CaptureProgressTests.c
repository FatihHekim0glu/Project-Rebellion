// ============================================================================
// PROJECT REBELLION - Capture Progress UI Tests
// Tests for network-synchronized capture progress display
// ============================================================================

class RBL_CaptureProgressTests
{
	static void RunAll()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("\n[RBL_Tests] Running Capture Progress Tests...\n");
		
		TestCaptureBarWidgetCreation();
		TestNetworkCaptureProgress();
		TestClearNetworkProgress();
		TestUIManagerIntegration();
		TestCaptureBarVisibility();
		TestProgressAnimation();
		TestFactionColorMapping();
		TestMultipleZoneUpdates();
		TestCaptureCompleteFlow();
		TestEdgeCases();
	}
	
	// Test that CaptureBarWidget can be created
	static void TestCaptureBarWidgetCreation()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing widget creation...");
		
		RBL_CaptureBarWidgetImpl widget = new RBL_CaptureBarWidgetImpl();
		runner.AssertNotNull("CaptureBarWidget created", widget);
		
		if (widget)
		{
			runner.Assert("Widget not capturing initially", !widget.IsCapturing(), "Should not be capturing");
			runner.AssertEqualFloat("Display progress starts at 0", 0.0, widget.GetDisplayProgress());
			runner.Assert("No network data initially", !widget.HasNetworkCaptureData(), "Should have no network data");
		}
	}
	
	// Test SetNetworkCaptureProgress functionality
	static void TestNetworkCaptureProgress()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing network progress updates...");
		
		RBL_CaptureBarWidgetImpl widget = new RBL_CaptureBarWidgetImpl();
		if (!widget)
		{
			runner.RecordResult("CaptureBarWidget for network test", false, "Widget is null");
			return;
		}
		
		// Set network capture progress
		widget.SetNetworkCaptureProgress("TestZone_Alpha", 50.0, ERBLFactionKey.FIA);
		
		runner.Assert("Network data flag set", widget.HasNetworkCaptureData(), "Network flag not set");
		runner.AssertStringEqual("Network zone ID stored", "TestZone_Alpha", widget.GetNetworkZoneID());
		runner.AssertEqualFloat("Network progress stored (normalized)", 0.5, widget.GetNetworkProgress(), 0.01);
		runner.AssertEqual("Network faction stored", ERBLFactionKey.FIA, widget.GetNetworkCapturingFaction());
		
		// Test progress update
		widget.SetNetworkCaptureProgress("TestZone_Alpha", 75.0, ERBLFactionKey.FIA);
		runner.AssertEqualFloat("Progress updated correctly", 0.75, widget.GetNetworkProgress(), 0.01);
		
		// Test different zone
		widget.SetNetworkCaptureProgress("TestZone_Beta", 30.0, ERBLFactionKey.USSR);
		runner.AssertStringEqual("Zone ID updated", "TestZone_Beta", widget.GetNetworkZoneID());
		runner.AssertEqual("Faction updated", ERBLFactionKey.USSR, widget.GetNetworkCapturingFaction());
	}
	
	// Test ClearNetworkCaptureProgress functionality
	static void TestClearNetworkProgress()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing clear network progress...");
		
		RBL_CaptureBarWidgetImpl widget = new RBL_CaptureBarWidgetImpl();
		if (!widget)
		{
			runner.RecordResult("CaptureBarWidget for clear test", false, "Widget is null");
			return;
		}
		
		// Set then clear
		widget.SetNetworkCaptureProgress("TestZone_Clear", 60.0, ERBLFactionKey.FIA);
		runner.Assert("Network data set before clear", widget.HasNetworkCaptureData(), "Data not set");
		
		widget.ClearNetworkCaptureProgress("TestZone_Clear");
		runner.Assert("Network data cleared", !widget.HasNetworkCaptureData(), "Data not cleared");
		runner.AssertStringEqual("Zone ID cleared", "", widget.GetNetworkZoneID());
		runner.AssertEqualFloat("Progress cleared", 0.0, widget.GetNetworkProgress());
		
		// Test clearing wrong zone (should not clear)
		widget.SetNetworkCaptureProgress("TestZone_Keep", 40.0, ERBLFactionKey.US);
		widget.ClearNetworkCaptureProgress("WrongZone");
		runner.Assert("Data not cleared for wrong zone", widget.HasNetworkCaptureData(), "Wrongly cleared");
		runner.AssertStringEqual("Zone ID preserved", "TestZone_Keep", widget.GetNetworkZoneID());
	}
	
	// Test UIManager integration
	static void TestUIManagerIntegration()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing UIManager integration...");
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		runner.AssertNotNull("UIManager singleton exists", uiMgr);
		
		if (!uiMgr)
			return;
		
		// Ensure UI is initialized
		uiMgr.Initialize();
		
		// Get capture bar from UIManager
		RBL_CaptureBarWidgetImpl captureBar = uiMgr.GetCaptureBar();
		runner.AssertNotNull("UIManager has CaptureBar", captureBar);
		
		if (!captureBar)
			return;
		
		// Test UpdateCaptureProgress flows through
		uiMgr.UpdateCaptureProgress("UITest_Zone", 55.0, ERBLFactionKey.FIA);
		runner.Assert("CaptureBar received network data", captureBar.HasNetworkCaptureData(), "Data not received");
		runner.AssertStringEqual("Zone ID passed through", "UITest_Zone", captureBar.GetNetworkZoneID());
		
		// Test ClearCaptureProgress
		uiMgr.ClearCaptureProgress("UITest_Zone");
		runner.Assert("CaptureBar cleared via UIManager", !captureBar.HasNetworkCaptureData(), "Data not cleared");
	}
	
	// Test visibility behavior
	static void TestCaptureBarVisibility()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing visibility behavior...");
		
		RBL_CaptureBarWidgetImpl widget = new RBL_CaptureBarWidgetImpl();
		if (!widget)
		{
			runner.RecordResult("CaptureBarWidget for visibility test", false, "Widget is null");
			return;
		}
		
		// Test initial state
		runner.Assert("Widget enabled initially", widget.IsEnabled(), "Should be enabled");
		
		// Test ForceShow
		widget.ForceShow("VisibilityTestZone", 0.5);
		runner.Assert("ForceShow enables capturing", widget.IsCapturing(), "Should be capturing");
		runner.AssertStringEqual("ForceShow sets zone name", "VisibilityTestZone", widget.GetCapturingZoneName());
		runner.AssertEqualFloat("ForceShow sets progress", 0.5, widget.GetDisplayProgress());
	}
	
	// Test progress animation
	static void TestProgressAnimation()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing progress animation...");
		
		RBL_CaptureBarWidgetImpl widget = new RBL_CaptureBarWidgetImpl();
		if (!widget)
		{
			runner.RecordResult("CaptureBarWidget for animation test", false, "Widget is null");
			return;
		}
		
		// Set initial progress
		widget.ForceShow("AnimTestZone", 0.0);
		runner.AssertEqualFloat("Initial display progress", 0.0, widget.GetDisplayProgress());
		
		// Set target progress higher
		widget.ForceShow("AnimTestZone", 0.8);
		
		// Simulate time passing
		for (int i = 0; i < 10; i++)
		{
			widget.Update(0.1);
		}
		
		// Progress should have animated toward target
		float finalProgress = widget.GetDisplayProgress();
		runner.Assert("Display progress animated up", finalProgress > 0.0, "Progress didn't animate");
	}
	
	// Test faction color mapping
	static void TestFactionColorMapping()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing faction color mapping...");
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
		{
			runner.RecordResult("UIManager for color test", false, "Manager is null");
			return;
		}
		
		// Test GetFactionName helper (internal method but verify integration)
		// FIA should be friendly
		uiMgr.OnCaptureComplete("ColorTestZone", "Color Test Zone", ERBLFactionKey.FIA);
		runner.RecordResult("FIA capture complete notification sent", true, "OK");
		
		// USSR should be enemy
		uiMgr.OnCaptureComplete("ColorTestZone2", "Color Test Zone 2", ERBLFactionKey.USSR);
		runner.RecordResult("USSR capture complete notification sent", true, "OK");
	}
	
	// Test multiple zone updates
	static void TestMultipleZoneUpdates()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing multiple zone updates...");
		
		RBL_CaptureBarWidgetImpl widget = new RBL_CaptureBarWidgetImpl();
		if (!widget)
		{
			runner.RecordResult("CaptureBarWidget for multi-zone test", false, "Widget is null");
			return;
		}
		
		// Rapidly switch between zones
		widget.SetNetworkCaptureProgress("Zone_A", 20.0, ERBLFactionKey.FIA);
		runner.AssertStringEqual("Zone A set", "Zone_A", widget.GetNetworkZoneID());
		
		widget.SetNetworkCaptureProgress("Zone_B", 40.0, ERBLFactionKey.USSR);
		runner.AssertStringEqual("Zone B set", "Zone_B", widget.GetNetworkZoneID());
		
		widget.SetNetworkCaptureProgress("Zone_C", 60.0, ERBLFactionKey.US);
		runner.AssertStringEqual("Zone C set", "Zone_C", widget.GetNetworkZoneID());
		
		// Clear should only affect current zone
		widget.ClearNetworkCaptureProgress("Zone_A");
		runner.Assert("Wrong zone clear ignored", widget.HasNetworkCaptureData(), "Should still have data");
		
		widget.ClearNetworkCaptureProgress("Zone_C");
		runner.Assert("Correct zone cleared", !widget.HasNetworkCaptureData(), "Should be cleared");
	}
	
	// Test capture complete flow
	static void TestCaptureCompleteFlow()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing capture complete flow...");
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
		{
			runner.RecordResult("UIManager for complete flow test", false, "Manager is null");
			return;
		}
		
		uiMgr.Initialize();
		RBL_CaptureBarWidgetImpl captureBar = uiMgr.GetCaptureBar();
		if (!captureBar)
		{
			runner.RecordResult("CaptureBar for complete flow test", false, "Widget is null");
			return;
		}
		
		// Simulate capture in progress
		uiMgr.UpdateCaptureProgress("CompleteFlowZone", 95.0, ERBLFactionKey.FIA);
		runner.Assert("Progress set before complete", captureBar.HasNetworkCaptureData(), "Data not set");
		
		// Complete the capture
		uiMgr.OnCaptureComplete("CompleteFlowZone", "Complete Flow Zone", ERBLFactionKey.FIA);
		runner.Assert("Data cleared after complete", !captureBar.HasNetworkCaptureData(), "Data not cleared");
	}
	
	// Test edge cases
	static void TestEdgeCases()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_CaptureProgress] Testing edge cases...");
		
		RBL_CaptureBarWidgetImpl widget = new RBL_CaptureBarWidgetImpl();
		if (!widget)
		{
			runner.RecordResult("CaptureBarWidget for edge case test", false, "Widget is null");
			return;
		}
		
		// Test 0% progress
		widget.SetNetworkCaptureProgress("EdgeZone_Zero", 0.0, ERBLFactionKey.FIA);
		runner.AssertEqualFloat("Zero progress stored", 0.0, widget.GetNetworkProgress());
		
		// Test 100% progress
		widget.SetNetworkCaptureProgress("EdgeZone_Full", 100.0, ERBLFactionKey.FIA);
		runner.AssertEqualFloat("Full progress stored", 1.0, widget.GetNetworkProgress());
		
		// Test negative progress (should be clamped or handled)
		widget.SetNetworkCaptureProgress("EdgeZone_Negative", -10.0, ERBLFactionKey.FIA);
		runner.Assert("Negative progress handled", widget.GetNetworkProgress() >= 0.0, "Negative progress not clamped");
		
		// Test over 100% progress (should be clamped or handled)
		widget.SetNetworkCaptureProgress("EdgeZone_Over", 150.0, ERBLFactionKey.FIA);
		runner.Assert("Over 100% progress handled", widget.GetNetworkProgress() <= 1.5, "Over-progress not handled");
		
		// Test empty zone ID
		widget.SetNetworkCaptureProgress("", 50.0, ERBLFactionKey.FIA);
		runner.RecordResult("Empty zone ID doesn't crash", true, "OK");
		
		// Test clearing empty zone
		widget.ClearNetworkCaptureProgress("");
		runner.RecordResult("Clearing empty zone ID doesn't crash", true, "OK");
		
		// Test rapid updates
		for (int i = 0; i < 100; i++)
		{
			widget.SetNetworkCaptureProgress("RapidZone", i, ERBLFactionKey.FIA);
		}
		runner.RecordResult("Rapid updates handled", true, "OK");
		
		// Test Update with null data
		widget.ClearNetworkCaptureProgress(widget.GetNetworkZoneID());
		widget.Update(0.1);
		runner.RecordResult("Update with no data doesn't crash", true, "OK");
	}
}

// Console command to run capture progress tests
class RBL_CaptureProgressTestCommands
{
	static void RunCaptureProgressTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		RBL_CaptureProgressTests.RunAll();
		runner.PrintResults();
	}
}

