// ============================================================================
// PROJECT REBELLION - Main HUD Widget
// Top-left panel showing money, HR, war level, aggression
// ============================================================================

class RBL_MainHUDWidget : RBL_PanelWidget
{
	// Cached data
	protected int m_iMoney;
	protected int m_iHR;
	protected int m_iWarLevel;
	protected int m_iAggression;
	protected int m_iFIAZones;
	protected int m_iEnemyZones;
	protected int m_iTotalZones;
	protected float m_fPlayTime;
	
	// Visual state
	protected bool m_bMoneyChanged;
	protected bool m_bHRChanged;
	protected float m_fMoneyFlashTimer;
	protected float m_fHRFlashTimer;
	protected int m_iPreviousMoney;
	protected int m_iPreviousHR;
	
	void RBL_MainHUDWidget()
	{
		// Position in top-left corner
		m_fPosX = RBL_UISizes.HUD_MARGIN;
		m_fPosY = RBL_UISizes.HUD_MARGIN;
		m_fWidth = RBL_UISizes.HUD_PANEL_WIDTH;
		m_fHeight = RBL_UISizes.HUD_PANEL_HEIGHT;
		
		m_fUpdateInterval = RBL_UITiming.HUD_UPDATE_INTERVAL;
		
		m_iMoney = 0;
		m_iHR = 0;
		m_iWarLevel = 1;
		m_iAggression = 0;
		m_iFIAZones = 0;
		m_iEnemyZones = 0;
		m_iTotalZones = 0;
		m_fPlayTime = 0;
		
		m_bMoneyChanged = false;
		m_bHRChanged = false;
		m_fMoneyFlashTimer = 0;
		m_fHRFlashTimer = 0;
		m_iPreviousMoney = 0;
		m_iPreviousHR = 0;
	}
	
	override void OnUpdate()
	{
		// Fetch economy data
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			int newMoney = econMgr.GetMoney();
			int newHR = econMgr.GetHR();
			
			// Check for changes
			if (newMoney != m_iMoney)
			{
				m_iPreviousMoney = m_iMoney;
				m_iMoney = newMoney;
				m_bMoneyChanged = true;
				m_fMoneyFlashTimer = 0.5;
			}
			
			if (newHR != m_iHR)
			{
				m_iPreviousHR = m_iHR;
				m_iHR = newHR;
				m_bHRChanged = true;
				m_fHRFlashTimer = 0.5;
			}
		}
		
		// Fetch campaign data
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			m_iWarLevel = campaignMgr.GetWarLevel();
			m_iAggression = campaignMgr.GetAggression();
			m_fPlayTime = campaignMgr.GetTotalPlayTime();
		}
		
		// Fetch zone counts
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			m_iFIAZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
			m_iEnemyZones = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR);
			m_iTotalZones = zoneMgr.GetTotalZoneCount();
		}
	}
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		// Decay flash timers
		if (m_fMoneyFlashTimer > 0)
		{
			m_fMoneyFlashTimer -= timeSlice;
			if (m_fMoneyFlashTimer <= 0)
				m_bMoneyChanged = false;
		}
		
		if (m_fHRFlashTimer > 0)
		{
			m_fHRFlashTimer -= timeSlice;
			if (m_fHRFlashTimer <= 0)
				m_bHRChanged = false;
		}
	}
	
	override void Draw()
	{
		if (!IsVisible())
			return;
		
		// Draw background panel
		DrawBackground();
		if (m_bShowBorder)
			DrawBorder();
		
		// Draw content
		DrawHeader();
		DrawResources();
		DrawWarStatus();
		DrawTerritoryBar();
	}
	
	protected void DrawHeader()
	{
		float x = GetContentX();
		float y = GetContentY();
		
		// Title
		int titleColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("HUD_Title", x, y);
		DbgUI.Text("PROJECT REBELLION");
		DbgUI.End();
	}
	
	protected void DrawResources()
	{
		float x = GetContentX();
		float y = GetContentY() + 24;
		float colWidth = GetContentWidth() / 2;
		
		// Money - left column
		int moneyColor = RBL_UIColors.COLOR_ACCENT_AMBER;
		if (m_bMoneyChanged)
		{
			// Flash green if gained, red if lost
			if (m_iMoney > m_iPreviousMoney)
				moneyColor = RBL_UIColors.COLOR_ACCENT_GREEN;
			else
				moneyColor = RBL_UIColors.COLOR_ACCENT_RED;
		}
		moneyColor = ApplyAlpha(moneyColor, m_fAlpha);
		
		DbgUI.Begin("HUD_Money", x, y);
		DbgUI.Text("FUNDS");
		DbgUI.End();
		
		DbgUI.Begin("HUD_MoneyVal", x, y + 16);
		DbgUI.Text(RBL_UIStrings.FormatMoney(m_iMoney));
		DbgUI.End();
		
		// HR - right column
		int hrColor = RBL_UIColors.COLOR_ACCENT_BLUE;
		if (m_bHRChanged)
		{
			if (m_iHR > m_iPreviousHR)
				hrColor = RBL_UIColors.COLOR_ACCENT_GREEN;
			else
				hrColor = RBL_UIColors.COLOR_ACCENT_RED;
		}
		hrColor = ApplyAlpha(hrColor, m_fAlpha);
		
		DbgUI.Begin("HUD_HR", x + colWidth, y);
		DbgUI.Text("MANPOWER");
		DbgUI.End();
		
		DbgUI.Begin("HUD_HRVal", x + colWidth, y + 16);
		DbgUI.Text(RBL_UIStrings.FormatHR(m_iHR));
		DbgUI.End();
	}
	
	protected void DrawWarStatus()
	{
		float x = GetContentX();
		float y = GetContentY() + 64;
		float colWidth = GetContentWidth() / 2;
		
		// War Level
		int warColor = GetWarLevelColor();
		warColor = ApplyAlpha(warColor, m_fAlpha);
		
		DbgUI.Begin("HUD_War", x, y);
		DbgUI.Text("WAR LEVEL");
		DbgUI.End();
		
		DbgUI.Begin("HUD_WarVal", x, y + 16);
		DbgUI.Text(m_iWarLevel.ToString() + " / 10");
		DbgUI.End();
		
		// Aggression
		int aggColor = GetAggressionColor();
		aggColor = ApplyAlpha(aggColor, m_fAlpha);
		
		DbgUI.Begin("HUD_Agg", x + colWidth, y);
		DbgUI.Text("ALERT");
		DbgUI.End();
		
		DbgUI.Begin("HUD_AggVal", x + colWidth, y + 16);
		DbgUI.Text(m_iAggression.ToString() + "%");
		DbgUI.End();
	}
	
	protected void DrawTerritoryBar()
	{
		float x = GetContentX();
		float y = GetContentY() + 108;
		float barWidth = GetContentWidth();
		float barHeight = 16;
		
		// Label
		DbgUI.Begin("HUD_TerLabel", x, y);
		DbgUI.Text("TERRITORY");
		DbgUI.End();
		
		// Background
		y += 16;
		int bgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_LIGHT, m_fAlpha);
		DrawRect(x, y, barWidth, barHeight, bgColor);
		
		// Calculate proportions
		if (m_iTotalZones > 0)
		{
			float fiaPercent = m_iFIAZones / (float)m_iTotalZones;
			float enemyPercent = m_iEnemyZones / (float)m_iTotalZones;
			
			// FIA portion (left, green)
			int fiaColor = ApplyAlpha(RBL_UIColors.COLOR_FACTION_FIA, m_fAlpha);
			float fiaWidth = barWidth * fiaPercent;
			if (fiaWidth > 0)
				DrawRect(x, y, fiaWidth, barHeight, fiaColor);
			
			// Enemy portion (right, red)
			int enemyColor = ApplyAlpha(RBL_UIColors.COLOR_FACTION_USSR, m_fAlpha);
			float enemyWidth = barWidth * enemyPercent;
			if (enemyWidth > 0)
				DrawRect(x + barWidth - enemyWidth, y, enemyWidth, barHeight, enemyColor);
		}
		
		// Zone counts text
		DbgUI.Begin("HUD_Zones", x, y + barHeight + 2);
		DbgUI.Text("FIA: " + m_iFIAZones.ToString() + "  |  Enemy: " + m_iEnemyZones.ToString());
		DbgUI.End();
	}
	
	protected int GetWarLevelColor()
	{
		if (m_iWarLevel <= 3)
			return RBL_UIColors.COLOR_ACCENT_GREEN;
		if (m_iWarLevel <= 6)
			return RBL_UIColors.COLOR_ACCENT_AMBER;
		return RBL_UIColors.COLOR_ACCENT_RED;
	}
	
	protected int GetAggressionColor()
	{
		if (m_iAggression <= 25)
			return RBL_UIColors.COLOR_ACCENT_GREEN;
		if (m_iAggression <= 50)
			return RBL_UIColors.COLOR_ACCENT_AMBER;
		if (m_iAggression <= 75)
			return RBL_UIColors.COLOR_STATUS_SPOTTED;
		return RBL_UIColors.COLOR_ACCENT_RED;
	}
	
	// Getters for testing
	int GetDisplayMoney() { return m_iMoney; }
	int GetDisplayHR() { return m_iHR; }
	int GetDisplayWarLevel() { return m_iWarLevel; }
	int GetDisplayAggression() { return m_iAggression; }
	int GetDisplayFIAZones() { return m_iFIAZones; }
	int GetDisplayEnemyZones() { return m_iEnemyZones; }
}

