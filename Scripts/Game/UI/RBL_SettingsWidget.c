// ============================================================================
// PROJECT REBELLION - Settings Menu Widget
// Full-screen settings interface with categories, navigation, and controls
// ============================================================================

class RBL_SettingsMenuWidget : RBL_PanelWidget
{
	protected bool m_bMenuVisible;
	protected bool m_bClosing;
	protected float m_fOpenAnimation;
	
	// Categories
	protected ref array<string> m_aCategoryNames;
	protected ref array<ERBLSettingsCategory> m_aCategories;
	protected int m_iSelectedCategoryIndex;
	
	// Items in current category
	protected ref array<ref RBL_SettingItem> m_aCurrentItems;
	protected int m_iSelectedItemIndex;
	protected int m_iScrollOffset;
	protected int m_iMaxVisibleItems;
	
	// Editing state
	protected bool m_bEditingValue;
	protected float m_fEditValue;
	protected int m_iEditIntValue;
	
	// Confirmation dialog
	protected bool m_bShowingConfirmation;
	protected string m_sConfirmationAction;
	
	// ========================================================================
	// CONSTRUCTOR
	// ========================================================================
	
	void RBL_SettingsMenuWidget()
	{
		// Centered position
		vector pos = RBL_UILayout.GetCenteredPosition(RBL_UISizes.SHOP_WIDTH, RBL_UISizes.SHOP_HEIGHT);
		m_fPosX = pos[0];
		m_fPosY = pos[1];
		m_fWidth = RBL_UISizes.SHOP_WIDTH;
		m_fHeight = RBL_UISizes.SHOP_HEIGHT;
		
		m_bMenuVisible = false;
		m_bClosing = false;
		m_fOpenAnimation = 0;
		
		m_iBackgroundColor = RBL_UIColors.COLOR_BG_DARK;
		m_iBorderColor = RBL_UIColors.COLOR_BORDER_LIGHT;
		
		// Setup categories
		m_aCategoryNames = new array<string>();
		m_aCategories = new array<ERBLSettingsCategory>();
		
		m_aCategoryNames.Insert("GAMEPLAY");
		m_aCategories.Insert(ERBLSettingsCategory.GAMEPLAY);
		
		m_aCategoryNames.Insert("DISPLAY");
		m_aCategories.Insert(ERBLSettingsCategory.DISPLAY);
		
		m_aCategoryNames.Insert("AUDIO");
		m_aCategories.Insert(ERBLSettingsCategory.AUDIO);
		
		m_aCategoryNames.Insert("CONTROLS");
		m_aCategories.Insert(ERBLSettingsCategory.CONTROLS);
		
		m_iSelectedCategoryIndex = 0;
		m_iSelectedItemIndex = 0;
		m_iScrollOffset = 0;
		m_iMaxVisibleItems = 10;
		
		m_aCurrentItems = new array<ref RBL_SettingItem>();
		
		m_bEditingValue = false;
		m_bShowingConfirmation = false;
		
		RefreshCurrentCategory();
	}
	
	// ========================================================================
	// OPEN / CLOSE
	// ========================================================================
	
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
		
		RefreshCurrentCategory();
		RBL_Notifications.Show("Press [ESC] to close settings");
	}
	
	void Close()
	{
		// Check for unsaved changes
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		if (mgr && mgr.HasUnsavedChanges())
		{
			ShowConfirmation("discard");
			return;
		}
		
		m_bClosing = true;
	}
	
	void ForceClose()
	{
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		if (mgr)
			mgr.DiscardChanges();
		
		m_bClosing = true;
	}
	
	override bool IsVisible()
	{
		return m_bMenuVisible;
	}
	
	// ========================================================================
	// UPDATE
	// ========================================================================
	
	override void Update(float timeSlice)
	{
		if (!m_bMenuVisible)
			return;
		
		// Animation
		if (m_bClosing)
		{
			m_fOpenAnimation -= timeSlice * 4;
			if (m_fOpenAnimation <= 0)
			{
				m_fOpenAnimation = 0;
				m_bMenuVisible = false;
				m_bClosing = false;
				m_bShowingConfirmation = false;
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
		if (!m_bClosing)
			HandleInput();
	}
	
	// ========================================================================
	// INPUT HANDLING
	// ========================================================================
	
	protected void HandleInput()
	{
		InputManager input = GetGame().GetInputManager();
		if (!input)
			return;
		
		if (m_bShowingConfirmation)
		{
			HandleConfirmationInput(input);
			return;
		}
		
		if (m_bEditingValue)
		{
			HandleEditInput(input);
			return;
		}
		
		// Category navigation (Q/E)
		if (input.GetActionTriggered("MenuLeft"))
			SelectPreviousCategory();
		if (input.GetActionTriggered("MenuRight"))
			SelectNextCategory();
		
		// Item navigation (W/S)
		if (input.GetActionTriggered("MenuUp"))
			SelectPreviousItem();
		if (input.GetActionTriggered("MenuDown"))
			SelectNextItem();
		
		// Toggle/Edit (Enter)
		if (input.GetActionTriggered("MenuSelect"))
			InteractWithSelected();
		
		// Value adjust (A/D) for sliders
		if (input.GetActionTriggered("CharacterLeft"))
			AdjustSelectedValue(-1);
		if (input.GetActionTriggered("CharacterRight"))
			AdjustSelectedValue(1);
		
		// Apply settings (P)
		if (input.GetActionValue("InventoryOpen") > 0.5)
			ApplySettings();
		
		// Reset defaults (R)
		if (input.GetActionTriggered("Reload"))
			ShowConfirmation("reset");
		
		// Close (Escape)
		if (input.GetActionTriggered("MenuBack"))
			Close();
	}
	
	protected void HandleEditInput(InputManager input)
	{
		// Left/Right to adjust
		if (input.GetActionTriggered("CharacterLeft") || input.GetActionTriggered("MenuLeft"))
		{
			m_fEditValue -= GetSelectedItem().m_fStepValue;
			m_fEditValue = Math.Max(m_fEditValue, GetSelectedItem().m_fMinValue);
		}
		if (input.GetActionTriggered("CharacterRight") || input.GetActionTriggered("MenuRight"))
		{
			m_fEditValue += GetSelectedItem().m_fStepValue;
			m_fEditValue = Math.Min(m_fEditValue, GetSelectedItem().m_fMaxValue);
		}
		
		// Confirm edit
		if (input.GetActionTriggered("MenuSelect"))
		{
			RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
			if (mgr)
			{
				mgr.SetSettingFloat(GetSelectedItem().m_sID, m_fEditValue);
			}
			m_bEditingValue = false;
		}
		
		// Cancel edit
		if (input.GetActionTriggered("MenuBack"))
		{
			m_bEditingValue = false;
		}
	}
	
	protected void HandleConfirmationInput(InputManager input)
	{
		// Yes (Enter)
		if (input.GetActionTriggered("MenuSelect"))
		{
			if (m_sConfirmationAction == "discard")
			{
				RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
				if (mgr)
					mgr.DiscardChanges();
				m_bShowingConfirmation = false;
				m_bClosing = true;
			}
			else if (m_sConfirmationAction == "reset")
			{
				RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
				if (mgr)
					mgr.ResetToDefaults();
				m_bShowingConfirmation = false;
				RefreshCurrentCategory();
			}
			else if (m_sConfirmationAction == "apply")
			{
				RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
				if (mgr)
					mgr.ApplySettings();
				m_bShowingConfirmation = false;
			}
		}
		
		// No (Escape)
		if (input.GetActionTriggered("MenuBack"))
		{
			m_bShowingConfirmation = false;
		}
	}
	
	// ========================================================================
	// NAVIGATION
	// ========================================================================
	
	void SelectNextCategory()
	{
		m_iSelectedCategoryIndex++;
		if (m_iSelectedCategoryIndex >= m_aCategories.Count())
			m_iSelectedCategoryIndex = 0;
		
		m_iSelectedItemIndex = 0;
		m_iScrollOffset = 0;
		RefreshCurrentCategory();
	}
	
	void SelectPreviousCategory()
	{
		m_iSelectedCategoryIndex--;
		if (m_iSelectedCategoryIndex < 0)
			m_iSelectedCategoryIndex = m_aCategories.Count() - 1;
		
		m_iSelectedItemIndex = 0;
		m_iScrollOffset = 0;
		RefreshCurrentCategory();
	}
	
	void SelectNextItem()
	{
		if (m_aCurrentItems.Count() == 0)
			return;
		
		m_iSelectedItemIndex++;
		if (m_iSelectedItemIndex >= m_aCurrentItems.Count())
			m_iSelectedItemIndex = 0;
		
		// Adjust scroll
		if (m_iSelectedItemIndex >= m_iScrollOffset + m_iMaxVisibleItems)
			m_iScrollOffset = m_iSelectedItemIndex - m_iMaxVisibleItems + 1;
		if (m_iSelectedItemIndex < m_iScrollOffset)
			m_iScrollOffset = m_iSelectedItemIndex;
	}
	
	void SelectPreviousItem()
	{
		if (m_aCurrentItems.Count() == 0)
			return;
		
		m_iSelectedItemIndex--;
		if (m_iSelectedItemIndex < 0)
			m_iSelectedItemIndex = m_aCurrentItems.Count() - 1;
		
		// Adjust scroll
		if (m_iSelectedItemIndex < m_iScrollOffset)
			m_iScrollOffset = m_iSelectedItemIndex;
		if (m_iSelectedItemIndex >= m_iScrollOffset + m_iMaxVisibleItems)
			m_iScrollOffset = m_iSelectedItemIndex - m_iMaxVisibleItems + 1;
	}
	
	void RefreshCurrentCategory()
	{
		m_aCurrentItems.Clear();
		
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		if (!mgr)
			return;
		
		ERBLSettingsCategory cat = m_aCategories[m_iSelectedCategoryIndex];
		m_aCurrentItems = mgr.GetSettingsByCategory(cat);
	}
	
	// ========================================================================
	// VALUE INTERACTION
	// ========================================================================
	
	void InteractWithSelected()
	{
		RBL_SettingItem item = GetSelectedItem();
		if (!item)
			return;
		
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		if (!mgr)
			return;
		
		switch (item.m_eType)
		{
			case ERBLSettingType.TOGGLE:
				// Toggle boolean
				bool current = mgr.GetSettingBool(item.m_sID);
				mgr.SetSettingBool(item.m_sID, !current);
				break;
			
			case ERBLSettingType.SLIDER:
				// Enter edit mode
				m_bEditingValue = true;
				m_fEditValue = mgr.GetSettingFloat(item.m_sID);
				break;
			
			case ERBLSettingType.DROPDOWN:
				// Cycle through options
				int currentInt = mgr.GetSettingInt(item.m_sID);
				currentInt++;
				if (currentInt >= item.m_aOptions.Count())
					currentInt = 0;
				mgr.SetSettingInt(item.m_sID, currentInt);
				break;
		}
	}
	
	void AdjustSelectedValue(int direction)
	{
		RBL_SettingItem item = GetSelectedItem();
		if (!item)
			return;
		
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		if (!mgr)
			return;
		
		if (item.m_eType == ERBLSettingType.SLIDER)
		{
			float current = mgr.GetSettingFloat(item.m_sID);
			float newVal = current + direction * item.m_fStepValue;
			newVal = Math.Clamp(newVal, item.m_fMinValue, item.m_fMaxValue);
			mgr.SetSettingFloat(item.m_sID, newVal);
		}
		else if (item.m_eType == ERBLSettingType.DROPDOWN)
		{
			int currentInt = mgr.GetSettingInt(item.m_sID);
			currentInt += direction;
			if (currentInt < 0)
				currentInt = item.m_aOptions.Count() - 1;
			if (currentInt >= item.m_aOptions.Count())
				currentInt = 0;
			mgr.SetSettingInt(item.m_sID, currentInt);
		}
	}
	
	void ApplySettings()
	{
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		if (mgr)
		{
			mgr.ApplySettings();
			RBL_Notifications.Show("Settings applied");
		}
	}
	
	void ShowConfirmation(string action)
	{
		m_bShowingConfirmation = true;
		m_sConfirmationAction = action;
	}
	
	RBL_SettingItem GetSelectedItem()
	{
		if (m_iSelectedItemIndex < 0 || m_iSelectedItemIndex >= m_aCurrentItems.Count())
			return null;
		return m_aCurrentItems[m_iSelectedItemIndex];
	}
	
	// ========================================================================
	// DRAWING
	// ========================================================================
	
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
		
		// Main panel
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
		
		// Confirmation overlay
		if (m_bShowingConfirmation)
			DrawConfirmation(drawX, drawY, scaledWidth, scaledHeight);
	}
	
	protected void DrawHeader(float x, float y, float width)
	{
		// Title
		int titleColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("Settings_Title", x + 16, y + 16);
		DbgUI.Text("SETTINGS");
		DbgUI.End();
		
		// Unsaved indicator
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		if (mgr && mgr.HasUnsavedChanges())
		{
			int unsavedColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_AMBER, m_fAlpha);
			DbgUI.Begin("Settings_Unsaved", x + width - 150, y + 16);
			DbgUI.Text("* Unsaved Changes");
			DbgUI.End();
		}
		
		// Separator
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(x + 16, y + 50, width - 32, 1, lineColor);
	}
	
	protected void DrawCategories(float x, float y, float width)
	{
		float tabWidth = (width - 32) / m_aCategoryNames.Count();
		
		for (int i = 0; i < m_aCategoryNames.Count(); i++)
		{
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
			
			DbgUI.Begin("Settings_Tab_" + i.ToString(), tabX + 8, y + 6);
			DbgUI.Text(m_aCategoryNames[i]);
			DbgUI.End();
		}
	}
	
	protected void DrawItems(float x, float y, float width, float height)
	{
		if (m_aCurrentItems.Count() == 0)
		{
			int emptyColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
			DbgUI.Begin("Settings_Empty", x + width / 2 - 50, y + height / 2);
			DbgUI.Text("No settings in this category");
			DbgUI.End();
			return;
		}
		
		float itemHeight = 36;
		float listX = x + 16;
		float listWidth = width * 0.5 - 24;
		float detailX = x + width * 0.5;
		float detailWidth = width * 0.5 - 16;
		
		// Item list background
		int listBgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_MEDIUM, m_fAlpha * 0.5);
		DrawRect(listX, y, listWidth, height, listBgColor);
		
		// Draw items
		float currentY = y + 8;
		for (int i = m_iScrollOffset; i < m_aCurrentItems.Count() && i < m_iScrollOffset + m_iMaxVisibleItems; i++)
		{
			RBL_SettingItem item = m_aCurrentItems[i];
			bool selected = (i == m_iSelectedItemIndex);
			
			DrawSettingRow(item, listX, currentY, listWidth, itemHeight, selected);
			currentY += itemHeight + 4;
		}
		
		// Scroll indicators
		if (m_iScrollOffset > 0)
		{
			DbgUI.Begin("Settings_ScrollUp", listX + listWidth / 2 - 10, y - 16);
			DbgUI.Text("▲");
			DbgUI.End();
		}
		if (m_iScrollOffset + m_iMaxVisibleItems < m_aCurrentItems.Count())
		{
			DbgUI.Begin("Settings_ScrollDown", listX + listWidth / 2 - 10, y + height);
			DbgUI.Text("▼");
			DbgUI.End();
		}
		
		// Detail panel
		DrawItemDetail(detailX, y, detailWidth, height);
	}
	
	protected void DrawSettingRow(RBL_SettingItem item, float x, float y, float width, float height, bool selected)
	{
		// Background
		int rowColor = RBL_UIColors.COLOR_BG_LIGHT;
		if (selected)
			rowColor = RBL_UIColors.COLOR_BG_HIGHLIGHT;
		rowColor = ApplyAlpha(rowColor, m_fAlpha);
		
		DrawRect(x, y, width, height, rowColor);
		
		// Selection indicator
		if (selected)
		{
			int indicatorColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_GREEN, m_fAlpha);
			DrawRect(x, y, 3, height, indicatorColor);
		}
		
		// Setting name
		int nameColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("Settings_Item_" + item.m_sID, x + 12, y + 10);
		DbgUI.Text(item.m_sName);
		DbgUI.End();
		
		// Setting value
		string valueStr = GetSettingValueString(item);
		int valueColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
		DbgUI.Begin("Settings_Value_" + item.m_sID, x + width - 100, y + 10);
		DbgUI.Text(valueStr);
		DbgUI.End();
	}
	
	protected void DrawItemDetail(float x, float y, float width, float height)
	{
		RBL_SettingItem item = GetSelectedItem();
		if (!item)
			return;
		
		// Background
		int detailBg = ApplyAlpha(RBL_UIColors.COLOR_BG_MEDIUM, m_fAlpha * 0.5);
		DrawRect(x, y, width, height, detailBg);
		
		float contentX = x + 12;
		float contentY = y + 12;
		
		// Item name
		int nameColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("Settings_Detail_Name", contentX, contentY);
		DbgUI.Text(item.m_sName);
		DbgUI.End();
		
		contentY += 32;
		
		// Description
		int descColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
		DbgUI.Begin("Settings_Detail_Desc", contentX, contentY);
		DbgUI.Text(item.m_sDescription);
		DbgUI.End();
		
		contentY += 48;
		
		// Separator
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(contentX, contentY, width - 24, 1, lineColor);
		contentY += 16;
		
		// Current value
		DbgUI.Begin("Settings_Detail_ValueLabel", contentX, contentY);
		DbgUI.Text("Current Value:");
		DbgUI.End();
		contentY += 20;
		
		string valueStr = GetSettingValueString(item);
		int valueColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_GREEN, m_fAlpha);
		DbgUI.Begin("Settings_Detail_Value", contentX + 20, contentY);
		DbgUI.Text(valueStr);
		DbgUI.End();
		contentY += 32;
		
		// Type-specific controls hint
		DrawControlsHint(item, contentX, contentY, width - 24);
	}
	
	protected void DrawControlsHint(RBL_SettingItem item, float x, float y, float width)
	{
		string hintText = "";
		
		switch (item.m_eType)
		{
			case ERBLSettingType.TOGGLE:
				hintText = "[ENTER] Toggle On/Off";
				break;
			case ERBLSettingType.SLIDER:
				hintText = "[A/D] Adjust   [ENTER] Edit";
				break;
			case ERBLSettingType.DROPDOWN:
				hintText = "[A/D] Cycle   [ENTER] Select";
				break;
		}
		
		if (m_bEditingValue)
		{
			hintText = "[A/D] Adjust   [ENTER] Confirm   [ESC] Cancel";
		}
		
		int hintColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
		DbgUI.Begin("Settings_Controls_Hint", x, y);
		DbgUI.Text(hintText);
		DbgUI.End();
	}
	
	protected void DrawFooter(float x, float y, float width)
	{
		// Separator
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(x + 16, y, width - 32, 1, lineColor);
		
		// Controls
		int hintColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
		DbgUI.Begin("Settings_Footer", x + 16, y + 16);
		DbgUI.Text("[Q/E] Category   [W/S] Navigate   [P] Apply   [R] Reset   [ESC] Close");
		DbgUI.End();
	}
	
	protected void DrawConfirmation(float x, float y, float width, float height)
	{
		// Overlay
		int overlayColor = ApplyAlpha(ARGB(200, 0, 0, 0), m_fAlpha);
		DrawRect(x, y, width, height, overlayColor);
		
		// Dialog box
		float dialogW = 350;
		float dialogH = 150;
		float dialogX = x + (width - dialogW) / 2;
		float dialogY = y + (height - dialogH) / 2;
		
		int dialogBg = ApplyAlpha(RBL_UIColors.COLOR_BG_DARK, m_fAlpha);
		DrawRect(dialogX, dialogY, dialogW, dialogH, dialogBg);
		
		int borderColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_AMBER, m_fAlpha);
		DrawRectOutline(dialogX, dialogY, dialogW, dialogH, borderColor, 2);
		
		// Title
		string title = "";
		string message = "";
		
		if (m_sConfirmationAction == "discard")
		{
			title = "Unsaved Changes";
			message = "Discard unsaved changes?";
		}
		else if (m_sConfirmationAction == "reset")
		{
			title = "Reset Settings";
			message = "Reset all settings to defaults?";
		}
		else if (m_sConfirmationAction == "apply")
		{
			title = "Apply Settings";
			message = "Apply all changes?";
		}
		
		int titleColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_AMBER, m_fAlpha);
		DbgUI.Begin("Settings_Confirm_Title", dialogX + 16, dialogY + 16);
		DbgUI.Text(title);
		DbgUI.End();
		
		int msgColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("Settings_Confirm_Msg", dialogX + 16, dialogY + 50);
		DbgUI.Text(message);
		DbgUI.End();
		
		int hintColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
		DbgUI.Begin("Settings_Confirm_Hint", dialogX + 16, dialogY + dialogH - 40);
		DbgUI.Text("[ENTER] Yes   [ESC] No");
		DbgUI.End();
	}
	
	// ========================================================================
	// VALUE FORMATTING
	// ========================================================================
	
	protected string GetSettingValueString(RBL_SettingItem item)
	{
		RBL_SettingsManager mgr = RBL_SettingsManager.GetInstance();
		if (!mgr)
			return "N/A";
		
		switch (item.m_eType)
		{
			case ERBLSettingType.TOGGLE:
				return mgr.GetSettingBool(item.m_sID) ? "ON" : "OFF";
			
			case ERBLSettingType.SLIDER:
				float val = mgr.GetSettingFloat(item.m_sID);
				// Format based on setting
				if (item.m_sID.Contains("volume") || item.m_sID.Contains("opacity"))
					return (val * 100).ToString() + "%";
				else if (item.m_sID.Contains("interval"))
					return val.ToString() + " min";
				else if (item.m_sID.Contains("duration"))
					return val.ToString() + " sec";
				else if (item.m_sID.Contains("scale"))
					return (val * 100).ToString() + "%";
				else
					return val.ToString();
			
			case ERBLSettingType.DROPDOWN:
				int idx = mgr.GetSettingInt(item.m_sID);
				if (idx >= 0 && idx < item.m_aOptions.Count())
					return item.m_aOptions[idx];
				return "Unknown";
			
			case ERBLSettingType.INTEGER:
				return mgr.GetSettingInt(item.m_sID).ToString();
		}
		
		return "N/A";
	}
	
	// ========================================================================
	// QUERIES
	// ========================================================================
	
	int GetSelectedCategoryIndex() { return m_iSelectedCategoryIndex; }
	int GetSelectedItemIndex() { return m_iSelectedItemIndex; }
	int GetCategoryCount() { return m_aCategories.Count(); }
	bool IsEditingValue() { return m_bEditingValue; }
	bool IsShowingConfirmation() { return m_bShowingConfirmation; }
}


