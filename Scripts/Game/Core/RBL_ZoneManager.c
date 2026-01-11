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
		
		ERBLFactionKey owner = zone.GetOwnerFaction();
		array<RBL_CampaignZone> factionZones = m_mZonesByFaction.Get(owner);
		if (factionZones)
			factionZones.Insert(zone);
		
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
		
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			m_aAllZones[i].UpdateCaptureState(timeSlice);
		}
		
		m_fTimeSinceSimulation += timeSlice;
		if (m_fTimeSinceSimulation >= SIMULATION_INTERVAL)
		{
			m_fTimeSinceSimulation = 0;
			SimulateDistantZones();
		}
	}
	
	protected void SimulateDistantZones()
	{
		array<vector> playerPositions = GetAllPlayerPositions();
		
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			RBL_CampaignZone zone = m_aAllZones[i];
			bool isNearPlayer = false;
			vector zonePos = zone.GetZonePosition();
			
			for (int j = 0; j < playerPositions.Count(); j++)
			{
				if (vector.Distance(zonePos, playerPositions[j]) < 1000)
				{
					isNearPlayer = true;
					break;
				}
			}
			
			if (!isNearPlayer)
			{
				SimulateZone(zone);
			}
		}
	}
	
	protected void SimulateZone(RBL_CampaignZone zone)
	{
		if (zone.GetCurrentGarrison() < zone.GetMaxGarrison() * 0.5)
		{
			// Placeholder for garrison regeneration
		}
	}
	
	protected array<vector> GetAllPlayerPositions()
	{
		array<vector> positions = new array<vector>();
		
		array<int> playerIDs = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		
		for (int i = 0; i < playerIDs.Count(); i++)
		{
			IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerIDs[i]);
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
		array<RBL_CampaignZone> result = new array<RBL_CampaignZone>();
		
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			if (m_aAllZones[i].GetZoneType() == type)
				result.Insert(m_aAllZones[i]);
		}
		
		return result;
	}
	
	RBL_CampaignZone GetNearestZone(vector position)
	{
		RBL_CampaignZone nearest = null;
		float nearestDist = 999999.0;
		
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			RBL_CampaignZone zone = m_aAllZones[i];
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
		float nearestDist = 999999.0;
		
		array<RBL_CampaignZone> factionZones = GetZonesByFaction(faction);
		
		for (int i = 0; i < factionZones.Count(); i++)
		{
			RBL_CampaignZone zone = factionZones[i];
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
		array<RBL_CampaignZone> result = new array<RBL_CampaignZone>();
		
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			RBL_CampaignZone zone = m_aAllZones[i];
			if (vector.Distance(position, zone.GetZonePosition()) <= radius)
				result.Insert(zone);
		}
		
		return result;
	}
	
	RBL_CampaignZone GetPlayerHQ()
	{
		array<RBL_CampaignZone> fiaZones = GetZonesByFaction(ERBLFactionKey.FIA);
		
		for (int i = 0; i < fiaZones.Count(); i++)
		{
			if (fiaZones[i].GetZoneType() == ERBLZoneType.HQ)
				return fiaZones[i];
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
		
		for (int i = 0; i < factionZones.Count(); i++)
		{
			total += factionZones[i].GetCurrentGarrison();
		}
		
		return total;
	}
	
	float GetAverageSupportByFaction(ERBLFactionKey faction)
	{
		array<RBL_CampaignZone> factionZones = GetZonesByFaction(faction);
		
		if (factionZones.Count() == 0)
			return 0;
		
		int totalSupport = 0;
		for (int i = 0; i < factionZones.Count(); i++)
		{
			totalSupport += factionZones[i].GetCivilianSupport();
		}
		
		return totalSupport / factionZones.Count();
	}
	
	bool AreAllZonesCapturedByFaction(ERBLFactionKey faction)
	{
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			if (m_aAllZones[i].GetOwnerFaction() != faction)
				return false;
		}
		return true;
	}
	
	// ========================================================================
	// GARRISON MANAGEMENT
	// ========================================================================
	
	void SpawnAllGarrisons()
	{
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			RBL_CampaignZone zone = m_aAllZones[i];
			if (zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				zone.SpawnGarrison();
		}
	}
	
	void ClearAllGarrisons()
	{
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			m_aAllZones[i].ClearGarrison();
		}
	}
	
	// ========================================================================
	// EVENT HANDLERS
	// ========================================================================
	
	protected void OnZoneOwnershipChanged(RBL_CampaignZone zone, ERBLFactionKey previousOwner, ERBLFactionKey newOwner)
	{
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
		
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			RBL_CampaignZone zone = m_aAllZones[i];
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
