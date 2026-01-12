// ============================================================================
// PROJECT REBELLION - Economy Manager
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

	int GetMoney() { return m_iMoney; }

	bool CanAfford(int amount) { return m_iMoney >= amount; }

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

	void PrintEconomyStatus()
	{
		PrintFormat("[RBL_Economy] === ECONOMY STATUS ===");
		PrintFormat("Money: $%1", m_iMoney);
		PrintFormat("Human Resources: %1", m_iHumanResources);
		PrintFormat("Arsenal Items: %1", m_mArsenalInventory.Count());
		PrintFormat("Unlocked Items: %1", m_sUnlockedItems.Count());
	}
}
