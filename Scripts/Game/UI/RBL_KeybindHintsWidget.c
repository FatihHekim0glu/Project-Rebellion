// ============================================================================
// PROJECT REBELLION - Keybind Hints Widget
// Bottom-right contextual keybind hints
// ============================================================================

// Single keybind hint entry
class RBL_KeybindHint
{
	string m_sKey;        // Display key (e.g., "F", "ESC", "TAB")
	string m_sAction;     // Action description
	int m_iPriority;      // Display priority (higher = more important)
	bool m_bVisible;      // Currently visible
	float m_fAlpha;       // Current alpha (for fade)
	
	void RBL_KeybindHint(string key, string action, int priority)
	{
		m_sKey = key;
		m_sAction = action;
		m_iPriority = priority;
		m_bVisible = true;
		m_fAlpha = 0;
	}
}

// Keybind hints manager
class RBL_KeybindHintsWidgetImpl : RBL_BaseWidget
{
	protected ref array<ref RBL_KeybindHint> m_aHints;
	protected ref array<ref RBL_KeybindHint> m_aContextHints;  // Temporary context-sensitive hints
	protected int m_iMaxVisible;
	
	void RBL_KeybindHintsWidgetImpl()
	{
		// Position in bottom-right
		m_fPosX = RBL_UISizes.REFERENCE_WIDTH - 300;
		m_fPosY = RBL_UISizes.REFERENCE_HEIGHT - 150;
		m_fWidth = 280;
		m_fHeight = 130;
		
		m_aHints = new array<ref RBL_KeybindHint>();
		m_aContextHints = new array<ref RBL_KeybindHint>();
		m_iMaxVisible = 5;
		
		m_fUpdateInterval = 0.1;
		
		// Initialize default hints
		InitializeDefaultHints();
	}
	
	protected void InitializeDefaultHints()
	{
		// Always visible hints
		m_aHints.Insert(new RBL_KeybindHint("B", "Shop", 10));
		m_aHints.Insert(new RBL_KeybindHint("M", "Map", 9));
		m_aHints.Insert(new RBL_KeybindHint("H", "Toggle HUD", 5));
	}
	
	override void OnUpdate()
	{
		// Clear and rebuild context hints based on player state
		m_aContextHints.Clear();
		
		// Get player state
		IEntity player = GetLocalPlayer();
		if (!player)
			return;
		
		vector playerPos = player.GetOrigin();
		
		// Check if near virtual zone
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			RBL_VirtualZone nearestZone = zoneMgr.GetNearestVirtualZone(playerPos);
			if (nearestZone)
			{
				float dist = vector.Distance(playerPos, nearestZone.GetZonePosition());
				
				if (dist <= nearestZone.GetCaptureRadius())
				{
					// In zone - show capture hint
					if (nearestZone.GetOwnerFaction() != ERBLFactionKey.FIA)
					{
						if (nearestZone.GetGarrisonStrength() > 0)
						{
							m_aContextHints.Insert(new RBL_KeybindHint("!", "Clear Garrison First", 20));
						}
						else
						{
							m_aContextHints.Insert(new RBL_KeybindHint("STAY", "Capture Zone", 20));
						}
					}
					else
					{
						m_aContextHints.Insert(new RBL_KeybindHint("E", "Zone Actions", 15));
					}
				}
			}
		}
		
		// Check undercover state
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
		{
			RBL_PlayerCoverState playerState = undercover.GetPlayerState(player);
			if (playerState)
			{
				ERBLCoverStatus status = playerState.GetStatus();
				
				if (status == ERBLCoverStatus.SUSPICIOUS || status == ERBLCoverStatus.SPOTTED)
				{
					m_aContextHints.Insert(new RBL_KeybindHint("HOLSTER", "Lower Suspicion", 18));
				}
				else if (status == ERBLCoverStatus.COMPROMISED || status == ERBLCoverStatus.HOSTILE)
				{
					m_aContextHints.Insert(new RBL_KeybindHint("!", "Evade or Fight", 18));
				}
			}
		}
		
		// Check if in combat (high aggression)
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr && campaignMgr.GetAggression() > 50)
		{
			m_aContextHints.Insert(new RBL_KeybindHint("T", "Tactical View", 12));
		}
		
		// Check if shop is available
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr && econMgr.GetMoney() > 0)
		{
			// Shop is available
		}
	}
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		// Fade in/out hints
		UpdateHintAlphas(m_aHints, timeSlice);
		UpdateHintAlphas(m_aContextHints, timeSlice);
	}
	
	protected void UpdateHintAlphas(array<ref RBL_KeybindHint> hints, float timeSlice)
	{
		float fadeSpeed = 4.0;
		
		for (int i = 0; i < hints.Count(); i++)
		{
			RBL_KeybindHint hint = hints[i];
			
			if (hint.m_bVisible)
			{
				hint.m_fAlpha += fadeSpeed * timeSlice;
				if (hint.m_fAlpha > 1.0)
					hint.m_fAlpha = 1.0;
			}
			else
			{
				hint.m_fAlpha -= fadeSpeed * timeSlice;
				if (hint.m_fAlpha < 0)
					hint.m_fAlpha = 0;
			}
		}
	}
	
	override void Draw()
	{
		if (!IsVisible())
			return;
		
		// Combine and sort hints by priority
		ref array<ref RBL_KeybindHint> allHints = new array<ref RBL_KeybindHint>();
		
		// Add context hints first (higher priority)
		for (int i = 0; i < m_aContextHints.Count(); i++)
			allHints.Insert(m_aContextHints[i]);
		
		// Add default hints
		for (int i = 0; i < m_aHints.Count(); i++)
			allHints.Insert(m_aHints[i]);
		
		// Sort by priority (descending)
		SortHintsByPriority(allHints);
		
		// Draw hints
		float x = m_fPosX;
		float y = m_fPosY;
		float rowHeight = 24;
		int visibleCount = 0;
		
		for (int i = 0; i < allHints.Count() && visibleCount < m_iMaxVisible; i++)
		{
			RBL_KeybindHint hint = allHints[i];
			if (hint.m_fAlpha <= 0.01)
				continue;
			
			DrawHintRow(hint, x, y + visibleCount * rowHeight);
			visibleCount++;
		}
	}
	
	protected void DrawHintRow(RBL_KeybindHint hint, float x, float y)
	{
		float alpha = hint.m_fAlpha * m_fAlpha;
		float keyBoxWidth = 50;
		float keyBoxHeight = 20;
		
		// Key box background
		int keyBgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_MEDIUM, alpha);
		DrawRect(x, y, keyBoxWidth, keyBoxHeight, keyBgColor);
		
		// Key box border
		int keyBorderColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_LIGHT, alpha);
		DrawRectOutline(x, y, keyBoxWidth, keyBoxHeight, keyBorderColor, 1);
		
		// Key text (centered)
		int keyColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, alpha);
		float keyTextX = x + (keyBoxWidth - hint.m_sKey.Length() * 6) / 2;
		
		DbgUI.Begin("Hint_Key_" + hint.m_sKey, keyTextX, y + 3);
		DbgUI.Text(hint.m_sKey);
		DbgUI.End();
		
		// Action text
		int actionColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, alpha);
		
		DbgUI.Begin("Hint_Action_" + hint.m_sKey, x + keyBoxWidth + 10, y + 3);
		DbgUI.Text(hint.m_sAction);
		DbgUI.End();
	}
	
	protected void SortHintsByPriority(array<ref RBL_KeybindHint> hints)
	{
		// Simple bubble sort (small array)
		for (int i = 0; i < hints.Count() - 1; i++)
		{
			for (int j = 0; j < hints.Count() - i - 1; j++)
			{
				if (hints[j].m_iPriority < hints[j + 1].m_iPriority)
				{
					RBL_KeybindHint temp = hints[j];
					hints[j] = hints[j + 1];
					hints[j + 1] = temp;
				}
			}
		}
	}
	
	// Manual hint management
	void AddHint(string key, string action, int priority)
	{
		// Check if already exists
		for (int i = 0; i < m_aHints.Count(); i++)
		{
			if (m_aHints[i].m_sKey == key)
			{
				m_aHints[i].m_sAction = action;
				m_aHints[i].m_iPriority = priority;
				m_aHints[i].m_bVisible = true;
				return;
			}
		}
		
		m_aHints.Insert(new RBL_KeybindHint(key, action, priority));
	}
	
	void RemoveHint(string key)
	{
		for (int i = m_aHints.Count() - 1; i >= 0; i--)
		{
			if (m_aHints[i].m_sKey == key)
			{
				m_aHints[i].m_bVisible = false;
				return;
			}
		}
	}
	
	void ShowContextHint(string key, string action, int priority)
	{
		RBL_KeybindHint hint = new RBL_KeybindHint(key, action, priority);
		hint.m_fAlpha = 1.0; // Immediately visible
		m_aContextHints.Insert(hint);
	}
	
	void ClearContextHints()
	{
		m_aContextHints.Clear();
	}
	
	// Helpers
	protected IEntity GetLocalPlayer()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;
		return pc.GetControlledEntity();
	}
}

