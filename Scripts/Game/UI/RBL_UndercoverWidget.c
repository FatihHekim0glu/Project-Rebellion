// ============================================================================
// PROJECT REBELLION - Undercover/Suspicion Widget
// Top-right indicator showing stealth status
// ============================================================================

class RBL_UndercoverWidgetImpl : RBL_BaseWidget
{
	// Status data
	protected ERBLCoverStatus m_eCoverStatus;
	protected float m_fSuspicionLevel;
	protected bool m_bIsDisguised;
	protected bool m_bInVehicle;
	protected bool m_bCarryingWeapon;
	protected int m_iNearbyEnemies;
	protected float m_fClosestEnemyDist;
	
	// Visual state
	protected float m_fDisplaySuspicion;
	protected float m_fPulseTimer;
	protected float m_fStatusChangeTimer;
	protected ERBLCoverStatus m_ePreviousStatus;
	protected bool m_bStatusChanged;
	
	// Eye icon animation
	protected float m_fEyeOpenness;
	
	void RBL_UndercoverWidgetImpl()
	{
		// Position in top-right corner
		float panelWidth = 200;
		float panelHeight = 80;
		vector pos = RBL_UILayout.GetCornerPosition(1, panelWidth, panelHeight, RBL_UISizes.HUD_MARGIN);
		m_fPosX = pos[0];
		m_fPosY = pos[1];
		m_fWidth = panelWidth;
		m_fHeight = panelHeight;
		
		m_fUpdateInterval = 0.2;
		
		m_eCoverStatus = ERBLCoverStatus.HIDDEN;
		m_fSuspicionLevel = 0;
		m_bIsDisguised = false;
		m_bInVehicle = false;
		m_bCarryingWeapon = false;
		m_iNearbyEnemies = 0;
		m_fClosestEnemyDist = 999999;
		
		m_fDisplaySuspicion = 0;
		m_fPulseTimer = 0;
		m_fStatusChangeTimer = 0;
		m_ePreviousStatus = ERBLCoverStatus.HIDDEN;
		m_bStatusChanged = false;
		
		m_fEyeOpenness = 0;
	}
	
	override void OnUpdate()
	{
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (!undercover)
		{
			m_eCoverStatus = ERBLCoverStatus.HIDDEN;
			m_fSuspicionLevel = 0;
			return;
		}
		
		// Get local player
		int localPlayerId = GetLocalPlayerId();
		if (localPlayerId < 0)
			return;
		
		// Fetch undercover state
		m_eCoverStatus = undercover.GetCoverStatus(localPlayerId);
		m_fSuspicionLevel = undercover.GetSuspicionLevel(localPlayerId);
		m_bIsDisguised = undercover.IsDisguised(localPlayerId);
		m_bInVehicle = undercover.IsInCivilianVehicle(localPlayerId);
		m_bCarryingWeapon = undercover.IsWeaponVisible(localPlayerId);
		m_iNearbyEnemies = undercover.GetNearbyEnemyCount(localPlayerId);
		m_fClosestEnemyDist = undercover.GetClosestEnemyDistance(localPlayerId);
		
		// Detect status changes
		if (m_eCoverStatus != m_ePreviousStatus)
		{
			m_bStatusChanged = true;
			m_fStatusChangeTimer = 1.0;
			
			// Notification on status change
			NotifyStatusChange(m_ePreviousStatus, m_eCoverStatus);
		}
		
		m_ePreviousStatus = m_eCoverStatus;
	}
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		// Smooth suspicion animation
		float targetSuspicion = m_fSuspicionLevel;
		float suspicionSpeed = 2.0;
		
		if (m_fDisplaySuspicion < targetSuspicion)
		{
			m_fDisplaySuspicion += suspicionSpeed * timeSlice;
			if (m_fDisplaySuspicion > targetSuspicion)
				m_fDisplaySuspicion = targetSuspicion;
		}
		else if (m_fDisplaySuspicion > targetSuspicion)
		{
			m_fDisplaySuspicion -= suspicionSpeed * timeSlice;
			if (m_fDisplaySuspicion < targetSuspicion)
				m_fDisplaySuspicion = targetSuspicion;
		}
		
		// Pulse timer (faster when more suspicious)
		float pulseSpeed = 1.0 + m_fDisplaySuspicion * 3.0;
		m_fPulseTimer += timeSlice * pulseSpeed;
		if (m_fPulseTimer > RBL_UITiming.PULSE_PERIOD)
			m_fPulseTimer = 0;
		
		// Status change timer decay
		if (m_fStatusChangeTimer > 0)
		{
			m_fStatusChangeTimer -= timeSlice;
			if (m_fStatusChangeTimer <= 0)
				m_bStatusChanged = false;
		}
		
		// Eye openness based on status
		float targetEye = GetTargetEyeOpenness();
		float eyeSpeed = 4.0;
		if (m_fEyeOpenness < targetEye)
		{
			m_fEyeOpenness += eyeSpeed * timeSlice;
			if (m_fEyeOpenness > targetEye)
				m_fEyeOpenness = targetEye;
		}
		else
		{
			m_fEyeOpenness -= eyeSpeed * timeSlice;
			if (m_fEyeOpenness < targetEye)
				m_fEyeOpenness = targetEye;
		}
	}
	
	override void Draw()
	{
		if (!IsVisible())
			return;
		
		DrawStatusIndicator();
		DrawSuspicionBar();
		DrawModifiers();
		DrawEnemyProximity();
	}
	
	protected void DrawStatusIndicator()
	{
		float x = m_fPosX;
		float y = m_fPosY;
		
		// Status text
		string statusText = RBL_UIStrings.GetCoverStatusName(m_eCoverStatus);
		int statusColor = RBL_UIColors.GetStatusColor(m_eCoverStatus);
		
		// Pulse effect on color when suspicious
		if (m_eCoverStatus != ERBLCoverStatus.HIDDEN)
		{
			float pulse = Math.Sin(m_fPulseTimer / RBL_UITiming.PULSE_PERIOD * 6.28) * 0.2 + 0.8;
			statusColor = ApplyAlpha(statusColor, pulse * m_fAlpha);
		}
		else
		{
			statusColor = ApplyAlpha(statusColor, m_fAlpha);
		}
		
		// Draw eye icon (simplified as text)
		string eyeIcon = GetEyeIcon();
		DbgUI.Begin("Cover_Eye", x, y);
		DbgUI.Text(eyeIcon);
		DbgUI.End();
		
		// Status text
		DbgUI.Begin("Cover_Status", x + 40, y);
		DbgUI.Text(statusText);
		DbgUI.End();
	}
	
	protected void DrawSuspicionBar()
	{
		float x = m_fPosX;
		float y = m_fPosY + 24;
		float barWidth = m_fWidth;
		float barHeight = 8;
		
		// Background
		int bgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_DARK, m_fAlpha);
		DrawRect(x, y, barWidth, barHeight, bgColor);
		
		// Fill
		if (m_fDisplaySuspicion > 0)
		{
			float fillWidth = barWidth * m_fDisplaySuspicion;
			int fillColor = GetSuspicionColor(m_fDisplaySuspicion);
			fillColor = ApplyAlpha(fillColor, m_fAlpha);
			
			DrawRect(x, y, fillWidth, barHeight, fillColor);
		}
		
		// Border
		int borderColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRectOutline(x, y, barWidth, barHeight, borderColor, 1);
		
		// Suspicion percentage
		if (m_fDisplaySuspicion > 0.05)
		{
			string suspText = RBL_UIStrings.FormatPercent(m_fDisplaySuspicion);
			int suspColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
			
			DbgUI.Begin("Cover_SuspPct", x + barWidth + 8, y - 2);
			DbgUI.Text(suspText);
			DbgUI.End();
		}
	}
	
	protected void DrawModifiers()
	{
		float x = m_fPosX;
		float y = m_fPosY + 40;
		
		// Build modifier string
		string modifiers = "";
		int modColor = RBL_UIColors.COLOR_TEXT_SECONDARY;
		
		if (m_bIsDisguised)
		{
			modifiers = modifiers + "[DISGUISED] ";
			modColor = RBL_UIColors.COLOR_ACCENT_GREEN;
		}
		
		if (m_bInVehicle)
		{
			modifiers = modifiers + "[VEHICLE] ";
		}
		
		if (m_bCarryingWeapon)
		{
			modifiers = modifiers + "[ARMED!] ";
			modColor = RBL_UIColors.COLOR_ACCENT_AMBER;
		}
		
		if (!modifiers.IsEmpty())
		{
			modColor = ApplyAlpha(modColor, m_fAlpha);
			DbgUI.Begin("Cover_Mods", x, y);
			DbgUI.Text(modifiers);
			DbgUI.End();
		}
	}
	
	protected void DrawEnemyProximity()
	{
		float x = m_fPosX;
		float y = m_fPosY + 58;
		
		if (m_iNearbyEnemies <= 0)
			return;
		
		// Enemy proximity warning
		string enemyText = "";
		int enemyColor = RBL_UIColors.COLOR_TEXT_SECONDARY;
		
		if (m_fClosestEnemyDist < 50)
		{
			enemyText = "! ENEMY VERY CLOSE !";
			enemyColor = RBL_UIColors.COLOR_ACCENT_RED;
		}
		else if (m_fClosestEnemyDist < 150)
		{
			enemyText = "Enemy nearby: " + RBL_UIStrings.FormatDistance(m_fClosestEnemyDist);
			enemyColor = RBL_UIColors.COLOR_ACCENT_AMBER;
		}
		else if (m_iNearbyEnemies > 0)
		{
			enemyText = m_iNearbyEnemies.ToString() + " enemies in area";
			enemyColor = RBL_UIColors.COLOR_TEXT_MUTED;
		}
		
		if (!enemyText.IsEmpty())
		{
			// Pulse for close enemies
			if (m_fClosestEnemyDist < 50)
			{
				float pulse = Math.Sin(m_fPulseTimer / RBL_UITiming.PULSE_PERIOD * 12.56) * 0.3 + 0.7;
				enemyColor = ApplyAlpha(enemyColor, pulse * m_fAlpha);
			}
			else
			{
				enemyColor = ApplyAlpha(enemyColor, m_fAlpha);
			}
			
			DbgUI.Begin("Cover_Enemy", x, y);
			DbgUI.Text(enemyText);
			DbgUI.End();
		}
	}
	
	protected int GetSuspicionColor(float level)
	{
		if (level < 0.3)
			return RBL_UIColors.COLOR_STATUS_HIDDEN;
		if (level < 0.6)
			return RBL_UIColors.COLOR_STATUS_SUSPICIOUS;
		if (level < 0.9)
			return RBL_UIColors.COLOR_STATUS_SPOTTED;
		return RBL_UIColors.COLOR_STATUS_COMPROMISED;
	}
	
	protected float GetTargetEyeOpenness()
	{
		switch (m_eCoverStatus)
		{
			case ERBLCoverStatus.HIDDEN: return 0.0;
			case ERBLCoverStatus.SUSPICIOUS: return 0.5;
			case ERBLCoverStatus.COMPROMISED: return 1.0;
			case ERBLCoverStatus.HOSTILE: return 1.0;
		}
		return 0.0;
	}
	
	protected string GetEyeIcon()
	{
		// ASCII art eye based on openness
		if (m_fEyeOpenness < 0.2)
			return "(-)";   // Closed - hidden
		if (m_fEyeOpenness < 0.6)
			return "(o)";   // Half open - suspicious
		return "(O)";       // Wide open - spotted
	}
	
	protected void NotifyStatusChange(ERBLCoverStatus oldStatus, ERBLCoverStatus newStatus)
	{
		string message = "";
		int color = RBL_UIColors.COLOR_TEXT_BRIGHT;
		
		// Determine message based on transition
		if (newStatus == ERBLCoverStatus.HIDDEN)
		{
			message = "Back in cover";
			color = RBL_UIColors.COLOR_ACCENT_GREEN;
		}
		else if (newStatus == ERBLCoverStatus.SUSPICIOUS)
		{
			message = "Drawing attention...";
			color = RBL_UIColors.COLOR_ACCENT_AMBER;
		}
		else if (newStatus == ERBLCoverStatus.COMPROMISED)
		{
			message = "COVER BLOWN!";
			color = RBL_UIColors.COLOR_ACCENT_RED;
		}
		else if (newStatus == ERBLCoverStatus.HOSTILE)
		{
			message = "ENEMY ENGAGED!";
			color = RBL_UIColors.COLOR_ACCENT_RED;
		}
		
		if (!message.IsEmpty())
		{
			RBL_UIManager.GetInstance().ShowNotification(message, color, 2.5);
		}
	}
	
	protected int GetLocalPlayerId()
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return -1;
		
		return pm.GetPlayerIdFromControlledEntity(GetGame().GetPlayerController().GetControlledEntity());
	}
	
	// Force state for testing
	void ForceState(ERBLCoverStatus status, float suspicion)
	{
		m_eCoverStatus = status;
		m_fSuspicionLevel = suspicion;
		m_fDisplaySuspicion = suspicion;
	}
	
	// Getters for testing
	ERBLCoverStatus GetDisplayStatus() { return m_eCoverStatus; }
	float GetDisplaySuspicion() { return m_fDisplaySuspicion; }
}

