// ============================================================================
// PROJECT REBELLION - HUD System
// Displays money, HR, zone info, war level on screen
// ============================================================================

class RBL_HUDManager
{
	protected static ref RBL_HUDManager s_Instance;
	
	protected bool m_bVisible;
	protected float m_fUpdateInterval;
	protected float m_fTimeSinceUpdate;
	
	// Cached display values
	protected int m_iDisplayMoney;
	protected int m_iDisplayHR;
	protected int m_iDisplayWarLevel;
	protected int m_iDisplayAggression;
	protected string m_sNearestZoneName;
	protected string m_sNearestZoneOwner;
	protected float m_fNearestZoneDistance;
	protected int m_iFIAZones;
	protected int m_iEnemyZones;
	
	static RBL_HUDManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_HUDManager();
		return s_Instance;
	}
	
	void RBL_HUDManager()
	{
		m_bVisible = true;
		m_fUpdateInterval = 0.5;
		m_fTimeSinceUpdate = 0;
		m_sNearestZoneName = "None";
		m_sNearestZoneOwner = "Unknown";
	}
	
	void Update(float timeSlice)
	{
		if (!m_bVisible)
			return;
		
		m_fTimeSinceUpdate += timeSlice;
		if (m_fTimeSinceUpdate >= m_fUpdateInterval)
		{
			m_fTimeSinceUpdate = 0;
			RefreshData();
			DrawHUD();
		}
	}
	
	protected void RefreshData()
	{
		// Get economy data
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			m_iDisplayMoney = econMgr.GetMoney();
			m_iDisplayHR = econMgr.GetHR();
		}
		
		// Get campaign data
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			m_iDisplayWarLevel = campaignMgr.GetWarLevel();
			m_iDisplayAggression = campaignMgr.GetAggression();
		}
		
		// Get zone data
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			m_iFIAZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
			m_iEnemyZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR);
			
			// Find nearest zone to player
			IEntity player = GetGame().GetPlayerController().GetControlledEntity();
			if (player)
			{
				vector playerPos = player.GetOrigin();
				RBL_VirtualZone nearestZone = zoneMgr.GetNearestVirtualZone(playerPos);
				if (nearestZone)
				{
					m_sNearestZoneName = nearestZone.GetZoneID();
					m_fNearestZoneDistance = vector.Distance(playerPos, nearestZone.GetZonePosition());
					
					ERBLFactionKey owner = nearestZone.GetOwnerFaction();
					if (owner == ERBLFactionKey.FIA)
						m_sNearestZoneOwner = "FIA";
					else if (owner == ERBLFactionKey.USSR)
						m_sNearestZoneOwner = "USSR";
					else if (owner == ERBLFactionKey.US)
						m_sNearestZoneOwner = "US";
					else
						m_sNearestZoneOwner = "Neutral";
				}
			}
		}
	}
	
	protected void DrawHUD()
	{
		// Use debug shapes to draw on screen (simple approach)
		int screenW = 1920;
		int screenH = 1080;
		
		// Draw to console for now (visible HUD requires layout files)
		// This will be replaced with proper UI widgets
	}
	
	void ToggleVisibility()
	{
		m_bVisible = !m_bVisible;
	}
	
	bool IsVisible() { return m_bVisible; }
	
	// Getters for UI widgets
	int GetDisplayMoney() { return m_iDisplayMoney; }
	int GetDisplayHR() { return m_iDisplayHR; }
	int GetDisplayWarLevel() { return m_iDisplayWarLevel; }
	int GetDisplayAggression() { return m_iDisplayAggression; }
	string GetNearestZoneName() { return m_sNearestZoneName; }
	string GetNearestZoneOwner() { return m_sNearestZoneOwner; }
	float GetNearestZoneDistance() { return m_fNearestZoneDistance; }
	int GetFIAZoneCount() { return m_iFIAZones; }
	int GetEnemyZoneCount() { return m_iEnemyZones; }
}

// ============================================================================
// HUD Component - Attach to player or game mode
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

