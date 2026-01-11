// ============================================================================
// PROJECT REBELLION - Shop/Arsenal Menu
// Buy weapons, equipment, vehicles, recruit units
// ============================================================================

class RBL_ShopItem
{
	string ID;
	string DisplayName;
	string Category;
	int Price;
	int HRCost;
	string PrefabPath;
	bool RequiresUnlock;
	
	void RBL_ShopItem()
	{
		HRCost = 0;
		RequiresUnlock = false;
	}
}

class RBL_ShopManager
{
	protected static ref RBL_ShopManager s_Instance;
	
	protected bool m_bMenuOpen;
	protected ref array<ref RBL_ShopItem> m_aWeapons;
	protected ref array<ref RBL_ShopItem> m_aEquipment;
	protected ref array<ref RBL_ShopItem> m_aVehicles;
	protected ref array<ref RBL_ShopItem> m_aRecruits;
	
	protected ref ScriptInvoker m_OnMenuOpened;
	protected ref ScriptInvoker m_OnMenuClosed;
	protected ref ScriptInvoker m_OnPurchase;
	
	static RBL_ShopManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_ShopManager();
		return s_Instance;
	}
	
	void RBL_ShopManager()
	{
		m_bMenuOpen = false;
		m_aWeapons = new array<ref RBL_ShopItem>();
		m_aEquipment = new array<ref RBL_ShopItem>();
		m_aVehicles = new array<ref RBL_ShopItem>();
		m_aRecruits = new array<ref RBL_ShopItem>();
		
		m_OnMenuOpened = new ScriptInvoker();
		m_OnMenuClosed = new ScriptInvoker();
		m_OnPurchase = new ScriptInvoker();
		
		InitializeShopItems();
	}
	
	protected void InitializeShopItems()
	{
		// === WEAPONS ===
		AddWeapon("makarov", "Makarov PM", 25, "{3E413771E1834D2E}Prefabs/Weapons/Handguns/PM/Weapon_PM.et");
		AddWeapon("akm", "AKM", 150, "{1BC151E0D4DE3D99}Prefabs/Weapons/Rifles/AKM/Weapon_AKM.et");
		AddWeapon("ak74", "AK-74", 175, "{9B3DFBAE74363E7A}Prefabs/Weapons/Rifles/AK74/Weapon_AK74.et");
		AddWeapon("svd", "SVD Dragunov", 400, "{3E36C5A1DDAE0CF8}Prefabs/Weapons/Rifles/SVD/Weapon_SVD.et");
		AddWeapon("rpg7", "RPG-7", 500, "{519E924C1C8C5FB4}Prefabs/Weapons/Launchers/RPG7/Weapon_RPG7.et");
		AddWeapon("pkm", "PKM", 600, "{5C3AD3CD9F747118}Prefabs/Weapons/MachineGuns/PKM/Weapon_PKM.et");
		
		// === EQUIPMENT ===
		AddEquipment("bandage", "Bandage", 10, "Bandage");
		AddEquipment("grenade_rgd", "RGD-5 Grenade", 50, "RGD5");
		AddEquipment("grenade_smoke", "Smoke Grenade", 30, "SmokeGrenade");
		AddEquipment("medkit", "Medical Kit", 75, "MedKit");
		AddEquipment("binocs", "Binoculars", 100, "Binoculars");
		AddEquipment("nvg", "Night Vision", 500, "NVG");
		
		// === VEHICLES ===
		AddVehicle("uaz", "UAZ-469", 500, "{5E74787BB083789B}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469.et");
		AddVehicle("ural", "Ural Truck", 750, "{91B01E6C0D20E1D1}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320.et");
		AddVehicle("btr70", "BTR-70", 3000, "{D85A504DF4E2C128}Prefabs/Vehicles/Wheeled/BTR70/BTR70.et");
		AddVehicle("bmp1", "BMP-1", 5000, "{BB0E7CE42F0F3E19}Prefabs/Vehicles/Tracked/BMP1/BMP1.et");
		
		// === RECRUITS ===
		AddRecruit("rifleman", "Rifleman", 50, 1);
		AddRecruit("medic", "Combat Medic", 75, 1);
		AddRecruit("mg", "Machine Gunner", 100, 1);
		AddRecruit("at", "AT Specialist", 125, 1);
		AddRecruit("sniper", "Sniper", 150, 1);
		AddRecruit("squad", "Fire Team (4)", 300, 4);
		
		PrintFormat("[RBL_Shop] Initialized: %1 weapons, %2 equipment, %3 vehicles, %4 recruit types",
			m_aWeapons.Count(), m_aEquipment.Count(), m_aVehicles.Count(), m_aRecruits.Count());
	}
	
	protected void AddWeapon(string id, string name, int price, string prefab)
	{
		RBL_ShopItem item = new RBL_ShopItem();
		item.ID = id;
		item.DisplayName = name;
		item.Category = "Weapons";
		item.Price = price;
		item.PrefabPath = prefab;
		m_aWeapons.Insert(item);
	}
	
	protected void AddEquipment(string id, string name, int price, string prefab)
	{
		RBL_ShopItem item = new RBL_ShopItem();
		item.ID = id;
		item.DisplayName = name;
		item.Category = "Equipment";
		item.Price = price;
		item.PrefabPath = prefab;
		m_aEquipment.Insert(item);
	}
	
	protected void AddVehicle(string id, string name, int price, string prefab)
	{
		RBL_ShopItem item = new RBL_ShopItem();
		item.ID = id;
		item.DisplayName = name;
		item.Category = "Vehicles";
		item.Price = price;
		item.PrefabPath = prefab;
		m_aVehicles.Insert(item);
	}
	
	protected void AddRecruit(string id, string name, int price, int hrCost)
	{
		RBL_ShopItem item = new RBL_ShopItem();
		item.ID = id;
		item.DisplayName = name;
		item.Category = "Recruits";
		item.Price = price;
		item.HRCost = hrCost;
		m_aRecruits.Insert(item);
	}
	
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
		PrintShopToConsole();
	}
	
	void CloseMenu()
	{
		m_bMenuOpen = false;
		m_OnMenuClosed.Invoke();
		PrintFormat("[RBL_Shop] Menu closed");
	}
	
	bool IsMenuOpen() { return m_bMenuOpen; }
	
	bool PurchaseItem(string itemID)
	{
		RBL_ShopItem item = FindItemByID(itemID);
		if (!item)
		{
			PrintFormat("[RBL_Shop] Item not found: %1", itemID);
			return false;
		}
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
			return false;
		
		// Check if can afford
		if (econMgr.GetMoney() < item.Price)
		{
			PrintFormat("[RBL_Shop] Cannot afford %1 ($%2 needed, have $%3)", item.DisplayName, item.Price, econMgr.GetMoney());
			return false;
		}
		
		// Check HR for recruits
		if (item.HRCost > 0 && econMgr.GetHR() < item.HRCost)
		{
			PrintFormat("[RBL_Shop] Not enough HR for %1 (%2 needed, have %3)", item.DisplayName, item.HRCost, econMgr.GetHR());
			return false;
		}
		
		// Process purchase
		econMgr.SpendMoney(item.Price);
		if (item.HRCost > 0)
			econMgr.SpendHR(item.HRCost);
		
		m_OnPurchase.Invoke(item);
		
		PrintFormat("[RBL_Shop] *** PURCHASED: %1 for $%2 ***", item.DisplayName, item.Price);
		PrintFormat("[RBL_Shop] Remaining: $%1, HR: %2", econMgr.GetMoney(), econMgr.GetHR());
		
		return true;
	}
	
	protected RBL_ShopItem FindItemByID(string itemID)
	{
		for (int i = 0; i < m_aWeapons.Count(); i++)
		{
			if (m_aWeapons[i].ID == itemID)
				return m_aWeapons[i];
		}
		for (int i = 0; i < m_aEquipment.Count(); i++)
		{
			if (m_aEquipment[i].ID == itemID)
				return m_aEquipment[i];
		}
		for (int i = 0; i < m_aVehicles.Count(); i++)
		{
			if (m_aVehicles[i].ID == itemID)
				return m_aVehicles[i];
		}
		for (int i = 0; i < m_aRecruits.Count(); i++)
		{
			if (m_aRecruits[i].ID == itemID)
				return m_aRecruits[i];
		}
		return null;
	}
	
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
		
		PrintFormat("========================================");
		PrintFormat("        PROJECT REBELLION SHOP");
		PrintFormat("   Your funds: $%1 | HR: %2", money, hr);
		PrintFormat("========================================");
		
		PrintFormat("\n--- WEAPONS ---");
		for (int i = 0; i < m_aWeapons.Count(); i++)
		{
			RBL_ShopItem item = m_aWeapons[i];
			string affordable = "";
			if (money >= item.Price)
				affordable = "[CAN BUY]";
			PrintFormat("  %1: %2 - $%3 %4", item.ID, item.DisplayName, item.Price, affordable);
		}
		
		PrintFormat("\n--- EQUIPMENT ---");
		for (int i = 0; i < m_aEquipment.Count(); i++)
		{
			RBL_ShopItem item = m_aEquipment[i];
			string affordable = "";
			if (money >= item.Price)
				affordable = "[CAN BUY]";
			PrintFormat("  %1: %2 - $%3 %4", item.ID, item.DisplayName, item.Price, affordable);
		}
		
		PrintFormat("\n--- VEHICLES ---");
		for (int i = 0; i < m_aVehicles.Count(); i++)
		{
			RBL_ShopItem item = m_aVehicles[i];
			string affordable = "";
			if (money >= item.Price)
				affordable = "[CAN BUY]";
			PrintFormat("  %1: %2 - $%3 %4", item.ID, item.DisplayName, item.Price, affordable);
		}
		
		PrintFormat("\n--- RECRUITS ---");
		for (int i = 0; i < m_aRecruits.Count(); i++)
		{
			RBL_ShopItem item = m_aRecruits[i];
			string affordable = "";
			if (money >= item.Price && hr >= item.HRCost)
				affordable = "[CAN BUY]";
			PrintFormat("  %1: %2 - $%3 + %4 HR %5", item.ID, item.DisplayName, item.Price, item.HRCost, affordable);
		}
		
		PrintFormat("\n========================================");
		PrintFormat("  To buy: RBL_ShopManager.GetInstance().PurchaseItem(\"itemid\")");
		PrintFormat("========================================");
	}
	
	array<ref RBL_ShopItem> GetWeapons() { return m_aWeapons; }
	array<ref RBL_ShopItem> GetEquipment() { return m_aEquipment; }
	array<ref RBL_ShopItem> GetVehicles() { return m_aVehicles; }
	array<ref RBL_ShopItem> GetRecruits() { return m_aRecruits; }
	
	ScriptInvoker GetOnMenuOpened() { return m_OnMenuOpened; }
	ScriptInvoker GetOnMenuClosed() { return m_OnMenuClosed; }
	ScriptInvoker GetOnPurchase() { return m_OnPurchase; }
}

