// ============================================================================
// PROJECT REBELLION - Map Overlay Widget
// Shows clickable buttons when map is open
// ============================================================================

class RBL_MapOverlayWidget : RBL_BaseWidget
{
	protected bool m_bMapOpen;
	protected bool m_bWasMapOpen;
	protected bool m_bMapToggleState;
	
	protected Widget m_wRoot;
	protected TextWidget m_wShopLabel;
	protected TextWidget m_wSettingsLabel;
	protected TextWidget m_wShopHint;
	protected TextWidget m_wSettingsHint;
	protected bool m_bLayoutAttempted;
	protected bool m_bLayoutLoadFailed;
	
protected const string LAYOUT_PATH_GUID = "{685F68F508FD0CAE}UI/layouts/RBL_MapOverlay.layout";
protected const string LAYOUT_PATH_ALIAS = "$NewEnfusionProject:UI/layouts/RBL_MapOverlay.layout";
protected const string LAYOUT_PATH_RELATIVE = "UI/layouts/RBL_MapOverlay.layout";
	
	void RBL_MapOverlayWidget()
	{
		m_bMapOpen = false;
		m_bWasMapOpen = false;
		m_bMapToggleState = false;
		m_bLayoutAttempted = false;
		m_bLayoutLoadFailed = false;
		m_fUpdateInterval = 0.05;
	}
	
	override void OnUpdate()
	{
		// Track map open/close using the map toggle action
		InputManager input = GetGame().GetInputManager();
		if (input)
		{
			bool mapToggled = input.GetActionTriggered(RBL_InputActions.TOGGLE_MAP) || 
			                  input.GetActionTriggered(RBL_InputActions.FALLBACK_MAP);
			
			if (mapToggled)
				m_bMapToggleState = !m_bMapToggleState;
			
			m_bMapOpen = m_bMapToggleState;
		}
		
		// Fallback: if map toolbar exists and is visible, treat map as open
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace && !m_bMapOpen)
		{
			Widget mapToolbar = workspace.FindAnyWidget("MapToolbar");
			if (!mapToolbar)
				mapToolbar = workspace.FindAnyWidget("MapButtons");
			if (!mapToolbar)
				mapToolbar = workspace.FindAnyWidget("MapControls");
			
			if (mapToolbar && mapToolbar.IsVisible())
				m_bMapOpen = true;
		}
		
		if (m_bMapOpen != m_bWasMapOpen)
		{
			if (m_bMapOpen)
				OnMapOpened();
			else
				OnMapClosed();
			
			m_bWasMapOpen = m_bMapOpen;
		}
	}
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		EnsureLayout();
		if (!m_wRoot)
			return;
		
		bool visible = m_bMapOpen && m_bVisible;
		m_wRoot.SetVisible(visible);
	}
	
	override void Draw()
	{
		// Widget layout handles rendering
	}
	
	protected void EnsureLayout()
	{
		if (m_wRoot || m_bLayoutLoadFailed || m_bLayoutAttempted)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		
		m_bLayoutAttempted = true;
		
		Widget parent = FindMapParentWidget(workspace);
		m_wRoot = TryCreateLayout(workspace, parent, LAYOUT_PATH_GUID);
		if (!m_wRoot)
			m_wRoot = TryCreateLayout(workspace, parent, LAYOUT_PATH_ALIAS);
		if (!m_wRoot)
			m_wRoot = TryCreateLayout(workspace, parent, LAYOUT_PATH_RELATIVE);
		if (!m_wRoot)
		{
			PrintFormat("[RBL_MapOverlay] Failed to load layout");
			m_bLayoutLoadFailed = true;
			return;
		}
		
		PrintFormat("[RBL_MapOverlay] Layout created");
		
		m_wShopLabel = TextWidget.Cast(m_wRoot.FindAnyWidget("ShopLabel"));
		m_wSettingsLabel = TextWidget.Cast(m_wRoot.FindAnyWidget("SettingsLabel"));
		m_wShopHint = TextWidget.Cast(m_wRoot.FindAnyWidget("ShopHint"));
		m_wSettingsHint = TextWidget.Cast(m_wRoot.FindAnyWidget("SettingsHint"));
		
		UpdateButtonLabels();
		m_wRoot.SetVisible(false);
	}
	
	protected void UpdateButtonLabels()
	{
		string shopKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_SHOP);
		string settingsKey = RBL_InputConfig.GetKeyDisplay(RBL_InputActions.TOGGLE_SETTINGS);
		
		if (m_wShopLabel)
			m_wShopLabel.SetText("SHOP");
		if (m_wSettingsLabel)
			m_wSettingsLabel.SetText("SETTINGS");
		
		if (m_wShopHint)
			m_wShopHint.SetText(string.Format("Press [%1]", shopKey));
		if (m_wSettingsHint)
			m_wSettingsHint.SetText(string.Format("Press [%1]", settingsKey));
	}
	
	protected Widget TryCreateLayout(WorkspaceWidget workspace, Widget parent, string path)
	{
		if (!workspace || path.IsEmpty())
			return null;
		
		if (parent)
			return workspace.CreateWidgets(path, parent);
		
		return workspace.CreateWidgets(path);
	}
	
	protected Widget FindMapParentWidget(WorkspaceWidget workspace)
	{
		if (!workspace)
			return null;
		
		array<string> candidates = {};
		candidates.Insert("MapToolbar");
		candidates.Insert("MapButtons");
		candidates.Insert("MapControls");
		candidates.Insert("MapPanel");
		candidates.Insert("MapMenu");
		candidates.Insert("Map");
		candidates.Insert("SCR_MapMenu");
		candidates.Insert("MapWidget");
		
		foreach (string name : candidates)
		{
			Widget found = workspace.FindAnyWidget(name);
			if (found)
				return found;
		}
		
		return null;
	}
	
	protected void OnShopButtonClicked()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.ToggleShop();
	}
	
	protected void OnSettingsButtonClicked()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.ToggleSettings();
	}
	
	protected void OnMapOpened()
	{
		m_fTargetAlpha = 1.0;
		m_fAlpha = 1.0;
		UpdateButtonLabels();
	}
	
	protected void OnMapClosed()
	{
		m_fTargetAlpha = 0.0;
	}
}

class RBL_MapOverlayEventHandler : ScriptedWidgetEventHandler
{
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (!w)
			return false;
		
		Widget current = w;
		while (current)
		{
			string name = current.GetName();
			if (name == "ShopButton")
			{
				RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
				if (uiMgr)
					uiMgr.ToggleShop();
				return true;
			}
			
			if (name == "SettingsButton")
			{
				RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
				if (uiMgr)
					uiMgr.ToggleSettings();
				return true;
			}
			
			current = current.GetParent();
		}
		
		return false;
	}
}
