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
		TestConfigValidator();
		TestExpectedActions();
		TestValidationIssues();
		
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
	
	// Test config validator
	static void TestConfigValidator()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing config validator...");
		
		// Initialize registry first
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		registry.Initialize();
		
		// Get validator singleton
		RBL_InputConfigValidator validator = RBL_InputConfigValidator.GetInstance();
		runner.AssertNotNull("Validator singleton exists", validator);
		
		if (!validator)
			return;
		
		// Test initial state
		runner.Assert("Validator not validated initially", !validator.IsValidated(), "Already validated");
		
		// Run validation
		bool isValid = validator.Validate();
		
		// Test post-validation state
		runner.Assert("Validator marked as validated", validator.IsValidated(), "Not marked validated");
		runner.Assert("Validation ran without crash", true, "Validation crashed");
		
		// Test issue tracking
		int totalIssues = validator.GetTotalIssueCount();
		int errors = validator.GetErrorCount();
		int warnings = validator.GetWarningCount();
		
		runner.Assert("Total issues count is non-negative", totalIssues >= 0, "Negative issue count");
		runner.Assert("Error count is non-negative", errors >= 0, "Negative error count");
		runner.Assert("Warning count is non-negative", warnings >= 0, "Negative warning count");
		runner.Assert("Issues breakdown matches total", errors + warnings <= totalIssues, "Issue count mismatch");
		
		// Test GetIssues returns array
		array<ref RBL_InputValidationIssue> issues = validator.GetIssues();
		runner.AssertNotNull("GetIssues returns array", issues);
		runner.AssertEqual("GetIssues count matches", totalIssues, issues.Count());
		
		// Test GetErrors returns array
		array<ref RBL_InputValidationIssue> errorList = validator.GetErrors();
		runner.AssertNotNull("GetErrors returns array", errorList);
		runner.AssertEqual("GetErrors count matches", errors, errorList.Count());
		
		// Test GetWarnings returns array
		array<ref RBL_InputValidationIssue> warningList = validator.GetWarnings();
		runner.AssertNotNull("GetWarnings returns array", warningList);
		runner.AssertEqual("GetWarnings count matches", warnings, warningList.Count());
		
		// Log results
		PrintFormat("[RBL_InputTests] Validation result: %1 (Errors: %2, Warnings: %3)", 
			isValid ? "VALID" : "INVALID", errors, warnings);
	}
	
	// Test expected actions configuration
	static void TestExpectedActions()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing expected actions configuration...");
		
		// Test GetRequiredActions returns array
		array<string> requiredActions = RBL_ExpectedInputActions.GetRequiredActions();
		runner.AssertNotNull("GetRequiredActions returns array", requiredActions);
		runner.Assert("Required actions count > 0", requiredActions.Count() > 0, "No required actions");
		runner.Assert("At least 9 required actions", requiredActions.Count() >= 9, 
			string.Format("Only %1 required actions", requiredActions.Count()));
		
		// Test GetMenuActions returns array
		array<string> menuActions = RBL_ExpectedInputActions.GetMenuActions();
		runner.AssertNotNull("GetMenuActions returns array", menuActions);
		runner.Assert("Menu actions count > 0", menuActions.Count() > 0, "No menu actions");
		runner.Assert("At least 6 menu actions", menuActions.Count() >= 6,
			string.Format("Only %1 menu actions", menuActions.Count()));
		
		// Test expected key codes
		runner.AssertEqual("Shop expected key is J", RBL_KeyCodes.KEY_J, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.TOGGLE_SHOP));
		runner.AssertEqual("Settings expected key is K", RBL_KeyCodes.KEY_K, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.TOGGLE_SETTINGS));
		runner.AssertEqual("HUD expected key is H", RBL_KeyCodes.KEY_H, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.TOGGLE_HUD));
		runner.AssertEqual("QuickSave expected key is F5", RBL_KeyCodes.KEY_F5, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.QUICK_SAVE));
		runner.AssertEqual("QuickLoad expected key is F9", RBL_KeyCodes.KEY_F9, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.QUICK_LOAD));
		runner.AssertEqual("Map expected key is M", RBL_KeyCodes.KEY_M, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.TOGGLE_MAP));
		runner.AssertEqual("Missions expected key is L", RBL_KeyCodes.KEY_L, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.TOGGLE_MISSIONS));
		runner.AssertEqual("DebugHUD expected key is F6", RBL_KeyCodes.KEY_F6, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.TOGGLE_DEBUG_HUD));
		runner.AssertEqual("CloseMenu expected key is ESC", RBL_KeyCodes.KEY_ESC, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.CLOSE_MENU));
		runner.AssertEqual("Interact expected key is F", RBL_KeyCodes.KEY_F, 
			RBL_ExpectedInputActions.GetExpectedKeyCode(RBL_InputActions.INTERACT));
		
		// Test menu navigation expected keys
		runner.AssertEqual("MenuUp expected key is W", RBL_KeyCodes.KEY_W, 
			RBL_ExpectedInputActions.GetExpectedKeyCode("RBL_MenuUp"));
		runner.AssertEqual("MenuDown expected key is S", RBL_KeyCodes.KEY_S, 
			RBL_ExpectedInputActions.GetExpectedKeyCode("RBL_MenuDown"));
		runner.AssertEqual("MenuLeft expected key is Q", RBL_KeyCodes.KEY_Q, 
			RBL_ExpectedInputActions.GetExpectedKeyCode("RBL_MenuLeft"));
		runner.AssertEqual("MenuRight expected key is E", RBL_KeyCodes.KEY_E, 
			RBL_ExpectedInputActions.GetExpectedKeyCode("RBL_MenuRight"));
		runner.AssertEqual("MenuSelect expected key is ENTER", RBL_KeyCodes.KEY_ENTER, 
			RBL_ExpectedInputActions.GetExpectedKeyCode("RBL_MenuSelect"));
		runner.AssertEqual("MenuBack expected key is ESC", RBL_KeyCodes.KEY_ESC, 
			RBL_ExpectedInputActions.GetExpectedKeyCode("RBL_MenuBack"));
		
		// Test unknown action returns -1
		runner.AssertEqual("Unknown action returns -1", -1, 
			RBL_ExpectedInputActions.GetExpectedKeyCode("UnknownAction"));
	}
	
	// Test validation issue class
	static void TestValidationIssues()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_InputTests] Testing validation issue class...");
		
		// Test issue creation with different result types
		RBL_InputValidationIssue validIssue = new RBL_InputValidationIssue(
			ERBLInputValidationResult.VALID, "TestAction", "All good");
		runner.AssertNotNull("Valid issue created", validIssue);
		runner.AssertStringEqual("Valid issue severity is OK", "OK", validIssue.GetSeverity());
		runner.AssertEqual("Valid issue result is VALID", ERBLInputValidationResult.VALID, validIssue.Result);
		
		RBL_InputValidationIssue missingIssue = new RBL_InputValidationIssue(
			ERBLInputValidationResult.MISSING_ACTION, "MissingAction", "Action not found");
		runner.AssertNotNull("Missing issue created", missingIssue);
		runner.AssertStringEqual("Missing issue severity is ERROR", "ERROR", missingIssue.GetSeverity());
		runner.AssertStringEqual("Missing issue action name", "MissingAction", missingIssue.ActionName);
		runner.AssertStringEqual("Missing issue message", "Action not found", missingIssue.Message);
		
		RBL_InputValidationIssue duplicateIssue = new RBL_InputValidationIssue(
			ERBLInputValidationResult.DUPLICATE_KEYBIND, "DupeAction", "Duplicate key", RBL_KeyCodes.KEY_J);
		runner.AssertNotNull("Duplicate issue created", duplicateIssue);
		runner.AssertStringEqual("Duplicate issue severity is WARNING", "WARNING", duplicateIssue.GetSeverity());
		runner.AssertEqual("Duplicate issue key code", RBL_KeyCodes.KEY_J, duplicateIssue.KeyCode);
		
		RBL_InputValidationIssue invalidKeyIssue = new RBL_InputValidationIssue(
			ERBLInputValidationResult.INVALID_KEY_CODE, "BadKeyAction", "Invalid key code", 999);
		runner.AssertNotNull("Invalid key issue created", invalidKeyIssue);
		runner.AssertStringEqual("Invalid key issue severity is ERROR", "ERROR", invalidKeyIssue.GetSeverity());
		
		RBL_InputValidationIssue configIssue = new RBL_InputValidationIssue(
			ERBLInputValidationResult.CONFIG_NOT_LOADED, "", "Config not loaded");
		runner.AssertNotNull("Config issue created", configIssue);
		runner.AssertStringEqual("Config issue severity is ERROR", "ERROR", configIssue.GetSeverity());
		
		RBL_InputValidationIssue contextIssue = new RBL_InputValidationIssue(
			ERBLInputValidationResult.CONTEXT_ERROR, "ContextAction", "Context error");
		runner.AssertNotNull("Context issue created", contextIssue);
		runner.AssertStringEqual("Context issue severity is ERROR", "ERROR", contextIssue.GetSeverity());
		
		// Test default key code (-1) when not provided
		runner.AssertEqual("Valid issue default key code is -1", -1, validIssue.KeyCode);
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
	
	static void RunValidatorTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_InputTests.TestConfigValidator();
		RBL_InputTests.TestExpectedActions();
		RBL_InputTests.TestValidationIssues();
		
		runner.PrintResults();
	}
	
	static void ValidateAndTest()
	{
		PrintFormat("\n========================================");
		PrintFormat("   INPUT VALIDATION + TESTS");
		PrintFormat("========================================\n");
		
		// First run validation
		RBL_InputValidatorCommands.ValidateInputConfig();
		
		// Then run tests
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_InputTests.TestConfigValidator();
		RBL_InputTests.TestExpectedActions();
		RBL_InputTests.TestValidationIssues();
		
		runner.PrintResults();
		
		PrintFormat("\n========================================");
		PrintFormat("   VALIDATION + TESTS COMPLETE");
		PrintFormat("========================================\n");
	}
}

