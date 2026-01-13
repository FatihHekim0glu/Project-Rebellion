// ============================================================================
// PROJECT REBELLION - Save Slot Manager
// Manages save slots, selection, and slot-based operations
// ============================================================================

class RBL_SaveSlotManager
{
	protected static ref RBL_SaveSlotManager s_Instance;
	
	// Configuration
	protected const int MAX_SAVE_SLOTS = 10;
	protected const int RECENT_SAVES_COUNT = 5;
	
	// State
	protected int m_iCurrentSlot;
	protected int m_iLastUsedSlot;
	protected ref array<ref RBL_SaveSlotInfo> m_aSlotInfos;
	protected ref array<string> m_aRecentSaves;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_SaveSlotManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_SaveSlotManager();
		return s_Instance;
	}
	
	void RBL_SaveSlotManager()
	{
		m_iCurrentSlot = 0;
		m_iLastUsedSlot = -1;
		m_aSlotInfos = new array<ref RBL_SaveSlotInfo>();
		m_aRecentSaves = new array<string>();
		
		RefreshSlotInfos();
	}
	
	// ========================================================================
	// SLOT SELECTION
	// ========================================================================
	
	// Select a save slot
	void SelectSlot(int slotIndex)
	{
		if (slotIndex < 0 || slotIndex >= MAX_SAVE_SLOTS)
		{
			PrintFormat("[RBL_SaveSlot] Invalid slot index: %1", slotIndex);
			return;
		}
		
		m_iCurrentSlot = slotIndex;
		PrintFormat("[RBL_SaveSlot] Selected slot: %1", slotIndex);
	}
	
	// Get currently selected slot
	int GetCurrentSlot()
	{
		return m_iCurrentSlot;
	}
	
	// Get last used slot (for continue game)
	int GetLastUsedSlot()
	{
		if (m_iLastUsedSlot < 0)
		{
			// Find most recently saved slot
			m_iLastUsedSlot = FindMostRecentSlot();
		}
		return m_iLastUsedSlot;
	}
	
	// Find first empty slot
	int FindFirstEmptySlot()
	{
		RefreshSlotInfos();
		
		for (int i = 0; i < m_aSlotInfos.Count(); i++)
		{
			if (m_aSlotInfos[i].m_bIsEmpty)
				return i;
		}
		
		return -1; // All slots full
	}
	
	// Find most recent slot
	int FindMostRecentSlot()
	{
		RefreshSlotInfos();
		
		int mostRecentSlot = -1;
		float mostRecentPlayTime = -1;
		
		for (int i = 0; i < m_aSlotInfos.Count(); i++)
		{
			if (!m_aSlotInfos[i].m_bIsEmpty && !m_aSlotInfos[i].m_bIsCorrupted)
			{
				if (m_aSlotInfos[i].m_fPlayTime > mostRecentPlayTime)
				{
					mostRecentPlayTime = m_aSlotInfos[i].m_fPlayTime;
					mostRecentSlot = i;
				}
			}
		}
		
		return mostRecentSlot;
	}
	
	// ========================================================================
	// SLOT OPERATIONS
	// ========================================================================
	
	// Save to selected slot
	bool SaveToCurrentSlot(RBL_SaveData saveData)
	{
		return SaveToSlot(m_iCurrentSlot, saveData);
	}
	
	// Save to specific slot
	bool SaveToSlot(int slotIndex, RBL_SaveData saveData)
	{
		if (slotIndex < 0 || slotIndex >= MAX_SAVE_SLOTS)
		{
			PrintFormat("[RBL_SaveSlot] Invalid slot for save: %1", slotIndex);
			return false;
		}
		
		if (!saveData)
		{
			PrintFormat("[RBL_SaveSlot] Null save data");
			return false;
		}
		
		// Update save timestamp
		saveData.m_sSaveTime = GetCurrentTimestamp();
		
		// Serialize
		string json = RBL_SaveSerializer.SerializeToJson(saveData);
		if (json.IsEmpty())
		{
			PrintFormat("[RBL_SaveSlot] Serialization failed");
			return false;
		}
		
		// Get filename for slot
		RBL_SaveFileManager fileMgr = RBL_SaveFileManager.GetInstance();
		string filename = fileMgr.GetSlotFilename(slotIndex);
		
		// Write to file
		bool result = fileMgr.WriteToFile(filename, json);
		
		if (result)
		{
			m_iLastUsedSlot = slotIndex;
			AddToRecentSaves(filename);
			RefreshSlotInfos();
			PrintFormat("[RBL_SaveSlot] Saved to slot %1", slotIndex);
		}
		
		return result;
	}
	
	// Load from selected slot
	RBL_SaveData LoadFromCurrentSlot()
	{
		return LoadFromSlot(m_iCurrentSlot);
	}
	
	// Load from specific slot
	RBL_SaveData LoadFromSlot(int slotIndex)
	{
		if (slotIndex < 0 || slotIndex >= MAX_SAVE_SLOTS)
		{
			PrintFormat("[RBL_SaveSlot] Invalid slot for load: %1", slotIndex);
			return null;
		}
		
		RBL_SaveFileManager fileMgr = RBL_SaveFileManager.GetInstance();
		string filename = fileMgr.GetSlotFilename(slotIndex);
		
		// Read from file
		string json = fileMgr.ReadFromFile(filename);
		if (json.IsEmpty())
		{
			PrintFormat("[RBL_SaveSlot] Slot %1 is empty or unreadable", slotIndex);
			return null;
		}
		
		// Deserialize
		RBL_SaveData saveData = RBL_SaveSerializer.DeserializeFromJson(json);
		
		if (saveData)
		{
			m_iLastUsedSlot = slotIndex;
			PrintFormat("[RBL_SaveSlot] Loaded from slot %1", slotIndex);
		}
		
		return saveData;
	}
	
	// Delete slot
	bool DeleteSlot(int slotIndex)
	{
		if (slotIndex < 0 || slotIndex >= MAX_SAVE_SLOTS)
		{
			PrintFormat("[RBL_SaveSlot] Invalid slot for delete: %1", slotIndex);
			return false;
		}
		
		RBL_SaveFileManager fileMgr = RBL_SaveFileManager.GetInstance();
		string filename = fileMgr.GetSlotFilename(slotIndex);
		
		bool result = fileMgr.DeleteFile(filename);
		
		if (result)
		{
			RefreshSlotInfos();
			PrintFormat("[RBL_SaveSlot] Deleted slot %1", slotIndex);
		}
		
		return result;
	}
	
	// Copy slot to another slot
	bool CopySlot(int sourceSlot, int destSlot)
	{
		if (sourceSlot < 0 || sourceSlot >= MAX_SAVE_SLOTS ||
			destSlot < 0 || destSlot >= MAX_SAVE_SLOTS)
		{
			PrintFormat("[RBL_SaveSlot] Invalid slot indices for copy");
			return false;
		}
		
		RBL_SaveData saveData = LoadFromSlot(sourceSlot);
		if (!saveData)
			return false;
		
		return SaveToSlot(destSlot, saveData);
	}
	
	// ========================================================================
	// SLOT INFO
	// ========================================================================
	
	// Get slot info
	RBL_SaveSlotInfo GetSlotInfo(int slotIndex)
	{
		if (slotIndex < 0 || slotIndex >= m_aSlotInfos.Count())
			return null;
		
		return m_aSlotInfos[slotIndex];
	}
	
	// Get all slot infos
	array<ref RBL_SaveSlotInfo> GetAllSlotInfos()
	{
		return m_aSlotInfos;
	}
	
	// Refresh slot info cache
	void RefreshSlotInfos()
	{
		m_aSlotInfos.Clear();
		
		RBL_SaveFileManager fileMgr = RBL_SaveFileManager.GetInstance();
		
		for (int i = 0; i < MAX_SAVE_SLOTS; i++)
		{
			RBL_SaveSlotInfo info = fileMgr.GetSlotInfo(i);
			m_aSlotInfos.Insert(info);
		}
	}
	
	// Check if slot is empty
	bool IsSlotEmpty(int slotIndex)
	{
		RBL_SaveSlotInfo info = GetSlotInfo(slotIndex);
		if (!info)
			return true;
		return info.m_bIsEmpty;
	}
	
	// Check if slot is corrupted
	bool IsSlotCorrupted(int slotIndex)
	{
		RBL_SaveSlotInfo info = GetSlotInfo(slotIndex);
		if (!info)
			return false;
		return info.m_bIsCorrupted;
	}
	
	// Get slot count
	int GetMaxSlots()
	{
		return MAX_SAVE_SLOTS;
	}
	
	// Get used slot count
	int GetUsedSlotCount()
	{
		int count = 0;
		for (int i = 0; i < m_aSlotInfos.Count(); i++)
		{
			if (!m_aSlotInfos[i].m_bIsEmpty)
				count++;
		}
		return count;
	}
	
	// ========================================================================
	// RECENT SAVES
	// ========================================================================
	
	// Add to recent saves list
	protected void AddToRecentSaves(string filename)
	{
		// Remove if already exists
		int existingIdx = m_aRecentSaves.Find(filename);
		if (existingIdx >= 0)
			m_aRecentSaves.Remove(existingIdx);
		
		// Add at front
		m_aRecentSaves.InsertAt(filename, 0);
		
		// Trim to max count
		while (m_aRecentSaves.Count() > RECENT_SAVES_COUNT)
		{
			m_aRecentSaves.Remove(m_aRecentSaves.Count() - 1);
		}
	}
	
	// Get recent saves
	array<string> GetRecentSaves()
	{
		return m_aRecentSaves;
	}
	
	// ========================================================================
	// UTILITIES
	// ========================================================================
	
	// Get current timestamp string
	protected string GetCurrentTimestamp()
	{
		int year, month, day, hour, minute, second;
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, second);
		
		return string.Format("%1-%2-%3 %4:%5:%6",
			year, PadZero(month), PadZero(day),
			PadZero(hour), PadZero(minute), PadZero(second));
	}
	
	// Pad single digit with zero
	protected string PadZero(int value)
	{
		if (value < 10)
			return "0" + value.ToString();
		return value.ToString();
	}
	
	// ========================================================================
	// SLOT DISPLAY HELPERS
	// ========================================================================
	
	// Get display text for slot
	string GetSlotDisplayText(int slotIndex)
	{
		RBL_SaveSlotInfo info = GetSlotInfo(slotIndex);
		if (!info)
			return "Invalid Slot";
		return info.GetDisplayText();
	}
	
	// Get all display texts
	array<string> GetAllSlotDisplayTexts()
	{
		array<string> texts = new array<string>();
		
		for (int i = 0; i < m_aSlotInfos.Count(); i++)
		{
			texts.Insert(GetSlotDisplayText(i));
		}
		
		return texts;
	}
}

// ============================================================================
// SAVE SLOT UI WIDGET
// ============================================================================
class RBL_SaveSlotWidget : RBL_PanelWidget
{
	protected int m_iSlotIndex;
	protected RBL_SaveSlotInfo m_SlotInfo;
	protected bool m_bIsSelected;
	protected bool m_bIsHovered;
	
	void RBL_SaveSlotWidget(int slotIndex)
	{
		m_iSlotIndex = slotIndex;
		m_bIsSelected = false;
		m_bIsHovered = false;
		
		RefreshInfo();
	}
	
	void RefreshInfo()
	{
		m_SlotInfo = RBL_SaveSlotManager.GetInstance().GetSlotInfo(m_iSlotIndex);
	}
	
	void SetSelected(bool selected)
	{
		m_bIsSelected = selected;
	}
	
	void SetHovered(bool hovered)
	{
		m_bIsHovered = hovered;
	}
	
	override void Draw()
	{
		if (!IsVisible() || !m_SlotInfo)
			return;
		
		// Background color based on state
		int bgColor = RBL_UIColors.COLOR_BG_MEDIUM;
		if (m_bIsSelected)
			bgColor = RBL_UIColors.COLOR_BG_HIGHLIGHT;
		else if (m_bIsHovered)
			bgColor = RBL_UIColors.COLOR_BG_LIGHT;
		
		if (m_SlotInfo.m_bIsCorrupted)
			bgColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_RED, 0.3);
		
		bgColor = ApplyAlpha(bgColor, m_fAlpha);
		DrawRect(m_fPosX, m_fPosY, m_fWidth, m_fHeight, bgColor);
		
		// Border
		int borderColor = RBL_UIColors.COLOR_BORDER_DARK;
		if (m_bIsSelected)
			borderColor = RBL_UIColors.COLOR_ACCENT_GREEN;
		borderColor = ApplyAlpha(borderColor, m_fAlpha);
		DrawRectOutline(m_fPosX, m_fPosY, m_fWidth, m_fHeight, borderColor, 1);
		
		// Slot number
		int numColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
		DbgUI.Begin("Slot_Num_" + m_iSlotIndex.ToString(), m_fPosX + 8, m_fPosY + 8);
		DbgUI.Text("SLOT " + m_iSlotIndex.ToString());
		DbgUI.End();
		
		// Content
		if (m_SlotInfo.m_bIsEmpty)
		{
			int emptyColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
			DbgUI.Begin("Slot_Empty_" + m_iSlotIndex.ToString(), m_fPosX + 8, m_fPosY + 28);
			DbgUI.Text("- Empty -");
			DbgUI.End();
		}
		else if (m_SlotInfo.m_bIsCorrupted)
		{
			int corruptColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_RED, m_fAlpha);
			DbgUI.Begin("Slot_Corrupt_" + m_iSlotIndex.ToString(), m_fPosX + 8, m_fPosY + 28);
			DbgUI.Text("! CORRUPTED !");
			DbgUI.End();
		}
		else
		{
			// World name
			int worldColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
			DbgUI.Begin("Slot_World_" + m_iSlotIndex.ToString(), m_fPosX + 8, m_fPosY + 28);
			DbgUI.Text(m_SlotInfo.m_sWorldName);
			DbgUI.End();
			
			// War level and zones
			string infoText = string.Format("WL%1 | %2/%3 zones", 
				m_SlotInfo.m_iWarLevel, m_SlotInfo.m_iZonesControlled, m_SlotInfo.m_iTotalZones);
			int infoColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
			DbgUI.Begin("Slot_Info_" + m_iSlotIndex.ToString(), m_fPosX + 8, m_fPosY + 48);
			DbgUI.Text(infoText);
			DbgUI.End();
			
			// Save time
			int timeColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
			DbgUI.Begin("Slot_Time_" + m_iSlotIndex.ToString(), m_fPosX + 8, m_fPosY + 68);
			DbgUI.Text(m_SlotInfo.m_sSaveTime);
			DbgUI.End();
		}
	}
	
	int GetSlotIndex() { return m_iSlotIndex; }
	bool IsEmpty() { return m_SlotInfo && m_SlotInfo.m_bIsEmpty; }
	bool IsCorrupted() { return m_SlotInfo && m_SlotInfo.m_bIsCorrupted; }
}

