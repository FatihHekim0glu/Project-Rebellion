// ============================================================================
// PROJECT REBELLION - Persistence Data Structures
// ============================================================================

class RBL_ZoneSaveData
{
	string ZoneID;
	ERBLFactionKey Owner;
	int Support;
	ERBLAlertState AlertState;
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
	ERBLItemAvailability Availability;
}

class RBL_PlayerSaveData
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
	
	void RBL_CampaignInfoData()
	{
		SaveVersion = 1;
	}
}

class RBL_ActiveMissionData
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
// PERSISTENCE MANAGER
// ============================================================================
class RBL_PersistenceManager
{
	protected static ref RBL_PersistenceManager s_Instance;
	
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
		
		m_bSaveInProgress = true;
		
		CollectCampaignState();
		
		// Simplified save - actual implementation would use proper JSON
		PrintFormat("[RBL] Campaign saved. Zones: %1", m_CurrentSaveData.Zones.Count());
		
		m_fTimeSinceLastSave = 0;
		m_bSaveInProgress = false;
		
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
	}
	
	bool LoadCampaign()
	{
		PrintFormat("[RBL] Load campaign not yet implemented");
		return false;
	}
	
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
		return false; // Simplified
	}
	
	void DeleteSave()
	{
		// Simplified
	}
	
	RBL_CampaignSaveData GetCurrentSaveData()
	{
		return m_CurrentSaveData;
	}
}
