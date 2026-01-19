// ============================================================================
// PROJECT REBELLION - Auto Initializer
// Automatically sets up the entire campaign when game starts
// ============================================================================

class RBL_AutoInitializer
{
	protected static ref RBL_AutoInitializer s_Instance;
	protected bool m_bInitialized;

	static RBL_AutoInitializer GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_AutoInitializer();
		return s_Instance;
	}

	void RBL_AutoInitializer()
	{
		m_bInitialized = false;
	}

	void Initialize()
	{
		if (m_bInitialized)
			return;

		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] PROJECT REBELLION - Auto Initializing");
		PrintFormat("[RBL] ========================================");

		// Initialize all managers
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		RBL_PersistenceIntegration persistence = RBL_PersistenceIntegration.GetInstance();
		RBL_ZoneConfigurator zoneConfig = RBL_ZoneConfigurator.GetInstance();

		// Create virtual zones from config
		CreateVirtualZones(zoneConfig, zoneMgr);

		// Set starting resources from config
		if (econMgr)
		{
			econMgr.SetMoney(RBL_Config.STARTING_MONEY);
			econMgr.SetHR(RBL_Config.STARTING_HR);
		}

		// Give some starting items
		if (econMgr)
		{
			econMgr.DepositItem("AK74", 5);
			econMgr.DepositItem("AKM", 3);
			econMgr.DepositItem("Makarov", 10);
			econMgr.DepositItem("RGD5_Grenade", 10);
			econMgr.DepositItem("Bandage", 20);
		}

		m_bInitialized = true;

		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] Initialization Complete!");
		PrintFormat("[RBL] Zones: %1", zoneMgr.GetAllZones().Count());
		PrintFormat("[RBL] Starting Money: $%1", econMgr.GetMoney());
		PrintFormat("[RBL] Starting HR: %1", econMgr.GetHR());
		PrintFormat("[RBL] ========================================");
	}

	protected void CreateVirtualZones(RBL_ZoneConfigurator config, RBL_ZoneManager zoneMgr)
	{
		if (!config || !zoneMgr)
			return;

		array<ref RBL_ZoneDefinition> definitions = config.GetAllDefinitions();
		if (!definitions)
			return;

		for (int i = 0; i < definitions.Count(); i++)
		{
			RBL_ZoneDefinition def = definitions[i];
			if (!def)
				continue;

			// Create a virtual zone object
			RBL_VirtualZone vZone = new RBL_VirtualZone();
			if (vZone)
			{
				vZone.InitFromDefinition(def);
				zoneMgr.RegisterVirtualZone(vZone);

				PrintFormat("[RBL] Created zone: %1 (%2) - Owner: %3",
					def.ZoneID,
					typename.EnumToString(ERBLZoneType, def.Type),
					typename.EnumToString(ERBLFactionKey, def.StartingOwner)
				);
			}
		}
	}

	bool IsInitialized() { return m_bInitialized; }
}

// Virtual zone that doesn't require entity placement
class RBL_VirtualZone
{
	protected string m_sZoneID;
	protected string m_sDisplayName;
	protected vector m_vPosition;
	protected ERBLZoneType m_eZoneType;
	protected ERBLFactionKey m_eOwnerFaction;
	protected int m_iMaxGarrison;
	protected int m_iCurrentGarrison;
	protected int m_iCivilianSupport;
	protected float m_fCaptureRadius;
	protected int m_iResourceValue;
	protected int m_iVehicleSpawns;
	protected int m_iLootTier;
	protected ERBLAlertState m_eAlertState;
	protected bool m_bIsUnderAttack;

	void RBL_VirtualZone()
	{
		m_eAlertState = ERBLAlertState.RELAXED;
		m_bIsUnderAttack = false;
		m_iCurrentGarrison = 0;
		m_iResourceValue = 100;
		m_iVehicleSpawns = 0;
		m_iLootTier = 1;
	}

	void InitFromDefinition(RBL_ZoneDefinition def)
	{
		m_sZoneID = def.ZoneID;
		m_sDisplayName = def.DisplayName;
		m_vPosition = def.Position;
		m_eZoneType = def.Type;
		m_eOwnerFaction = def.StartingOwner;
		m_iMaxGarrison = def.MaxGarrison;
		m_iCivilianSupport = def.CivilianSupport;
		m_fCaptureRadius = def.CaptureRadius;
		m_iResourceValue = def.ResourceValue;
		m_iVehicleSpawns = def.VehicleSpawns;
		m_iLootTier = def.LootTier;

		// Start with partial garrison
		m_iCurrentGarrison = m_iMaxGarrison / 2;
	}

	// Getters matching RBL_CampaignZone interface
	string GetZoneID() { return m_sZoneID; }
	string GetZoneName() { return m_sDisplayName; }
	vector GetZonePosition() { return m_vPosition; }
	vector GetOrigin() { return m_vPosition; }
	ERBLZoneType GetZoneType() { return m_eZoneType; }
	ERBLFactionKey GetOwnerFaction() { return m_eOwnerFaction; }
	int GetMaxGarrison() { return m_iMaxGarrison; }
	int GetCurrentGarrison() { return m_iCurrentGarrison; }
	int GetCivilianSupport() { return m_iCivilianSupport; }
	float GetCaptureRadius() { return m_fCaptureRadius; }
	ERBLAlertState GetAlertState() { return m_eAlertState; }
	bool IsUnderAttack() { return m_bIsUnderAttack; }
	int GetResourceValue() { return m_iResourceValue; }
	int GetVehicleSpawns() { return m_iVehicleSpawns; }
	int GetLootTier() { return m_iLootTier; }
	string GetDisplayName() { return m_sDisplayName; }
	
	bool HasVehicleType(int vehicleType) { return (m_iVehicleSpawns & vehicleType) != 0; }
	bool HasAPCs() { return HasVehicleType(RBL_VehicleSpawnType.APCS); }
	bool HasHelicopters() { return HasVehicleType(RBL_VehicleSpawnType.HELICOPTERS); }
	bool HasBoats() { return HasVehicleType(RBL_VehicleSpawnType.BOATS); }
	bool HasTrucks() { return HasVehicleType(RBL_VehicleSpawnType.TRUCKS); }

	void SetOwnerFaction(ERBLFactionKey faction) { m_eOwnerFaction = faction; }
	void SetUnderAttack(bool attacked) { m_bIsUnderAttack = attacked; }
	void SetAlertState(ERBLAlertState state) { m_eAlertState = state; }
	void SetCivilianSupport(int support) { m_iCivilianSupport = Math.Clamp(support, 0, 100); }

	float GetDistanceTo(RBL_VirtualZone other)
	{
		if (!other)
			return 999999.0;
		return vector.Distance(m_vPosition, other.GetZonePosition());
	}

	int GetStrategicValue()
	{
		int value = 0;
		switch (m_eZoneType)
		{
			case ERBLZoneType.HQ: value = RBL_Config.STRATEGIC_VALUE_HQ; break;
			case ERBLZoneType.Airbase: value = RBL_Config.STRATEGIC_VALUE_AIRBASE; break;
			case ERBLZoneType.Factory: value = RBL_Config.STRATEGIC_VALUE_FACTORY; break;
			case ERBLZoneType.Resource: value = RBL_Config.STRATEGIC_VALUE_RESOURCE; break;
			case ERBLZoneType.Town: value = RBL_Config.STRATEGIC_VALUE_TOWN + m_iCivilianSupport; break;
			case ERBLZoneType.Outpost: value = RBL_Config.STRATEGIC_VALUE_OUTPOST; break;
			case ERBLZoneType.Seaport: value = RBL_Config.STRATEGIC_VALUE_SEAPORT; break;
			default: value = RBL_Config.STRATEGIC_VALUE_ROADBLOCK; break;
		}
		return value;
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

	void SpawnGarrison()
	{
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
		{
			garMgr.SpawnGarrisonForZone(
				m_sZoneID,
				m_vPosition,
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
	}
	
	int GetGarrisonStrength()
	{
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
			return garMgr.GetGarrisonStrength(m_sZoneID);
		return 0;
	}
}
