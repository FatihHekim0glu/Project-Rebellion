// ============================================================================
// PROJECT REBELLION - Economy Persistence
// Saves and restores economy state (resources, arsenal, vehicles)
// ============================================================================

class RBL_EconomyPersistence
{
	protected static ref RBL_EconomyPersistence s_Instance;
	
	static RBL_EconomyPersistence GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_EconomyPersistence();
		return s_Instance;
	}
	
	// ========================================================================
	// COLLECT ECONOMY DATA
	// ========================================================================
	
	// Collect full economy state
	RBL_EconomySaveData CollectEconomyState()
	{
		RBL_EconomySaveData data = new RBL_EconomySaveData();
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
		{
			PrintFormat("[RBL_EconomyPersistence] Economy manager not available");
			return data;
		}
		
		// FIA Resources
		data.m_iFIAMoney = econMgr.GetMoney();
		data.m_iFIAHumanResources = econMgr.GetHR();
		data.m_iFIAFuel = econMgr.GetFuel();
		data.m_iFIAAmmo = econMgr.GetAmmo();
		
		// Income rates
		data.m_iMoneyPerMinute = econMgr.GetMoneyIncomeRate();
		data.m_iHRPerMinute = econMgr.GetHRIncomeRate();
		
		// Arsenal items
		CollectArsenalItems(econMgr, data.m_aArsenalItems);
		
		// Unlocked items
		CollectUnlockedItems(econMgr, data.m_aUnlockedItems);
		
		// Vehicle pool
		CollectVehiclePool(data.m_aVehiclePool);
		
		// Trade history
		data.m_iTotalMoneyEarned = econMgr.GetTotalMoneyEarned();
		data.m_iTotalMoneySpent = econMgr.GetTotalMoneySpent();
		data.m_iTotalItemsBought = econMgr.GetTotalItemsBought();
		data.m_iTotalItemsSold = econMgr.GetTotalItemsSold();
		
		PrintFormat("[RBL_EconomyPersistence] Collected economy state: $%1, %2 HR, %3 items",
			data.m_iFIAMoney, data.m_iFIAHumanResources, data.m_aArsenalItems.Count());
		
		return data;
	}
	
	// Collect arsenal items
	protected void CollectArsenalItems(RBL_EconomyManager econMgr, array<ref RBL_ArsenalItemSave> outItems)
	{
		outItems.Clear();
		
		if (!econMgr)
			return;
		
		// Get all items in arsenal
		array<string> itemIDs = econMgr.GetArsenalItemIDs();
		if (!itemIDs)
			return;
		
		for (int i = 0; i < itemIDs.Count(); i++)
		{
			string itemID = itemIDs[i];
			int quantity = econMgr.GetArsenalItemQuantity(itemID);
			
			if (quantity > 0)
			{
				outItems.Insert(new RBL_ArsenalItemSave(itemID, quantity));
			}
		}
	}
	
	// Collect unlocked items
	protected void CollectUnlockedItems(RBL_EconomyManager econMgr, array<string> outUnlocks)
	{
		outUnlocks.Clear();
		
		if (!econMgr)
			return;
		
		array<string> unlocks = econMgr.GetUnlockedItems();
		if (!unlocks)
			return;
		
		for (int i = 0; i < unlocks.Count(); i++)
		{
			outUnlocks.Insert(unlocks[i]);
		}
	}
	
	// Collect vehicle pool
	protected void CollectVehiclePool(array<ref RBL_VehicleSave> outVehicles)
	{
		outVehicles.Clear();
		
		// Get vehicle manager if exists
		RBL_VehicleManager vehMgr = RBL_VehicleManager.GetInstance();
		if (!vehMgr)
			return;
		
		array<string> vehicleIDs = vehMgr.GetAllVehicleIDs();
		if (!vehicleIDs)
			return;
		
		for (int i = 0; i < vehicleIDs.Count(); i++)
		{
			RBL_VehicleSave vehSave = CollectVehicleState(vehicleIDs[i]);
			if (vehSave)
				outVehicles.Insert(vehSave);
		}
	}
	
	// Collect single vehicle state
	protected RBL_VehicleSave CollectVehicleState(string vehicleID)
	{
		RBL_VehicleManager vehMgr = RBL_VehicleManager.GetInstance();
		if (!vehMgr)
			return null;
		
		IEntity vehicle = vehMgr.GetVehicleEntity(vehicleID);
		if (!vehicle)
			return null;
		
		RBL_VehicleSave save = new RBL_VehicleSave();
		save.m_sVehicleID = vehicleID;
		save.m_sVehicleType = vehMgr.GetVehicleType(vehicleID);
		save.m_fFuel = vehMgr.GetVehicleFuel(vehicleID);
		save.m_fHealth = vehMgr.GetVehicleHealth(vehicleID);
		save.m_bIsDeployed = vehMgr.IsVehicleDeployed(vehicleID);
		save.m_vPosition = vehicle.GetOrigin();
		
		return save;
	}
	
	// ========================================================================
	// RESTORE ECONOMY DATA
	// ========================================================================
	
	// Restore full economy state
	bool RestoreEconomyState(RBL_EconomySaveData data)
	{
		if (!data)
		{
			PrintFormat("[RBL_EconomyPersistence] Null economy data for restore");
			return false;
		}
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
		{
			PrintFormat("[RBL_EconomyPersistence] Economy manager not available for restore");
			return false;
		}
		
		// Restore FIA resources
		econMgr.SetMoney(data.m_iFIAMoney);
		econMgr.SetHR(data.m_iFIAHumanResources);
		econMgr.SetFuel(data.m_iFIAFuel);
		econMgr.SetAmmo(data.m_iFIAAmmo);
		
		// Restore income rates
		econMgr.SetMoneyIncomeRate(data.m_iMoneyPerMinute);
		econMgr.SetHRIncomeRate(data.m_iHRPerMinute);
		
		// Restore arsenal
		RestoreArsenalItems(econMgr, data.m_aArsenalItems);
		
		// Restore unlocks
		RestoreUnlockedItems(econMgr, data.m_aUnlockedItems);
		
		// Restore vehicles
		RestoreVehiclePool(data.m_aVehiclePool);
		
		// Restore trade history
		econMgr.SetTradeHistory(
			data.m_iTotalMoneyEarned,
			data.m_iTotalMoneySpent,
			data.m_iTotalItemsBought,
			data.m_iTotalItemsSold
		);
		
		PrintFormat("[RBL_EconomyPersistence] Restored economy state: $%1, %2 HR",
			data.m_iFIAMoney, data.m_iFIAHumanResources);
		
		return true;
	}
	
	// Restore arsenal items
	protected void RestoreArsenalItems(RBL_EconomyManager econMgr, array<ref RBL_ArsenalItemSave> items)
	{
		if (!econMgr || !items)
			return;
		
		// Clear existing arsenal
		econMgr.ClearArsenal();
		
		// Add all items
		for (int i = 0; i < items.Count(); i++)
		{
			RBL_ArsenalItemSave item = items[i];
			if (item && item.m_iQuantity > 0)
			{
				econMgr.AddToArsenal(item.m_sItemID, item.m_iQuantity);
			}
		}
	}
	
	// Restore unlocked items
	protected void RestoreUnlockedItems(RBL_EconomyManager econMgr, array<string> unlocks)
	{
		if (!econMgr || !unlocks)
			return;
		
		// Clear existing unlocks
		econMgr.ClearUnlocks();
		
		// Add all unlocks
		for (int i = 0; i < unlocks.Count(); i++)
		{
			econMgr.UnlockItem(unlocks[i]);
		}
	}
	
	// Restore vehicle pool
	protected void RestoreVehiclePool(array<ref RBL_VehicleSave> vehicles)
	{
		if (!vehicles)
			return;
		
		RBL_VehicleManager vehMgr = RBL_VehicleManager.GetInstance();
		if (!vehMgr)
			return;
		
		// Clear existing vehicles
		vehMgr.DespawnAllVehicles();
		
		// Spawn saved vehicles
		for (int i = 0; i < vehicles.Count(); i++)
		{
			RBL_VehicleSave vehSave = vehicles[i];
			if (!vehSave)
				continue;
			
			// Only restore deployed vehicles
			if (vehSave.m_bIsDeployed && vehSave.m_vPosition != vector.Zero)
			{
				vehMgr.SpawnVehicle(vehSave.m_sVehicleType, vehSave.m_vPosition, vehSave.m_fFuel, vehSave.m_fHealth);
			}
		}
	}
	
	// ========================================================================
	// VALIDATION
	// ========================================================================
	
	// Validate economy data
	bool ValidateEconomyData(RBL_EconomySaveData data)
	{
		if (!data)
			return false;
		
		// Resources should be non-negative
		if (data.m_iFIAMoney < 0)
			return false;
		if (data.m_iFIAHumanResources < 0)
			return false;
		if (data.m_iFIAFuel < 0)
			return false;
		if (data.m_iFIAAmmo < 0)
			return false;
		
		// Validate arsenal items
		for (int i = 0; i < data.m_aArsenalItems.Count(); i++)
		{
			RBL_ArsenalItemSave item = data.m_aArsenalItems[i];
			if (!item || item.m_sItemID.IsEmpty() || item.m_iQuantity < 0)
				return false;
		}
		
		return true;
	}
	
	// ========================================================================
	// UTILITIES
	// ========================================================================
	
	// Calculate total arsenal value
	int CalculateArsenalValue(array<ref RBL_ArsenalItemSave> items)
	{
		if (!items)
			return 0;
		
		int totalValue = 0;
		
		RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
		if (!shopMgr)
			return 0;
		
		for (int i = 0; i < items.Count(); i++)
		{
			RBL_ArsenalItemSave item = items[i];
			if (!item)
				continue;
			
			int itemPrice = shopMgr.GetItemPrice(item.m_sItemID);
			totalValue += itemPrice * item.m_iQuantity;
		}
		
		return totalValue;
	}
	
	// Get total net worth
	int GetNetWorth(RBL_EconomySaveData data)
	{
		if (!data)
			return 0;
		
		int worth = data.m_iFIAMoney;
		worth += CalculateArsenalValue(data.m_aArsenalItems);
		
		// Add vehicle values
		for (int i = 0; i < data.m_aVehiclePool.Count(); i++)
		{
			RBL_VehicleSave veh = data.m_aVehiclePool[i];
			if (veh)
			{
				RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
				if (shopMgr)
					worth += shopMgr.GetVehiclePrice(veh.m_sVehicleType);
			}
		}
		
		return worth;
	}
	
	// Compare economy states
	RBL_EconomyDiff CompareEconomyStates(RBL_EconomySaveData oldState, RBL_EconomySaveData newState)
	{
		RBL_EconomyDiff diff = new RBL_EconomyDiff();
		
		if (!oldState || !newState)
			return diff;
		
		diff.m_iMoneyDiff = newState.m_iFIAMoney - oldState.m_iFIAMoney;
		diff.m_iHRDiff = newState.m_iFIAHumanResources - oldState.m_iFIAHumanResources;
		diff.m_iFuelDiff = newState.m_iFIAFuel - oldState.m_iFIAFuel;
		diff.m_iAmmoDiff = newState.m_iFIAAmmo - oldState.m_iFIAAmmo;
		diff.m_iArsenalItemsDiff = newState.m_aArsenalItems.Count() - oldState.m_aArsenalItems.Count();
		diff.m_iVehiclesDiff = newState.m_aVehiclePool.Count() - oldState.m_aVehiclePool.Count();
		
		return diff;
	}
}

// Economy difference tracking
class RBL_EconomyDiff
{
	int m_iMoneyDiff;
	int m_iHRDiff;
	int m_iFuelDiff;
	int m_iAmmoDiff;
	int m_iArsenalItemsDiff;
	int m_iVehiclesDiff;
	
	void RBL_EconomyDiff()
	{
		m_iMoneyDiff = 0;
		m_iHRDiff = 0;
		m_iFuelDiff = 0;
		m_iAmmoDiff = 0;
		m_iArsenalItemsDiff = 0;
		m_iVehiclesDiff = 0;
	}
	
	bool HasChanges()
	{
		return m_iMoneyDiff != 0 || m_iHRDiff != 0 || m_iFuelDiff != 0 ||
			   m_iAmmoDiff != 0 || m_iArsenalItemsDiff != 0 || m_iVehiclesDiff != 0;
	}
	
	string GetSummary()
	{
		string summary = "";
		string sign;
		
		if (m_iMoneyDiff != 0)
		{
			if (m_iMoneyDiff > 0)
				sign = "+";
			else
				sign = "";
			summary += string.Format("Money: %1%2, ", sign, m_iMoneyDiff);
		}
		if (m_iHRDiff != 0)
		{
			if (m_iHRDiff > 0)
				sign = "+";
			else
				sign = "";
			summary += string.Format("HR: %1%2, ", sign, m_iHRDiff);
		}
		if (m_iArsenalItemsDiff != 0)
		{
			if (m_iArsenalItemsDiff > 0)
				sign = "+";
			else
				sign = "";
			summary += string.Format("Items: %1%2, ", sign, m_iArsenalItemsDiff);
		}
		if (m_iVehiclesDiff != 0)
		{
			if (m_iVehiclesDiff > 0)
				sign = "+";
			else
				sign = "";
			summary += string.Format("Vehicles: %1%2", sign, m_iVehiclesDiff);
		}
		
		if (summary.IsEmpty())
			return "No changes";
		
		return summary;
	}
}

// Placeholder for vehicle manager interface
class RBL_VehicleManager
{
	protected static ref RBL_VehicleManager s_Instance;
	
	static RBL_VehicleManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_VehicleManager();
		return s_Instance;
	}
	
	array<string> GetAllVehicleIDs() { return new array<string>(); }
	IEntity GetVehicleEntity(string id) { return null; }
	string GetVehicleType(string id) { return ""; }
	float GetVehicleFuel(string id) { return 0; }
	float GetVehicleHealth(string id) { return 0; }
	bool IsVehicleDeployed(string id) { return false; }
	void DespawnAllVehicles() {}
	void SpawnVehicle(string type, vector pos, float fuel, float health) {}
}

