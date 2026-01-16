// ============================================================================
// PROJECT REBELLION - Zone Manager
// With multiplayer support - server authoritative zone changes
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
	
	// Network events
	protected ref ScriptInvoker m_OnZoneOwnershipChanged;
	protected ref ScriptInvoker m_OnZoneSupportChanged;

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
		
		m_OnZoneOwnershipChanged = new ScriptInvoker();
		m_OnZoneSupportChanged = new ScriptInvoker();
	}
	
	// ========================================================================
	// NETWORK HELPERS
	// ========================================================================
	
	protected bool CanModifyState()
	{
		return RBL_NetworkUtils.IsSinglePlayer() || RBL_NetworkUtils.IsServer();
	}
	
	ScriptInvoker GetOnZoneOwnershipChanged() { return m_OnZoneOwnershipChanged; }
	ScriptInvoker GetOnZoneSupportChanged() { return m_OnZoneSupportChanged; }

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
			RBL_CampaignZone zone = m_aAllZones[i];
			if (zone)
				zone.UpdateCaptureState(timeSlice);
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

	RBL_CampaignZone GetZone(string zoneID)
	{
		return GetZoneByID(zoneID);
	}

	RBL_VirtualZone GetVirtualZoneByID(string zoneID)
	{
		RBL_VirtualZone zone;
		m_mVirtualZonesByID.Find(zoneID, zone);
		return zone;
	}

	RBL_VirtualZone GetVirtualZone(string zoneID)
	{
		return GetVirtualZoneByID(zoneID);
	}

	array<RBL_CampaignZone> GetAllZones() { return m_aAllZones; }
	array<ref RBL_VirtualZone> GetAllVirtualZones() { return m_aVirtualZones; }

	void GetAllZoneIDs(out array<string> outIDs)
	{
		if (!outIDs)
			outIDs = new array<string>();
		
		outIDs.Clear();
		
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			if (m_aAllZones[i])
				outIDs.Insert(m_aAllZones[i].GetZoneID());
		}
		
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			if (m_aVirtualZones[i])
				outIDs.Insert(m_aVirtualZones[i].GetZoneID());
		}
	}

	void GetAllVirtualZoneIDs(out array<string> outIDs)
	{
		if (!outIDs)
			outIDs = new array<string>();
		
		outIDs.Clear();
		
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			if (m_aVirtualZones[i])
				outIDs.Insert(m_aVirtualZones[i].GetZoneID());
		}
	}

	int GetTotalZoneCount()
	{
		return m_aAllZones.Count() + m_aVirtualZones.Count();
	}

	void RefreshZoneStates()
	{
		PrintFormat("[RBL_ZoneManager] Refreshing zone states...");
		
		// Re-sync zone states after load by notifying all listeners of current state
		// This ensures zone ownership, support, and garrison are properly synchronized
		
		int entityZonesRefreshed = 0;
		int virtualZonesRefreshed = 0;
		
		// Refresh entity zones
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			RBL_CampaignZone zone = m_aAllZones[i];
			if (zone)
			{
				// Notify listeners of current state to trigger UI updates
				ERBLFactionKey owner = zone.GetOwnerFaction();
				m_OnZoneOwnershipChanged.Invoke(zone.GetZoneID(), owner, owner);
				
				// Notify support changes
				int support = zone.GetSupportLevel();
				m_OnZoneSupportChanged.Invoke(zone.GetZoneID(), support);
				
				entityZonesRefreshed++;
			}
		}
		
		// Refresh virtual zones
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = m_aVirtualZones[i];
			if (zone)
			{
				ERBLFactionKey owner = zone.GetOwnerFaction();
				m_OnZoneOwnershipChanged.Invoke(zone.GetZoneID(), owner, owner);
				
				int support = zone.GetCivilianSupport();
				m_OnZoneSupportChanged.Invoke(zone.GetZoneID(), support);
				
				virtualZonesRefreshed++;
			}
		}
		
		PrintFormat("[RBL_ZoneManager] Zone states refreshed: %1 entity zones, %2 virtual zones", 
			entityZonesRefreshed, virtualZonesRefreshed);
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
			if (zone && zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				zone.SpawnGarrison();
		}
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = m_aVirtualZones[i];
			if (zone && zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				zone.SpawnGarrison();
		}
	}

	// Get player (FIA) HQ zone
	RBL_CampaignZone GetPlayerHQ()
	{
		// Check entity zones
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			RBL_CampaignZone zone = m_aAllZones[i];
			if (zone && zone.GetZoneType() == ERBLZoneType.HQ && zone.GetOwnerFaction() == ERBLFactionKey.FIA)
				return zone;
		}
		return null;
	}
	
	// Get enemy HQ zone
	RBL_CampaignZone GetEnemyHQ()
	{
		// Check entity zones
		for (int i = 0; i < m_aAllZones.Count(); i++)
		{
			RBL_CampaignZone zone = m_aAllZones[i];
			if (zone && zone.GetZoneType() == ERBLZoneType.HQ && zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				return zone;
		}
		return null;
	}
	
	// Get virtual player HQ
	RBL_VirtualZone GetPlayerHQVirtual()
	{
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = m_aVirtualZones[i];
			if (zone && zone.GetZoneType() == ERBLZoneType.HQ && zone.GetOwnerFaction() == ERBLFactionKey.FIA)
				return zone;
		}
		return null;
	}
	
	// Get virtual enemy HQ
	RBL_VirtualZone GetEnemyHQVirtual()
	{
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = m_aVirtualZones[i];
			if (zone && zone.GetZoneType() == ERBLZoneType.HQ && zone.GetOwnerFaction() != ERBLFactionKey.FIA)
				return zone;
		}
		return null;
	}
	
	// Check if FIA still has any HQ
	bool PlayerHasHQ()
	{
		if (GetPlayerHQ())
			return true;
		if (GetPlayerHQVirtual())
			return true;
		return false;
	}
	
	// Check if enemy still has any HQ
	bool EnemyHasHQ()
	{
		if (GetEnemyHQ())
			return true;
		if (GetEnemyHQVirtual())
			return true;
		return false;
	}

	void PrintZoneStatus()
	{
		PrintFormat("[RBL_ZoneManager] === ZONE STATUS ===");
		PrintFormat("Entity zones: %1", m_aAllZones.Count());
		PrintFormat("Virtual zones: %1", m_aVirtualZones.Count());
		PrintFormat("FIA zones: %1", GetZoneCountByFaction(ERBLFactionKey.FIA));
		PrintFormat("USSR zones: %1", GetZoneCountByFaction(ERBLFactionKey.USSR));
		PrintFormat("US zones: %1", GetZoneCountByFaction(ERBLFactionKey.US));
		PrintFormat("Is Server: %1", RBL_NetworkUtils.IsServer());
		PrintFormat("Can Modify: %1", CanModifyState());
	}
	
	// ========================================================================
	// NETWORK: ZONE STATE CHANGES (Server authoritative)
	// ========================================================================
	
	void SetZoneOwner(string zoneID, ERBLFactionKey newOwner)
	{
		if (!CanModifyState())
		{
			PrintFormat("[RBL_ZoneManager] SetZoneOwner blocked - not server");
			return;
		}
		
		RBL_VirtualZone vZone = GetVirtualZoneByID(zoneID);
		if (vZone)
		{
			ERBLFactionKey previousOwner = vZone.GetOwnerFaction();
			vZone.SetOwnerFaction(newOwner);
			
			m_OnZoneOwnershipChanged.Invoke(zoneID, previousOwner, newOwner);
			
			// Broadcast to all clients
			RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
			if (netMgr)
				netMgr.BroadcastZoneCapture(zoneID, previousOwner, newOwner);
			
			return;
		}
		
		RBL_CampaignZone eZone = GetZoneByID(zoneID);
		if (eZone)
		{
			ERBLFactionKey previousOwner = eZone.GetOwnerFaction();
			eZone.SetOwnerFaction(newOwner);
			
			m_OnZoneOwnershipChanged.Invoke(zoneID, previousOwner, newOwner);
			
			RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
			if (netMgr)
				netMgr.BroadcastZoneCapture(zoneID, previousOwner, newOwner);
		}
	}
	
	void SetZoneOwnerLocal(string zoneID, ERBLFactionKey newOwner)
	{
		RBL_VirtualZone vZone = GetVirtualZoneByID(zoneID);
		if (vZone)
		{
			vZone.SetOwnerFaction(newOwner);
			return;
		}
		
		RBL_CampaignZone eZone = GetZoneByID(zoneID);
		if (eZone)
			eZone.SetOwnerFaction(newOwner);
	}
	
	// ========================================================================
	// NETWORK: SERIALIZATION
	// ========================================================================
	
	string SerializeZoneOwnership()
	{
		string result = "";
		
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = m_aVirtualZones[i];
			if (!zone)
				continue;
			
			if (result.Length() > 0)
				result += ";";
			
			result += zone.GetZoneID() + ":" + zone.GetOwnerFaction().ToString();
		}
		
		return result;
	}
	
	void DeserializeZoneOwnership(string data)
	{
		if (data.IsEmpty())
			return;
		
		array<string> entries = new array<string>();
		data.Split(";", entries, false);
		
		for (int i = 0; i < entries.Count(); i++)
		{
			array<string> parts = new array<string>();
			entries[i].Split(":", parts, false);
			
			if (parts.Count() != 2)
				continue;
			
			string zoneID = parts[0];
			int owner = parts[1].ToInt();
			
			SetZoneOwnerLocal(zoneID, owner);
		}
	}
	
	string SerializeZoneSupport()
	{
		string result = "";
		
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = m_aVirtualZones[i];
			if (!zone)
				continue;
			
			if (result.Length() > 0)
				result += ";";
			
			result += zone.GetZoneID() + ":" + zone.GetCivilianSupport().ToString();
		}
		
		return result;
	}
	
	void DeserializeZoneSupport(string data)
	{
		if (data.IsEmpty())
			return;
		
		array<string> entries = new array<string>();
		data.Split(";", entries, false);
		
		for (int i = 0; i < entries.Count(); i++)
		{
			array<string> parts = new array<string>();
			entries[i].Split(":", parts, false);
			
			if (parts.Count() != 2)
				continue;
			
			string zoneID = parts[0];
			int support = parts[1].ToInt();
			
			RBL_VirtualZone zone = GetVirtualZoneByID(zoneID);
			if (zone)
				zone.SetCivilianSupport(support);
		}
	}
	
	string SerializeFullZoneState()
	{
		string result = "";
		
		for (int i = 0; i < m_aVirtualZones.Count(); i++)
		{
			RBL_VirtualZone zone = m_aVirtualZones[i];
			if (!zone)
				continue;
			
			if (result.Length() > 0)
				result += "|";
			
			result += string.Format("%1,%2,%3,%4",
				zone.GetZoneID(),
				zone.GetOwnerFaction(),
				zone.GetCivilianSupport(),
				zone.GetAlertState()
			);
		}
		
		return result;
	}
	
	void DeserializeFullZoneState(string data)
	{
		if (data.IsEmpty())
			return;
		
		array<string> entries = new array<string>();
		data.Split("|", entries, false);
		
		for (int i = 0; i < entries.Count(); i++)
		{
			array<string> parts = new array<string>();
			entries[i].Split(",", parts, false);
			
			if (parts.Count() != 4)
				continue;
			
			string zoneID = parts[0];
			int owner = parts[1].ToInt();
			int support = parts[2].ToInt();
			int alertState = parts[3].ToInt();
			
			RBL_VirtualZone zone = GetVirtualZoneByID(zoneID);
			if (zone)
			{
				zone.SetOwnerFaction(owner);
				zone.SetCivilianSupport(support);
				zone.SetAlertState(alertState);
			}
		}
	}
}
