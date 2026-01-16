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

// Vehicle Manager - Tracks and manages all vehicles spawned by the system
class RBL_VehicleManager
{
	protected static ref RBL_VehicleManager s_Instance;
	
	protected ref map<string, IEntity> m_mVehiclesByID;
	protected ref map<IEntity, string> m_mVehicleIDs;
	protected ref map<string, string> m_mVehicleTypes;
	protected int m_iNextVehicleID;
	
	static RBL_VehicleManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_VehicleManager();
		return s_Instance;
	}
	
	void RBL_VehicleManager()
	{
		m_mVehiclesByID = new map<string, IEntity>();
		m_mVehicleIDs = new map<IEntity, string>();
		m_mVehicleTypes = new map<string, string>();
		m_iNextVehicleID = 1;
	}
	
	void RegisterVehicle(IEntity vehicle, string vehicleType)
	{
		if (!vehicle)
			return;
		
		string vehicleID = "vehicle_" + m_iNextVehicleID.ToString();
		m_iNextVehicleID++;
		
		m_mVehiclesByID.Set(vehicleID, vehicle);
		m_mVehicleIDs.Set(vehicle, vehicleID);
		m_mVehicleTypes.Set(vehicleID, vehicleType);
		
		PrintFormat("[RBL_VehicleMgr] Registered vehicle: %1 (%2)", vehicleID, vehicleType);
	}
	
	void UnregisterVehicle(string vehicleID)
	{
		IEntity vehicle;
		if (m_mVehiclesByID.Find(vehicleID, vehicle))
		{
			m_mVehiclesByID.Remove(vehicleID);
			m_mVehicleIDs.Remove(vehicle);
			m_mVehicleTypes.Remove(vehicleID);
		}
	}
	
	void UnregisterVehicleEntity(IEntity vehicle)
	{
		string vehicleID;
		if (m_mVehicleIDs.Find(vehicle, vehicleID))
		{
			UnregisterVehicle(vehicleID);
		}
	}
	
	array<string> GetAllVehicleIDs()
	{
		array<string> ids = new array<string>();
		for (int i = 0; i < m_mVehiclesByID.Count(); i++)
		{
			ids.Insert(m_mVehiclesByID.GetKey(i));
		}
		return ids;
	}
	
	IEntity GetVehicleEntity(string id)
	{
		IEntity vehicle;
		m_mVehiclesByID.Find(id, vehicle);
		return vehicle;
	}
	
	string GetVehicleType(string id)
	{
		string type;
		m_mVehicleTypes.Find(id, type);
		return type;
	}
	
	string GetVehicleID(IEntity vehicle)
	{
		string id;
		m_mVehicleIDs.Find(vehicle, id);
		return id;
	}
	
	float GetVehicleFuel(string id)
	{
		IEntity vehicle = GetVehicleEntity(id);
		if (!vehicle)
			return 0;
		
		// Try to get fuel manager component
		FuelManagerComponent fuelComp = FuelManagerComponent.Cast(vehicle.FindComponent(FuelManagerComponent));
		if (fuelComp)
		{
			// Get fuel level from component
			// Note: Arma Reforger fuel component API may vary
			// For now, return default value if component exists
			return 100.0;
		}
		
		return 0;
	}
	
	float GetVehicleHealth(string id)
	{
		IEntity vehicle = GetVehicleEntity(id);
		if (!vehicle)
			return 0;
		
		// Use damage manager to check health
		DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(vehicle.FindComponent(DamageManagerComponent));
		if (dmgMgr)
		{
			// Check if destroyed
			if (dmgMgr.GetState() == EDamageState.DESTROYED)
				return 0;
			
			// Return approximate health (100 if not destroyed)
			return 100.0;
		}
		
		return 100;
	}
	
	bool IsVehicleDeployed(string id)
	{
		IEntity vehicle = GetVehicleEntity(id);
		if (!vehicle)
			return false;
		
		// Check if entity exists and is not destroyed
		DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(vehicle.FindComponent(DamageManagerComponent));
		if (dmgMgr && dmgMgr.GetState() == EDamageState.DESTROYED)
			return false;
		
		return true;
	}
	
	void DespawnAllVehicles()
	{
		array<string> vehicleIDs = GetAllVehicleIDs();
		for (int i = 0; i < vehicleIDs.Count(); i++)
		{
			IEntity vehicle = GetVehicleEntity(vehicleIDs[i]);
			if (vehicle)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
			}
		}
		
		m_mVehiclesByID.Clear();
		m_mVehicleIDs.Clear();
		m_mVehicleTypes.Clear();
		
		PrintFormat("[RBL_VehicleMgr] Despawned all vehicles");
	}
	
	void SpawnVehicle(string type, vector pos, float fuel, float health)
	{
		// Get vehicle prefab from delivery system
		RBL_ItemDelivery delivery = RBL_ItemDelivery.GetInstance();
		if (!delivery)
			return;
		
		string prefabPath = delivery.GetVehiclePrefabPath(type);
		if (prefabPath.IsEmpty())
		{
			PrintFormat("[RBL_VehicleMgr] Unknown vehicle type: %1", type);
			return;
		}
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		// Adjust position to terrain
		pos[1] = world.GetSurfaceY(pos[0], pos[2]) + 0.5;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = pos;
		
		Resource resource = Resource.Load(prefabPath);
		if (!resource.IsValid())
		{
			PrintFormat("[RBL_VehicleMgr] Invalid vehicle prefab: %1", prefabPath);
			return;
		}
		
		IEntity vehicle = GetGame().SpawnEntityPrefab(resource, world, params);
		if (vehicle)
		{
			RegisterVehicle(vehicle, type);
			
			// Note: Fuel and health setting would require specific component APIs
			// For now, vehicles spawn with default values
			// Future: Integrate with vehicle component APIs when available
			
			PrintFormat("[RBL_VehicleMgr] Spawned vehicle: %1 at %2", type, pos.ToString());
		}
	}
	
	void Update(float timeSlice)
	{
		// Clean up destroyed vehicles
		array<string> toRemove = new array<string>();
		
		for (int i = 0; i < m_mVehiclesByID.Count(); i++)
		{
			string vehicleID = m_mVehiclesByID.GetKey(i);
			IEntity vehicle = m_mVehiclesByID.GetElement(i);
			
			if (!vehicle)
			{
				toRemove.Insert(vehicleID);
				continue;
			}
			
			// Check if vehicle is destroyed
			DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(vehicle.FindComponent(DamageManagerComponent));
			if (dmgMgr && dmgMgr.GetState() == EDamageState.DESTROYED)
			{
				toRemove.Insert(vehicleID);
			}
		}
		
		for (int i = 0; i < toRemove.Count(); i++)
		{
			UnregisterVehicle(toRemove[i]);
		}
	}
	
	int GetVehicleCount()
	{
		return m_mVehiclesByID.Count();
	}
}

