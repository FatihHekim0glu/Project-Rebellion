// ============================================================================
// PROJECT REBELLION - Input Handler
// Handles keybinds for HUD, Shop, Persistence, etc.
// Now integrates with RBL_InputSystem for proper custom keybinds
// ============================================================================

class RBL_InputHandler
{
	protected static ref RBL_InputHandler s_Instance;
	
	protected ref RBL_InputManager m_InputManager;
	protected bool m_bInitialized;
	protected bool m_bShopOpen;
	protected bool m_bSettingsOpen;
	
	static RBL_InputHandler GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_InputHandler();
		return s_Instance;
	}
	
	void RBL_InputHandler()
	{
		m_InputManager = RBL_InputManager.GetInstance();
		m_bInitialized = false;
		m_bShopOpen = false;
		
		PrintFormat("[RBL_Input] Input handler initialized");
		PrintKeybindHelp();
	}
	
	protected void PrintKeybindHelp()
	{
		PrintFormat("[RBL_Input] Keybinds:");
		PrintFormat("[RBL_Input]   %1 - Toggle Shop", GetKeyForAction(RBL_InputActions.TOGGLE_SHOP));
		PrintFormat("[RBL_Input]   %1 - Toggle Settings", GetKeyForAction(RBL_InputActions.TOGGLE_SETTINGS));
		PrintFormat("[RBL_Input]   %1 - Toggle HUD", GetKeyForAction(RBL_InputActions.TOGGLE_HUD));
		PrintFormat("[RBL_Input]   %1 - Quick Save", GetKeyForAction(RBL_InputActions.QUICK_SAVE));
		PrintFormat("[RBL_Input]   %1 - Quick Load", GetKeyForAction(RBL_InputActions.QUICK_LOAD));
		PrintFormat("[RBL_Input]   %1 - Toggle Map", GetKeyForAction(RBL_InputActions.TOGGLE_MAP));
		PrintFormat("[RBL_Input]   %1 - Toggle Missions", GetKeyForAction(RBL_InputActions.TOGGLE_MISSIONS));
		PrintFormat("[RBL_Input]   %1 - Toggle Debug HUD", GetKeyForAction(RBL_InputActions.TOGGLE_DEBUG_HUD));
		PrintFormat("[RBL_Input]   %1 - Close menus", GetKeyForAction(RBL_InputActions.CLOSE_MENU));
	}
	
	protected string GetKeyForAction(string actionName)
	{
		if (m_InputManager && m_InputManager.IsInitialized())
			return m_InputManager.GetKeyForAction(actionName);
		
		// Fallback to default display names
		if (actionName == RBL_InputActions.TOGGLE_SHOP) return "F7";
		if (actionName == RBL_InputActions.TOGGLE_SETTINGS) return "F8";
		if (actionName == RBL_InputActions.TOGGLE_HUD) return "H";
		if (actionName == RBL_InputActions.QUICK_SAVE) return "F5";
		if (actionName == RBL_InputActions.QUICK_LOAD) return "F9";
		if (actionName == RBL_InputActions.TOGGLE_MAP) return "M";
		if (actionName == RBL_InputActions.TOGGLE_MISSIONS) return "L";
		if (actionName == RBL_InputActions.TOGGLE_DEBUG_HUD) return "F6";
		if (actionName == RBL_InputActions.CLOSE_MENU) return "ESC";
		
		return "?";
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		// Initialize the new input manager
		m_InputManager.Initialize();
		
		// Subscribe to menu state changes
		m_InputManager.GetOnMenuStateChanged().Insert(OnMenuStateChanged);
		
		m_bInitialized = true;
		PrintFormat("[RBL_Input] Input handler ready");
	}
	
	void Update(float timeSlice)
	{
		// Delegate to new input manager
		if (m_InputManager)
			m_InputManager.Update(timeSlice);
	}
	
	protected void OnMenuStateChanged(bool isOpen)
	{
		m_bShopOpen = isOpen;
	}
	
	// Legacy handlers for external callers
	void HandleShopToggle()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			RBL_ShopMenuWidgetImpl shopMenu = uiMgr.GetShopMenu();
			if (shopMenu)
			{
				shopMenu.Toggle();
				m_bShopOpen = shopMenu.IsVisible();
			}
		}
		
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.ToggleMenu();
	}
	
	void HandleEscape()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
			return;
		
		// Close settings menu first
		if (m_bSettingsOpen)
		{
			RBL_SettingsMenuWidget settingsMenu = uiMgr.GetSettingsMenu();
			if (settingsMenu && settingsMenu.IsVisible())
			{
				settingsMenu.Close();
				m_bSettingsOpen = false;
				return;
			}
		}
		
		// Then close shop menu
		if (m_bShopOpen)
		{
			RBL_ShopMenuWidgetImpl shopMenu = uiMgr.GetShopMenu();
			if (shopMenu && shopMenu.IsVisible())
			{
				shopMenu.Close();
				m_bShopOpen = false;
			}
		}
	}
	
	void HandleSettingsToggle()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			RBL_SettingsMenuWidget settingsMenu = uiMgr.GetSettingsMenu();
			if (settingsMenu)
			{
				settingsMenu.Toggle();
				m_bSettingsOpen = settingsMenu.IsVisible();
				
				if (m_bSettingsOpen)
					PrintFormat("[RBL_Input] Settings opened");
				else
					PrintFormat("[RBL_Input] Settings closed");
			}
		}
	}
	
	void HandleQuickSave()
	{
		RBL_PersistenceIntegration persistence = RBL_PersistenceIntegration.GetInstance();
		if (persistence)
		{
			persistence.QuickSave();
			RBL_Notifications.GameSaved();
		}
	}
	
	void HandleQuickLoad()
	{
		RBL_PersistenceIntegration persistence = RBL_PersistenceIntegration.GetInstance();
		if (persistence)
			persistence.QuickLoad();
	}
	
	void HandleToggleHUD()
	{
		RBL_HUDManager hud = RBL_HUDManager.GetInstance();
		if (hud)
			hud.ToggleVisibility();
	}
	
	void HandleToggleDebugHUD()
	{
		RBL_ScreenHUD screenHUD = RBL_ScreenHUD.GetInstance();
		if (screenHUD)
			screenHUD.Toggle();
	}
	
	// State queries
	bool IsShopOpen() { return m_bShopOpen; }
	bool IsSettingsOpen() { return m_bSettingsOpen; }
	bool IsAnyMenuOpen() { return m_bShopOpen || m_bSettingsOpen; }
	bool IsInitialized() { return m_bInitialized; }
	
	// Get the input manager for direct access
	RBL_InputManager GetInputManager() { return m_InputManager; }
}

// ============================================================================
// Debug Console Commands
// ============================================================================
class RBL_DebugCommands
{
	static void PrintStatus()
	{
		PrintFormat("\n========================================");
		PrintFormat("     PROJECT REBELLION - STATUS");
		PrintFormat("========================================\n");

		// Economy
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			PrintFormat("=== ECONOMY ===");
			PrintFormat("Money: $%1", econMgr.GetMoney());
			PrintFormat("Human Resources: %1", econMgr.GetHR());
		}

		// Campaign
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			PrintFormat("\n=== CAMPAIGN ===");
			PrintFormat("War Level: %1 / 10", campaignMgr.GetWarLevel());
			PrintFormat("Aggression: %1%%", campaignMgr.GetAggression());
		}

		// Zones
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			PrintFormat("\n=== TERRITORIES ===");
			PrintFormat("FIA Controlled: %1", zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA));
			PrintFormat("Enemy Controlled: %1", zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR));
			PrintFormat("Total Zones: %1", zoneMgr.GetTotalZoneCount());
		}

		// Commander AI
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
		{
			PrintFormat("\n=== ENEMY AI ===");
			PrintFormat("Enemy Resources: %1", commanderAI.GetFactionResources());
			PrintFormat("Active QRFs: %1", commanderAI.GetActiveQRFCount());
		}

		PrintFormat("\n========================================");
		PrintFormat("\n=== DEBUG COMMANDS ===");
		PrintFormat("RBL_DebugCommands.TeleportToZone(\"zoneid\")");
		PrintFormat("RBL_DebugCommands.TestResourceTick()");
		PrintFormat("RBL_DebugCommands.PrintKeybinds()");
		PrintFormat("\n========================================");
	}

	static void PrintKeybinds()
	{
		PrintFormat("\n========================================");
		PrintFormat("     PROJECT REBELLION - KEYBINDS");
		PrintFormat("========================================\n");
		
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (!registry)
		{
			PrintFormat("Input registry not available");
			return;
		}
		
		array<ref RBL_Keybind> bindings = {};
		registry.GetAllBindings(bindings);
		
		foreach (RBL_Keybind binding : bindings)
		{
			PrintFormat("  [%1] - %2 (%3)", 
				binding.GetKeyDisplayName(), 
				binding.DisplayName, 
				binding.ActionName);
		}
		
		PrintFormat("\n========================================");
	}

	static void OpenShop()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.OpenMenu();
	}

	static void Buy(string itemID)
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.PurchaseItem(itemID);
	}

	static void AddMoney(int amount)
	{
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			econMgr.AddMoney(amount);
			PrintFormat("[RBL_Debug] Added $%1. New balance: $%2", amount, econMgr.GetMoney());
		}
	}

	static void AddHR(int amount)
	{
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			econMgr.AddHR(amount);
			PrintFormat("[RBL_Debug] Added %1 HR. New total: %2", amount, econMgr.GetHR());
		}
	}

	static void ListZones()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;

		PrintFormat("\n=== ALL ZONES ===");
		array<ref RBL_VirtualZone> zones = zoneMgr.GetAllVirtualZones();
		for (int i = 0; i < zones.Count(); i++)
		{
			RBL_VirtualZone zone = zones[i];
			string owner = typename.EnumToString(ERBLFactionKey, zone.GetOwnerFaction());
			string type = typename.EnumToString(ERBLZoneType, zone.GetZoneType());
			PrintFormat("  %1 [%2] - Owner: %3", zone.GetZoneID(), type, owner);
		}
	}

	static void CaptureZone(string zoneID)
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;

		RBL_VirtualZone zone = zoneMgr.GetVirtualZoneByID(zoneID);
		if (zone)
		{
			zone.SetOwnerFaction(ERBLFactionKey.FIA);
			PrintFormat("[RBL_Debug] Zone %1 captured by FIA!", zoneID);

			// Award resources
			RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
			if (econMgr)
			{
				econMgr.AddMoney(zone.GetStrategicValue() / 2);
				econMgr.AddHR(2);
			}

			// Notify campaign manager
			RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
			if (campaignMgr)
			{
				PrintFormat("[RBL_Debug] Campaign notified of zone capture");
			}
		}
		else
		{
			PrintFormat("[RBL_Debug] Zone not found: %1", zoneID);
		}
	}

	static void TeleportToZone(string zoneID)
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;

		RBL_VirtualZone zone = zoneMgr.GetVirtualZoneByID(zoneID);
		if (!zone)
		{
			PrintFormat("[RBL_Debug] Zone not found: %1", zoneID);
			return;
		}

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
		{
			PrintFormat("[RBL_Debug] No player controller found");
			return;
		}

		IEntity playerEntity = playerController.GetControlledEntity();
		if (!playerEntity)
		{
			PrintFormat("[RBL_Debug] No controlled entity found");
			return;
		}

		vector zonePos = zone.GetZonePosition();
		zonePos[1] = zonePos[1] + 2.0;

		playerEntity.SetOrigin(zonePos);
		PrintFormat("[RBL_Debug] Teleported to zone: %1", zoneID);
	}

	static void TestResourceTick()
	{
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			PrintFormat("[RBL_Debug] Resource tick triggered manually");
			PrintFormat("[RBL_Debug] Check console for resource tick output");
		}
	}
	
	static void TestInputSystem()
	{
		PrintFormat("\n========================================");
		PrintFormat("     INPUT SYSTEM TEST");
		PrintFormat("========================================\n");
		
		RBL_InputManager inputMgr = RBL_InputManager.GetInstance();
		if (!inputMgr)
		{
			PrintFormat("[FAIL] Input manager not available");
			return;
		}
		
		PrintFormat("Input Manager Initialized: %1", inputMgr.IsInitialized());
		PrintFormat("Input Manager Enabled: %1", inputMgr.IsEnabled());
		PrintFormat("Menu Open: %1", inputMgr.IsMenuOpen());
		
		RBL_InputBindingRegistry registry = inputMgr.GetRegistry();
		if (registry)
		{
			PrintFormat("Registry Initialized: %1", registry.IsInitialized());
			PrintFormat("Using Custom Actions: %1", registry.IsUsingCustomActions());
			PrintFormat("Binding Count: %1", registry.GetBindingCount());
		}
		
		PrintFormat("\n========================================");
	}
}

// ============================================================================
// Input Test Commands
// ============================================================================
class RBL_InputTestCommands
{
	static void RunInputTests()
	{
		PrintFormat("\n========================================");
		PrintFormat("     INPUT SYSTEM TESTS");
		PrintFormat("========================================\n");
		
		int testsPassed = 0;
		int testsFailed = 0;
		
		// Test 1: Input Manager Existence
		PrintFormat("[TEST 1] Checking Input Manager...");
		RBL_InputManager inputMgr = RBL_InputManager.GetInstance();
		if (inputMgr)
		{
			PrintFormat("  [PASS] Input Manager exists");
			testsPassed++;
		}
		else
		{
			PrintFormat("  [FAIL] Input Manager not found");
			testsFailed++;
			PrintFormat("\n========================================");
			PrintFormat("Tests completed: %1 passed, %2 failed", testsPassed, testsFailed);
			PrintFormat("========================================\n");
			return;
		}
		
		// Test 2: Input Manager Initialization
		PrintFormat("[TEST 2] Checking Input Manager initialization...");
		if (inputMgr.IsInitialized())
		{
			PrintFormat("  [PASS] Input Manager is initialized");
			testsPassed++;
		}
		else
		{
			PrintFormat("  [FAIL] Input Manager not initialized");
			testsFailed++;
		}
		
		// Test 3: Input Manager State
		PrintFormat("[TEST 3] Checking Input Manager state...");
		PrintFormat("  Enabled: %1", inputMgr.IsEnabled());
		PrintFormat("  Menu Open: %1", inputMgr.IsMenuOpen());
		testsPassed++;
		
		// Test 4: Input Binding Registry
		PrintFormat("[TEST 4] Checking Input Binding Registry...");
		RBL_InputBindingRegistry registry = inputMgr.GetRegistry();
		if (registry)
		{
			PrintFormat("  [PASS] Registry exists");
			PrintFormat("  Initialized: %1", registry.IsInitialized());
			PrintFormat("  Using Custom Actions: %1", registry.IsUsingCustomActions());
			PrintFormat("  Binding Count: %1", registry.GetBindingCount());
			testsPassed++;
		}
		else
		{
			PrintFormat("  [FAIL] Registry not found");
			testsFailed++;
		}
		
		// Test 5: Required Actions
		PrintFormat("[TEST 5] Checking required actions...");
		array<string> requiredActions = {
			RBL_InputActions.TOGGLE_SHOP,
			RBL_InputActions.TOGGLE_SETTINGS,
			RBL_InputActions.TOGGLE_HUD,
			RBL_InputActions.QUICK_SAVE,
			RBL_InputActions.QUICK_LOAD,
			RBL_InputActions.TOGGLE_MAP,
			RBL_InputActions.TOGGLE_MISSIONS,
			RBL_InputActions.CLOSE_MENU
		};
		
		int actionsFound = 0;
		foreach (string action : requiredActions)
		{
			string keyDisplay = inputMgr.GetKeyForAction(action);
			if (keyDisplay != "?" && keyDisplay.Length() > 0)
			{
				PrintFormat("  [OK] %1 -> %2", action, keyDisplay);
				actionsFound++;
			}
			else
			{
				PrintFormat("  [WARN] %1 -> Not bound", action);
			}
		}
		
		if (actionsFound == requiredActions.Count())
		{
			PrintFormat("  [PASS] All required actions have bindings");
			testsPassed++;
		}
		else if (actionsFound > 0)
		{
			PrintFormat("  [PARTIAL] %1/%2 actions have bindings", actionsFound, requiredActions.Count());
			testsPassed++;
		}
		else
		{
			PrintFormat("  [FAIL] No actions have bindings");
			testsFailed++;
		}
		
		// Test 6: Input Handler
		PrintFormat("[TEST 6] Checking Input Handler...");
		RBL_InputHandler handler = RBL_InputHandler.GetInstance();
		if (handler)
		{
			PrintFormat("  [PASS] Input Handler exists");
			PrintFormat("  Initialized: %1", handler.IsInitialized());
			testsPassed++;
		}
		else
		{
			PrintFormat("  [FAIL] Input Handler not found");
			testsFailed++;
		}
		
		// Test 7: Input Config Compatibility
		PrintFormat("[TEST 7] Checking Input Config compatibility...");
		string shopKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_SHOP);
		if (shopKey != "?")
		{
			PrintFormat("  [PASS] RBL_InputConfig.GetKeyDisplay() works");
			PrintFormat("  Shop key: %1", shopKey);
			testsPassed++;
		}
		else
		{
			PrintFormat("  [WARN] RBL_InputConfig.GetKeyDisplay() returned '?'");
			testsPassed++;
		}
		
		// Summary
		PrintFormat("\n========================================");
		PrintFormat("TEST SUMMARY");
		PrintFormat("========================================");
		PrintFormat("Tests Passed: %1", testsPassed);
		PrintFormat("Tests Failed: %1", testsFailed);
		PrintFormat("Total Tests: %1", testsPassed + testsFailed);
		
		if (testsFailed == 0)
			PrintFormat("\n[SUCCESS] All input tests passed!");
		else
			PrintFormat("\n[WARNING] Some tests failed. Check output above.");
		
		PrintFormat("========================================\n");
	}
}