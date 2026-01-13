// ============================================================================
// PROJECT REBELLION - Shop Menu Widget
// Full-screen shop UI - uses RBL_ShopManager for item data
// ============================================================================

// Main shop menu widget
class RBL_ShopMenuWidgetImpl : RBL_PanelWidget
{
	protected bool m_bMenuVisible;
	protected int m_iSelectedCategoryIndex;
	protected int m_iSelectedItemIndex;
	protected int m_iScrollOffset;
	protected int m_iMaxVisibleItems;
	
	// Cached data from ShopManager
	protected ref array<ref RBL_ShopCategory> m_aCategories;
	
	// Player resources
	protected int m_iPlayerMoney;
	protected int m_iPlayerHR;
	protected int m_iWarLevel;
	
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
		m_iWarLevel = 1;
		
		m_fOpenAnimation = 0;
		m_bClosing = false;
		
		m_iBackgroundColor = RBL_UIColors.COLOR_BG_DARK;
		m_iBorderColor = RBL_UIColors.COLOR_BORDER_LIGHT;
		
		// Get categories from ShopManager
		RefreshCategoriesFromManager();
	}
	
	protected void RefreshCategoriesFromManager()
	{
		RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
		if (shopMgr)
		{
			m_aCategories = shopMgr.GetCategories();
		}
		else
		{
			m_aCategories = new array<ref RBL_ShopCategory>();
		}
	}
	
	void Toggle()
	{
		if (m_bMenuVisible)
			Close();
		else
			Open();
	}
	
	void Open()
	{
		m_bMenuVisible = true;
		m_bClosing = false;
		m_fOpenAnimation = 0;
		
		// Refresh data from manager
		RefreshCategoriesFromManager();
		RefreshPlayerResources();
		
		// Notify shop manager
		RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
		if (shopMgr)
			shopMgr.OpenMenu();
		
		// Get close key from input system
		string closeKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.CLOSE_MENU);
		RBL_Notifications.Show(string.Format("Press [%1] to close shop", closeKey));
	}
	
	void Close()
	{
		m_bClosing = true;
		
		// Notify shop manager
		RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
		if (shopMgr)
			shopMgr.CloseMenu();
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
		if (input.GetActionTriggered("RBL_MenuLeft") || input.GetActionTriggered("MenuLeft"))
			SelectPreviousCategory();
		if (input.GetActionTriggered("RBL_MenuRight") || input.GetActionTriggered("MenuRight"))
			SelectNextCategory();
		
		// Item navigation (W/S or arrows)
		if (input.GetActionTriggered("RBL_MenuUp") || input.GetActionTriggered("MenuUp"))
			SelectPreviousItem();
		if (input.GetActionTriggered("RBL_MenuDown") || input.GetActionTriggered("MenuDown"))
			SelectNextItem();
		
		// Purchase (Enter or Space)
		if (input.GetActionTriggered("RBL_MenuSelect") || input.GetActionTriggered("MenuSelect"))
			PurchaseSelectedItem();
		
		// Close (Escape)
		if (input.GetActionTriggered("RBL_MenuBack") || input.GetActionTriggered("MenuBack"))
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
		DbgUI.Begin("Shop_Money", x + width - 200, y + 16);
		DbgUI.Text(RBL_UIStrings.FormatMoney(m_iPlayerMoney));
		DbgUI.End();
		
		DbgUI.Begin("Shop_HR", x + width - 80, y + 16);
		DbgUI.Text(RBL_UIStrings.FormatHR(m_iPlayerHR));
		DbgUI.End();
		
		// War level indicator
		DbgUI.Begin("Shop_WarLevel", x + width - 300, y + 16);
		DbgUI.Text(string.Format("WL:%1", m_iWarLevel));
		DbgUI.End();
		
		// Separator line
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(x + 16, y + 50, width - 32, 1, lineColor);
	}
	
	protected void DrawCategories(float x, float y, float width)
	{
		if (!m_aCategories || m_aCategories.Count() == 0)
			return;
		
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
			DbgUI.Text(cat.GetDisplayName());
			DbgUI.End();
		}
	}
	
	protected void DrawItems(float x, float y, float width, float height)
	{
		if (!m_aCategories || m_iSelectedCategoryIndex >= m_aCategories.Count())
			return;
		
		RBL_ShopCategory category = m_aCategories[m_iSelectedCategoryIndex];
		array<ref RBL_ShopItem> items = category.GetItems();
		
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
		for (int i = m_iScrollOffset; i < items.Count() && i < m_iScrollOffset + m_iMaxVisibleItems; i++)
		{
			RBL_ShopItem item = items[i];
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
		if (m_iScrollOffset + m_iMaxVisibleItems < items.Count())
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
		bool canAfford = item.CanPurchase(m_iPlayerMoney, m_iPlayerHR, m_iWarLevel);
		
		// Row background
		int rowColor = RBL_UIColors.COLOR_BG_LIGHT;
		if (selected)
			rowColor = RBL_UIColors.COLOR_BG_HIGHLIGHT;
		if (!canAfford)
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
		if (!canAfford)
			nameColor = RBL_UIColors.COLOR_TEXT_MUTED;
		nameColor = ApplyAlpha(nameColor, m_fAlpha);
		
		DbgUI.Begin("Shop_Item_" + item.GetID(), x + 12, y + 8);
		DbgUI.Text(item.GetDisplayName());
		DbgUI.End();
		
		// War level indicator if required
		if (item.GetRequiredWarLevel() > 1)
		{
			int wlColor = RBL_UIColors.COLOR_TEXT_MUTED;
			if (m_iWarLevel < item.GetRequiredWarLevel())
				wlColor = RBL_UIColors.COLOR_ACCENT_RED;
			wlColor = ApplyAlpha(wlColor, m_fAlpha);
			
			DbgUI.Begin("Shop_WL_" + item.GetID(), x + width - 180, y + 8);
			DbgUI.Text(string.Format("WL%1+", item.GetRequiredWarLevel()));
			DbgUI.End();
		}
		
		// Price (right aligned)
		string priceText = "";
		int priceColor = RBL_UIColors.COLOR_TEXT_SECONDARY;
		
		if (item.GetPrice() > 0)
		{
			priceText = RBL_UIStrings.FormatMoney(item.GetPrice());
			if (m_iPlayerMoney < item.GetPrice())
				priceColor = RBL_UIColors.COLOR_ACCENT_RED;
		}
		if (item.GetHRCost() > 0)
		{
			if (!priceText.IsEmpty())
				priceText = priceText + " + ";
			priceText = priceText + item.GetHRCost().ToString() + " HR";
			if (m_iPlayerHR < item.GetHRCost())
				priceColor = RBL_UIColors.COLOR_ACCENT_RED;
		}
		
		priceColor = ApplyAlpha(priceColor, m_fAlpha);
		
		DbgUI.Begin("Shop_Price_" + item.GetID(), x + width - 100, y + 8);
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
		DbgUI.Text(selectedItem.GetDisplayName());
		DbgUI.End();
		
		contentY += 32;
		
		// Description
		int descColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
		DbgUI.Begin("Shop_Detail_Desc", contentX, contentY);
		DbgUI.Text(selectedItem.GetDescription());
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
		
		if (selectedItem.GetPrice() > 0)
		{
			int moneyColor = RBL_UIColors.COLOR_ACCENT_AMBER;
			if (m_iPlayerMoney < selectedItem.GetPrice())
				moneyColor = RBL_UIColors.COLOR_ACCENT_RED;
			moneyColor = ApplyAlpha(moneyColor, m_fAlpha);
			
			DbgUI.Begin("Shop_Detail_Money", contentX, contentY);
			DbgUI.Text("  " + RBL_UIStrings.FormatMoney(selectedItem.GetPrice()));
			DbgUI.End();
			contentY += 20;
		}
		
		if (selectedItem.GetHRCost() > 0)
		{
			int hrColor = RBL_UIColors.COLOR_ACCENT_BLUE;
			if (m_iPlayerHR < selectedItem.GetHRCost())
				hrColor = RBL_UIColors.COLOR_ACCENT_RED;
			hrColor = ApplyAlpha(hrColor, m_fAlpha);
			
			DbgUI.Begin("Shop_Detail_HR", contentX, contentY);
			DbgUI.Text("  " + selectedItem.GetHRCost().ToString() + " Human Resources");
			DbgUI.End();
			contentY += 20;
		}
		
		// War level requirement
		if (selectedItem.GetRequiredWarLevel() > 1)
		{
			int wlColor = RBL_UIColors.COLOR_TEXT_MUTED;
			if (m_iWarLevel < selectedItem.GetRequiredWarLevel())
				wlColor = RBL_UIColors.COLOR_ACCENT_RED;
			wlColor = ApplyAlpha(wlColor, m_fAlpha);
			
			contentY += 10;
			DbgUI.Begin("Shop_Detail_WL", contentX, contentY);
			DbgUI.Text(string.Format("  Requires War Level %1", selectedItem.GetRequiredWarLevel()));
			DbgUI.End();
			contentY += 20;
		}
		
		// Purchase hint
		contentY = y + height - 40;
		bool canPurchase = selectedItem.CanPurchase(m_iPlayerMoney, m_iPlayerHR, m_iWarLevel);
		string hintText = "[ENTER] Purchase";
		int hintColor = RBL_UIColors.COLOR_ACCENT_GREEN;
		
		if (!canPurchase)
		{
			hintText = selectedItem.GetUnavailableReason(m_iPlayerMoney, m_iPlayerHR, m_iWarLevel);
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
		if (!m_aCategories || m_aCategories.Count() == 0)
			return;
		
		m_iSelectedCategoryIndex++;
		if (m_iSelectedCategoryIndex >= m_aCategories.Count())
			m_iSelectedCategoryIndex = 0;
		m_iSelectedItemIndex = 0;
		m_iScrollOffset = 0;
	}
	
	void SelectPreviousCategory()
	{
		if (!m_aCategories || m_aCategories.Count() == 0)
			return;
		
		m_iSelectedCategoryIndex--;
		if (m_iSelectedCategoryIndex < 0)
			m_iSelectedCategoryIndex = m_aCategories.Count() - 1;
		m_iSelectedItemIndex = 0;
		m_iScrollOffset = 0;
	}
	
	void SelectNextItem()
	{
		if (!m_aCategories || m_iSelectedCategoryIndex >= m_aCategories.Count())
			return;
		
		RBL_ShopCategory cat = m_aCategories[m_iSelectedCategoryIndex];
		array<ref RBL_ShopItem> items = cat.GetItems();
		
		m_iSelectedItemIndex++;
		if (m_iSelectedItemIndex >= items.Count())
			m_iSelectedItemIndex = 0;
		
		// Adjust scroll
		if (m_iSelectedItemIndex >= m_iScrollOffset + m_iMaxVisibleItems)
			m_iScrollOffset = m_iSelectedItemIndex - m_iMaxVisibleItems + 1;
		if (m_iSelectedItemIndex < m_iScrollOffset)
			m_iScrollOffset = m_iSelectedItemIndex;
	}
	
	void SelectPreviousItem()
	{
		if (!m_aCategories || m_iSelectedCategoryIndex >= m_aCategories.Count())
			return;
		
		RBL_ShopCategory cat = m_aCategories[m_iSelectedCategoryIndex];
		array<ref RBL_ShopItem> items = cat.GetItems();
		
		m_iSelectedItemIndex--;
		if (m_iSelectedItemIndex < 0)
			m_iSelectedItemIndex = items.Count() - 1;
		
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
		
		if (!item.CanPurchase(m_iPlayerMoney, m_iPlayerHR, m_iWarLevel))
		{
			string reason = item.GetUnavailableReason(m_iPlayerMoney, m_iPlayerHR, m_iWarLevel);
			RBL_Notifications.Show(reason);
			return;
		}
		
		// Execute purchase through ShopManager
		RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
		if (!shopMgr)
		{
			RBL_Notifications.Show("Shop system unavailable");
			return;
		}
		
		bool success = shopMgr.PurchaseItem(item.GetID());
		
		if (success)
		{
			RefreshPlayerResources();
			PrintFormat("[RBL_ShopWidget] Purchased: %1", item.GetDisplayName());
		}
	}
	
	// Helpers
	protected RBL_ShopItem GetSelectedItem()
	{
		if (!m_aCategories || m_iSelectedCategoryIndex >= m_aCategories.Count())
			return null;
		
		RBL_ShopCategory cat = m_aCategories[m_iSelectedCategoryIndex];
		return cat.GetItemByIndex(m_iSelectedItemIndex);
	}
	
	protected void RefreshPlayerResources()
	{
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			m_iPlayerMoney = econMgr.GetMoney();
			m_iPlayerHR = econMgr.GetHR();
		}
		
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (campMgr)
		{
			m_iWarLevel = campMgr.GetWarLevel();
		}
	}
	
	// Getters for testing
	int GetSelectedCategoryIndex() { return m_iSelectedCategoryIndex; }
	int GetSelectedItemIndex() { return m_iSelectedItemIndex; }
	int GetCategoryCount() { return m_aCategories ? m_aCategories.Count() : 0; }
}
