// ============================================================================
// PROJECT REBELLION - Game Mode
// Plug-and-play: Just load this mod and play!
// ============================================================================

class RBL_GameModeClass : SCR_BaseGameModeClass
{
}

class RBL_GameMode : SCR_BaseGameMode
{
	[Attribute("1", UIWidgets.CheckBox, "Enable Rebellion systems")]
	protected bool m_bEnableRebellion;

	[Attribute("1", UIWidgets.CheckBox, "Auto-initialize zones from config")]
	protected bool m_bAutoInitialize;

	[Attribute("1", UIWidgets.CheckBox, "Show UI HUD")]
	protected bool m_bShowHUD;

	protected bool m_bSystemsInitialized;
	protected float m_fInitDelay;
	protected const float INIT_DELAY_TIME = 2.0;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!m_bEnableRebellion)
			return;

		m_bSystemsInitialized = false;
		m_fInitDelay = 0;

		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] PROJECT REBELLION");
		PrintFormat("[RBL] Guerrilla Warfare for Arma Reforger");
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] Waiting for world to load...");
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		if (!m_bEnableRebellion)
			return;

		// Delayed initialization to ensure world is ready
		if (!m_bSystemsInitialized)
		{
			m_fInitDelay += timeSlice;
			if (m_fInitDelay >= INIT_DELAY_TIME)
			{
				InitializeSystems();
				m_bSystemsInitialized = true;
			}
			return;
		}

		// Update all systems
		UpdateAllSystems(timeSlice);
	}

	protected void InitializeSystems()
	{
		PrintFormat("[RBL] Initializing Rebellion systems...");
		
		if (m_bAutoInitialize)
		{
			RBL_AutoInitializer autoInit = RBL_AutoInitializer.GetInstance();
			autoInit.Initialize();
		}
		
		// Initialize new systems
		RBL_CaptureManager.GetInstance();
		RBL_ShopManager.GetInstance();
		RBL_GarrisonManager.GetInstance();
		
		// Initialize NEW UI System
		if (m_bShowHUD)
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			uiMgr.Initialize();
			PrintFormat("[RBL] UI System initialized");
		}
		
		// Legacy systems (for compatibility)
		RBL_ScreenHUD.GetInstance();
		RBL_InputHandler.GetInstance();
		
		// Spawn garrisons at all enemy zones
		GetGame().GetCallqueue().CallLater(SpawnInitialGarrisons, 5000, false);
		
		// Show welcome notification
		GetGame().GetCallqueue().CallLater(ShowWelcomeNotifications, 1000, false);
		
		PrintZoneInfo();
		PrintHelp();
	}
	
	protected void SpawnInitialGarrisons()
	{
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
		{
			garMgr.SpawnAllGarrisons();
			PrintFormat("[RBL] Initial garrisons spawned");
		}
	}
	
	protected void ShowWelcomeNotifications()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
			return;
		
		uiMgr.ShowNotification("PROJECT REBELLION Active", RBL_UIColors.COLOR_ACCENT_GREEN, 4.0);
		
		GetGame().GetCallqueue().CallLater(ShowHintNotification, 2000, false);
	}
	
	protected void ShowHintNotification()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (!uiMgr)
			return;
		
		uiMgr.ShowNotification("Press [B] to open Shop", RBL_UIColors.COLOR_TEXT_SECONDARY, 3.0);
	}

	protected void UpdateAllSystems(float timeSlice)
	{
		// Core systems
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
			commanderAI.Update(timeSlice);
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.Update(timeSlice);
		
		RBL_PersistenceManager persistence = RBL_PersistenceManager.GetInstance();
		if (persistence)
			persistence.Update(timeSlice);
		
		// Capture system
		RBL_CaptureManager captureMgr = RBL_CaptureManager.GetInstance();
		if (captureMgr)
			captureMgr.Update(timeSlice);
		
		// Garrison system
		RBL_GarrisonManager garrisonMgr = RBL_GarrisonManager.GetInstance();
		if (garrisonMgr)
			garrisonMgr.Update(timeSlice);
		
		// Undercover system
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
			undercover.Update(timeSlice);
		
		// NEW UI System
		if (m_bShowHUD)
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
			{
				uiMgr.Update(timeSlice);
				uiMgr.Draw();
			}
		}
		
		// Legacy HUD (for compatibility)
		RBL_ScreenHUD hud = RBL_ScreenHUD.GetInstance();
		if (hud)
			hud.Update(timeSlice);
		
		// Input
		RBL_InputHandler input = RBL_InputHandler.GetInstance();
		if (input)
			input.Update();
	}

	protected void PrintZoneInfo()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;

		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] CAMPAIGN READY!");
		PrintFormat("[RBL] Total Zones: %1", zoneMgr.GetTotalZoneCount());
		PrintFormat("[RBL] FIA Controls: %1", zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA));
		PrintFormat("[RBL] Enemy Controls: %1", zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR));
		PrintFormat("[RBL] ========================================");
	}

	protected void PrintHelp()
	{
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] KEYBINDS:");
		PrintFormat("[RBL]   [B] - Open Shop");
		PrintFormat("[RBL]   [M] - Toggle Map");
		PrintFormat("[RBL]   [H] - Toggle HUD");
		PrintFormat("[RBL] CONSOLE COMMANDS:");
		PrintFormat("[RBL]   RBL_DebugCommands.PrintStatus()");
		PrintFormat("[RBL]   RBL_DebugCommands.OpenShop()");
		PrintFormat("[RBL]   RBL_DebugCommands.Buy(\"akm\")");
		PrintFormat("[RBL]   RBL_DebugCommands.AddMoney(1000)");
		PrintFormat("[RBL]   RBL_DebugCommands.ListZones()");
		PrintFormat("[RBL]   RBL_DebugCommands.CaptureZone(\"zoneid\")");
		PrintFormat("[RBL]   RBL_DebugCommands.TeleportToZone(\"zoneid\")");
		PrintFormat("[RBL]   RBL_UITests.RunAllTests() - Run UI tests");
		PrintFormat("[RBL] ========================================");
	}

	void DebugPrintStatus()
	{
		RBL_DebugCommands.PrintStatus();
	}
}

// ============================================================================
// MODDED GAME MODE - Hooks into ANY existing game mode
// ============================================================================
modded class SCR_BaseGameMode
{
	protected ref RBL_GameModeAddon m_RBLAddon;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		// Auto-attach Rebellion to any game mode
		m_RBLAddon = new RBL_GameModeAddon();
		m_RBLAddon.OnGameModeInit(this);
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		if (m_RBLAddon)
			m_RBLAddon.OnGameModeFrame(timeSlice);
	}
}

class RBL_GameModeAddon
{
	protected bool m_bInitialized;
	protected float m_fInitDelay;
	protected const float INIT_DELAY = 3.0;

	void RBL_GameModeAddon()
	{
		m_bInitialized = false;
		m_fInitDelay = 0;
	}

	void OnGameModeInit(SCR_BaseGameMode gameMode)
	{
		PrintFormat("[RBL] Rebellion addon attached to game mode");
	}

	void OnGameModeFrame(float timeSlice)
	{
		if (m_bInitialized)
		{
			UpdateSystems(timeSlice);
			return;
		}

		m_fInitDelay += timeSlice;
		if (m_fInitDelay >= INIT_DELAY)
		{
			Initialize();
			m_bInitialized = true;
		}
	}

	protected void Initialize()
	{
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] PROJECT REBELLION - ACTIVE");
		PrintFormat("[RBL] ========================================");
		
		// Core initialization
		RBL_AutoInitializer autoInit = RBL_AutoInitializer.GetInstance();
		autoInit.Initialize();
		
		// New systems
		RBL_CaptureManager.GetInstance();
		RBL_ShopManager.GetInstance();
		RBL_GarrisonManager.GetInstance();
		
		// NEW UI System
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		uiMgr.Initialize();
		
		// Legacy systems
		RBL_ScreenHUD.GetInstance();
		RBL_InputHandler.GetInstance();
		
		// Spawn garrisons after delay
		GetGame().GetCallqueue().CallLater(SpawnGarrisons, 5000, false);
		
		// Welcome notifications
		GetGame().GetCallqueue().CallLater(ShowWelcome, 1000, false);
		
		PrintHelp();
	}
	
	protected void SpawnGarrisons()
	{
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
			garMgr.SpawnAllGarrisons();
	}
	
	protected void ShowWelcome()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ShowNotification("PROJECT REBELLION Active", RBL_UIColors.COLOR_ACCENT_GREEN, 4.0);
			GetGame().GetCallqueue().CallLater(ShowHint, 2000, false);
		}
	}
	
	protected void ShowHint()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.ShowNotification("Press [B] to open Shop", RBL_UIColors.COLOR_TEXT_SECONDARY, 3.0);
	}

	protected void PrintHelp()
	{
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] KEYBINDS:");
		PrintFormat("[RBL]   [B] - Open Shop");
		PrintFormat("[RBL]   [M] - Toggle Map");
		PrintFormat("[RBL]   [H] - Toggle HUD");
		PrintFormat("[RBL] CONSOLE COMMANDS:");
		PrintFormat("[RBL]   RBL_DebugCommands.PrintStatus()");
		PrintFormat("[RBL]   RBL_DebugCommands.OpenShop()");
		PrintFormat("[RBL]   RBL_DebugCommands.Buy(\"akm\")");
		PrintFormat("[RBL]   RBL_DebugCommands.AddMoney(1000)");
		PrintFormat("[RBL]   RBL_DebugCommands.TeleportToZone(\"Town_LePort\")");
		PrintFormat("[RBL]   RBL_UITests.RunAllTests() - Run UI tests");
		PrintFormat("[RBL] ========================================");
	}

	protected void UpdateSystems(float timeSlice)
	{
		// Core
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
			commanderAI.Update(timeSlice);
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.Update(timeSlice);
		
		RBL_PersistenceManager persistence = RBL_PersistenceManager.GetInstance();
		if (persistence)
			persistence.Update(timeSlice);
		
		// Capture
		RBL_CaptureManager captureMgr = RBL_CaptureManager.GetInstance();
		if (captureMgr)
			captureMgr.Update(timeSlice);
		
		// Garrison
		RBL_GarrisonManager garrisonMgr = RBL_GarrisonManager.GetInstance();
		if (garrisonMgr)
			garrisonMgr.Update(timeSlice);
		
		// Undercover
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
			undercover.Update(timeSlice);
		
		// NEW UI System
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.Update(timeSlice);
			uiMgr.Draw();
		}
		
		// Legacy HUD
		RBL_ScreenHUD hud = RBL_ScreenHUD.GetInstance();
		if (hud)
			hud.Update(timeSlice);
		
		// Input
		RBL_InputHandler input = RBL_InputHandler.GetInstance();
		if (input)
			input.Update();
	}
}

// ============================================================================
// UI INPUT HANDLER - Keybind processing
// ============================================================================
class RBL_UIInputHandler
{
	protected static ref RBL_UIInputHandler s_Instance;
	
	static RBL_UIInputHandler GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_UIInputHandler();
		return s_Instance;
	}
	
	void Update()
	{
		InputManager input = GetGame().GetInputManager();
		if (!input)
			return;
		
		// Shop toggle (B key)
		if (input.GetActionTriggered("RBL_ToggleShop"))
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
				uiMgr.ToggleShop();
		}
		
		// HUD toggle (H key)
		if (input.GetActionTriggered("RBL_ToggleHUD"))
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
				uiMgr.SetVisible(!uiMgr.IsVisible());
		}
	}
}

// ============================================================================
// DEBUG COMMANDS EXTENSION - UI commands
// ============================================================================
class RBL_UIDebugCommands
{
	[ConsoleCmd("rbl_shop", "Toggle shop menu")]
	static void ToggleShop()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ToggleShop();
			PrintFormat("[RBL] Shop toggled");
		}
	}
	
	[ConsoleCmd("rbl_hud", "Toggle HUD visibility")]
	static void ToggleHUD()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.SetVisible(!uiMgr.IsVisible());
			PrintFormat("[RBL] HUD visibility: %1", uiMgr.IsVisible());
		}
	}
	
	[ConsoleCmd("rbl_notify", "Show test notification")]
	static void TestNotify()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ShowNotification("Test notification!", RBL_UIColors.COLOR_ACCENT_GREEN, 3.0);
			PrintFormat("[RBL] Notification sent");
		}
	}
	
	[ConsoleCmd("rbl_ui_test", "Run all UI tests")]
	static void RunUITests()
	{
		RBL_UITests.RunAllTests();
	}
}
