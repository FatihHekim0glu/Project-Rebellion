// ============================================================================
// PROJECT REBELLION - Zone Configuration
// Auto-spawns zones from config - no manual placement needed
// ============================================================================

class RBL_ZoneDefinition
{
	string ZoneID;
	vector Position;
	ERBLZoneType Type;
	ERBLFactionKey StartingOwner;
	int MaxGarrison;
	int CivilianSupport;
	float CaptureRadius;
	
	void RBL_ZoneDefinition()
	{
		MaxGarrison = 10;
		CivilianSupport = 50;
		CaptureRadius = 100;
		StartingOwner = ERBLFactionKey.USSR;
	}
}

class RBL_ZoneConfigurator
{
	protected static ref RBL_ZoneConfigurator s_Instance;
	protected ref array<ref RBL_ZoneDefinition> m_aZoneDefinitions;
	
	static RBL_ZoneConfigurator GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_ZoneConfigurator();
		return s_Instance;
	}
	
	void RBL_ZoneConfigurator()
	{
		m_aZoneDefinitions = new array<ref RBL_ZoneDefinition>();
		LoadEveronZones();
	}
	
	protected void LoadEveronZones()
	{
		// FIA Starting HQ
		AddZone("HQ_FIA", "7500 0 3500", ERBLZoneType.HQ, ERBLFactionKey.FIA, 5, 90, 150);
		
		// USSR Main Bases
		AddZone("Airbase_Everon", "4800 0 6200", ERBLZoneType.Airbase, ERBLFactionKey.USSR, 30, 10, 200);
		AddZone("HQ_USSR", "2500 0 8000", ERBLZoneType.HQ, ERBLFactionKey.USSR, 40, 5, 200);
		
		// Towns
		AddZone("Town_Morton", "3200 0 5400", ERBLZoneType.Town, ERBLFactionKey.USSR, 15, 45, 150);
		AddZone("Town_Levie", "5600 0 4200", ERBLZoneType.Town, ERBLFactionKey.USSR, 12, 55, 120);
		AddZone("Town_Regina", "4100 0 7800", ERBLZoneType.Town, ERBLFactionKey.USSR, 18, 35, 140);
		AddZone("Town_Montignac", "6200 0 6800", ERBLZoneType.Town, ERBLFactionKey.USSR, 20, 40, 160);
		AddZone("Town_Lamentin", "2800 0 4600", ERBLZoneType.Town, ERBLFactionKey.USSR, 10, 60, 100);
		AddZone("Town_Gravette", "5200 0 8200", ERBLZoneType.Town, ERBLFactionKey.USSR, 14, 50, 130);
		
		// Outposts
		AddZone("Outpost_Alpha", "3800 0 4800", ERBLZoneType.Outpost, ERBLFactionKey.USSR, 8, 20, 80);
		AddZone("Outpost_Bravo", "5000 0 5600", ERBLZoneType.Outpost, ERBLFactionKey.USSR, 8, 20, 80);
		AddZone("Outpost_Charlie", "4400 0 7200", ERBLZoneType.Outpost, ERBLFactionKey.USSR, 10, 15, 80);
		AddZone("Outpost_Delta", "6800 0 5800", ERBLZoneType.Outpost, ERBLFactionKey.USSR, 8, 25, 80);
		AddZone("Outpost_Echo", "3000 0 6800", ERBLZoneType.Outpost, ERBLFactionKey.USSR, 6, 30, 80);
		
		// Resource/Factory locations
		AddZone("Factory_East", "6400 0 7400", ERBLZoneType.Factory, ERBLFactionKey.USSR, 12, 25, 100);
		AddZone("Resource_Quarry", "4000 0 6000", ERBLZoneType.Resource, ERBLFactionKey.USSR, 6, 30, 90);
		AddZone("Seaport_South", "5800 0 3000", ERBLZoneType.Seaport, ERBLFactionKey.USSR, 15, 35, 120);
		
		// Roadblocks / Watchposts
		AddZone("Roadblock_North", "4200 0 8600", ERBLZoneType.Roadblock, ERBLFactionKey.USSR, 4, 10, 50);
		AddZone("Roadblock_West", "2400 0 5200", ERBLZoneType.Roadblock, ERBLFactionKey.USSR, 4, 10, 50);
		AddZone("Watchpost_Hill", "5400 0 5000", ERBLZoneType.Watchpost, ERBLFactionKey.USSR, 3, 15, 40);
		AddZone("Watchpost_Coast", "7000 0 4400", ERBLZoneType.Watchpost, ERBLFactionKey.USSR, 3, 20, 40);
		
		PrintFormat("[RBL] Loaded %1 zone definitions for Everon", m_aZoneDefinitions.Count());
	}
	
	protected void AddZone(string id, string posStr, ERBLZoneType type, ERBLFactionKey owner, int garrison, int support, float radius)
	{
		RBL_ZoneDefinition def = new RBL_ZoneDefinition();
		def.ZoneID = id;
		def.Position = posStr.ToVector();
		def.Type = type;
		def.StartingOwner = owner;
		def.MaxGarrison = garrison;
		def.CivilianSupport = support;
		def.CaptureRadius = radius;
		m_aZoneDefinitions.Insert(def);
	}
	
	array<ref RBL_ZoneDefinition> GetAllDefinitions()
	{
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
}

