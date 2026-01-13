// ============================================================================
// PROJECT REBELLION - Base UI Widget Classes
// Foundation components for building the UI system
// ============================================================================

// ============================================================================
// UI MANAGER - Central UI coordinator
// ============================================================================
class RBL_UIManager
{
	protected static ref RBL_UIManager s_Instance;
	
	protected ref RBL_MainHUD m_MainHUD;
	protected ref RBL_ZoneInfoWidget m_ZoneInfo;
	protected ref RBL_CaptureBarWidget m_CaptureBar;
	protected ref RBL_UndercoverWidget m_UndercoverIndicator;
	protected ref RBL_NotificationManager m_Notifications;
	protected ref RBL_ShopMenuWidget m_ShopMenu;
	protected ref RBL_KeybindHints m_KeybindHints;
	
	protected bool m_bInitialized;
	protected bool m_bVisible;
	
	static RBL_UIManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_UIManager();
		return s_Instance;
	}
	
	void RBL_UIManager()
	{
		m_bInitialized = false;
		m_bVisible = true;
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		PrintFormat("[RBL_UI] Initializing UI Manager...");
		
		// Create UI components
		m_MainHUD = new RBL_MainHUD();
		m_ZoneInfo = new RBL_ZoneInfoWidget();
		m_CaptureBar = new RBL_CaptureBarWidget();
		m_UndercoverIndicator = new RBL_UndercoverWidget();
		m_Notifications = new RBL_NotificationManager();
		m_ShopMenu = new RBL_ShopMenuWidget();
		m_KeybindHints = new RBL_KeybindHints();
		
		m_bInitialized = true;
		PrintFormat("[RBL_UI] UI Manager initialized");
	}
	
	void Update(float timeSlice)
	{
		if (!m_bInitialized || !m_bVisible)
			return;
		
		// Update all components
		if (m_MainHUD)
			m_MainHUD.Update(timeSlice);
		
		if (m_ZoneInfo)
			m_ZoneInfo.Update(timeSlice);
		
		if (m_CaptureBar)
			m_CaptureBar.Update(timeSlice);
		
		if (m_UndercoverIndicator)
			m_UndercoverIndicator.Update(timeSlice);
		
		if (m_Notifications)
			m_Notifications.Update(timeSlice);
		
		if (m_KeybindHints)
			m_KeybindHints.Update(timeSlice);
	}
	
	void Draw()
	{
		if (!m_bInitialized || !m_bVisible)
			return;
		
		// Draw all visible components
		if (m_MainHUD)
			m_MainHUD.Draw();
		
		if (m_ZoneInfo)
			m_ZoneInfo.Draw();
		
		if (m_CaptureBar)
			m_CaptureBar.Draw();
		
		if (m_UndercoverIndicator)
			m_UndercoverIndicator.Draw();
		
		if (m_Notifications)
			m_Notifications.Draw();
		
		if (m_ShopMenu && m_ShopMenu.IsVisible())
			m_ShopMenu.Draw();
		
		if (m_KeybindHints)
			m_KeybindHints.Draw();
	}
	
	void ToggleShop()
	{
		if (m_ShopMenu)
			m_ShopMenu.Toggle();
	}
	
	void ShowNotification(string message, int color, float duration)
	{
		if (m_Notifications)
			m_Notifications.ShowNotification(message, color, duration);
	}
	
	void SetVisible(bool visible) { m_bVisible = visible; }
	bool IsVisible() { return m_bVisible; }
	
	RBL_MainHUD GetMainHUD() { return m_MainHUD; }
	RBL_ZoneInfoWidget GetZoneInfo() { return m_ZoneInfo; }
	RBL_CaptureBarWidget GetCaptureBar() { return m_CaptureBar; }
	RBL_UndercoverWidget GetUndercoverIndicator() { return m_UndercoverIndicator; }
	RBL_NotificationManager GetNotifications() { return m_Notifications; }
	RBL_ShopMenuWidget GetShopMenu() { return m_ShopMenu; }
}

// ============================================================================
// BASE WIDGET - Foundation for all UI widgets
// ============================================================================
class RBL_BaseWidget
{
	protected bool m_bVisible;
	protected bool m_bEnabled;
	protected float m_fAlpha;
	protected float m_fTargetAlpha;
	protected float m_fPosX;
	protected float m_fPosY;
	protected float m_fWidth;
	protected float m_fHeight;
	protected float m_fUpdateTimer;
	protected float m_fUpdateInterval;
	
	void RBL_BaseWidget()
	{
		m_bVisible = true;
		m_bEnabled = true;
		m_fAlpha = 1.0;
		m_fTargetAlpha = 1.0;
		m_fPosX = 0;
		m_fPosY = 0;
		m_fWidth = 100;
		m_fHeight = 100;
		m_fUpdateTimer = 0;
		m_fUpdateInterval = RBL_UITiming.HUD_UPDATE_INTERVAL;
	}
	
	void Update(float timeSlice)
	{
		// Animate alpha
		if (m_fAlpha != m_fTargetAlpha)
		{
			float speed = 1.0 / RBL_UITiming.FADE_NORMAL;
			if (m_fAlpha < m_fTargetAlpha)
			{
				m_fAlpha += speed * timeSlice;
				if (m_fAlpha > m_fTargetAlpha)
					m_fAlpha = m_fTargetAlpha;
			}
			else
			{
				m_fAlpha -= speed * timeSlice;
				if (m_fAlpha < m_fTargetAlpha)
					m_fAlpha = m_fTargetAlpha;
			}
		}
		
		// Update timer
		m_fUpdateTimer += timeSlice;
		if (m_fUpdateTimer >= m_fUpdateInterval)
		{
			m_fUpdateTimer = 0;
			OnUpdate();
		}
	}
	
	void OnUpdate()
	{
		// Override in subclasses to refresh data
	}
	
	void Draw()
	{
		// Override in subclasses
	}
	
	void Show()
	{
		m_bVisible = true;
		m_fTargetAlpha = 1.0;
	}
	
	void Hide()
	{
		m_fTargetAlpha = 0.0;
	}
	
	void SetPosition(float x, float y)
	{
		m_fPosX = x;
		m_fPosY = y;
	}
	
	void SetSize(float width, float height)
	{
		m_fWidth = width;
		m_fHeight = height;
	}
	
	bool IsVisible() { return m_bVisible && m_fAlpha > 0.01; }
	bool IsEnabled() { return m_bEnabled; }
	float GetAlpha() { return m_fAlpha; }
}

// ============================================================================
// PANEL WIDGET - Base for rectangular panels with background
// ============================================================================
class RBL_PanelWidget : RBL_BaseWidget
{
	protected int m_iBackgroundColor;
	protected int m_iBorderColor;
	protected float m_fPadding;
	protected bool m_bShowBorder;
	
	void RBL_PanelWidget()
	{
		m_iBackgroundColor = RBL_UIColors.COLOR_BG_DARK;
		m_iBorderColor = RBL_UIColors.COLOR_BORDER_DARK;
		m_fPadding = RBL_UISizes.HUD_PADDING;
		m_bShowBorder = true;
	}
	
	override void Draw()
	{
		if (!IsVisible())
			return;
		
		DrawBackground();
		DrawContent();
		
		if (m_bShowBorder)
			DrawBorder();
	}
	
	protected void DrawBackground()
	{
		int color = ApplyAlpha(m_iBackgroundColor, m_fAlpha);
		DrawRect(m_fPosX, m_fPosY, m_fWidth, m_fHeight, color);
	}
	
	protected void DrawBorder()
	{
		int color = ApplyAlpha(m_iBorderColor, m_fAlpha);
		DrawRectOutline(m_fPosX, m_fPosY, m_fWidth, m_fHeight, color, RBL_UISizes.BORDER_WIDTH);
	}
	
	protected void DrawContent()
	{
		// Override in subclasses
	}
	
	void SetBackgroundColor(int color) { m_iBackgroundColor = color; }
	void SetBorderColor(int color) { m_iBorderColor = color; }
	void SetPadding(float padding) { m_fPadding = padding; }
	void SetShowBorder(bool show) { m_bShowBorder = show; }
	
	// Get content area (inside padding)
	float GetContentX() { return m_fPosX + m_fPadding; }
	float GetContentY() { return m_fPosY + m_fPadding; }
	float GetContentWidth() { return m_fWidth - m_fPadding * 2; }
	float GetContentHeight() { return m_fHeight - m_fPadding * 2; }
}

// ============================================================================
// DRAWING HELPERS - DbgUI wrappers with color support
// ============================================================================

// Apply alpha to a color
int ApplyAlpha(int color, float alpha)
{
	int a = (color >> 24) & 0xFF;
	int r = (color >> 16) & 0xFF;
	int g = (color >> 8) & 0xFF;
	int b = color & 0xFF;
	
	a = Math.Round(a * alpha);
	
	return (a << 24) | (r << 16) | (g << 8) | b;
}

// Extract color components
void ExtractRGBA(int color, out int r, out int g, out int b, out int a)
{
	a = (color >> 24) & 0xFF;
	r = (color >> 16) & 0xFF;
	g = (color >> 8) & 0xFF;
	b = color & 0xFF;
}

// Create ARGB color
int ARGB(int a, int r, int g, int b)
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}

// Draw filled rectangle
void DrawRect(float x, float y, float w, float h, int color)
{
	int r, g, b, a;
	ExtractRGBA(color, r, g, b, a);
	
	// Use DbgUI for now - will be replaced with Canvas when available
	// This is a placeholder that creates visual output
	DbgUI.Begin("RBL_Rect_" + x.ToString() + "_" + y.ToString(), x, y);
	DbgUI.Panel(color, w, h);
	DbgUI.End();
}

// Draw rectangle outline
void DrawRectOutline(float x, float y, float w, float h, int color, float thickness)
{
	// Top
	DrawRect(x, y, w, thickness, color);
	// Bottom
	DrawRect(x, y + h - thickness, w, thickness, color);
	// Left
	DrawRect(x, y, thickness, h, color);
	// Right
	DrawRect(x + w - thickness, y, thickness, h, color);
}

// Draw text at position
void DrawText(float x, float y, string text, int color, float fontSize)
{
	int r, g, b, a;
	ExtractRGBA(color, r, g, b, a);
	
	DbgUI.Begin("RBL_Text_" + x.ToString() + "_" + y.ToString(), x, y);
	DbgUI.Text(text);
	DbgUI.End();
}

// Draw progress bar
void DrawProgressBar(float x, float y, float w, float h, float progress, int bgColor, int fillColor)
{
	// Background
	DrawRect(x, y, w, h, bgColor);
	
	// Fill (clamped progress)
	float fillWidth = Math.Clamp(progress, 0, 1) * w;
	if (fillWidth > 0)
		DrawRect(x, y, fillWidth, h, fillColor);
}

// ============================================================================
// PLACEHOLDER WIDGET CLASSES (Will be implemented in separate files)
// ============================================================================

class RBL_MainHUD : RBL_PanelWidget
{
	void RBL_MainHUD()
	{
		SetPosition(RBL_UISizes.HUD_MARGIN, RBL_UISizes.HUD_MARGIN);
		SetSize(RBL_UISizes.HUD_PANEL_WIDTH, RBL_UISizes.HUD_PANEL_HEIGHT);
	}
	
	override void OnUpdate() {}
	override void Draw() {}
}

class RBL_ZoneInfoWidget : RBL_PanelWidget
{
	void RBL_ZoneInfoWidget() {}
	override void OnUpdate() {}
	override void Draw() {}
}

class RBL_CaptureBarWidget : RBL_BaseWidget
{
	void RBL_CaptureBarWidget() {}
	override void OnUpdate() {}
	override void Draw() {}
}

class RBL_UndercoverWidget : RBL_BaseWidget
{
	void RBL_UndercoverWidget() {}
	override void OnUpdate() {}
	override void Draw() {}
}

class RBL_NotificationManager : RBL_BaseWidget
{
	void RBL_NotificationManager() {}
	void ShowNotification(string msg, int color, float duration) {}
	override void OnUpdate() {}
	override void Draw() {}
}

class RBL_ShopMenuWidget : RBL_PanelWidget
{
	protected bool m_bMenuVisible;
	
	void RBL_ShopMenuWidget()
	{
		m_bMenuVisible = false;
	}
	
	void Toggle()
	{
		m_bMenuVisible = !m_bMenuVisible;
	}
	
	override bool IsVisible() { return m_bMenuVisible; }
	override void OnUpdate() {}
	override void Draw() {}
}

class RBL_KeybindHints : RBL_BaseWidget
{
	void RBL_KeybindHints() {}
	override void OnUpdate() {}
	override void Draw() {}
}

