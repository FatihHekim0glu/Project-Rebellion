// ============================================================================
// PROJECT REBELLION - Input System Tests
// Automated testing for the input binding system
// ============================================================================

class RBL_InputTests
{
	// Run all input tests
	static void RunAll()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("\n[RBL_InputTests] Starting input system tests...\n");
		
		TestKeyCodesClass();
		TestKeybindClass();
		TestInputBindingRegistry();
		TestInputManager();
		TestInputActions();
		TestInputHandler();
		TestCooldownSystem();
		TestEventSystem();
		
		PrintFormat("\n[RBL_InputTests] Input tests complete.\n");
	}
	
	// Test RBL_KeyCodes class
	static void TestKeyCodesClass()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing KeyCodes class...");
		
		// Test key code values are unique
		runner.Assert("KEY_J has expected value", RBL_KeyCodes.KEY_J == 36, "Wrong value for KEY_J");
		runner.Assert("KEY_H has expected value", RBL_KeyCodes.KEY_H == 35, "Wrong value for KEY_H");
		runner.Assert("KEY_F5 has expected value", RBL_KeyCodes.KEY_F5 == 63, "Wrong value for KEY_F5");
		runner.Assert("KEY_F9 has expected value", RBL_KeyCodes.KEY_F9 == 67, "Wrong value for KEY_F9");
		runner.Assert("KEY_ESC has expected value", RBL_KeyCodes.KEY_ESC == 1, "Wrong value for KEY_ESC");
		runner.Assert("KEY_M has expected value", RBL_KeyCodes.KEY_M == 50, "Wrong value for KEY_M");
		runner.Assert("KEY_L has expected value", RBL_KeyCodes.KEY_L == 38, "Wrong value for KEY_L");
		runner.Assert("KEY_F has expected value", RBL_KeyCodes.KEY_F == 33, "Wrong value for KEY_F");
		
		// Test GetKeyName returns correct strings
		runner.AssertStringEqual("GetKeyName J", "J", RBL_KeyCodes.GetKeyName(RBL_KeyCodes.KEY_J));
		runner.AssertStringEqual("GetKeyName H", "H", RBL_KeyCodes.GetKeyName(RBL_KeyCodes.KEY_H));
		runner.AssertStringEqual("GetKeyName F5", "F5", RBL_KeyCodes.GetKeyName(RBL_KeyCodes.KEY_F5));
		runner.AssertStringEqual("GetKeyName F9", "F9", RBL_KeyCodes.GetKeyName(RBL_KeyCodes.KEY_F9));
		runner.AssertStringEqual("GetKeyName ESC", "ESC", RBL_KeyCodes.GetKeyName(RBL_KeyCodes.KEY_ESC));
		runner.AssertStringEqual("GetKeyName M", "M", RBL_KeyCodes.GetKeyName(RBL_KeyCodes.KEY_M));
		runner.AssertStringEqual("GetKeyName SPACE", "SPACE", RBL_KeyCodes.GetKeyName(RBL_KeyCodes.KEY_SPACE));
		runner.AssertStringEqual("GetKeyName TAB", "TAB", RBL_KeyCodes.GetKeyName(RBL_KeyCodes.KEY_TAB));
		runner.AssertStringEqual("GetKeyName ENTER", "ENTER", RBL_KeyCodes.GetKeyName(RBL_KeyCodes.KEY_ENTER));
		
		// Test unknown key code
		runner.AssertStringEqual("GetKeyName unknown", "?", RBL_KeyCodes.GetKeyName(9999));
	}
	
	// Test RBL_Keybind class
	static void TestKeybindClass()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing Keybind class...");
		
		// Test keybind creation
		RBL_Keybind binding = new RBL_Keybind("TestAction", "Test Display", RBL_KeyCodes.KEY_T, 0.5);
		
		runner.AssertNotNull("Keybind created", binding);
		runner.AssertStringEqual("Keybind ActionName", "TestAction", binding.ActionName);
		runner.AssertStringEqual("Keybind DisplayName", "Test Display", binding.DisplayName);
		runner.AssertEqual("Keybind DefaultKeyCode", RBL_KeyCodes.KEY_T, binding.DefaultKeyCode);
		runner.AssertEqual("Keybind CurrentKeyCode", RBL_KeyCodes.KEY_T, binding.CurrentKeyCode);
		runner.AssertEqualFloat("Keybind Cooldown", 0.5, binding.Cooldown, 0.001);
		
		// Test GetKeyDisplayName
		runner.AssertStringEqual("Keybind GetKeyDisplayName", "T", binding.GetKeyDisplayName());
		
		// Test with different key
		RBL_Keybind binding2 = new RBL_Keybind("SaveAction", "Quick Save", RBL_KeyCodes.KEY_F5, 1.0);
		runner.AssertStringEqual("Keybind2 GetKeyDisplayName", "F5", binding2.GetKeyDisplayName());
		
		// Test default cooldown
		RBL_Keybind binding3 = new RBL_Keybind("DefaultCooldown", "Default", RBL_KeyCodes.KEY_A);
		runner.AssertEqualFloat("Default cooldown is 0.3", 0.3, binding3.Cooldown, 0.001);
	}
	
	// Test RBL_InputBindingRegistry
	static void TestInputBindingRegistry()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing InputBindingRegistry...");
		
		// Get singleton
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		runner.AssertNotNull("Registry singleton exists", registry);
		
		if (!registry)
			return;
		
		// Initialize if needed
		registry.Initialize();
		runner.Assert("Registry is initialized", registry.IsInitialized(), "Not initialized");
		
		// Test binding count
		int bindingCount = registry.GetBindingCount();
		runner.Assert("Registry has bindings", bindingCount > 0, "No bindings registered");
		runner.Assert("Expected binding count", bindingCount >= 9, string.Format("Only %1 bindings", bindingCount));
		
		// Test GetBinding for known actions
		RBL_Keybind shopBinding = registry.GetBinding(RBL_InputActions.TOGGLE_SHOP);
		runner.AssertNotNull("Shop binding exists", shopBinding);
		if (shopBinding)
		{
			runner.AssertStringEqual("Shop action name", RBL_InputActions.TOGGLE_SHOP, shopBinding.ActionName);
			runner.AssertEqual("Shop default key is J", RBL_KeyCodes.KEY_J, shopBinding.DefaultKeyCode);
		}
		
		RBL_Keybind hudBinding = registry.GetBinding(RBL_InputActions.TOGGLE_HUD);
		runner.AssertNotNull("HUD binding exists", hudBinding);
		if (hudBinding)
		{
			runner.AssertEqual("HUD default key is H", RBL_KeyCodes.KEY_H, hudBinding.DefaultKeyCode);
		}
		
		RBL_Keybind saveBinding = registry.GetBinding(RBL_InputActions.QUICK_SAVE);
		runner.AssertNotNull("Save binding exists", saveBinding);
		if (saveBinding)
		{
			runner.AssertEqual("Save default key is F5", RBL_KeyCodes.KEY_F5, saveBinding.DefaultKeyCode);
			runner.AssertEqualFloat("Save cooldown is 1.0", 1.0, saveBinding.Cooldown, 0.001);
		}
		
		RBL_Keybind loadBinding = registry.GetBinding(RBL_InputActions.QUICK_LOAD);
		runner.AssertNotNull("Load binding exists", loadBinding);
		if (loadBinding)
		{
			runner.AssertEqual("Load default key is F9", RBL_KeyCodes.KEY_F9, loadBinding.DefaultKeyCode);
		}
		
		// Test GetBinding for non-existent action
		RBL_Keybind nullBinding = registry.GetBinding("NonExistentAction");
		runner.AssertNull("Non-existent binding is null", nullBinding);
		
		// Test GetKeyDisplayForAction
		runner.AssertStringEqual("GetKeyDisplayForAction Shop", "J", registry.GetKeyDisplayForAction(RBL_InputActions.TOGGLE_SHOP));
		runner.AssertStringEqual("GetKeyDisplayForAction HUD", "H", registry.GetKeyDisplayForAction(RBL_InputActions.TOGGLE_HUD));
		runner.AssertStringEqual("GetKeyDisplayForAction Save", "F5", registry.GetKeyDisplayForAction(RBL_InputActions.QUICK_SAVE));
		runner.AssertStringEqual("GetKeyDisplayForAction unknown", "?", registry.GetKeyDisplayForAction("Unknown"));
		
		// Test GetAllBindings
		array<ref RBL_Keybind> allBindings = {};
		registry.GetAllBindings(allBindings);
		runner.AssertEqual("GetAllBindings count matches", bindingCount, allBindings.Count());
	}
	
	// Test RBL_InputManager
	static void TestInputManager()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing InputManager...");
		
		// Get singleton
		RBL_InputManager inputMgr = RBL_InputManager.GetInstance();
		runner.AssertNotNull("InputManager singleton exists", inputMgr);
		
		if (!inputMgr)
			return;
		
		// Initialize
		inputMgr.Initialize();
		runner.Assert("InputManager is initialized", inputMgr.IsInitialized(), "Not initialized");
		
		// Test default states
		runner.Assert("InputManager is enabled by default", inputMgr.IsEnabled(), "Not enabled");
		runner.Assert("No menu open initially", !inputMgr.IsMenuOpen(), "Menu open unexpectedly");
		
		// Test GetRegistry
		RBL_InputBindingRegistry registry = inputMgr.GetRegistry();
		runner.AssertNotNull("GetRegistry returns registry", registry);
		
		// Test GetKeyForAction helper
		runner.AssertStringEqual("GetKeyForAction Shop", "J", inputMgr.GetKeyForAction(RBL_InputActions.TOGGLE_SHOP));
		runner.AssertStringEqual("GetKeyForAction HUD", "H", inputMgr.GetKeyForAction(RBL_InputActions.TOGGLE_HUD));
		
		// Test SetEnabled
		inputMgr.SetEnabled(false);
		runner.Assert("SetEnabled false works", !inputMgr.IsEnabled(), "Still enabled");
		inputMgr.SetEnabled(true);
		runner.Assert("SetEnabled true works", inputMgr.IsEnabled(), "Not enabled");
		
		// Test SetMenuOpen
		inputMgr.SetMenuOpen(true);
		runner.Assert("SetMenuOpen true works", inputMgr.IsMenuOpen(), "Menu not open");
		inputMgr.SetMenuOpen(false);
		runner.Assert("SetMenuOpen false works", !inputMgr.IsMenuOpen(), "Menu still open");
		
		// Test event invokers exist
		runner.AssertNotNull("OnActionTriggered invoker", inputMgr.GetOnActionTriggered());
		runner.AssertNotNull("OnMenuStateChanged invoker", inputMgr.GetOnMenuStateChanged());
		
		// Test Update doesn't crash
		inputMgr.Update(0.016);
		runner.RecordResult("InputManager.Update runs", true, "No crash");
	}
	
	// Test RBL_InputActions constants
	static void TestInputActions()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing InputActions constants...");
		
		// Test action name format (should start with RBL_)
		runner.Assert("TOGGLE_SHOP starts with RBL_", 
			RBL_InputActions.TOGGLE_SHOP.IndexOf("RBL_") == 0, "Wrong prefix");
		runner.Assert("TOGGLE_HUD starts with RBL_", 
			RBL_InputActions.TOGGLE_HUD.IndexOf("RBL_") == 0, "Wrong prefix");
		runner.Assert("QUICK_SAVE starts with RBL_", 
			RBL_InputActions.QUICK_SAVE.IndexOf("RBL_") == 0, "Wrong prefix");
		runner.Assert("QUICK_LOAD starts with RBL_", 
			RBL_InputActions.QUICK_LOAD.IndexOf("RBL_") == 0, "Wrong prefix");
		runner.Assert("TOGGLE_MAP starts with RBL_", 
			RBL_InputActions.TOGGLE_MAP.IndexOf("RBL_") == 0, "Wrong prefix");
		runner.Assert("TOGGLE_MISSIONS starts with RBL_", 
			RBL_InputActions.TOGGLE_MISSIONS.IndexOf("RBL_") == 0, "Wrong prefix");
		runner.Assert("TOGGLE_DEBUG_HUD starts with RBL_", 
			RBL_InputActions.TOGGLE_DEBUG_HUD.IndexOf("RBL_") == 0, "Wrong prefix");
		runner.Assert("CLOSE_MENU starts with RBL_", 
			RBL_InputActions.CLOSE_MENU.IndexOf("RBL_") == 0, "Wrong prefix");
		runner.Assert("INTERACT starts with RBL_", 
			RBL_InputActions.INTERACT.IndexOf("RBL_") == 0, "Wrong prefix");
		
		// Test action names are unique
		array<string> actions = {};
		actions.Insert(RBL_InputActions.TOGGLE_SHOP);
		actions.Insert(RBL_InputActions.TOGGLE_HUD);
		actions.Insert(RBL_InputActions.QUICK_SAVE);
		actions.Insert(RBL_InputActions.QUICK_LOAD);
		actions.Insert(RBL_InputActions.TOGGLE_MAP);
		actions.Insert(RBL_InputActions.TOGGLE_MISSIONS);
		actions.Insert(RBL_InputActions.TOGGLE_DEBUG_HUD);
		actions.Insert(RBL_InputActions.CLOSE_MENU);
		actions.Insert(RBL_InputActions.INTERACT);
		
		bool allUnique = true;
		for (int i = 0; i < actions.Count() && allUnique; i++)
		{
			for (int j = i + 1; j < actions.Count(); j++)
			{
				if (actions[i] == actions[j])
				{
					allUnique = false;
					break;
				}
			}
		}
		runner.Assert("All action names are unique", allUnique, "Duplicate action names found");
		
		// Test fallback actions exist
		runner.Assert("FALLBACK_SHOP is not empty", RBL_InputActions.FALLBACK_SHOP.Length() > 0, "Empty fallback");
		runner.Assert("FALLBACK_MAP is not empty", RBL_InputActions.FALLBACK_MAP.Length() > 0, "Empty fallback");
		runner.Assert("FALLBACK_ESCAPE is not empty", RBL_InputActions.FALLBACK_ESCAPE.Length() > 0, "Empty fallback");
		runner.Assert("FALLBACK_USE is not empty", RBL_InputActions.FALLBACK_USE.Length() > 0, "Empty fallback");
	}
	
	// Test RBL_InputHandler
	static void TestInputHandler()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing InputHandler...");
		
		// Get singleton
		RBL_InputHandler handler = RBL_InputHandler.GetInstance();
		runner.AssertNotNull("InputHandler singleton exists", handler);
		
		if (!handler)
			return;
		
		// Initialize
		handler.Initialize();
		runner.Assert("InputHandler is initialized", handler.IsInitialized(), "Not initialized");
		
		// Test default states
		runner.Assert("Shop not open initially", !handler.IsShopOpen(), "Shop open unexpectedly");
		runner.Assert("No menu open initially", !handler.IsAnyMenuOpen(), "Menu open unexpectedly");
		
		// Test GetInputManager
		RBL_InputManager inputMgr = handler.GetInputManager();
		runner.AssertNotNull("GetInputManager returns manager", inputMgr);
		
		// Test Update doesn't crash
		handler.Update(0.016);
		runner.RecordResult("InputHandler.Update runs", true, "No crash");
	}
	
	// Test cooldown system
	static void TestCooldownSystem()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing cooldown system...");
		
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (!registry)
		{
			runner.RecordResult("Cooldown test", false, "Registry null");
			return;
		}
		
		registry.Initialize();
		
		// Test initial cooldown state
		runner.Assert("Shop not on cooldown initially", 
			!registry.IsOnCooldown(RBL_InputActions.TOGGLE_SHOP), "On cooldown unexpectedly");
		
		// Test trigger cooldown
		registry.TriggerCooldown(RBL_InputActions.TOGGLE_SHOP);
		runner.Assert("Shop on cooldown after trigger", 
			registry.IsOnCooldown(RBL_InputActions.TOGGLE_SHOP), "Not on cooldown");
		
		// Test cooldown value
		float cooldown = registry.GetCooldown(RBL_InputActions.TOGGLE_SHOP);
		runner.Assert("Cooldown value > 0", cooldown > 0, string.Format("Cooldown is %1", cooldown));
		
		// Test cooldown decay
		registry.UpdateCooldowns(0.5);
		float newCooldown = registry.GetCooldown(RBL_InputActions.TOGGLE_SHOP);
		runner.Assert("Cooldown decayed", newCooldown < cooldown, "Cooldown didn't decay");
		
		// Test cooldown expiry
		registry.UpdateCooldowns(10.0); // Large update to expire all cooldowns
		runner.Assert("Shop cooldown expired", 
			!registry.IsOnCooldown(RBL_InputActions.TOGGLE_SHOP), "Still on cooldown");
		
		// Test SetCooldown
		registry.SetCooldown(RBL_InputActions.QUICK_SAVE, 2.0);
		runner.AssertEqualFloat("SetCooldown works", 2.0, registry.GetCooldown(RBL_InputActions.QUICK_SAVE), 0.01);
		
		// Clear cooldowns for other tests
		registry.SetCooldown(RBL_InputActions.QUICK_SAVE, 0);
	}
	
	// Test event system
	static void TestEventSystem()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing event system...");
		
		RBL_InputManager inputMgr = RBL_InputManager.GetInstance();
		if (!inputMgr)
		{
			runner.RecordResult("Event system test", false, "Manager null");
			return;
		}
		
		inputMgr.Initialize();
		
		// Test event invokers are not null
		ScriptInvoker onAction = inputMgr.GetOnActionTriggered();
		runner.AssertNotNull("OnActionTriggered invoker exists", onAction);
		
		ScriptInvoker onMenu = inputMgr.GetOnMenuStateChanged();
		runner.AssertNotNull("OnMenuStateChanged invoker exists", onMenu);
		
		// Test RBL_InputActionEvent creation
		RBL_InputActionEvent evt = new RBL_InputActionEvent("TestAction", RBL_KeyCodes.KEY_T, true);
		runner.AssertNotNull("InputActionEvent created", evt);
		runner.AssertStringEqual("Event ActionName", "TestAction", evt.ActionName);
		runner.AssertEqual("Event KeyCode", RBL_KeyCodes.KEY_T, evt.KeyCode);
		runner.Assert("Event IsPressed", evt.IsPressed, "Not pressed");
		runner.Assert("Event has timestamp", evt.Timestamp > 0, "No timestamp");
		
		// Test RBL_InputState
		RBL_InputState state = new RBL_InputState();
		runner.AssertNotNull("InputState created", state);
		runner.Assert("Initial state not triggered", !state.IsTriggered(), "Triggered unexpectedly");
		runner.Assert("Initial state not held", !state.IsHeld(), "Held unexpectedly");
		runner.AssertEqualFloat("Initial held time is 0", 0, state.GetHeldTime(), 0.001);
		
		// Test state update - press
		state.Update(true, 0.1);
		runner.Assert("State triggered on press", state.IsTriggered(), "Not triggered on press");
		runner.Assert("State held on press", state.IsHeld(), "Not held on press");
		
		// Test state update - hold
		state.Update(true, 0.5);
		runner.Assert("State not triggered on hold", !state.IsTriggered(), "Triggered on hold");
		runner.Assert("State still held", state.IsHeld(), "Not held anymore");
		runner.Assert("Held time accumulated", state.GetHeldTime() > 0.5, "Held time not tracked");
		
		// Test state update - release
		state.Update(false, 0.1);
		runner.Assert("State not triggered on release", !state.IsTriggered(), "Triggered on release");
		runner.Assert("State not held on release", !state.IsHeld(), "Still held after release");
		runner.AssertEqualFloat("Held time reset", 0, state.GetHeldTime(), 0.001);
	}
}

// Console command to run input tests
class RBL_InputTestCommands
{
	static void RunInputTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_InputTests.RunAll();
		
		runner.PrintResults();
	}
	
	static void RunKeyCodesTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_InputTests.TestKeyCodesClass();
		
		runner.PrintResults();
	}
	
	static void RunBindingTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_InputTests.TestInputBindingRegistry();
		
		runner.PrintResults();
	}
	
	static void RunCooldownTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_InputTests.TestCooldownSystem();
		
		runner.PrintResults();
	}
}

