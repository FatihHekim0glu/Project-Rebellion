// ============================================================================
// PROJECT REBELLION - Shop Manager
// Central shop/arsenal system - buy weapons, equipment, vehicles, recruit units
// ============================================================================

class RBL_ShopManager
{
	protected static ref RBL_ShopManager s_Instance;
	
	// Shop state
	protected bool m_bMenuOpen;
	protected bool m_bInitialized;
	
	// Item catalog organized by category
	protected ref array<ref RBL_ShopCategory> m_aCategories;
	
	// Quick lookup maps
	protected ref map<string, ref RBL_ShopItem> m_mItemsByID;
	protected ref map<string, int> m_mVehiclePrices;
	
	// Events
	protected ref ScriptInvoker m_OnMenuOpened;
	protected ref ScriptInvoker m_OnMenuClosed;
	protected ref ScriptInvoker m_OnPurchase;
	protected ref ScriptInvoker m_OnPurchaseFailed;
	
	// Singleton
	static RBL_ShopManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_ShopManager();
		return s_Instance;
	}
	
	void RBL_ShopManager()
	{
		m_bMenuOpen = false;
		m_bInitialized = false;
		
		m_aCategories = new array<ref RBL_ShopCategory>();
		m_mItemsByID = new map<string, ref RBL_ShopItem>();
		m_mVehiclePrices = new map<string, int>();
		
		m_OnMenuOpened = new ScriptInvoker();
		m_OnMenuClosed = new ScriptInvoker();
		m_OnPurchase = new ScriptInvoker();
		m_OnPurchaseFailed = new ScriptInvoker();
		
		Initialize();
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		InitializeCategories();
		BuildLookupMaps();
		
		m_bInitialized = true;
		
		PrintFormat("[RBL_Shop] Initialized: %1 categories, %2 total items",
			m_aCategories.Count(), m_mItemsByID.Count());
	}
	
	protected void InitializeCategories()
	{
		// ====== WEAPONS ======
		RBL_ShopCategory weapons = new RBL_ShopCategory("weapons", "WEAPONS", ERBLShopItemType.WEAPON);
		
		weapons.AddItem(RBL_ShopItemCatalog.CreateWeapon(
			"makarov", "PM Makarov", "Compact 9mm pistol",
			50, "{3E413771E1834D2E}Prefabs/Weapons/Handguns/PM/Weapon_PM.et"));
		
		weapons.AddItem(RBL_ShopItemCatalog.CreateWeapon(
			"akm", "AKM", "Reliable 7.62mm assault rifle",
			120, "{1BC151E0D4DE3D99}Prefabs/Weapons/Rifles/AKM/Weapon_AKM.et"));
		
		weapons.AddItem(RBL_ShopItemCatalog.CreateWeapon(
			"ak74", "AK-74", "Standard 5.45mm assault rifle",
			150, "{9B3DFBAE74363E7A}Prefabs/Weapons/Rifles/AK74/Weapon_AK74.et"));
		
		weapons.AddItem(RBL_ShopItemCatalog.CreateWeapon(
			"svd", "SVD Dragunov", "Semi-auto marksman rifle",
			400, "{3E36C5A1DDAE0CF8}Prefabs/Weapons/Rifles/SVD/Weapon_SVD.et", 3));
		
		weapons.AddItem(RBL_ShopItemCatalog.CreateWeapon(
			"pkm", "PKM", "General purpose machine gun",
			600, "{5C3AD3CD9F747118}Prefabs/Weapons/MachineGuns/PKM/Weapon_PKM.et", 4));
		
		weapons.AddItem(RBL_ShopItemCatalog.CreateWeapon(
			"rpg7", "RPG-7", "Anti-tank rocket launcher",
			800, "{519E924C1C8C5FB4}Prefabs/Weapons/Launchers/RPG7/Weapon_RPG7.et", 5));
		
		m_aCategories.Insert(weapons);
		
		// ====== EQUIPMENT ======
		RBL_ShopCategory equipment = new RBL_ShopCategory("equipment", "EQUIPMENT", ERBLShopItemType.EQUIPMENT);
		
		equipment.AddItem(RBL_ShopItemCatalog.CreateEquipment(
			"vest_light", "Light Vest", "Basic ballistic protection",
			100, "{VestLight}"));
		
		equipment.AddItem(RBL_ShopItemCatalog.CreateEquipment(
			"vest_heavy", "Heavy Vest", "Enhanced ballistic protection",
			300, "{VestHeavy}", 3));
		
		equipment.AddItem(RBL_ShopItemCatalog.CreateEquipment(
			"helmet", "Combat Helmet", "Head protection",
			150, "{Helmet}"));
		
		equipment.AddItem(RBL_ShopItemCatalog.CreateEquipment(
			"binoculars", "Binoculars", "Long range observation",
			75, "{Binoculars}"));
		
		equipment.AddItem(RBL_ShopItemCatalog.CreateEquipment(
			"nvg", "Night Vision", "NVG goggles for night ops",
			500, "{NVG}", 5));
		
		equipment.AddItem(RBL_ShopItemCatalog.CreateEquipment(
			"radio", "Radio", "Team communication device",
			100, "{Radio}"));
		
		m_aCategories.Insert(equipment);
		
		// ====== SUPPLIES ======
		RBL_ShopCategory supplies = new RBL_ShopCategory("supplies", "SUPPLIES", ERBLShopItemType.SUPPLY);
		
		supplies.AddItem(RBL_ShopItemCatalog.CreateSupply(
			"bandage", "Bandage", "Basic medical supply",
			15, "Bandage"));
		
		supplies.AddItem(RBL_ShopItemCatalog.CreateSupply(
			"medkit", "Medical Kit", "Full healing supplies",
			75, "MedKit"));
		
		supplies.AddItem(RBL_ShopItemCatalog.CreateSupply(
			"ammo_rifle", "Rifle Ammo", "5.45/7.62mm magazines",
			20, "AmmoRifle"));
		
		supplies.AddItem(RBL_ShopItemCatalog.CreateSupply(
			"ammo_pistol", "Pistol Ammo", "9mm magazines",
			10, "AmmoPistol"));
		
		supplies.AddItem(RBL_ShopItemCatalog.CreateSupply(
			"grenade_frag", "F1 Grenade", "Fragmentation grenade",
			50, "RGD5"));
		
		supplies.AddItem(RBL_ShopItemCatalog.CreateSupply(
			"grenade_smoke", "Smoke Grenade", "Smoke cover grenade",
			30, "SmokeGrenade"));
		
		m_aCategories.Insert(supplies);
		
		// ====== VEHICLES ======
		RBL_ShopCategory vehicles = new RBL_ShopCategory("vehicles", "VEHICLES", ERBLShopItemType.VEHICLE);
		
		vehicles.AddItem(RBL_ShopItemCatalog.CreateVehicle(
			"uaz", "UAZ-469", "Light utility vehicle",
			500, "{5E74787BB083789B}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469.et"));
		
		vehicles.AddItem(RBL_ShopItemCatalog.CreateVehicle(
			"ural", "Ural Truck", "Heavy transport truck",
			800, "{91B01E6C0D20E1D1}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320.et", 2));
		
		vehicles.AddItem(RBL_ShopItemCatalog.CreateVehicle(
			"brdm", "BRDM-2", "Armored scout car",
			1500, "{BRDM2}", 4));
		
		vehicles.AddItem(RBL_ShopItemCatalog.CreateVehicle(
			"btr70", "BTR-70", "Armored personnel carrier",
			3000, "{D85A504DF4E2C128}Prefabs/Vehicles/Wheeled/BTR70/BTR70.et", 5));
		
		vehicles.AddItem(RBL_ShopItemCatalog.CreateVehicle(
			"bmp1", "BMP-1", "Infantry fighting vehicle",
			5000, "{BB0E7CE42F0F3E19}Prefabs/Vehicles/Tracked/BMP1/BMP1.et", 7));
		
		m_aCategories.Insert(vehicles);
		
		// ====== RECRUITMENT ======
		RBL_ShopCategory recruits = new RBL_ShopCategory("recruits", "RECRUITMENT", ERBLShopItemType.RECRUIT);
		
		recruits.AddItem(RBL_ShopItemCatalog.CreateRecruit(
			"militia", "Militia", "Basic resistance fighter",
			100, 1, "{MilitiaRifleman}"));
		
		recruits.AddItem(RBL_ShopItemCatalog.CreateRecruit(
			"rifleman", "Rifleman", "Trained infantry soldier",
			200, 1, "{Rifleman}"));
		
		recruits.AddItem(RBL_ShopItemCatalog.CreateRecruit(
			"medic", "Combat Medic", "Field medical support",
			300, 1, "{Medic}"));
		
		recruits.AddItem(RBL_ShopItemCatalog.CreateRecruit(
			"marksman", "Marksman", "Long range specialist",
			400, 1, "{Marksman}"));
		
		recruits.AddItem(RBL_ShopItemCatalog.CreateRecruit(
			"mg_soldier", "MG Gunner", "Machine gun support",
			450, 1, "{MGunner}"));
		
		recruits.AddItem(RBL_ShopItemCatalog.CreateRecruit(
			"at_soldier", "AT Specialist", "Anti-tank specialist",
			500, 1, "{ATSpecialist}"));
		
		recruits.AddItem(RBL_ShopItemCatalog.CreateRecruit(
			"squad", "Fire Team", "4-man tactical squad",
			600, 4, "{FireTeam}"));
		
		m_aCategories.Insert(recruits);
	}
	
	protected void BuildLookupMaps()
	{
		m_mItemsByID.Clear();
		m_mVehiclePrices.Clear();
		
		foreach (RBL_ShopCategory category : m_aCategories)
		{
			array<ref RBL_ShopItem> items = category.GetItems();
			foreach (RBL_ShopItem item : items)
			{
				m_mItemsByID.Set(item.GetID(), item);
				
				// Build vehicle price lookup
				if (item.GetType() == ERBLShopItemType.VEHICLE)
				{
					m_mVehiclePrices.Set(item.GetID(), item.GetPrice());
				}
			}
		}
	}
	
	// ========================================================================
	// MENU CONTROL
	// ========================================================================
	
	void ToggleMenu()
	{
		if (m_bMenuOpen)
			CloseMenu();
		else
			OpenMenu();
	}
	
	void OpenMenu()
	{
		m_bMenuOpen = true;
		m_OnMenuOpened.Invoke();
		PrintFormat("[RBL_Shop] Menu opened");
	}
	
	void CloseMenu()
	{
		m_bMenuOpen = false;
		m_OnMenuClosed.Invoke();
		PrintFormat("[RBL_Shop] Menu closed");
	}
	
	bool IsMenuOpen() { return m_bMenuOpen; }
	
	// ========================================================================
	// ITEM LOOKUP
	// ========================================================================
	
	RBL_ShopItem GetItemByID(string itemID)
	{
		RBL_ShopItem item;
		m_mItemsByID.Find(itemID, item);
		return item;
	}
	
	RBL_ShopItem FindItem(string itemID)
	{
		return GetItemByID(itemID);
	}
	
	bool ItemExists(string itemID)
	{
		return m_mItemsByID.Contains(itemID);
	}
	
	// ========================================================================
	// PRICE LOOKUP
	// ========================================================================
	
	int GetItemPrice(string itemID)
	{
		RBL_ShopItem item = GetItemByID(itemID);
		if (item)
			return item.GetPrice();
		return 0;
	}
	
	int GetVehiclePrice(string vehicleType)
	{
		int price;
		if (m_mVehiclePrices.Find(vehicleType, price))
			return price;
		return 0;
	}
	
	int GetWeaponPrice(string weaponID)
	{
		return GetItemPrice(weaponID);
	}
	
	int GetEquipmentPrice(string equipID)
	{
		return GetItemPrice(equipID);
	}
	
	int GetRecruitPrice(string recruitID)
	{
		return GetItemPrice(recruitID);
	}
	
	int GetRecruitHRCost(string recruitID)
	{
		RBL_ShopItem item = GetItemByID(recruitID);
		if (item)
			return item.GetHRCost();
		return 0;
	}
	
	// ========================================================================
	// PURCHASING
	// ========================================================================
	
	bool PurchaseItem(string itemID)
	{
		RBL_ShopItem item = GetItemByID(itemID);
		if (!item)
		{
			PrintFormat("[RBL_Shop] Item not found: %1", itemID);
			return false;
		}
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
			return false;
		
		// Get current war level
		int warLevel = 1;
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (campMgr)
			warLevel = campMgr.GetWarLevel();
		
		// Check requirements
		if (!item.CanPurchase(econMgr.GetMoney(), econMgr.GetHR(), warLevel))
		{
			string reason = item.GetUnavailableReason(econMgr.GetMoney(), econMgr.GetHR(), warLevel);
			PrintFormat("[RBL_Shop] Cannot purchase %1: %2", item.GetDisplayName(), reason);
			
			// Show appropriate notification
			if (reason.Contains("funds"))
				RBL_Notifications.InsufficientFunds();
			else if (reason.Contains("HR"))
				RBL_Notifications.InsufficientHR();
			else
				RBL_Notifications.Show(reason);
			
			m_OnPurchaseFailed.Invoke(item, reason);
			return false;
		}
		
		// In multiplayer, send purchase request to server
		if (!RBL_NetworkUtils.IsSinglePlayer())
		{
			RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
			if (netMgr)
			{
				int playerID = RBL_NetworkUtils.GetLocalPlayerID();
				netMgr.RequestPurchase(playerID, itemID, item.GetPrice(), item.GetHRCost());
				PrintFormat("[RBL_Shop] Purchase request sent to server: %1", item.GetDisplayName());
				return true;
			}
		}
		
		// Singleplayer: process locally
		return ProcessLocalPurchase(item);
	}
	
	protected bool ProcessLocalPurchase(RBL_ShopItem item)
	{
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
			return false;
		
		// Get player ID for delivery
		int playerID = RBL_NetworkUtils.GetLocalPlayerID();
		if (playerID < 0)
		{
			PrintFormat("[RBL_Shop] No local player found");
			return false;
		}
		
		// Deliver the item
		RBL_ItemDelivery delivery = RBL_ItemDelivery.GetInstance();
		ERBLDeliveryResult result = ERBLDeliveryResult.FAILED_NO_PLAYER;
		
		if (delivery)
			result = delivery.DeliverShopItem(item, playerID);
		
		// Only deduct money if delivery succeeded
		if (result == ERBLDeliveryResult.SUCCESS)
		{
			econMgr.SpendMoney(item.GetPrice());
			if (item.GetHRCost() > 0)
				econMgr.SpendHR(item.GetHRCost());
			
			m_OnPurchase.Invoke(item);
			RBL_Notifications.ItemPurchased(item.GetDisplayName(), item.GetPrice());
			
			PrintFormat("[RBL_Shop] *** PURCHASED: %1 for $%2 ***", item.GetDisplayName(), item.GetPrice());
			PrintFormat("[RBL_Shop] Remaining: $%1, HR: %2", econMgr.GetMoney(), econMgr.GetHR());
			
			return true;
		}
		else
		{
			PrintFormat("[RBL_Shop] Purchase failed - delivery error: %1", 
				typename.EnumToString(ERBLDeliveryResult, result));
			RBL_Notifications.DeliveryFailed(item.GetDisplayName());
			m_OnPurchaseFailed.Invoke(item, "Delivery failed");
			return false;
		}
	}
	
	// Quick purchase alias
	bool Buy(string itemID)
	{
		return PurchaseItem(itemID);
	}
	
	// Server-side purchase processing (called by NetworkManager)
	bool ProcessServerPurchase(int playerID, string itemID)
	{
		if (!RBL_NetworkUtils.IsServer())
			return false;
		
		RBL_ShopItem item = GetItemByID(itemID);
		if (!item)
			return false;
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
			return false;
		
		if (!econMgr.CanAffordPurchase(item.GetPrice(), item.GetHRCost()))
			return false;
		
		// Deduct money
		econMgr.SpendMoney(item.GetPrice());
		if (item.GetHRCost() > 0)
			econMgr.SpendHR(item.GetHRCost());
		
		// Deliver to player
		RBL_ItemDelivery delivery = RBL_ItemDelivery.GetInstance();
		if (delivery)
		{
			ERBLDeliveryResult result = delivery.DeliverShopItem(item, playerID);
			if (result == ERBLDeliveryResult.SUCCESS)
			{
				m_OnPurchase.Invoke(item);
				return true;
			}
		}
		
		return false;
	}
	
	// ========================================================================
	// CATEGORY ACCESS
	// ========================================================================
	
	array<ref RBL_ShopCategory> GetCategories()
	{
		return m_aCategories;
	}
	
	RBL_ShopCategory GetCategoryByID(string categoryID)
	{
		foreach (RBL_ShopCategory category : m_aCategories)
		{
			if (category.GetID() == categoryID)
				return category;
		}
		return null;
	}
	
	RBL_ShopCategory GetCategoryByIndex(int index)
	{
		if (index >= 0 && index < m_aCategories.Count())
			return m_aCategories[index];
		return null;
	}
	
	int GetCategoryCount()
	{
		return m_aCategories.Count();
	}
	
	// ========================================================================
	// ITEM LIST ACCESS (Legacy compatibility)
	// ========================================================================
	
	array<ref RBL_ShopItem> GetWeapons()
	{
		RBL_ShopCategory cat = GetCategoryByID("weapons");
		if (cat)
			return cat.GetItems();
		return new array<ref RBL_ShopItem>();
	}
	
	array<ref RBL_ShopItem> GetEquipment()
	{
		RBL_ShopCategory cat = GetCategoryByID("equipment");
		if (cat)
			return cat.GetItems();
		return new array<ref RBL_ShopItem>();
	}
	
	array<ref RBL_ShopItem> GetSupplies()
	{
		RBL_ShopCategory cat = GetCategoryByID("supplies");
		if (cat)
			return cat.GetItems();
		return new array<ref RBL_ShopItem>();
	}
	
	array<ref RBL_ShopItem> GetVehicles()
	{
		RBL_ShopCategory cat = GetCategoryByID("vehicles");
		if (cat)
			return cat.GetItems();
		return new array<ref RBL_ShopItem>();
	}
	
	array<ref RBL_ShopItem> GetRecruits()
	{
		RBL_ShopCategory cat = GetCategoryByID("recruits");
		if (cat)
			return cat.GetItems();
		return new array<ref RBL_ShopItem>();
	}
	
	// ========================================================================
	// EVENTS
	// ========================================================================
	
	ScriptInvoker GetOnMenuOpened() { return m_OnMenuOpened; }
	ScriptInvoker GetOnMenuClosed() { return m_OnMenuClosed; }
	ScriptInvoker GetOnPurchase() { return m_OnPurchase; }
	ScriptInvoker GetOnPurchaseFailed() { return m_OnPurchaseFailed; }
	
	// ========================================================================
	// DEBUG
	// ========================================================================
	
	void PrintShopToConsole()
	{
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		int money = 0;
		int hr = 0;
		if (econMgr)
		{
			money = econMgr.GetMoney();
			hr = econMgr.GetHR();
		}
		
		int warLevel = 1;
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (campMgr)
			warLevel = campMgr.GetWarLevel();
		
		PrintFormat("========================================");
		PrintFormat("        PROJECT REBELLION SHOP");
		PrintFormat("   Your funds: $%1 | HR: %2 | War Level: %3", money, hr, warLevel);
		PrintFormat("========================================");
		
		foreach (RBL_ShopCategory category : m_aCategories)
		{
			PrintFormat("\n--- %1 ---", category.GetDisplayName());
			
			array<ref RBL_ShopItem> items = category.GetItems();
			foreach (RBL_ShopItem item : items)
			{
				string status = "";
				if (item.CanPurchase(money, hr, warLevel))
					status = "[CAN BUY]";
				else
					status = "[" + item.GetUnavailableReason(money, hr, warLevel) + "]";
				
				string priceStr = "$" + item.GetPrice().ToString();
				if (item.GetHRCost() > 0)
					priceStr = priceStr + " + " + item.GetHRCost().ToString() + "HR";
				
				PrintFormat("  %1: %2 - %3 %4", item.GetID(), item.GetDisplayName(), priceStr, status);
			}
		}
		
		PrintFormat("\n========================================");
		PrintFormat("  To buy: RBL_ShopManager.GetInstance().Buy(\"itemid\")");
		PrintFormat("========================================");
	}
	
	// ========================================================================
	// STATE
	// ========================================================================
	
	bool IsInitialized() { return m_bInitialized; }
	int GetTotalItemCount() { return m_mItemsByID.Count(); }
}

// ============================================================================
// DEBUG COMMANDS
// ============================================================================
class RBL_ShopDebugCommands
{
	static void PrintShop()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.PrintShopToConsole();
	}
	
	static void Buy(string itemID)
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.Buy(itemID);
	}
	
	static void ListItems()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (!shop)
			return;
		
		PrintFormat("\n=== ALL SHOP ITEMS ===");
		PrintFormat("Total items: %1", shop.GetTotalItemCount());
		PrintFormat("Categories: %1", shop.GetCategoryCount());
	}
	
	static void TestPurchase(string itemID)
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (!shop)
			return;
		
		RBL_ShopItem item = shop.GetItemByID(itemID);
		if (!item)
		{
			PrintFormat("Item not found: %1", itemID);
			return;
		}
		
		PrintFormat("Item: %1", item.GetDisplayName());
		PrintFormat("Price: $%1 + %2 HR", item.GetPrice(), item.GetHRCost());
		PrintFormat("Prefab: %1", item.GetPrefabPath());
		PrintFormat("War Level Required: %1", item.GetRequiredWarLevel());
	}
}

