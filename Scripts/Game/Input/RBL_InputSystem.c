// ============================================================================
// PROJECT REBELLION - Input System
// Centralized input management with custom keybinds
// ============================================================================

// Input action definitions - maps to RBL_InputActions.conf
class RBL_InputActions
{
	// Custom Project Rebellion actions (defined in RBL_InputActions.conf)
	static const string TOGGLE_SHOP = "RBL_ToggleShop";
	static const string TOGGLE_HUD = "RBL_ToggleHUD";
	static const string QUICK_SAVE = "RBL_QuickSave";
	static const string QUICK_LOAD = "RBL_QuickLoad";
	static const string TOGGLE_MAP = "RBL_ToggleMap";
	static const string TOGGLE_MISSIONS = "RBL_ToggleMissions";
	static const string TOGGLE_DEBUG_HUD = "RBL_ToggleDebugHUD";
	static const string CLOSE_MENU = "RBL_CloseMenu";
	static const string INTERACT = "RBL_Interact";
	
	// Fallback to vanilla actions if custom not available
	static const string FALLBACK_SHOP = "CharacterInspect";
	static const string FALLBACK_MAP = "ToggleMap";
	static const string FALLBACK_ESCAPE = "MenuOpen";
	static const string FALLBACK_USE = "CharacterAction";
}

// Input context names
class RBL_InputContexts
{
	static const string GAMEPLAY = "RBLInputContext";
	static const string MENU = "RBLMenuContext";
}

// Key codes for Arma Reforger keyboard
class RBL_KeyCodes
{
	static const int KEY_ESC = 1;
	static const int KEY_1 = 2;
	static const int KEY_2 = 3;
	static const int KEY_3 = 4;
	static const int KEY_4 = 5;
	static const int KEY_5 = 6;
	static const int KEY_6 = 7;
	static const int KEY_7 = 8;
	static const int KEY_8 = 9;
	static const int KEY_9 = 10;
	static const int KEY_0 = 11;
	static const int KEY_Q = 16;
	static const int KEY_W = 17;
	static const int KEY_E = 18;
	static const int KEY_R = 19;
	static const int KEY_T = 20;
	static const int KEY_Y = 21;
	static const int KEY_U = 22;
	static const int KEY_I = 23;
	static const int KEY_O = 24;
	static const int KEY_P = 25;
	static const int KEY_A = 30;
	static const int KEY_S = 31;
	static const int KEY_D = 32;
	static const int KEY_F = 33;
	static const int KEY_G = 34;
	static const int KEY_H = 35;
	static const int KEY_J = 36;
	static const int KEY_K = 37;
	static const int KEY_L = 38;
	static const int KEY_Z = 44;
	static const int KEY_X = 45;
	static const int KEY_C = 46;
	static const int KEY_V = 47;
	static const int KEY_B = 48;
	static const int KEY_N = 49;
	static const int KEY_M = 50;
	static const int KEY_F1 = 59;
	static const int KEY_F2 = 60;
	static const int KEY_F3 = 61;
	static const int KEY_F4 = 62;
	static const int KEY_F5 = 63;
	static const int KEY_F6 = 64;
	static const int KEY_F7 = 65;
	static const int KEY_F8 = 66;
	static const int KEY_F9 = 67;
	static const int KEY_F10 = 68;
	static const int KEY_F11 = 87;
	static const int KEY_F12 = 88;
	static const int KEY_TAB = 15;
	static const int KEY_SPACE = 57;
	static const int KEY_ENTER = 28;
	static const int KEY_LSHIFT = 42;
	static const int KEY_RSHIFT = 54;
	static const int KEY_LCTRL = 29;
	static const int KEY_RCTRL = 157;
	static const int KEY_LALT = 56;
	static const int KEY_RALT = 184;
	
	// Get display name for key code
	static string GetKeyName(int keyCode)
	{
		switch (keyCode)
		{
			case KEY_ESC: return "ESC";
			case KEY_1: return "1";
			case KEY_2: return "2";
			case KEY_3: return "3";
			case KEY_4: return "4";
			case KEY_5: return "5";
			case KEY_6: return "6";
			case KEY_7: return "7";
			case KEY_8: return "8";
			case KEY_9: return "9";
			case KEY_0: return "0";
			case KEY_Q: return "Q";
			case KEY_W: return "W";
			case KEY_E: return "E";
			case KEY_R: return "R";
			case KEY_T: return "T";
			case KEY_Y: return "Y";
			case KEY_U: return "U";
			case KEY_I: return "I";
			case KEY_O: return "O";
			case KEY_P: return "P";
			case KEY_A: return "A";
			case KEY_S: return "S";
			case KEY_D: return "D";
			case KEY_F: return "F";
			case KEY_G: return "G";
			case KEY_H: return "H";
			case KEY_J: return "J";
			case KEY_K: return "K";
			case KEY_L: return "L";
			case KEY_Z: return "Z";
			case KEY_X: return "X";
			case KEY_C: return "C";
			case KEY_V: return "V";
			case KEY_B: return "B";
			case KEY_N: return "N";
			case KEY_M: return "M";
			case KEY_F1: return "F1";
			case KEY_F2: return "F2";
			case KEY_F3: return "F3";
			case KEY_F4: return "F4";
			case KEY_F5: return "F5";
			case KEY_F6: return "F6";
			case KEY_F7: return "F7";
			case KEY_F8: return "F8";
			case KEY_F9: return "F9";
			case KEY_F10: return "F10";
			case KEY_F11: return "F11";
			case KEY_F12: return "F12";
			case KEY_TAB: return "TAB";
			case KEY_SPACE: return "SPACE";
			case KEY_ENTER: return "ENTER";
			case KEY_LSHIFT: return "SHIFT";
			case KEY_RSHIFT: return "SHIFT";
			case KEY_LCTRL: return "CTRL";
			case KEY_RCTRL: return "CTRL";
			case KEY_LALT: return "ALT";
			case KEY_RALT: return "ALT";
			default: return "?";
		}
	}
}

// Keybind definition for display and lookup
class RBL_Keybind
{
	string ActionName;
	string DisplayName;
	int DefaultKeyCode;
	int CurrentKeyCode;
	float Cooldown;
	
	void RBL_Keybind(string actionName, string displayName, int defaultKeyCode, float cooldown = 0.3)
	{
		ActionName = actionName;
		DisplayName = displayName;
		DefaultKeyCode = defaultKeyCode;
		CurrentKeyCode = defaultKeyCode;
		Cooldown = cooldown;
	}
	
	string GetKeyDisplayName()
	{
		return RBL_KeyCodes.GetKeyName(CurrentKeyCode);
	}
}

// Input binding registry - tracks all keybinds and their states
class RBL_InputBindingRegistry
{
	protected static ref RBL_InputBindingRegistry s_Instance;
	
	protected ref map<string, ref RBL_Keybind> m_mBindings;
	protected ref map<string, float> m_mCooldowns;
	protected bool m_bInitialized;
	protected bool m_bUsingCustomActions;
	
	static RBL_InputBindingRegistry GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_InputBindingRegistry();
		return s_Instance;
	}
	
	void RBL_InputBindingRegistry()
	{
		m_mBindings = new map<string, ref RBL_Keybind>();
		m_mCooldowns = new map<string, float>();
		m_bInitialized = false;
		m_bUsingCustomActions = false;
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		// Register all Project Rebellion keybinds
		RegisterDefaultBindings();
		
		// Check if custom actions are available
		m_bUsingCustomActions = CheckCustomActionsAvailable();
		
		m_bInitialized = true;
		
		if (m_bUsingCustomActions)
			PrintFormat("[RBL_Input] Initialized with custom input actions");
		else
			PrintFormat("[RBL_Input] Initialized with fallback input actions");
	}
	
	protected void RegisterDefaultBindings()
	{
		// Shop toggle - J key
		RegisterBinding(new RBL_Keybind(
			RBL_InputActions.TOGGLE_SHOP,
			"Toggle Shop",
			RBL_KeyCodes.KEY_J,
			0.3
		));
		
		// HUD toggle - H key
		RegisterBinding(new RBL_Keybind(
			RBL_InputActions.TOGGLE_HUD,
			"Toggle HUD",
			RBL_KeyCodes.KEY_H,
			0.3
		));
		
		// Quick save - F5
		RegisterBinding(new RBL_Keybind(
			RBL_InputActions.QUICK_SAVE,
			"Quick Save",
			RBL_KeyCodes.KEY_F5,
			1.0
		));
		
		// Quick load - F9
		RegisterBinding(new RBL_Keybind(
			RBL_InputActions.QUICK_LOAD,
			"Quick Load",
			RBL_KeyCodes.KEY_F9,
			1.0
		));
		
		// Map toggle - M key
		RegisterBinding(new RBL_Keybind(
			RBL_InputActions.TOGGLE_MAP,
			"Toggle Map",
			RBL_KeyCodes.KEY_M,
			0.3
		));
		
		// Mission log - L key
		RegisterBinding(new RBL_Keybind(
			RBL_InputActions.TOGGLE_MISSIONS,
			"Toggle Missions",
			RBL_KeyCodes.KEY_L,
			0.3
		));
		
		// Debug HUD - F6
		RegisterBinding(new RBL_Keybind(
			RBL_InputActions.TOGGLE_DEBUG_HUD,
			"Toggle Debug HUD",
			RBL_KeyCodes.KEY_F6,
			0.3
		));
		
		// Close menu - ESC
		RegisterBinding(new RBL_Keybind(
			RBL_InputActions.CLOSE_MENU,
			"Close Menu",
			RBL_KeyCodes.KEY_ESC,
			0.1
		));
		
		// Interact - F key
		RegisterBinding(new RBL_Keybind(
			RBL_InputActions.INTERACT,
			"Interact",
			RBL_KeyCodes.KEY_F,
			0.2
		));
	}
	
	protected bool CheckCustomActionsAvailable()
	{
		InputManager inputMgr = GetGame().GetInputManager();
		if (!inputMgr)
			return false;
		
		// Try to check if our custom action exists
		// If InputManager doesn't recognize it, we'll use fallbacks
		return true; // Assume true, will fall back gracefully in handler
	}
	
	void RegisterBinding(RBL_Keybind binding)
	{
		if (binding && binding.ActionName.Length() > 0)
		{
			m_mBindings.Set(binding.ActionName, binding);
			m_mCooldowns.Set(binding.ActionName, 0);
		}
	}
	
	RBL_Keybind GetBinding(string actionName)
	{
		RBL_Keybind binding;
		m_mBindings.Find(actionName, binding);
		return binding;
	}
	
	string GetKeyDisplayForAction(string actionName)
	{
		RBL_Keybind binding = GetBinding(actionName);
		if (binding)
			return binding.GetKeyDisplayName();
		return "?";
	}
	
	float GetCooldown(string actionName)
	{
		float cooldown;
		if (m_mCooldowns.Find(actionName, cooldown))
			return cooldown;
		return 0;
	}
	
	void SetCooldown(string actionName, float value)
	{
		m_mCooldowns.Set(actionName, value);
	}
	
	void UpdateCooldowns(float timeSlice)
	{
		foreach (string actionName, float cooldown : m_mCooldowns)
		{
			if (cooldown > 0)
				m_mCooldowns.Set(actionName, cooldown - timeSlice);
		}
	}
	
	bool IsOnCooldown(string actionName)
	{
		return GetCooldown(actionName) > 0;
	}
	
	void TriggerCooldown(string actionName)
	{
		RBL_Keybind binding = GetBinding(actionName);
		if (binding)
			SetCooldown(actionName, binding.Cooldown);
	}
	
	bool IsUsingCustomActions() { return m_bUsingCustomActions; }
	bool IsInitialized() { return m_bInitialized; }
	
	// Get all registered bindings for UI display
	void GetAllBindings(notnull array<ref RBL_Keybind> outBindings)
	{
		foreach (string actionName, RBL_Keybind binding : m_mBindings)
		{
			outBindings.Insert(binding);
		}
	}
	
	int GetBindingCount()
	{
		return m_mBindings.Count();
	}
}

// Input state tracker for action triggers
class RBL_InputState
{
	protected bool m_bTriggered;
	protected bool m_bHeld;
	protected float m_fHeldTime;
	
	void RBL_InputState()
	{
		m_bTriggered = false;
		m_bHeld = false;
		m_fHeldTime = 0;
	}
	
	void Update(bool isActive, float timeSlice)
	{
		bool wasHeld = m_bHeld;
		m_bHeld = isActive;
		
		m_bTriggered = isActive && !wasHeld;
		
		if (m_bHeld)
			m_fHeldTime += timeSlice;
		else
			m_fHeldTime = 0;
	}
	
	bool IsTriggered() { return m_bTriggered; }
	bool IsHeld() { return m_bHeld; }
	float GetHeldTime() { return m_fHeldTime; }
}

// Input action event for callbacks
class RBL_InputActionEvent
{
	string ActionName;
	int KeyCode;
	bool IsPressed;
	float Timestamp;
	
	void RBL_InputActionEvent(string actionName, int keyCode, bool isPressed)
	{
		ActionName = actionName;
		KeyCode = keyCode;
		IsPressed = isPressed;
		Timestamp = System.GetTickCount() / 1000.0;
	}
}

// Main input manager - singleton
class RBL_InputManager
{
	protected static ref RBL_InputManager s_Instance;
	
	protected ref RBL_InputBindingRegistry m_Registry;
	protected ref map<string, ref RBL_InputState> m_mActionStates;
	
	protected bool m_bInitialized;
	protected bool m_bEnabled;
	protected bool m_bMenuOpen;
	
	// Events
	protected ref ScriptInvoker m_OnActionTriggered;
	protected ref ScriptInvoker m_OnMenuStateChanged;
	
	static RBL_InputManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_InputManager();
		return s_Instance;
	}
	
	void RBL_InputManager()
	{
		m_Registry = RBL_InputBindingRegistry.GetInstance();
		m_mActionStates = new map<string, ref RBL_InputState>();
		m_OnActionTriggered = new ScriptInvoker();
		m_OnMenuStateChanged = new ScriptInvoker();
		m_bInitialized = false;
		m_bEnabled = true;
		m_bMenuOpen = false;
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		m_Registry.Initialize();
		
		// Initialize action states
		array<ref RBL_Keybind> bindings = {};
		m_Registry.GetAllBindings(bindings);
		
		foreach (RBL_Keybind binding : bindings)
		{
			m_mActionStates.Set(binding.ActionName, new RBL_InputState());
		}
		
		m_bInitialized = true;
		PrintFormat("[RBL_Input] Input Manager initialized with %1 bindings", m_Registry.GetBindingCount());
	}
	
	void Update(float timeSlice)
	{
		if (!m_bInitialized || !m_bEnabled)
			return;
		
		// Update cooldowns
		m_Registry.UpdateCooldowns(timeSlice);
		
		// Get input manager
		InputManager inputMgr = GetGame().GetInputManager();
		if (!inputMgr)
			return;
		
		// Process each registered action
		ProcessAction(inputMgr, RBL_InputActions.TOGGLE_SHOP, RBL_InputActions.FALLBACK_SHOP);
		ProcessAction(inputMgr, RBL_InputActions.TOGGLE_HUD, null);
		ProcessAction(inputMgr, RBL_InputActions.QUICK_SAVE, null);
		ProcessAction(inputMgr, RBL_InputActions.QUICK_LOAD, null);
		ProcessAction(inputMgr, RBL_InputActions.TOGGLE_MAP, RBL_InputActions.FALLBACK_MAP);
		ProcessAction(inputMgr, RBL_InputActions.TOGGLE_MISSIONS, null);
		ProcessAction(inputMgr, RBL_InputActions.TOGGLE_DEBUG_HUD, null);
		ProcessAction(inputMgr, RBL_InputActions.CLOSE_MENU, RBL_InputActions.FALLBACK_ESCAPE);
		ProcessAction(inputMgr, RBL_InputActions.INTERACT, RBL_InputActions.FALLBACK_USE);
	}
	
	protected void ProcessAction(InputManager inputMgr, string actionName, string fallbackAction)
	{
		bool triggered = false;
		
		// Try custom action first
		if (inputMgr.GetActionTriggered(actionName))
			triggered = true;
		// Try fallback if available
		else if (fallbackAction && inputMgr.GetActionTriggered(fallbackAction))
			triggered = true;
		
		if (triggered && !m_Registry.IsOnCooldown(actionName))
		{
			m_Registry.TriggerCooldown(actionName);
			HandleActionTriggered(actionName);
		}
	}
	
	protected void HandleActionTriggered(string actionName)
	{
		RBL_Keybind binding = m_Registry.GetBinding(actionName);
		int keyCode = binding ? binding.CurrentKeyCode : 0;
		
		RBL_InputActionEvent evt = new RBL_InputActionEvent(actionName, keyCode, true);
		m_OnActionTriggered.Invoke(evt);
		
		// Handle built-in actions
		switch (actionName)
		{
			case RBL_InputActions.TOGGLE_SHOP:
				OnToggleShop();
				break;
			case RBL_InputActions.TOGGLE_HUD:
				OnToggleHUD();
				break;
			case RBL_InputActions.QUICK_SAVE:
				OnQuickSave();
				break;
			case RBL_InputActions.QUICK_LOAD:
				OnQuickLoad();
				break;
			case RBL_InputActions.TOGGLE_DEBUG_HUD:
				OnToggleDebugHUD();
				break;
			case RBL_InputActions.CLOSE_MENU:
				OnCloseMenu();
				break;
		}
	}
	
	// Action handlers
	protected void OnToggleShop()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			RBL_ShopMenuWidgetImpl shopMenu = uiMgr.GetShopMenu();
			if (shopMenu)
			{
				shopMenu.Toggle();
				SetMenuOpen(shopMenu.IsVisible());
				PrintFormat("[RBL_Input] Shop %1", shopMenu.IsVisible() ? "opened" : "closed");
			}
		}
		
		// Also try legacy shop
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.ToggleMenu();
	}
	
	protected void OnToggleHUD()
	{
		RBL_HUDManager hud = RBL_HUDManager.GetInstance();
		if (hud)
			hud.ToggleVisibility();
		
		PrintFormat("[RBL_Input] HUD toggled");
	}
	
	protected void OnQuickSave()
	{
		RBL_PersistenceIntegration persistence = RBL_PersistenceIntegration.GetInstance();
		if (persistence)
		{
			persistence.QuickSave();
			RBL_Notifications.GameSaved();
			PrintFormat("[RBL_Input] Quick save triggered");
		}
	}
	
	protected void OnQuickLoad()
	{
		RBL_PersistenceIntegration persistence = RBL_PersistenceIntegration.GetInstance();
		if (persistence)
		{
			persistence.QuickLoad();
			PrintFormat("[RBL_Input] Quick load triggered");
		}
	}
	
	protected void OnToggleDebugHUD()
	{
		RBL_ScreenHUD screenHUD = RBL_ScreenHUD.GetInstance();
		if (screenHUD)
			screenHUD.Toggle();
		
		PrintFormat("[RBL_Input] Debug HUD toggled");
	}
	
	protected void OnCloseMenu()
	{
		if (m_bMenuOpen)
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
			{
				RBL_ShopMenuWidgetImpl shopMenu = uiMgr.GetShopMenu();
				if (shopMenu && shopMenu.IsVisible())
				{
					shopMenu.Close();
					SetMenuOpen(false);
					PrintFormat("[RBL_Input] Menu closed via ESC");
				}
			}
		}
	}
	
	// State management
	void SetMenuOpen(bool open)
	{
		bool changed = m_bMenuOpen != open;
		m_bMenuOpen = open;
		
		if (changed)
			m_OnMenuStateChanged.Invoke(open);
	}
	
	bool IsMenuOpen() { return m_bMenuOpen; }
	bool IsEnabled() { return m_bEnabled; }
	void SetEnabled(bool enabled) { m_bEnabled = enabled; }
	bool IsInitialized() { return m_bInitialized; }
	
	// Getters
	RBL_InputBindingRegistry GetRegistry() { return m_Registry; }
	ScriptInvoker GetOnActionTriggered() { return m_OnActionTriggered; }
	ScriptInvoker GetOnMenuStateChanged() { return m_OnMenuStateChanged; }
	
	// Utility - get key display name for action
	string GetKeyForAction(string actionName)
	{
		return m_Registry.GetKeyDisplayForAction(actionName);
	}
}

// Legacy compatibility wrapper - forwards to new system
class RBL_InputConfig
{
	static const string ACTION_SHOP = "RBL_ToggleShop";
	static const string ACTION_MAP = "RBL_ToggleMap";
	static const string ACTION_INVENTORY = "Inventory";
	static const string ACTION_ESCAPE = "RBL_CloseMenu";
	static const string ACTION_USE = "RBL_Interact";
	static const string ACTION_QUICK_SAVE = "RBL_QuickSave";
	static const string ACTION_QUICK_LOAD = "RBL_QuickLoad";
	
	static const float SHOP_COOLDOWN = 0.3;
	static const float SAVE_COOLDOWN = 1.0;
	
	// Helper to get display key from action name
	static string GetKeyDisplay(string actionName)
	{
		RBL_InputManager mgr = RBL_InputManager.GetInstance();
		if (mgr)
			return mgr.GetKeyForAction(actionName);
		return "?";
	}
}

