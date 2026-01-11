// ============================================================================
// PROJECT REBELLION - Zone Manager
// Manages the grid of all campaign zones, tracks ownership and simulation
// ============================================================================

class RBL_ZoneManager
{
	protected static RBL_ZoneManager s_Instance;
	
	// ========================================================================
	// STATE
	// ========================================================================
	
	protected ref array<RBL_CampaignZone> m_aAllZones;
	protected ref map<string, RBL_CampaignZone> m_mZonesByID;
	protected ref map<ERBLFactionKey, ref array<RBL_CampaignZone>> m_mZonesByFaction;
	
	protected float m_fTimeSinceSimulation;
	protected const float SIMULATION_INTERVAL = 5.0;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_ZoneManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_ZoneManager();
		return s_Instance;
	}
	
	void RBL_ZoneManager()
	{
		m_aAllZones = new array<RBL_CampaignZone>();
		m_mZonesByID = new map<string, RBL_CampaignZone>();
		m_mZonesByFaction = new map<ERBLFactionKey, ref array<RBL_CampaignZone>>();
		
		// Initialize faction arrays
		m_mZonesByFaction.Set(ERBLFactionKey.NONE, new array<RBL_CampaignZone>());
		m_mZonesByFaction.Set(ERBLFactionKey.US, new array<RBL_CampaignZone>());
		m_mZonesByFaction.Set(ERBLFactionKey.USSR, new array<RBL_CampaignZone>());
		m_mZonesByFaction.Set(ERBLFactionKey.FIA, new array<RBL_CampaignZone>());
		
		m_fTimeSinceSimulation = 0;
	}
	
	// ========================================================================
	// ZONE REGISTRATION
	// ========================================================================
	
	void RegisterZone(RBL_CampaignZone zone)
	{
		if (!zone)
			return;
		
		string zoneID = zone.GetZoneID();
		
		if (m_mZonesByID.Contains(zoneID))
		{
			PrintFormat("[RBL_ZoneManager] Warning: Zone %1 already registered", zoneID);
			return;
		}
		
		m_aAllZones.Insert(zone);
		m_mZonesByID.Set(zoneID, zone);
		
		// Add to faction list
		ERBLFactionKey owner = zone.GetOwnerFaction();
		array<RBL_CampaignZone> factionZones = m_mZonesByFaction.Get(owner);
		if (factionZones)
			factionZones.Insert(zone);
		
		// Subscribe to zone events
		zone.GetOnZoneCaptured().Insert(OnZoneOwnershipChanged);
		
		PrintFormat("[RBL_ZoneManager] Registered zone: %1 (%2)", zoneID, typename.EnumToString(ERBLZoneType, zone.GetZoneType()));
	}
	
	void UnregisterZone(RBL_CampaignZone zone)
	{
		if (!zone)
			return;
		
		string zoneID = zone.GetZoneID();
		
		m_aAllZones.RemoveItem(zone);
		m_mZonesByID.Remove(zoneID);
		
		// Remove from faction list
		ERBLFactionKey owner = zone.GetOwnerFaction();
		array<RBL_CampaignZone> factionZones = m_mZonesByFaction.Get(owner);
		if (factionZones)
			factionZones.RemoveItem(zone);
		
		zone.GetOnZoneCaptured().Remove(OnZoneOwnershipChanged);
	}
	
	// ========================================================================
	// UPDATE LOOP
	// ========================================================================
	
	void Update(float timeSlice)
	{
		if (!Replication.IsServer())
			return;
		
		// Update capture states
		foreach (RBL_CampaignZone zone : m_aAllZones)
		{
			zone.UpdateCaptureState(timeSlice);
		}
		
		// Simulation tick for distant zones
		m_fTimeSinceSimulation += timeSlice;
		if (m_fTimeSinceSimulation >= SIMULATION_INTERVAL)
		{
			m_fTimeSinceSimulation = 0;
			SimulateDistantZones();
		}
	}
	
	protected void SimulateDistantZones()
	{
		// Get player positions
		array<vector> playerPositions = GetAllPlayerPositions();
		
		foreach (RBL_CampaignZone zone : m_aAllZones)
		{
			bool isNearPlayer = false;
			vector zonePos = zone.GetZonePosition();
			
			foreach (vector playerPos : playerPositions)
			{
				if (vector.Distance(zonePos, playerPos) < 1000)
				{
					isNearPlayer = true;
					break;
				}
			}
			
			// Zones far from players get simplified simulation
			if (!isNearPlayer)
			{
				SimulateZone(zone);
			}
		}
	}
	
	protected void SimulateZone(RBL_CampaignZone zone)
	{
		// Simplified garrison regeneration for distant zones
		// Prevents "empty" zones when players aren't looking
		
		if (zone.GetCurrentGarrison() < zone.GetMaxGarrison() * 0.5)
		{
			// Slowly regenerate garrison over time
			// This represents reinforcements arriving
		}
	}
	
	protected array<vector> GetAllPlayerPositions()
	{
		array<vector> positions = {};
		
		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		
		foreach (int playerID : playerIDs)
		{
			IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (playerEntity)
				positions.Insert(playerEntity.GetOrigin());
		}
		
		return positions;
	}
	
	// ========================================================================
	// ZONE QUERIES
	// ========================================================================
	
	RBL_CampaignZone GetZoneByID(string zoneID)
	{
		RBL_CampaignZone zone;
		m_mZonesByID.Find(zoneID, zone);
		return zone;
	}
	
	array<RBL_CampaignZone> GetAllZones()
	{
		return m_aAllZones;
	}
	
	array<RBL_CampaignZone> GetZonesByFaction(ERBLFactionKey faction)
	{
		array<RBL_CampaignZone> result = m_mZonesByFaction.Get(faction);
		if (!result)
			result = new array<RBL_CampaignZone>();
		return result;
	}
	
	array<RBL_CampaignZone> GetZonesByType(ERBLZoneType type)
	{
		array<RBL_CampaignZone> result = {};
		
		foreach (RBL_CampaignZone zone : m_aAllZones)
		{
			if (zone.GetZoneType() == type)
				result.Insert(zone);
		}
		
		return result;
	}
	
	RBL_CampaignZone GetNearestZone(vector position)
	{
		RBL_CampaignZone nearest = null;
		float nearestDist = float.MAX;
		
		foreach (RBL_CampaignZone zone : m_aAllZones)
		{
			float dist = vector.Distance(position, zone.GetZonePosition());
			if (dist < nearestDist)
			{
				nearestDist = dist;
				nearest = zone;
			}
		}
		
		return nearest;
	}
	
	RBL_CampaignZone GetNearestFriendlyZone(vector position, ERBLFactionKey faction)
	{
		RBL_CampaignZone nearest = null;
		float nearestDist = float.MAX;
		
		array<RBL_CampaignZone> factionZones = GetZonesByFaction(faction);
		
		foreach (RBL_CampaignZone zone : factionZones)
		{
			float dist = vector.Distance(position, zone.GetZonePosition());
			if (dist < nearestDist)
			{
				nearestDist = dist;
				nearest = zone;
			}
		}
		
		return nearest;
	}
	
	array<RBL_CampaignZone> GetZonesInRadius(vector position, float radius)
	{
		array<RBL_CampaignZone> result = {};
		
		foreach (RBL_CampaignZone zone : m_aAllZones)
		{
			if (vector.Distance(position, zone.GetZonePosition()) <= radius)
				result.Insert(zone);
		}
		
		return result;
	}
	
	RBL_CampaignZone GetPlayerHQ()
	{
		array<RBL_CampaignZone> fiaZones = GetZonesByFaction(ERBLFactionKey.FIA);
		
		foreach (RBL_CampaignZone zone : fiaZones)
		{
			if (zone.GetZoneType() == ERBLZoneType.HQ)
				return zone;
		}
		
		return null;
	}
	
	// ========================================================================
	// FACTION STATISTICS
	// ========================================================================
	
	int GetZoneCountByFaction(ERBLFactionKey faction)
	{
		array<RBL_CampaignZone> factionZones = GetZonesByFaction(faction);
		return factionZones.Count();
	}
	
	int GetTotalGarrisonByFaction(ERBLFactionKey faction)
	{
		int total = 0;
		array<RBL_CampaignZone> factionZones = GetZonesByFaction(faction);
		
		foreach (RBL_CampaignZone zone : factionZones)
		{
			total += zone.GetCurrentGarrison();
		}
		
		return total;
	}
	
	float GetAverageSupportByFaction(ERBLFactionKey faction)
	{
		array<RBL_CampaignZone> factionZones = GetZonesByFaction(faction);
		
		if (factionZones.Count() == 0)
			return 0;
		
		int totalSupport = 0;
		foreach (RBL_CampaignZone zone : factionZones)
		{
			totalSupport += zone.GetCivilianSupport();
		}
		
		return totalSupport / (float)factionZones.Count();
	}
	
	bool AreAllZonesCapturedByFaction(ERBLFactionKey faction)
	{
		foreach (RBL_CampaignZone zone : m_aAllZones)
		{
			if (zone.GetOwnerFaction() != faction)
				return false;
		}
		return true;
	}
	
	// ========================================================================
	// GARRISON MANAGEMENT
	// ========================================================================
	
	void SpawnAllGarrisons()
	{
		foreach (RBL_CampaignZone zone : m_aAllZones)
		{
			// Only spawn for enemy-controlled zones
			if (zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				zone.SpawnGarrison();
		}
	}
	
	void ClearAllGarrisons()
	{
		foreach (RBL_CampaignZone zone : m_aAllZones)
		{
			zone.ClearGarrison();
		}
	}
	
	// ========================================================================
	// EVENT HANDLERS
	// ========================================================================
	
	protected void OnZoneOwnershipChanged(RBL_CampaignZone zone, ERBLFactionKey previousOwner, ERBLFactionKey newOwner)
	{
		// Update faction lists
		array<RBL_CampaignZone> oldFactionZones = m_mZonesByFaction.Get(previousOwner);
		if (oldFactionZones)
			oldFactionZones.RemoveItem(zone);
		
		array<RBL_CampaignZone> newFactionZones = m_mZonesByFaction.Get(newOwner);
		if (newFactionZones)
			newFactionZones.Insert(zone);
		
		PrintFormat("[RBL_ZoneManager] Zone %1 changed ownership: %2 -> %3",
			zone.GetZoneID(),
			typename.EnumToString(ERBLFactionKey, previousOwner),
			typename.EnumToString(ERBLFactionKey, newOwner)
		);
	}
	
	// ========================================================================
	// DEBUG
	// ========================================================================
	
	void PrintZoneStatus()
	{
		PrintFormat("[RBL_ZoneManager] === ZONE STATUS ===");
		PrintFormat("Total zones: %1", m_aAllZones.Count());
		PrintFormat("FIA zones: %1", GetZoneCountByFaction(ERBLFactionKey.FIA));
		PrintFormat("USSR zones: %1", GetZoneCountByFaction(ERBLFactionKey.USSR));
		PrintFormat("US zones: %1", GetZoneCountByFaction(ERBLFactionKey.US));
		
		foreach (RBL_CampaignZone zone : m_aAllZones)
		{
			PrintFormat("  %1 [%2] - Owner: %3, Support: %4%, Garrison: %5",
				zone.GetZoneID(),
				typename.EnumToString(ERBLZoneType, zone.GetZoneType()),
				typename.EnumToString(ERBLFactionKey, zone.GetOwnerFaction()),
				zone.GetCivilianSupport(),
				zone.GetCurrentGarrison()
			);
		}
	}
}

