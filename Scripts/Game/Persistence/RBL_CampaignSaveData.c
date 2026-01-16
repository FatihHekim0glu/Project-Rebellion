// ============================================================================
// PROJECT REBELLION - Simple Persistence Manager
// Lightweight save/load using RBL_SaveData structures
// ============================================================================

class RBL_PersistenceManager
{
	protected static ref RBL_PersistenceManager s_Instance;
	
	protected const string SAVE_FOLDER = "$profile:RBL_Saves/";
	protected const string SAVE_FILE_NAME = "campaign_save.json";
	protected const float AUTO_SAVE_INTERVAL = 300.0;
	
	protected ref RBL_SaveData m_CurrentSaveData;
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
		m_CurrentSaveData = new RBL_SaveData();
		m_fTimeSinceLastSave = 0;
		m_bSaveInProgress = false;
		m_bHasLoadedSave = false;
		m_sLastSaveTime = "";
	}
	
	bool SaveCampaign()
	{
		if (m_bSaveInProgress)
			return false;
		
		m_bSaveInProgress = true;
		PrintFormat("[RBL_Persistence] Saving campaign...");
		
		CollectCampaignState();
		string jsonData = SerializeToJSON();
		bool success = WriteToFile(jsonData);
		
		if (success)
		{
			m_fTimeSinceLastSave = 0;
			m_sLastSaveTime = GetCurrentTimeString();
			PrintFormat("[RBL_Persistence] Campaign saved successfully!");
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
		// Campaign info
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			m_CurrentSaveData.m_Campaign.m_iWarLevel = campaignMgr.GetWarLevel();
			m_CurrentSaveData.m_Campaign.m_iAggression = campaignMgr.GetAggression();
			m_CurrentSaveData.m_Campaign.m_fTotalPlayTime = campaignMgr.GetTotalPlayTime();
		}
		
		// Economy
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			m_CurrentSaveData.m_Economy.m_iFIAMoney = econMgr.GetMoney();
			m_CurrentSaveData.m_Economy.m_iFIAHumanResources = econMgr.GetHR();
		}
		
		// Zones
		m_CurrentSaveData.m_aZones.Clear();
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			array<RBL_CampaignZone> zones = zoneMgr.GetAllZones();
			foreach (RBL_CampaignZone zone : zones)
			{
				RBL_ZoneSaveData zoneData = new RBL_ZoneSaveData();
				zoneData.m_sZoneID = zone.GetZoneID();
				zoneData.m_iOwnerFaction = zone.GetOwnerFaction();
				zoneData.m_iGarrisonStrength = zone.GetGarrisonStrength();
				zoneData.m_fCaptureProgress = zone.GetCaptureProgress();
				m_CurrentSaveData.m_aZones.Insert(zoneData);
			}
		}
		
		m_CurrentSaveData.m_iVersion = RBL_SAVE_VERSION;
		m_CurrentSaveData.m_sSaveTime = GetCurrentTimeString();
	}
	
	protected string SerializeToJSON()
	{
		string json = "{";
		json += string.Format("\"version\":%1,", m_CurrentSaveData.m_iVersion);
		json += string.Format("\"warLevel\":%1,", m_CurrentSaveData.m_Campaign.m_iWarLevel);
		json += string.Format("\"aggression\":%1,", m_CurrentSaveData.m_Campaign.m_iAggression);
		json += string.Format("\"money\":%1,", m_CurrentSaveData.m_Economy.m_iFIAMoney);
		json += string.Format("\"hr\":%1,", m_CurrentSaveData.m_Economy.m_iFIAHumanResources);
		json += string.Format("\"playTime\":%1", m_CurrentSaveData.m_Campaign.m_fTotalPlayTime);
		json += "}";
		return json;
	}
	
	protected bool WriteToFile(string jsonData)
	{
		if (!FileIO.FileExists(SAVE_FOLDER))
			FileIO.MakeDirectory(SAVE_FOLDER);
		
		string fullPath = SAVE_FOLDER + SAVE_FILE_NAME;
		FileHandle file = FileIO.OpenFile(fullPath, FileMode.WRITE);
		if (!file)
		{
			PrintFormat("[RBL_Persistence] ERROR: Could not open file: %1", fullPath);
			return false;
		}
		
		file.Write(jsonData);
		file.Close();
		PrintFormat("[RBL_Persistence] Saved to: %1", fullPath);
		return true;
	}
	
	bool LoadCampaign()
	{
		PrintFormat("[RBL_Persistence] Loading campaign...");
		
		string fullPath = SAVE_FOLDER + SAVE_FILE_NAME;
		if (!FileIO.FileExists(fullPath))
		{
			PrintFormat("[RBL_Persistence] No save file found");
			return false;
		}
		
		string jsonData = ReadFromFile(fullPath);
		if (jsonData.IsEmpty())
		{
			PrintFormat("[RBL_Persistence] ERROR: Failed to read save file");
			return false;
		}
		
		bool success = DeserializeFromJSON(jsonData);
		if (success)
		{
			ApplyLoadedState();
			m_bHasLoadedSave = true;
			PrintFormat("[RBL_Persistence] Campaign loaded successfully!");
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
			content += line;
		}
		file.Close();
		return content;
	}
	
	protected bool DeserializeFromJSON(string jsonData)
	{
		// Simple key:value parsing
		m_CurrentSaveData.m_Campaign.m_iWarLevel = ParseInt(jsonData, "warLevel", 1);
		m_CurrentSaveData.m_Campaign.m_iAggression = ParseInt(jsonData, "aggression", 0);
		m_CurrentSaveData.m_Economy.m_iFIAMoney = ParseInt(jsonData, "money", 500);
		m_CurrentSaveData.m_Economy.m_iFIAHumanResources = ParseInt(jsonData, "hr", 10);
		m_CurrentSaveData.m_Campaign.m_fTotalPlayTime = ParseFloat(jsonData, "playTime", 0);
		return true;
	}
	
	protected int ParseInt(string json, string key, int defaultVal)
	{
		string search = "\"" + key + "\":";
		int pos = json.IndexOf(search);
		if (pos < 0)
			return defaultVal;
		
		int start = pos + search.Length();
		int end = start;
		while (end < json.Length())
		{
			string c = json.Get(end);
			if (c == "," || c == "}")
				break;
			end++;
		}
		
		if (end <= start)
			return defaultVal;
		
		string valStr = json.Substring(start, end - start);
		return valStr.ToInt();
	}
	
	protected float ParseFloat(string json, string key, float defaultVal)
	{
		string search = "\"" + key + "\":";
		int pos = json.IndexOf(search);
		if (pos < 0)
			return defaultVal;
		
		int start = pos + search.Length();
		int end = start;
		while (end < json.Length())
		{
			string c = json.Get(end);
			if (c == "," || c == "}")
				break;
			end++;
		}
		
		if (end <= start)
			return defaultVal;
		
		string valStr = json.Substring(start, end - start);
		return valStr.ToFloat();
	}
	
	protected void ApplyLoadedState()
	{
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			campaignMgr.SetWarLevel(m_CurrentSaveData.m_Campaign.m_iWarLevel);
			campaignMgr.SetAggression(m_CurrentSaveData.m_Campaign.m_iAggression);
		}
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			econMgr.SetMoney(m_CurrentSaveData.m_Economy.m_iFIAMoney);
			econMgr.SetHR(m_CurrentSaveData.m_Economy.m_iFIAHumanResources);
		}
	}
	
	void Update(float timeSlice)
	{
		m_fTimeSinceLastSave += timeSlice;
		if (m_fTimeSinceLastSave >= AUTO_SAVE_INTERVAL)
			SaveCampaign();
	}
	
	void TriggerAutoSave() { SaveCampaign(); }
	bool HasExistingSave() { return FileIO.FileExists(SAVE_FOLDER + SAVE_FILE_NAME); }
	void DeleteSave() 
	{ 
		string path = SAVE_FOLDER + SAVE_FILE_NAME;
		if (FileIO.FileExists(path))
			FileIO.DeleteFile(path);
	}
	
	string GetCurrentTimeString()
	{
		int year, month, day, hour, minute, second;
		System.GetYearMonthDay(year, month, day);
		System.GetHourMinuteSecond(hour, minute, second);
		return string.Format("%1-%2-%3 %4:%5:%6", year, month, day, hour, minute, second);
	}
	
	RBL_SaveData GetCurrentSaveData() { return m_CurrentSaveData; }
	bool HasLoadedSave() { return m_bHasLoadedSave; }
	string GetLastSaveTime() { return m_sLastSaveTime; }
	float GetTimeSinceLastSave() { return m_fTimeSinceLastSave; }
}
