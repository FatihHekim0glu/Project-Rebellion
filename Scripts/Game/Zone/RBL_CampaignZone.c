// ============================================================================
// PROJECT REBELLION - Campaign Zone Entity
// Dynamic zone representing a capturable location on the map
// ============================================================================

class RBL_CampaignZoneClass : GenericEntityClass
{
}

class RBL_CampaignZone : GenericEntity
{
	// ========================================================================
	// REPLICATED PROPERTIES
	// ========================================================================
	
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
	
	protected ref array<RBL_CampaignZone> m_aLinkedZones;
	protected ref array<string> m_aGarrisonUnits;
	protected ref array<IEntity> m_aSpawnedEntities;
	
	// ========================================================================
	// SIGNALS
	// ========================================================================
	
	protected ref ScriptInvoker m_OnZoneCaptured;
	protected ref ScriptInvoker m_OnZoneAttacked;
	protected ref ScriptInvoker m_OnGarrisonChanged;
	protected ref ScriptInvoker m_OnSupportChanged;
	
	// ========================================================================
	// INITIALIZATION
	// ========================================================================
	
	void RBL_CampaignZone(IEntitySource src, IEntity parent)
	{
		m_aLinkedZones = new array<RBL_CampaignZone>();
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
		
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.RegisterZone(this);
	}
	
	// ========================================================================
	// GETTERS
	// ========================================================================
	
	string GetZoneID() { return m_sZoneID; }
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
	
	vector GetZonePosition()
	{
		return GetOrigin();
	}
	
	array<string> GetGarrisonComposition()
	{
		return m_aGarrisonUnits;
	}
	
	ScriptInvoker GetOnZoneCaptured() { return m_OnZoneCaptured; }
	ScriptInvoker GetOnZoneAttacked() { return m_OnZoneAttacked; }
	ScriptInvoker GetOnGarrisonChanged() { return m_OnGarrisonChanged; }
	ScriptInvoker GetOnSupportChanged() { return m_OnSupportChanged; }
	
	// ========================================================================
	// SETTERS
	// ========================================================================
	
	void SetOwnerFaction(ERBLFactionKey newOwner)
	{
		if (m_eOwnerFaction == newOwner)
			return;
		
		ERBLFactionKey previousOwner = m_eOwnerFaction;
		m_eOwnerFaction = newOwner;
		
		Replication.BumpMe();
		
		m_OnZoneCaptured.Invoke(this, previousOwner, newOwner);
	}
	
	void SetCivilianSupport(int support)
	{
		int previousSupport = m_iCivilianSupport;
		m_iCivilianSupport = Math.Clamp(support, 0, 100);
		
		if (previousSupport != m_iCivilianSupport)
		{
			Replication.BumpMe();
			m_OnSupportChanged.Invoke(this, m_iCivilianSupport);
		}
	}
	
	void ModifyCivilianSupport(int delta)
	{
		SetCivilianSupport(m_iCivilianSupport + delta);
	}
	
	void SetAlertState(ERBLAlertState state)
	{
		if (m_eAlertState != state)
		{
			m_eAlertState = state;
			Replication.BumpMe();
		}
	}
	
	void SetUnderAttack(bool isAttacked, ERBLFactionKey attacker)
	{
		if (m_bIsUnderAttack != isAttacked)
		{
			m_bIsUnderAttack = isAttacked;
			Replication.BumpMe();
			
			if (isAttacked)
				m_OnZoneAttacked.Invoke(this, attacker);
		}
	}
	
	// ========================================================================
	// CAPTURE LOGIC
	// ========================================================================
	
	void UpdateCaptureState(float timeSlice)
	{
		if (!Replication.IsServer())
			return;
		
		int friendlyCount = CountUnitsOfFaction(m_eOwnerFaction);
		int hostileCount = 0;
		ERBLFactionKey dominantHostile = ERBLFactionKey.NONE;
		int maxHostileCount = 0;
		
		// Check each faction
		int usCount = CountUnitsOfFaction(ERBLFactionKey.US);
		int ussrCount = CountUnitsOfFaction(ERBLFactionKey.USSR);
		int fiaCount = CountUnitsOfFaction(ERBLFactionKey.FIA);
		
		if (m_eOwnerFaction != ERBLFactionKey.US)
		{
			hostileCount += usCount;
			if (usCount > maxHostileCount)
			{
				maxHostileCount = usCount;
				dominantHostile = ERBLFactionKey.US;
			}
		}
		
		if (m_eOwnerFaction != ERBLFactionKey.USSR)
		{
			hostileCount += ussrCount;
			if (ussrCount > maxHostileCount)
			{
				maxHostileCount = ussrCount;
				dominantHostile = ERBLFactionKey.USSR;
			}
		}
		
		if (m_eOwnerFaction != ERBLFactionKey.FIA)
		{
			hostileCount += fiaCount;
			if (fiaCount > maxHostileCount)
			{
				maxHostileCount = fiaCount;
				dominantHostile = ERBLFactionKey.FIA;
			}
		}
		
		if (hostileCount > 0 && friendlyCount == 0)
		{
			m_bIsBeingCaptured = true;
			m_eCapturingFaction = dominantHostile;
			m_fCaptureProgress += timeSlice * CalculateCaptureRate(maxHostileCount);
			
			SetUnderAttack(true, dominantHostile);
			
			if (m_fCaptureProgress >= 100.0)
			{
				CompleteCaptureEvent(dominantHostile);
			}
		}
		else if (friendlyCount > 0 && hostileCount == 0 && m_fCaptureProgress > 0)
		{
			m_fCaptureProgress = Math.Max(0, m_fCaptureProgress - timeSlice * 2.0);
			
			if (m_fCaptureProgress == 0)
			{
				m_bIsBeingCaptured = false;
				m_eCapturingFaction = ERBLFactionKey.NONE;
				SetUnderAttack(false, ERBLFactionKey.NONE);
			}
		}
		else if (friendlyCount > 0 && hostileCount > 0)
		{
			SetUnderAttack(true, dominantHostile);
		}
		else
		{
			SetUnderAttack(false, ERBLFactionKey.NONE);
		}
		
		Replication.BumpMe();
	}
	
	protected float CalculateCaptureRate(int attackerCount)
	{
		float rate = 1.0 + (attackerCount - 1) * 0.5;
		return Math.Clamp(rate, 0.5, 5.0);
	}
	
	protected void CompleteCaptureEvent(ERBLFactionKey newOwner)
	{
		ERBLFactionKey previousOwner = m_eOwnerFaction;
		
		m_fCaptureProgress = 0.0;
		m_bIsBeingCaptured = false;
		m_eCapturingFaction = ERBLFactionKey.NONE;
		
		ClearGarrison();
		SetOwnerFaction(newOwner);
		
		if (newOwner == ERBLFactionKey.FIA)
			ModifyCivilianSupport(10);
		else
			ModifyCivilianSupport(-15);
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
			campaignMgr.OnZoneCaptured(this, previousOwner, newOwner);
	}
	
	protected int CountUnitsOfFaction(ERBLFactionKey faction)
	{
		int count = 0;
		// Placeholder - actual implementation would query entities
		return count;
	}
	
	protected bool FilterCaptureEntities(IEntity entity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (character && character.IsAlive())
			return true;
		return false;
	}
	
	// ========================================================================
	// GARRISON MANAGEMENT
	// ========================================================================
	
	void SpawnGarrison()
	{
		if (!Replication.IsServer())
			return;
		
		ClearGarrison();
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (!campaignMgr)
			return;
		
		array<string> template = campaignMgr.GetGarrisonTemplate(m_eZoneType, m_eOwnerFaction);
		
		for (int i = 0; i < template.Count(); i++)
		{
			SpawnGarrisonUnit(template[i]);
		}
		
		m_OnGarrisonChanged.Invoke(this, m_iCurrentGarrison);
	}
	
	protected void SpawnGarrisonUnit(string prefabName)
	{
		Resource prefab = Resource.Load(prefabName);
		if (!prefab || !prefab.IsValid())
			return;
		
		vector spawnPos = FindSpawnPosition();
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = spawnPos;
		
		IEntity unit = GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld(), params);
		if (unit)
		{
			m_aSpawnedEntities.Insert(unit);
			m_aGarrisonUnits.Insert(prefabName);
			m_iCurrentGarrison++;
		}
	}
	
	protected vector FindSpawnPosition()
	{
		vector basePos = GetOrigin();
		float randomAngle = Math.RandomFloat(0, Math.PI2);
		float randomDist = Math.RandomFloat(5, m_fCaptureRadius * 0.5);
		
		vector offset = Vector(Math.Cos(randomAngle) * randomDist, 0, Math.Sin(randomAngle) * randomDist);
		vector spawnPos = basePos + offset;
		
		if (GetGame() && GetGame().GetWorld())
			spawnPos[1] = GetGame().GetWorld().GetSurfaceY(spawnPos[0], spawnPos[2]);
		
		return spawnPos;
	}
	
	void ClearGarrison()
	{
		for (int i = 0; i < m_aSpawnedEntities.Count(); i++)
		{
			IEntity entity = m_aSpawnedEntities[i];
			if (entity)
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		
		m_aSpawnedEntities.Clear();
		m_aGarrisonUnits.Clear();
		m_iCurrentGarrison = 0;
	}
	
	void OnGarrisonUnitKilled(IEntity unit)
	{
		int idx = m_aSpawnedEntities.Find(unit);
		if (idx != -1)
		{
			m_aSpawnedEntities.Remove(idx);
			if (m_aGarrisonUnits.Count() > idx)
				m_aGarrisonUnits.Remove(idx);
			m_iCurrentGarrison--;
			
			m_OnGarrisonChanged.Invoke(this, m_iCurrentGarrison);
		}
	}
	
	// ========================================================================
	// INCOME CALCULATION
	// ========================================================================
	
	int CalculateResourceIncome()
	{
		int baseIncome = 0;
		
		switch (m_eZoneType)
		{
			case ERBLZoneType.Resource:
				baseIncome = 100;
				break;
			case ERBLZoneType.Factory:
				baseIncome = 150;
				break;
			case ERBLZoneType.Town:
				baseIncome = 50;
				break;
			case ERBLZoneType.Airbase:
				baseIncome = 75;
				break;
			case ERBLZoneType.Seaport:
				baseIncome = 125;
				break;
			default:
				baseIncome = 25;
				break;
		}
		
		float supportMultiplier = 0.5 + (m_iCivilianSupport / 100.0);
		
		return baseIncome * supportMultiplier;
	}
	
	int CalculateHRIncome()
	{
		if (m_eZoneType != ERBLZoneType.Town)
			return 0;
		
		return m_iCivilianSupport * 0.1;
	}
	
	// ========================================================================
	// SERIALIZATION
	// ========================================================================
	
	void SerializeToStruct(out RBL_ZoneSaveData data)
	{
		data = new RBL_ZoneSaveData();
		data.ZoneID = m_sZoneID;
		data.Owner = m_eOwnerFaction;
		data.Support = m_iCivilianSupport;
		data.AlertState = m_eAlertState;
		data.GarrisonUnits = new array<string>();
		
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
		
		if (m_aGarrisonUnits.Count() > 0)
			SpawnGarrison();
		
		Replication.BumpMe();
	}
	
	// ========================================================================
	// STRATEGIC VALUE
	// ========================================================================
	
	int GetStrategicValue()
	{
		int value = 0;
		
		switch (m_eZoneType)
		{
			case ERBLZoneType.HQ:
				value = 1000;
				break;
			case ERBLZoneType.Airbase:
				value = 500;
				break;
			case ERBLZoneType.Factory:
				value = 300;
				break;
			case ERBLZoneType.Resource:
				value = 200;
				break;
			case ERBLZoneType.Town:
				value = 150 + m_iCivilianSupport;
				break;
			case ERBLZoneType.Outpost:
				value = 100;
				break;
			case ERBLZoneType.Seaport:
				value = 250;
				break;
			default:
				value = 50;
				break;
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
