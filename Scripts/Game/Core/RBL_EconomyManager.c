// ============================================================================
// PROJECT REBELLION - Economy Manager
// Handles Virtual Arsenal (persistent loot), faction funds (HR/Money), supplies
// ============================================================================

class RBL_EconomyManager
{
	protected static RBL_EconomyManager s_Instance;
	
	// ========================================================================
	// CONFIGURATION
	// ========================================================================
	
	protected const int UNLOCK_THRESHOLD = 25;            // Items needed for unlimited
	protected const int MAX_MONEY = 999999;
	protected const int MAX_HR = 9999;
	
	// ========================================================================
	// STATE
	// ========================================================================
	
	protected int m_iMoney;
	protected int m_iHumanResources;
	
	// Arsenal inventory: prefab name -> count
	protected ref map<string, int> m_mArsenalInventory;
	
	// Unlocked items (count >= threshold)
	protected ref set<string> m_sUnlockedItems;
	
	// ========================================================================
	// SIGNALS
	// ========================================================================
	
	protected ref ScriptInvoker m_OnMoneyChanged;
	protected ref ScriptInvoker m_OnHRChanged;
	protected ref ScriptInvoker m_OnItemDeposited;
	protected ref ScriptInvoker m_OnItemUnlocked;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_EconomyManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_EconomyManager();
		return s_Instance;
	}
	
	void RBL_EconomyManager()
	{
		m_mArsenalInventory = new map<string, int>();
		m_sUnlockedItems = new set<string>();
		
		m_OnMoneyChanged = new ScriptInvoker();
		m_OnHRChanged = new ScriptInvoker();
		m_OnItemDeposited = new ScriptInvoker();
		m_OnItemUnlocked = new ScriptInvoker();
		
		m_iMoney = 0;
		m_iHumanResources = 0;
	}
	
	// ========================================================================
	// MONEY MANAGEMENT
	// ========================================================================
	
	void SetMoney(int amount)
	{
		int previous = m_iMoney;
		m_iMoney = Math.Clamp(amount, 0, MAX_MONEY);
		
		if (previous != m_iMoney)
			m_OnMoneyChanged.Invoke(m_iMoney);
	}
	
	void AddMoney(int amount)
	{
		SetMoney(m_iMoney + amount);
	}
	
	bool SpendMoney(int amount)
	{
		if (m_iMoney < amount)
			return false;
		
		SetMoney(m_iMoney - amount);
		return true;
	}
	
	int GetMoney()
	{
		return m_iMoney;
	}
	
	bool CanAfford(int amount)
	{
		return m_iMoney >= amount;
	}
	
	// ========================================================================
	// HUMAN RESOURCES MANAGEMENT
	// ========================================================================
	
	void SetHR(int amount)
	{
		int previous = m_iHumanResources;
		m_iHumanResources = Math.Clamp(amount, 0, MAX_HR);
		
		if (previous != m_iHumanResources)
			m_OnHRChanged.Invoke(m_iHumanResources);
	}
	
	void AddHR(int amount)
	{
		SetHR(m_iHumanResources + amount);
	}
	
	bool SpendHR(int amount)
	{
		if (m_iHumanResources < amount)
			return false;
		
		SetHR(m_iHumanResources - amount);
		return true;
	}
	
	int GetHR()
	{
		return m_iHumanResources;
	}
	
	// ========================================================================
	// ARSENAL SYSTEM
	// ========================================================================
	
	void DepositItem(string itemPrefab, int count = 1)
	{
		int currentCount = 0;
		m_mArsenalInventory.Find(itemPrefab, currentCount);
		
		int newCount = currentCount + count;
		m_mArsenalInventory.Set(itemPrefab, newCount);
		
		m_OnItemDeposited.Invoke(itemPrefab, newCount);
		
		// Check unlock threshold
		if (newCount >= UNLOCK_THRESHOLD && !m_sUnlockedItems.Contains(itemPrefab))
		{
			m_sUnlockedItems.Insert(itemPrefab);
			
			PrintFormat("[RBL_Economy] Item UNLOCKED: %1", itemPrefab);
			m_OnItemUnlocked.Invoke(itemPrefab);
			
			RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
			if (campaignMgr)
				campaignMgr.OnCampaignEvent(ERBLCampaignEvent.ITEM_UNLOCKED, null);
		}
		
		PrintFormat("[RBL_Economy] Deposited %1 x%2 (Total: %3)", itemPrefab, count, newCount);
	}
	
	bool WithdrawItem(string itemPrefab, int count = 1)
	{
		// Unlimited items don't consume stock
		if (m_sUnlockedItems.Contains(itemPrefab))
			return true;
		
		int currentCount = 0;
		if (!m_mArsenalInventory.Find(itemPrefab, currentCount))
			return false;
		
		if (currentCount < count)
			return false;
		
		int newCount = currentCount - count;
		m_mArsenalInventory.Set(itemPrefab, newCount);
		
		return true;
	}
	
	int GetItemCount(string itemPrefab)
	{
		int count = 0;
		m_mArsenalInventory.Find(itemPrefab, count);
		return count;
	}
	
	ERBLItemAvailability GetItemAvailability(string itemPrefab)
	{
		if (m_sUnlockedItems.Contains(itemPrefab))
			return ERBLItemAvailability.UNLIMITED;
		
		int count = GetItemCount(itemPrefab);
		if (count > 0)
			return ERBLItemAvailability.LIMITED;
		
		return ERBLItemAvailability.LOCKED;
	}
	
	bool IsItemUnlocked(string itemPrefab)
	{
		return m_sUnlockedItems.Contains(itemPrefab);
	}
	
	bool IsItemAvailable(string itemPrefab)
	{
		return m_sUnlockedItems.Contains(itemPrefab) || GetItemCount(itemPrefab) > 0;
	}
	
	void SetArsenalItemCount(string itemPrefab, int count)
	{
		m_mArsenalInventory.Set(itemPrefab, count);
		
		if (count >= UNLOCK_THRESHOLD)
			m_sUnlockedItems.Insert(itemPrefab);
	}
	
	map<string, int> GetArsenalInventory()
	{
		return m_mArsenalInventory;
	}
	
	array<string> GetAvailableItems()
	{
		array<string> result = {};
		
		// Unlimited items first
		foreach (string item : m_sUnlockedItems)
		{
			result.Insert(item);
		}
		
		// Limited items
		array<string> keys = {};
		m_mArsenalInventory.GetKeyArray(keys);
		
		foreach (string key : keys)
		{
			if (!m_sUnlockedItems.Contains(key))
			{
				int count = m_mArsenalInventory.Get(key);
				if (count > 0)
					result.Insert(key);
			}
		}
		
		return result;
	}
	
	// ========================================================================
	// SIGNAL ACCESSORS
	// ========================================================================
	
	ScriptInvoker GetOnMoneyChanged() { return m_OnMoneyChanged; }
	ScriptInvoker GetOnHRChanged() { return m_OnHRChanged; }
	ScriptInvoker GetOnItemDeposited() { return m_OnItemDeposited; }
	ScriptInvoker GetOnItemUnlocked() { return m_OnItemUnlocked; }
	
	// ========================================================================
	// ITEM DEPOSIT ACTION
	// Custom action for arsenal boxes
	// ========================================================================
	
	void ProcessItemDeposit(IEntity player, IEntity item)
	{
		if (!player || !item)
			return;
		
		// Get item prefab
		EntityPrefabData prefabData = item.GetPrefabData();
		if (!prefabData)
			return;
		
		string prefabName = prefabData.GetPrefabName();
		
		// Remove item from player inventory
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(
			player.FindComponent(SCR_InventoryStorageManagerComponent)
		);
		
		if (!inventory)
			return;
		
		// Try to remove item
		if (inventory.TryRemoveItemFromStorage(item))
		{
			// Add to arsenal
			DepositItem(prefabName, 1);
			
			// Destroy the physical item
			SCR_EntityHelper.DeleteEntityAndChildren(item);
			
			// Notify campaign
			RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
			if (campaignMgr)
				campaignMgr.OnCampaignEvent(ERBLCampaignEvent.ITEM_DEPOSITED, null);
		}
	}
	
	// ========================================================================
	// PRICING
	// ========================================================================
	
	int GetVehiclePrice(string vehiclePrefab)
	{
		// Base prices - would be configured externally
		if (vehiclePrefab.Contains("BTR") || vehiclePrefab.Contains("BMP"))
			return 5000;
		if (vehiclePrefab.Contains("Heli") || vehiclePrefab.Contains("Mi8"))
			return 10000;
		if (vehiclePrefab.Contains("Truck"))
			return 1000;
		if (vehiclePrefab.Contains("UAZ") || vehiclePrefab.Contains("HMMWV"))
			return 750;
		
		return 500; // Default for cars
	}
	
	int GetRecruitPrice()
	{
		return 50; // Base price per recruit
	}
	
	bool PurchaseVehicle(string vehiclePrefab)
	{
		int price = GetVehiclePrice(vehiclePrefab);
		return SpendMoney(price);
	}
	
	bool RecruitUnit(int hrCost = 1, int moneyCost = 50)
	{
		if (m_iHumanResources < hrCost || m_iMoney < moneyCost)
			return false;
		
		SpendHR(hrCost);
		SpendMoney(moneyCost);
		return true;
	}
	
	// ========================================================================
	// DEBUG
	// ========================================================================
	
	void PrintEconomyStatus()
	{
		PrintFormat("[RBL_Economy] === ECONOMY STATUS ===");
		PrintFormat("Money: $%1", m_iMoney);
		PrintFormat("Human Resources: %1", m_iHumanResources);
		PrintFormat("Arsenal Items: %1", m_mArsenalInventory.Count());
		PrintFormat("Unlocked Items: %1", m_sUnlockedItems.Count());
		
		array<string> keys = {};
		m_mArsenalInventory.GetKeyArray(keys);
		
		foreach (string key : keys)
		{
			int count = m_mArsenalInventory.Get(key);
			string status = m_sUnlockedItems.Contains(key) ? "[UNLIMITED]" : "[LIMITED]";
			PrintFormat("  %1: %2 %3", key, count, status);
		}
	}
}

