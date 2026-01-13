// ============================================================================
// PROJECT REBELLION - Garrison Manager
// Handles spawning and tracking of AI defenders at zones
// ============================================================================

// Tracks spawned units for a single zone
class RBL_GarrisonData
{
	string ZoneID;
	ref array<IEntity> SpawnedUnits;
	ref array<IEntity> SpawnedVehicles;
	int TargetStrength;
	int CurrentStrength;
	float TimeSinceLastCheck;
	bool IsSpawned;
	
	void RBL_GarrisonData()
	{
		SpawnedUnits = new array<IEntity>();
		SpawnedVehicles = new array<IEntity>();
		TargetStrength = 0;
		CurrentStrength = 0;
		TimeSinceLastCheck = 0;
		IsSpawned = false;
	}
	
	void Clear()
	{
		// Delete all spawned entities
		for (int i = SpawnedUnits.Count() - 1; i >= 0; i--)
		{
			IEntity unit = SpawnedUnits[i];
			if (unit)
				SCR_EntityHelper.DeleteEntityAndChildren(unit);
		}
		SpawnedUnits.Clear();
		
		for (int i = SpawnedVehicles.Count() - 1; i >= 0; i--)
		{
			IEntity vehicle = SpawnedVehicles[i];
			if (vehicle)
				SCR_EntityHelper.DeleteEntityAndChildren(vehicle);
		}
		SpawnedVehicles.Clear();
		
		CurrentStrength = 0;
		IsSpawned = false;
	}
	
	int CountAliveUnits()
	{
		int alive = 0;
		for (int i = SpawnedUnits.Count() - 1; i >= 0; i--)
		{
			IEntity unit = SpawnedUnits[i];
			if (!unit)
			{
				SpawnedUnits.Remove(i);
				continue;
			}
			
			// Check if unit is alive
			DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(unit.FindComponent(DamageManagerComponent));
			if (dmgMgr && dmgMgr.GetState() != EDamageState.DESTROYED)
				alive++;
			else
				SpawnedUnits.Remove(i);
		}
		CurrentStrength = alive;
		return alive;
	}
}

// Template for garrison composition at each war level
class RBL_GarrisonTemplate
{
	ERBLZoneType ZoneType;
	int WarLevel;
	ref array<string> InfantryPrefabs;
	ref array<string> VehiclePrefabs;
	int MinInfantry;
	int MaxInfantry;
	int MinVehicles;
	int MaxVehicles;
	
	void RBL_GarrisonTemplate()
	{
		InfantryPrefabs = new array<string>();
		VehiclePrefabs = new array<string>();
	}
}

// ============================================================================
// GARRISON MANAGER - Main singleton
// ============================================================================
class RBL_GarrisonManager
{
	protected static ref RBL_GarrisonManager s_Instance;
	
	protected ref map<string, ref RBL_GarrisonData> m_mGarrisons;
	protected ref array<ref RBL_GarrisonTemplate> m_aTemplates;
	
	protected const float GARRISON_CHECK_INTERVAL = 10.0;
	protected const float SPAWN_RADIUS_MULTIPLIER = 0.6;
	protected const float MIN_SPAWN_DISTANCE = 5.0;
	
	// USSR faction prefabs (Arma Reforger default)
	protected const string PREFAB_USSR_RIFLEMAN = "{6D62B5D93627D}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_Rifleman.et";
	protected const string PREFAB_USSR_MG = "{DCB41E1B7E697DE8}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_MG.et";
	protected const string PREFAB_USSR_AT = "{2A79433F26F22D2D}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_AT.et";
	protected const string PREFAB_USSR_MEDIC = "{6B5F3AE10AA0C35E}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_Medic.et";
	protected const string PREFAB_USSR_OFFICER = "{E8C5ED3082B4D2A1}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_Officer.et";
	protected const string PREFAB_USSR_SNIPER = "{DF25789F0CD75F22}Prefabs/Characters/Factions/OPFOR/USSR_Army/Character_USSR_Sniper.et";
	
	protected const string PREFAB_UAZ = "{5E74787BB083789B}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469.et";
	protected const string PREFAB_UAZ_MG = "{C38D37EE2C0E0888}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469_MG.et";
	protected const string PREFAB_URAL = "{91B01E6C0D20E1D1}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320.et";
	protected const string PREFAB_BTR70 = "{D85A504DF4E2C128}Prefabs/Vehicles/Wheeled/BTR70/BTR70.et";
	protected const string PREFAB_BMP1 = "{BB0E7CE42F0F3E19}Prefabs/Vehicles/Tracked/BMP1/BMP1.et";
	
	static RBL_GarrisonManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_GarrisonManager();
		return s_Instance;
	}
	
	void RBL_GarrisonManager()
	{
		m_mGarrisons = new map<string, ref RBL_GarrisonData>();
		m_aTemplates = new array<ref RBL_GarrisonTemplate>();
		InitializeTemplates();
		
		PrintFormat("[RBL_Garrison] Garrison Manager initialized with %1 templates", m_aTemplates.Count());
	}
	
	// ========================================================================
	// TEMPLATE INITIALIZATION
	// ========================================================================
	protected void InitializeTemplates()
	{
		// HQ - Heavy garrison
		CreateTemplate(ERBLZoneType.HQ, 1, 3, 8, 15, 0, 1);
		CreateTemplate(ERBLZoneType.HQ, 5, 3, 12, 20, 1, 2);
		CreateTemplate(ERBLZoneType.HQ, 8, 3, 15, 25, 2, 3);
		
		// Airbase - Medium-heavy garrison with vehicles
		CreateTemplate(ERBLZoneType.Airbase, 1, 3, 6, 12, 0, 1);
		CreateTemplate(ERBLZoneType.Airbase, 5, 3, 10, 18, 1, 2);
		CreateTemplate(ERBLZoneType.Airbase, 8, 3, 12, 22, 2, 3);
		
		// Town - Light garrison
		CreateTemplate(ERBLZoneType.Town, 1, 3, 3, 6, 0, 0);
		CreateTemplate(ERBLZoneType.Town, 5, 3, 5, 10, 0, 1);
		CreateTemplate(ERBLZoneType.Town, 8, 3, 8, 14, 1, 2);
		
		// Outpost - Small garrison
		CreateTemplate(ERBLZoneType.Outpost, 1, 3, 2, 4, 0, 0);
		CreateTemplate(ERBLZoneType.Outpost, 5, 3, 4, 8, 0, 1);
		CreateTemplate(ERBLZoneType.Outpost, 8, 3, 6, 10, 1, 1);
		
		// Factory - Medium garrison
		CreateTemplate(ERBLZoneType.Factory, 1, 3, 4, 8, 0, 0);
		CreateTemplate(ERBLZoneType.Factory, 5, 3, 6, 12, 0, 1);
		CreateTemplate(ERBLZoneType.Factory, 8, 3, 10, 16, 1, 2);
		
		// Resource - Light garrison
		CreateTemplate(ERBLZoneType.Resource, 1, 3, 2, 5, 0, 0);
		CreateTemplate(ERBLZoneType.Resource, 5, 3, 4, 8, 0, 1);
		CreateTemplate(ERBLZoneType.Resource, 8, 3, 6, 10, 1, 1);
		
		// Seaport - Medium garrison
		CreateTemplate(ERBLZoneType.Seaport, 1, 3, 4, 8, 0, 0);
		CreateTemplate(ERBLZoneType.Seaport, 5, 3, 8, 12, 1, 1);
		CreateTemplate(ERBLZoneType.Seaport, 8, 3, 10, 16, 1, 2);
		
		// Roadblock - Very light
		CreateTemplate(ERBLZoneType.Roadblock, 1, 3, 2, 3, 0, 0);
		CreateTemplate(ERBLZoneType.Roadblock, 5, 3, 2, 4, 0, 1);
		CreateTemplate(ERBLZoneType.Roadblock, 8, 3, 3, 5, 1, 1);
		
		// Watchpost - Minimal
		CreateTemplate(ERBLZoneType.Watchpost, 1, 3, 1, 2, 0, 0);
		CreateTemplate(ERBLZoneType.Watchpost, 5, 3, 2, 3, 0, 0);
		CreateTemplate(ERBLZoneType.Watchpost, 8, 3, 2, 4, 0, 1);
	}
	
	protected void CreateTemplate(ERBLZoneType zoneType, int warLevel, int minWarLevel, int minInf, int maxInf, int minVeh, int maxVeh)
	{
		RBL_GarrisonTemplate template = new RBL_GarrisonTemplate();
		template.ZoneType = zoneType;
		template.WarLevel = warLevel;
		template.MinInfantry = minInf;
		template.MaxInfantry = maxInf;
		template.MinVehicles = minVeh;
		template.MaxVehicles = maxVeh;
		
		// Add appropriate prefabs based on war level
		template.InfantryPrefabs.Insert(PREFAB_USSR_RIFLEMAN);
		template.InfantryPrefabs.Insert(PREFAB_USSR_RIFLEMAN);
		
		if (warLevel >= 3)
		{
			template.InfantryPrefabs.Insert(PREFAB_USSR_MG);
			template.InfantryPrefabs.Insert(PREFAB_USSR_MEDIC);
		}
		
		if (warLevel >= 5)
		{
			template.InfantryPrefabs.Insert(PREFAB_USSR_AT);
			template.InfantryPrefabs.Insert(PREFAB_USSR_OFFICER);
		}
		
		if (warLevel >= 7)
		{
			template.InfantryPrefabs.Insert(PREFAB_USSR_SNIPER);
		}
		
		// Vehicles based on war level
		if (warLevel < 3)
		{
			template.VehiclePrefabs.Insert(PREFAB_UAZ);
		}
		else if (warLevel < 5)
		{
			template.VehiclePrefabs.Insert(PREFAB_UAZ_MG);
			template.VehiclePrefabs.Insert(PREFAB_URAL);
		}
		else if (warLevel < 7)
		{
			template.VehiclePrefabs.Insert(PREFAB_UAZ_MG);
			template.VehiclePrefabs.Insert(PREFAB_BTR70);
		}
		else
		{
			template.VehiclePrefabs.Insert(PREFAB_BTR70);
			template.VehiclePrefabs.Insert(PREFAB_BMP1);
		}
		
		m_aTemplates.Insert(template);
	}
	
	// Get best matching template for zone type and war level
	protected RBL_GarrisonTemplate GetTemplate(ERBLZoneType zoneType, int warLevel)
	{
		RBL_GarrisonTemplate bestMatch = null;
		int bestWarLevel = 0;
		
		for (int i = 0; i < m_aTemplates.Count(); i++)
		{
			RBL_GarrisonTemplate t = m_aTemplates[i];
			if (t.ZoneType != zoneType)
				continue;
			
			if (t.WarLevel <= warLevel && t.WarLevel > bestWarLevel)
			{
				bestMatch = t;
				bestWarLevel = t.WarLevel;
			}
		}
		
		return bestMatch;
	}
	
	// ========================================================================
	// SPAWN GARRISON
	// ========================================================================
	bool SpawnGarrisonForZone(string zoneID, vector position, float radius, ERBLZoneType zoneType, ERBLFactionKey owner, int maxGarrison)
	{
		// Only spawn for enemy factions
		if (owner == ERBLFactionKey.FIA || owner == ERBLFactionKey.NONE)
			return false;
		
		// Check if already spawned
		RBL_GarrisonData existingData;
		if (m_mGarrisons.Find(zoneID, existingData) && existingData.IsSpawned)
		{
			PrintFormat("[RBL_Garrison] Zone %1 already has garrison", zoneID);
			return false;
		}
		
		// Get war level
		int warLevel = 1;
		RBL_CampaignManager campaign = RBL_CampaignManager.GetInstance();
		if (campaign)
			warLevel = campaign.GetWarLevel();
		
		// Get template
		RBL_GarrisonTemplate template = GetTemplate(zoneType, warLevel);
		if (!template)
		{
			PrintFormat("[RBL_Garrison] No template for zone type %1", typename.EnumToString(ERBLZoneType, zoneType));
			return false;
		}
		
		// Create garrison data
		RBL_GarrisonData data = new RBL_GarrisonData();
		data.ZoneID = zoneID;
		
		// Calculate garrison size (scaled by maxGarrison)
		float scale = maxGarrison / 10.0;
		int infantryCount = Math.RandomIntInclusive(template.MinInfantry, template.MaxInfantry);
		infantryCount = Math.Round(infantryCount * scale);
		infantryCount = Math.ClampInt(infantryCount, 1, maxGarrison);
		
		int vehicleCount = Math.RandomIntInclusive(template.MinVehicles, template.MaxVehicles);
		vehicleCount = Math.Round(vehicleCount * scale);
		
		data.TargetStrength = infantryCount;
		
		PrintFormat("[RBL_Garrison] Spawning garrison at %1: %2 infantry, %3 vehicles (War Level %4)", 
			zoneID, infantryCount, vehicleCount, warLevel);
		
		// Generate spawn positions
		array<vector> spawnPositions = GenerateSpawnPositions(position, radius * SPAWN_RADIUS_MULTIPLIER, infantryCount + vehicleCount);
		
		int posIndex = 0;
		
		// Spawn infantry
		for (int i = 0; i < infantryCount && posIndex < spawnPositions.Count(); i++)
		{
			string prefab = template.InfantryPrefabs.GetRandomElement();
			vector spawnPos = spawnPositions[posIndex++];
			
			IEntity unit = SpawnUnit(prefab, spawnPos);
			if (unit)
				data.SpawnedUnits.Insert(unit);
		}
		
		// Spawn vehicles
		for (int i = 0; i < vehicleCount && posIndex < spawnPositions.Count(); i++)
		{
			string prefab = template.VehiclePrefabs.GetRandomElement();
			vector spawnPos = spawnPositions[posIndex++];
			
			IEntity vehicle = SpawnVehicle(prefab, spawnPos);
			if (vehicle)
				data.SpawnedVehicles.Insert(vehicle);
		}
		
		data.CurrentStrength = data.SpawnedUnits.Count();
		data.IsSpawned = true;
		
		m_mGarrisons.Set(zoneID, data);
		
		PrintFormat("[RBL_Garrison] Spawned %1 units, %2 vehicles at %3", 
			data.SpawnedUnits.Count(), data.SpawnedVehicles.Count(), zoneID);
		
		return true;
	}
	
	// Generate random positions within radius
	protected array<vector> GenerateSpawnPositions(vector center, float radius, int count)
	{
		array<vector> positions = new array<vector>();
		
		BaseWorld world = GetGame().GetWorld();
		
		for (int i = 0; i < count; i++)
		{
			float angle = Math.RandomFloat(0, Math.PI2);
			float dist = Math.RandomFloat(MIN_SPAWN_DISTANCE, radius);
			
			float x = center[0] + Math.Cos(angle) * dist;
			float z = center[2] + Math.Sin(angle) * dist;
			float y = center[1];
			
			// Get terrain height
			if (world)
				y = world.GetSurfaceY(x, z);
			
			positions.Insert(Vector(x, y, z));
		}
		
		return positions;
	}
	
	// Spawn a single unit
	protected IEntity SpawnUnit(string prefab, vector position)
	{
		if (prefab.IsEmpty())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return null;
		
		// Adjust Y to terrain
		position[1] = world.GetSurfaceY(position[0], position[2]);
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position;
		
		// Random rotation
		float yaw = Math.RandomFloat(0, 360);
		vector angles = Vector(0, yaw, 0);
		Math3D.AnglesToMatrix(angles, params.Transform);
		params.Transform[3] = position;
		
		Resource resource = Resource.Load(prefab);
		if (!resource.IsValid())
		{
			PrintFormat("[RBL_Garrison] Invalid prefab: %1", prefab);
			return null;
		}
		
		IEntity entity = GetGame().SpawnEntityPrefab(resource, world, params);
		return entity;
	}
	
	// Spawn a vehicle
	protected IEntity SpawnVehicle(string prefab, vector position)
	{
		if (prefab.IsEmpty())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return null;
		
		// Adjust Y to terrain + offset for vehicle
		position[1] = world.GetSurfaceY(position[0], position[2]) + 0.5;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position;
		
		// Random rotation
		float yaw = Math.RandomFloat(0, 360);
		vector angles = Vector(0, yaw, 0);
		Math3D.AnglesToMatrix(angles, params.Transform);
		params.Transform[3] = position;
		
		Resource resource = Resource.Load(prefab);
		if (!resource.IsValid())
		{
			PrintFormat("[RBL_Garrison] Invalid vehicle prefab: %1", prefab);
			return null;
		}
		
		IEntity entity = GetGame().SpawnEntityPrefab(resource, world, params);
		return entity;
	}
	
	// ========================================================================
	// GARRISON MANAGEMENT
	// ========================================================================
	
	void ClearGarrison(string zoneID)
	{
		RBL_GarrisonData data;
		if (m_mGarrisons.Find(zoneID, data))
		{
			data.Clear();
			m_mGarrisons.Remove(zoneID);
			PrintFormat("[RBL_Garrison] Cleared garrison at %1", zoneID);
		}
	}
	
	void ClearAllGarrisons()
	{
		array<string> zoneIDs = new array<string>();
		for (int i = 0; i < m_mGarrisons.Count(); i++)
		{
			string key;
			m_mGarrisons.GetKeyByIndex(i, key);
			zoneIDs.Insert(key);
		}
		
		for (int i = 0; i < zoneIDs.Count(); i++)
		{
			ClearGarrison(zoneIDs[i]);
		}
	}
	
	int GetGarrisonStrength(string zoneID)
	{
		RBL_GarrisonData data;
		if (m_mGarrisons.Find(zoneID, data))
			return data.CountAliveUnits();
		return 0;
	}
	
	bool IsGarrisonSpawned(string zoneID)
	{
		RBL_GarrisonData data;
		if (m_mGarrisons.Find(zoneID, data))
			return data.IsSpawned;
		return false;
	}
	
	RBL_GarrisonData GetGarrisonData(string zoneID)
	{
		RBL_GarrisonData data;
		m_mGarrisons.Find(zoneID, data);
		return data;
	}
	
	// Update all garrisons (check alive counts)
	void Update(float timeSlice)
	{
		for (int i = 0; i < m_mGarrisons.Count(); i++)
		{
			string key;
			RBL_GarrisonData data;
			m_mGarrisons.GetKeyByIndex(i, key);
			m_mGarrisons.Find(key, data);
			
			if (!data || !data.IsSpawned)
				continue;
			
			data.TimeSinceLastCheck += timeSlice;
			if (data.TimeSinceLastCheck >= GARRISON_CHECK_INTERVAL)
			{
				data.TimeSinceLastCheck = 0;
				data.CountAliveUnits();
			}
		}
	}
	
	// Spawn all garrisons for all enemy zones
	void SpawnAllGarrisons()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		PrintFormat("[RBL_Garrison] Spawning garrisons for all enemy zones...");
		
		// Virtual zones
		array<ref RBL_VirtualZone> virtualZones = zoneMgr.GetAllVirtualZones();
		for (int i = 0; i < virtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = virtualZones[i];
			if (zone.GetOwnerFaction() != ERBLFactionKey.FIA)
			{
				SpawnGarrisonForZone(
					zone.GetZoneID(),
					zone.GetZonePosition(),
					zone.GetCaptureRadius(),
					zone.GetZoneType(),
					zone.GetOwnerFaction(),
					zone.GetMaxGarrison()
				);
			}
		}
		
		// Entity zones
		array<RBL_CampaignZone> entityZones = zoneMgr.GetAllZones();
		for (int i = 0; i < entityZones.Count(); i++)
		{
			RBL_CampaignZone zone = entityZones[i];
			if (zone.GetOwnerFaction() != ERBLFactionKey.FIA)
			{
				SpawnGarrisonForZone(
					zone.GetZoneID(),
					zone.GetZonePosition(),
					zone.GetCaptureRadius(),
					zone.GetZoneType(),
					zone.GetOwnerFaction(),
					zone.GetMaxGarrison()
				);
			}
		}
	}
	
	// Get stats
	int GetTotalSpawnedUnits()
	{
		int total = 0;
		for (int i = 0; i < m_mGarrisons.Count(); i++)
		{
			string key;
			RBL_GarrisonData data;
			m_mGarrisons.GetKeyByIndex(i, key);
			m_mGarrisons.Find(key, data);
			if (data)
				total += data.SpawnedUnits.Count();
		}
		return total;
	}
	
	int GetTotalSpawnedVehicles()
	{
		int total = 0;
		for (int i = 0; i < m_mGarrisons.Count(); i++)
		{
			string key;
			RBL_GarrisonData data;
			m_mGarrisons.GetKeyByIndex(i, key);
			m_mGarrisons.Find(key, data);
			if (data)
				total += data.SpawnedVehicles.Count();
		}
		return total;
	}
	
	int GetGarrisonedZoneCount()
	{
		return m_mGarrisons.Count();
	}
}

// ============================================================================
// DEBUG COMMANDS
// ============================================================================
class RBL_GarrisonCommands
{
	static void SpawnAll()
	{
		RBL_GarrisonManager mgr = RBL_GarrisonManager.GetInstance();
		if (mgr)
			mgr.SpawnAllGarrisons();
	}
	
	static void ClearAll()
	{
		RBL_GarrisonManager mgr = RBL_GarrisonManager.GetInstance();
		if (mgr)
			mgr.ClearAllGarrisons();
	}
	
	static void SpawnZone(string zoneID)
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		
		if (!zoneMgr || !garMgr)
			return;
		
		RBL_VirtualZone zone = zoneMgr.GetVirtualZoneByID(zoneID);
		if (zone)
		{
			garMgr.SpawnGarrisonForZone(
				zone.GetZoneID(),
				zone.GetZonePosition(),
				zone.GetCaptureRadius(),
				zone.GetZoneType(),
				zone.GetOwnerFaction(),
				zone.GetMaxGarrison()
			);
		}
		else
		{
			PrintFormat("[RBL_Garrison] Zone not found: %1", zoneID);
		}
	}
	
	static void PrintStats()
	{
		RBL_GarrisonManager mgr = RBL_GarrisonManager.GetInstance();
		if (!mgr)
			return;
		
		PrintFormat("[RBL_Garrison] === GARRISON STATS ===");
		PrintFormat("Garrisoned zones: %1", mgr.GetGarrisonedZoneCount());
		PrintFormat("Total units: %1", mgr.GetTotalSpawnedUnits());
		PrintFormat("Total vehicles: %1", mgr.GetTotalSpawnedVehicles());
	}
}
