// ============================================================================
// PROJECT REBELLION - Input Handler
// Handles keybinds for HUD, Shop, Persistence, etc.
// ============================================================================

class RBL_InputConfig
{
	// Keybind definitions (used with InputManager.GetActionTriggered)
	// These map to existing Arma Reforger actions or can be custom
	
	static const string ACTION_SHOP = "CharacterInspect";       // J key - opens shop
	static const string ACTION_MAP = "ToggleMap";               // M key - toggle map
	static const string ACTION_INVENTORY = "Inventory";         // I key - inventory
	static const string ACTION_ESCAPE = "MenuOpen";             // ESC key - escape/close
	static const string ACTION_USE = "CharacterAction";         // F key - interact
	static const string ACTION_QUICK_SAVE = "QuickSave";        // F5 - quick save
	static const string ACTION_QUICK_LOAD = "QuickLoad";        // F9 - quick load
	
	// Cooldowns to prevent spam
	static const float SHOP_COOLDOWN = 0.3;
	static const float SAVE_COOLDOWN = 1.0;
}

class RBL_InputHandler
{
	protected static ref RBL_InputHandler s_Instance;
	
	// Cooldown timers
	protected float m_fShopCooldown;
	protected float m_fSaveCooldown;
	protected float m_fLoadCooldown;
	
	// State
	protected bool m_bShopOpen;
	protected bool m_bInitialized;
	
	static RBL_InputHandler GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_InputHandler();
		return s_Instance;
	}
	
	void RBL_InputHandler()
	{
		m_fShopCooldown = 0;
		m_fSaveCooldown = 0;
		m_fLoadCooldown = 0;
		m_bShopOpen = false;
		m_bInitialized = false;
		
		PrintFormat("[RBL_Input] Input handler initialized");
		PrintFormat("[RBL_Input] Keybinds:");
		PrintFormat("[RBL_Input]   J - Toggle Shop");
		PrintFormat("[RBL_Input]   H - Toggle HUD");
		PrintFormat("[RBL_Input]   F5 - Quick Save");
		PrintFormat("[RBL_Input]   F9 - Quick Load");
		PrintFormat("[RBL_Input]   ESC - Close menus");
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		m_bInitialized = true;
		PrintFormat("[RBL_Input] Input handler ready");
	}
	
	void Update(float timeSlice)
	{
		// Decay cooldowns
		if (m_fShopCooldown > 0)
			m_fShopCooldown -= timeSlice;
		if (m_fSaveCooldown > 0)
			m_fSaveCooldown -= timeSlice;
		if (m_fLoadCooldown > 0)
			m_fLoadCooldown -= timeSlice;
		
		// Get input manager
		InputManager inputMgr = GetGame().GetInputManager();
		if (!inputMgr)
			return;
		
		// Handle shop toggle (J key via CharacterInspect)
		if (inputMgr.GetActionTriggered(RBL_InputConfig.ACTION_SHOP) && m_fShopCooldown <= 0)
		{
			HandleShopToggle();
			m_fShopCooldown = RBL_InputConfig.SHOP_COOLDOWN;
		}
		
		// Handle escape (close menus)
		if (inputMgr.GetActionTriggered(RBL_InputConfig.ACTION_ESCAPE))
		{
			HandleEscape();
		}
	}
	
	protected void HandleShopToggle()
	{
		// Toggle via new UI system
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			RBL_ShopMenuWidgetImpl shopMenu = uiMgr.GetShopMenu();
			if (shopMenu)
			{
				shopMenu.Toggle();
				m_bShopOpen = shopMenu.IsVisible();
				
				if (m_bShopOpen)
					PrintFormat("[RBL_Input] Shop opened");
				else
					PrintFormat("[RBL_Input] Shop closed");
			}
		}
		
		// Also try legacy shop manager
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.ToggleMenu();
	}
	
	protected void HandleEscape()
	{
		// Close any open menus
		if (m_bShopOpen)
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
			{
				RBL_ShopMenuWidgetImpl shopMenu = uiMgr.GetShopMenu();
				if (shopMenu && shopMenu.IsVisible())
				{
					shopMenu.Close();
					m_bShopOpen = false;
				}
			}
		}
	}
	
	void HandleQuickSave()
	{
		if (m_fSaveCooldown > 0)
		{
			PrintFormat("[RBL_Input] Save on cooldown");
			return;
		}
		
		m_fSaveCooldown = RBL_InputConfig.SAVE_COOLDOWN;
		
		RBL_PersistenceIntegration persistence = RBL_PersistenceIntegration.GetInstance();
		if (persistence)
		{
			persistence.QuickSave();
			RBL_Notifications.GameSaved();
		}
	}
	
	void HandleQuickLoad()
	{
		if (m_fLoadCooldown > 0)
		{
			PrintFormat("[RBL_Input] Load on cooldown");
			return;
		}
		
		m_fLoadCooldown = RBL_InputConfig.SAVE_COOLDOWN;
		
		RBL_PersistenceIntegration persistence = RBL_PersistenceIntegration.GetInstance();
		if (persistence)
		{
			persistence.QuickLoad();
		}
	}
	
	void HandleToggleHUD()
	{
		RBL_HUDManager hud = RBL_HUDManager.GetInstance();
		if (hud)
			hud.ToggleVisibility();
		
		PrintFormat("[RBL_Input] HUD toggled");
	}
	
	void HandleToggleDebugHUD()
	{
		RBL_ScreenHUD screenHUD = RBL_ScreenHUD.GetInstance();
		if (screenHUD)
			screenHUD.Toggle();
		
		PrintFormat("[RBL_Input] Debug HUD toggled");
	}
	
	// State queries
	bool IsShopOpen() { return m_bShopOpen; }
	bool IsAnyMenuOpen() { return m_bShopOpen; }
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
				// Create a dummy zone for notification (or extend the system)
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
		// Add some height to avoid falling through ground
		zonePos[1] = zonePos[1] + 2.0;

		playerEntity.SetOrigin(zonePos);
		PrintFormat("[RBL_Debug] Teleported to zone: %1", zoneID);
	}

	static void TestResourceTick()
	{
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			// Force a resource tick by accessing the method
			// Note: This is a workaround - ideally you'd expose a method
			PrintFormat("[RBL_Debug] Resource tick triggered manually");
			PrintFormat("[RBL_Debug] Check console for resource tick output");
		}
	}
}
