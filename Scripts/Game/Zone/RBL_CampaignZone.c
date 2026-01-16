// ============================================================================
// PROJECT REBELLION - Campaign Zone Entity
// ============================================================================

class RBL_CampaignZoneClass : GenericEntityClass
{
}

class RBL_CampaignZone : GenericEntity
{
	[Attribute("", UIWidgets.EditBox, "Unique zone identifier")]
	protected string m_sZoneID;
	
	[Attribute("0", UIWidgets.ComboBox, "Zone classification", "", ParamEnumArray.FromEnum(ERBLZoneType))]
	protected ERBLZoneType m_eZoneType;
	
	[Attribute("0", UIWidgets.ComboBox, "Current owner faction", "", ParamEnumArray.FromEnum(ERBLFactionKey))]
	protected ERBLFactionKey m_eOwnerFaction;

	protected ERBLFactionKey m_ePreviousOwner;
	
	[Attribute("50", UIWidgets.Slider, "Civilian support percentage", "0 100 1")]
	protected int m_iCivilianSupport;
	
	[Attribute("100", UIWidgets.EditBox, "Capture radius in meters")]
	protected float m_fCaptureRadius;
	
	[Attribute("10", UIWidgets.EditBox, "Base garrison capacity")]
	protected int m_iMaxGarrison;
	
	protected int m_iCurrentGarrison;
	protected bool m_bIsUnderAttack;
	protected bool m_bIsBeingCaptured;
	protected float m_fCaptureProgress;
	protected ERBLAlertState m_eAlertState;
	protected ERBLFactionKey m_eCapturingFaction;
	protected int m_iSupportTrend;
	protected int m_iResourceOutput;
	protected float m_fResourceTimer;
	protected bool m_bIsContested;
	protected bool m_bIsDestroyed;
	protected float m_fLastCaptureTime;
	protected float m_fLastAttackTime;
	
	protected ref array<string> m_aGarrisonUnits;
	protected ref array<IEntity> m_aSpawnedEntities;
	
	protected ref ScriptInvoker m_OnZoneCaptured;
	protected ref ScriptInvoker m_OnZoneAttacked;
	protected ref ScriptInvoker m_OnGarrisonChanged;
	protected ref ScriptInvoker m_OnSupportChanged;
	
	void RBL_CampaignZone(IEntitySource src, IEntity parent)
	{
		m_aGarrisonUnits = new array<string>();
		m_aSpawnedEntities = new array<IEntity>();
		
		m_OnZoneCaptured = new ScriptInvoker();
		m_OnZoneAttacked = new ScriptInvoker();
		m_OnGarrisonChanged = new ScriptInvoker();
		m_OnSupportChanged = new ScriptInvoker();
		
		m_iCurrentGarrison = 0;
		m_bIsUnderAttack = false;
		m_bIsBeingCaptured = false;
		m_fCaptureProgress = 0.0;
		m_eAlertState = ERBLAlertState.RELAXED;
		m_eCapturingFaction = ERBLFactionKey.NONE;
		m_ePreviousOwner = m_eOwnerFaction;
		m_iSupportTrend = 0;
		m_iResourceOutput = 0;
		m_fResourceTimer = 0;
		m_bIsContested = false;
		m_bIsDestroyed = false;
		m_fLastCaptureTime = 0;
		m_fLastAttackTime = 0;
		
		SetEventMask(EntityEvent.INIT);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.RegisterZone(this);
	}
	
	string GetZoneID() { return m_sZoneID; }
	string GetZoneName() { return m_sZoneID; }
	ERBLZoneType GetZoneType() { return m_eZoneType; }
	ERBLFactionKey GetOwnerFaction() { return m_eOwnerFaction; }
	ERBLFactionKey GetPreviousOwner() { return m_ePreviousOwner; }
	int GetCivilianSupport() { return m_iCivilianSupport; }
	int GetSupportLevel() { return m_iCivilianSupport; }
	int GetSupportTrend() { return m_iSupportTrend; }
	float GetCaptureRadius() { return m_fCaptureRadius; }
	int GetMaxGarrison() { return m_iMaxGarrison; }
	int GetCurrentGarrison() { return m_iCurrentGarrison; }
	bool IsUnderAttack() { return m_bIsUnderAttack; }
	bool IsBeingCaptured() { return m_bIsBeingCaptured; }
	bool IsContested() { return m_bIsBeingCaptured || m_bIsContested; }
	bool IsDestroyed() { return m_bIsDestroyed; }
	float GetCaptureProgress() { return m_fCaptureProgress; }
	ERBLAlertState GetAlertState() { return m_eAlertState; }
	ERBLFactionKey GetCapturingFaction() { return m_eCapturingFaction; }
	int GetResourceOutput() { return m_iResourceOutput; }
	float GetResourceTimer() { return m_fResourceTimer; }
	float GetLastCaptureTime() { return m_fLastCaptureTime; }
	float GetLastAttackTime() { return m_fLastAttackTime; }
	
	vector GetZonePosition() { return GetOrigin(); }
	
	array<string> GetGarrisonComposition() { return m_aGarrisonUnits; }
	array<string> GetGarrisonUnitTypes() { return m_aGarrisonUnits; }
	
	ScriptInvoker GetOnZoneCaptured() { return m_OnZoneCaptured; }
	ScriptInvoker GetOnZoneAttacked() { return m_OnZoneAttacked; }
	ScriptInvoker GetOnGarrisonChanged() { return m_OnGarrisonChanged; }
	ScriptInvoker GetOnSupportChanged() { return m_OnSupportChanged; }
	
	void SetOwnerFaction(ERBLFactionKey newOwner)
	{
		if (m_eOwnerFaction == newOwner)
			return;
		
		ERBLFactionKey previousOwner = m_eOwnerFaction;
		m_ePreviousOwner = previousOwner;
		m_eOwnerFaction = newOwner;
		
		m_OnZoneCaptured.Invoke(this, previousOwner, newOwner);
	}

	void SetPreviousOwner(ERBLFactionKey previousOwner)
	{
		m_ePreviousOwner = previousOwner;
	}
	
	void SetCivilianSupport(int support)
	{
		int previousSupport = m_iCivilianSupport;
		m_iCivilianSupport = Math.Clamp(support, 0, 100);
		
		if (previousSupport != m_iCivilianSupport)
			m_OnSupportChanged.Invoke(this, m_iCivilianSupport);
	}

	void SetSupportLevel(int support)
	{
		SetCivilianSupport(support);
	}

	void SetSupportTrend(int trend)
	{
		m_iSupportTrend = trend;
	}
	
	void ModifyCivilianSupport(int delta)
	{
		SetCivilianSupport(m_iCivilianSupport + delta);
	}
	
	void SetAlertState(ERBLAlertState state)
	{
		m_eAlertState = state;
	}

	void SetUnderAttack(bool isAttacked)
	{
		SetUnderAttack(isAttacked, ERBLFactionKey.NONE);
	}
	
	void SetUnderAttack(bool isAttacked, ERBLFactionKey attacker)
	{
		if (m_bIsUnderAttack != isAttacked)
		{
			m_bIsUnderAttack = isAttacked;
			
			if (isAttacked)
				m_OnZoneAttacked.Invoke(this, attacker);
		}
	}

	void SetCaptureProgress(float progress)
	{
		m_fCaptureProgress = Math.Clamp(progress, 0.0, 1.0);
	}

	void SetCapturingFaction(ERBLFactionKey faction)
	{
		m_eCapturingFaction = faction;
	}

	void SetResourceOutput(int output)
	{
		m_iResourceOutput = output;
	}

	void SetResourceTimer(float timer)
	{
		m_fResourceTimer = timer;
	}

	void SetContested(bool contested)
	{
		m_bIsContested = contested;
		m_bIsBeingCaptured = contested;
	}

	void SetDestroyed(bool destroyed)
	{
		m_bIsDestroyed = destroyed;
	}

	void SetLastCaptureTime(float time)
	{
		m_fLastCaptureTime = time;
	}

	void SetLastAttackTime(float time)
	{
		m_fLastAttackTime = time;
	}
	
	void UpdateCaptureState(float timeSlice)
	{
		// Simplified capture logic
	}
	
	void SpawnGarrison()
	{
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
		{
			garMgr.SpawnGarrisonForZone(
				m_sZoneID,
				GetOrigin(),
				m_fCaptureRadius,
				m_eZoneType,
				m_eOwnerFaction,
				m_iMaxGarrison
			);
		}
	}
	
	void ClearGarrison()
	{
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
			garMgr.ClearGarrison(m_sZoneID);
		
		m_aSpawnedEntities.Clear();
		m_aGarrisonUnits.Clear();
		m_iCurrentGarrison = 0;
	}

	void SetGarrisonStrength(int strength)
	{
		m_iCurrentGarrison = Math.Max(0, strength);
	}

	void SetMaxGarrison(int maxGarrison)
	{
		m_iMaxGarrison = Math.Max(0, maxGarrison);
	}

	void ClearGarrisonUnitTypes()
	{
		m_aGarrisonUnits.Clear();
	}

	void AddGarrisonUnitType(string unitType)
	{
		if (!unitType.IsEmpty())
			m_aGarrisonUnits.Insert(unitType);
	}
	
	int GetGarrisonStrength()
	{
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
			return garMgr.GetGarrisonStrength(m_sZoneID);
		return m_iCurrentGarrison;
	}
	
	int CalculateResourceIncome()
	{
		int baseIncome = 0;
		
		switch (m_eZoneType)
		{
			case ERBLZoneType.Resource: baseIncome = RBL_Config.INCOME_RESOURCE_BASE; break;
			case ERBLZoneType.Factory: baseIncome = RBL_Config.INCOME_FACTORY_BASE; break;
			case ERBLZoneType.Town: baseIncome = RBL_Config.INCOME_TOWN_BASE; break;
			case ERBLZoneType.Airbase: baseIncome = RBL_Config.INCOME_AIRBASE_BASE; break;
			case ERBLZoneType.Seaport: baseIncome = RBL_Config.INCOME_SEAPORT_BASE; break;
			default: baseIncome = RBL_Config.INCOME_OUTPOST_BASE; break;
		}
		
		float supportMultiplier = RBL_Config.INCOME_SUPPORT_MIN_MULT + 
			((RBL_Config.INCOME_SUPPORT_MAX_MULT - RBL_Config.INCOME_SUPPORT_MIN_MULT) * (m_iCivilianSupport / 100.0));
		
		return Math.Round(baseIncome * supportMultiplier);
	}
	
	int CalculateHRIncome()
	{
		if (m_eZoneType != ERBLZoneType.Town)
			return 0;
		
		return Math.Round(m_iCivilianSupport * 0.1);
	}
	
	void SerializeToStruct(out RBL_ZoneSaveData data)
	{
		data = new RBL_ZoneSaveData();
		data.m_sZoneID = m_sZoneID;
		data.m_sZoneName = GetZoneName();
		data.m_iZoneType = m_eZoneType;
		data.m_vPosition = GetOrigin();
		data.m_iOwnerFaction = m_eOwnerFaction;
		data.m_iPreviousOwner = m_ePreviousOwner;
		data.m_iGarrisonStrength = GetGarrisonStrength();
		data.m_iMaxGarrison = m_iMaxGarrison;
		data.m_iSupportLevel = m_iCivilianSupport;
		data.m_iSupportTrend = m_iSupportTrend;
		data.m_fCaptureProgress = m_fCaptureProgress;
		data.m_iCapturingFaction = m_eCapturingFaction;
		data.m_iResourceOutput = m_iResourceOutput;
		data.m_fResourceTimer = m_fResourceTimer;
		data.m_bIsUnderAttack = m_bIsUnderAttack;
		data.m_bIsContested = IsContested();
		data.m_bIsDestroyed = m_bIsDestroyed;
		data.m_fLastCaptureTime = m_fLastCaptureTime;
		data.m_fLastAttackTime = m_fLastAttackTime;
		
		for (int i = 0; i < m_aGarrisonUnits.Count(); i++)
		{
			data.m_aGarrisonUnitTypes.Insert(m_aGarrisonUnits[i]);
		}
	}
	
	void DeserializeFromStruct(RBL_ZoneSaveData data)
	{
		SetOwnerFaction(data.m_iOwnerFaction);
		SetPreviousOwner(data.m_iPreviousOwner);
		SetSupportLevel(data.m_iSupportLevel);
		SetSupportTrend(data.m_iSupportTrend);
		SetGarrisonStrength(data.m_iGarrisonStrength);
		SetMaxGarrison(data.m_iMaxGarrison);
		SetCaptureProgress(data.m_fCaptureProgress);
		SetCapturingFaction(data.m_iCapturingFaction);
		SetResourceOutput(data.m_iResourceOutput);
		SetResourceTimer(data.m_fResourceTimer);
		SetUnderAttack(data.m_bIsUnderAttack);
		SetContested(data.m_bIsContested);
		SetDestroyed(data.m_bIsDestroyed);
		SetLastCaptureTime(data.m_fLastCaptureTime);
		SetLastAttackTime(data.m_fLastAttackTime);
		
		ClearGarrisonUnitTypes();
		for (int i = 0; i < data.m_aGarrisonUnitTypes.Count(); i++)
		{
			AddGarrisonUnitType(data.m_aGarrisonUnitTypes[i]);
		}
	}
	
	int GetStrategicValue()
	{
		int value = 0;
		
		switch (m_eZoneType)
		{
			case ERBLZoneType.HQ: value = 1000; break;
			case ERBLZoneType.Airbase: value = 500; break;
			case ERBLZoneType.Factory: value = 300; break;
			case ERBLZoneType.Resource: value = 200; break;
			case ERBLZoneType.Town: value = 150 + m_iCivilianSupport; break;
			case ERBLZoneType.Outpost: value = 100; break;
			case ERBLZoneType.Seaport: value = 250; break;
			default: value = 50; break;
		}
		
		return value;
	}
	
	float GetDistanceTo(RBL_CampaignZone otherZone)
	{
		if (!otherZone)
			return 999999.0;
		
		return vector.Distance(GetOrigin(), otherZone.GetOrigin());
	}
}
