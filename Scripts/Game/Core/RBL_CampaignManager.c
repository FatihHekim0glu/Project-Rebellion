// ============================================================================
// PROJECT REBELLION - Campaign Manager
// The "Brain" - Handles main loop, global state, and system coordination
// ============================================================================

class RBL_CampaignManagerClass : GenericEntityClass
{
}

class RBL_CampaignManager : GenericEntity
{
	protected static RBL_CampaignManager s_Instance;
	
	// ========================================================================
	// CONFIGURATION
	// ========================================================================
	
	[Attribute("1", UIWidgets.Slider, "Starting War Level", "1 10 1")]
	protected int m_iStartingWarLevel;
	
	[Attribute("25", UIWidgets.Slider, "Starting Aggression", "0 100 1")]
	protected int m_iStartingAggression;
	
	[Attribute("500", UIWidgets.EditBox, "Starting Money")]
	protected int m_iStartingMoney;
	
	[Attribute("10", UIWidgets.EditBox, "Starting Human Resources")]
	protected int m_iStartingHR;
	
	[Attribute("600", UIWidgets.EditBox, "Resource tick interval (seconds)")]
	protected float m_fResourceTickInterval;
	
	// ========================================================================
	// STATE
	// ========================================================================
	
	protected int m_iWarLevel;
	protected int m_iAggression;
	protected int m_iDayNumber;
	protected float m_fTotalPlayTime;
	protected float m_fTimeSinceResourceTick;
	protected bool m_bCampaignActive;
	protected bool m_bInitialized;
	
	// Aggression decay configuration
	protected const float AGGRESSION_DECAY_RATE = 0.5;    // Per minute
	protected const int AGGRESSION_KILL_INCREASE = 2;
	protected const int AGGRESSION_ZONE_CAPTURE = 15;
	
	// War level escalation thresholds
	protected ref array<int> m_aWarLevelThresholds;
	protected int m_iEnemyKillCount;
	protected int m_iZonesCaptured;
	
	// ========================================================================
	// SIGNALS
	// ========================================================================
	
	protected ref ScriptInvoker m_OnWarLevelChanged;
	protected ref ScriptInvoker m_OnAggressionChanged;
	protected ref ScriptInvoker m_OnCampaignEvent;
	protected ref ScriptInvoker m_OnDayChanged;
	
	// ========================================================================
	// GARRISON TEMPLATES
	// ========================================================================
	
	protected ref map<ERBLZoneType, ref array<string>> m_mGarrisonTemplates;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_CampaignManager GetInstance()
	{
		return s_Instance;
	}
	
	// ========================================================================
	// INITIALIZATION
	// ========================================================================
	
	void RBL_CampaignManager(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
		
		m_OnWarLevelChanged = new ScriptInvoker();
		m_OnAggressionChanged = new ScriptInvoker();
		m_OnCampaignEvent = new ScriptInvoker();
		m_OnDayChanged = new ScriptInvoker();
		
		m_aWarLevelThresholds = {};
		InitializeWarLevelThresholds();
		InitializeGarrisonTemplates();
		
		m_bCampaignActive = false;
		m_bInitialized = false;
		
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!Replication.IsServer())
			return;
		
		// Check for existing save
		RBL_PersistenceManager persistence = RBL_PersistenceManager.GetInstance();
		
		if (persistence && persistence.HasExistingSave())
		{
			PrintFormat("[RBL] Found existing save. Loading...");
			persistence.LoadCampaign();
		}
		else
		{
			PrintFormat("[RBL] Starting new campaign.");
			StartNewCampaign();
		}
		
		m_bInitialized = true;
	}
	
	protected void InitializeWarLevelThresholds()
	{
		// Kills + Zones captured needed for each war level
		m_aWarLevelThresholds.Insert(0);      // Level 1: Start
		m_aWarLevelThresholds.Insert(20);     // Level 2
		m_aWarLevelThresholds.Insert(50);     // Level 3
		m_aWarLevelThresholds.Insert(100);    // Level 4
		m_aWarLevelThresholds.Insert(175);    // Level 5
		m_aWarLevelThresholds.Insert(275);    // Level 6
		m_aWarLevelThresholds.Insert(400);    // Level 7
		m_aWarLevelThresholds.Insert(550);    // Level 8
		m_aWarLevelThresholds.Insert(750);    // Level 9
		m_aWarLevelThresholds.Insert(1000);   // Level 10
	}
	
	protected void InitializeGarrisonTemplates()
	{
		m_mGarrisonTemplates = new map<ERBLZoneType, ref array<string>>();
		
		// Placeholder prefab paths - would be actual resource paths
		string rifleman = "{ABC123}Prefabs/Characters/OPFOR/Rifleman.et";
		string grenadier = "{ABC124}Prefabs/Characters/OPFOR/Grenadier.et";
		string medic = "{ABC125}Prefabs/Characters/OPFOR/Medic.et";
		string mg = "{ABC126}Prefabs/Characters/OPFOR/Machinegunner.et";
		string officer = "{ABC127}Prefabs/Characters/OPFOR/Officer.et";
		string sniper = "{ABC128}Prefabs/Characters/OPFOR/Marksman.et";
		
		// Outpost garrison
		array<string> outpostGarrison = {};
		outpostGarrison.Insert(officer);
		outpostGarrison.Insert(rifleman);
		outpostGarrison.Insert(rifleman);
		outpostGarrison.Insert(rifleman);
		outpostGarrison.Insert(mg);
		outpostGarrison.Insert(medic);
		m_mGarrisonTemplates.Set(ERBLZoneType.Outpost, outpostGarrison);
		
		// Airbase garrison (larger)
		array<string> airbaseGarrison = {};
		airbaseGarrison.Insert(officer);
		airbaseGarrison.Insert(officer);
		for (int i = 0; i < 8; i++)
			airbaseGarrison.Insert(rifleman);
		airbaseGarrison.Insert(mg);
		airbaseGarrison.Insert(mg);
		airbaseGarrison.Insert(grenadier);
		airbaseGarrison.Insert(grenadier);
		airbaseGarrison.Insert(medic);
		airbaseGarrison.Insert(medic);
		airbaseGarrison.Insert(sniper);
		m_mGarrisonTemplates.Set(ERBLZoneType.Airbase, airbaseGarrison);
		
		// Town garrison (small, checkpoint)
		array<string> townGarrison = {};
		townGarrison.Insert(rifleman);
		townGarrison.Insert(rifleman);
		m_mGarrisonTemplates.Set(ERBLZoneType.Town, townGarrison);
		
		// Resource garrison
		array<string> resourceGarrison = {};
		resourceGarrison.Insert(rifleman);
		resourceGarrison.Insert(rifleman);
		resourceGarrison.Insert(rifleman);
		resourceGarrison.Insert(mg);
		m_mGarrisonTemplates.Set(ERBLZoneType.Resource, resourceGarrison);
		
		// Factory garrison
		array<string> factoryGarrison = {};
		factoryGarrison.Insert(officer);
		factoryGarrison.Insert(rifleman);
		factoryGarrison.Insert(rifleman);
		factoryGarrison.Insert(rifleman);
		factoryGarrison.Insert(rifleman);
		factoryGarrison.Insert(mg);
		factoryGarrison.Insert(medic);
		m_mGarrisonTemplates.Set(ERBLZoneType.Factory, factoryGarrison);
		
		// Roadblock garrison (minimal)
		array<string> roadblockGarrison = {};
		roadblockGarrison.Insert(rifleman);
		roadblockGarrison.Insert(rifleman);
		m_mGarrisonTemplates.Set(ERBLZoneType.Roadblock, roadblockGarrison);
	}
	
	// ========================================================================
	// CAMPAIGN LIFECYCLE
	// ========================================================================
	
	void StartNewCampaign()
	{
		m_iWarLevel = m_iStartingWarLevel;
		m_iAggression = m_iStartingAggression;
		m_iDayNumber = 1;
		m_fTotalPlayTime = 0;
		m_fTimeSinceResourceTick = 0;
		m_iEnemyKillCount = 0;
		m_iZonesCaptured = 0;
		m_bCampaignActive = true;
		
		// Initialize economy
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			econMgr.SetMoney(m_iStartingMoney);
			econMgr.SetHR(m_iStartingHR);
		}
		
		// Spawn initial garrisons
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.SpawnAllGarrisons();
		
		PrintFormat("[RBL] New campaign started. War Level: %1, Aggression: %2", m_iWarLevel, m_iAggression);
	}
	
	void EndCampaign(bool victory)
	{
		m_bCampaignActive = false;
		
		if (victory)
		{
			PrintFormat("[RBL] VICTORY! Campaign completed in %1 days.", m_iDayNumber);
		}
		else
		{
			PrintFormat("[RBL] DEFEAT. Campaign ended on day %1.", m_iDayNumber);
		}
		
		// Could trigger end-game UI
	}
	
	// ========================================================================
	// MAIN LOOP
	// ========================================================================
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!Replication.IsServer() || !m_bCampaignActive || !m_bInitialized)
			return;
		
		m_fTotalPlayTime += timeSlice;
		
		// Update subsystems
		UpdateAggression(timeSlice);
		UpdateResourceTick(timeSlice);
		CheckWarLevelEscalation();
		CheckDayProgression();
		
		// Update other managers
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
			commanderAI.Update(timeSlice);
		
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
			undercover.Update(timeSlice);
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.Update(timeSlice);
		
		RBL_PersistenceManager persistence = RBL_PersistenceManager.GetInstance();
		if (persistence)
			persistence.Update(timeSlice);
	}
	
	// ========================================================================
	// AGGRESSION SYSTEM
	// ========================================================================
	
	protected void UpdateAggression(float timeSlice)
	{
		// Decay aggression over time
		float decay = AGGRESSION_DECAY_RATE * (timeSlice / 60.0); // Per minute
		SetAggression(m_iAggression - Math.Round(decay));
	}
	
	void AddAggression(int amount)
	{
		SetAggression(m_iAggression + amount);
	}
	
	void SetAggression(int value)
	{
		int previousAggression = m_iAggression;
		m_iAggression = Math.Clamp(value, 0, 100);
		
		if (previousAggression != m_iAggression)
		{
			Replication.BumpMe();
			m_OnAggressionChanged.Invoke(m_iAggression);
		}
	}
	
	// ========================================================================
	// WAR LEVEL SYSTEM
	// ========================================================================
	
	protected void CheckWarLevelEscalation()
	{
		if (m_iWarLevel >= 10)
			return;
		
		int totalScore = m_iEnemyKillCount + (m_iZonesCaptured * 10);
		int nextThreshold = m_aWarLevelThresholds[m_iWarLevel];
		
		if (totalScore >= nextThreshold)
		{
			SetWarLevel(m_iWarLevel + 1);
		}
	}
	
	void SetWarLevel(int level)
	{
		int previousLevel = m_iWarLevel;
		m_iWarLevel = Math.Clamp(level, 1, 10);
		
		if (previousLevel != m_iWarLevel)
		{
			PrintFormat("[RBL] War Level escalated: %1 -> %2", previousLevel, m_iWarLevel);
			
			Replication.BumpMe();
			m_OnWarLevelChanged.Invoke(m_iWarLevel);
			OnCampaignEvent(ERBLCampaignEvent.WAR_LEVEL_CHANGED, null);
		}
	}
	
	// ========================================================================
	// RESOURCE TICK
	// ========================================================================
	
	protected void UpdateResourceTick(float timeSlice)
	{
		m_fTimeSinceResourceTick += timeSlice;
		
		if (m_fTimeSinceResourceTick >= m_fResourceTickInterval)
		{
			m_fTimeSinceResourceTick = 0;
			ProcessResourceTick();
		}
	}
	
	protected void ProcessResourceTick()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		
		if (!zoneMgr || !econMgr)
			return;
		
		int totalMoney = 0;
		int totalHR = 0;
		
		array<RBL_CampaignZone> playerZones = zoneMgr.GetZonesByFaction(ERBLFactionKey.FIA);
		
		foreach (RBL_CampaignZone zone : playerZones)
		{
			totalMoney += zone.CalculateResourceIncome();
			totalHR += zone.CalculateHRIncome();
		}
		
		econMgr.AddMoney(totalMoney);
		econMgr.AddHR(totalHR);
		
		PrintFormat("[RBL] Resource tick: +%1 Money, +%2 HR from %3 zones",
			totalMoney, totalHR, playerZones.Count());
	}
	
	// ========================================================================
	// DAY PROGRESSION
	// ========================================================================
	
	protected void CheckDayProgression()
	{
		TimeAndWeatherManagerEntity timeManager = GetGame().GetTimeAndWeatherManager();
		if (!timeManager)
			return;
		
		// Check if new day (06:00)
		int hours = timeManager.GetTime().GetHours();
		
		// Simple day tracking based on hours passing 6:00
		// More complex implementation would track actual day changes
	}
	
	void SetDayNumber(int day)
	{
		if (m_iDayNumber != day)
		{
			m_iDayNumber = day;
			Replication.BumpMe();
			m_OnDayChanged.Invoke(m_iDayNumber);
		}
	}
	
	// ========================================================================
	// EVENT HANDLERS
	// ========================================================================
	
	void OnZoneCaptured(RBL_CampaignZone zone, ERBLFactionKey previousOwner, ERBLFactionKey newOwner)
	{
		if (newOwner == ERBLFactionKey.FIA)
		{
			// Player captured a zone
			m_iZonesCaptured++;
			AddAggression(AGGRESSION_ZONE_CAPTURE);
			
			PrintFormat("[RBL] Zone %1 captured by FIA. Aggression: %2", zone.GetZoneID(), m_iAggression);
			
			OnCampaignEvent(ERBLCampaignEvent.ZONE_CAPTURED, zone);
			
			// Check victory condition
			RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
			if (zoneMgr && zoneMgr.AreAllZonesCapturedByFaction(ERBLFactionKey.FIA))
			{
				EndCampaign(true);
			}
		}
		else if (previousOwner == ERBLFactionKey.FIA)
		{
			// Player lost a zone
			PrintFormat("[RBL] Zone %1 lost to %2", zone.GetZoneID(), typename.EnumToString(ERBLFactionKey, newOwner));
			
			OnCampaignEvent(ERBLCampaignEvent.ZONE_LOST, zone);
			
			// Check defeat condition (lost HQ)
			if (zone.GetZoneType() == ERBLZoneType.HQ)
			{
				EndCampaign(false);
			}
		}
	}
	
	void OnEnemyKilled()
	{
		m_iEnemyKillCount++;
		AddAggression(AGGRESSION_KILL_INCREASE);
		
		OnCampaignEvent(ERBLCampaignEvent.ENEMY_KILLED, null);
	}
	
	void OnPlayerKilled()
	{
		OnCampaignEvent(ERBLCampaignEvent.PLAYER_KILLED, null);
	}
	
	void OnCampaignEvent(ERBLCampaignEvent eventType, RBL_CampaignZone relatedZone)
	{
		m_OnCampaignEvent.Invoke(eventType, relatedZone);
	}
	
	// ========================================================================
	// GETTERS
	// ========================================================================
	
	int GetWarLevel() { return m_iWarLevel; }
	int GetAggression() { return m_iAggression; }
	int GetDayNumber() { return m_iDayNumber; }
	float GetTotalPlayTime() { return m_fTotalPlayTime; }
	bool IsCampaignActive() { return m_bCampaignActive; }
	
	ScriptInvoker GetOnWarLevelChanged() { return m_OnWarLevelChanged; }
	ScriptInvoker GetOnAggressionChanged() { return m_OnAggressionChanged; }
	ScriptInvoker GetOnCampaignEvent() { return m_OnCampaignEvent; }
	
	array<string> GetGarrisonTemplate(ERBLZoneType zoneType, ERBLFactionKey faction)
	{
		array<string> template;
		if (m_mGarrisonTemplates.Find(zoneType, template))
			return template;
		
		// Default empty
		return new array<string>();
	}
}

