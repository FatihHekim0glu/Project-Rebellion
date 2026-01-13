// ============================================================================
// PROJECT REBELLION - Shop/Arsenal Menu (Legacy Compatibility)
// This file provides backwards compatibility redirects
// Main implementation is in Scripts/Game/Shop/RBL_ShopManager.c
// ============================================================================

// Legacy class - now redirects to RBL_ShopManager in Shop folder
// Kept for backwards compatibility with existing code references

class RBL_ShopMenuLegacy
{
	static RBL_ShopManager GetShopManager()
	{
		return RBL_ShopManager.GetInstance();
	}
	
	static void OpenShop()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.OpenMenu();
	}
	
	static void CloseShop()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.CloseMenu();
	}
	
	static void ToggleShop()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.ToggleMenu();
	}
	
	static bool PurchaseItem(string itemID)
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			return shop.PurchaseItem(itemID);
		return false;
	}
	
	static void PrintShop()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.PrintShopToConsole();
	}
}
