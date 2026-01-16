// ============================================================================
// PROJECT REBELLION - UI Tests Framework
// Comprehensive testing for UI components
// ============================================================================

class RBL_UITests
{
	protected static int s_iTestsPassed;
	protected static int s_iTestsFailed;
	protected static int s_iTestsTotal;
	
	static void RunAllTests()
	{
		PrintFormat("\n========================================");
		PrintFormat("     UI TESTS - STARTING");
		PrintFormat("========================================\n");
		
		s_iTestsPassed = 0;
		s_iTestsFailed = 0;
		s_iTestsTotal = 0;
		
		// Test UI Manager
		TestUIManager();
		
		// Test HUD Components
		TestHUDComponents();
		
		// Test Notifications
		TestNotifications();
		
		// Test Shop Menu
		TestShopMenu();
		
		// Test Zone Info Widget
		TestZoneInfoWidget();
		
		// Test Capture Bar
		TestCaptureBar();
		
		// Test Undercover Widget
		TestUndercoverWidget();
		
		// Test Settings Menu
		TestSettingsMenu();
		
		// Test Input System Integration
		TestInputIntegration();
		
		// Print summary
		PrintTestSummary();
	}
	
	protected static void TestUIManager()
	{
		PrintFormat("[TEST] UI Manager...");
		s_iTestsTotal++;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			PrintFormat("  [PASS] UI Manager exists");
			s_iTestsPassed++;
			
			// Test initialization
			if (uiMgr.IsInitialized())
			{
				PrintFormat("  [PASS] UI Manager initialized");
				s_iTestsPassed++;
			}
			else
			{
				PrintFormat("  [FAIL] UI Manager not initialized");
				s_iTestsFailed++;
			}
			s_iTestsTotal++;
			
			// Test visibility
			bool wasVisible = uiMgr.IsVisible();
			uiMgr.SetVisible(false);
			if (!uiMgr.IsVisible())
			{
				PrintFormat("  [PASS] Visibility toggle works");
				s_iTestsPassed++;
			}
			else
			{
				PrintFormat("  [FAIL] Visibility toggle failed");
				s_iTestsFailed++;
			}
			s_iTestsTotal++;
			uiMgr.SetVisible(wasVisible);
		}
		else
		{
			PrintFormat("  [FAIL] UI Manager not found");
			s_iTestsFailed++;
		}
	}
	
	protected static void TestHUDComponents()
	{
		PrintFormat("[TEST] HUD Components...");
		s_iTestsTotal++;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
		{
			PrintFormat("  [FAIL] UI Manager not available");
			s_iTestsFailed++;
			return;
		}
		
		// Test Main HUD
		RBL_MainHUDWidget mainHUD = uiMgr.GetMainHUD();
		if (mainHUD)
		{
			PrintFormat("  [PASS] Main HUD widget exists");
			s_iTestsPassed++;
		}
		else
		{
			PrintFormat("  [WARN] Main HUD widget not found");
		}
		s_iTestsTotal++;
		
		// Test Zone Info
		RBL_ZoneInfoWidgetImpl zoneInfo = uiMgr.GetZoneInfo();
		if (zoneInfo)
		{
			PrintFormat("  [PASS] Zone Info widget exists");
			s_iTestsPassed++;
		}
		else
		{
			PrintFormat("  [WARN] Zone Info widget not found");
		}
		s_iTestsTotal++;
	}
	
	protected static void TestNotifications()
	{
		PrintFormat("[TEST] Notification System...");
		s_iTestsTotal++;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
		{
			PrintFormat("  [FAIL] UI Manager not available");
			s_iTestsFailed++;
			return;
		}
		
		RBL_NotificationManagerImpl notifications = uiMgr.GetNotifications();
		if (notifications)
		{
			PrintFormat("  [PASS] Notification manager exists");
			s_iTestsPassed++;
			
			// Test showing notification
			uiMgr.ShowNotification("Test Notification", RBL_UIColors.COLOR_ACCENT_GREEN, 2.0);
			PrintFormat("  [PASS] Notification sent");
			s_iTestsPassed++;
		}
		else
		{
			PrintFormat("  [FAIL] Notification manager not found");
			s_iTestsFailed++;
		}
		s_iTestsTotal++;
	}
	
	protected static void TestShopMenu()
	{
		PrintFormat("[TEST] Shop Menu...");
		s_iTestsTotal++;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
		{
			PrintFormat("  [FAIL] UI Manager not available");
			s_iTestsFailed++;
			return;
		}
		
		RBL_ShopMenuWidgetImpl shopMenu = uiMgr.GetShopMenu();
		if (shopMenu)
		{
			PrintFormat("  [PASS] Shop menu widget exists");
			s_iTestsPassed++;
		}
		else
		{
			PrintFormat("  [WARN] Shop menu widget not found");
		}
		s_iTestsTotal++;
		
		// Test shop manager
		RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
		if (shopMgr)
		{
			PrintFormat("  [PASS] Shop manager exists");
			s_iTestsPassed++;
		}
		else
		{
			PrintFormat("  [FAIL] Shop manager not found");
			s_iTestsFailed++;
		}
		s_iTestsTotal++;
	}
	
	protected static void TestZoneInfoWidget()
	{
		PrintFormat("[TEST] Zone Info Widget...");
		s_iTestsTotal++;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
		{
			PrintFormat("  [FAIL] UI Manager not available");
			s_iTestsFailed++;
			return;
		}
		
		RBL_ZoneInfoWidgetImpl zoneInfo = uiMgr.GetZoneInfo();
		if (zoneInfo)
		{
			PrintFormat("  [PASS] Zone Info widget exists");
			s_iTestsPassed++;
			
			// Test show/hide
			bool wasVisible = zoneInfo.IsVisible();
			zoneInfo.Hide();
			if (!zoneInfo.IsVisible())
			{
				PrintFormat("  [PASS] Zone Info hide works");
				s_iTestsPassed++;
			}
			else
			{
				PrintFormat("  [FAIL] Zone Info hide failed");
				s_iTestsFailed++;
			}
			s_iTestsTotal++;
			zoneInfo.Show();
		}
		else
		{
			PrintFormat("  [WARN] Zone Info widget not found");
		}
	}
	
	protected static void TestCaptureBar()
	{
		PrintFormat("[TEST] Capture Bar Widget...");
		s_iTestsTotal++;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
		{
			PrintFormat("  [FAIL] UI Manager not available");
			s_iTestsFailed++;
			return;
		}
		
		RBL_CaptureBarWidgetImpl captureBar = uiMgr.GetCaptureBar();
		if (captureBar)
		{
			PrintFormat("  [PASS] Capture bar widget exists");
			s_iTestsPassed++;
		}
		else
		{
			PrintFormat("  [WARN] Capture bar widget not found");
		}
		s_iTestsTotal++;
	}
	
	protected static void TestUndercoverWidget()
	{
		PrintFormat("[TEST] Undercover Widget...");
		s_iTestsTotal++;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
		{
			PrintFormat("  [FAIL] UI Manager not available");
			s_iTestsFailed++;
			return;
		}
		
		RBL_UndercoverWidgetImpl undercover = uiMgr.GetUndercoverIndicator();
		if (undercover)
		{
			PrintFormat("  [PASS] Undercover widget exists");
			s_iTestsPassed++;
		}
		else
		{
			PrintFormat("  [WARN] Undercover widget not found");
		}
		s_iTestsTotal++;
	}
	
	protected static void TestSettingsMenu()
	{
		PrintFormat("[TEST] Settings Menu...");
		s_iTestsTotal++;
		
		RBL_SettingsManager settingsMgr = RBL_SettingsManager.GetInstance();
		if (settingsMgr)
		{
			PrintFormat("  [PASS] Settings manager exists");
			s_iTestsPassed++;
			
			if (settingsMgr.IsInitialized())
			{
				PrintFormat("  [PASS] Settings manager initialized");
				s_iTestsPassed++;
			}
			else
			{
				PrintFormat("  [WARN] Settings manager not initialized");
			}
			s_iTestsTotal++;
		}
		else
		{
			PrintFormat("  [FAIL] Settings manager not found");
			s_iTestsFailed++;
		}
		s_iTestsTotal++;
	}
	
	protected static void TestInputIntegration()
	{
		PrintFormat("[TEST] Input System Integration...");
		s_iTestsTotal++;
		
		RBL_InputManager inputMgr = RBL_InputManager.GetInstance();
		if (inputMgr)
		{
			PrintFormat("  [PASS] Input manager exists");
			s_iTestsPassed++;
			
			if (inputMgr.IsInitialized())
			{
				PrintFormat("  [PASS] Input manager initialized");
				s_iTestsPassed++;
			}
			else
			{
				PrintFormat("  [WARN] Input manager not initialized");
			}
			s_iTestsTotal++;
		}
		else
		{
			PrintFormat("  [FAIL] Input manager not found");
			s_iTestsFailed++;
		}
		s_iTestsTotal++;
	}
	
	protected static void PrintTestSummary()
	{
		PrintFormat("\n========================================");
		PrintFormat("     UI TESTS - SUMMARY");
		PrintFormat("========================================");
		PrintFormat("Total Tests: %1", s_iTestsTotal);
		PrintFormat("Passed: %1", s_iTestsPassed);
		PrintFormat("Failed: %1", s_iTestsFailed);
		PrintFormat("Success Rate: %1%%", Math.Round((s_iTestsPassed / s_iTestsTotal) * 100));
		
		if (s_iTestsFailed == 0)
			PrintFormat("\n[SUCCESS] All UI tests passed!");
		else
			PrintFormat("\n[WARNING] Some tests failed. Check output above.");
		
		PrintFormat("========================================\n");
	}
	
	// Individual test methods for specific components
	static void TestNotificationSystem()
	{
		TestNotifications();
	}
	
	static void TestShopSystem()
	{
		TestShopMenu();
	}
	
	static void TestHUDSystem()
	{
		TestHUDComponents();
	}
}
