// ============================================================================
// PROJECT REBELLION - Zone Configuration
// Everon map with real coordinates from game data
// ============================================================================

// Vehicle spawn types for zones
class RBL_VehicleSpawnType
{
	static const int NONE = 0;
	static const int TRUCKS = 1;
	static const int APCS = 2;
	static const int HELICOPTERS = 4;
	static const int BOATS = 8;
	static const int LIGHT_VEHICLES = 16;
}

class RBL_ZoneDefinition
{
	string ZoneID;
	string DisplayName;
	vector Position;
	ERBLZoneType Type;
	ERBLFactionKey StartingOwner;
	int MaxGarrison;
	int CivilianSupport;
	float CaptureRadius;
	int ResourceValue;
	int VehicleSpawns;
	int LootTier;
	
	void RBL_ZoneDefinition()
	{
		MaxGarrison = 10;
		CivilianSupport = 50;
		CaptureRadius = 100;
		StartingOwner = ERBLFactionKey.USSR;
		ResourceValue = 100;
		VehicleSpawns = RBL_VehicleSpawnType.NONE;
		LootTier = 1;
	}
}

class RBL_ZoneConfigurator
{
	protected static ref RBL_ZoneConfigurator s_Instance;
	protected ref array<ref RBL_ZoneDefinition> m_aZoneDefinitions;
	protected bool m_bTerrainHeightsResolved;
	
	static RBL_ZoneConfigurator GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_ZoneConfigurator();
		return s_Instance;
	}
	
	void RBL_ZoneConfigurator()
	{
		m_aZoneDefinitions = new array<ref RBL_ZoneDefinition>();
		m_bTerrainHeightsResolved = false;
		LoadEveronZones();
	}
	
	protected void LoadEveronZones()
	{
		// ====================================================================
		// PLAYER HQ - Starting location (pick a remote town for rebels)
		// ====================================================================
		AddZoneEx("HQ_FIA", "Rebel HQ", 1279.669, 5940.503, ERBLZoneType.HQ, ERBLFactionKey.FIA, 
			5, 90, 150, 50, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		// ====================================================================
		// TOWNS & CITIES (20 locations) - Civilian population centers
		// Garrison: 8-20, Support: 40-70, Radius: 120-180
		// ====================================================================
		AddZoneEx("Town_StPierre", "St. Pierre", 9689.018, 1558.482, ERBLZoneType.Town, ERBLFactionKey.USSR,
			12, 55, 150, 120, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		AddZoneEx("Town_Chotain", "Chotain", 7086.131, 6012.347, ERBLZoneType.Town, ERBLFactionKey.USSR,
			15, 50, 160, 150, RBL_VehicleSpawnType.TRUCKS, 2);
		
		AddZoneEx("Town_Durras", "Durras", 8826.849, 2745.832, ERBLZoneType.Town, ERBLFactionKey.USSR,
			10, 60, 140, 100, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		AddZoneEx("Town_EntreDeux", "Entre-Deux", 5766.788, 7035.573, ERBLZoneType.Town, ERBLFactionKey.USSR,
			14, 45, 150, 130, RBL_VehicleSpawnType.TRUCKS, 2);
		
		AddZoneEx("Town_Erquy", "Erquy", 10918.685, 11706.15, ERBLZoneType.Town, ERBLFactionKey.USSR,
			8, 65, 130, 80, RBL_VehicleSpawnType.LIGHT_VEHICLES, 1);
		
		AddZoneEx("Town_Figari", "Figari", 5250.764, 5337.823, ERBLZoneType.Town, ERBLFactionKey.USSR,
			16, 50, 160, 140, RBL_VehicleSpawnType.TRUCKS, 2);
		
		AddZoneEx("Town_Gravette", "Gravette", 4120.761, 7796.203, ERBLZoneType.Town, ERBLFactionKey.USSR,
			12, 55, 150, 110, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		AddZoneEx("Town_Lamentin", "Lamentin", 1279.669, 5940.503, ERBLZoneType.Town, ERBLFactionKey.FIA,
			8, 70, 120, 80, RBL_VehicleSpawnType.LIGHT_VEHICLES, 1);
		
		AddZoneEx("Town_Laruns", "Laruns", 7558.336, 5540.855, ERBLZoneType.Town, ERBLFactionKey.USSR,
			14, 50, 150, 130, RBL_VehicleSpawnType.TRUCKS, 2);
		
		AddZoneEx("Town_LeMoule", "Le Moule", 2615.962, 5378.784, ERBLZoneType.Town, ERBLFactionKey.USSR,
			10, 60, 140, 100, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		AddZoneEx("Town_Levie", "Levie", 7464.442, 4738.911, ERBLZoneType.Town, ERBLFactionKey.USSR,
			18, 45, 170, 160, RBL_VehicleSpawnType.TRUCKS, 3);
		
		AddZoneEx("Town_Meaux", "Meaux", 4520.45, 9467.977, ERBLZoneType.Town, ERBLFactionKey.USSR,
			10, 55, 140, 100, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		AddZoneEx("Town_Montignac", "Montignac", 4773.455, 7094.566, ERBLZoneType.Town, ERBLFactionKey.USSR,
			20, 40, 180, 180, RBL_VehicleSpawnType.TRUCKS, 3);
		
		AddZoneEx("Town_Morton", "Morton", 5135.238, 4011.78, ERBLZoneType.Town, ERBLFactionKey.USSR,
			15, 50, 160, 140, RBL_VehicleSpawnType.TRUCKS, 2);
		
		AddZoneEx("Town_Provins", "Provins", 5486.487, 6087.398, ERBLZoneType.Town, ERBLFactionKey.USSR,
			12, 55, 150, 120, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		AddZoneEx("Town_Regina", "Regina", 7205.023, 2324.232, ERBLZoneType.Town, ERBLFactionKey.USSR,
			14, 50, 160, 130, RBL_VehicleSpawnType.TRUCKS, 2);
		
		AddZoneEx("Town_StPhilippe", "St. Philippe", 4502.756, 10771.987, ERBLZoneType.Town, ERBLFactionKey.USSR,
			10, 60, 140, 100, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		AddZoneEx("Town_Tyrone", "Tyrone", 4927.979, 9092.888, ERBLZoneType.Town, ERBLFactionKey.USSR,
			12, 55, 150, 110, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		AddZoneEx("Town_Vernon", "Vernon", 9243.437, 2077.243, ERBLZoneType.Town, ERBLFactionKey.USSR,
			10, 60, 140, 100, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		AddZoneEx("Town_Villeneuve", "Villeneuve", 2838.686, 6339.803, ERBLZoneType.Town, ERBLFactionKey.USSR,
			12, 55, 150, 110, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		// ====================================================================
		// INDUSTRIAL ZONES (3 locations) - Resource production
		// Higher resource value, moderate garrison
		// ====================================================================
		AddZoneEx("Factory_North", "Northern Factory", 4536.628, 10561.202, ERBLZoneType.Factory, ERBLFactionKey.USSR,
			15, 25, 120, 300, RBL_VehicleSpawnType.TRUCKS, 3);
		
		AddZoneEx("Sawmill_West", "Western Sawmill", 3069.223, 5195.226, ERBLZoneType.Resource, ERBLFactionKey.USSR,
			8, 35, 100, 200, RBL_VehicleSpawnType.TRUCKS, 2);
		
		AddZoneEx("Sawmill_South", "Southern Sawmill", 7272.375, 2541.164, ERBLZoneType.Resource, ERBLFactionKey.USSR,
			8, 35, 100, 200, RBL_VehicleSpawnType.TRUCKS, 2);
		
		// ====================================================================
		// SMALL LOCATIONS (1) - Hamlet/Outpost
		// ====================================================================
		AddZoneEx("Hamlet_Perelle", "Perelle", 9341.147, 5080.949, ERBLZoneType.Outpost, ERBLFactionKey.USSR,
			4, 50, 80, 50, RBL_VehicleSpawnType.NONE, 1);
		
		// ====================================================================
		// MILITARY BASES (3) - Heavy garrison, APCs, high loot
		// ====================================================================
		AddZoneEx("MilBase_Coastal", "Coastal Military Base", 7447.122, 6704.909, ERBLZoneType.HQ, ERBLFactionKey.USSR,
			35, 10, 200, 400, RBL_VehicleSpawnType.APCS | RBL_VehicleSpawnType.TRUCKS, 4);
		
		AddZoneEx("MilBase_Northern", "Northern Military Base", 3901.866, 8450.66, ERBLZoneType.Outpost, ERBLFactionKey.USSR,
			25, 15, 180, 350, RBL_VehicleSpawnType.APCS | RBL_VehicleSpawnType.TRUCKS, 4);
		
		AddZoneEx("MilBase_Highland", "Highland Military Base", 7505.482, 4283.153, ERBLZoneType.Outpost, ERBLFactionKey.USSR,
			20, 15, 160, 300, RBL_VehicleSpawnType.APCS | RBL_VehicleSpawnType.LIGHT_VEHICLES, 3);
		
		// ====================================================================
		// SHOOTING RANGE (1) - Training facility
		// ====================================================================
		AddZoneEx("ShootingRange", "Military Shooting Range", 7631.798, 8141.2, ERBLZoneType.Outpost, ERBLFactionKey.USSR,
			12, 20, 120, 150, RBL_VehicleSpawnType.LIGHT_VEHICLES, 3);
		
		// ====================================================================
		// AIRPORTS (2) - Helicopter spawns, high strategic value
		// ====================================================================
		AddZoneEx("Airport_Main", "Main Airport", 4936.765, 11921.946, ERBLZoneType.Airbase, ERBLFactionKey.USSR,
			30, 10, 250, 500, RBL_VehicleSpawnType.HELICOPTERS | RBL_VehicleSpawnType.TRUCKS, 4);
		
		AddZoneEx("Airport_Secondary", "Secondary Airfield", 5354.837, 10542.704, ERBLZoneType.Airbase, ERBLFactionKey.USSR,
			20, 15, 200, 400, RBL_VehicleSpawnType.HELICOPTERS | RBL_VehicleSpawnType.LIGHT_VEHICLES, 3);
		
		// ====================================================================
		// RADIO TOWER (1) - Communications, intel value
		// ====================================================================
		AddZoneEx("RadioTower", "Radio Communications Tower", 1931.796, 5138.725, ERBLZoneType.Watchpost, ERBLFactionKey.USSR,
			6, 20, 80, 150, RBL_VehicleSpawnType.LIGHT_VEHICLES, 2);
		
		// ====================================================================
		// SEAPORT (1) - Naval access, boat spawns
		// ====================================================================
		AddZoneEx("Seaport_Main", "Main Seaport", 5135.238, 3800.0, ERBLZoneType.Seaport, ERBLFactionKey.USSR,
			18, 30, 150, 250, RBL_VehicleSpawnType.BOATS | RBL_VehicleSpawnType.TRUCKS, 3);
		
		// ====================================================================
		// ROADBLOCKS & CHECKPOINTS - Light garrison, strategic choke points
		// ====================================================================
		AddZoneEx("Checkpoint_North", "Northern Checkpoint", 5000.0, 10000.0, ERBLZoneType.Roadblock, ERBLFactionKey.USSR,
			4, 15, 60, 30, RBL_VehicleSpawnType.LIGHT_VEHICLES, 1);
		
		AddZoneEx("Checkpoint_East", "Eastern Checkpoint", 9000.0, 4000.0, ERBLZoneType.Roadblock, ERBLFactionKey.USSR,
			4, 15, 60, 30, RBL_VehicleSpawnType.LIGHT_VEHICLES, 1);
		
		AddZoneEx("Checkpoint_West", "Western Checkpoint", 2000.0, 6500.0, ERBLZoneType.Roadblock, ERBLFactionKey.USSR,
			4, 15, 60, 30, RBL_VehicleSpawnType.LIGHT_VEHICLES, 1);
		
		AddZoneEx("Checkpoint_South", "Southern Checkpoint", 6500.0, 2000.0, ERBLZoneType.Roadblock, ERBLFactionKey.USSR,
			4, 15, 60, 30, RBL_VehicleSpawnType.LIGHT_VEHICLES, 1);
		
		PrintFormat("[RBL] Loaded %1 zone definitions for Everon", m_aZoneDefinitions.Count());
	}
	
	protected void AddZone(string id, float x, float z, ERBLZoneType type, ERBLFactionKey owner, int garrison, int support, float radius)
	{
		AddZoneEx(id, id, x, z, type, owner, garrison, support, radius, 100, RBL_VehicleSpawnType.NONE, 1);
	}
	
	protected void AddZoneEx(string id, string displayName, float x, float z, ERBLZoneType type, ERBLFactionKey owner, 
		int garrison, int support, float radius, int resourceValue, int vehicleSpawns, int lootTier)
	{
		RBL_ZoneDefinition def = new RBL_ZoneDefinition();
		def.ZoneID = id;
		def.DisplayName = displayName;
		def.Position = Vector(x, 0, z);
		def.Type = type;
		def.StartingOwner = owner;
		def.MaxGarrison = garrison;
		def.CivilianSupport = support;
		def.CaptureRadius = radius;
		def.ResourceValue = resourceValue;
		def.VehicleSpawns = vehicleSpawns;
		def.LootTier = lootTier;
		m_aZoneDefinitions.Insert(def);
	}
	
	void ResolveTerrainHeights()
	{
		if (m_bTerrainHeightsResolved)
			return;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
		{
			PrintFormat("[RBL] Warning: Cannot resolve terrain heights - world not available");
			return;
		}
		
		int resolved = 0;
		for (int i = 0; i < m_aZoneDefinitions.Count(); i++)
		{
			RBL_ZoneDefinition def = m_aZoneDefinitions[i];
			float terrainY = GetTerrainHeight(world, def.Position[0], def.Position[2]);
			def.Position[1] = terrainY;
			resolved++;
		}
		
		m_bTerrainHeightsResolved = true;
		PrintFormat("[RBL] Resolved terrain heights for %1 zones", resolved);
	}
	
	static float GetTerrainHeight(BaseWorld world, float x, float z)
	{
		if (!world)
			return 0;
		
		float terrainY = world.GetSurfaceY(x, z);
		
		if (terrainY > -1000)
			return terrainY;
		
		return 0;
	}
	
	static float GetTerrainHeightAtPosition(vector pos)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return pos[1];
		
		return GetTerrainHeight(world, pos[0], pos[2]);
	}
	
	static vector SnapToTerrain(vector pos, float heightOffset = 0)
	{
		float terrainY = GetTerrainHeightAtPosition(pos);
		return Vector(pos[0], terrainY + heightOffset, pos[2]);
	}
	
	array<ref RBL_ZoneDefinition> GetAllDefinitions()
	{
		if (!m_bTerrainHeightsResolved)
			ResolveTerrainHeights();
		
		return m_aZoneDefinitions;
	}
	
	RBL_ZoneDefinition GetDefinitionByID(string zoneID)
	{
		for (int i = 0; i < m_aZoneDefinitions.Count(); i++)
		{
			if (m_aZoneDefinitions[i].ZoneID == zoneID)
				return m_aZoneDefinitions[i];
		}
		return null;
	}
	
	bool AreTerrainHeightsResolved() { return m_bTerrainHeightsResolved; }
	
	// Get zones by vehicle spawn type
	array<ref RBL_ZoneDefinition> GetZonesWithVehicles(int vehicleType)
	{
		array<ref RBL_ZoneDefinition> result = new array<ref RBL_ZoneDefinition>();
		
		for (int i = 0; i < m_aZoneDefinitions.Count(); i++)
		{
			if (m_aZoneDefinitions[i].VehicleSpawns & vehicleType)
				result.Insert(m_aZoneDefinitions[i]);
		}
		
		return result;
	}
	
	// Get total resource value for a faction
	int GetTotalResourceValue(ERBLFactionKey faction)
	{
		int total = 0;
		
		for (int i = 0; i < m_aZoneDefinitions.Count(); i++)
		{
			if (m_aZoneDefinitions[i].StartingOwner == faction)
				total += m_aZoneDefinitions[i].ResourceValue;
		}
		
		return total;
	}
}
