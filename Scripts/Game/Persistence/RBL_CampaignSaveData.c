// ============================================================================
// PROJECT REBELLION - Persistence Data Structures & Manager
// Full JSON save/load implementation
// ============================================================================

class RBL_ZoneSaveData
{
	string ZoneID;
	int Owner; // ERBLFactionKey as int for JSON
	int Support;
	int AlertState; // ERBLAlertState as int
	ref array<string> GarrisonUnits;
	
	void RBL_ZoneSaveData()
	{
		GarrisonUnits = new array<string>();
	}
}

class RBL_ArsenalItemData
{
	string ItemPrefab;
	int Count;
	int Availability; // ERBLItemAvailability as int
}

class RBL_PlayerSaveData
{
	string PlayerUID;
	string PlayerName;
	int Kills;
	int Deaths;
	int ZonesCaptured;
	float PlayTime;
	float PosX;
	float PosY;
	float PosZ;
	ref array<string> LastLoadout;
	
	void RBL_PlayerSaveData()
	{
		LastLoadout = new array<string>();
	}
}

class RBL_CampaignInfoData
{
	int WarLevel;
	int Aggression;
	int Money;
	int HumanResources;
	float TimeOfDay;
	int DayNumber;
	float TotalPlayTime;
	string CurrentWeather;
	int SaveVersion;
	int EnemyResources;
	
	void RBL_CampaignInfoData()
	{
		SaveVersion = 2;
		CurrentWeather = "Clear";
	}
}

class RBL_ActiveMissionData
{
	string MissionID;
	int MissionType; // ERBLQRFType as int
	string TargetZoneID;
	string SourceZoneID;
	float PosX;
	float PosY;
	float PosZ;
	ref array<string> AssignedUnits;
	float TimeStarted;
	
	void RBL_ActiveMissionData()
	{
		AssignedUnits = new array<string>();
	}
}

class RBL_CampaignSaveData
{
	ref RBL_CampaignInfoData CampaignInfo;
	ref array<ref RBL_ZoneSaveData> Zones;
	ref array<ref RBL_ArsenalItemData> Arsenal;
	ref array<ref RBL_PlayerSaveData> Players;
	ref array<ref RBL_ActiveMissionData> ActiveMissions;
	
	void RBL_CampaignSaveData()
	{
		CampaignInfo = new RBL_CampaignInfoData();
		Zones = new array<ref RBL_ZoneSaveData>();
		Arsenal = new array<ref RBL_ArsenalItemData>();
		Players = new array<ref RBL_PlayerSaveData>();
		ActiveMissions = new array<ref RBL_ActiveMissionData>();
	}
	
	void ClearAll()
	{
		Zones.Clear();
		Arsenal.Clear();
		Players.Clear();
		ActiveMissions.Clear();
	}
}

// ============================================================================
// PERSISTENCE MANAGER - Full JSON Implementation
// ============================================================================
class RBL_PersistenceManager
{
	protected static ref RBL_PersistenceManager s_Instance;
	
	protected const string SAVE_FOLDER = "$profile:RBL_Saves/";
	protected const string SAVE_FILE_NAME = "campaign_save.json";
	protected const float AUTO_SAVE_INTERVAL = 300.0;
	protected const int CURRENT_SAVE_VERSION = 2;
	
	protected ref RBL_CampaignSaveData m_CurrentSaveData;
	protected float m_fTimeSinceLastSave;
	protected bool m_bSaveInProgress;
	protected bool m_bHasLoadedSave;
	protected string m_sLastSaveTime;
	
	static RBL_PersistenceManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_PersistenceManager();
		return s_Instance;
	}
	
	void RBL_PersistenceManager()
	{
		m_CurrentSaveData = new RBL_CampaignSaveData();
		m_fTimeSinceLastSave = 0;
		m_bSaveInProgress = false;
		m_bHasLoadedSave = false;
		m_sLastSaveTime = "";
	}
	
	// ========================================================================
	// SAVE CAMPAIGN
	// ========================================================================
	bool SaveCampaign()
	{
		if (m_bSaveInProgress)
			return false;
		
		m_bSaveInProgress = true;
		
		PrintFormat("[RBL_Persistence] Saving campaign...");
		
		// Collect current state
		CollectCampaignState();
		
		// Serialize to JSON string
		string jsonData = SerializeToJSON();
		
		// Write to file
		bool success = WriteToFile(jsonData);
		
		if (success)
		{
			m_fTimeSinceLastSave = 0;
			m_sLastSaveTime = GetCurrentTimeString();
			PrintFormat("[RBL_Persistence] Campaign saved successfully!");
			PrintFormat("[RBL_Persistence]   Zones: %1", m_CurrentSaveData.Zones.Count());
			PrintFormat("[RBL_Persistence]   Arsenal items: %1", m_CurrentSaveData.Arsenal.Count());
		}
		else
		{
			PrintFormat("[RBL_Persistence] ERROR: Failed to save campaign!");
		}
		
		m_bSaveInProgress = false;
		return success;
	}
	
	protected void CollectCampaignState()
	{
		m_CurrentSaveData.ClearAll();
		
		// Campaign info
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			m_CurrentSaveData.CampaignInfo.WarLevel = campaignMgr.GetWarLevel();
			m_CurrentSaveData.CampaignInfo.Aggression = campaignMgr.GetAggression();
			m_CurrentSaveData.CampaignInfo.DayNumber = campaignMgr.GetDayNumber();
			m_CurrentSaveData.CampaignInfo.TotalPlayTime = campaignMgr.GetTotalPlayTime();
		}
		
		// Economy
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			m_CurrentSaveData.CampaignInfo.Money = econMgr.GetMoney();
			m_CurrentSaveData.CampaignInfo.HumanResources = econMgr.GetHR();
			
			// Save arsenal inventory
			map<string, int> arsenal = econMgr.GetArsenalInventory();
			if (arsenal)
			{
				for (int i = 0; i < arsenal.Count(); i++)
				{
					string itemKey = "";
					int itemCount = 0;
					arsenal.GetKeyByIndex(i, itemKey);
					arsenal.Find(itemKey, itemCount);
					
					if (itemCount > 0)
					{
						RBL_ArsenalItemData itemData = new RBL_ArsenalItemData();
						itemData.ItemPrefab = itemKey;
						itemData.Count = itemCount;
						itemData.Availability = econMgr.GetItemAvailability(itemKey);
						m_CurrentSaveData.Arsenal.Insert(itemData);
					}
				}
			}
		}
		
		// Commander AI resources
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
		{
			m_CurrentSaveData.CampaignInfo.EnemyResources = commanderAI.GetFactionResources();
		}
		
		// Virtual zones
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			array<ref RBL_VirtualZone> virtualZones = zoneMgr.GetAllVirtualZones();
			for (int i = 0; i < virtualZones.Count(); i++)
			{
				RBL_VirtualZone zone = virtualZones[i];
				RBL_ZoneSaveData zoneData = new RBL_ZoneSaveData();
				zoneData.ZoneID = zone.GetZoneID();
				zoneData.Owner = zone.GetOwnerFaction();
				zoneData.Support = zone.GetCivilianSupport();
				zoneData.AlertState = zone.GetAlertState();
				m_CurrentSaveData.Zones.Insert(zoneData);
			}
			
			// Entity zones
			array<RBL_CampaignZone> entityZones = zoneMgr.GetAllZones();
			for (int i = 0; i < entityZones.Count(); i++)
			{
				RBL_ZoneSaveData zoneData;
				entityZones[i].SerializeToStruct(zoneData);
				m_CurrentSaveData.Zones.Insert(zoneData);
			}
		}
		
		m_CurrentSaveData.CampaignInfo.SaveVersion = CURRENT_SAVE_VERSION;
	}
	
	protected string SerializeToJSON()
	{
		string json = "{\n";
		
		// Campaign info
		json += "  \"CampaignInfo\": {\n";
		json += string.Format("    \"SaveVersion\": %1,\n", m_CurrentSaveData.CampaignInfo.SaveVersion);
		json += string.Format("    \"WarLevel\": %1,\n", m_CurrentSaveData.CampaignInfo.WarLevel);
		json += string.Format("    \"Aggression\": %1,\n", m_CurrentSaveData.CampaignInfo.Aggression);
		json += string.Format("    \"Money\": %1,\n", m_CurrentSaveData.CampaignInfo.Money);
		json += string.Format("    \"HumanResources\": %1,\n", m_CurrentSaveData.CampaignInfo.HumanResources);
		json += string.Format("    \"DayNumber\": %1,\n", m_CurrentSaveData.CampaignInfo.DayNumber);
		json += string.Format("    \"TotalPlayTime\": %1,\n", m_CurrentSaveData.CampaignInfo.TotalPlayTime);
		json += string.Format("    \"EnemyResources\": %1\n", m_CurrentSaveData.CampaignInfo.EnemyResources);
		json += "  },\n";
		
		// Zones
		json += "  \"Zones\": [\n";
		for (int i = 0; i < m_CurrentSaveData.Zones.Count(); i++)
		{
			RBL_ZoneSaveData zone = m_CurrentSaveData.Zones[i];
			json += "    {\n";
			json += string.Format("      \"ZoneID\": \"%1\",\n", zone.ZoneID);
			json += string.Format("      \"Owner\": %1,\n", zone.Owner);
			json += string.Format("      \"Support\": %1,\n", zone.Support);
			json += string.Format("      \"AlertState\": %1\n", zone.AlertState);
			json += "    }";
			if (i < m_CurrentSaveData.Zones.Count() - 1)
				json += ",";
			json += "\n";
		}
		json += "  ],\n";
		
		// Arsenal
		json += "  \"Arsenal\": [\n";
		for (int i = 0; i < m_CurrentSaveData.Arsenal.Count(); i++)
		{
			RBL_ArsenalItemData item = m_CurrentSaveData.Arsenal[i];
			json += "    {\n";
			json += string.Format("      \"ItemPrefab\": \"%1\",\n", item.ItemPrefab);
			json += string.Format("      \"Count\": %1,\n", item.Count);
			json += string.Format("      \"Availability\": %1\n", item.Availability);
			json += "    }";
			if (i < m_CurrentSaveData.Arsenal.Count() - 1)
				json += ",";
			json += "\n";
		}
		json += "  ]\n";
		
		json += "}";
		
		return json;
	}
	
	protected bool WriteToFile(string jsonData)
	{
		// Create directory if needed
		if (!FileIO.FileExist(SAVE_FOLDER))
			FileIO.MakeDirectory(SAVE_FOLDER);
		
		string fullPath = SAVE_FOLDER + SAVE_FILE_NAME;
		
		FileHandle file = FileIO.OpenFile(fullPath, FileMode.WRITE);
		if (!file)
		{
			PrintFormat("[RBL_Persistence] ERROR: Could not open file for writing: %1", fullPath);
			return false;
		}
		
		file.Write(jsonData);
		file.Close();
		
		PrintFormat("[RBL_Persistence] Saved to: %1", fullPath);
		return true;
	}
	
	// ========================================================================
	// LOAD CAMPAIGN
	// ========================================================================
	bool LoadCampaign()
	{
		PrintFormat("[RBL_Persistence] Loading campaign...");
		
		string fullPath = SAVE_FOLDER + SAVE_FILE_NAME;
		
		if (!FileIO.FileExist(fullPath))
		{
			PrintFormat("[RBL_Persistence] No save file found at: %1", fullPath);
			return false;
		}
		
		// Read file
		string jsonData = ReadFromFile(fullPath);
		if (jsonData.IsEmpty())
		{
			PrintFormat("[RBL_Persistence] ERROR: Failed to read save file");
			return false;
		}
		
		// Parse JSON and apply state
		bool success = DeserializeFromJSON(jsonData);
		
		if (success)
		{
			ApplyLoadedState();
			m_bHasLoadedSave = true;
			PrintFormat("[RBL_Persistence] Campaign loaded successfully!");
		}
		else
		{
			PrintFormat("[RBL_Persistence] ERROR: Failed to parse save file");
		}
		
		return success;
	}
	
	protected string ReadFromFile(string path)
	{
		FileHandle file = FileIO.OpenFile(path, FileMode.READ);
		if (!file)
			return "";
		
		string content = "";
		string line;
		
		while (file.ReadLine(line) >= 0)
		{
			content += line + "\n";
		}
		
		file.Close();
		return content;
	}
	
	protected bool DeserializeFromJSON(string jsonData)
	{
		m_CurrentSaveData.ClearAll();
		
		// Simple JSON parsing (line-by-line key:value extraction)
		// Note: This is a simplified parser. For complex JSON, use proper serializer.
		
		array<string> lines = new array<string>();
		jsonData.Split("\n", lines, false);
		
		string currentSection = "";
		RBL_ZoneSaveData currentZone = null;
		RBL_ArsenalItemData currentItem = null;
		
		for (int i = 0; i < lines.Count(); i++)
		{
			string line = lines[i].Trim();
			
			// Detect section
			if (line.Contains("\"CampaignInfo\""))
				currentSection = "CampaignInfo";
			else if (line.Contains("\"Zones\""))
				currentSection = "Zones";
			else if (line.Contains("\"Arsenal\""))
				currentSection = "Arsenal";
			
			// Parse values based on section
			if (currentSection == "CampaignInfo")
			{
				ParseCampaignInfoLine(line);
			}
			else if (currentSection == "Zones")
			{
				if (line == "{")
				{
					currentZone = new RBL_ZoneSaveData();
				}
				else if (line.StartsWith("}"))
				{
					if (currentZone && currentZone.ZoneID != "")
						m_CurrentSaveData.Zones.Insert(currentZone);
					currentZone = null;
				}
				else if (currentZone)
				{
					ParseZoneLine(line, currentZone);
				}
			}
			else if (currentSection == "Arsenal")
			{
				if (line == "{")
				{
					currentItem = new RBL_ArsenalItemData();
				}
				else if (line.StartsWith("}"))
				{
					if (currentItem && currentItem.ItemPrefab != "")
						m_CurrentSaveData.Arsenal.Insert(currentItem);
					currentItem = null;
				}
				else if (currentItem)
				{
					ParseArsenalLine(line, currentItem);
				}
			}
		}
		
		return m_CurrentSaveData.CampaignInfo.SaveVersion > 0;
	}
	
	protected void ParseCampaignInfoLine(string line)
	{
		if (line.Contains("\"SaveVersion\""))
			m_CurrentSaveData.CampaignInfo.SaveVersion = ExtractIntValue(line);
		else if (line.Contains("\"WarLevel\""))
			m_CurrentSaveData.CampaignInfo.WarLevel = ExtractIntValue(line);
		else if (line.Contains("\"Aggression\""))
			m_CurrentSaveData.CampaignInfo.Aggression = ExtractIntValue(line);
		else if (line.Contains("\"Money\""))
			m_CurrentSaveData.CampaignInfo.Money = ExtractIntValue(line);
		else if (line.Contains("\"HumanResources\""))
			m_CurrentSaveData.CampaignInfo.HumanResources = ExtractIntValue(line);
		else if (line.Contains("\"DayNumber\""))
			m_CurrentSaveData.CampaignInfo.DayNumber = ExtractIntValue(line);
		else if (line.Contains("\"TotalPlayTime\""))
			m_CurrentSaveData.CampaignInfo.TotalPlayTime = ExtractFloatValue(line);
		else if (line.Contains("\"EnemyResources\""))
			m_CurrentSaveData.CampaignInfo.EnemyResources = ExtractIntValue(line);
	}
	
	protected void ParseZoneLine(string line, RBL_ZoneSaveData zone)
	{
		if (line.Contains("\"ZoneID\""))
			zone.ZoneID = ExtractStringValue(line);
		else if (line.Contains("\"Owner\""))
			zone.Owner = ExtractIntValue(line);
		else if (line.Contains("\"Support\""))
			zone.Support = ExtractIntValue(line);
		else if (line.Contains("\"AlertState\""))
			zone.AlertState = ExtractIntValue(line);
	}
	
	protected void ParseArsenalLine(string line, RBL_ArsenalItemData item)
	{
		if (line.Contains("\"ItemPrefab\""))
			item.ItemPrefab = ExtractStringValue(line);
		else if (line.Contains("\"Count\""))
			item.Count = ExtractIntValue(line);
		else if (line.Contains("\"Availability\""))
			item.Availability = ExtractIntValue(line);
	}
	
	protected int ExtractIntValue(string line)
	{
		int colonPos = line.IndexOf(":");
		if (colonPos < 0)
			return 0;
		
		string valueStr = line.Substring(colonPos + 1, line.Length() - colonPos - 1);
		valueStr = valueStr.Trim();
		valueStr = valueStr.Replace(",", "");
		
		return valueStr.ToInt();
	}
	
	protected float ExtractFloatValue(string line)
	{
		int colonPos = line.IndexOf(":");
		if (colonPos < 0)
			return 0;
		
		string valueStr = line.Substring(colonPos + 1, line.Length() - colonPos - 1);
		valueStr = valueStr.Trim();
		valueStr = valueStr.Replace(",", "");
		
		return valueStr.ToFloat();
	}
	
	protected string ExtractStringValue(string line)
	{
		int firstQuote = line.IndexOf("\"");
		if (firstQuote < 0)
			return "";
		
		int colonPos = line.IndexOf(":");
		if (colonPos < 0)
			return "";
		
		string afterColon = line.Substring(colonPos + 1, line.Length() - colonPos - 1);
		afterColon = afterColon.Trim();
		
		// Remove quotes
		afterColon = afterColon.Replace("\"", "");
		afterColon = afterColon.Replace(",", "");
		
		return afterColon.Trim();
	}
	
	protected void ApplyLoadedState()
	{
		// Apply campaign info
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			campaignMgr.SetWarLevel(m_CurrentSaveData.CampaignInfo.WarLevel);
			campaignMgr.SetAggression(m_CurrentSaveData.CampaignInfo.Aggression);
			campaignMgr.SetDayNumber(m_CurrentSaveData.CampaignInfo.DayNumber);
		}
		
		// Apply economy
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			econMgr.SetMoney(m_CurrentSaveData.CampaignInfo.Money);
			econMgr.SetHR(m_CurrentSaveData.CampaignInfo.HumanResources);
			
			// Restore arsenal
			for (int i = 0; i < m_CurrentSaveData.Arsenal.Count(); i++)
			{
				RBL_ArsenalItemData item = m_CurrentSaveData.Arsenal[i];
				econMgr.SetArsenalItemCount(item.ItemPrefab, item.Count);
			}
		}
		
		// Apply zone ownership
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			for (int i = 0; i < m_CurrentSaveData.Zones.Count(); i++)
			{
				RBL_ZoneSaveData zoneData = m_CurrentSaveData.Zones[i];
				
				// Try virtual zone first
				RBL_VirtualZone vZone = zoneMgr.GetVirtualZoneByID(zoneData.ZoneID);
				if (vZone)
				{
					vZone.SetOwnerFaction(zoneData.Owner);
					vZone.SetCivilianSupport(zoneData.Support);
					vZone.SetAlertState(zoneData.AlertState);
				}
				
				// Try entity zone
				RBL_CampaignZone eZone = zoneMgr.GetZoneByID(zoneData.ZoneID);
				if (eZone)
				{
					eZone.DeserializeFromStruct(zoneData);
				}
			}
		}
		
		PrintFormat("[RBL_Persistence] Applied loaded state:");
		PrintFormat("[RBL_Persistence]   War Level: %1", m_CurrentSaveData.CampaignInfo.WarLevel);
		PrintFormat("[RBL_Persistence]   Money: $%1", m_CurrentSaveData.CampaignInfo.Money);
		PrintFormat("[RBL_Persistence]   Zones restored: %1", m_CurrentSaveData.Zones.Count());
	}
	
	// ========================================================================
	// UPDATE & UTILITIES
	// ========================================================================
	
	void Update(float timeSlice)
	{
		m_fTimeSinceLastSave += timeSlice;
		
		if (m_fTimeSinceLastSave >= AUTO_SAVE_INTERVAL)
		{
			SaveCampaign();
		}
	}
	
	void TriggerAutoSave()
	{
		SaveCampaign();
	}
	
	bool HasExistingSave()
	{
		string fullPath = SAVE_FOLDER + SAVE_FILE_NAME;
		return FileIO.FileExist(fullPath);
	}
	
	void DeleteSave()
	{
		string fullPath = SAVE_FOLDER + SAVE_FILE_NAME;
		if (FileIO.FileExist(fullPath))
		{
			FileIO.DeleteFile(fullPath);
			PrintFormat("[RBL_Persistence] Save file deleted");
		}
	}
	
	string GetCurrentTimeString()
	{
		int year, month, day, hour, minute, second;
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, second);
		return string.Format("%1-%2-%3 %4:%5:%6", year, month, day, hour, minute, second);
	}
	
	RBL_CampaignSaveData GetCurrentSaveData() { return m_CurrentSaveData; }
	bool HasLoadedSave() { return m_bHasLoadedSave; }
	string GetLastSaveTime() { return m_sLastSaveTime; }
	float GetTimeSinceLastSave() { return m_fTimeSinceLastSave; }
}
