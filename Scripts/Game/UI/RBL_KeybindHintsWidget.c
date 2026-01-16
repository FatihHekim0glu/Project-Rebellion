// ============================================================================
// PROJECT REBELLION - Keybind Hints Widget
// Bottom-right contextual keybind hints with dynamic key lookup
// ============================================================================

// Single keybind hint entry
class RBL_KeybindHint
{
	string m_sKey;        // Display key (e.g., "F", "ESC", "TAB")
	string m_sAction;     // Action description
	string m_sActionName; // Internal action name for dynamic key lookup
	int m_iPriority;      // Display priority (higher = more important)
	bool m_bVisible;      // Currently visible
	float m_fAlpha;       // Current alpha (for fade)
	bool m_bDynamic;      // If true, key is looked up dynamically
	
	void RBL_KeybindHint(string key, string action, int priority, string actionName = "")
	{
		m_sKey = key;
		m_sAction = action;
		m_sActionName = actionName;
		m_iPriority = priority;
		m_bVisible = true;
		m_fAlpha = 0;
		m_bDynamic = (actionName.Length() > 0);
	}
	
	string GetDisplayKey()
	{
		if (!m_bDynamic)
			return m_sKey;
		
		// Look up current key from input system
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (registry && registry.IsInitialized())
		{
			string dynamicKey = registry.GetKeyDisplayForAction(m_sActionName);
			if (dynamicKey != "?")
				return dynamicKey;
		}
		
		// Fallback to static key
		return m_sKey;
	}
}

// Keybind hints manager
class RBL_KeybindHintsWidgetImpl : RBL_BaseWidget
{
	protected ref array<ref RBL_KeybindHint> m_aHints;
	protected ref array<ref RBL_KeybindHint> m_aContextHints;
	protected int m_iMaxVisible;
	protected bool m_bInputSystemReady;
	
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
		m_bInputSystemReady = false;
		
		m_fUpdateInterval = 0.1;
		
		// Initialize default hints
		InitializeDefaultHints();
	}
	
	protected void InitializeDefaultHints()
	{
		// Use dynamic hints that look up keys from the input system
		// Format: RBL_KeybindHint(fallbackKey, description, priority, actionName)
		
		m_aHints.Insert(new RBL_KeybindHint("F7", "Shop", 10, RBL_InputActions.TOGGLE_SHOP));
		m_aHints.Insert(new RBL_KeybindHint("M", "Map", 9, RBL_InputActions.TOGGLE_MAP));
		m_aHints.Insert(new RBL_KeybindHint("H", "Toggle HUD", 5, RBL_InputActions.TOGGLE_HUD));
		m_aHints.Insert(new RBL_KeybindHint("L", "Missions", 6, RBL_InputActions.TOGGLE_MISSIONS));
		m_aHints.Insert(new RBL_KeybindHint("F8", "Settings", 8, RBL_InputActions.TOGGLE_SETTINGS));
	}
	
	protected void CheckInputSystemReady()
	{
		if (m_bInputSystemReady)
			return;
		
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (registry && registry.IsInitialized())
		{
			m_bInputSystemReady = true;
			PrintFormat("[RBL_KeybindHints] Input system ready, using dynamic keys");
		}
	}
	
	override void OnUpdate()
	{
		// Check if input system is available
		CheckInputSystemReady();
		
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
						// Dynamic interact key
						m_aContextHints.Insert(new RBL_KeybindHint("F", "Zone Actions", 15, RBL_InputActions.INTERACT));
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
		
		// Check if menu is open - show close hint
		RBL_InputManager inputMgr = RBL_InputManager.GetInstance();
		if (inputMgr && inputMgr.IsMenuOpen())
		{
			m_aContextHints.Insert(new RBL_KeybindHint("ESC", "Close Menu", 25, RBL_InputActions.CLOSE_MENU));
		}
		
		// Check if in combat (high aggression)
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr && campaignMgr.GetAggression() > 50)
		{
			m_aContextHints.Insert(new RBL_KeybindHint("T", "Tactical View", 12));
		}
		
		// Show save/load hints when appropriate
		if (campaignMgr && campaignMgr.GetAggression() < 25)
		{
			// Safe to save
			m_aContextHints.Insert(new RBL_KeybindHint("F5", "Quick Save", 8, RBL_InputActions.QUICK_SAVE));
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
		
		// Get dynamic key display
		string displayKey = hint.GetDisplayKey();
		
		// Adjust box width for longer key names
		if (displayKey.Length() > 3)
			keyBoxWidth = 60;
		if (displayKey.Length() > 5)
			keyBoxWidth = 80;
		
		// Key box background
		int keyBgColor = ApplyAlpha(RBL_UIColors.COLOR_BG_MEDIUM, alpha);
		DrawRect(x, y, keyBoxWidth, keyBoxHeight, keyBgColor);
		
		// Key box border
		int keyBorderColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_LIGHT, alpha);
		DrawRectOutline(x, y, keyBoxWidth, keyBoxHeight, keyBorderColor, 1);
		
		// Key text (centered)
		int keyColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, alpha);
		float keyTextX = x + (keyBoxWidth - displayKey.Length() * 6) / 2;
		
		DbgUI.Begin("Hint_Key_" + displayKey + hint.m_sAction, keyTextX, y + 3);
		DbgUI.Text(displayKey);
		DbgUI.End();
		
		// Action text
		int actionColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, alpha);
		
		DbgUI.Begin("Hint_Action_" + displayKey + hint.m_sAction, x + keyBoxWidth + 10, y + 3);
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
	
	// Manual hint management with action support
	void AddHint(string key, string action, int priority, string actionName = "")
	{
		// Check if already exists
		for (int i = 0; i < m_aHints.Count(); i++)
		{
			if (m_aHints[i].m_sAction == action)
			{
				m_aHints[i].m_sKey = key;
				m_aHints[i].m_sActionName = actionName;
				m_aHints[i].m_iPriority = priority;
				m_aHints[i].m_bVisible = true;
				m_aHints[i].m_bDynamic = (actionName.Length() > 0);
				return;
			}
		}
		
		m_aHints.Insert(new RBL_KeybindHint(key, action, priority, actionName));
	}
	
	void AddDynamicHint(string actionName, string description, int priority)
	{
		// Get current key from registry
		string key = "?";
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (registry)
			key = registry.GetKeyDisplayForAction(actionName);
		
		AddHint(key, description, priority, actionName);
	}
	
	void RemoveHint(string action)
	{
		for (int i = m_aHints.Count() - 1; i >= 0; i--)
		{
			if (m_aHints[i].m_sAction == action)
			{
				m_aHints[i].m_bVisible = false;
				return;
			}
		}
	}
	
	void ShowContextHint(string key, string action, int priority, string actionName = "")
	{
		RBL_KeybindHint hint = new RBL_KeybindHint(key, action, priority, actionName);
		hint.m_fAlpha = 1.0; // Immediately visible
		m_aContextHints.Insert(hint);
	}
	
	void ClearContextHints()
	{
		m_aContextHints.Clear();
	}
	
	// Refresh all hints from input system
	void RefreshDynamicHints()
	{
		for (int i = 0; i < m_aHints.Count(); i++)
		{
			if (m_aHints[i].m_bDynamic)
			{
				// Key will be looked up next time GetDisplayKey is called
				m_aHints[i].m_bVisible = true;
			}
		}
	}
	
	// Helpers
	protected IEntity GetLocalPlayer()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;
		return pc.GetControlledEntity();
	}
	
	// Get formatted keybind string for external use
	static string GetKeybindString(string actionName, string description)
	{
		string key = "?";
		RBL_InputBindingRegistry registry = RBL_InputBindingRegistry.GetInstance();
		if (registry)
			key = registry.GetKeyDisplayForAction(actionName);
		
		return string.Format("[%1] %2", key, description);
	}
}
