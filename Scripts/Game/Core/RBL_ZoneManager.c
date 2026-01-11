// ============================================================================
// PROJECT REBELLION - Zone Manager
// ============================================================================

class RBL_ZoneManager
{
	protected static ref RBL_ZoneManager s_Instance;
	
	protected ref array<RBL_CampaignZone> m_aAllZones;
	protected ref array<ref RBL_VirtualZone> m_aVirtualZones;
	protected ref map<string, RBL_CampaignZone> m_mZonesByID;
	protected ref map<string, ref RBL_VirtualZone> m_mVirtualZonesByID;
	
	protected float m_fTimeSinceSimulation;
	protected const float SIMULATION_INTERVAL = 5.0;
	
	static RBL_ZoneManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_ZoneManager();
		return s_Instance;
	}
	
	void RBL_ZoneManager()
	{
		m_aAllZones = new array<RBL_CampaignZone>();
		m_aVirtualZones = new array<ref RBL_VirtualZone>();
		m_mZonesByID = new map<string, RBL_CampaignZone>();
		m_mVirtualZonesByID = new map<string, ref RBL_VirtualZone>();
		m_fTimeSinceSimulation = 0;
	}
	
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
		
		PrintFormat("[RBL_ZoneManager] Registered zone: %1", zoneID);
	}
	
	void RegisterVirtualZone(RBL_VirtualZone zone)
	{
		if (!zone)
			return;
		
		string zoneID = zone.GetZoneID();
		
		if (m_mVirtualZonesByID.Contains(zoneID))
		{
			PrintFormat("[RBL_ZoneManager] Warning: Virtual zone %1 already registered", zoneID);
			return;
		}
		
		m_aVirtualZones.Insert(zone);
		m_mVirtualZonesByID.Set(zoneID, zone);
	}
	
	void UnregisterZone(RBL_CampaignZone zone)
	{
		if (!zone)
			return;
		
		string zoneID = zone.GetZoneID();
		
		m_aAllZones.RemoveItem(zone);
		m_mZonesByID.Remove(zoneID);
	}
	
	void Update(float timeSlice)
	{
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			m_aAllZones[i].UpdateCaptureState(timeSlice);
		}
		
		m_fTimeSinceSimulation += timeSlice;
		if (m_fTimeSinceSimulation >= SIMULATION_INTERVAL)
		{
			m_fTimeSinceSimulation = 0;
		}
	}
	
	RBL_CampaignZone GetZoneByID(string zoneID)
	{
		RBL_CampaignZone zone;
		m_mZonesByID.Find(zoneID, zone);
		return zone;
	}
	
	RBL_VirtualZone GetVirtualZoneByID(string zoneID)
	{
		RBL_VirtualZone zone;
		m_mVirtualZonesByID.Find(zoneID, zone);
		return zone;
	}
	
	array<RBL_CampaignZone> GetAllZones() { return m_aAllZones; }
	array<ref RBL_VirtualZone> GetAllVirtualZones() { return m_aVirtualZones; }
	
	int GetTotalZoneCount()
	{
		return m_aAllZones.Count() + m_aVirtualZones.Count();
	}
	
	array<RBL_CampaignZone> GetZonesByFaction(ERBLFactionKey faction)
	{
		array<RBL_CampaignZone> result = new array<RBL_CampaignZone>();
		
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			if (m_aAllZones[i].GetOwnerFaction() == faction)
				result.Insert(m_aAllZones[i]);
		}
		
		return result;
	}
	
	array<ref RBL_VirtualZone> GetVirtualZonesByFaction(ERBLFactionKey faction)
	{
		array<ref RBL_VirtualZone> result = new array<ref RBL_VirtualZone>();
		
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			if (m_aVirtualZones[i].GetOwnerFaction() == faction)
				result.Insert(m_aVirtualZones[i]);
		}
		
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
	
	RBL_VirtualZone GetNearestVirtualZone(vector position)
	{
		RBL_VirtualZone nearest = null;
		float nearestDist = 999999.0;
		
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = m_aVirtualZones[i];
			float dist = vector.Distance(position, zone.GetZonePosition());
			if (dist < nearestDist)
			{
				nearestDist = dist;
				nearest = zone;
			}
		}
		
		return nearest;
	}
	
	int GetZoneCountByFaction(ERBLFactionKey faction)
	{
		int count = 0;
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			if (m_aAllZones[i].GetOwnerFaction() == faction)
				count++;
		}
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			if (m_aVirtualZones[i].GetOwnerFaction() == faction)
				count++;
		}
		return count;
	}
	
	bool AreAllZonesCapturedByFaction(ERBLFactionKey faction)
	{
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			if (m_aAllZones[i].GetOwnerFaction() != faction)
				return false;
		}
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			if (m_aVirtualZones[i].GetOwnerFaction() != faction)
				return false;
		}
		return true;
	}
	
	void SpawnAllGarrisons()
	{
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			RBL_CampaignZone zone = m_aAllZones[i];
			if (zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				zone.SpawnGarrison();
		}
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = m_aVirtualZones[i];
			if (zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				zone.SpawnGarrison();
		}
	}
	
	void PrintZoneStatus()
	{
		PrintFormat("[RBL_ZoneManager] === ZONE STATUS ===");
		PrintFormat("Entity zones: %1", m_aAllZones.Count());
		PrintFormat("Virtual zones: %1", m_aVirtualZones.Count());
		PrintFormat("FIA zones: %1", GetZoneCountByFaction(ERBLFactionKey.FIA));
		PrintFormat("USSR zones: %1", GetZoneCountByFaction(ERBLFactionKey.USSR));
		PrintFormat("US zones: %1", GetZoneCountByFaction(ERBLFactionKey.US));
	}
}
