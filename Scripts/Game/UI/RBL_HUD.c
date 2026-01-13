// ============================================================================
// PROJECT REBELLION - HUD System (Legacy Compatibility Layer)
// Delegates to RBL_UIManager for actual display
// Use RBL_UIManager directly for new code
// ============================================================================

class RBL_HUDManager
{
	protected static ref RBL_HUDManager s_Instance;
	protected bool m_bVisible;
	
	static RBL_HUDManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_HUDManager();
		return s_Instance;
	}
	
	void RBL_HUDManager()
	{
		m_bVisible = true;
	}
	
	void Update(float timeSlice)
	{
		// Delegate to new UI system
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.Update(timeSlice);
		
		// Also update legacy ScreenHUD if enabled
		RBL_ScreenHUD screenHUD = RBL_ScreenHUD.GetInstance();
		if (screenHUD && screenHUD.IsEnabled())
			screenHUD.Update(timeSlice);
	}
	
	void Draw()
	{
		// Delegate to new UI system
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.Draw();
	}
	
	void ToggleVisibility()
	{
		m_bVisible = !m_bVisible;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.SetVisible(m_bVisible);
		
		RBL_ScreenHUD screenHUD = RBL_ScreenHUD.GetInstance();
		if (screenHUD)
			screenHUD.SetEnabled(m_bVisible);
	}
	
	void SetVisible(bool visible)
	{
		m_bVisible = visible;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.SetVisible(visible);
		
		RBL_ScreenHUD screenHUD = RBL_ScreenHUD.GetInstance();
		if (screenHUD)
			screenHUD.SetEnabled(visible);
	}
	
	bool IsVisible() { return m_bVisible; }
	
	static void Show()
	{
		RBL_HUDManager inst = GetInstance();
		if (inst)
			inst.SetVisible(true);
	}
	
	static void Hide()
	{
		RBL_HUDManager inst = GetInstance();
		if (inst)
			inst.SetVisible(false);
	}
	
	static string GetNearestZoneName()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return "Unknown";
		
		IEntity player = RBL_NetworkComponent.GetLocalPlayerEntity();
		if (!player)
			return "Unknown";
		
		RBL_CampaignZone nearest = zoneMgr.GetNearestZone(player.GetOrigin());
		if (nearest)
			return nearest.GetZoneName();
		return "Unknown";
	}
	
	static int GetFIAZoneCount()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			return zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
		return 0;
	}
	
	static string GetNearestZoneOwner()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return "Unknown";
		
		IEntity player = RBL_NetworkComponent.GetLocalPlayerEntity();
		if (!player)
			return "Unknown";
		
		RBL_CampaignZone nearest = zoneMgr.GetNearestZone(player.GetOrigin());
		if (nearest)
		{
			ERBLFactionKey owner = nearest.GetOwnerFaction();
			if (owner == ERBLFactionKey.FIA)
				return "FIA";
			else if (owner == ERBLFactionKey.USSR)
				return "USSR";
			else if (owner == ERBLFactionKey.US)
				return "US";
			return "Neutral";
		}
		return "Unknown";
	}
	
	static int GetEnemyZoneCount()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			return zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR);
		return 0;
	}
	
	static float GetNearestZoneDistance()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return 0;
		
		IEntity player = RBL_NetworkComponent.GetLocalPlayerEntity();
		if (!player)
			return 0;
		
		RBL_CampaignZone nearest = zoneMgr.GetNearestZone(player.GetOrigin());
		if (nearest)
			return vector.Distance(player.GetOrigin(), nearest.GetOrigin());
		return 0;
	}
	
	// Legacy getters - delegate to MainHUD widget
	int GetDisplayMoney()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr && uiMgr.GetMainHUD())
			return uiMgr.GetMainHUD().GetDisplayMoney();
		return 0;
	}
	
	int GetDisplayHR()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr && uiMgr.GetMainHUD())
			return uiMgr.GetMainHUD().GetDisplayHR();
		return 0;
	}
	
	int GetDisplayWarLevel()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr && uiMgr.GetMainHUD())
			return uiMgr.GetMainHUD().GetDisplayWarLevel();
		return 1;
	}
	
	int GetDisplayAggression()
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr && uiMgr.GetMainHUD())
			return uiMgr.GetMainHUD().GetDisplayAggression();
		return 0;
	}
}

// ============================================================================
// HUD Component - Attach to layout files for proper UI (future)
// Currently DbgUI is used via RBL_ScreenHUD
// ============================================================================
class RBL_HUDComponent : ScriptedWidgetComponent
{
	protected Widget m_wRoot;
	protected TextWidget m_wMoneyText;
	protected TextWidget m_wHRText;
	protected TextWidget m_wZoneText;
	protected TextWidget m_wWarLevelText;
	protected TextWidget m_wScoreText;
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		
		m_wMoneyText = TextWidget.Cast(w.FindAnyWidget("MoneyText"));
		m_wHRText = TextWidget.Cast(w.FindAnyWidget("HRText"));
		m_wZoneText = TextWidget.Cast(w.FindAnyWidget("ZoneText"));
		m_wWarLevelText = TextWidget.Cast(w.FindAnyWidget("WarLevelText"));
		m_wScoreText = TextWidget.Cast(w.FindAnyWidget("ScoreText"));
	}
	
	void UpdateHUD()
	{
		RBL_HUDManager hud = RBL_HUDManager.GetInstance();
		if (!hud)
			return;
		
		if (m_wMoneyText)
			m_wMoneyText.SetText("$" + hud.GetDisplayMoney().ToString());
		
		if (m_wHRText)
			m_wHRText.SetText("HR: " + hud.GetDisplayHR().ToString());
		
		if (m_wZoneText)
		{
			string zoneInfo = hud.GetNearestZoneName() + " [" + hud.GetNearestZoneOwner() + "] - " + Math.Round(hud.GetNearestZoneDistance()).ToString() + "m";
			m_wZoneText.SetText(zoneInfo);
		}
		
		if (m_wWarLevelText)
			m_wWarLevelText.SetText("War: " + hud.GetDisplayWarLevel().ToString() + " | Alert: " + hud.GetDisplayAggression().ToString() + "%");
		
		if (m_wScoreText)
			m_wScoreText.SetText("FIA: " + hud.GetFIAZoneCount().ToString() + " | Enemy: " + hud.GetEnemyZoneCount().ToString());
	}
}
