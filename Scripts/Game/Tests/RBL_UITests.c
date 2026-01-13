// ============================================================================
// PROJECT REBELLION - UI Automated Tests
// Test suite for all UI components
// ============================================================================

class RBL_UITests
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
		
		PrintFormat("[RBL_UITests] ========================================");
		PrintFormat("[RBL_UITests] STARTING UI TEST SUITE");
		PrintFormat("[RBL_UITests] ========================================");
		
		// Run test groups
		Test_UIColors();
		Test_UISizes();
		Test_UIStrings();
		Test_UILayout();
		Test_BaseWidget();
		Test_Notifications();
		Test_ShopMenu();
		Test_MainHUD();
		Test_ZoneInfo();
		Test_CaptureBar();
		Test_Undercover();
		Test_UIManager();
		
		// Print results
		PrintFormat("[RBL_UITests] ========================================");
		PrintFormat("[RBL_UITests] TEST RESULTS");
		PrintFormat("[RBL_UITests] Passed: %1", s_iTestsPassed);
		PrintFormat("[RBL_UITests] Failed: %1", s_iTestsFailed);
		
		if (s_iTestsFailed > 0)
		{
			PrintFormat("[RBL_UITests] FAILED TESTS:");
			for (int i = 0; i < s_aFailedTests.Count(); i++)
			{
				PrintFormat("[RBL_UITests]   - %1", s_aFailedTests[i]);
			}
		}
		
		PrintFormat("[RBL_UITests] ========================================");
	}
	
	// ========================================================================
	// ASSERTION HELPERS
	// ========================================================================
	
	protected static void Assert(bool condition, string testName)
	{
		if (condition)
		{
			s_iTestsPassed++;
			PrintFormat("[RBL_UITests] PASS: %1", testName);
		}
		else
		{
			s_iTestsFailed++;
			s_aFailedTests.Insert(testName);
			PrintFormat("[RBL_UITests] FAIL: %1", testName);
		}
	}
	
	protected static void AssertEqual(int actual, int expected, string testName)
	{
		if (actual == expected)
		{
			s_iTestsPassed++;
			PrintFormat("[RBL_UITests] PASS: %1", testName);
		}
		else
		{
			s_iTestsFailed++;
			s_aFailedTests.Insert(testName + " (expected " + expected.ToString() + ", got " + actual.ToString() + ")");
			PrintFormat("[RBL_UITests] FAIL: %1 (expected %2, got %3)", testName, expected, actual);
		}
	}
	
	protected static void AssertEqualFloat(float actual, float expected, float tolerance, string testName)
	{
		if (Math.AbsFloat(actual - expected) <= tolerance)
		{
			s_iTestsPassed++;
			PrintFormat("[RBL_UITests] PASS: %1", testName);
		}
		else
		{
			s_iTestsFailed++;
			s_aFailedTests.Insert(testName);
			PrintFormat("[RBL_UITests] FAIL: %1 (expected %2, got %3)", testName, expected, actual);
		}
	}
	
	protected static void AssertNotNull(Class obj, string testName)
	{
		if (obj)
		{
			s_iTestsPassed++;
			PrintFormat("[RBL_UITests] PASS: %1", testName);
		}
		else
		{
			s_iTestsFailed++;
			s_aFailedTests.Insert(testName + " (null)");
			PrintFormat("[RBL_UITests] FAIL: %1 (object is null)", testName);
		}
	}
	
	protected static void AssertStringEqual(string actual, string expected, string testName)
	{
		if (actual == expected)
		{
			s_iTestsPassed++;
			PrintFormat("[RBL_UITests] PASS: %1", testName);
		}
		else
		{
			s_iTestsFailed++;
			s_aFailedTests.Insert(testName);
			PrintFormat("[RBL_UITests] FAIL: %1 (expected '%2', got '%3')", testName, expected, actual);
		}
	}
	
	// ========================================================================
	// TEST GROUPS
	// ========================================================================
	
	static void Test_UIColors()
	{
		PrintFormat("[RBL_UITests] --- Testing UI Colors ---");
		
		// Test faction colors
		Assert(RBL_UIColors.COLOR_FACTION_FIA != 0, "FIA color is defined");
		Assert(RBL_UIColors.COLOR_FACTION_USSR != 0, "USSR color is defined");
		Assert(RBL_UIColors.COLOR_FACTION_US != 0, "US color is defined");
		
		// Test GetFactionColor
		Assert(RBL_UIColors.GetFactionColor(ERBLFactionKey.FIA) == RBL_UIColors.COLOR_FACTION_FIA, "GetFactionColor(FIA)");
		Assert(RBL_UIColors.GetFactionColor(ERBLFactionKey.USSR) == RBL_UIColors.COLOR_FACTION_USSR, "GetFactionColor(USSR)");
		
		// Test status colors
		Assert(RBL_UIColors.COLOR_STATUS_HIDDEN != RBL_UIColors.COLOR_STATUS_COMPROMISED, "Status colors are distinct");
	}
	
	static void Test_UISizes()
	{
		PrintFormat("[RBL_UITests] --- Testing UI Sizes ---");
		
		// Test reference dimensions
		Assert(RBL_UISizes.REFERENCE_WIDTH > 0, "Reference width is positive");
		Assert(RBL_UISizes.REFERENCE_HEIGHT > 0, "Reference height is positive");
		
		// Test panel sizes
		Assert(RBL_UISizes.HUD_PANEL_WIDTH > 0, "HUD panel width is positive");
		Assert(RBL_UISizes.SHOP_WIDTH > RBL_UISizes.SHOP_HEIGHT * 0.5, "Shop aspect ratio is reasonable");
		
		// Test icon sizes
		Assert(RBL_UISizes.ICON_SMALL < RBL_UISizes.ICON_MEDIUM, "Icon small < medium");
		Assert(RBL_UISizes.ICON_MEDIUM < RBL_UISizes.ICON_LARGE, "Icon medium < large");
	}
	
	static void Test_UIStrings()
	{
		PrintFormat("[RBL_UITests] --- Testing UI Strings ---");
		
		// Test money formatting
		AssertStringEqual(RBL_UIStrings.FormatMoney(100), "$100", "FormatMoney(100)");
		AssertStringEqual(RBL_UIStrings.FormatMoney(0), "$0", "FormatMoney(0)");
		
		// Test HR formatting
		AssertStringEqual(RBL_UIStrings.FormatHR(5), "5 HR", "FormatHR(5)");
		
		// Test distance formatting
		AssertStringEqual(RBL_UIStrings.FormatDistance(500), "500m", "FormatDistance(500)");
		AssertStringEqual(RBL_UIStrings.FormatDistance(1500), "1.5km", "FormatDistance(1500)");
		
		// Test percentage formatting
		AssertStringEqual(RBL_UIStrings.FormatPercent(0.5), "50%", "FormatPercent(0.5)");
		AssertStringEqual(RBL_UIStrings.FormatPercent(1.0), "100%", "FormatPercent(1.0)");
		
		// Test time formatting
		AssertStringEqual(RBL_UIStrings.FormatTime(65), "1:05", "FormatTime(65)");
		AssertStringEqual(RBL_UIStrings.FormatTime(3600), "60:00", "FormatTime(3600)");
		
		// Test zone type names
		AssertStringEqual(RBL_UIStrings.GetZoneTypeName(ERBLZoneType.Town), "TOWN", "GetZoneTypeName(Town)");
		AssertStringEqual(RBL_UIStrings.GetZoneTypeName(ERBLZoneType.HQ), "HQ", "GetZoneTypeName(HQ)");
	}
	
	static void Test_UILayout()
	{
		PrintFormat("[RBL_UITests] --- Testing UI Layout ---");
		
		// Test corner positioning
		vector topLeft = RBL_UILayout.GetCornerPosition(0, 100, 50, 10);
		AssertEqualFloat(topLeft[0], 10, 0.01, "Top-left X position");
		AssertEqualFloat(topLeft[1], 10, 0.01, "Top-left Y position");
		
		vector topRight = RBL_UILayout.GetCornerPosition(1, 100, 50, 10);
		AssertEqualFloat(topRight[0], RBL_UISizes.REFERENCE_WIDTH - 110, 0.01, "Top-right X position");
		
		// Test centered positioning
		vector centered = RBL_UILayout.GetCenteredPosition(200, 100);
		AssertEqualFloat(centered[0], (RBL_UISizes.REFERENCE_WIDTH - 200) / 2, 0.01, "Centered X position");
		AssertEqualFloat(centered[1], (RBL_UISizes.REFERENCE_HEIGHT - 100) / 2, 0.01, "Centered Y position");
	}
	
	static void Test_BaseWidget()
	{
		PrintFormat("[RBL_UITests] --- Testing Base Widget ---");
		
		// Create test widget
		RBL_BaseWidget widget = new RBL_BaseWidget();
		
		Assert(widget.IsVisible(), "Widget default visible");
		Assert(widget.IsEnabled(), "Widget default enabled");
		AssertEqualFloat(widget.GetAlpha(), 1.0, 0.01, "Widget default alpha");
		
		// Test hide
		widget.Hide();
		// After animation, should fade to 0
		// (Can't test animation in sync test)
		
		// Test position
		widget.SetPosition(100, 200);
		widget.SetSize(300, 150);
		// Would need getters to verify
	}
	
	static void Test_Notifications()
	{
		PrintFormat("[RBL_UITests] --- Testing Notifications ---");
		
		// Create notification manager
		RBL_NotificationManagerImpl notifMgr = new RBL_NotificationManagerImpl();
		
		AssertEqual(notifMgr.GetActiveCount(), 0, "Initial notification count is 0");
		
		// Add notification
		notifMgr.ShowNotification("Test message", RBL_UIColors.COLOR_TEXT_BRIGHT, 5.0);
		AssertEqual(notifMgr.GetActiveCount(), 1, "Notification count after add is 1");
		
		// Add duplicate (should not create new)
		notifMgr.ShowNotification("Test message", RBL_UIColors.COLOR_TEXT_BRIGHT, 5.0);
		AssertEqual(notifMgr.GetActiveCount(), 1, "Duplicate notification not added");
		
		// Add different notification
		notifMgr.ShowNotification("Different message", RBL_UIColors.COLOR_ACCENT_GREEN, 5.0);
		AssertEqual(notifMgr.GetActiveCount(), 2, "Different notification added");
		
		// Clear all
		notifMgr.ClearAll();
		AssertEqual(notifMgr.GetActiveCount(), 0, "ClearAll removes all notifications");
	}
	
	static void Test_ShopMenu()
	{
		PrintFormat("[RBL_UITests] --- Testing Shop Menu ---");
		
		// Create shop menu
		RBL_ShopMenuWidgetImpl shop = new RBL_ShopMenuWidgetImpl();
		
		// Test initial state
		Assert(!shop.IsVisible(), "Shop initially hidden");
		Assert(shop.GetCategoryCount() > 0, "Shop has categories");
		AssertEqual(shop.GetSelectedCategoryIndex(), 0, "Initial category is 0");
		AssertEqual(shop.GetSelectedItemIndex(), 0, "Initial item is 0");
		
		// Test toggle
		shop.Toggle();
		Assert(shop.IsVisible(), "Shop visible after toggle");
		
		shop.Toggle();
		// Shop starts closing, but still visible until animation completes
		// Assert(!shop.IsVisible(), "Shop hidden after second toggle");
		
		// Test navigation
		shop.Open();
		shop.SelectNextCategory();
		AssertEqual(shop.GetSelectedCategoryIndex(), 1, "Category incremented");
		
		shop.SelectPreviousCategory();
		AssertEqual(shop.GetSelectedCategoryIndex(), 0, "Category decremented");
		
		shop.SelectNextItem();
		AssertEqual(shop.GetSelectedItemIndex(), 1, "Item incremented");
	}
	
	static void Test_MainHUD()
	{
		PrintFormat("[RBL_UITests] --- Testing Main HUD ---");
		
		// Create main HUD
		RBL_MainHUDWidget hud = new RBL_MainHUDWidget();
		
		// Test initial state
		Assert(hud.IsVisible(), "HUD initially visible");
		AssertEqual(hud.GetDisplayMoney(), 0, "Initial money is 0");
		AssertEqual(hud.GetDisplayHR(), 0, "Initial HR is 0");
		AssertEqual(hud.GetDisplayWarLevel(), 1, "Initial war level is 1");
	}
	
	static void Test_ZoneInfo()
	{
		PrintFormat("[RBL_UITests] --- Testing Zone Info ---");
		
		// Create zone info widget
		RBL_ZoneInfoWidgetImpl zoneInfo = new RBL_ZoneInfoWidgetImpl();
		
		// Test initial state
		Assert(zoneInfo.GetDisplayZoneID().IsEmpty(), "Initial zone ID is empty");
		Assert(!zoneInfo.IsInCaptureRange(), "Initially not in capture range");
	}
	
	static void Test_CaptureBar()
	{
		PrintFormat("[RBL_UITests] --- Testing Capture Bar ---");
		
		// Create capture bar
		RBL_CaptureBarWidgetImpl captureBar = new RBL_CaptureBarWidgetImpl();
		
		// Test initial state
		Assert(!captureBar.IsCapturing(), "Initially not capturing");
		AssertEqualFloat(captureBar.GetDisplayProgress(), 0, 0.01, "Initial progress is 0");
		
		// Test force show
		captureBar.ForceShow("Test Zone", 0.5);
		Assert(captureBar.IsCapturing(), "Capturing after ForceShow");
		AssertEqualFloat(captureBar.GetDisplayProgress(), 0.5, 0.01, "Progress after ForceShow");
		AssertStringEqual(captureBar.GetCapturingZoneName(), "Test Zone", "Zone name after ForceShow");
	}
	
	static void Test_Undercover()
	{
		PrintFormat("[RBL_UITests] --- Testing Undercover Widget ---");
		
		// Create undercover widget
		RBL_UndercoverWidgetImpl undercover = new RBL_UndercoverWidgetImpl();
		
		// Test initial state
		Assert(undercover.GetDisplayStatus() == ERBLCoverStatus.HIDDEN, "Initially hidden");
		AssertEqualFloat(undercover.GetDisplaySuspicion(), 0, 0.01, "Initial suspicion is 0");
		
		// Test force state
		undercover.ForceState(ERBLCoverStatus.SUSPICIOUS, 0.5);
		Assert(undercover.GetDisplayStatus() == ERBLCoverStatus.SUSPICIOUS, "Status after ForceState");
		AssertEqualFloat(undercover.GetDisplaySuspicion(), 0.5, 0.01, "Suspicion after ForceState");
	}
	
	static void Test_UIManager()
	{
		PrintFormat("[RBL_UITests] --- Testing UI Manager ---");
		
		// Test singleton
		RBL_UIManager mgr1 = RBL_UIManager.GetInstance();
		RBL_UIManager mgr2 = RBL_UIManager.GetInstance();
		
		AssertNotNull(mgr1, "UIManager singleton exists");
		Assert(mgr1 == mgr2, "UIManager singleton is same instance");
		
		// Test visibility
		mgr1.SetVisible(true);
		Assert(mgr1.IsVisible(), "UIManager visibility set to true");
		
		mgr1.SetVisible(false);
		Assert(!mgr1.IsVisible(), "UIManager visibility set to false");
		
		// Restore
		mgr1.SetVisible(true);
	}
}

// ============================================================================
// TEST CONSOLE COMMANDS
// ============================================================================

class RBL_UITestCommands
{
	// Run all UI tests
	[ConsoleCmd("rbl_ui_test", "Run all UI tests")]
	static void RunTests()
	{
		RBL_UITests.RunAllTests();
	}
	
	// Test specific components
	[ConsoleCmd("rbl_ui_test_notif", "Test notification system")]
	static void TestNotifications()
	{
		// Show a series of test notifications
		RBL_UIManager mgr = RBL_UIManager.GetInstance();
		if (!mgr)
		{
			Print("[Test] UIManager not available");
			return;
		}
		
		mgr.ShowNotification("Test notification 1", RBL_UIColors.COLOR_TEXT_BRIGHT, 3.0);
		mgr.ShowNotification("Test notification 2 - Green", RBL_UIColors.COLOR_ACCENT_GREEN, 3.0);
		mgr.ShowNotification("Test notification 3 - Red", RBL_UIColors.COLOR_ACCENT_RED, 3.0);
		mgr.ShowNotification("Test notification 4 - Amber", RBL_UIColors.COLOR_ACCENT_AMBER, 3.0);
		
		Print("[Test] Sent 4 test notifications");
	}
	
	[ConsoleCmd("rbl_ui_test_shop", "Toggle shop menu")]
	static void TestShop()
	{
		RBL_UIManager mgr = RBL_UIManager.GetInstance();
		if (!mgr)
		{
			Print("[Test] UIManager not available");
			return;
		}
		
		mgr.ToggleShop();
		Print("[Test] Toggled shop menu");
	}
	
	[ConsoleCmd("rbl_ui_toggle", "Toggle UI visibility")]
	static void ToggleUI()
	{
		RBL_UIManager mgr = RBL_UIManager.GetInstance();
		if (!mgr)
		{
			Print("[Test] UIManager not available");
			return;
		}
		
		mgr.SetVisible(!mgr.IsVisible());
		PrintFormat("[Test] UI visibility: %1", mgr.IsVisible());
	}
}

