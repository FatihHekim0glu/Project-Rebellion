// ============================================================================
// PROJECT REBELLION - Zone Persistence
// Saves and restores zone states
// ============================================================================

class RBL_ZonePersistence
{
	protected static ref RBL_ZonePersistence s_Instance;
	
	static RBL_ZonePersistence GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_ZonePersistence();
		return s_Instance;
	}
	
	// ========================================================================
	// COLLECT ZONE DATA
	// ========================================================================
	
	// Collect all zone states into save data array
	array<ref RBL_ZoneSaveData> CollectAllZoneStates()
	{
		array<ref RBL_ZoneSaveData> zoneStates = new array<ref RBL_ZoneSaveData>();
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
		{
			PrintFormat("[RBL_ZonePersistence] Zone manager not available");
			return zoneStates;
		}
		
		// Get all zone IDs
		array<string> zoneIDs = new array<string>();
		zoneMgr.GetAllZoneIDs(zoneIDs);
		
		// Collect each zone's state
		for (int i = 0; i < zoneIDs.Count(); i++)
		{
			RBL_CampaignZone zone = zoneMgr.GetZone(zoneIDs[i]);
			if (!zone)
				continue;
			
			RBL_ZoneSaveData zoneData = CollectZoneState(zone);
			if (zoneData)
				zoneStates.Insert(zoneData);
		}
		
		PrintFormat("[RBL_ZonePersistence] Collected %1 zone states", zoneStates.Count());
		return zoneStates;
	}
	
	// Collect single zone state
	RBL_ZoneSaveData CollectZoneState(RBL_CampaignZone zone)
	{
		if (!zone)
			return null;
		
		RBL_ZoneSaveData data = new RBL_ZoneSaveData();
		
		// Identity
		data.m_sZoneID = zone.GetZoneID();
		data.m_sZoneName = zone.GetZoneName();
		data.m_iZoneType = zone.GetZoneType();
		data.m_vPosition = zone.GetOrigin();
		
		// Ownership
		data.m_iOwnerFaction = zone.GetOwnerFaction();
		data.m_iPreviousOwner = zone.GetPreviousOwner();
		
		// Garrison
		data.m_iGarrisonStrength = zone.GetGarrisonStrength();
		data.m_iMaxGarrison = zone.GetMaxGarrison();
		CollectGarrisonUnitTypes(zone, data.m_aGarrisonUnitTypes);
		
		// Support
		data.m_iSupportLevel = zone.GetSupportLevel();
		data.m_iSupportTrend = zone.GetSupportTrend();
		
		// Capture
		data.m_fCaptureProgress = zone.GetCaptureProgress();
		data.m_iCapturingFaction = zone.GetCapturingFaction();
		
		// Resources
		data.m_iResourceOutput = zone.GetResourceOutput();
		data.m_fResourceTimer = zone.GetResourceTimer();
		
		// Status
		data.m_bIsUnderAttack = zone.IsUnderAttack();
		data.m_bIsContested = zone.IsContested();
		data.m_bIsDestroyed = zone.IsDestroyed();
		
		// Timestamps
		data.m_fLastCaptureTime = zone.GetLastCaptureTime();
		data.m_fLastAttackTime = zone.GetLastAttackTime();
		
		return data;
	}
	
	// Collect garrison unit types
	protected void CollectGarrisonUnitTypes(RBL_CampaignZone zone, array<string> outTypes)
	{
		outTypes.Clear();
		
		// Get garrison info from zone
		array<string> unitTypes = zone.GetGarrisonUnitTypes();
		if (unitTypes)
		{
			for (int i = 0; i < unitTypes.Count(); i++)
			{
				outTypes.Insert(unitTypes[i]);
			}
		}
	}
	
	// ========================================================================
	// RESTORE ZONE DATA
	// ========================================================================
	
	// Restore all zones from save data
	int RestoreAllZoneStates(array<ref RBL_ZoneSaveData> zoneStates)
	{
		if (!zoneStates || zoneStates.IsEmpty())
		{
			PrintFormat("[RBL_ZonePersistence] No zone states to restore");
			return 0;
		}
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
		{
			PrintFormat("[RBL_ZonePersistence] Zone manager not available for restore");
			return 0;
		}
		
		int restoredCount = 0;
		int failedCount = 0;
		
		for (int i = 0; i < zoneStates.Count(); i++)
		{
			RBL_ZoneSaveData zoneData = zoneStates[i];
			if (!zoneData)
				continue;
			
			// Find zone by ID
			RBL_CampaignZone zone = zoneMgr.GetZone(zoneData.m_sZoneID);
			
			if (!zone)
			{
				// Zone doesn't exist - might need to create it
				PrintFormat("[RBL_ZonePersistence] Zone not found for restore: %1", zoneData.m_sZoneID);
				
				// Try to create the zone if position is valid
				if (zoneData.m_vPosition != vector.Zero)
				{
					zone = CreateZoneFromData(zoneData);
					if (zone)
						zoneMgr.RegisterZone(zone);
				}
			}
			
			if (zone)
			{
				if (RestoreZoneState(zone, zoneData))
					restoredCount++;
				else
					failedCount++;
			}
			else
			{
				failedCount++;
			}
		}
		
		PrintFormat("[RBL_ZonePersistence] Restored %1 zones (%2 failed)", restoredCount, failedCount);
		return restoredCount;
	}
	
	// Restore single zone state
	bool RestoreZoneState(RBL_CampaignZone zone, RBL_ZoneSaveData data)
	{
		if (!zone || !data)
			return false;
		
		// Validate zone ID matches
		if (zone.GetZoneID() != data.m_sZoneID)
		{
			PrintFormat("[RBL_ZonePersistence] Zone ID mismatch: %1 vs %2", zone.GetZoneID(), data.m_sZoneID);
			return false;
		}
		
		// Restore ownership
		zone.SetOwnerFaction(data.m_iOwnerFaction);
		zone.SetPreviousOwner(data.m_iPreviousOwner);
		
		// Restore garrison
		zone.SetGarrisonStrength(data.m_iGarrisonStrength);
		zone.SetMaxGarrison(data.m_iMaxGarrison);
		RestoreGarrisonUnitTypes(zone, data.m_aGarrisonUnitTypes);
		
		// Restore support
		zone.SetSupportLevel(data.m_iSupportLevel);
		zone.SetSupportTrend(data.m_iSupportTrend);
		
		// Restore capture state
		zone.SetCaptureProgress(data.m_fCaptureProgress);
		zone.SetCapturingFaction(data.m_iCapturingFaction);
		
		// Restore resources
		zone.SetResourceOutput(data.m_iResourceOutput);
		zone.SetResourceTimer(data.m_fResourceTimer);
		
		// Restore status flags
		zone.SetUnderAttack(data.m_bIsUnderAttack);
		zone.SetContested(data.m_bIsContested);
		zone.SetDestroyed(data.m_bIsDestroyed);
		
		// Restore timestamps
		zone.SetLastCaptureTime(data.m_fLastCaptureTime);
		zone.SetLastAttackTime(data.m_fLastAttackTime);
		
		return true;
	}
	
	// Restore garrison unit types
	protected void RestoreGarrisonUnitTypes(RBL_CampaignZone zone, array<string> unitTypes)
	{
		if (!zone || !unitTypes)
			return;
		
		zone.ClearGarrisonUnitTypes();
		
		for (int i = 0; i < unitTypes.Count(); i++)
		{
			zone.AddGarrisonUnitType(unitTypes[i]);
		}
	}
	
	// Create zone from save data (for zones that don't exist)
	protected RBL_CampaignZone CreateZoneFromData(RBL_ZoneSaveData data)
	{
		// This would normally spawn a zone entity
		// For now, return null - zones should be pre-existing
		PrintFormat("[RBL_ZonePersistence] Cannot create zone from data: %1", data.m_sZoneID);
		return null;
	}
	
	// ========================================================================
	// VALIDATION
	// ========================================================================
	
	// Validate zone save data
	bool ValidateZoneData(RBL_ZoneSaveData data)
	{
		if (!data)
			return false;
		
		// Must have ID
		if (data.m_sZoneID.IsEmpty())
			return false;
		
		// Valid faction
		if (data.m_iOwnerFaction < 0 || data.m_iOwnerFaction > 2)
			return false;
		
		// Valid garrison
		if (data.m_iGarrisonStrength < 0)
			return false;
		
		// Valid support
		if (data.m_iSupportLevel < 0 || data.m_iSupportLevel > 100)
			return false;
		
		// Valid capture progress
		if (data.m_fCaptureProgress < 0 || data.m_fCaptureProgress > 1)
			return false;
		
		return true;
	}
	
	// Validate all zone data
	int ValidateAllZoneData(array<ref RBL_ZoneSaveData> zoneStates)
	{
		if (!zoneStates)
			return 0;
		
		int validCount = 0;
		
		for (int i = 0; i < zoneStates.Count(); i++)
		{
			if (ValidateZoneData(zoneStates[i]))
				validCount++;
		}
		
		return validCount;
	}
	
	// ========================================================================
	// DIFFERENCE DETECTION
	// ========================================================================
	
	// Compare saved and current zone state
	bool HasZoneChanged(RBL_CampaignZone zone, RBL_ZoneSaveData savedData)
	{
		if (!zone || !savedData)
			return true;
		
		// Check key fields for changes
		if (zone.GetOwnerFaction() != savedData.m_iOwnerFaction)
			return true;
		
		if (zone.GetGarrisonStrength() != savedData.m_iGarrisonStrength)
			return true;
		
		if (zone.GetSupportLevel() != savedData.m_iSupportLevel)
			return true;
		
		if (Math.AbsFloat(zone.GetCaptureProgress() - savedData.m_fCaptureProgress) > 0.01)
			return true;
		
		return false;
	}
	
	// Get list of changed zones
	array<string> GetChangedZones(array<ref RBL_ZoneSaveData> savedStates)
	{
		array<string> changedZones = new array<string>();
		
		if (!savedStates)
			return changedZones;
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return changedZones;
		
		for (int i = 0; i < savedStates.Count(); i++)
		{
			RBL_ZoneSaveData savedData = savedStates[i];
			if (!savedData)
				continue;
			
			RBL_CampaignZone zone = zoneMgr.GetZone(savedData.m_sZoneID);
			if (!zone || HasZoneChanged(zone, savedData))
			{
				changedZones.Insert(savedData.m_sZoneID);
			}
		}
		
		return changedZones;
	}
	
	// ========================================================================
	// STATISTICS
	// ========================================================================
	
	// Count zones by faction from save data
	int CountZonesByFaction(array<ref RBL_ZoneSaveData> zoneStates, int factionKey)
	{
		if (!zoneStates)
			return 0;
		
		int count = 0;
		for (int i = 0; i < zoneStates.Count(); i++)
		{
			if (zoneStates[i] && zoneStates[i].m_iOwnerFaction == factionKey)
				count++;
		}
		
		return count;
	}
	
	// Get total garrison strength from save data
	int GetTotalGarrisonStrength(array<ref RBL_ZoneSaveData> zoneStates)
	{
		if (!zoneStates)
			return 0;
		
		int total = 0;
		for (int i = 0; i < zoneStates.Count(); i++)
		{
			if (zoneStates[i])
				total += zoneStates[i].m_iGarrisonStrength;
		}
		
		return total;
	}
	
	// Get average support level from save data
	float GetAverageSupportLevel(array<ref RBL_ZoneSaveData> zoneStates, int factionKey)
	{
		if (!zoneStates || zoneStates.IsEmpty())
			return 0;
		
		float total = 0;
		int count = 0;
		
		for (int i = 0; i < zoneStates.Count(); i++)
		{
			if (zoneStates[i] && zoneStates[i].m_iOwnerFaction == factionKey)
			{
				total += zoneStates[i].m_iSupportLevel;
				count++;
			}
		}
		
		if (count == 0)
			return 0;
		
		return total / count;
	}
}

