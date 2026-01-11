// ============================================================================
// PROJECT REBELLION - Input Handler
// Handles keybinds for HUD, Shop, etc.
// ============================================================================

class RBL_InputHandler
{
	protected static ref RBL_InputHandler s_Instance;
	
	protected const string ACTION_TOGGLE_SHOP = "RBL_ToggleShop";
	protected const string ACTION_TOGGLE_HUD = "RBL_ToggleHUD";
	protected const string ACTION_DEBUG_STATUS = "RBL_DebugStatus";
	
	static RBL_InputHandler GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_InputHandler();
		return s_Instance;
	}
	
	void RBL_InputHandler()
	{
		PrintFormat("[RBL_Input] Input handler initialized");
		PrintFormat("[RBL_Input] Press J to toggle shop (in console)");
		PrintFormat("[RBL_Input] Press K to show debug status");
	}
	
	void Update()
	{
		// Manual key checking since custom actions need config
		InputManager inputMgr = GetGame().GetInputManager();
		if (!inputMgr)
			return;
		
		// J key for shop
		if (inputMgr.GetActionTriggered("CharacterInspect"))
		{
			RBL_ShopManager shop = RBL_ShopManager.GetInstance();
			if (shop)
				shop.ToggleMenu();
		}
	}
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
		}
		else
		{
			PrintFormat("[RBL_Debug] Zone not found: %1", zoneID);
		}
	}
}

