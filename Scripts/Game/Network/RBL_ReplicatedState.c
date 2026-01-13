// ============================================================================
// PROJECT REBELLION - Replicated State
// Shared game state that replicates from server to all clients
// ============================================================================

class RBL_ReplicatedStateClass : GenericEntityClass
{
}

class RBL_ReplicatedState : GenericEntity
{
	protected static RBL_ReplicatedState s_Instance;
	
	protected RplComponent m_RplComponent;
	protected bool m_bIsAuthority;
	protected bool m_bInitialized;
	
	// ========================================================================
	// REPLICATED ECONOMY STATE
	// ========================================================================
	
	[RplProp(onRplName: "OnRpl_Money")]
	protected int m_iMoney;
	
	[RplProp(onRplName: "OnRpl_HR")]
	protected int m_iHumanResources;
	
	// ========================================================================
	// REPLICATED CAMPAIGN STATE
	// ========================================================================
	
	[RplProp(onRplName: "OnRpl_WarLevel")]
	protected int m_iWarLevel;
	
	[RplProp(onRplName: "OnRpl_Aggression")]
	protected int m_iAggression;
	
	[RplProp(onRplName: "OnRpl_DayNumber")]
	protected int m_iDayNumber;
	
	[RplProp(onRplName: "OnRpl_CampaignActive")]
	protected bool m_bCampaignActive;
	
	// ========================================================================
	// REPLICATED ZONE STATES (packed as strings for efficiency)
	// ========================================================================
	
	[RplProp(onRplName: "OnRpl_ZoneOwnership")]
	protected string m_sZoneOwnershipData;
	
	[RplProp(onRplName: "OnRpl_ZoneSupport")]
	protected string m_sZoneSupportData;
	
	// ========================================================================
	// REPLICATED MISSION STATE
	// ========================================================================
	
	[RplProp(onRplName: "OnRpl_ActiveMissions")]
	protected string m_sActiveMissionsData;
	
	[RplProp(onRplName: "OnRpl_AvailableMissions")]
	protected string m_sAvailableMissionsData;
	
	// ========================================================================
	// REPLICATED VICTORY STATE
	// ========================================================================
	
	[RplProp(onRplName: "OnRpl_VictoryState")]
	protected int m_iVictoryState;
	
	[RplProp(onRplName: "OnRpl_PlayerDeaths")]
	protected int m_iPlayerDeaths;
	
	// ========================================================================
	// EVENTS
	// ========================================================================
	
	protected ref ScriptInvoker m_OnMoneyChanged;
	protected ref ScriptInvoker m_OnHRChanged;
	protected ref ScriptInvoker m_OnWarLevelChanged;
	protected ref ScriptInvoker m_OnAggressionChanged;
	protected ref ScriptInvoker m_OnZoneOwnershipChanged;
	protected ref ScriptInvoker m_OnMissionsChanged;
	protected ref ScriptInvoker m_OnVictoryStateChanged;
	
	// ========================================================================
	// INITIALIZATION
	// ========================================================================
	
	static RBL_ReplicatedState GetInstance()
	{
		return s_Instance;
	}
	
	void RBL_ReplicatedState(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
		
		m_OnMoneyChanged = new ScriptInvoker();
		m_OnHRChanged = new ScriptInvoker();
		m_OnWarLevelChanged = new ScriptInvoker();
		m_OnAggressionChanged = new ScriptInvoker();
		m_OnZoneOwnershipChanged = new ScriptInvoker();
		m_OnMissionsChanged = new ScriptInvoker();
		m_OnVictoryStateChanged = new ScriptInvoker();
		
		m_bInitialized = false;
		m_iVictoryState = 0;
		m_bCampaignActive = true;
		
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		UpdateAuthorityState();
		
		m_bInitialized = true;
		
		PrintFormat("[RBL_RplState] Initialized. Authority: %1", m_bIsAuthority);
	}
	
	protected float m_fSyncTimer;
	protected const float SYNC_INTERVAL = 0.5;
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bInitialized || !m_bIsAuthority)
			return;
		
		m_fSyncTimer += timeSlice;
		if (m_fSyncTimer >= SYNC_INTERVAL)
		{
			m_fSyncTimer = 0;
			SyncFromLocalManagers();
		}
	}
	
	protected void UpdateAuthorityState()
	{
		if (m_RplComponent)
			m_bIsAuthority = m_RplComponent.IsMaster();
		else
			m_bIsAuthority = RBL_NetworkUtils.IsSinglePlayer();
	}
	
	bool IsAuthority()
	{
		return m_bIsAuthority;
	}
	
	// ========================================================================
	// SERVER: SYNC FROM LOCAL MANAGERS
	// ========================================================================
	
	protected void SyncFromLocalManagers()
	{
		if (!m_bIsAuthority)
			return;
		
		bool needsBump = false;
		
		// Sync Economy
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			int newMoney = econMgr.GetMoney();
			int newHR = econMgr.GetHR();
			
			if (newMoney != m_iMoney)
			{
				m_iMoney = newMoney;
				needsBump = true;
			}
			if (newHR != m_iHumanResources)
			{
				m_iHumanResources = newHR;
				needsBump = true;
			}
		}
		
		// Sync Campaign
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (campMgr)
		{
			int newWarLevel = campMgr.GetWarLevel();
			int newAggression = campMgr.GetAggression();
			int newDay = campMgr.GetDayNumber();
			bool newActive = campMgr.IsCampaignActive();
			
			if (newWarLevel != m_iWarLevel)
			{
				m_iWarLevel = newWarLevel;
				needsBump = true;
			}
			if (newAggression != m_iAggression)
			{
				m_iAggression = newAggression;
				needsBump = true;
			}
			if (newDay != m_iDayNumber)
			{
				m_iDayNumber = newDay;
				needsBump = true;
			}
			if (newActive != m_bCampaignActive)
			{
				m_bCampaignActive = newActive;
				needsBump = true;
			}
		}
		
		// Sync Victory
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (victoryMgr)
		{
			int newDeaths = victoryMgr.GetPlayerDeaths();
			int newState = 0;
			if (victoryMgr.IsVictory())
				newState = 1;
			else if (victoryMgr.IsDefeat())
				newState = 2;
			
			if (newDeaths != m_iPlayerDeaths)
			{
				m_iPlayerDeaths = newDeaths;
				needsBump = true;
			}
			if (newState != m_iVictoryState)
			{
				m_iVictoryState = newState;
				needsBump = true;
			}
		}
		
		// Sync Zone Ownership (less frequently)
		string newZoneData = BuildZoneOwnershipString();
		if (newZoneData != m_sZoneOwnershipData)
		{
			m_sZoneOwnershipData = newZoneData;
			needsBump = true;
		}
		
		if (needsBump)
			Replication.BumpMe();
	}
	
	protected string BuildZoneOwnershipString()
	{
		string result = "";
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return result;
		
		array<ref RBL_VirtualZone> zones = zoneMgr.GetAllVirtualZones();
		for (int i = 0; i < zones.Count(); i++)
		{
			RBL_VirtualZone zone = zones[i];
			if (!zone)
				continue;
			
			if (result.Length() > 0)
				result += ";";
			
			result += zone.GetZoneID() + ":" + zone.GetOwnerFaction().ToString();
		}
		
		return result;
	}
	
	// ========================================================================
	// CLIENT: REPLICATION CALLBACKS
	// ========================================================================
	
	protected void OnRpl_Money()
	{
		if (!m_bIsAuthority)
		{
			RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
			if (econMgr)
				econMgr.SetMoney(m_iMoney);
		}
		m_OnMoneyChanged.Invoke(m_iMoney);
	}
	
	protected void OnRpl_HR()
	{
		if (!m_bIsAuthority)
		{
			RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
			if (econMgr)
				econMgr.SetHR(m_iHumanResources);
		}
		m_OnHRChanged.Invoke(m_iHumanResources);
	}
	
	protected void OnRpl_WarLevel()
	{
		m_OnWarLevelChanged.Invoke(m_iWarLevel);
	}
	
	protected void OnRpl_Aggression()
	{
		m_OnAggressionChanged.Invoke(m_iAggression);
	}
	
	protected void OnRpl_DayNumber()
	{
		// Day number changed
	}
	
	protected void OnRpl_CampaignActive()
	{
		// Campaign active state changed
	}
	
	protected void OnRpl_ZoneOwnership()
	{
		if (!m_bIsAuthority)
			ApplyZoneOwnershipFromString(m_sZoneOwnershipData);
		
		m_OnZoneOwnershipChanged.Invoke();
	}
	
	protected void ApplyZoneOwnershipFromString(string data)
	{
		if (data.IsEmpty())
			return;
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		array<string> entries = new array<string>();
		data.Split(";", entries, false);
		
		for (int i = 0; i < entries.Count(); i++)
		{
			array<string> parts = new array<string>();
			entries[i].Split(":", parts, false);
			
			if (parts.Count() != 2)
				continue;
			
			string zoneID = parts[0];
			int owner = parts[1].ToInt();
			
			RBL_VirtualZone zone = zoneMgr.GetVirtualZoneByID(zoneID);
			if (zone)
				zone.SetOwnerFaction(owner);
		}
	}
	
	protected void OnRpl_ZoneSupport()
	{
		// Zone support data changed
	}
	
	protected void OnRpl_ActiveMissions()
	{
		m_OnMissionsChanged.Invoke();
	}
	
	protected void OnRpl_AvailableMissions()
	{
		m_OnMissionsChanged.Invoke();
	}
	
	protected void OnRpl_VictoryState()
	{
		m_OnVictoryStateChanged.Invoke(m_iVictoryState);
		
		if (!m_bIsAuthority)
		{
			RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
			if (victoryMgr)
			{
				if (m_iVictoryState == 1)
					victoryMgr.TriggerVictory();
				else if (m_iVictoryState == 2)
					victoryMgr.TriggerDefeat();
			}
		}
	}
	
	protected void OnRpl_PlayerDeaths()
	{
		// Player deaths changed
	}
	
	// ========================================================================
	// GETTERS
	// ========================================================================
	
	int GetMoney() { return m_iMoney; }
	int GetHR() { return m_iHumanResources; }
	int GetWarLevel() { return m_iWarLevel; }
	int GetAggression() { return m_iAggression; }
	int GetDayNumber() { return m_iDayNumber; }
	bool IsCampaignActive() { return m_bCampaignActive; }
	int GetVictoryState() { return m_iVictoryState; }
	int GetPlayerDeaths() { return m_iPlayerDeaths; }
	
	// ========================================================================
	// SETTERS (Server only)
	// ========================================================================
	
	void SetMoney(int value)
	{
		if (!m_bIsAuthority)
			return;
		
		m_iMoney = value;
		Replication.BumpMe();
	}
	
	void SetHR(int value)
	{
		if (!m_bIsAuthority)
			return;
		
		m_iHumanResources = value;
		Replication.BumpMe();
	}
	
	void SetWarLevel(int value)
	{
		if (!m_bIsAuthority)
			return;
		
		m_iWarLevel = value;
		Replication.BumpMe();
	}
	
	void SetAggression(int value)
	{
		if (!m_bIsAuthority)
			return;
		
		m_iAggression = value;
		Replication.BumpMe();
	}
	
	void SetVictoryState(int state)
	{
		if (!m_bIsAuthority)
			return;
		
		m_iVictoryState = state;
		Replication.BumpMe();
	}
	
	// ========================================================================
	// EVENT ACCESSORS
	// ========================================================================
	
	ScriptInvoker GetOnMoneyChanged() { return m_OnMoneyChanged; }
	ScriptInvoker GetOnHRChanged() { return m_OnHRChanged; }
	ScriptInvoker GetOnWarLevelChanged() { return m_OnWarLevelChanged; }
	ScriptInvoker GetOnAggressionChanged() { return m_OnAggressionChanged; }
	ScriptInvoker GetOnZoneOwnershipChanged() { return m_OnZoneOwnershipChanged; }
	ScriptInvoker GetOnMissionsChanged() { return m_OnMissionsChanged; }
	ScriptInvoker GetOnVictoryStateChanged() { return m_OnVictoryStateChanged; }
	
	// ========================================================================
	// DEBUG
	// ========================================================================
	
	void PrintStatus()
	{
		PrintFormat("[RBL_RplState] === REPLICATED STATE ===");
		PrintFormat("  Authority: %1", m_bIsAuthority);
		PrintFormat("  Money: %1", m_iMoney);
		PrintFormat("  HR: %1", m_iHumanResources);
		PrintFormat("  War Level: %1", m_iWarLevel);
		PrintFormat("  Aggression: %1", m_iAggression);
		PrintFormat("  Day: %1", m_iDayNumber);
		PrintFormat("  Campaign Active: %1", m_bCampaignActive);
		PrintFormat("  Victory State: %1", m_iVictoryState);
		PrintFormat("  Player Deaths: %1", m_iPlayerDeaths);
	}
}

// ============================================================================
// ZONE REPLICATED STATE - Per-zone network data
// ============================================================================
class RBL_ZoneNetworkState
{
	string ZoneID;
	int OwnerFaction;
	int CivilianSupport;
	float CaptureProgress;
	int CapturingFaction;
	int GarrisonStrength;
	int AlertState;
	
	void RBL_ZoneNetworkState()
	{
		OwnerFaction = 0;
		CivilianSupport = 50;
		CaptureProgress = 0;
		CapturingFaction = 0;
		GarrisonStrength = 0;
		AlertState = 0;
	}
	
	string Serialize()
	{
		return string.Format("%1,%2,%3,%4,%5,%6,%7",
			ZoneID, OwnerFaction, CivilianSupport, CaptureProgress,
			CapturingFaction, GarrisonStrength, AlertState);
	}
	
	bool Deserialize(string data)
	{
		array<string> parts = new array<string>();
		data.Split(",", parts, false);
		
		if (parts.Count() != 7)
			return false;
		
		ZoneID = parts[0];
		OwnerFaction = parts[1].ToInt();
		CivilianSupport = parts[2].ToInt();
		CaptureProgress = parts[3].ToFloat();
		CapturingFaction = parts[4].ToInt();
		GarrisonStrength = parts[5].ToInt();
		AlertState = parts[6].ToInt();
		
		return true;
	}
}

// ============================================================================
// MISSION REPLICATED STATE - Mission network data
// ============================================================================
class RBL_MissionNetworkState
{
	string MissionID;
	string MissionName;
	int MissionType;
	int Status;
	float Progress;
	int RewardMoney;
	int RewardHR;
	
	string Serialize()
	{
		return string.Format("%1|%2|%3|%4|%5|%6|%7",
			MissionID, MissionName, MissionType, Status, Progress, RewardMoney, RewardHR);
	}
	
	bool Deserialize(string data)
	{
		array<string> parts = new array<string>();
		data.Split("|", parts, false);
		
		if (parts.Count() != 7)
			return false;
		
		MissionID = parts[0];
		MissionName = parts[1];
		MissionType = parts[2].ToInt();
		Status = parts[3].ToInt();
		Progress = parts[4].ToFloat();
		RewardMoney = parts[5].ToInt();
		RewardHR = parts[6].ToInt();
		
		return true;
	}
}

