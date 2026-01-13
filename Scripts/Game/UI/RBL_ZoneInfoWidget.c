// ============================================================================
// PROJECT REBELLION - Zone Info Widget
// Shows information about the nearest zone (bottom-left)
// ============================================================================

class RBL_ZoneInfoWidgetImpl : RBL_PanelWidget
{
	// Zone data
	protected string m_sZoneID;
	protected string m_sZoneName;
	protected ERBLZoneType m_eZoneType;
	protected ERBLFactionKey m_eOwnerFaction;
	protected float m_fDistanceToZone;
	protected int m_iGarrisonCurrent;
	protected int m_iGarrisonMax;
	protected int m_iSupportLevel;
	protected bool m_bInCaptureRange;
	protected bool m_bZoneVisible;
	
	// Animation state
	protected float m_fPulseTimer;
	protected bool m_bWasInRange;
	
	void RBL_ZoneInfoWidgetImpl()
	{
		// Position in bottom-left corner
		vector pos = RBL_UILayout.GetCornerPosition(2, RBL_UISizes.ZONE_PANEL_WIDTH, RBL_UISizes.ZONE_PANEL_HEIGHT + 40, RBL_UISizes.HUD_MARGIN);
		m_fPosX = pos[0];
		m_fPosY = pos[1];
		m_fWidth = RBL_UISizes.ZONE_PANEL_WIDTH;
		m_fHeight = RBL_UISizes.ZONE_PANEL_HEIGHT + 40;
		
		m_fUpdateInterval = RBL_UITiming.ZONE_UPDATE_INTERVAL;
		
		m_sZoneID = "";
		m_sZoneName = "";
		m_eZoneType = ERBLZoneType.Town;
		m_eOwnerFaction = ERBLFactionKey.USSR;
		m_fDistanceToZone = 999999;
		m_iGarrisonCurrent = 0;
		m_iGarrisonMax = 0;
		m_iSupportLevel = 0;
		m_bInCaptureRange = false;
		m_bZoneVisible = false;
		
		m_fPulseTimer = 0;
		m_bWasInRange = false;
	}
	
	override void OnUpdate()
	{
		// Get player position
		IEntity player = GetLocalPlayer();
		if (!player)
		{
			m_bZoneVisible = false;
			return;
		}
		
		vector playerPos = player.GetOrigin();
		
		// Find nearest zone (check both entity and virtual zones)
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
		{
			m_bZoneVisible = false;
			return;
		}
		
		// Check virtual zones first (primary zone type)
		RBL_VirtualZone nearestVirtual = zoneMgr.GetNearestVirtualZone(playerPos);
		RBL_CampaignZone nearestEntity = zoneMgr.GetNearestZone(playerPos);
		
		// Determine which is closer
		float virtualDist = 999999;
		float entityDist = 999999;
		
		if (nearestVirtual)
			virtualDist = vector.Distance(playerPos, nearestVirtual.GetZonePosition());
		if (nearestEntity)
			entityDist = vector.Distance(playerPos, nearestEntity.GetZonePosition());
		
		// Use virtual zone if closer or no entity zone
		if (nearestVirtual && virtualDist <= entityDist)
		{
			m_sZoneID = nearestVirtual.GetZoneID();
			m_sZoneName = nearestVirtual.GetZoneID();
			m_eZoneType = nearestVirtual.GetZoneType();
			m_eOwnerFaction = nearestVirtual.GetOwnerFaction();
			m_fDistanceToZone = virtualDist;
			m_iGarrisonCurrent = nearestVirtual.GetGarrisonStrength();
			m_iGarrisonMax = nearestVirtual.GetMaxGarrison();
			m_iSupportLevel = nearestVirtual.GetCivilianSupport();
			
			float captureRadius = nearestVirtual.GetCaptureRadius();
			m_bInCaptureRange = m_fDistanceToZone <= captureRadius;
		}
		else if (nearestEntity)
		{
			m_sZoneID = nearestEntity.GetZoneID();
			m_sZoneName = nearestEntity.GetZoneID();
			m_eZoneType = nearestEntity.GetZoneType();
			m_eOwnerFaction = nearestEntity.GetOwnerFaction();
			m_fDistanceToZone = entityDist;
			m_iGarrisonCurrent = nearestEntity.GetGarrisonStrength();
			m_iGarrisonMax = nearestEntity.GetMaxGarrison();
			m_iSupportLevel = nearestEntity.GetCivilianSupport();
			
			float captureRadius = nearestEntity.GetCaptureRadius();
			m_bInCaptureRange = m_fDistanceToZone <= captureRadius;
		}
		else
		{
			m_bZoneVisible = false;
			return;
		}
		
		// Only show if within reasonable distance (5km)
		m_bZoneVisible = m_fDistanceToZone < 5000;
		
		// Detect entering/leaving capture range
		if (m_bInCaptureRange && !m_bWasInRange)
		{
			RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
			if (uiMgr)
			{
				uiMgr.ShowNotification(
					"Entered " + m_sZoneName + " (" + RBL_UIStrings.GetZoneTypeName(m_eZoneType) + ")",
					RBL_UIColors.COLOR_TEXT_BRIGHT,
					2.0
				);
			}
		}
		m_bWasInRange = m_bInCaptureRange;
	}
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		// Pulse animation when in capture range
		if (m_bInCaptureRange)
		{
			m_fPulseTimer += timeSlice;
			if (m_fPulseTimer > RBL_UITiming.PULSE_PERIOD)
				m_fPulseTimer = 0;
		}
	}
	
	override void Draw()
	{
		if (!m_bZoneVisible || !IsVisible())
			return;
		
		// Draw background with faction-tinted border when in range
		if (m_bInCaptureRange)
		{
			// Pulsing border
			float pulse = Math.Sin(m_fPulseTimer / RBL_UITiming.PULSE_PERIOD * 6.28) * 0.3 + 0.7;
			m_iBorderColor = ApplyAlpha(RBL_UIColors.GetFactionColor(m_eOwnerFaction), pulse);
		}
		else
		{
			m_iBorderColor = RBL_UIColors.COLOR_BORDER_DARK;
		}
		
		DrawBackground();
		DrawBorder();
		DrawZoneHeader();
		DrawZoneDetails();
		DrawGarrisonBar();
	}
	
	protected void DrawZoneHeader()
	{
		float x = GetContentX();
		float y = GetContentY();
		
		// Zone type badge
		int typeColor = ApplyAlpha(GetZoneTypeColor(), m_fAlpha);
		DbgUI.Begin("Zone_Type", x, y);
		DbgUI.Text("[" + RBL_UIStrings.GetZoneTypeName(m_eZoneType) + "]");
		DbgUI.End();
		
		// Zone name
		int nameColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("Zone_Name", x, y + 18);
		DbgUI.Text(m_sZoneName);
		DbgUI.End();
		
		// Owner badge (right side)
		int ownerColor = ApplyAlpha(RBL_UIColors.GetFactionColor(m_eOwnerFaction), m_fAlpha);
		DbgUI.Begin("Zone_Owner", x + GetContentWidth() - 60, y);
		DbgUI.Text(RBL_UIStrings.GetFactionName(m_eOwnerFaction));
		DbgUI.End();
	}
	
	protected void DrawZoneDetails()
	{
		float x = GetContentX();
		float y = GetContentY() + 44;
		
		// Distance
		string distText = "Distance: " + RBL_UIStrings.FormatDistance(m_fDistanceToZone);
		int distColor = RBL_UIColors.COLOR_TEXT_SECONDARY;
		if (m_bInCaptureRange)
			distColor = RBL_UIColors.COLOR_ACCENT_GREEN;
		distColor = ApplyAlpha(distColor, m_fAlpha);
		
		DbgUI.Begin("Zone_Dist", x, y);
		DbgUI.Text(distText);
		DbgUI.End();
		
		// Status indicator
		string statusText = "";
		int statusColor = RBL_UIColors.COLOR_TEXT_MUTED;
		
		if (m_bInCaptureRange)
		{
			if (m_eOwnerFaction == ERBLFactionKey.FIA)
			{
				statusText = "FRIENDLY ZONE";
				statusColor = RBL_UIColors.COLOR_ACCENT_GREEN;
			}
			else if (m_iGarrisonCurrent > 0)
			{
				statusText = "HOSTILE - CLEAR GARRISON";
				statusColor = RBL_UIColors.COLOR_ACCENT_RED;
			}
			else
			{
				statusText = "READY TO CAPTURE";
				statusColor = RBL_UIColors.COLOR_ACCENT_AMBER;
			}
		}
		else
		{
			statusText = "OUT OF RANGE";
			statusColor = RBL_UIColors.COLOR_TEXT_MUTED;
		}
		
		statusColor = ApplyAlpha(statusColor, m_fAlpha);
		DbgUI.Begin("Zone_Status", x, y + 18);
		DbgUI.Text(statusText);
		DbgUI.End();
		
		// Support level (only for FIA zones)
		if (m_eOwnerFaction == ERBLFactionKey.FIA)
		{
			DbgUI.Begin("Zone_Support", x + GetContentWidth() - 80, y);
			DbgUI.Text("Support: " + m_iSupportLevel.ToString() + "%");
			DbgUI.End();
		}
	}
	
	protected void DrawGarrisonBar()
	{
		float x = GetContentX();
		float y = GetContentY() + 88;
		float barWidth = GetContentWidth();
		float barHeight = 12;
		
		// Label
		DbgUI.Begin("Zone_GarLabel", x, y);
		DbgUI.Text("Garrison: " + m_iGarrisonCurrent.ToString() + "/" + m_iGarrisonMax.ToString());
		DbgUI.End();
		
		y += 18;
		
		// Background
		int bgColor = ApplyAlpha(RBL_UIColors.COLOR_PROGRESS_BG, m_fAlpha);
		DrawRect(x, y, barWidth, barHeight, bgColor);
		
		// Fill
		if (m_iGarrisonMax > 0)
		{
			float percent = m_iGarrisonCurrent / (float)m_iGarrisonMax;
			float fillWidth = barWidth * percent;
			
			int fillColor = GetGarrisonColor(percent);
			fillColor = ApplyAlpha(fillColor, m_fAlpha);
			
			if (fillWidth > 0)
				DrawRect(x, y, fillWidth, barHeight, fillColor);
		}
	}
	
	protected int GetZoneTypeColor()
	{
		switch (m_eZoneType)
		{
			case ERBLZoneType.HQ: return RBL_UIColors.COLOR_ACCENT_GREEN;
			case ERBLZoneType.Airbase: return RBL_UIColors.COLOR_ACCENT_RED;
			case ERBLZoneType.Seaport: return RBL_UIColors.COLOR_ACCENT_BLUE;
			case ERBLZoneType.Factory: return RBL_UIColors.COLOR_ACCENT_AMBER;
		}
		return RBL_UIColors.COLOR_TEXT_SECONDARY;
	}
	
	protected int GetGarrisonColor(float percent)
	{
		if (percent <= 0.0)
			return RBL_UIColors.COLOR_ACCENT_GREEN;
		if (percent <= 0.33)
			return RBL_UIColors.COLOR_ACCENT_AMBER;
		if (percent <= 0.66)
			return RBL_UIColors.COLOR_STATUS_SPOTTED;
		return RBL_UIColors.COLOR_ACCENT_RED;
	}
	
	// Helper to get local player
	protected IEntity GetLocalPlayer()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;
		
		return pc.GetControlledEntity();
	}
	
	// Getters for testing
	string GetDisplayZoneID() { return m_sZoneID; }
	string GetDisplayZoneName() { return m_sZoneName; }
	ERBLFactionKey GetDisplayOwner() { return m_eOwnerFaction; }
	float GetDisplayDistance() { return m_fDistanceToZone; }
	bool IsInCaptureRange() { return m_bInCaptureRange; }
}

