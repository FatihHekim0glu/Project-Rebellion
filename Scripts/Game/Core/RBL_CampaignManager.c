// ============================================================================
// PROJECT REBELLION - Campaign Manager
// ============================================================================

class RBL_CampaignManagerClass : GenericEntityClass
{
}

class RBL_CampaignManager : GenericEntity
{
	protected static RBL_CampaignManager s_Instance;

	[Attribute("1", UIWidgets.Slider, "Starting War Level", "1 10 1")]
	protected int m_iStartingWarLevel = RBL_Config.STARTING_WAR_LEVEL;

	[Attribute("25", UIWidgets.Slider, "Starting Aggression", "0 100 1")]
	protected int m_iStartingAggression = RBL_Config.STARTING_AGGRESSION;

	[Attribute("500", UIWidgets.EditBox, "Starting Money")]
	protected int m_iStartingMoney = RBL_Config.STARTING_MONEY;

	[Attribute("10", UIWidgets.EditBox, "Starting Human Resources")]
	protected int m_iStartingHR = RBL_Config.STARTING_HR;

	[Attribute("600", UIWidgets.EditBox, "Resource tick interval (seconds)")]
	protected float m_fResourceTickInterval = RBL_Config.RESOURCE_TICK_INTERVAL;

	protected int m_iWarLevel;
	protected int m_iAggression;
	protected int m_iDayNumber;
	protected float m_fTotalPlayTime;
	protected float m_fTimeSinceResourceTick;
	protected bool m_bCampaignActive;
	protected bool m_bInitialized;

	// Using RBL_Config constants instead of hardcoded values

	protected ref array<int> m_aWarLevelThresholds;
	protected int m_iEnemyKillCount;
	protected int m_iZonesCaptured;

	protected ref ScriptInvoker m_OnWarLevelChanged;
	protected ref ScriptInvoker m_OnAggressionChanged;
	protected ref ScriptInvoker m_OnCampaignEvent;

	protected ref map<ERBLZoneType, ref array<string>> m_mGarrisonTemplates;

	static RBL_CampaignManager GetInstance()
	{
		return s_Instance;
	}

	void RBL_CampaignManager(IEntitySource src, IEntity parent)
	{
		s_Instance = this;

		m_OnWarLevelChanged = new ScriptInvoker();
		m_OnAggressionChanged = new ScriptInvoker();
		m_OnCampaignEvent = new ScriptInvoker();

		m_aWarLevelThresholds = new array<int>();
		InitializeWarLevelThresholds();
		InitializeGarrisonTemplates();

		m_bCampaignActive = false;
		m_bInitialized = false;

		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		PrintFormat("[RBL] CampaignManager initialized");

		StartNewCampaign();
		m_bInitialized = true;
	}

	protected void InitializeWarLevelThresholds()
	{
		// Using RBL_Config constants for maintainability
		m_aWarLevelThresholds.Insert(0);
		m_aWarLevelThresholds.Insert(RBL_Config.WAR_LEVEL_2_THRESHOLD);
		m_aWarLevelThresholds.Insert(RBL_Config.WAR_LEVEL_3_THRESHOLD);
		m_aWarLevelThresholds.Insert(RBL_Config.WAR_LEVEL_4_THRESHOLD);
		m_aWarLevelThresholds.Insert(RBL_Config.WAR_LEVEL_5_THRESHOLD);
		m_aWarLevelThresholds.Insert(RBL_Config.WAR_LEVEL_6_THRESHOLD);
		m_aWarLevelThresholds.Insert(RBL_Config.WAR_LEVEL_7_THRESHOLD);
		m_aWarLevelThresholds.Insert(RBL_Config.WAR_LEVEL_8_THRESHOLD);
		m_aWarLevelThresholds.Insert(RBL_Config.WAR_LEVEL_9_THRESHOLD);
		m_aWarLevelThresholds.Insert(RBL_Config.WAR_LEVEL_10_THRESHOLD);
	}

	protected void InitializeGarrisonTemplates()
	{
		m_mGarrisonTemplates = new map<ERBLZoneType, ref array<string>>();
	}

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

		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			econMgr.SetMoney(m_iStartingMoney);
			econMgr.SetHR(m_iStartingHR);
		}

		PrintFormat("[RBL] New campaign started. War Level: %1, Aggression: %2", m_iWarLevel, m_iAggression);
	}

	void EndCampaign(bool victory)
	{
		m_bCampaignActive = false;

		if (victory)
			PrintFormat("[RBL] VICTORY! Campaign completed in %1 days.", m_iDayNumber);
		else
			PrintFormat("[RBL] DEFEAT. Campaign ended on day %1.", m_iDayNumber);
	}

	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bCampaignActive || !m_bInitialized)
			return;

		m_fTotalPlayTime += timeSlice;

		UpdateAggression(timeSlice);
		UpdateResourceTick(timeSlice);
		CheckWarLevelEscalation();
	}

	protected void UpdateAggression(float timeSlice)
	{
		float decay = RBL_Config.AGGRESSION_DECAY_PER_MINUTE * (timeSlice / 60.0);
		SetAggression(m_iAggression - decay);
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
			m_OnAggressionChanged.Invoke(m_iAggression);
	}

	protected void CheckWarLevelEscalation()
	{
		if (m_iWarLevel >= 10)
			return;

		int totalScore = m_iEnemyKillCount + (m_iZonesCaptured * 10);

		if (m_iWarLevel < m_aWarLevelThresholds.Count())
		{
			int nextThreshold = m_aWarLevelThresholds[m_iWarLevel];
			if (totalScore >= nextThreshold)
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
			m_OnWarLevelChanged.Invoke(m_iWarLevel);
			OnCampaignEvent(ERBLCampaignEvent.WAR_LEVEL_CHANGED, null);
		}
	}

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
		if (playerZones)
		{
			for (int i = 0; i < playerZones.Count(); i++)
			{
				RBL_CampaignZone zone = playerZones[i];
				if (zone)
				{
					totalMoney += zone.CalculateResourceIncome();
					totalHR += zone.CalculateHRIncome();
				}
			}
		}

		// Also check virtual zones
		array<ref RBL_VirtualZone> virtualZones = zoneMgr.GetVirtualZonesByFaction(ERBLFactionKey.FIA);
		if (virtualZones)
		{
			for (int i = 0; i < virtualZones.Count(); i++)
			{
				RBL_VirtualZone zone = virtualZones[i];
				if (zone)
				{
					totalMoney += zone.CalculateResourceIncome();
					totalHR += zone.CalculateHRIncome();
				}
			}
		}

		if (totalMoney > 0 || totalHR > 0)
		{
			econMgr.AddMoney(totalMoney);
			econMgr.AddHR(totalHR);
			PrintFormat("[RBL] Resource tick: +%1 Money, +%2 HR", totalMoney, totalHR);
		}
	}

	void SetDayNumber(int day)
	{
		m_iDayNumber = day;
	}

	void OnZoneCaptured(RBL_CampaignZone zone, ERBLFactionKey previousOwner, ERBLFactionKey newOwner)
	{
		if (newOwner == ERBLFactionKey.FIA)
		{
			m_iZonesCaptured++;
			AddAggression(RBL_Config.AGGRESSION_PER_ZONE_CAPTURE);

			PrintFormat("[RBL] Zone %1 captured by FIA. Aggression: %2", zone.GetZoneID(), m_iAggression);
			OnCampaignEvent(ERBLCampaignEvent.ZONE_CAPTURED, zone);

			RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
			if (zoneMgr && zoneMgr.AreAllZonesCapturedByFaction(ERBLFactionKey.FIA))
				EndCampaign(true);
		}
		else if (previousOwner == ERBLFactionKey.FIA)
		{
			PrintFormat("[RBL] Zone %1 lost", zone.GetZoneID());
			OnCampaignEvent(ERBLCampaignEvent.ZONE_LOST, zone);

			if (zone.GetZoneType() == ERBLZoneType.HQ)
				EndCampaign(false);
		}
	}

	void OnEnemyKilled()
	{
		m_iEnemyKillCount++;
		AddAggression(RBL_Config.AGGRESSION_PER_KILL);
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

		return new array<string>();
	}
}
