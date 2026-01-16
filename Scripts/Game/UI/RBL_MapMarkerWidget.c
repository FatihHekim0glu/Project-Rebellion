// ============================================================================
// PROJECT REBELLION - Map Marker System
// World-space markers for zones visible in-game
// ============================================================================

// Individual zone marker
class RBL_ZoneMarker
{
	string m_sZoneID;
	string m_sZoneName;
	ERBLZoneType m_eZoneType;
	ERBLFactionKey m_eOwnerFaction;
	vector m_vWorldPosition;
	float m_fDistanceToPlayer;
	bool m_bIsVisible;
	float m_fAlpha;
	float m_fScale;
	bool m_bUnderAttack;
	
	void RBL_ZoneMarker(string zoneID)
	{
		m_sZoneID = zoneID;
		m_sZoneName = "";
		m_eZoneType = ERBLZoneType.Town;
		m_eOwnerFaction = ERBLFactionKey.USSR;
		m_vWorldPosition = vector.Zero;
		m_fDistanceToPlayer = 0;
		m_bIsVisible = true;
		m_fAlpha = 1.0;
		m_fScale = 1.0;
		m_bUnderAttack = false;
	}
}

// Map marker manager
class RBL_MapMarkerManagerImpl : RBL_BaseWidget
{
	protected ref map<string, ref RBL_ZoneMarker> m_mMarkers;
	protected ref array<string> m_aMarkerIDs;
	protected float m_fMaxDrawDistance;
	protected float m_fMinDrawDistance;
	protected float m_fFadeStartDistance;
	protected bool m_bShowAllMarkers;
	protected bool m_bShowDistantMarkers;
	
	// Animation
	protected float m_fPulseTimer;
	
	void RBL_MapMarkerManagerImpl()
	{
		m_mMarkers = new map<string, ref RBL_ZoneMarker>();
		m_aMarkerIDs = new array<string>();
		m_fMaxDrawDistance = 3000.0;
		m_fMinDrawDistance = 50.0;
		m_fFadeStartDistance = 2500.0;
		m_bShowAllMarkers = true;
		m_bShowDistantMarkers = true;
		
		m_fPulseTimer = 0;
		
		m_fUpdateInterval = RBL_UITiming.MAP_UPDATE_INTERVAL;
	}
	
	override void OnUpdate()
	{
		// Get player position
		IEntity player = GetLocalPlayer();
		if (!player)
			return;
		
		vector playerPos = player.GetOrigin();
		vector playerForward = GetPlayerLookDirection(player);
		
		// Refresh markers from zone manager
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		// Update markers from virtual zones (primary zone system)
		array<ref RBL_VirtualZone> virtualZones = zoneMgr.GetAllVirtualZones();
		for (int i = 0; i < virtualZones.Count(); i++)
		{
			RBL_VirtualZone vZone = virtualZones[i];
			if (!vZone)
				continue;
			
			string zoneID = vZone.GetZoneID();
			
			// Get or create marker
			RBL_ZoneMarker marker;
			if (!m_mMarkers.Find(zoneID, marker))
			{
				marker = new RBL_ZoneMarker(zoneID);
				m_mMarkers.Insert(zoneID, marker);
				m_aMarkerIDs.Insert(zoneID);
			}
			
			// Update marker data from virtual zone
			marker.m_sZoneName = vZone.GetZoneName();
			marker.m_eZoneType = vZone.GetZoneType();
			marker.m_eOwnerFaction = vZone.GetOwnerFaction();
			marker.m_vWorldPosition = vZone.GetZonePosition();
			marker.m_bUnderAttack = vZone.IsUnderAttack();
			
			UpdateMarkerVisibility(marker, playerPos, playerForward);
		}
		
		// Also update markers from entity zones if any
		array<RBL_CampaignZone> entityZones = zoneMgr.GetAllZones();
		for (int i = 0; i < entityZones.Count(); i++)
		{
			RBL_CampaignZone eZone = entityZones[i];
			if (!eZone)
				continue;
			
			string zoneID = eZone.GetZoneID();
			
			// Skip if already have marker from virtual zone
			if (m_mMarkers.Contains(zoneID))
				continue;
			
			// Get or create marker
			RBL_ZoneMarker marker;
			if (!m_mMarkers.Find(zoneID, marker))
			{
				marker = new RBL_ZoneMarker(zoneID);
				m_mMarkers.Insert(zoneID, marker);
				m_aMarkerIDs.Insert(zoneID);
			}
			
			// Update marker data from entity zone
			marker.m_sZoneName = eZone.GetZoneID();
			marker.m_eZoneType = eZone.GetZoneType();
			marker.m_eOwnerFaction = eZone.GetOwnerFaction();
			marker.m_vWorldPosition = eZone.GetZonePosition();
			marker.m_bUnderAttack = false;
			
			UpdateMarkerVisibility(marker, playerPos, playerForward);
		}
	}
	
	protected void UpdateMarkerVisibility(RBL_ZoneMarker marker, vector playerPos, vector playerForward)
	{
		// Calculate distance
		marker.m_fDistanceToPlayer = vector.Distance(playerPos, marker.m_vWorldPosition);
		
		// Visibility checks
		bool inRange = marker.m_fDistanceToPlayer >= m_fMinDrawDistance && 
					   marker.m_fDistanceToPlayer <= m_fMaxDrawDistance;
		bool inFOV = IsInPlayerFOV(playerPos, playerForward, marker.m_vWorldPosition);
		
		marker.m_bIsVisible = inRange && (inFOV || m_bShowDistantMarkers);
		
		// Calculate alpha based on distance
		if (marker.m_fDistanceToPlayer > m_fFadeStartDistance)
		{
			float fadeRange = m_fMaxDrawDistance - m_fFadeStartDistance;
			float fadeProgress = (marker.m_fDistanceToPlayer - m_fFadeStartDistance) / fadeRange;
			marker.m_fAlpha = Math.Max(0.3, 1.0 - fadeProgress);
		}
		else
		{
			marker.m_fAlpha = 1.0;
		}
		
		// Scale based on distance (closer = larger)
		float baseScale = 1.0;
		float distanceScale = Math.Clamp(1.0 - (marker.m_fDistanceToPlayer / m_fMaxDrawDistance) * 0.5, 0.5, 1.5);
		marker.m_fScale = baseScale * distanceScale;
	}
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		// Pulse animation
		m_fPulseTimer += timeSlice;
		if (m_fPulseTimer > RBL_UITiming.PULSE_PERIOD)
			m_fPulseTimer = 0;
	}
	
	override void Draw()
	{
		if (!IsVisible())
			return;
		
		// Draw all visible markers using our tracked ID array
		for (int i = 0; i < m_aMarkerIDs.Count(); i++)
		{
			RBL_ZoneMarker marker;
			if (!m_mMarkers.Find(m_aMarkerIDs[i], marker))
				continue;
			
			if (!marker.m_bIsVisible)
				continue;
			
			DrawMarker(marker);
		}
	}
	
	protected void DrawMarker(RBL_ZoneMarker marker)
	{
		// Convert world position to screen position
		vector screenPos;
		if (!WorldToScreen(marker.m_vWorldPosition, screenPos))
			return;
		
		float x = screenPos[0];
		float y = screenPos[1];
		float alpha = marker.m_fAlpha * m_fAlpha;
		float scale = marker.m_fScale;
		
		// Icon size
		float iconSize = RBL_UISizes.ICON_LARGE * scale;
		
		// Marker background
		int bgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_DARK, alpha * 0.8);
		DrawRect(x - iconSize / 2 - 4, y - iconSize / 2 - 4, iconSize + 8, iconSize + 8, bgColor);
		
		// Faction-colored border
		int factionColor = RBL_UIColors.GetFactionColor(marker.m_eOwnerFaction);
		
		// Pulse if under attack
		if (marker.m_bUnderAttack)
		{
			float pulse = Math.Sin(m_fPulseTimer / RBL_UITiming.PULSE_PERIOD * 12.56) * 0.3 + 0.7;
			factionColor = RBL_UIColors.COLOR_ACCENT_RED;
			alpha *= pulse;
		}
		
		factionColor = ApplyAlpha(factionColor, alpha);
		DrawRectOutline(x - iconSize / 2 - 4, y - iconSize / 2 - 4, iconSize + 8, iconSize + 8, factionColor, 2);
		
		// Zone type icon (text-based)
		string icon = GetZoneTypeIcon(marker.m_eZoneType);
		int iconColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, alpha);
		
		DbgUI.Begin("Marker_Icon_" + marker.m_sZoneID, x - 6, y - 8);
		DbgUI.Text(icon);
		DbgUI.End();
		
		// Zone name (below icon)
		int nameColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, alpha);
		DbgUI.Begin("Marker_Name_" + marker.m_sZoneID, x - 40, y + iconSize / 2 + 4);
		DbgUI.Text(marker.m_sZoneName);
		DbgUI.End();
		
		// Distance (below name)
		string distText = RBL_UIStrings.FormatDistance(marker.m_fDistanceToPlayer);
		int distColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, alpha);
		
		DbgUI.Begin("Marker_Dist_" + marker.m_sZoneID, x - 20, y + iconSize / 2 + 20);
		DbgUI.Text(distText);
		DbgUI.End();
		
		// Under attack indicator
		if (marker.m_bUnderAttack)
		{
			int attackColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_RED, alpha);
			DbgUI.Begin("Marker_Attack_" + marker.m_sZoneID, x - 30, y - iconSize / 2 - 20);
			DbgUI.Text("! ATTACK !");
			DbgUI.End();
		}
	}
	
	protected string GetZoneTypeIcon(ERBLZoneType type)
	{
		switch (type)
		{
			case ERBLZoneType.HQ: return "HQ";
			case ERBLZoneType.Town: return "T";
			case ERBLZoneType.Outpost: return "OP";
			case ERBLZoneType.Airbase: return "AB";
			case ERBLZoneType.Factory: return "F";
			case ERBLZoneType.Seaport: return "SP";
			case ERBLZoneType.Resource: return "R";
			case ERBLZoneType.Roadblock: return "RB";
			case ERBLZoneType.Watchpost: return "W";
		}
		return "?";
	}
	
	// Helpers
	protected IEntity GetLocalPlayer()
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return null;
		
		return GetGame().GetPlayerController().GetControlledEntity();
	}
	
	protected vector GetPlayerLookDirection(IEntity player)
	{
		if (!player)
			return vector.Forward;
		
		vector mat[4];
		player.GetTransform(mat);
		
		// Forward vector is third element (Z axis)
		return mat[2];
	}
	
	protected bool IsInPlayerFOV(vector playerPos, vector playerForward, vector targetPos)
	{
		vector toTarget = (targetPos - playerPos).Normalized();
		float dot = vector.Dot(playerForward, toTarget);
		
		// ~120 degree FOV
		return dot > 0.0;
	}
	
	protected bool WorldToScreen(vector worldPos, out vector screenPos)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return false;
		
		IEntity player = GetLocalPlayer();
		if (!player)
			return false;
		
		int screen_w = workspace.GetWidth();
		int screen_h = workspace.GetHeight();
		
		// Adjust world position to be at eye level
		vector adjustedPos = worldPos;
		adjustedPos[1] = adjustedPos[1] + 2.0;
		
		// Get camera position and direction from player transform
		vector camPos = player.GetOrigin();
		vector camForward = GetPlayerLookDirection(player);
		vector camRight = GetPlayerRightDirection(player);
		vector camUp = Vector(0, 1, 0);
		
		// Calculate vector from camera to target
		vector toTarget = adjustedPos - camPos;
		float dist = toTarget.Length();
		
		if (dist < 1)
			return false;
		
		vector toTargetNorm = toTarget.Normalized();
		
		// Check if target is behind camera
		float dotForward = vector.Dot(camForward, toTargetNorm);
		if (dotForward < 0)
			return false;
		
		// Calculate horizontal and vertical angles
		float dotRight = vector.Dot(camRight, toTargetNorm);
		float dotUp = vector.Dot(camUp, toTargetNorm);
		
		// Approximate FOV (typical FOV is around 70-90 degrees)
		float fov = 75.0;
		float fovRad = fov * Math.DEG2RAD;
		float halfFov = fovRad * 0.5;
		
		// Calculate screen position using perspective projection
		// Horizontal angle from forward direction
		float horizontalAngle = Math.Atan2(dotRight, dotForward);
		float verticalAngle = Math.Atan2(dotUp, dotForward);
		
		// Project to screen coordinates
		// Normalize angles to screen space (-1 to 1)
		float screenX = Math.Tan(horizontalAngle) / Math.Tan(halfFov);
		float screenY = -Math.Tan(verticalAngle) / Math.Tan(halfFov);
		
		// Convert to pixel coordinates
		screenPos[0] = (screen_w * 0.5) + (screenX * screen_w * 0.5);
		screenPos[1] = (screen_h * 0.5) + (screenY * screen_h * 0.5);
		screenPos[2] = dist;
		
		// Check if on screen
		bool onScreen = screenPos[0] >= 0 && screenPos[0] <= screen_w &&
		                screenPos[1] >= 0 && screenPos[1] <= screen_h;
		
		return onScreen;
	}
	
	protected vector GetPlayerRightDirection(IEntity player)
	{
		if (!player)
			return Vector(1, 0, 0);
		
		// Get player transform matrix
		vector mat[4];
		player.GetTransform(mat);
		
		// Right vector is first element (X axis)
		return mat[0];
	}
	
	// Settings
	void SetMaxDrawDistance(float distance) { m_fMaxDrawDistance = distance; }
	void SetShowAllMarkers(bool show) { m_bShowAllMarkers = show; }
	void SetShowDistantMarkers(bool show) { m_bShowDistantMarkers = show; }
	
	// Get marker count
	int GetVisibleMarkerCount()
	{
		int count = 0;
		
		for (int i = 0; i < m_aMarkerIDs.Count(); i++)
		{
			RBL_ZoneMarker marker;
			if (!m_mMarkers.Find(m_aMarkerIDs[i], marker))
				continue;
			
			if (marker.m_bIsVisible)
				count++;
		}
		return count;
	}
	
	// Get total marker count
	int GetTotalMarkerCount()
	{
		return m_aMarkerIDs.Count();
	}
}

