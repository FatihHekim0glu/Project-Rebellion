// ============================================================================
// PROJECT REBELLION - Persistence Data Structures
// JSON-serializable structures for saving/loading campaign state
// ============================================================================

class RBL_ZoneSaveData : JsonApiStruct
{
	string ZoneID;
	ERBLFactionKey Owner;
	int Support;
	ERBLAlertState AlertState;
	ref array<string> GarrisonUnits;
	
	void RBL_ZoneSaveData()
	{
		GarrisonUnits = new array<string>();
		RegV("ZoneID");
		RegV("Owner");
		RegV("Support");
		RegV("AlertState");
		RegV("GarrisonUnits");
	}
}

class RBL_ArsenalItemData : JsonApiStruct
{
	string ItemPrefab;
	int Count;
	ERBLItemAvailability Availability;
	
	void RBL_ArsenalItemData()
	{
		RegV("ItemPrefab");
		RegV("Count");
		RegV("Availability");
	}
}

class RBL_PlayerSaveData : JsonApiStruct
{
	string PlayerUID;
	string PlayerName;
	int Kills;
	int Deaths;
	int ZonesCaptured;
	float PlayTime;
	vector LastPosition;
	ref array<string> LastLoadout;
	
	void RBL_PlayerSaveData()
	{
		LastLoadout = new array<string>();
		RegV("PlayerUID");
		RegV("PlayerName");
		RegV("Kills");
		RegV("Deaths");
		RegV("ZonesCaptured");
		RegV("PlayTime");
		RegV("LastPosition");
		RegV("LastLoadout");
	}
}

class RBL_CampaignInfoData : JsonApiStruct
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
	
	void RBL_CampaignInfoData()
	{
		SaveVersion = 1;
		RegV("WarLevel");
		RegV("Aggression");
		RegV("Money");
		RegV("HumanResources");
		RegV("TimeOfDay");
		RegV("DayNumber");
		RegV("TotalPlayTime");
		RegV("CurrentWeather");
		RegV("SaveVersion");
	}
}

class RBL_ActiveMissionData : JsonApiStruct
{
	string MissionID;
	ERBLQRFType MissionType;
	string TargetZoneID;
	string SourceZoneID;
	vector CurrentPosition;
	ref array<string> AssignedUnits;
	float TimeStarted;
	
	void RBL_ActiveMissionData()
	{
		AssignedUnits = new array<string>();
		RegV("MissionID");
		RegV("MissionType");
		RegV("TargetZoneID");
		RegV("SourceZoneID");
		RegV("CurrentPosition");
		RegV("AssignedUnits");
		RegV("TimeStarted");
	}
}

// ============================================================================
// MASTER SAVE STRUCTURE
// ============================================================================
class RBL_CampaignSaveData : JsonApiStruct
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
		
		RegV("CampaignInfo");
		RegV("Zones");
		RegV("Arsenal");
		RegV("Players");
		RegV("ActiveMissions");
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
// PERSISTENCE MANAGER
// ============================================================================
class RBL_PersistenceManager
{
	protected static RBL_PersistenceManager s_Instance;
	
	protected const string SAVE_FILE_NAME = "RBL_Campaign_Save";
	protected const int AUTO_SAVE_INTERVAL = 300;
	
	protected ref RBL_CampaignSaveData m_CurrentSaveData;
	protected float m_fTimeSinceLastSave;
	protected bool m_bSaveInProgress;
	
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
	}
	
	bool SaveCampaign()
	{
		if (m_bSaveInProgress)
			return false;
		
		if (!Replication.IsServer())
			return false;
		
		m_bSaveInProgress = true;
		
		CollectCampaignState();
		
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		
		if (!saveContext.WriteValue("", m_CurrentSaveData))
		{
			PrintFormat("[RBL] ERROR: Failed to serialize campaign data");
			m_bSaveInProgress = false;
			return false;
		}
		
		string jsonData = saveContext.ExportToString();
		
		FileHandle file = FileIO.OpenFile(GetSavePath(), FileMode.WRITE);
		if (!file)
		{
			PrintFormat("[RBL] ERROR: Failed to open save file for writing");
			m_bSaveInProgress = false;
			return false;
		}
		
		file.Write(jsonData);
		file.Close();
		
		m_fTimeSinceLastSave = 0;
		m_bSaveInProgress = false;
		
		PrintFormat("[RBL] Campaign saved successfully. Zones: %1, Arsenal Items: %2",
			m_CurrentSaveData.Zones.Count(),
			m_CurrentSaveData.Arsenal.Count()
		);
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
			campaignMgr.OnCampaignEvent(ERBLCampaignEvent.SAVE_TRIGGERED, null);
		
		return true;
	}
	
	protected void CollectCampaignState()
	{
		m_CurrentSaveData.ClearAll();
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			m_CurrentSaveData.CampaignInfo.WarLevel = campaignMgr.GetWarLevel();
			m_CurrentSaveData.CampaignInfo.Aggression = campaignMgr.GetAggression();
			m_CurrentSaveData.CampaignInfo.DayNumber = campaignMgr.GetDayNumber();
			m_CurrentSaveData.CampaignInfo.TotalPlayTime = campaignMgr.GetTotalPlayTime();
		}
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			m_CurrentSaveData.CampaignInfo.Money = econMgr.GetMoney();
			m_CurrentSaveData.CampaignInfo.HumanResources = econMgr.GetHR();
			
			map<string, int> arsenalItems = econMgr.GetArsenalInventory();
			array<string> keys = new array<string>();
			arsenalItems.GetKeyArray(keys);
			
			for (int i = 0; i < keys.Count(); i++)
			{
				string prefab = keys[i];
				int count = arsenalItems.Get(prefab);
				RBL_ArsenalItemData itemData = new RBL_ArsenalItemData();
				itemData.ItemPrefab = prefab;
				itemData.Count = count;
				itemData.Availability = econMgr.GetItemAvailability(prefab);
				m_CurrentSaveData.Arsenal.Insert(itemData);
			}
		}
		
		TimeAndWeatherManagerEntity timeManager = GetGame().GetTimeAndWeatherManager();
		if (timeManager)
		{
			m_CurrentSaveData.CampaignInfo.TimeOfDay = timeManager.GetTime().GetHours() * 100 + timeManager.GetTime().GetMinutes();
		}
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			array<RBL_CampaignZone> allZones = zoneMgr.GetAllZones();
			for (int i = 0; i < allZones.Count(); i++)
			{
				RBL_ZoneSaveData zoneData;
				allZones[i].SerializeToStruct(zoneData);
				m_CurrentSaveData.Zones.Insert(zoneData);
			}
		}
		
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
		{
			array<ref RBL_ActiveMissionData> missions = commanderAI.GetActiveMissionData();
			for (int i = 0; i < missions.Count(); i++)
			{
				m_CurrentSaveData.ActiveMissions.Insert(missions[i]);
			}
		}
	}
	
	bool LoadCampaign()
	{
		if (!Replication.IsServer())
			return false;
		
		string savePath = GetSavePath();
		
		if (!FileIO.FileExists(savePath))
		{
			PrintFormat("[RBL] No save file found at: %1", savePath);
			return false;
		}
		
		FileHandle file = FileIO.OpenFile(savePath, FileMode.READ);
		if (!file)
		{
			PrintFormat("[RBL] ERROR: Failed to open save file for reading");
			return false;
		}
		
		string jsonData;
		file.Read(jsonData, file.GetLength());
		file.Close();
		
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		if (!loadContext.ImportFromString(jsonData))
		{
			PrintFormat("[RBL] ERROR: Failed to parse save JSON");
			return false;
		}
		
		m_CurrentSaveData = new RBL_CampaignSaveData();
		if (!loadContext.ReadValue("", m_CurrentSaveData))
		{
			PrintFormat("[RBL] ERROR: Failed to deserialize campaign data");
			return false;
		}
		
		ApplyCampaignState();
		
		PrintFormat("[RBL] Campaign loaded successfully. Zones: %1, Arsenal Items: %2",
			m_CurrentSaveData.Zones.Count(),
			m_CurrentSaveData.Arsenal.Count()
		);
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
			campaignMgr.OnCampaignEvent(ERBLCampaignEvent.LOAD_TRIGGERED, null);
		
		return true;
	}
	
	protected void ApplyCampaignState()
	{
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			campaignMgr.SetWarLevel(m_CurrentSaveData.CampaignInfo.WarLevel);
			campaignMgr.SetAggression(m_CurrentSaveData.CampaignInfo.Aggression);
			campaignMgr.SetDayNumber(m_CurrentSaveData.CampaignInfo.DayNumber);
		}
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			econMgr.SetMoney(m_CurrentSaveData.CampaignInfo.Money);
			econMgr.SetHR(m_CurrentSaveData.CampaignInfo.HumanResources);
			
			for (int i = 0; i < m_CurrentSaveData.Arsenal.Count(); i++)
			{
				RBL_ArsenalItemData itemData = m_CurrentSaveData.Arsenal[i];
				econMgr.SetArsenalItemCount(itemData.ItemPrefab, itemData.Count);
			}
		}
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			for (int i = 0; i < m_CurrentSaveData.Zones.Count(); i++)
			{
				RBL_ZoneSaveData zoneData = m_CurrentSaveData.Zones[i];
				RBL_CampaignZone zone = zoneMgr.GetZoneByID(zoneData.ZoneID);
				if (zone)
					zone.DeserializeFromStruct(zoneData);
			}
		}
		
		TimeAndWeatherManagerEntity timeManager = GetGame().GetTimeAndWeatherManager();
		if (timeManager)
		{
			int hours = m_CurrentSaveData.CampaignInfo.TimeOfDay / 100;
			int minutes = m_CurrentSaveData.CampaignInfo.TimeOfDay % 100;
			timeManager.SetTime(new TimeContainer(hours, minutes, 0));
		}
	}
	
	void Update(float timeSlice)
	{
		if (!Replication.IsServer())
			return;
		
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
	
	protected string GetSavePath()
	{
		return "$profile:" + SAVE_FILE_NAME + ".json";
	}
	
	bool HasExistingSave()
	{
		return FileIO.FileExists(GetSavePath());
	}
	
	void DeleteSave()
	{
		if (HasExistingSave())
			FileIO.DeleteFile(GetSavePath());
	}
	
	RBL_CampaignSaveData GetCurrentSaveData()
	{
		return m_CurrentSaveData;
	}
}
