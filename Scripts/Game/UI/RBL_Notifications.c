// ============================================================================
// PROJECT REBELLION - Notifications Helper
// Static helper class for showing common game notifications
// With multiplayer broadcast support
// ============================================================================

class RBL_Notifications
{
	// ========================================================================
	// NETWORK BROADCAST HELPERS
	// ========================================================================
	
	protected static void BroadcastNotification(string message, int color, float duration)
	{
		if (RBL_NetworkUtils.IsServer())
		{
			RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
			if (netMgr)
			{
				netMgr.BroadcastNotification(message, color, duration);
				return;
			}
		}
		
		// Fallback to local notification
		ShowNotification(message, color, duration);
	}
	
	protected static void SendToPlayer(int playerID, string message, int color, float duration)
	{
		if (RBL_NetworkUtils.IsServer())
		{
			RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
			if (netMgr)
			{
				netMgr.SendNotificationToPlayer(playerID, message, color, duration);
				return;
			}
		}
		
		// Fallback to local if we're that player
		if (playerID == RBL_NetworkUtils.GetLocalPlayerID())
			ShowNotification(message, color, duration);
	}
	// ========================================================================
	// GAME STATE NOTIFICATIONS
	// ========================================================================
	
	static void GameSaved()
	{
		ShowNotification("Game Saved", RBL_UIColors.COLOR_ACCENT_GREEN, 2.0);
		PrintFormat("[RBL] Game saved successfully");
	}
	
	static void GameLoaded()
	{
		ShowNotification("Game Loaded", RBL_UIColors.COLOR_ACCENT_GREEN, 2.0);
		PrintFormat("[RBL] Game loaded successfully");
	}
	
	static void AutoSaved()
	{
		ShowNotification("Auto-Save Complete", RBL_UIColors.COLOR_TEXT_SECONDARY, 1.5);
	}
	
	static void SaveFailed(string reason)
	{
		ShowNotification("Save Failed: " + reason, RBL_UIColors.COLOR_ACCENT_RED, 3.0);
		PrintFormat("[RBL] Save failed: %1", reason);
	}
	
	static void LoadFailed(string reason)
	{
		ShowNotification("Load Failed: " + reason, RBL_UIColors.COLOR_ACCENT_RED, 3.0);
		PrintFormat("[RBL] Load failed: %1", reason);
	}
	
	// ========================================================================
	// ZONE NOTIFICATIONS (Broadcast to all)
	// ========================================================================
	
	static void ZoneCaptured(string zoneName)
	{
		BroadcastNotification("ZONE CAPTURED: " + zoneName, RBL_UIColors.COLOR_ACCENT_GREEN, 4.0);
		PrintFormat("[RBL] Zone captured: %1", zoneName);
	}
	
	static void ZoneLost(string zoneName)
	{
		BroadcastNotification("ZONE LOST: " + zoneName, RBL_UIColors.COLOR_ACCENT_RED, 4.0);
		PrintFormat("[RBL] Zone lost: %1", zoneName);
	}
	
	static void ZoneContested(string zoneName)
	{
		BroadcastNotification("Zone Contested: " + zoneName, RBL_UIColors.COLOR_ACCENT_YELLOW, 3.0);
	}
	
	static void CaptureStarted(string zoneName)
	{
		ShowNotification("Capturing: " + zoneName, RBL_UIColors.COLOR_TEXT_PRIMARY, 2.0);
	}
	
	// ========================================================================
	// COMBAT NOTIFICATIONS (Broadcast to all)
	// ========================================================================
	
	static void QRFIncoming(string type, string targetZone)
	{
		BroadcastNotification("ENEMY QRF INCOMING: " + type, RBL_UIColors.COLOR_ACCENT_RED, 5.0);
		PrintFormat("[RBL] QRF incoming: %1 to %2", type, targetZone);
	}
	
	static void QRFDefeated()
	{
		BroadcastNotification("Enemy QRF Eliminated", RBL_UIColors.COLOR_ACCENT_GREEN, 3.0);
	}
	
	static void WarLevelIncreased(int newLevel)
	{
		BroadcastNotification("WAR LEVEL: " + newLevel.ToString(), RBL_UIColors.COLOR_ACCENT_YELLOW, 4.0);
		PrintFormat("[RBL] War level increased to %1", newLevel);
	}
	
	// ========================================================================
	// UNDERCOVER NOTIFICATIONS
	// ========================================================================
	
	static void CoverBlown(string reason)
	{
		ShowNotification("COVER BLOWN: " + reason, RBL_UIColors.COLOR_ACCENT_RED, 4.0);
		PrintFormat("[RBL] Cover blown: %1", reason);
	}
	
	static void SuspicionIncreased()
	{
		ShowNotification("Suspicion Rising...", RBL_UIColors.COLOR_ACCENT_YELLOW, 2.0);
	}
	
	static void CoverRestored()
	{
		ShowNotification("Cover Restored", RBL_UIColors.COLOR_ACCENT_GREEN, 2.0);
	}
	
	// ========================================================================
	// ECONOMY NOTIFICATIONS
	// ========================================================================
	
	static void MoneyReceived(int amount, string source)
	{
		ShowNotification("+$" + amount.ToString() + " (" + source + ")", RBL_UIColors.COLOR_ACCENT_GREEN, 2.0);
	}
	
	static void HRReceived(int amount, string source)
	{
		ShowNotification("+" + amount.ToString() + " HR (" + source + ")", RBL_UIColors.COLOR_ACCENT_GREEN, 2.0);
	}
	
	static void InsufficientFunds()
	{
		ShowNotification("Insufficient Funds", RBL_UIColors.COLOR_ACCENT_RED, 2.0);
	}
	
	static void InsufficientHR()
	{
		ShowNotification("Insufficient Human Resources", RBL_UIColors.COLOR_ACCENT_RED, 2.0);
	}
	
	// ========================================================================
	// SHOP/DELIVERY NOTIFICATIONS
	// ========================================================================
	
	static void ItemPurchased(string itemName, int cost)
	{
		ShowNotification("Purchased: " + itemName + " (-$" + cost.ToString() + ")", RBL_UIColors.COLOR_ACCENT_GREEN, 2.5);
	}
	
	static void ItemDelivered(string itemName)
	{
		ShowNotification("Delivered: " + itemName, RBL_UIColors.COLOR_TEXT_PRIMARY, 2.0);
	}
	
	static void DeliveryFailed(string itemName)
	{
		ShowNotification("Delivery Failed: " + itemName, RBL_UIColors.COLOR_ACCENT_RED, 3.0);
	}
	
	static void RecruitJoined(string type)
	{
		ShowNotification("Recruit Joined: " + type, RBL_UIColors.COLOR_ACCENT_GREEN, 2.5);
	}
	
	static void VehicleDelivered(string type)
	{
		ShowNotification("Vehicle Ready: " + type, RBL_UIColors.COLOR_ACCENT_GREEN, 2.5);
	}
	
	// ========================================================================
	// MISSION NOTIFICATIONS (Broadcast to all)
	// ========================================================================
	
	static void MissionReceived(string missionName)
	{
		BroadcastNotification("NEW MISSION: " + missionName, RBL_UIColors.COLOR_ACCENT_BLUE, 4.0);
	}
	
	static void MissionComplete(string missionName)
	{
		BroadcastNotification("MISSION COMPLETE: " + missionName, RBL_UIColors.COLOR_ACCENT_GREEN, 4.0);
	}
	
	static void MissionFailed(string missionName)
	{
		BroadcastNotification("MISSION FAILED: " + missionName, RBL_UIColors.COLOR_ACCENT_RED, 4.0);
	}
	
	// ========================================================================
	// SYSTEM NOTIFICATIONS
	// ========================================================================
	
	static void SystemMessage(string message)
	{
		ShowNotification(message, RBL_UIColors.COLOR_TEXT_SECONDARY, 3.0);
	}
	
	static void ErrorMessage(string message)
	{
		ShowNotification("ERROR: " + message, RBL_UIColors.COLOR_ACCENT_RED, 4.0);
		PrintFormat("[RBL] Error: %1", message);
	}
	
	static void DebugMessage(string message)
	{
		ShowNotification("[DEBUG] " + message, RBL_UIColors.COLOR_TEXT_MUTED, 2.0);
		PrintFormat("[RBL_DEBUG] %1", message);
	}
	
	// ========================================================================
	// CORE NOTIFICATION METHOD
	// ========================================================================
	
	protected static void ShowNotification(string message, int color, float duration)
	{
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ShowNotification(message, color, duration);
		}
		else
		{
			PrintFormat("[RBL_Notification] %1", message);
		}
	}
}

// ============================================================================
// NOTIFICATION QUEUE ITEM
// ============================================================================
class RBL_NotificationItem
{
	string m_sMessage;
	int m_iColor;
	float m_fDuration;
	float m_fTimeRemaining;
	float m_fAlpha;
	bool m_bFadingOut;
	
	void RBL_NotificationItem(string message, int color, float duration)
	{
		m_sMessage = message;
		m_iColor = color;
		m_fDuration = duration;
		m_fTimeRemaining = duration;
		m_fAlpha = 0;
		m_bFadingOut = false;
	}
	
	void Update(float timeSlice)
	{
		// Fade in
		if (!m_bFadingOut && m_fAlpha < 1.0)
		{
			m_fAlpha += timeSlice * 4.0;
			if (m_fAlpha > 1.0)
				m_fAlpha = 1.0;
		}
		
		// Duration countdown
		m_fTimeRemaining -= timeSlice;
		
		// Start fade out
		if (m_fTimeRemaining <= RBL_UITiming.FADE_FAST && !m_bFadingOut)
		{
			m_bFadingOut = true;
		}
		
		// Fade out
		if (m_bFadingOut)
		{
			m_fAlpha -= timeSlice * 2.0;
			if (m_fAlpha < 0)
				m_fAlpha = 0;
		}
	}
	
	bool IsExpired()
	{
		return m_fTimeRemaining <= 0 && m_fAlpha <= 0;
	}
	
	string GetMessage() { return m_sMessage; }
	int GetColor() { return m_iColor; }
	float GetAlpha() { return m_fAlpha; }
}

