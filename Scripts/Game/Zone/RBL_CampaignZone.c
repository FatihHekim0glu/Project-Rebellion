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
	
	// Runtime state (replicated)
	protected int m_iCurrentGarrison;
	protected bool m_bIsUnderAttack;
	protected bool m_bIsBeingCaptured;
	protected float m_fCaptureProgress;
	protected ERBLAlertState m_eAlertState;
	protected ERBLFactionKey m_eCapturingFaction;
	
	// Linked zones for supply routes
	protected ref array<RBL_CampaignZone> m_aLinkedZones;
	
	// Garrison composition (unit prefab names)
	protected ref array<string> m_aGarrisonUnits;
	
	// Spawned entities tracked for cleanup
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
		
		// Register with ZoneManager
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
	
	// Signal accessors
	ScriptInvoker GetOnZoneCaptured() { return m_OnZoneCaptured; }
	ScriptInvoker GetOnZoneAttacked() { return m_OnZoneAttacked; }
	ScriptInvoker GetOnGarrisonChanged() { return m_OnGarrisonChanged; }
	ScriptInvoker GetOnSupportChanged() { return m_OnSupportChanged; }
	
	// ========================================================================
	// SETTERS (Server-side authority)
	// ========================================================================
	
	void SetOwnerFaction(ERBLFactionKey newOwner)
	{
		if (m_eOwnerFaction == newOwner)
			return;
		
		ERBLFactionKey previousOwner = m_eOwnerFaction;
		m_eOwnerFaction = newOwner;
		
		// Broadcast ownership change
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
	
	void SetUnderAttack(bool isAttacked, ERBLFactionKey attacker = ERBLFactionKey.NONE)
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
		
		array<IEntity> nearbyEntities = {};
		GetGame().GetWorld().QueryEntitiesBySphere(
			GetOrigin(),
			m_fCaptureRadius,
			null,
			FilterCaptureEntities,
			EQueryEntitiesFlags.DYNAMIC
		);
		
		int friendlyCount = CountUnitsOfFaction(m_eOwnerFaction);
		int hostileCount = 0;
		ERBLFactionKey dominantHostile = ERBLFactionKey.NONE;
		int maxHostileCount = 0;
		
		// Count hostile units by faction
		foreach (ERBLFactionKey faction : ERBLFactionKey)
		{
			if (faction == m_eOwnerFaction || faction == ERBLFactionKey.NONE || faction == ERBLFactionKey.CIVILIAN)
				continue;
			
			int count = CountUnitsOfFaction(faction);
			hostileCount += count;
			
			if (count > maxHostileCount)
			{
				maxHostileCount = count;
				dominantHostile = faction;
			}
		}
		
		// Determine capture state
		if (hostileCount > 0 && friendlyCount == 0)
		{
			// Zone is being captured
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
			// Reclaiming - progress decays
			m_fCaptureProgress = Math.Max(0, m_fCaptureProgress - timeSlice * 2.0);
			
			if (m_fCaptureProgress == 0)
			{
				m_bIsBeingCaptured = false;
				m_eCapturingFaction = ERBLFactionKey.NONE;
				SetUnderAttack(false);
			}
		}
		else if (friendlyCount > 0 && hostileCount > 0)
		{
			// Contested - no progress change
			SetUnderAttack(true, dominantHostile);
		}
		else
		{
			// Empty zone
			SetUnderAttack(false);
		}
		
		Replication.BumpMe();
	}
	
	protected float CalculateCaptureRate(int attackerCount)
	{
		// Base rate: 1% per second, +0.5% per additional attacker, capped
		float rate = 1.0 + (attackerCount - 1) * 0.5;
		return Math.Clamp(rate, 0.5, 5.0);
	}
	
	protected void CompleteCaptureEvent(ERBLFactionKey newOwner)
	{
		ERBLFactionKey previousOwner = m_eOwnerFaction;
		
		m_fCaptureProgress = 0.0;
		m_bIsBeingCaptured = false;
		m_eCapturingFaction = ERBLFactionKey.NONE;
		
		// Clear garrison
		ClearGarrison();
		
		// Transfer ownership
		SetOwnerFaction(newOwner);
		
		// Affect support based on who captured
		if (newOwner == ERBLFactionKey.FIA)
			ModifyCivilianSupport(10);
		else
			ModifyCivilianSupport(-15);
		
		// Notify campaign manager
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
			campaignMgr.OnZoneCaptured(this, previousOwner, newOwner);
	}
	
	protected int CountUnitsOfFaction(ERBLFactionKey faction)
	{
		// Query all characters in radius and count by faction
		int count = 0;
		
		array<IEntity> entities = {};
		GetGame().GetWorld().QueryEntitiesBySphere(
			GetOrigin(),
			m_fCaptureRadius,
			null,
			null,
			EQueryEntitiesFlags.DYNAMIC
		);
		
		// Actual faction check implementation would use FactionAffiliationComponent
		// This is a placeholder for the logic
		return count;
	}
	
	protected bool FilterCaptureEntities(IEntity entity)
	{
		// Filter for controllable characters only
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		return character != null && character.IsAlive();
	}
	
	// ========================================================================
	// GARRISON MANAGEMENT
	// ========================================================================
	
	void SpawnGarrison()
	{
		if (!Replication.IsServer())
			return;
		
		ClearGarrison();
		
		// Get garrison template based on zone type and war level
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (!campaignMgr)
			return;
		
		array<string> template = campaignMgr.GetGarrisonTemplate(m_eZoneType, m_eOwnerFaction);
		
		foreach (string unitPrefab : template)
		{
			SpawnGarrisonUnit(unitPrefab);
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
		// Find valid spawn position within zone
		vector basePos = GetOrigin();
		float randomAngle = Math.RandomFloat(0, Math.PI2);
		float randomDist = Math.RandomFloat(5, m_fCaptureRadius * 0.5);
		
		vector offset = Vector(Math.Cos(randomAngle) * randomDist, 0, Math.Sin(randomAngle) * randomDist);
		vector spawnPos = basePos + offset;
		
		// Snap to terrain
		spawnPos[1] = GetGame().GetWorld().GetSurfaceY(spawnPos[0], spawnPos[2]);
		
		return spawnPos;
	}
	
	void ClearGarrison()
	{
		foreach (IEntity entity : m_aSpawnedEntities)
		{
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
		
		// Support multiplier (0.5x to 1.5x based on civilian support)
		float supportMultiplier = 0.5 + (m_iCivilianSupport / 100.0);
		
		return Math.Round(baseIncome * supportMultiplier);
	}
	
	int CalculateHRIncome()
	{
		if (m_eZoneType != ERBLZoneType.Town)
			return 0;
		
		// HR (Human Resources) only from towns, scaled by support
		return Math.Round(m_iCivilianSupport * 0.1);
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
		data.GarrisonUnits = {};
		
		foreach (string unit : m_aGarrisonUnits)
		{
			data.GarrisonUnits.Insert(unit);
		}
	}
	
	void DeserializeFromStruct(RBL_ZoneSaveData data)
	{
		m_eOwnerFaction = data.Owner;
		m_iCivilianSupport = data.Support;
		m_eAlertState = data.AlertState;
		
		m_aGarrisonUnits.Clear();
		foreach (string unit : data.GarrisonUnits)
		{
			m_aGarrisonUnits.Insert(unit);
		}
		
		// Respawn garrison based on loaded data
		if (m_aGarrisonUnits.Count() > 0)
			SpawnGarrison();
		
		Replication.BumpMe();
	}
	
	// ========================================================================
	// STRATEGIC VALUE (For AI decision making)
	// ========================================================================
	
	int GetStrategicValue()
	{
		int value = 0;
		
		switch (m_eZoneType)
		{
			case ERBLZoneType.HQ:
				value = 1000; // Highest priority
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
				value = 150 + m_iCivilianSupport; // Towns with high support are valuable
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
			return float.MAX;
		
		return vector.Distance(GetOrigin(), otherZone.GetOrigin());
	}
}

