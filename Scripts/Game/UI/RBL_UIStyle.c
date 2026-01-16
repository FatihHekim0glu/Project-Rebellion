// ============================================================================
// PROJECT REBELLION - UI Style Constants
// Military/tactical aesthetic - clean, minimal, functional
// ============================================================================

class RBL_UIColors
{
	// Primary palette - Military tactical
	static const int COLOR_BG_DARK = ARGB(200, 15, 20, 15);           // Near-black green
	static const int COLOR_BG_MEDIUM = ARGB(180, 25, 35, 25);         // Dark military green
	static const int COLOR_BG_LIGHT = ARGB(160, 40, 50, 40);          // Lighter green
	static const int COLOR_BG_HIGHLIGHT = ARGB(140, 50, 65, 50);      // Hover/selected
	
	// Borders
	static const int COLOR_BORDER_DARK = ARGB(255, 45, 55, 45);       // Subtle border
	static const int COLOR_BORDER_LIGHT = ARGB(255, 70, 85, 70);      // Accent border
	static const int COLOR_BORDER_GLOW = ARGB(200, 100, 130, 100);    // Active glow
	
	// Text colors
	static const int COLOR_TEXT_PRIMARY = ARGB(255, 200, 210, 195);   // Main text - off-white
	static const int COLOR_TEXT_SECONDARY = ARGB(200, 150, 160, 145); // Secondary - gray
	static const int COLOR_TEXT_MUTED = ARGB(150, 100, 110, 95);      // Hints - dark gray
	static const int COLOR_TEXT_BRIGHT = ARGB(255, 240, 245, 235);    // Highlighted text
	
	// Accent colors
	static const int COLOR_ACCENT_GREEN = ARGB(255, 80, 180, 80);     // Positive/FIA
	static const int COLOR_ACCENT_AMBER = ARGB(255, 220, 180, 60);    // Warning/caution
	static const int COLOR_ACCENT_YELLOW = ARGB(255, 220, 200, 60);   // Alert/attention
	static const int COLOR_ACCENT_RED = ARGB(255, 200, 70, 70);       // Negative/enemy
	static const int COLOR_ACCENT_BLUE = ARGB(255, 70, 130, 200);     // Info/neutral
	
	// Faction colors
	static const int COLOR_FACTION_FIA = ARGB(255, 60, 160, 60);      // Rebel green
	static const int COLOR_FACTION_USSR = ARGB(255, 180, 50, 50);     // Soviet red
	static const int COLOR_FACTION_US = ARGB(255, 50, 100, 180);      // US blue
	static const int COLOR_FACTION_NEUTRAL = ARGB(255, 130, 130, 130);// Gray
	
	// Progress bars
	static const int COLOR_PROGRESS_BG = ARGB(200, 20, 25, 20);       // Empty progress
	static const int COLOR_PROGRESS_FILL = ARGB(255, 80, 160, 80);    // Filled progress
	static const int COLOR_PROGRESS_CAPTURE = ARGB(255, 200, 170, 50);// Capturing
	
	// Status colors
	static const int COLOR_STATUS_HIDDEN = ARGB(255, 80, 180, 80);    // Safe/green
	static const int COLOR_STATUS_SUSPICIOUS = ARGB(255, 200, 180, 50);// Caution/amber
	static const int COLOR_STATUS_SPOTTED = ARGB(255, 220, 130, 50);  // Warning/orange
	static const int COLOR_STATUS_COMPROMISED = ARGB(255, 200, 60, 60);// Danger/red
	
	// Helper to get faction color
	static int GetFactionColor(ERBLFactionKey faction)
	{
		switch (faction)
		{
			case ERBLFactionKey.FIA: return COLOR_FACTION_FIA;
			case ERBLFactionKey.USSR: return COLOR_FACTION_USSR;
			case ERBLFactionKey.US: return COLOR_FACTION_US;
		}
		return COLOR_FACTION_NEUTRAL;
	}
	
	// Helper to get status color
	static int GetStatusColor(ERBLCoverStatus status)
	{
		switch (status)
		{
			case ERBLCoverStatus.HIDDEN: return COLOR_STATUS_HIDDEN;
			case ERBLCoverStatus.SUSPICIOUS: return COLOR_STATUS_SUSPICIOUS;
			case ERBLCoverStatus.SPOTTED: return COLOR_STATUS_SPOTTED;
			case ERBLCoverStatus.COMPROMISED: return COLOR_STATUS_COMPROMISED;
			case ERBLCoverStatus.HOSTILE: return COLOR_STATUS_COMPROMISED;
		}
		return COLOR_STATUS_HIDDEN;
	}
}

class RBL_UISizes
{
	// Screen reference (design at 1920x1080, scales automatically)
	static const float REFERENCE_WIDTH = 1920.0;
	static const float REFERENCE_HEIGHT = 1080.0;
	
	// Panel dimensions
	static const float HUD_PANEL_WIDTH = 280.0;
	static const float HUD_PANEL_HEIGHT = 160.0;
	static const float HUD_MARGIN = 16.0;
	static const float HUD_PADDING = 12.0;
	
	// Zone info panel
	static const float ZONE_PANEL_WIDTH = 320.0;
	static const float ZONE_PANEL_HEIGHT = 120.0;
	
	// Progress bars
	static const float PROGRESS_HEIGHT = 8.0;
	static const float PROGRESS_HEIGHT_LARGE = 16.0;
	
	// Capture bar
	static const float CAPTURE_BAR_WIDTH = 400.0;
	static const float CAPTURE_BAR_HEIGHT = 24.0;
	
	// Shop menu
	static const float SHOP_WIDTH = 800.0;
	static const float SHOP_HEIGHT = 600.0;
	static const float SHOP_ITEM_HEIGHT = 48.0;
	
	// Notification toast
	static const float TOAST_WIDTH = 350.0;
	static const float TOAST_HEIGHT = 60.0;
	static const float TOAST_MARGIN = 12.0;
	
	// Icon sizes
	static const float ICON_SMALL = 16.0;
	static const float ICON_MEDIUM = 24.0;
	static const float ICON_LARGE = 32.0;
	
	// Border/corner radius
	static const float BORDER_WIDTH = 1.0;
	static const float CORNER_RADIUS = 2.0;
	
	// Font sizes (scaled)
	static const float FONT_TINY = 10.0;
	static const float FONT_SMALL = 12.0;
	static const float FONT_MEDIUM = 14.0;
	static const float FONT_LARGE = 18.0;
	static const float FONT_XLARGE = 24.0;
	static const float FONT_TITLE = 32.0;
}

class RBL_UITiming
{
	// Animation durations (seconds)
	static const float FADE_FAST = 0.15;
	static const float FADE_NORMAL = 0.25;
	static const float FADE_SLOW = 0.4;
	
	// Toast display time
	static const float TOAST_DURATION = 4.0;
	static const float TOAST_FADE_IN = 0.2;
	static const float TOAST_FADE_OUT = 0.3;
	
	// Update intervals
	static const float HUD_UPDATE_INTERVAL = 0.25;
	static const float ZONE_UPDATE_INTERVAL = 0.5;
	static const float MAP_UPDATE_INTERVAL = 1.0;
	
	// Pulse animation
	static const float PULSE_PERIOD = 1.5;
}

class RBL_UIStrings
{
	// Currency format
	static string FormatMoney(int amount)
	{
		return "$" + amount.ToString();
	}
	
	// HR format
	static string FormatHR(int amount)
	{
		return amount.ToString() + " HR";
	}
	
	// Distance format
	static string FormatDistance(float meters)
	{
		if (meters >= 1000)
			return (meters / 1000.0).ToString(1, 1) + "km";
		return meters.ToString(1, 0) + "m";
	}
	
	// Percentage format
	static string FormatPercent(float value)
	{
		return (value * 100).ToString(1, 0) + "%";
	}
	
	// Time format (seconds to MM:SS)
	static string FormatTime(float seconds)
	{
		int totalSecs = seconds;
		int mins = totalSecs / 60;
		int secs = totalSecs - (mins * 60);
		
		string minStr = mins.ToString();
		string secStr = secs.ToString();
		if (secs < 10)
			secStr = "0" + secStr;
		
		return minStr + ":" + secStr;
	}
	
	// Zone type display names
	static string GetZoneTypeName(ERBLZoneType type)
	{
		switch (type)
		{
			case ERBLZoneType.Town: return "TOWN";
			case ERBLZoneType.Outpost: return "OUTPOST";
			case ERBLZoneType.Airbase: return "AIRBASE";
			case ERBLZoneType.HQ: return "HQ";
			case ERBLZoneType.Factory: return "FACTORY";
			case ERBLZoneType.Resource: return "RESOURCE";
			case ERBLZoneType.Seaport: return "SEAPORT";
			case ERBLZoneType.Roadblock: return "ROADBLOCK";
			case ERBLZoneType.Watchpost: return "WATCHPOST";
		}
		return "UNKNOWN";
	}
	
	// Faction display names
	static string GetFactionName(ERBLFactionKey faction)
	{
		switch (faction)
		{
			case ERBLFactionKey.FIA: return "FIA";
			case ERBLFactionKey.USSR: return "SOVIET";
			case ERBLFactionKey.US: return "US";
		}
		return "NEUTRAL";
	}
	
	// Cover status display names
	static string GetCoverStatusName(ERBLCoverStatus status)
	{
		switch (status)
		{
			case ERBLCoverStatus.HIDDEN: return "HIDDEN";
			case ERBLCoverStatus.SUSPICIOUS: return "SUSPICIOUS";
			case ERBLCoverStatus.SPOTTED: return "SPOTTED";
			case ERBLCoverStatus.COMPROMISED: return "COMPROMISED";
			case ERBLCoverStatus.HOSTILE: return "HOSTILE";
		}
		return "UNKNOWN";
	}
}

// Layout position helpers
class RBL_UILayout
{
	// Get position for corner-anchored elements
	static vector GetCornerPosition(int corner, float width, float height, float margin)
	{
		float x = 0;
		float y = 0;
		
		switch (corner)
		{
			case 0: // Top-left
				x = margin;
				y = margin;
				break;
			case 1: // Top-right
				x = RBL_UISizes.REFERENCE_WIDTH - width - margin;
				y = margin;
				break;
			case 2: // Bottom-left
				x = margin;
				y = RBL_UISizes.REFERENCE_HEIGHT - height - margin;
				break;
			case 3: // Bottom-right
				x = RBL_UISizes.REFERENCE_WIDTH - width - margin;
				y = RBL_UISizes.REFERENCE_HEIGHT - height - margin;
				break;
		}
		
		return Vector(x, y, 0);
	}
	
	// Get centered position
	static vector GetCenteredPosition(float width, float height)
	{
		float x = (RBL_UISizes.REFERENCE_WIDTH - width) / 2;
		float y = (RBL_UISizes.REFERENCE_HEIGHT - height) / 2;
		return Vector(x, y, 0);
	}
	
	// Get bottom-center position
	static vector GetBottomCenterPosition(float width, float height, float margin)
	{
		float x = (RBL_UISizes.REFERENCE_WIDTH - width) / 2;
		float y = RBL_UISizes.REFERENCE_HEIGHT - height - margin;
		return Vector(x, y, 0);
	}
}

