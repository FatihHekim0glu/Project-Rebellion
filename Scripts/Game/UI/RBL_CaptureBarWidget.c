// ============================================================================
// PROJECT REBELLION - Capture Progress Bar Widget
// Bottom-center progress bar shown during zone capture
// ============================================================================

class RBL_CaptureBarWidgetImpl : RBL_BaseWidget
{
	// Capture data
	protected string m_sCapturingZoneID;
	protected string m_sCapturingZoneName;
	protected float m_fCaptureProgress;
	protected int m_iCapturingPlayers;
	protected bool m_bIsCapturing;
	protected bool m_bIsContested;
	
	// Visual state
	protected float m_fDisplayProgress;
	protected float m_fPulseTimer;
	protected float m_fGlowIntensity;
	protected bool m_bWasCapturing;
	
	void RBL_CaptureBarWidgetImpl()
	{
		// Position at bottom-center
		vector pos = RBL_UILayout.GetBottomCenterPosition(RBL_UISizes.CAPTURE_BAR_WIDTH, RBL_UISizes.CAPTURE_BAR_HEIGHT + 30, RBL_UISizes.HUD_MARGIN + 60);
		m_fPosX = pos[0];
		m_fPosY = pos[1];
		m_fWidth = RBL_UISizes.CAPTURE_BAR_WIDTH;
		m_fHeight = RBL_UISizes.CAPTURE_BAR_HEIGHT + 30;
		
		m_fUpdateInterval = 0.1; // Fast updates for smooth progress
		
		m_sCapturingZoneID = "";
		m_sCapturingZoneName = "";
		m_fCaptureProgress = 0;
		m_iCapturingPlayers = 0;
		m_bIsCapturing = false;
		m_bIsContested = false;
		
		m_fDisplayProgress = 0;
		m_fPulseTimer = 0;
		m_fGlowIntensity = 0;
		m_bWasCapturing = false;
	}
	
	override void OnUpdate()
	{
		RBL_CaptureSystem captureSys = RBL_CaptureSystem.GetInstance();
		if (!captureSys)
		{
			m_bIsCapturing = false;
			return;
		}
		
		// Get capture state
		m_bIsCapturing = captureSys.IsCapturingAny();
		
		if (m_bIsCapturing)
		{
			m_sCapturingZoneID = captureSys.GetCapturingZoneID();
			m_sCapturingZoneName = captureSys.GetCapturingZoneName();
			m_fCaptureProgress = captureSys.GetCaptureProgress();
			m_iCapturingPlayers = captureSys.GetCapturingPlayerCount();
			m_bIsContested = captureSys.IsContested();
		}
		
		// Detect capture start/complete
		if (m_bIsCapturing && !m_bWasCapturing)
		{
			// Started capturing
			m_fDisplayProgress = 0;
			m_fGlowIntensity = 1.0;
			RBL_UIManager.GetInstance().ShowNotification(
				"Capturing " + m_sCapturingZoneName + "...",
				RBL_UIColors.COLOR_ACCENT_AMBER,
				2.0
			);
		}
		else if (!m_bIsCapturing && m_bWasCapturing && m_fDisplayProgress >= 0.95)
		{
			// Capture completed
			RBL_UIManager.GetInstance().ShowNotification(
				m_sCapturingZoneName + " CAPTURED!",
				RBL_UIColors.COLOR_ACCENT_GREEN,
				3.0
			);
		}
		
		m_bWasCapturing = m_bIsCapturing;
	}
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		// Smooth progress animation
		float targetProgress = m_bIsCapturing ? m_fCaptureProgress : 0;
		float progressSpeed = 2.0;
		
		if (m_fDisplayProgress < targetProgress)
		{
			m_fDisplayProgress += progressSpeed * timeSlice;
			if (m_fDisplayProgress > targetProgress)
				m_fDisplayProgress = targetProgress;
		}
		else if (m_fDisplayProgress > targetProgress)
		{
			m_fDisplayProgress -= progressSpeed * timeSlice * 2; // Faster decay
			if (m_fDisplayProgress < targetProgress)
				m_fDisplayProgress = targetProgress;
		}
		
		// Pulse animation
		if (m_bIsCapturing)
		{
			m_fPulseTimer += timeSlice;
			if (m_fPulseTimer > RBL_UITiming.PULSE_PERIOD)
				m_fPulseTimer = 0;
		}
		
		// Glow decay
		if (m_fGlowIntensity > 0)
		{
			m_fGlowIntensity -= timeSlice * 2;
			if (m_fGlowIntensity < 0)
				m_fGlowIntensity = 0;
		}
		
		// Visibility based on capturing or recent capture
		if (m_bIsCapturing || m_fDisplayProgress > 0.01)
		{
			Show();
		}
		else
		{
			Hide();
		}
	}
	
	override void Draw()
	{
		if (!IsVisible() || m_fDisplayProgress <= 0.01)
			return;
		
		DrawCapturingLabel();
		DrawProgressBar();
		DrawSpeedIndicator();
	}
	
	protected void DrawCapturingLabel()
	{
		float x = m_fPosX + m_fWidth / 2;
		float y = m_fPosY;
		
		// Zone name
		string labelText = "CAPTURING: " + m_sCapturingZoneName;
		int labelColor = RBL_UIColors.COLOR_TEXT_BRIGHT;
		
		if (m_bIsContested)
		{
			labelText = "CONTESTED: " + m_sCapturingZoneName;
			labelColor = RBL_UIColors.COLOR_ACCENT_RED;
		}
		
		labelColor = ApplyAlpha(labelColor, m_fAlpha);
		
		DbgUI.Begin("Cap_Label", m_fPosX, y);
		DbgUI.Text(labelText);
		DbgUI.End();
	}
	
	protected void DrawProgressBar()
	{
		float x = m_fPosX;
		float y = m_fPosY + 20;
		float barWidth = m_fWidth;
		float barHeight = RBL_UISizes.CAPTURE_BAR_HEIGHT;
		
		// Outer glow (when first starting capture)
		if (m_fGlowIntensity > 0)
		{
			int glowColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_AMBER, m_fGlowIntensity * 0.3 * m_fAlpha);
			DrawRect(x - 4, y - 4, barWidth + 8, barHeight + 8, glowColor);
		}
		
		// Background
		int bgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_DARK, m_fAlpha);
		DrawRect(x, y, barWidth, barHeight, bgColor);
		
		// Progress fill
		float fillWidth = barWidth * m_fDisplayProgress;
		if (fillWidth > 0)
		{
			int fillColor = GetProgressColor();
			
			// Pulse effect
			float pulse = Math.Sin(m_fPulseTimer / RBL_UITiming.PULSE_PERIOD * 6.28) * 0.1 + 0.9;
			fillColor = ApplyAlpha(fillColor, pulse * m_fAlpha);
			
			DrawRect(x, y, fillWidth, barHeight, fillColor);
			
			// Highlight edge
			int edgeColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, 0.5 * m_fAlpha);
			DrawRect(x + fillWidth - 2, y, 2, barHeight, edgeColor);
		}
		
		// Border
		int borderColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_LIGHT, m_fAlpha);
		if (m_bIsContested)
			borderColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_RED, m_fAlpha);
		
		DrawRectOutline(x, y, barWidth, barHeight, borderColor, 1);
		
		// Percentage text
		string percentText = RBL_UIStrings.FormatPercent(m_fDisplayProgress);
		int percentColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		
		DbgUI.Begin("Cap_Percent", x + barWidth / 2 - 20, y + barHeight / 2 - 6);
		DbgUI.Text(percentText);
		DbgUI.End();
	}
	
	protected void DrawSpeedIndicator()
	{
		float x = m_fPosX;
		float y = m_fPosY + 20 + RBL_UISizes.CAPTURE_BAR_HEIGHT + 4;
		
		// Player count and capture speed
		string speedText = "";
		if (m_iCapturingPlayers == 1)
			speedText = "1 rebel capturing (1x speed)";
		else if (m_iCapturingPlayers > 1)
			speedText = m_iCapturingPlayers.ToString() + " rebels capturing (" + m_iCapturingPlayers.ToString() + "x speed)";
		
		if (m_bIsContested)
			speedText = "CONTESTED - Clear enemies!";
		
		int speedColor = RBL_UIColors.COLOR_TEXT_SECONDARY;
		if (m_bIsContested)
			speedColor = RBL_UIColors.COLOR_ACCENT_RED;
		else if (m_iCapturingPlayers > 1)
			speedColor = RBL_UIColors.COLOR_ACCENT_GREEN;
		
		speedColor = ApplyAlpha(speedColor, m_fAlpha);
		
		DbgUI.Begin("Cap_Speed", x, y);
		DbgUI.Text(speedText);
		DbgUI.End();
	}
	
	protected int GetProgressColor()
	{
		if (m_bIsContested)
			return RBL_UIColors.COLOR_ACCENT_RED;
		
		// Gradient from amber to green as progress increases
		if (m_fDisplayProgress < 0.5)
			return RBL_UIColors.COLOR_ACCENT_AMBER;
		if (m_fDisplayProgress < 0.9)
			return RBL_UIColors.COLOR_PROGRESS_CAPTURE;
		
		return RBL_UIColors.COLOR_ACCENT_GREEN;
	}
	
	// Force show for testing
	void ForceShow(string zoneName, float progress)
	{
		m_sCapturingZoneName = zoneName;
		m_fCaptureProgress = progress;
		m_fDisplayProgress = progress;
		m_bIsCapturing = true;
		m_iCapturingPlayers = 1;
		Show();
	}
	
	// Getters for testing
	bool IsCapturing() { return m_bIsCapturing; }
	float GetDisplayProgress() { return m_fDisplayProgress; }
	string GetCapturingZoneName() { return m_sCapturingZoneName; }
}

