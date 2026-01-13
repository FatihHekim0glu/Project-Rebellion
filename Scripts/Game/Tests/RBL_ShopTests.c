// ============================================================================
// PROJECT REBELLION - Shop System Tests
// Comprehensive automated testing for the shop/arsenal system
// ============================================================================

class RBL_ShopTests
{
	// Run all shop tests
	static void RunAll()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("\n[RBL_ShopTests] Starting shop system tests...\n");
		
		TestShopItemClass();
		TestShopItemCatalog();
		TestShopCategory();
		TestShopManager();
		TestShopManagerLookup();
		TestShopManagerPricing();
		TestPurchaseValidation();
		TestShopMenuWidget();
		
		PrintFormat("\n[RBL_ShopTests] Shop tests complete.\n");
	}
	
	// Test RBL_ShopItem class
	static void TestShopItemClass()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_ShopTests] Testing ShopItem class...");
		
		// Test basic creation
		RBL_ShopItem item = new RBL_ShopItem();
		runner.AssertNotNull("ShopItem created", item);
		
		// Test default values
		runner.AssertStringEqual("Default ID is empty", "", item.GetID());
		runner.AssertEqual("Default price is 0", 0, item.GetPrice());
		runner.AssertEqual("Default HR cost is 0", 0, item.GetHRCost());
		runner.AssertEqual("Default war level is 1", 1, item.GetRequiredWarLevel());
		runner.Assert("Default unlocked is true", item.IsUnlocked(), "Not unlocked by default");
		
		// Test builder pattern
		item.SetID("test_item")
			.SetName("Test Item")
			.SetDescription("A test item")
			.SetCategory("TestCategory")
			.SetPrice(100)
			.SetHRCost(2)
			.SetPrefab("{TestPrefab}")
			.SetRequiredWarLevel(3);
		
		runner.AssertStringEqual("SetID works", "test_item", item.GetID());
		runner.AssertStringEqual("SetName works", "Test Item", item.GetDisplayName());
		runner.AssertStringEqual("SetDescription works", "A test item", item.GetDescription());
		runner.AssertStringEqual("SetCategory works", "TestCategory", item.GetCategory());
		runner.AssertEqual("SetPrice works", 100, item.GetPrice());
		runner.AssertEqual("SetHRCost works", 2, item.GetHRCost());
		runner.AssertStringEqual("SetPrefab works", "{TestPrefab}", item.GetPrefabPath());
		runner.AssertEqual("SetRequiredWarLevel works", 3, item.GetRequiredWarLevel());
		
		// Test legacy aliases
		runner.AssertStringEqual("Legacy ID() alias", "test_item", item.ID());
		runner.AssertStringEqual("Legacy DisplayName() alias", "Test Item", item.DisplayName());
		runner.AssertEqual("Legacy Price() alias", 100, item.Price());
		runner.AssertEqual("Legacy HRCost() alias", 2, item.HRCost());
		
		// Test type enum
		item.SetType(ERBLShopItemType.WEAPON);
		runner.Assert("SetType WEAPON works", item.GetType() == ERBLShopItemType.WEAPON, "Wrong type");
		
		item.SetType(ERBLShopItemType.VEHICLE);
		runner.Assert("SetType VEHICLE works", item.GetType() == ERBLShopItemType.VEHICLE, "Wrong type");
	}
	
	// Test RBL_ShopItemCatalog factory methods
	static void TestShopItemCatalog()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_ShopTests] Testing ShopItemCatalog factories...");
		
		// Test CreateWeapon
		RBL_ShopItem weapon = RBL_ShopItemCatalog.CreateWeapon(
			"test_ak", "AK-Test", "Test weapon", 200, "{AKPrefab}", 2);
		
		runner.AssertNotNull("CreateWeapon returns item", weapon);
		runner.AssertStringEqual("Weapon ID", "test_ak", weapon.GetID());
		runner.AssertStringEqual("Weapon name", "AK-Test", weapon.GetDisplayName());
		runner.AssertEqual("Weapon price", 200, weapon.GetPrice());
		runner.AssertEqual("Weapon type", ERBLShopItemType.WEAPON, weapon.GetType());
		runner.AssertEqual("Weapon war level", 2, weapon.GetRequiredWarLevel());
		runner.AssertStringEqual("Weapon category", "Weapons", weapon.GetCategory());
		
		// Test CreateEquipment
		RBL_ShopItem equip = RBL_ShopItemCatalog.CreateEquipment(
			"test_vest", "Vest", "Test vest", 150, "{VestPrefab}");
		
		runner.AssertNotNull("CreateEquipment returns item", equip);
		runner.AssertEqual("Equipment type", ERBLShopItemType.EQUIPMENT, equip.GetType());
		runner.AssertStringEqual("Equipment category", "Equipment", equip.GetCategory());
		
		// Test CreateVehicle
		RBL_ShopItem vehicle = RBL_ShopItemCatalog.CreateVehicle(
			"test_car", "Car", "Test car", 1000, "{CarPrefab}", 5);
		
		runner.AssertNotNull("CreateVehicle returns item", vehicle);
		runner.AssertEqual("Vehicle type", ERBLShopItemType.VEHICLE, vehicle.GetType());
		runner.AssertEqual("Vehicle war level", 5, vehicle.GetRequiredWarLevel());
		
		// Test CreateRecruit
		RBL_ShopItem recruit = RBL_ShopItemCatalog.CreateRecruit(
			"test_soldier", "Soldier", "Test soldier", 100, 1, "{SoldierPrefab}");
		
		runner.AssertNotNull("CreateRecruit returns item", recruit);
		runner.AssertEqual("Recruit type", ERBLShopItemType.RECRUIT, recruit.GetType());
		runner.AssertEqual("Recruit HR cost", 1, recruit.GetHRCost());
		runner.AssertStringEqual("Recruit category", "Recruitment", recruit.GetCategory());
		
		// Test CreateSupply
		RBL_ShopItem supply = RBL_ShopItemCatalog.CreateSupply(
			"test_ammo", "Ammo", "Test ammo", 20, "{AmmoPrefab}");
		
		runner.AssertNotNull("CreateSupply returns item", supply);
		runner.AssertEqual("Supply type", ERBLShopItemType.SUPPLY, supply.GetType());
	}
	
	// Test RBL_ShopCategory class
	static void TestShopCategory()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_ShopTests] Testing ShopCategory class...");
		
		// Test creation
		RBL_ShopCategory category = new RBL_ShopCategory("weapons", "WEAPONS", ERBLShopItemType.WEAPON);
		
		runner.AssertNotNull("ShopCategory created", category);
		runner.AssertStringEqual("Category ID", "weapons", category.GetID());
		runner.AssertStringEqual("Category display name", "WEAPONS", category.GetDisplayName());
		runner.AssertEqual("Category type", ERBLShopItemType.WEAPON, category.GetType());
		runner.AssertEqual("Initial item count is 0", 0, category.GetItemCount());
		
		// Test adding items
		RBL_ShopItem item1 = new RBL_ShopItem();
		item1.SetID("item1").SetName("Item 1");
		
		RBL_ShopItem item2 = new RBL_ShopItem();
		item2.SetID("item2").SetName("Item 2");
		
		category.AddItem(item1);
		runner.AssertEqual("Item count after add", 1, category.GetItemCount());
		
		category.AddItem(item2);
		runner.AssertEqual("Item count after second add", 2, category.GetItemCount());
		
		// Test GetItemByID
		RBL_ShopItem found = category.GetItemByID("item1");
		runner.AssertNotNull("GetItemByID finds item", found);
		runner.AssertStringEqual("Found item has correct ID", "item1", found.GetID());
		
		RBL_ShopItem notFound = category.GetItemByID("nonexistent");
		runner.AssertNull("GetItemByID returns null for missing", notFound);
		
		// Test GetItemByIndex
		RBL_ShopItem byIndex = category.GetItemByIndex(0);
		runner.AssertNotNull("GetItemByIndex(0) returns item", byIndex);
		
		RBL_ShopItem outOfBounds = category.GetItemByIndex(999);
		runner.AssertNull("GetItemByIndex out of bounds returns null", outOfBounds);
		
		// Test GetItems
		array<ref RBL_ShopItem> items = category.GetItems();
		runner.AssertNotNull("GetItems returns array", items);
		runner.AssertEqual("GetItems count matches", 2, items.Count());
	}
	
	// Test RBL_ShopManager singleton
	static void TestShopManager()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_ShopTests] Testing ShopManager...");
		
		// Test singleton
		RBL_ShopManager shop1 = RBL_ShopManager.GetInstance();
		runner.AssertNotNull("ShopManager singleton exists", shop1);
		
		RBL_ShopManager shop2 = RBL_ShopManager.GetInstance();
		runner.Assert("Same instance returned", shop1 == shop2, "Different instances");
		
		// Test initialization
		runner.Assert("ShopManager is initialized", shop1.IsInitialized(), "Not initialized");
		
		// Test categories
		int categoryCount = shop1.GetCategoryCount();
		runner.Assert("Has categories", categoryCount > 0, "No categories");
		runner.Assert("Has expected categories (5)", categoryCount == 5, 
			string.Format("Expected 5, got %1", categoryCount));
		
		// Test total items
		int itemCount = shop1.GetTotalItemCount();
		runner.Assert("Has items", itemCount > 0, "No items");
		runner.Assert("Has expected items (>25)", itemCount >= 25, 
			string.Format("Expected 25+, got %1", itemCount));
		
		// Test category access
		array<ref RBL_ShopCategory> categories = shop1.GetCategories();
		runner.AssertNotNull("GetCategories returns array", categories);
		runner.AssertEqual("GetCategories count matches", categoryCount, categories.Count());
		
		// Test GetCategoryByID
		RBL_ShopCategory weapons = shop1.GetCategoryByID("weapons");
		runner.AssertNotNull("GetCategoryByID weapons", weapons);
		runner.AssertStringEqual("Weapons category ID", "weapons", weapons.GetID());
		
		RBL_ShopCategory vehicles = shop1.GetCategoryByID("vehicles");
		runner.AssertNotNull("GetCategoryByID vehicles", vehicles);
		
		RBL_ShopCategory recruits = shop1.GetCategoryByID("recruits");
		runner.AssertNotNull("GetCategoryByID recruits", recruits);
		
		// Test GetCategoryByIndex
		RBL_ShopCategory firstCat = shop1.GetCategoryByIndex(0);
		runner.AssertNotNull("GetCategoryByIndex(0) returns category", firstCat);
		
		// Test events
		runner.AssertNotNull("OnMenuOpened invoker", shop1.GetOnMenuOpened());
		runner.AssertNotNull("OnMenuClosed invoker", shop1.GetOnMenuClosed());
		runner.AssertNotNull("OnPurchase invoker", shop1.GetOnPurchase());
		runner.AssertNotNull("OnPurchaseFailed invoker", shop1.GetOnPurchaseFailed());
	}
	
	// Test ShopManager item lookup
	static void TestShopManagerLookup()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_ShopTests] Testing ShopManager item lookup...");
		
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (!shop)
		{
			runner.RecordResult("ShopManager lookup tests", false, "No shop manager");
			return;
		}
		
		// Test GetItemByID
		RBL_ShopItem akm = shop.GetItemByID("akm");
		runner.AssertNotNull("GetItemByID akm", akm);
		if (akm)
		{
			runner.AssertStringEqual("AKM ID correct", "akm", akm.GetID());
			runner.Assert("AKM has price > 0", akm.GetPrice() > 0, "No price");
		}
		
		RBL_ShopItem uaz = shop.GetItemByID("uaz");
		runner.AssertNotNull("GetItemByID uaz", uaz);
		if (uaz)
		{
			runner.Assert("UAZ is VEHICLE type", uaz.GetType() == ERBLShopItemType.VEHICLE, "Wrong type");
		}
		
		RBL_ShopItem rifleman = shop.GetItemByID("rifleman");
		runner.AssertNotNull("GetItemByID rifleman", rifleman);
		if (rifleman)
		{
			runner.Assert("Rifleman has HR cost", rifleman.GetHRCost() > 0, "No HR cost");
			runner.Assert("Rifleman is RECRUIT type", rifleman.GetType() == ERBLShopItemType.RECRUIT, "Wrong type");
		}
		
		// Test FindItem alias
		RBL_ShopItem foundItem = shop.FindItem("ak74");
		runner.AssertNotNull("FindItem alias works", foundItem);
		
		// Test ItemExists
		runner.Assert("ItemExists akm", shop.ItemExists("akm"), "akm not found");
		runner.Assert("ItemExists uaz", shop.ItemExists("uaz"), "uaz not found");
		runner.Assert("!ItemExists fake_item", !shop.ItemExists("fake_item_xyz"), "Fake item found");
		
		// Test missing item
		RBL_ShopItem missing = shop.GetItemByID("nonexistent_item");
		runner.AssertNull("GetItemByID nonexistent returns null", missing);
		
		// Test legacy getters
		array<ref RBL_ShopItem> weapons = shop.GetWeapons();
		runner.AssertNotNull("GetWeapons returns array", weapons);
		runner.Assert("GetWeapons has items", weapons.Count() > 0, "No weapons");
		
		array<ref RBL_ShopItem> vehicles = shop.GetVehicles();
		runner.AssertNotNull("GetVehicles returns array", vehicles);
		runner.Assert("GetVehicles has items", vehicles.Count() > 0, "No vehicles");
		
		array<ref RBL_ShopItem> recruits = shop.GetRecruits();
		runner.AssertNotNull("GetRecruits returns array", recruits);
		runner.Assert("GetRecruits has items", recruits.Count() > 0, "No recruits");
	}
	
	// Test ShopManager pricing methods
	static void TestShopManagerPricing()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_ShopTests] Testing ShopManager pricing...");
		
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (!shop)
		{
			runner.RecordResult("ShopManager pricing tests", false, "No shop manager");
			return;
		}
		
		// Test GetItemPrice
		int akmPrice = shop.GetItemPrice("akm");
		runner.Assert("GetItemPrice akm > 0", akmPrice > 0, "No price");
		runner.Assert("GetItemPrice akm reasonable", akmPrice >= 100 && akmPrice <= 500, 
			string.Format("Price %1 out of expected range", akmPrice));
		
		// Test GetVehiclePrice (critical - was missing before)
		int uazPrice = shop.GetVehiclePrice("uaz");
		runner.Assert("GetVehiclePrice uaz > 0", uazPrice > 0, "No vehicle price");
		runner.Assert("GetVehiclePrice uaz reasonable", uazPrice >= 400 && uazPrice <= 1000,
			string.Format("Price %1 out of expected range", uazPrice));
		
		int btrPrice = shop.GetVehiclePrice("btr70");
		runner.Assert("GetVehiclePrice btr70 > 0", btrPrice > 0, "No BTR price");
		runner.Assert("GetVehiclePrice btr70 > uaz", btrPrice > uazPrice, "BTR should cost more than UAZ");
		
		// Test GetWeaponPrice
		int weaponPrice = shop.GetWeaponPrice("ak74");
		runner.Assert("GetWeaponPrice ak74 > 0", weaponPrice > 0, "No weapon price");
		
		// Test GetRecruitPrice
		int recruitPrice = shop.GetRecruitPrice("rifleman");
		runner.Assert("GetRecruitPrice rifleman > 0", recruitPrice > 0, "No recruit price");
		
		// Test GetRecruitHRCost
		int recruitHR = shop.GetRecruitHRCost("rifleman");
		runner.Assert("GetRecruitHRCost rifleman > 0", recruitHR > 0, "No HR cost");
		
		// Test invalid item prices
		runner.AssertEqual("GetItemPrice nonexistent = 0", 0, shop.GetItemPrice("nonexistent"));
		runner.AssertEqual("GetVehiclePrice nonexistent = 0", 0, shop.GetVehiclePrice("nonexistent"));
	}
	
	// Test purchase validation
	static void TestPurchaseValidation()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_ShopTests] Testing purchase validation...");
		
		// Create test item
		RBL_ShopItem item = new RBL_ShopItem();
		item.SetID("test")
			.SetName("Test Item")
			.SetPrice(100)
			.SetHRCost(2)
			.SetRequiredWarLevel(3);
		
		// Test CanPurchase - all requirements met
		runner.Assert("CanPurchase with enough resources", 
			item.CanPurchase(200, 5, 5), "Should be able to purchase");
		
		// Test CanPurchase - insufficient money
		runner.Assert("!CanPurchase insufficient money", 
			!item.CanPurchase(50, 5, 5), "Should not afford");
		
		// Test CanPurchase - insufficient HR
		runner.Assert("!CanPurchase insufficient HR", 
			!item.CanPurchase(200, 1, 5), "Should not afford HR");
		
		// Test CanPurchase - insufficient war level
		runner.Assert("!CanPurchase insufficient war level", 
			!item.CanPurchase(200, 5, 1), "War level too low");
		
		// Test GetUnavailableReason
		string reason1 = item.GetUnavailableReason(50, 5, 5);
		runner.Assert("GetUnavailableReason money", reason1.Contains("funds"), 
			"Should mention funds");
		
		string reason2 = item.GetUnavailableReason(200, 1, 5);
		runner.Assert("GetUnavailableReason HR", reason2.Contains("HR"), 
			"Should mention HR");
		
		string reason3 = item.GetUnavailableReason(200, 5, 1);
		runner.Assert("GetUnavailableReason war level", reason3.Contains("War Level"), 
			"Should mention war level");
		
		string reason4 = item.GetUnavailableReason(200, 5, 5);
		runner.Assert("GetUnavailableReason empty when can afford", reason4.IsEmpty(), 
			"Should be empty when affordable");
		
		// Test locked item
		item.SetRequiresUnlock(true);
		item.m_bUnlocked = false;
		runner.Assert("!CanPurchase locked item", 
			!item.CanPurchase(200, 5, 5), "Locked item should not be purchasable");
		
		string reason5 = item.GetUnavailableReason(200, 5, 5);
		runner.Assert("GetUnavailableReason locked", reason5.Contains("locked"), 
			"Should mention locked");
		
		// Test IsAvailableAtWarLevel
		item.SetRequiredWarLevel(5);
		runner.Assert("IsAvailableAtWarLevel WL5 at WL5", item.IsAvailableAtWarLevel(5), "Should be available");
		runner.Assert("IsAvailableAtWarLevel WL5 at WL10", item.IsAvailableAtWarLevel(10), "Should be available");
		runner.Assert("!IsAvailableAtWarLevel WL5 at WL3", !item.IsAvailableAtWarLevel(3), "Should not be available");
	}
	
	// Test RBL_ShopMenuWidgetImpl
	static void TestShopMenuWidget()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		
		PrintFormat("[RBL_ShopTests] Testing ShopMenuWidget...");
		
		// Test widget creation
		RBL_ShopMenuWidgetImpl widget = new RBL_ShopMenuWidgetImpl();
		runner.AssertNotNull("ShopMenuWidget created", widget);
		
		// Test initial state
		runner.Assert("Widget not visible initially", !widget.IsVisible(), "Should not be visible");
		runner.AssertEqual("Initial category index is 0", 0, widget.GetSelectedCategoryIndex());
		runner.AssertEqual("Initial item index is 0", 0, widget.GetSelectedItemIndex());
		
		// Test category count from manager
		int catCount = widget.GetCategoryCount();
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
		{
			runner.AssertEqual("Widget category count matches manager", 
				shop.GetCategoryCount(), catCount);
		}
		
		// Test Open/Close
		widget.Open();
		runner.Assert("Widget visible after Open", widget.IsVisible(), "Should be visible");
		
		widget.Close();
		// Note: Close triggers animation, so IsVisible might still be true briefly
		
		// Test Toggle
		widget.Toggle();
		runner.Assert("Widget visible after Toggle from closed", widget.IsVisible(), "Should be visible after toggle");
		
		// Test navigation
		widget.SelectNextCategory();
		runner.AssertEqual("Category index after SelectNextCategory", 1, widget.GetSelectedCategoryIndex());
		
		widget.SelectPreviousCategory();
		runner.AssertEqual("Category index after SelectPreviousCategory", 0, widget.GetSelectedCategoryIndex());
		
		widget.SelectNextItem();
		runner.AssertEqual("Item index after SelectNextItem", 1, widget.GetSelectedItemIndex());
		
		widget.SelectPreviousItem();
		runner.AssertEqual("Item index after SelectPreviousItem", 0, widget.GetSelectedItemIndex());
		
		// Cleanup
		widget.Close();
	}
}

// Console command to run shop tests
class RBL_ShopTestCommands
{
	static void RunShopTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_ShopTests.RunAll();
		
		runner.PrintResults();
	}
	
	static void RunItemTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_ShopTests.TestShopItemClass();
		RBL_ShopTests.TestShopItemCatalog();
		RBL_ShopTests.TestShopCategory();
		
		runner.PrintResults();
	}
	
	static void RunManagerTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_ShopTests.TestShopManager();
		RBL_ShopTests.TestShopManagerLookup();
		RBL_ShopTests.TestShopManagerPricing();
		
		runner.PrintResults();
	}
	
	static void RunPurchaseTests()
	{
		RBL_TestRunner runner = RBL_TestRunner.GetInstance();
		runner.Reset();
		
		RBL_ShopTests.TestPurchaseValidation();
		
		runner.PrintResults();
	}
}

