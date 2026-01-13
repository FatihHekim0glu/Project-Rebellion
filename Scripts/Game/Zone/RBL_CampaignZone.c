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
	int GetCivilianSupport() { return m_iCivilianSupport; }
	float GetCaptureRadius() { return m_fCaptureRadius; }
	int GetMaxGarrison() { return m_iMaxGarrison; }
	int GetCurrentGarrison() { return m_iCurrentGarrison; }
	bool IsUnderAttack() { return m_bIsUnderAttack; }
	bool IsBeingCaptured() { return m_bIsBeingCaptured; }
	float GetCaptureProgress() { return m_fCaptureProgress; }
	ERBLAlertState GetAlertState() { return m_eAlertState; }
	
	vector GetZonePosition() { return GetOrigin(); }
	
	array<string> GetGarrisonComposition() { return m_aGarrisonUnits; }
	
	ScriptInvoker GetOnZoneCaptured() { return m_OnZoneCaptured; }
	ScriptInvoker GetOnZoneAttacked() { return m_OnZoneAttacked; }
	ScriptInvoker GetOnGarrisonChanged() { return m_OnGarrisonChanged; }
	ScriptInvoker GetOnSupportChanged() { return m_OnSupportChanged; }
	
	void SetOwnerFaction(ERBLFactionKey newOwner)
	{
		if (m_eOwnerFaction == newOwner)
			return;
		
		ERBLFactionKey previousOwner = m_eOwnerFaction;
		m_eOwnerFaction = newOwner;
		
		m_OnZoneCaptured.Invoke(this, previousOwner, newOwner);
	}
	
	void SetCivilianSupport(int support)
	{
		int previousSupport = m_iCivilianSupport;
		m_iCivilianSupport = Math.Clamp(support, 0, 100);
		
		if (previousSupport != m_iCivilianSupport)
			m_OnSupportChanged.Invoke(this, m_iCivilianSupport);
	}
	
	void ModifyCivilianSupport(int delta)
	{
		SetCivilianSupport(m_iCivilianSupport + delta);
	}
	
	void SetAlertState(ERBLAlertState state)
	{
		m_eAlertState = state;
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
		data.ZoneID = m_sZoneID;
		data.Owner = m_eOwnerFaction;
		data.Support = m_iCivilianSupport;
		data.AlertState = m_eAlertState;
		
		for (int i = 0; i < m_aGarrisonUnits.Count(); i++)
		{
			data.GarrisonUnits.Insert(m_aGarrisonUnits[i]);
		}
	}
	
	void DeserializeFromStruct(RBL_ZoneSaveData data)
	{
		m_eOwnerFaction = data.Owner;
		m_iCivilianSupport = data.Support;
		m_eAlertState = data.AlertState;
		
		m_aGarrisonUnits.Clear();
		for (int i = 0; i < data.GarrisonUnits.Count(); i++)
		{
			m_aGarrisonUnits.Insert(data.GarrisonUnits[i]);
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
