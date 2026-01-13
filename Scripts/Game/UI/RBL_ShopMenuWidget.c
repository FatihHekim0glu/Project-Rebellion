// ============================================================================
// PROJECT REBELLION - Shop Menu Widget
// Full-screen shop for purchasing items, vehicles, and recruiting units
// ============================================================================

// Shop item data
class RBL_ShopItem
{
	string m_sID;
	string m_sName;
	string m_sDescription;
	string m_sCategory;
	int m_iMoneyCost;
	int m_iHRCost;
	bool m_bUnlocked;
	bool m_bAvailable;
	
	void RBL_ShopItem(string id, string name, string desc, string category, int money, int hr)
	{
		m_sID = id;
		m_sName = name;
		m_sDescription = desc;
		m_sCategory = category;
		m_iMoneyCost = money;
		m_iHRCost = hr;
		m_bUnlocked = true;
		m_bAvailable = true;
	}
}

// Shop category
class RBL_ShopCategory
{
	string m_sID;
	string m_sName;
	ref array<ref RBL_ShopItem> m_aItems;
	
	void RBL_ShopCategory(string id, string name)
	{
		m_sID = id;
		m_sName = name;
		m_aItems = new array<ref RBL_ShopItem>();
	}
	
	void AddItem(RBL_ShopItem item)
	{
		m_aItems.Insert(item);
	}
}

// Main shop menu
class RBL_ShopMenuWidgetImpl : RBL_PanelWidget
{
	protected bool m_bMenuVisible;
	protected ref array<ref RBL_ShopCategory> m_aCategories;
	protected int m_iSelectedCategoryIndex;
	protected int m_iSelectedItemIndex;
	protected int m_iScrollOffset;
	protected int m_iMaxVisibleItems;
	
	// Player resources
	protected int m_iPlayerMoney;
	protected int m_iPlayerHR;
	
	// Animation
	protected float m_fOpenAnimation;
	protected bool m_bClosing;
	
	void RBL_ShopMenuWidgetImpl()
	{
		// Centered position
		vector pos = RBL_UILayout.GetCenteredPosition(RBL_UISizes.SHOP_WIDTH, RBL_UISizes.SHOP_HEIGHT);
		m_fPosX = pos[0];
		m_fPosY = pos[1];
		m_fWidth = RBL_UISizes.SHOP_WIDTH;
		m_fHeight = RBL_UISizes.SHOP_HEIGHT;
		
		m_bMenuVisible = false;
		m_iSelectedCategoryIndex = 0;
		m_iSelectedItemIndex = 0;
		m_iScrollOffset = 0;
		m_iMaxVisibleItems = 8;
		
		m_iPlayerMoney = 0;
		m_iPlayerHR = 0;
		
		m_fOpenAnimation = 0;
		m_bClosing = false;
		
		m_iBackgroundColor = RBL_UIColors.COLOR_BG_DARK;
		m_iBorderColor = RBL_UIColors.COLOR_BORDER_LIGHT;
		
		InitializeShopData();
	}
	
	protected void InitializeShopData()
	{
		m_aCategories = new array<ref RBL_ShopCategory>();
		
		// ---- WEAPONS ----
		RBL_ShopCategory weapons = new RBL_ShopCategory("weapons", "WEAPONS");
		weapons.AddItem(new RBL_ShopItem("ak74", "AK-74", "Standard assault rifle", "weapons", 150, 0));
		weapons.AddItem(new RBL_ShopItem("akm", "AKM", "7.62mm assault rifle", "weapons", 120, 0));
		weapons.AddItem(new RBL_ShopItem("svd", "SVD Dragunov", "Marksman rifle", "weapons", 400, 0));
		weapons.AddItem(new RBL_ShopItem("pkm", "PKM", "General purpose machine gun", "weapons", 600, 0));
		weapons.AddItem(new RBL_ShopItem("rpg7", "RPG-7", "Rocket launcher", "weapons", 800, 0));
		weapons.AddItem(new RBL_ShopItem("makarov", "PM Makarov", "Compact pistol", "weapons", 50, 0));
		m_aCategories.Insert(weapons);
		
		// ---- EQUIPMENT ----
		RBL_ShopCategory equipment = new RBL_ShopCategory("equipment", "EQUIPMENT");
		equipment.AddItem(new RBL_ShopItem("vest_light", "Light Vest", "Basic protection", "equipment", 100, 0));
		equipment.AddItem(new RBL_ShopItem("vest_heavy", "Heavy Vest", "Enhanced protection", "equipment", 300, 0));
		equipment.AddItem(new RBL_ShopItem("helmet", "Combat Helmet", "Head protection", "equipment", 150, 0));
		equipment.AddItem(new RBL_ShopItem("nvg", "Night Vision", "NVG goggles", "equipment", 500, 0));
		equipment.AddItem(new RBL_ShopItem("binoculars", "Binoculars", "Observation tool", "equipment", 75, 0));
		equipment.AddItem(new RBL_ShopItem("radio", "Radio", "Team communication", "equipment", 100, 0));
		m_aCategories.Insert(equipment);
		
		// ---- SUPPLIES ----
		RBL_ShopCategory supplies = new RBL_ShopCategory("supplies", "SUPPLIES");
		supplies.AddItem(new RBL_ShopItem("ammo_rifle", "Rifle Ammo", "5.45/7.62mm magazines", "supplies", 20, 0));
		supplies.AddItem(new RBL_ShopItem("ammo_pistol", "Pistol Ammo", "9mm magazines", "supplies", 10, 0));
		supplies.AddItem(new RBL_ShopItem("grenade_frag", "F1 Grenade", "Fragmentation grenade", "supplies", 50, 0));
		supplies.AddItem(new RBL_ShopItem("grenade_smoke", "Smoke Grenade", "Smoke cover", "supplies", 30, 0));
		supplies.AddItem(new RBL_ShopItem("medkit", "Medical Kit", "Healing supplies", "supplies", 75, 0));
		supplies.AddItem(new RBL_ShopItem("bandage", "Bandage", "Basic medical", "supplies", 15, 0));
		m_aCategories.Insert(supplies);
		
		// ---- VEHICLES ----
		RBL_ShopCategory vehicles = new RBL_ShopCategory("vehicles", "VEHICLES");
		vehicles.AddItem(new RBL_ShopItem("uaz", "UAZ-469", "Light utility vehicle", "vehicles", 500, 0));
		vehicles.AddItem(new RBL_ShopItem("ural", "Ural Truck", "Transport truck", "vehicles", 800, 0));
		vehicles.AddItem(new RBL_ShopItem("btr", "BTR-70", "APC (requires War Level 5)", "vehicles", 3000, 0));
		vehicles.AddItem(new RBL_ShopItem("brdm", "BRDM-2", "Scout car", "vehicles", 1500, 0));
		m_aCategories.Insert(vehicles);
		
		// ---- RECRUITMENT ----
		RBL_ShopCategory recruit = new RBL_ShopCategory("recruit", "RECRUITMENT");
		recruit.AddItem(new RBL_ShopItem("militia", "Militia", "Basic fighter", "recruit", 100, 1));
		recruit.AddItem(new RBL_ShopItem("rifleman", "Rifleman", "Trained soldier", "recruit", 200, 1));
		recruit.AddItem(new RBL_ShopItem("marksman", "Marksman", "Sniper unit", "recruit", 400, 1));
		recruit.AddItem(new RBL_ShopItem("medic", "Combat Medic", "Healer unit", "recruit", 300, 1));
		recruit.AddItem(new RBL_ShopItem("at_soldier", "AT Specialist", "Anti-tank unit", "recruit", 500, 1));
		recruit.AddItem(new RBL_ShopItem("mg_soldier", "MG Gunner", "Support gunner", "recruit", 450, 1));
		m_aCategories.Insert(recruit);
	}
	
	void Toggle()
	{
		if (m_bMenuVisible)
		{
			Close();
		}
		else
		{
			Open();
		}
	}
	
	void Open()
	{
		m_bMenuVisible = true;
		m_bClosing = false;
		m_fOpenAnimation = 0;
		RefreshPlayerResources();
		RBL_Notifications.Show("Press [ESC] to close shop");
	}
	
	void Close()
	{
		m_bClosing = true;
	}
	
	override bool IsVisible()
	{
		return m_bMenuVisible;
	}
	
	override void Update(float timeSlice)
	{
		if (!m_bMenuVisible)
			return;
		
		// Open/close animation
		if (m_bClosing)
		{
			m_fOpenAnimation -= timeSlice * 4;
			if (m_fOpenAnimation <= 0)
			{
				m_fOpenAnimation = 0;
				m_bMenuVisible = false;
				m_bClosing = false;
			}
		}
		else
		{
			m_fOpenAnimation += timeSlice * 4;
			if (m_fOpenAnimation > 1.0)
				m_fOpenAnimation = 1.0;
		}
		
		m_fAlpha = m_fOpenAnimation;
		
		// Handle input
		HandleInput();
		
		// Refresh resources periodically
		RefreshPlayerResources();
	}
	
	protected void HandleInput()
	{
		InputManager input = GetGame().GetInputManager();
		if (!input)
			return;
		
		// Category navigation (Q/E or Tab)
		if (input.GetActionTriggered("MenuLeft"))
			SelectPreviousCategory();
		if (input.GetActionTriggered("MenuRight"))
			SelectNextCategory();
		
		// Item navigation (W/S or arrows)
		if (input.GetActionTriggered("MenuUp"))
			SelectPreviousItem();
		if (input.GetActionTriggered("MenuDown"))
			SelectNextItem();
		
		// Purchase (Enter or Space)
		if (input.GetActionTriggered("MenuSelect"))
			PurchaseSelectedItem();
		
		// Close (Escape)
		if (input.GetActionTriggered("MenuBack"))
			Close();
	}
	
	override void Draw()
	{
		if (!m_bMenuVisible || m_fAlpha <= 0.01)
			return;
		
		// Scale animation
		float scale = 0.9 + m_fOpenAnimation * 0.1;
		float scaledWidth = m_fWidth * scale;
		float scaledHeight = m_fHeight * scale;
		float offsetX = (m_fWidth - scaledWidth) / 2;
		float offsetY = (m_fHeight - scaledHeight) / 2;
		
		float drawX = m_fPosX + offsetX;
		float drawY = m_fPosY + offsetY;
		
		// Darkened backdrop
		int backdropColor = ApplyAlpha(ARGB(180, 0, 0, 0), m_fAlpha);
		DrawRect(0, 0, RBL_UISizes.REFERENCE_WIDTH, RBL_UISizes.REFERENCE_HEIGHT, backdropColor);
		
		// Main panel background
		int bgColor = ApplyAlpha(m_iBackgroundColor, m_fAlpha);
		DrawRect(drawX, drawY, scaledWidth, scaledHeight, bgColor);
		
		// Border
		int borderColor = ApplyAlpha(m_iBorderColor, m_fAlpha);
		DrawRectOutline(drawX, drawY, scaledWidth, scaledHeight, borderColor, 2);
		
		// Draw sections
		DrawHeader(drawX, drawY, scaledWidth);
		DrawCategories(drawX, drawY + 60, scaledWidth);
		DrawItems(drawX, drawY + 100, scaledWidth, scaledHeight - 160);
		DrawFooter(drawX, drawY + scaledHeight - 60, scaledWidth);
	}
	
	protected void DrawHeader(float x, float y, float width)
	{
		// Title
		int titleColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("Shop_Title", x + 16, y + 16);
		DbgUI.Text("FIA ARSENAL");
		DbgUI.End();
		
		// Player resources (right side)
		int moneyColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_AMBER, m_fAlpha);
		int hrColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_BLUE, m_fAlpha);
		
		DbgUI.Begin("Shop_Money", x + width - 200, y + 16);
		DbgUI.Text(RBL_UIStrings.FormatMoney(m_iPlayerMoney));
		DbgUI.End();
		
		DbgUI.Begin("Shop_HR", x + width - 80, y + 16);
		DbgUI.Text(RBL_UIStrings.FormatHR(m_iPlayerHR));
		DbgUI.End();
		
		// Separator line
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(x + 16, y + 50, width - 32, 1, lineColor);
	}
	
	protected void DrawCategories(float x, float y, float width)
	{
		float tabWidth = (width - 32) / m_aCategories.Count();
		
		for (int i = 0; i < m_aCategories.Count(); i++)
		{
			RBL_ShopCategory cat = m_aCategories[i];
			float tabX = x + 16 + i * tabWidth;
			
			// Tab background
			int tabColor = RBL_UIColors.COLOR_BG_MEDIUM;
			if (i == m_iSelectedCategoryIndex)
				tabColor = RBL_UIColors.COLOR_BG_HIGHLIGHT;
			tabColor = ApplyAlpha(tabColor, m_fAlpha);
			
			DrawRect(tabX, y, tabWidth - 4, 28, tabColor);
			
			// Tab text
			int textColor = RBL_UIColors.COLOR_TEXT_SECONDARY;
			if (i == m_iSelectedCategoryIndex)
				textColor = RBL_UIColors.COLOR_TEXT_BRIGHT;
			textColor = ApplyAlpha(textColor, m_fAlpha);
			
			DbgUI.Begin("Shop_Tab_" + i.ToString(), tabX + 8, y + 6);
			DbgUI.Text(cat.m_sName);
			DbgUI.End();
		}
	}
	
	protected void DrawItems(float x, float y, float width, float height)
	{
		if (m_iSelectedCategoryIndex >= m_aCategories.Count())
			return;
		
		RBL_ShopCategory category = m_aCategories[m_iSelectedCategoryIndex];
		float itemHeight = RBL_UISizes.SHOP_ITEM_HEIGHT;
		float listX = x + 16;
		float listWidth = width * 0.6 - 24;
		float detailX = x + width * 0.6;
		float detailWidth = width * 0.4 - 16;
		
		// Item list background
		int listBgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_MEDIUM, m_fAlpha * 0.5);
		DrawRect(listX, y, listWidth, height, listBgColor);
		
		// Draw visible items
		float currentY = y;
		for (int i = m_iScrollOffset; i < category.m_aItems.Count() && i < m_iScrollOffset + m_iMaxVisibleItems; i++)
		{
			RBL_ShopItem item = category.m_aItems[i];
			bool isSelected = (i == m_iSelectedItemIndex);
			
			DrawItemRow(item, listX, currentY, listWidth, itemHeight, isSelected);
			currentY += itemHeight + 4;
		}
		
		// Scroll indicators
		if (m_iScrollOffset > 0)
		{
			DbgUI.Begin("Shop_ScrollUp", listX + listWidth / 2 - 10, y - 16);
			DbgUI.Text("▲");
			DbgUI.End();
		}
		if (m_iScrollOffset + m_iMaxVisibleItems < category.m_aItems.Count())
		{
			DbgUI.Begin("Shop_ScrollDown", listX + listWidth / 2 - 10, y + height);
			DbgUI.Text("▼");
			DbgUI.End();
		}
		
		// Item details panel
		DrawItemDetails(detailX, y, detailWidth, height);
	}
	
	protected void DrawItemRow(RBL_ShopItem item, float x, float y, float width, float height, bool selected)
	{
		// Row background
		int rowColor = RBL_UIColors.COLOR_BG_LIGHT;
		if (selected)
			rowColor = RBL_UIColors.COLOR_BG_HIGHLIGHT;
		if (!CanAfford(item))
			rowColor = ApplyAlpha(rowColor, 0.5);
		rowColor = ApplyAlpha(rowColor, m_fAlpha);
		
		DrawRect(x, y, width, height, rowColor);
		
		// Selection indicator
		if (selected)
		{
			int indicatorColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_GREEN, m_fAlpha);
			DrawRect(x, y, 3, height, indicatorColor);
		}
		
		// Item name
		int nameColor = RBL_UIColors.COLOR_TEXT_BRIGHT;
		if (!CanAfford(item))
			nameColor = RBL_UIColors.COLOR_TEXT_MUTED;
		nameColor = ApplyAlpha(nameColor, m_fAlpha);
		
		DbgUI.Begin("Shop_Item_" + item.m_sID, x + 12, y + 8);
		DbgUI.Text(item.m_sName);
		DbgUI.End();
		
		// Price (right aligned)
		string priceText = "";
		int priceColor = RBL_UIColors.COLOR_TEXT_SECONDARY;
		
		if (item.m_iMoneyCost > 0)
		{
			priceText = RBL_UIStrings.FormatMoney(item.m_iMoneyCost);
			if (m_iPlayerMoney < item.m_iMoneyCost)
				priceColor = RBL_UIColors.COLOR_ACCENT_RED;
		}
		if (item.m_iHRCost > 0)
		{
			if (!priceText.IsEmpty())
				priceText = priceText + " + ";
			priceText = priceText + item.m_iHRCost.ToString() + " HR";
			if (m_iPlayerHR < item.m_iHRCost)
				priceColor = RBL_UIColors.COLOR_ACCENT_RED;
		}
		
		priceColor = ApplyAlpha(priceColor, m_fAlpha);
		
		DbgUI.Begin("Shop_Price_" + item.m_sID, x + width - 100, y + 8);
		DbgUI.Text(priceText);
		DbgUI.End();
	}
	
	protected void DrawItemDetails(float x, float y, float width, float height)
	{
		RBL_ShopItem selectedItem = GetSelectedItem();
		if (!selectedItem)
			return;
		
		// Details background
		int detailBg = ApplyAlpha(RBL_UIColors.COLOR_BG_MEDIUM, m_fAlpha * 0.5);
		DrawRect(x, y, width, height, detailBg);
		
		float contentX = x + 12;
		float contentY = y + 12;
		
		// Item name (large)
		int nameColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("Shop_Detail_Name", contentX, contentY);
		DbgUI.Text(selectedItem.m_sName);
		DbgUI.End();
		
		contentY += 32;
		
		// Description
		int descColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
		DbgUI.Begin("Shop_Detail_Desc", contentX, contentY);
		DbgUI.Text(selectedItem.m_sDescription);
		DbgUI.End();
		
		contentY += 48;
		
		// Separator
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(contentX, contentY, width - 24, 1, lineColor);
		contentY += 16;
		
		// Cost breakdown
		DbgUI.Begin("Shop_Detail_Cost_Label", contentX, contentY);
		DbgUI.Text("COST:");
		DbgUI.End();
		contentY += 20;
		
		if (selectedItem.m_iMoneyCost > 0)
		{
			int moneyColor = RBL_UIColors.COLOR_ACCENT_AMBER;
			if (m_iPlayerMoney < selectedItem.m_iMoneyCost)
				moneyColor = RBL_UIColors.COLOR_ACCENT_RED;
			moneyColor = ApplyAlpha(moneyColor, m_fAlpha);
			
			DbgUI.Begin("Shop_Detail_Money", contentX, contentY);
			DbgUI.Text("  " + RBL_UIStrings.FormatMoney(selectedItem.m_iMoneyCost));
			DbgUI.End();
			contentY += 20;
		}
		
		if (selectedItem.m_iHRCost > 0)
		{
			int hrColor = RBL_UIColors.COLOR_ACCENT_BLUE;
			if (m_iPlayerHR < selectedItem.m_iHRCost)
				hrColor = RBL_UIColors.COLOR_ACCENT_RED;
			hrColor = ApplyAlpha(hrColor, m_fAlpha);
			
			DbgUI.Begin("Shop_Detail_HR", contentX, contentY);
			DbgUI.Text("  " + selectedItem.m_iHRCost.ToString() + " Human Resources");
			DbgUI.End();
			contentY += 20;
		}
		
		// Purchase hint
		contentY = y + height - 40;
		string hintText = "[ENTER] Purchase";
		int hintColor = RBL_UIColors.COLOR_ACCENT_GREEN;
		
		if (!CanAfford(selectedItem))
		{
			hintText = "Insufficient resources";
			hintColor = RBL_UIColors.COLOR_ACCENT_RED;
		}
		
		hintColor = ApplyAlpha(hintColor, m_fAlpha);
		DbgUI.Begin("Shop_Detail_Hint", contentX, contentY);
		DbgUI.Text(hintText);
		DbgUI.End();
	}
	
	protected void DrawFooter(float x, float y, float width)
	{
		// Separator line
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(x + 16, y, width - 32, 1, lineColor);
		
		// Controls hint
		int hintColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
		DbgUI.Begin("Shop_Controls", x + 16, y + 16);
		DbgUI.Text("[Q/E] Category   [W/S] Navigate   [ENTER] Buy   [ESC] Close");
		DbgUI.End();
	}
	
	// Navigation
	void SelectNextCategory()
	{
		m_iSelectedCategoryIndex++;
		if (m_iSelectedCategoryIndex >= m_aCategories.Count())
			m_iSelectedCategoryIndex = 0;
		m_iSelectedItemIndex = 0;
		m_iScrollOffset = 0;
	}
	
	void SelectPreviousCategory()
	{
		m_iSelectedCategoryIndex--;
		if (m_iSelectedCategoryIndex < 0)
			m_iSelectedCategoryIndex = m_aCategories.Count() - 1;
		m_iSelectedItemIndex = 0;
		m_iScrollOffset = 0;
	}
	
	void SelectNextItem()
	{
		RBL_ShopCategory cat = m_aCategories[m_iSelectedCategoryIndex];
		m_iSelectedItemIndex++;
		if (m_iSelectedItemIndex >= cat.m_aItems.Count())
			m_iSelectedItemIndex = 0;
		
		// Adjust scroll
		if (m_iSelectedItemIndex >= m_iScrollOffset + m_iMaxVisibleItems)
			m_iScrollOffset = m_iSelectedItemIndex - m_iMaxVisibleItems + 1;
		if (m_iSelectedItemIndex < m_iScrollOffset)
			m_iScrollOffset = m_iSelectedItemIndex;
	}
	
	void SelectPreviousItem()
	{
		RBL_ShopCategory cat = m_aCategories[m_iSelectedCategoryIndex];
		m_iSelectedItemIndex--;
		if (m_iSelectedItemIndex < 0)
			m_iSelectedItemIndex = cat.m_aItems.Count() - 1;
		
		// Adjust scroll
		if (m_iSelectedItemIndex < m_iScrollOffset)
			m_iScrollOffset = m_iSelectedItemIndex;
		if (m_iSelectedItemIndex >= m_iScrollOffset + m_iMaxVisibleItems)
			m_iScrollOffset = m_iSelectedItemIndex - m_iMaxVisibleItems + 1;
	}
	
	// Purchasing
	void PurchaseSelectedItem()
	{
		RBL_ShopItem item = GetSelectedItem();
		if (!item)
			return;
		
		if (!CanAfford(item))
		{
			RBL_Notifications.InsufficientFunds();
			return;
		}
		
		// Execute purchase through economy manager
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
			return;
		
		bool success = econMgr.TryPurchase(item.m_sID, item.m_iMoneyCost, item.m_iHRCost);
		
		if (success)
		{
			RBL_Notifications.MoneySpent(item.m_iMoneyCost, item.m_sName);
			RefreshPlayerResources();
			PrintFormat("[RBL_Shop] Purchased: %1", item.m_sName);
		}
		else
		{
			RBL_Notifications.Show("Purchase failed");
		}
	}
	
	// Helpers
	protected RBL_ShopItem GetSelectedItem()
	{
		if (m_iSelectedCategoryIndex >= m_aCategories.Count())
			return null;
		
		RBL_ShopCategory cat = m_aCategories[m_iSelectedCategoryIndex];
		if (m_iSelectedItemIndex >= cat.m_aItems.Count())
			return null;
		
		return cat.m_aItems[m_iSelectedItemIndex];
	}
	
	protected bool CanAfford(RBL_ShopItem item)
	{
		return m_iPlayerMoney >= item.m_iMoneyCost && m_iPlayerHR >= item.m_iHRCost;
	}
	
	protected void RefreshPlayerResources()
	{
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
			return;
		
		m_iPlayerMoney = econMgr.GetMoney();
		m_iPlayerHR = econMgr.GetHR();
	}
	
	// Getters for testing
	int GetSelectedCategoryIndex() { return m_iSelectedCategoryIndex; }
	int GetSelectedItemIndex() { return m_iSelectedItemIndex; }
	int GetCategoryCount() { return m_aCategories.Count(); }
}

