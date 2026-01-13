// ============================================================================
// PROJECT REBELLION - Shop Item Data Classes
// Unified data structures for the shop/arsenal system
// ============================================================================

// Shop item type enumeration
enum ERBLShopItemType
{
	WEAPON,
	EQUIPMENT,
	SUPPLY,
	VEHICLE,
	RECRUIT
}

// Base shop item class - single source of truth for all shop items
class RBL_ShopItem
{
	string m_sID;              // Unique identifier (e.g., "akm", "uaz")
	string m_sDisplayName;     // Display name for UI
	string m_sDescription;     // Item description
	string m_sCategory;        // Category name for grouping
	ERBLShopItemType m_eType;  // Item type enum
	
	int m_iPrice;              // Money cost
	int m_iHRCost;             // Human resources cost (recruits)
	
	string m_sPrefabPath;      // Enfusion prefab path for spawning
	
	bool m_bRequiresUnlock;    // Requires unlock before available
	bool m_bUnlocked;          // Currently unlocked
	int m_iRequiredWarLevel;   // Minimum war level to purchase
	
	void RBL_ShopItem()
	{
		m_sID = "";
		m_sDisplayName = "";
		m_sDescription = "";
		m_sCategory = "";
		m_eType = ERBLShopItemType.EQUIPMENT;
		m_iPrice = 0;
		m_iHRCost = 0;
		m_sPrefabPath = "";
		m_bRequiresUnlock = false;
		m_bUnlocked = true;
		m_iRequiredWarLevel = 1;
	}
	
	// Builder pattern for fluent construction
	RBL_ShopItem SetID(string id)
	{
		m_sID = id;
		return this;
	}
	
	RBL_ShopItem SetName(string name)
	{
		m_sDisplayName = name;
		return this;
	}
	
	RBL_ShopItem SetDescription(string desc)
	{
		m_sDescription = desc;
		return this;
	}
	
	RBL_ShopItem SetCategory(string category)
	{
		m_sCategory = category;
		return this;
	}
	
	RBL_ShopItem SetType(ERBLShopItemType type)
	{
		m_eType = type;
		return this;
	}
	
	RBL_ShopItem SetPrice(int price)
	{
		m_iPrice = price;
		return this;
	}
	
	RBL_ShopItem SetHRCost(int hr)
	{
		m_iHRCost = hr;
		return this;
	}
	
	RBL_ShopItem SetPrefab(string prefab)
	{
		m_sPrefabPath = prefab;
		return this;
	}
	
	RBL_ShopItem SetRequiredWarLevel(int level)
	{
		m_iRequiredWarLevel = level;
		return this;
	}
	
	RBL_ShopItem SetRequiresUnlock(bool requires)
	{
		m_bRequiresUnlock = requires;
		return this;
	}
	
	// Getters for compatibility with both old field naming conventions
	string GetID() { return m_sID; }
	string GetDisplayName() { return m_sDisplayName; }
	string GetDescription() { return m_sDescription; }
	string GetCategory() { return m_sCategory; }
	ERBLShopItemType GetType() { return m_eType; }
	int GetPrice() { return m_iPrice; }
	int GetHRCost() { return m_iHRCost; }
	string GetPrefabPath() { return m_sPrefabPath; }
	int GetRequiredWarLevel() { return m_iRequiredWarLevel; }
	bool IsUnlocked() { return m_bUnlocked; }
	bool RequiresUnlock() { return m_bRequiresUnlock; }
	
	// Legacy compatibility aliases
	string ID() { return m_sID; }
	string DisplayName() { return m_sDisplayName; }
	string PrefabPath() { return m_sPrefabPath; }
	int Price() { return m_iPrice; }
	int HRCost() { return m_iHRCost; }
	string Category() { return m_sCategory; }
	
	// Check if purchasable at current war level
	bool IsAvailableAtWarLevel(int currentWarLevel)
	{
		return currentWarLevel >= m_iRequiredWarLevel;
	}
	
	// Check if item can be purchased
	bool CanPurchase(int money, int hr, int warLevel)
	{
		if (m_bRequiresUnlock && !m_bUnlocked)
			return false;
		if (warLevel < m_iRequiredWarLevel)
			return false;
		if (money < m_iPrice)
			return false;
		if (hr < m_iHRCost)
			return false;
		return true;
	}
	
	// Get unavailability reason
	string GetUnavailableReason(int money, int hr, int warLevel)
	{
		if (m_bRequiresUnlock && !m_bUnlocked)
			return "Item locked";
		if (warLevel < m_iRequiredWarLevel)
			return string.Format("Requires War Level %1", m_iRequiredWarLevel);
		if (money < m_iPrice)
			return "Insufficient funds";
		if (hr < m_iHRCost)
			return "Insufficient HR";
		return "";
	}
	
	// Serialization for persistence
	void Serialize(Serializer ctx)
	{
		ctx.Write(m_sID);
		ctx.Write(m_bUnlocked);
	}
	
	bool Deserialize(Serializer ctx)
	{
		if (!ctx.Read(m_sID))
			return false;
		if (!ctx.Read(m_bUnlocked))
			return false;
		return true;
	}
}

// Shop category for grouping items
class RBL_ShopCategory
{
	string m_sID;
	string m_sDisplayName;
	ERBLShopItemType m_eType;
	ref array<ref RBL_ShopItem> m_aItems;
	
	void RBL_ShopCategory(string id, string displayName, ERBLShopItemType type)
	{
		m_sID = id;
		m_sDisplayName = displayName;
		m_eType = type;
		m_aItems = new array<ref RBL_ShopItem>();
	}
	
	void AddItem(RBL_ShopItem item)
	{
		if (item)
			m_aItems.Insert(item);
	}
	
	RBL_ShopItem GetItemByID(string id)
	{
		foreach (RBL_ShopItem item : m_aItems)
		{
			if (item.GetID() == id)
				return item;
		}
		return null;
	}
	
	int GetItemCount()
	{
		return m_aItems.Count();
	}
	
	RBL_ShopItem GetItemByIndex(int index)
	{
		if (index >= 0 && index < m_aItems.Count())
			return m_aItems[index];
		return null;
	}
	
	array<ref RBL_ShopItem> GetItems()
	{
		return m_aItems;
	}
	
	string GetID() { return m_sID; }
	string GetDisplayName() { return m_sDisplayName; }
	ERBLShopItemType GetType() { return m_eType; }
}

// Purchase result for tracking
class RBL_PurchaseResult
{
	bool m_bSuccess;
	string m_sItemID;
	string m_sItemName;
	int m_iMoneyCost;
	int m_iHRCost;
	string m_sFailureReason;
	
	void RBL_PurchaseResult()
	{
		m_bSuccess = false;
		m_sItemID = "";
		m_sItemName = "";
		m_iMoneyCost = 0;
		m_iHRCost = 0;
		m_sFailureReason = "";
	}
	
	static RBL_PurchaseResult Success(RBL_ShopItem item)
	{
		RBL_PurchaseResult result = new RBL_PurchaseResult();
		result.m_bSuccess = true;
		result.m_sItemID = item.GetID();
		result.m_sItemName = item.GetDisplayName();
		result.m_iMoneyCost = item.GetPrice();
		result.m_iHRCost = item.GetHRCost();
		return result;
	}
	
	static RBL_PurchaseResult Failure(string reason)
	{
		RBL_PurchaseResult result = new RBL_PurchaseResult();
		result.m_bSuccess = false;
		result.m_sFailureReason = reason;
		return result;
	}
	
	bool IsSuccess() { return m_bSuccess; }
	string GetFailureReason() { return m_sFailureReason; }
}

// Item catalog - static definition of all available items
class RBL_ShopItemCatalog
{
	// Create weapon item helper
	static RBL_ShopItem CreateWeapon(string id, string name, string desc, int price, string prefab, int warLevel = 1)
	{
		RBL_ShopItem item = new RBL_ShopItem();
		item.SetID(id)
			.SetName(name)
			.SetDescription(desc)
			.SetCategory("Weapons")
			.SetType(ERBLShopItemType.WEAPON)
			.SetPrice(price)
			.SetPrefab(prefab)
			.SetRequiredWarLevel(warLevel);
		return item;
	}
	
	// Create equipment item helper
	static RBL_ShopItem CreateEquipment(string id, string name, string desc, int price, string prefab, int warLevel = 1)
	{
		RBL_ShopItem item = new RBL_ShopItem();
		item.SetID(id)
			.SetName(name)
			.SetDescription(desc)
			.SetCategory("Equipment")
			.SetType(ERBLShopItemType.EQUIPMENT)
			.SetPrice(price)
			.SetPrefab(prefab)
			.SetRequiredWarLevel(warLevel);
		return item;
	}
	
	// Create supply item helper
	static RBL_ShopItem CreateSupply(string id, string name, string desc, int price, string prefab)
	{
		RBL_ShopItem item = new RBL_ShopItem();
		item.SetID(id)
			.SetName(name)
			.SetDescription(desc)
			.SetCategory("Supplies")
			.SetType(ERBLShopItemType.SUPPLY)
			.SetPrice(price)
			.SetPrefab(prefab);
		return item;
	}
	
	// Create vehicle item helper
	static RBL_ShopItem CreateVehicle(string id, string name, string desc, int price, string prefab, int warLevel = 1)
	{
		RBL_ShopItem item = new RBL_ShopItem();
		item.SetID(id)
			.SetName(name)
			.SetDescription(desc)
			.SetCategory("Vehicles")
			.SetType(ERBLShopItemType.VEHICLE)
			.SetPrice(price)
			.SetPrefab(prefab)
			.SetRequiredWarLevel(warLevel);
		return item;
	}
	
	// Create recruit item helper
	static RBL_ShopItem CreateRecruit(string id, string name, string desc, int price, int hrCost, string prefab)
	{
		RBL_ShopItem item = new RBL_ShopItem();
		item.SetID(id)
			.SetName(name)
			.SetDescription(desc)
			.SetCategory("Recruitment")
			.SetType(ERBLShopItemType.RECRUIT)
			.SetPrice(price)
			.SetHRCost(hrCost)
			.SetPrefab(prefab);
		return item;
	}
}

