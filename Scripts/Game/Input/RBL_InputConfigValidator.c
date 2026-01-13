// ============================================================================
// PROJECT REBELLION - Input Config Validator
// Validates input configuration files and keybind integrity
// ============================================================================

// Validation result codes
enum ERBLInputValidationResult
{
	VALID,
	MISSING_ACTION,
	DUPLICATE_KEYBIND,
	INVALID_KEY_CODE,
	CONFIG_NOT_LOADED,
	CONTEXT_ERROR
}

// Validation issue details
class RBL_InputValidationIssue
{
	ERBLInputValidationResult Result;
	string ActionName;
	string Message;
	int InputKeyCode;
	
	void RBL_InputValidationIssue(ERBLInputValidationResult result, string actionName, string message, int keyCode = -1)
	{
		Result = result;
		ActionName = actionName;
		Message = message;
		InputKeyCode = keyCode;
	}
	
	string GetSeverity()
	{
		switch (Result)
		{
			case ERBLInputValidationResult.VALID: return "OK";
			case ERBLInputValidationResult.MISSING_ACTION: return "ERROR";
			case ERBLInputValidationResult.DUPLICATE_KEYBIND: return "WARNING";
			case ERBLInputValidationResult.INVALID_KEY_CODE: return "ERROR";
			case ERBLInputValidationResult.CONFIG_NOT_LOADED: return "ERROR";
			case ERBLInputValidationResult.CONTEXT_ERROR: return "ERROR";
		}
		return "UNKNOWN";
	}
}

// Expected actions for validation
class RBL_ExpectedInputActions
{
	// All actions that should be defined
	static ref array<string> GetRequiredActions()
	{
		array<string> actions = {};
		actions.Insert(RBL_InputActions.TOGGLE_SHOP);
		actions.Insert(RBL_InputActions.TOGGLE_SETTINGS);
		actions.Insert(RBL_InputActions.TOGGLE_HUD);
		actions.Insert(RBL_InputActions.QUICK_SAVE);
		actions.Insert(RBL_InputActions.QUICK_LOAD);
		actions.Insert(RBL_InputActions.TOGGLE_MAP);
		actions.Insert(RBL_InputActions.TOGGLE_MISSIONS);
		actions.Insert(RBL_InputActions.TOGGLE_DEBUG_HUD);
		actions.Insert(RBL_InputActions.CLOSE_MENU);
		actions.Insert(RBL_InputActions.INTERACT);
		return actions;
	}
	
	// Menu navigation actions
	static ref array<string> GetMenuActions()
	{
		array<string> actions = {};
		actions.Insert("RBL_MenuUp");
		actions.Insert("RBL_MenuDown");
		actions.Insert("RBL_MenuLeft");
		actions.Insert("RBL_MenuRight");
		actions.Insert("RBL_MenuSelect");
		actions.Insert("RBL_MenuBack");
		return actions;
	}
	
	// Get expected key code for action
	static int GetExpectedKeyCode(string actionName)
	{
		if (actionName == RBL_InputActions.TOGGLE_SHOP) return RBL_KeyCodes.KEY_J;
		if (actionName == RBL_InputActions.TOGGLE_SETTINGS) return RBL_KeyCodes.KEY_K;
		if (actionName == RBL_InputActions.TOGGLE_HUD) return RBL_KeyCodes.KEY_H;
		if (actionName == RBL_InputActions.QUICK_SAVE) return RBL_KeyCodes.KEY_F5;
		if (actionName == RBL_InputActions.QUICK_LOAD) return RBL_KeyCodes.KEY_F9;
		if (actionName == RBL_InputActions.TOGGLE_MAP) return RBL_KeyCodes.KEY_M;
		if (actionName == RBL_InputActions.TOGGLE_MISSIONS) return RBL_KeyCodes.KEY_L;
		if (actionName == RBL_InputActions.TOGGLE_DEBUG_HUD) return RBL_KeyCodes.KEY_F6;
		if (actionName == RBL_InputActions.CLOSE_MENU) return RBL_KeyCodes.KEY_ESC;
		if (actionName == RBL_InputActions.INTERACT) return RBL_KeyCodes.KEY_F;
		if (actionName == "RBL_MenuUp") return RBL_KeyCodes.KEY_W;
		if (actionName == "RBL_MenuDown") return RBL_KeyCodes.KEY_S;
		if (actionName == "RBL_MenuLeft") return RBL_KeyCodes.KEY_Q;
		if (actionName == "RBL_MenuRight") return RBL_KeyCodes.KEY_E;
		if (actionName == "RBL_MenuSelect") return RBL_KeyCodes.KEY_ENTER;
		if (actionName == "RBL_MenuBack") return RBL_KeyCodes.KEY_ESC;
		return -1;
	}
}

// Main validator class
class RBL_InputConfigValidator
{
	protected static ref RBL_InputConfigValidator s_Instance;
	
	protected ref array<ref RBL_InputValidationIssue> m_aIssues;
	protected bool m_bValidated;
	protected bool m_bIsValid;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_InputConfigValidator GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_InputConfigValidator();
		return s_Instance;
	}
	
	void RBL_InputConfigValidator()
	{
		m_aIssues = new array<ref RBL_InputValidationIssue>();
		m_bValidated = false;
		m_bIsValid = false;
	}
	
	// ========================================================================
	// VALIDATION
	// ========================================================================
	
	// Run full validation
	bool Validate()
	{
		m_aIssues.Clear();
		m_bValidated = true;
		m_bIsValid = true;
		
		PrintFormat("[RBL_InputValidator] Starting input configuration validation...");
		
		// Get registry
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (!registry || !registry.IsInitialized())
		{
			AddIssue(ERBLInputValidationResult.CONFIG_NOT_LOADED, "", "Input registry not initialized");
			m_bIsValid = false;
			return false;
		}
		
		// Validate required actions exist
		ValidateRequiredActions(registry);
		
		// Validate menu actions exist
		ValidateMenuActions(registry);
		
		// Check for duplicate keybinds
		ValidateNoDuplicates(registry);
		
		// Validate key codes are in valid range
		ValidateKeyCodes(registry);
		
		// Check InputManager availability
		ValidateInputManager();
		
		// Print summary
		PrintValidationSummary();
		
		return m_bIsValid;
	}
	
	protected void ValidateRequiredActions(RBL_InputBindingRegistry registry)
	{
		array<string> requiredActions = RBL_ExpectedInputActions.GetRequiredActions();
		
		foreach (string actionName : requiredActions)
		{
			RBL_Keybind binding = registry.GetBinding(actionName);
			if (!binding)
			{
				AddIssue(ERBLInputValidationResult.MISSING_ACTION, actionName, 
					string.Format("Required action '%1' is not registered", actionName));
				m_bIsValid = false;
			}
			else
			{
				// Verify key code matches expected
				int expectedKey = RBL_ExpectedInputActions.GetExpectedKeyCode(actionName);
				if (expectedKey > 0 && binding.CurrentKeyCode != expectedKey)
				{
					// This is just a warning, not a failure
					AddIssue(ERBLInputValidationResult.DUPLICATE_KEYBIND, actionName,
						string.Format("Action '%1' has key %2, expected %3", 
							actionName, 
							RBL_KeyCodes.GetKeyName(binding.CurrentKeyCode),
							RBL_KeyCodes.GetKeyName(expectedKey)),
						binding.CurrentKeyCode);
				}
			}
		}
	}
	
	protected void ValidateMenuActions(RBL_InputBindingRegistry registry)
	{
		// Menu actions are optional, just warn if missing
		array<string> menuActions = RBL_ExpectedInputActions.GetMenuActions();
		
		foreach (string actionName : menuActions)
		{
			RBL_Keybind binding = registry.GetBinding(actionName);
			if (!binding)
			{
				// Menu actions not in code registry is OK - they're in config only
				// This is informational, not an error
			}
		}
	}
	
	protected void ValidateNoDuplicates(RBL_InputBindingRegistry registry)
	{
		map<int, string> keyToAction = new map<int, string>();
		array<ref RBL_Keybind> bindings = {};
		registry.GetAllBindings(bindings);
		
		foreach (RBL_Keybind binding : bindings)
		{
			string existingAction;
			if (keyToAction.Find(binding.CurrentKeyCode, existingAction))
			{
				// ESC can be shared (CloseMenu and MenuBack)
				if (binding.CurrentKeyCode == RBL_KeyCodes.KEY_ESC)
					continue;
				
				AddIssue(ERBLInputValidationResult.DUPLICATE_KEYBIND, binding.ActionName,
					string.Format("Key %1 is bound to both '%2' and '%3'",
						RBL_KeyCodes.GetKeyName(binding.CurrentKeyCode),
						existingAction,
						binding.ActionName),
					binding.CurrentKeyCode);
				// Duplicates are warnings, not errors
			}
			else
			{
				keyToAction.Set(binding.CurrentKeyCode, binding.ActionName);
			}
		}
	}
	
	protected void ValidateKeyCodes(RBL_InputBindingRegistry registry)
	{
		array<ref RBL_Keybind> bindings = {};
		registry.GetAllBindings(bindings);
		
		foreach (RBL_Keybind binding : bindings)
		{
			if (binding.CurrentKeyCode < 1 || binding.CurrentKeyCode > 200)
			{
				AddIssue(ERBLInputValidationResult.INVALID_KEY_CODE, binding.ActionName,
					string.Format("Invalid key code %1 for action '%2'",
						binding.CurrentKeyCode, binding.ActionName),
					binding.CurrentKeyCode);
				m_bIsValid = false;
			}
		}
	}
	
	protected void ValidateInputManager()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		if (!inputMgr)
		{
			AddIssue(ERBLInputValidationResult.CONFIG_NOT_LOADED, "", 
				"Game InputManager not available - custom actions may not work");
			// Not marking as invalid since game might not be fully loaded
		}
	}
	
	// ========================================================================
	// ISSUE MANAGEMENT
	// ========================================================================
	
	protected void AddIssue(ERBLInputValidationResult result, string actionName, string message, int keyCode = -1)
	{
		RBL_InputValidationIssue issue = new RBL_InputValidationIssue(result, actionName, message, keyCode);
		m_aIssues.Insert(issue);
		
		string severity = issue.GetSeverity();
		PrintFormat("[RBL_InputValidator] [%1] %2", severity, message);
	}
	
	// ========================================================================
	// RESULTS
	// ========================================================================
	
	void PrintValidationSummary()
	{
		int errors = 0;
		int warnings = 0;
		
		foreach (RBL_InputValidationIssue issue : m_aIssues)
		{
			if (issue.Result == ERBLInputValidationResult.MISSING_ACTION || 
				issue.Result == ERBLInputValidationResult.INVALID_KEY_CODE ||
				issue.Result == ERBLInputValidationResult.CONFIG_NOT_LOADED)
			{
				errors++;
			}
			else if (issue.Result == ERBLInputValidationResult.DUPLICATE_KEYBIND)
			{
				warnings++;
			}
		}
		
		PrintFormat("\n========================================");
		PrintFormat("   INPUT CONFIG VALIDATION RESULTS");
		PrintFormat("========================================");
		PrintFormat("  Status: %1", m_bIsValid ? "VALID" : "INVALID");
		PrintFormat("  Errors: %1", errors);
		PrintFormat("  Warnings: %1", warnings);
		PrintFormat("========================================\n");
	}
	
	bool IsValid() { return m_bIsValid; }
	bool IsValidated() { return m_bValidated; }
	
	int GetErrorCount()
	{
		int count = 0;
		foreach (RBL_InputValidationIssue issue : m_aIssues)
		{
			if (issue.Result == ERBLInputValidationResult.MISSING_ACTION || 
				issue.Result == ERBLInputValidationResult.INVALID_KEY_CODE ||
				issue.Result == ERBLInputValidationResult.CONFIG_NOT_LOADED)
			{
				count++;
			}
		}
		return count;
	}
	
	int GetWarningCount()
	{
		int count = 0;
		foreach (RBL_InputValidationIssue issue : m_aIssues)
		{
			if (issue.Result == ERBLInputValidationResult.DUPLICATE_KEYBIND)
				count++;
		}
		return count;
	}
	
	int GetTotalIssueCount() { return m_aIssues.Count(); }
	
	array<ref RBL_InputValidationIssue> GetIssues() { return m_aIssues; }
	
	array<ref RBL_InputValidationIssue> GetErrors()
	{
		array<ref RBL_InputValidationIssue> errors = {};
		foreach (RBL_InputValidationIssue issue : m_aIssues)
		{
			if (issue.Result == ERBLInputValidationResult.MISSING_ACTION || 
				issue.Result == ERBLInputValidationResult.INVALID_KEY_CODE ||
				issue.Result == ERBLInputValidationResult.CONFIG_NOT_LOADED)
			{
				errors.Insert(issue);
			}
		}
		return errors;
	}
	
	array<ref RBL_InputValidationIssue> GetWarnings()
	{
		array<ref RBL_InputValidationIssue> warnings = {};
		foreach (RBL_InputValidationIssue issue : m_aIssues)
		{
			if (issue.Result == ERBLInputValidationResult.DUPLICATE_KEYBIND)
				warnings.Insert(issue);
		}
		return warnings;
	}
}

// ============================================================================
// CONSOLE COMMANDS FOR INPUT VALIDATION
// ============================================================================
class RBL_InputValidatorCommands
{
	// Run validation and print results
	static void ValidateInputConfig()
	{
		// Ensure registry is initialized
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		registry.Initialize();
		
		// Run validation
		RBL_InputConfigValidator validator = RBL_InputConfigValidator.GetInstance();
		validator.Validate();
	}
	
	// Print all registered keybinds
	static void PrintKeybinds()
	{
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (!registry || !registry.IsInitialized())
		{
			registry.Initialize();
		}
		
		PrintFormat("\n========================================");
		PrintFormat("   REGISTERED KEYBINDS");
		PrintFormat("========================================");
		
		array<ref RBL_Keybind> bindings = {};
		registry.GetAllBindings(bindings);
		
		foreach (RBL_Keybind binding : bindings)
		{
			PrintFormat("  [%1] %2 - %3",
				binding.GetKeyDisplayName(),
				binding.ActionName,
				binding.DisplayName);
		}
		
		PrintFormat("========================================");
		PrintFormat("  Total: %1 keybinds", bindings.Count());
		PrintFormat("========================================\n");
	}
	
	// Check if specific action is available
	static void CheckAction(string actionName)
	{
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (!registry || !registry.IsInitialized())
		{
			registry.Initialize();
		}
		
		RBL_Keybind binding = registry.GetBinding(actionName);
		if (binding)
		{
			PrintFormat("[RBL_Input] Action '%1' is registered:", actionName);
			PrintFormat("  Key: %1", binding.GetKeyDisplayName());
			PrintFormat("  Display: %1", binding.DisplayName);
			PrintFormat("  Cooldown: %1s", binding.Cooldown);
		}
		else
		{
			PrintFormat("[RBL_Input] Action '%1' is NOT registered", actionName);
		}
		
		// Also check InputManager
		InputManager inputMgr = GetGame().GetInputManager();
		if (inputMgr)
		{
			// Try triggering the action to see if it's recognized
			PrintFormat("[RBL_Input] InputManager is available");
		}
		else
		{
			PrintFormat("[RBL_Input] InputManager is NOT available");
		}
	}
	
	// Print expected vs actual keybinds
	static void CompareKeybinds()
	{
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (!registry || !registry.IsInitialized())
		{
			registry.Initialize();
		}
		
		PrintFormat("\n========================================");
		PrintFormat("   KEYBIND COMPARISON (Expected vs Actual)");
		PrintFormat("========================================");
		
		array<string> actions = RBL_ExpectedInputActions.GetRequiredActions();
		
		foreach (string actionName : actions)
		{
			int expectedKey = RBL_ExpectedInputActions.GetExpectedKeyCode(actionName);
			RBL_Keybind binding = registry.GetBinding(actionName);
			
			string expectedKeyName = RBL_KeyCodes.GetKeyName(expectedKey);
			string actualKeyName = binding ? binding.GetKeyDisplayName() : "NOT REGISTERED";
			
			string status = "OK";
			if (!binding)
				status = "MISSING";
			else if (binding.CurrentKeyCode != expectedKey)
				status = "MISMATCH";
			
			PrintFormat("  %1: Expected [%2], Actual [%3] - %4",
				actionName, expectedKeyName, actualKeyName, status);
		}
		
		PrintFormat("========================================\n");
	}
}


