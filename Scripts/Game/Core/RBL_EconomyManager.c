// ============================================================================
// PROJECT REBELLION - Economy Manager
// With multiplayer support - server authoritative
// ============================================================================

class RBL_EconomyManager
{
	protected static ref RBL_EconomyManager s_Instance;

	// Using RBL_Config constant for maintainability
	protected const int MAX_MONEY = 999999;
	protected const int MAX_HR = 9999;

	protected int m_iMoney;
	protected int m_iHumanResources;

	protected ref map<string, int> m_mArsenalInventory;
	protected ref set<string> m_sUnlockedItems;

	protected ref ScriptInvoker m_OnMoneyChanged;
	protected ref ScriptInvoker m_OnHRChanged;
	protected ref ScriptInvoker m_OnItemDeposited;
	protected ref ScriptInvoker m_OnItemUnlocked;
	
	// Network: flag to allow local-only updates without authority check
	protected bool m_bAllowLocalUpdate;

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
		m_bAllowLocalUpdate = false;
	}
	
	// ========================================================================
	// NETWORK AUTHORITY HELPERS
	// ========================================================================
	
	protected bool CanModifyState()
	{
		if (m_bAllowLocalUpdate)
			return true;
		
		return RBL_NetworkUtils.IsSinglePlayer() || RBL_NetworkUtils.IsServer();
	}
	
	void AllowLocalUpdate(bool allow)
	{
		m_bAllowLocalUpdate = allow;
	}

	void SetMoney(int amount)
	{
		if (!CanModifyState())
		{
			PrintFormat("[RBL_Economy] SetMoney blocked - not server");
			return;
		}
		
		int previous = m_iMoney;
		m_iMoney = Math.Clamp(amount, 0, MAX_MONEY);

		if (previous != m_iMoney)
			m_OnMoneyChanged.Invoke(m_iMoney);
	}
	
	void SetMoneyLocal(int amount)
	{
		int previous = m_iMoney;
		m_iMoney = Math.Clamp(amount, 0, MAX_MONEY);
		
		if (previous != m_iMoney)
			m_OnMoneyChanged.Invoke(m_iMoney);
	}

	void AddMoney(int amount)
	{
		if (!CanModifyState())
		{
			PrintFormat("[RBL_Economy] AddMoney blocked - not server");
			return;
		}
		SetMoney(m_iMoney + amount);
	}

	bool SpendMoney(int amount)
	{
		if (!CanModifyState())
		{
			PrintFormat("[RBL_Economy] SpendMoney blocked - not server");
			return false;
		}
		
		if (m_iMoney < amount)
			return false;

		SetMoney(m_iMoney - amount);
		return true;
	}

	int GetMoney() { return m_iMoney; }

	bool CanAfford(int amount) { return m_iMoney >= amount; }

	void SetHR(int amount)
	{
		if (!CanModifyState())
		{
			PrintFormat("[RBL_Economy] SetHR blocked - not server");
			return;
		}
		
		int previous = m_iHumanResources;
		m_iHumanResources = Math.Clamp(amount, 0, MAX_HR);

		if (previous != m_iHumanResources)
			m_OnHRChanged.Invoke(m_iHumanResources);
	}
	
	void SetHRLocal(int amount)
	{
		int previous = m_iHumanResources;
		m_iHumanResources = Math.Clamp(amount, 0, MAX_HR);
		
		if (previous != m_iHumanResources)
			m_OnHRChanged.Invoke(m_iHumanResources);
	}

	void AddHR(int amount)
	{
		if (!CanModifyState())
		{
			PrintFormat("[RBL_Economy] AddHR blocked - not server");
			return;
		}
		SetHR(m_iHumanResources + amount);
	}

	bool SpendHR(int amount)
	{
		if (!CanModifyState())
		{
			PrintFormat("[RBL_Economy] SpendHR blocked - not server");
			return false;
		}
		
		if (m_iHumanResources < amount)
			return false;

		SetHR(m_iHumanResources - amount);
		return true;
	}

	int GetHR() { return m_iHumanResources; }

	void DepositItem(string itemPrefab, int count)
	{
		int currentCount = 0;
		m_mArsenalInventory.Find(itemPrefab, currentCount);

		int newCount = currentCount + count;
		m_mArsenalInventory.Set(itemPrefab, newCount);

		m_OnItemDeposited.Invoke(itemPrefab, newCount);

		if (newCount >= RBL_Config.ARSENAL_UNLOCK_THRESHOLD && !m_sUnlockedItems.Contains(itemPrefab))
		{
			m_sUnlockedItems.Insert(itemPrefab);
			PrintFormat("[RBL_Economy] Item UNLOCKED: %1", itemPrefab);
			m_OnItemUnlocked.Invoke(itemPrefab);
		}

		PrintFormat("[RBL_Economy] Deposited %1 x%2 (Total: %3)", itemPrefab, count, newCount);
	}

	bool WithdrawItem(string itemPrefab, int count)
	{
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

	bool IsItemUnlocked(string itemPrefab) { return m_sUnlockedItems.Contains(itemPrefab); }

	bool IsItemAvailable(string itemPrefab)
	{
		if (m_sUnlockedItems.Contains(itemPrefab))
			return true;
		if (GetItemCount(itemPrefab) > 0)
			return true;
		return false;
	}

	void SetArsenalItemCount(string itemPrefab, int count)
	{
		m_mArsenalInventory.Set(itemPrefab, count);

		if (count >= RBL_Config.ARSENAL_UNLOCK_THRESHOLD)
			m_sUnlockedItems.Insert(itemPrefab);
	}

	map<string, int> GetArsenalInventory() { return m_mArsenalInventory; }

	ScriptInvoker GetOnMoneyChanged() { return m_OnMoneyChanged; }
	ScriptInvoker GetOnHRChanged() { return m_OnHRChanged; }
	ScriptInvoker GetOnItemDeposited() { return m_OnItemDeposited; }
	ScriptInvoker GetOnItemUnlocked() { return m_OnItemUnlocked; }

	int GetVehiclePrice(string vehiclePrefab)
	{
		if (vehiclePrefab.Contains("BTR") || vehiclePrefab.Contains("BMP"))
			return 5000;
		if (vehiclePrefab.Contains("Heli") || vehiclePrefab.Contains("Mi8"))
			return 10000;
		if (vehiclePrefab.Contains("Truck"))
			return 1000;
		if (vehiclePrefab.Contains("UAZ") || vehiclePrefab.Contains("HMMWV"))
			return 750;
		return 500;
	}

	int GetRecruitPrice() { return RBL_Config.RECRUIT_MONEY_COST; }

	bool PurchaseVehicle(string vehiclePrefab)
	{
		int price = GetVehiclePrice(vehiclePrefab);
		return SpendMoney(price);
	}

	bool RecruitUnit(int hrCost, int moneyCost)
	{
		if (m_iHumanResources < hrCost || m_iMoney < moneyCost)
			return false;

		SpendHR(hrCost);
		SpendMoney(moneyCost);
		return true;
	}

	bool TryPurchase(string itemID, int moneyCost, int hrCost)
	{
		if (!CanModifyState())
		{
			PrintFormat("[RBL_Economy] TryPurchase must be called on server");
			return false;
		}
		
		if (m_iMoney < moneyCost)
		{
			PrintFormat("[RBL_Economy] TryPurchase failed: Not enough money ($%1 < $%2)", m_iMoney, moneyCost);
			return false;
		}
		
		if (m_iHumanResources < hrCost)
		{
			PrintFormat("[RBL_Economy] TryPurchase failed: Not enough HR (%1 < %2)", m_iHumanResources, hrCost);
			return false;
		}
		
		SpendMoney(moneyCost);
		if (hrCost > 0)
			SpendHR(hrCost);
		
		PrintFormat("[RBL_Economy] Purchase successful: %1 ($%2, %3 HR)", itemID, moneyCost, hrCost);
		return true;
	}
	
	void RequestPurchase(string itemID, int moneyCost, int hrCost)
	{
		RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
		if (netMgr)
		{
			int playerID = RBL_NetworkUtils.GetLocalPlayerID();
			netMgr.RequestPurchase(playerID, itemID, moneyCost, hrCost);
		}
		else if (RBL_NetworkUtils.IsSinglePlayer())
		{
			TryPurchase(itemID, moneyCost, hrCost);
		}
	}

	bool CanAffordPurchase(int moneyCost, int hrCost)
	{
		return (m_iMoney >= moneyCost) && (m_iHumanResources >= hrCost);
	}

	void PrintEconomyStatus()
	{
		PrintFormat("[RBL_Economy] === ECONOMY STATUS ===");
		PrintFormat("Money: $%1", m_iMoney);
		PrintFormat("Human Resources: %1", m_iHumanResources);
		PrintFormat("Arsenal Items: %1", m_mArsenalInventory.Count());
		PrintFormat("Unlocked Items: %1", m_sUnlockedItems.Count());
		PrintFormat("Is Server: %1", RBL_NetworkUtils.IsServer());
		PrintFormat("Is Singleplayer: %1", RBL_NetworkUtils.IsSinglePlayer());
		PrintFormat("Can Modify: %1", CanModifyState());
	}
	
	// ========================================================================
	// NETWORK SERIALIZATION
	// ========================================================================
	
	void SerializeToNetwork(out int money, out int hr)
	{
		money = m_iMoney;
		hr = m_iHumanResources;
	}
	
	void DeserializeFromNetwork(int money, int hr)
	{
		m_bAllowLocalUpdate = true;
		SetMoneyLocal(money);
		SetHRLocal(hr);
		m_bAllowLocalUpdate = false;
	}
	
	string SerializeArsenalToString()
	{
		string result = "";
		
		array<string> keys = new array<string>();
		m_mArsenalInventory.GetKeyArray(keys);
		
		for (int i = 0; i < keys.Count(); i++)
		{
			int count;
			m_mArsenalInventory.Find(keys[i], count);
			
			if (result.Length() > 0)
				result += ";";
			
			result += keys[i] + ":" + count.ToString();
		}
		
		return result;
	}
	
	void DeserializeArsenalFromString(string data)
	{
		m_mArsenalInventory.Clear();
		
		if (data.IsEmpty())
			return;
		
		array<string> entries = new array<string>();
		data.Split(";", entries, false);
		
		for (int i = 0; i < entries.Count(); i++)
		{
			array<string> parts = new array<string>();
			entries[i].Split(":", parts, false);
			
			if (parts.Count() != 2)
				continue;
			
			string itemID = parts[0];
			int count = parts[1].ToInt();
			
			m_mArsenalInventory.Set(itemID, count);
		}
	}
}
