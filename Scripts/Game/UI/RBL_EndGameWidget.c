// ============================================================================
// PROJECT REBELLION - End Game UI Widget
// Displays victory/defeat screens
// ============================================================================

class RBL_EndGameWidget : RBL_PanelWidget
{
	protected bool m_bIsVictory;
	protected float m_fFadeProgress;
	protected float m_fAnimTimer;
	protected float m_fDisplayTime;
	
	// Statistics
	protected float m_fCampaignTime;
	protected int m_iZonesCaptured;
	protected int m_iEnemyKills;
	protected int m_iPlayerDeaths;
	protected int m_iMissionsCompleted;
	protected string m_sEndReason;
	
	void RBL_EndGameWidget()
	{
		// Full screen overlay
		m_fPosX = 0;
		m_fPosY = 0;
		m_fWidth = RBL_UISizes.REFERENCE_WIDTH;
		m_fHeight = RBL_UISizes.REFERENCE_HEIGHT;
		
		m_iBackgroundColor = RBL_UIColors.COLOR_BG_DARK;
		m_iBorderColor = RBL_UIColors.COLOR_BORDER_LIGHT;
		
		m_bVisible = false;
		m_fFadeProgress = 0;
		m_fAnimTimer = 0;
		m_fDisplayTime = 0;
		m_bIsVictory = false;
		
		m_fCampaignTime = 0;
		m_iZonesCaptured = 0;
		m_iEnemyKills = 0;
		m_iPlayerDeaths = 0;
		m_iMissionsCompleted = 0;
		m_sEndReason = "";
		
		// Subscribe to victory manager events
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (victoryMgr)
		{
			victoryMgr.GetOnVictory().Insert(OnVictory);
			victoryMgr.GetOnDefeat().Insert(OnDefeat);
		}
	}
	
	// ========================================================================
	// EVENT HANDLERS
	// ========================================================================
	
	protected void OnVictory(ERBLVictoryCondition condition)
	{
		m_bIsVictory = true;
		m_sEndReason = RBL_VictoryManager.GetVictoryConditionString(condition);
		GatherStatistics();
		Show();
	}
	
	protected void OnDefeat(ERBLDefeatCondition condition)
	{
		m_bIsVictory = false;
		m_sEndReason = RBL_VictoryManager.GetDefeatConditionString(condition);
		GatherStatistics();
		Show();
	}
	
	protected void GatherStatistics()
	{
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		
		if (victoryMgr)
		{
			m_fCampaignTime = victoryMgr.GetCampaignTime();
			m_iPlayerDeaths = victoryMgr.GetPlayerDeaths();
		}
		
		if (campMgr)
		{
			m_iZonesCaptured = campMgr.GetZonesCaptured();
			m_iEnemyKills = campMgr.GetEnemyKillCount();
		}
		
		if (missionMgr)
		{
			m_iMissionsCompleted = missionMgr.GetTotalMissionsCompleted();
		}
	}
	
	// ========================================================================
	// VISIBILITY
	// ========================================================================
	
	override void Show()
	{
		super.Show();
		m_fFadeProgress = 0;
		m_fAnimTimer = 0;
		m_fDisplayTime = 0;
	}
	
	override void Hide()
	{
		super.Hide();
	}
	
	override bool IsVisible()
	{
		return m_bVisible || m_fFadeProgress > 0;
	}
	
	// ========================================================================
	// UPDATE
	// ========================================================================
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		// Fade animation
		if (m_bVisible && m_fFadeProgress < 1.0)
		{
			m_fFadeProgress += timeSlice * 1.5;
			if (m_fFadeProgress > 1.0)
				m_fFadeProgress = 1.0;
		}
		else if (!m_bVisible && m_fFadeProgress > 0)
		{
			m_fFadeProgress -= timeSlice * 2.0;
			if (m_fFadeProgress < 0)
				m_fFadeProgress = 0;
		}
		
		m_fAlpha = m_fFadeProgress;
		m_fAnimTimer += timeSlice;
		m_fDisplayTime += timeSlice;
	}
	
	// ========================================================================
	// DRAW
	// ========================================================================
	
	override void Draw()
	{
		if (m_fAlpha <= 0.01)
			return;
		
		// Full screen backdrop
		int bgColor = ApplyAlpha(ARGB(230, 10, 10, 10), m_fAlpha);
		DrawRect(0, 0, m_fWidth, m_fHeight, bgColor);
		
		// Content panel
		float panelWidth = 600;
		float panelHeight = 500;
		float panelX = (m_fWidth - panelWidth) / 2;
		float panelY = (m_fHeight - panelHeight) / 2;
		
		// Panel background
		int panelBgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_DARK, m_fAlpha);
		DrawRect(panelX, panelY, panelWidth, panelHeight, panelBgColor);
		
		// Border with victory/defeat color
		int borderColor = RBL_UIColors.COLOR_ACCENT_RED;
		if (m_bIsVictory)
			borderColor = RBL_UIColors.COLOR_ACCENT_GREEN;
		borderColor = ApplyAlpha(borderColor, m_fAlpha);
		DrawRectOutline(panelX, panelY, panelWidth, panelHeight, borderColor, 3);
		
		// Header
		DrawHeader(panelX, panelY, panelWidth);
		
		// Statistics
		DrawStatistics(panelX, panelY + 150, panelWidth);
		
		// End reason
		DrawEndReason(panelX, panelY + 350, panelWidth);
		
		// Controls hint
		DrawControls(panelX, panelY + panelHeight - 60, panelWidth);
	}
	
	protected void DrawHeader(float panelX, float panelY, float panelWidth)
	{
		// Title
		string title = "DEFEAT";
		if (m_bIsVictory)
			title = "VICTORY";
		int titleColor = RBL_UIColors.COLOR_ACCENT_RED;
		if (m_bIsVictory)
			titleColor = RBL_UIColors.COLOR_ACCENT_GREEN;
		titleColor = ApplyAlpha(titleColor, m_fAlpha);
		
		// Pulsing effect
		float pulse = Math.Sin(m_fAnimTimer * 3.0) * 0.1 + 0.9;
		
		float titleX = panelX + (panelWidth / 2) - 80;
		float titleY = panelY + 40;
		
		DbgUI.Begin("EndGameTitle", titleX, titleY);
		DbgUI.Text(title);
		DbgUI.End();
		
		// Subtitle
		string subtitle = "REBELLION CRUSHED";
		if (m_bIsVictory)
			subtitle = "REBELLION SUCCESSFUL";
		int subtitleColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
		
		DbgUI.Begin("EndGameSubtitle", panelX + (panelWidth / 2) - 100, titleY + 40);
		DbgUI.Text(subtitle);
		DbgUI.End();
		
		// Separator
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(panelX + 40, titleY + 80, panelWidth - 80, 2, lineColor);
	}
	
	protected void DrawStatistics(float panelX, float panelY, float panelWidth)
	{
		float statX = panelX + 60;
		float statY = panelY;
		float valueX = panelX + panelWidth - 150;
		float lineHeight = 32;
		
		int labelColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
		int valueColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		
		// Campaign Time
		DrawStatLine("Campaign Duration:", FormatTime(m_fCampaignTime), statX, statY, valueX, labelColor, valueColor);
		statY += lineHeight;
		
		// Zones Captured
		DrawStatLine("Zones Captured:", m_iZonesCaptured.ToString(), statX, statY, valueX, labelColor, valueColor);
		statY += lineHeight;
		
		// Enemy Kills
		DrawStatLine("Enemies Eliminated:", m_iEnemyKills.ToString(), statX, statY, valueX, labelColor, valueColor);
		statY += lineHeight;
		
		// Player Deaths
		DrawStatLine("Casualties:", m_iPlayerDeaths.ToString(), statX, statY, valueX, labelColor, valueColor);
		statY += lineHeight;
		
		// Missions Completed
		DrawStatLine("Missions Completed:", m_iMissionsCompleted.ToString(), statX, statY, valueX, labelColor, valueColor);
		statY += lineHeight;
		
		// K/D Ratio
		float kdRatio = 0;
		if (m_iPlayerDeaths > 0)
			kdRatio = m_iEnemyKills / m_iPlayerDeaths;
		else
			kdRatio = m_iEnemyKills;
		
		string kdStr = kdRatio.ToString();
		if (kdStr.Length() > 4)
			kdStr = kdStr.Substring(0, 4);
		
		DrawStatLine("Kill/Death Ratio:", kdStr, statX, statY, valueX, labelColor, valueColor);
	}
	
	protected void DrawStatLine(string label, string value, float labelX, float y, float valueX, int labelColor, int valueColor)
	{
		DbgUI.Begin("StatLabel_" + label, labelX, y);
		DbgUI.Text(label);
		DbgUI.End();
		
		DbgUI.Begin("StatValue_" + label, valueX, y);
		DbgUI.Text(value);
		DbgUI.End();
	}
	
	protected void DrawEndReason(float panelX, float panelY, float panelWidth)
	{
		int labelColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
		int reasonColor = RBL_UIColors.COLOR_ACCENT_RED;
		if (m_bIsVictory)
			reasonColor = RBL_UIColors.COLOR_ACCENT_GREEN;
		reasonColor = ApplyAlpha(reasonColor, m_fAlpha);
		
		float centerX = panelX + (panelWidth / 2);
		
		DbgUI.Begin("EndReasonLabel", centerX - 60, panelY);
		DbgUI.Text("OUTCOME");
		DbgUI.End();
		
		DbgUI.Begin("EndReasonValue", centerX - 100, panelY + 24);
		DbgUI.Text(m_sEndReason);
		DbgUI.End();
	}
	
	protected void DrawControls(float panelX, float panelY, float panelWidth)
	{
		int hintColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
		
		string hint = "[ESC] Return to Menu    [ENTER] Play Again";
		float centerX = panelX + (panelWidth / 2) - 150;
		
		DbgUI.Begin("EndGameControls", centerX, panelY);
		DbgUI.Text(hint);
		DbgUI.End();
	}
	
	// ========================================================================
	// HELPERS
	// ========================================================================
	
	protected string FormatTime(float seconds)
	{
		int totalSeconds = Math.Floor(seconds);
		int hours = totalSeconds / 3600;
		int minutes = (totalSeconds % 3600) / 60;
		int secs = totalSeconds % 60;
		
		string result = "";
		if (hours > 0)
			result = hours.ToString() + "h ";
		
		result += minutes.ToString() + "m " + secs.ToString() + "s";
		return result;
	}
}

// ============================================================================
// CAMPAIGN SUMMARY WIDGET (Quick stats overlay)
// ============================================================================
class RBL_CampaignSummaryWidget : RBL_PanelWidget
{
	void RBL_CampaignSummaryWidget()
	{
		m_fPosX = RBL_UISizes.HUD_MARGIN;
		m_fPosY = RBL_UISizes.REFERENCE_HEIGHT - 120;
		m_fWidth = 200;
		m_fHeight = 100;
		
		m_iBackgroundColor = RBL_UIColors.COLOR_BG_DARK;
		m_iBorderColor = RBL_UIColors.COLOR_BORDER_DARK;
		
		m_fUpdateInterval = 1.0;
	}
	
	override void Draw()
	{
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (!victoryMgr || !victoryMgr.IsActive())
			return;
		
		// Background
		int bgColor = ApplyAlpha(m_iBackgroundColor, m_fAlpha * 0.7);
		DrawRect(m_fPosX, m_fPosY, m_fWidth, m_fHeight, bgColor);
		
		float textY = m_fPosY + 8;
		float textX = m_fPosX + 8;
		int labelColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
		int valueColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_PRIMARY, m_fAlpha);
		
		// Progress
		float progress = victoryMgr.GetCampaignProgress() * 100;
		DbgUI.Begin("CampProgress", textX, textY);
		DbgUI.Text("Progress: " + Math.Round(progress).ToString() + "%");
		DbgUI.End();
		textY += 22;
		
		// Campaign Time
		string timeStr = FormatTimeShort(victoryMgr.GetCampaignTime());
		DbgUI.Begin("CampTime", textX, textY);
		DbgUI.Text("Time: " + timeStr);
		DbgUI.End();
		textY += 22;
		
		// Deaths
		int deaths = victoryMgr.GetPlayerDeaths();
		int maxDeaths = victoryMgr.GetMaxPlayerDeaths();
		string deathStr = deaths.ToString();
		if (maxDeaths > 0)
			deathStr += "/" + maxDeaths.ToString();
		
		DbgUI.Begin("CampDeaths", textX, textY);
		DbgUI.Text("Casualties: " + deathStr);
		DbgUI.End();
	}
	
	protected string FormatTimeShort(float seconds)
	{
		int totalSeconds = Math.Floor(seconds);
		int minutes = totalSeconds / 60;
		int secs = totalSeconds % 60;
		
		string secPad = "";
		if (secs < 10)
			secPad = "0";
		return minutes.ToString() + ":" + secPad + secs.ToString();
	}
}

