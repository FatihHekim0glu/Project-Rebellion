// ============================================================================
// PROJECT REBELLION - Notification Toast System
// Right-side toast messages for game events
// ============================================================================

// Single notification entry
class RBL_NotificationEntry
{
	string m_sMessage;
	int m_iColor;
	float m_fDuration;
	float m_fLifetime;
	float m_fAlpha;
	int m_iPriority;
	
	void RBL_NotificationEntry(string message, int color, float duration, int priority)
	{
		m_sMessage = message;
		m_iColor = color;
		m_fDuration = duration;
		m_fLifetime = 0;
		m_fAlpha = 0;
		m_iPriority = priority;
	}
	
	bool IsExpired()
	{
		return m_fLifetime >= m_fDuration;
	}
	
	float GetFadeProgress()
	{
		// Fade in during first 20%, fade out during last 20%
		float fadeInEnd = RBL_UITiming.TOAST_FADE_IN;
		float fadeOutStart = m_fDuration - RBL_UITiming.TOAST_FADE_OUT;
		
		if (m_fLifetime < fadeInEnd)
			return m_fLifetime / fadeInEnd;
		else if (m_fLifetime > fadeOutStart)
			return (m_fDuration - m_fLifetime) / RBL_UITiming.TOAST_FADE_OUT;
		
		return 1.0;
	}
}

// Notification manager
class RBL_NotificationManagerImpl : RBL_BaseWidget
{
	protected ref array<ref RBL_NotificationEntry> m_aNotifications;
	protected int m_iMaxVisible;
	protected float m_fBaseY;
	
	void RBL_NotificationManagerImpl()
	{
		// Position on right side
		m_fPosX = RBL_UISizes.REFERENCE_WIDTH - RBL_UISizes.TOAST_WIDTH - RBL_UISizes.HUD_MARGIN;
		m_fPosY = RBL_UISizes.REFERENCE_HEIGHT / 3; // Upper third
		m_fWidth = RBL_UISizes.TOAST_WIDTH;
		m_fHeight = RBL_UISizes.TOAST_HEIGHT * 5; // Space for up to 5 toasts
		
		m_aNotifications = new array<ref RBL_NotificationEntry>();
		m_iMaxVisible = 5;
		m_fBaseY = m_fPosY;
		
		m_fUpdateInterval = 0.016; // 60fps for smooth animations
	}
	
	void ShowNotification(string message, int color, float duration)
	{
		ShowNotificationWithPriority(message, color, duration, 0);
	}
	
	void ShowNotificationWithPriority(string message, int color, float duration, int priority)
	{
		// Check for duplicate messages
		for (int i = 0; i < m_aNotifications.Count(); i++)
		{
			if (m_aNotifications[i].m_sMessage == message && !m_aNotifications[i].IsExpired())
			{
				// Reset existing notification instead
				m_aNotifications[i].m_fLifetime = 0;
				return;
			}
		}
		
		// Create new notification
		RBL_NotificationEntry entry = new RBL_NotificationEntry(message, color, duration, priority);
		m_aNotifications.Insert(entry);
		
		// Sort by priority (higher priority at top)
		SortByPriority();
		
		// Limit visible notifications
		while (m_aNotifications.Count() > m_iMaxVisible)
		{
			// Remove oldest low-priority notification
			int removeIdx = FindLowestPriorityIndex();
			m_aNotifications.Remove(removeIdx);
		}
		
		PrintFormat("[RBL_UI] Notification: %1", message);
	}
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		// Update all notifications
		for (int i = m_aNotifications.Count() - 1; i >= 0; i--)
		{
			RBL_NotificationEntry entry = m_aNotifications[i];
			entry.m_fLifetime += timeSlice;
			entry.m_fAlpha = entry.GetFadeProgress();
			
			// Remove expired
			if (entry.IsExpired())
			{
				m_aNotifications.Remove(i);
			}
		}
	}
	
	override void Draw()
	{
		if (!IsVisible() || m_aNotifications.IsEmpty())
			return;
		
		float currentY = m_fBaseY;
		
		for (int i = 0; i < m_aNotifications.Count(); i++)
		{
			RBL_NotificationEntry entry = m_aNotifications[i];
			if (entry.m_fAlpha <= 0.01)
				continue;
			
			DrawNotificationEntry(entry, m_fPosX, currentY);
			currentY += RBL_UISizes.TOAST_HEIGHT + RBL_UISizes.TOAST_MARGIN;
		}
	}
	
	protected void DrawNotificationEntry(RBL_NotificationEntry entry, float x, float y)
	{
		float width = RBL_UISizes.TOAST_WIDTH;
		float height = RBL_UISizes.TOAST_HEIGHT;
		float alpha = entry.m_fAlpha * m_fAlpha;
		
		// Slide-in animation
		float slideOffset = (1.0 - entry.m_fAlpha) * 50; // Slide from right
		x += slideOffset;
		
		// Background
		int bgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_DARK, alpha * 0.9);
		DrawRect(x, y, width, height, bgColor);
		
		// Left accent bar (colored by priority/type)
		int accentColor = ApplyAlpha(entry.m_iColor, alpha);
		DrawRect(x, y, 4, height, accentColor);
		
		// Border
		int borderColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, alpha);
		DrawRectOutline(x, y, width, height, borderColor, 1);
		
		// Message text
		int textColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, alpha);
		float textX = x + 12;
		float textY = y + height / 2 - 8;
		
		DbgUI.Begin("Notif_" + entry.m_fLifetime.ToString(), textX, textY);
		DbgUI.Text(entry.m_sMessage);
		DbgUI.End();
	}
	
	protected void SortByPriority()
	{
		// Simple bubble sort (small array)
		for (int i = 0; i < m_aNotifications.Count() - 1; i++)
		{
			for (int j = 0; j < m_aNotifications.Count() - i - 1; j++)
			{
				if (m_aNotifications[j].m_iPriority < m_aNotifications[j + 1].m_iPriority)
				{
					// Swap
					RBL_NotificationEntry temp = m_aNotifications[j];
					m_aNotifications[j] = m_aNotifications[j + 1];
					m_aNotifications[j + 1] = temp;
				}
			}
		}
	}
	
	protected int FindLowestPriorityIndex()
	{
		int lowestIdx = 0;
		int lowestPriority = 9999;
		float oldestTime = 0;
		
		for (int i = 0; i < m_aNotifications.Count(); i++)
		{
			RBL_NotificationEntry entry = m_aNotifications[i];
			
			// Prefer removing lower priority and older notifications
			if (entry.m_iPriority < lowestPriority || 
				(entry.m_iPriority == lowestPriority && entry.m_fLifetime > oldestTime))
			{
				lowestIdx = i;
				lowestPriority = entry.m_iPriority;
				oldestTime = entry.m_fLifetime;
			}
		}
		
		return lowestIdx;
	}
	
	// Clear all notifications
	void ClearAll()
	{
		m_aNotifications.Clear();
	}
	
	// Get active count
	int GetActiveCount()
	{
		return m_aNotifications.Count();
	}
}

// ============================================================================
// Note: Helper methods for notifications are in RBL_Notifications.c
// Usage: RBL_Notifications.ZoneCaptured("Morton"), etc.
// ============================================================================

