// ============================================================================
// PROJECT REBELLION - Map Zone Markers
// Creates actual map markers at zone positions
// ============================================================================

class RBL_MapZoneMarkers
{
	protected static ref RBL_MapZoneMarkers s_Instance;
	protected ref array<IEntity> m_aMarkerEntities;
	protected ref map<string, IEntity> m_mZoneToEntity;
	protected bool m_bInitialized;
	protected bool m_bMarkersCreated;
	
	static RBL_MapZoneMarkers GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_MapZoneMarkers();
		return s_Instance;
	}
	
	void RBL_MapZoneMarkers()
	{
		m_aMarkerEntities = new array<IEntity>();
		m_mZoneToEntity = new map<string, IEntity>();
		m_bInitialized = false;
		m_bMarkersCreated = false;
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		m_bInitialized = true;
		
		// Delay marker creation to ensure zones are loaded
		GetGame().GetCallqueue().CallLater(CreateZoneMarkers, 3000, false);
		
		PrintFormat("[RBL_MapMarkers] Map marker system initialized");
	}
	
	void CreateZoneMarkers()
	{
		if (m_bMarkersCreated)
			return;
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
		{
			PrintFormat("[RBL_MapMarkers] Zone manager not available, retrying...");
			GetGame().GetCallqueue().CallLater(CreateZoneMarkers, 2000, false);
			return;
		}
		
		array<ref RBL_VirtualZone> zones = zoneMgr.GetAllVirtualZones();
		if (!zones || zones.Count() == 0)
		{
			PrintFormat("[RBL_MapMarkers] No zones found, retrying...");
			GetGame().GetCallqueue().CallLater(CreateZoneMarkers, 2000, false);
			return;
		}
		
		PrintFormat("[RBL_MapMarkers] Creating %1 zone markers...", zones.Count());
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
		{
			PrintFormat("[RBL_MapMarkers] World not available");
			return;
		}
		
		int created = 0;
		for (int i = 0; i < zones.Count(); i++)
		{
			RBL_VirtualZone zone = zones[i];
			if (!zone)
				continue;
			
			if (CreateMarkerForZone(zone, world))
				created++;
		}
		
		m_bMarkersCreated = true;
		PrintFormat("[RBL_MapMarkers] Created %1 map markers", created);
	}
	
	protected bool CreateMarkerForZone(RBL_VirtualZone zone, BaseWorld world)
	{
		if (!zone || !world)
			return false;
		
		vector pos = zone.GetZonePosition();
		string zoneID = zone.GetZoneID();
		string zoneName = zone.GetZoneName();
		
		// Use our custom marker prefab
		ResourceName markerPrefab = "{BADE082C5B0F836A}Prefabs/Markers/RBL_ZoneMarker.et";
		
		// Load the prefab resource
		Resource res = Resource.Load(markerPrefab);
		if (!res || !res.IsValid())
		{
			PrintFormat("[RBL_MapMarkers] Failed to load marker prefab for %1", zoneID);
			return false;
		}
		
		// Spawn the marker entity at zone position
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = pos;
		
		IEntity markerEnt = GetGame().SpawnEntityPrefab(res, world, params);
		if (!markerEnt)
		{
			PrintFormat("[RBL_MapMarkers] Failed to spawn marker for %1", zoneID);
			return false;
		}
		
		// Configure the map descriptor with zone name
		SCR_MapDescriptorComponent mapDesc = SCR_MapDescriptorComponent.Cast(markerEnt.FindComponent(SCR_MapDescriptorComponent));
		if (mapDesc)
		{
			MapItem item = mapDesc.Item();
			if (item)
			{
				item.SetDisplayName(zoneName);
				item.SetVisible(true);
			}
		}
		
		// Store reference
		m_aMarkerEntities.Insert(markerEnt);
		m_mZoneToEntity.Set(zoneID, markerEnt);
		
		PrintFormat("[RBL_MapMarkers] Created marker: %1 at [%2, %3]", zoneName, pos[0], pos[2]);
		return true;
	}
	
	void UpdateZoneMarker(string zoneID, ERBLFactionKey newOwner)
	{
		PrintFormat("[RBL_MapMarkers] Zone %1 ownership changed to %2", zoneID, typename.EnumToString(ERBLFactionKey, newOwner));
	}
	
	void Cleanup()
	{
		foreach (IEntity ent : m_aMarkerEntities)
		{
			if (ent)
				delete ent;
		}
		m_aMarkerEntities.Clear();
		m_mZoneToEntity.Clear();
	}
}

// ============================================================================
// PLACEHOLDER - Debug overlay removed
// Map markers require prefab setup in Workbench
// ============================================================================
class RBL_MapOverlayDebug
{
	protected static ref RBL_MapOverlayDebug s_Instance;
	
	static RBL_MapOverlayDebug GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_MapOverlayDebug();
		return s_Instance;
	}
	
	void Update(float timeSlice) {}
	void Draw() {}
}

// ============================================================================
// MAP MARKER DATA
// ============================================================================
class RBL_MapIconData
{
	string m_sZoneID;
	string m_sDisplayName;
	vector m_vWorldPos;
	ERBLZoneType m_eZoneType;
	ERBLFactionKey m_eOwner;
	
	void RBL_MapIconData()
	{
		m_sZoneID = "";
		m_sDisplayName = "";
		m_vWorldPos = vector.Zero;
		m_eZoneType = ERBLZoneType.Town;
		m_eOwner = ERBLFactionKey.USSR;
	}
}
