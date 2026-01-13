// ============================================================================
// PROJECT REBELLION - Campaign Persistence
// Saves and restores campaign state and commander AI
// ============================================================================

class RBL_CampaignPersistence
{
	protected static ref RBL_CampaignPersistence s_Instance;
	
	static RBL_CampaignPersistence GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_CampaignPersistence();
		return s_Instance;
	}
	
	// ========================================================================
	// COLLECT CAMPAIGN DATA
	// ========================================================================
	
	// Collect full campaign state
	RBL_CampaignSaveData CollectCampaignState()
	{
		RBL_CampaignSaveData data = new RBL_CampaignSaveData();
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (!campaignMgr)
		{
			PrintFormat("[RBL_CampaignPersistence] Campaign manager not available");
			return data;
		}
		
		// Core stats
		data.m_iWarLevel = campaignMgr.GetWarLevel();
		data.m_iAggression = campaignMgr.GetAggression();
		data.m_iCivilianSupport = campaignMgr.GetCivilianSupport();
		
		// Time tracking
		data.m_fTotalPlayTime = campaignMgr.GetTotalPlayTime();
		data.m_iGameDays = campaignMgr.GetGameDays();
		data.m_fCurrentDayTime = campaignMgr.GetCurrentDayTime();
		
		// Alert state
		data.m_iAlertState = campaignMgr.GetAlertState();
		data.m_fAlertDecayTimer = campaignMgr.GetAlertDecayTimer();
		
		// Victory conditions / stats
		data.m_iZonesCaptured = campaignMgr.GetZonesCapturedCount();
		data.m_iZonesLost = campaignMgr.GetZonesLostCount();
		data.m_iEnemiesKilled = campaignMgr.GetEnemiesKilledCount();
		data.m_iFriendliesLost = campaignMgr.GetFriendliesLostCount();
		
		// Timestamps
		data.m_sStartDate = campaignMgr.GetCampaignStartDate();
		data.m_sLastPlayDate = GetCurrentDateString();
		
		PrintFormat("[RBL_CampaignPersistence] Collected campaign state: WL%1, %2 aggression",
			data.m_iWarLevel, data.m_iAggression);
		
		return data;
	}
	
	// Collect commander AI state
	RBL_CommanderSaveData CollectCommanderState()
	{
		RBL_CommanderSaveData data = new RBL_CommanderSaveData();
		
		RBL_CommanderAI commander = RBL_CommanderAI.GetInstance();
		if (!commander)
		{
			PrintFormat("[RBL_CampaignPersistence] Commander AI not available");
			return data;
		}
		
		// Resources
		data.m_iCommanderResources = commander.GetResources();
		data.m_iReinforcementPool = commander.GetReinforcementPool();
		
		// Threat assessment
		data.m_iThreatLevel = commander.GetThreatLevel();
		data.m_sHighestThreatZone = commander.GetHighestThreatZone();
		
		// Active operations
		CollectActiveOperations(commander, data.m_aActiveOperations);
		
		// Cooldowns
		data.m_fQRFCooldown = commander.GetQRFCooldown();
		data.m_fPatrolCooldown = commander.GetPatrolCooldown();
		data.m_fOffensiveCooldown = commander.GetOffensiveCooldown();
		
		// Memory
		CollectCommanderMemory(commander, data);
		
		PrintFormat("[RBL_CampaignPersistence] Collected commander state: %1 resources, %2 operations",
			data.m_iCommanderResources, data.m_aActiveOperations.Count());
		
		return data;
	}
	
	// Collect active operations
	protected void CollectActiveOperations(RBL_CommanderAI commander, array<ref RBL_OperationSave> outOperations)
	{
		outOperations.Clear();
		
		array<ref RBL_Operation> operations = commander.GetActiveOperations();
		if (!operations)
			return;
		
		for (int i = 0; i < operations.Count(); i++)
		{
			RBL_Operation op = operations[i];
			if (!op)
				continue;
			
			RBL_OperationSave save = new RBL_OperationSave();
			save.m_sOperationID = op.GetOperationID();
			save.m_iOperationType = op.GetOperationType();
			save.m_sTargetZone = op.GetTargetZone();
			save.m_iAssignedUnits = op.GetAssignedUnits();
			save.m_fStartTime = op.GetStartTime();
			save.m_fProgress = op.GetProgress();
			
			outOperations.Insert(save);
		}
	}
	
	// Collect commander memory
	protected void CollectCommanderMemory(RBL_CommanderAI commander, RBL_CommanderSaveData data)
	{
		// Known player positions
		data.m_aKnownPlayerPositions.Clear();
		array<string> knownPositions = commander.GetKnownPlayerPositions();
		if (knownPositions)
		{
			for (int i = 0; i < knownPositions.Count(); i++)
			{
				data.m_aKnownPlayerPositions.Insert(knownPositions[i]);
			}
		}
		
		// Recently attacked zones
		data.m_aRecentlyAttackedZones.Clear();
		array<string> attackedZones = commander.GetRecentlyAttackedZones();
		if (attackedZones)
		{
			for (int i = 0; i < attackedZones.Count(); i++)
			{
				data.m_aRecentlyAttackedZones.Insert(attackedZones[i]);
			}
		}
	}
	
	// Collect active missions
	array<ref RBL_MissionSaveData> CollectActiveMissions()
	{
		array<ref RBL_MissionSaveData> missions = new array<ref RBL_MissionSaveData>();
		
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (!missionMgr)
			return missions;
		
		array<ref RBL_Mission> activeMissions = missionMgr.GetActiveMissions();
		if (!activeMissions)
			return missions;
		
		for (int i = 0; i < activeMissions.Count(); i++)
		{
			RBL_Mission mission = activeMissions[i];
			if (!mission)
				continue;
			
			RBL_MissionSaveData save = CollectMissionState(mission);
			if (save)
				missions.Insert(save);
		}
		
		PrintFormat("[RBL_CampaignPersistence] Collected %1 active missions", missions.Count());
		return missions;
	}
	
	// Collect single mission state
	protected RBL_MissionSaveData CollectMissionState(RBL_Mission mission)
	{
		if (!mission)
			return null;
		
		RBL_MissionSaveData data = new RBL_MissionSaveData();
		
		data.m_sMissionID = mission.GetMissionID();
		data.m_sMissionType = mission.GetMissionType();
		data.m_sMissionName = mission.GetMissionName();
		data.m_sTargetZone = mission.GetTargetZone();
		data.m_vTargetPosition = mission.GetTargetPosition();
		data.m_fProgress = mission.GetProgress();
		data.m_iObjectivesCompleted = mission.GetObjectivesCompleted();
		data.m_iTotalObjectives = mission.GetTotalObjectives();
		data.m_fTimeLimit = mission.GetTimeLimit();
		data.m_fTimeRemaining = mission.GetTimeRemaining();
		data.m_fStartTime = mission.GetStartTime();
		data.m_iMoneyReward = mission.GetMoneyReward();
		data.m_iHRReward = mission.GetHRReward();
		
		array<string> itemRewards = mission.GetItemRewards();
		if (itemRewards)
		{
			for (int i = 0; i < itemRewards.Count(); i++)
			{
				data.m_aItemRewards.Insert(itemRewards[i]);
			}
		}
		
		data.m_bIsActive = mission.IsActive();
		data.m_bIsCompleted = mission.IsCompleted();
		data.m_bIsFailed = mission.IsFailed();
		
		return data;
	}
	
	// ========================================================================
	// RESTORE CAMPAIGN DATA
	// ========================================================================
	
	// Restore campaign state
	bool RestoreCampaignState(RBL_CampaignSaveData data)
	{
		if (!data)
		{
			PrintFormat("[RBL_CampaignPersistence] Null campaign data for restore");
			return false;
		}
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (!campaignMgr)
		{
			PrintFormat("[RBL_CampaignPersistence] Campaign manager not available for restore");
			return false;
		}
		
		// Core stats
		campaignMgr.SetWarLevel(data.m_iWarLevel);
		campaignMgr.SetAggression(data.m_iAggression);
		campaignMgr.SetCivilianSupport(data.m_iCivilianSupport);
		
		// Time tracking
		campaignMgr.SetTotalPlayTime(data.m_fTotalPlayTime);
		campaignMgr.SetGameDays(data.m_iGameDays);
		campaignMgr.SetCurrentDayTime(data.m_fCurrentDayTime);
		
		// Alert state
		campaignMgr.SetAlertState(data.m_iAlertState);
		campaignMgr.SetAlertDecayTimer(data.m_fAlertDecayTimer);
		
		// Victory stats
		campaignMgr.SetZonesCapturedCount(data.m_iZonesCaptured);
		campaignMgr.SetZonesLostCount(data.m_iZonesLost);
		campaignMgr.SetEnemiesKilledCount(data.m_iEnemiesKilled);
		campaignMgr.SetFriendliesLostCount(data.m_iFriendliesLost);
		
		// Timestamps
		campaignMgr.SetCampaignStartDate(data.m_sStartDate);
		
		PrintFormat("[RBL_CampaignPersistence] Restored campaign state: WL%1", data.m_iWarLevel);
		return true;
	}
	
	// Restore commander AI state
	bool RestoreCommanderState(RBL_CommanderSaveData data)
	{
		if (!data)
		{
			PrintFormat("[RBL_CampaignPersistence] Null commander data for restore");
			return false;
		}
		
		RBL_CommanderAI commander = RBL_CommanderAI.GetInstance();
		if (!commander)
		{
			PrintFormat("[RBL_CampaignPersistence] Commander AI not available for restore");
			return false;
		}
		
		// Resources
		commander.SetResources(data.m_iCommanderResources);
		commander.SetReinforcementPool(data.m_iReinforcementPool);
		
		// Threat
		commander.SetThreatLevel(data.m_iThreatLevel);
		commander.SetHighestThreatZone(data.m_sHighestThreatZone);
		
		// Operations
		RestoreActiveOperations(commander, data.m_aActiveOperations);
		
		// Cooldowns
		commander.SetQRFCooldown(data.m_fQRFCooldown);
		commander.SetPatrolCooldown(data.m_fPatrolCooldown);
		commander.SetOffensiveCooldown(data.m_fOffensiveCooldown);
		
		// Memory
		RestoreCommanderMemory(commander, data);
		
		PrintFormat("[RBL_CampaignPersistence] Restored commander state");
		return true;
	}
	
	// Restore active operations
	protected void RestoreActiveOperations(RBL_CommanderAI commander, array<ref RBL_OperationSave> operations)
	{
		if (!operations)
			return;
		
		commander.ClearOperations();
		
		for (int i = 0; i < operations.Count(); i++)
		{
			RBL_OperationSave save = operations[i];
			if (!save)
				continue;
			
			commander.RestoreOperation(
				save.m_sOperationID,
				save.m_iOperationType,
				save.m_sTargetZone,
				save.m_iAssignedUnits,
				save.m_fStartTime,
				save.m_fProgress
			);
		}
	}
	
	// Restore commander memory
	protected void RestoreCommanderMemory(RBL_CommanderAI commander, RBL_CommanderSaveData data)
	{
		commander.ClearKnownPositions();
		for (int i = 0; i < data.m_aKnownPlayerPositions.Count(); i++)
		{
			commander.AddKnownPosition(data.m_aKnownPlayerPositions[i]);
		}
		
		commander.ClearRecentAttacks();
		for (int i = 0; i < data.m_aRecentlyAttackedZones.Count(); i++)
		{
			commander.AddRecentAttack(data.m_aRecentlyAttackedZones[i]);
		}
	}
	
	// Restore active missions
	int RestoreActiveMissions(array<ref RBL_MissionSaveData> missions)
	{
		if (!missions || missions.IsEmpty())
			return 0;
		
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (!missionMgr)
			return 0;
		
		missionMgr.ClearAllMissions();
		
		int restoredCount = 0;
		
		for (int i = 0; i < missions.Count(); i++)
		{
			RBL_MissionSaveData missionData = missions[i];
			if (!missionData || !missionData.m_bIsActive)
				continue;
			
			if (RestoreMissionState(missionMgr, missionData))
				restoredCount++;
		}
		
		PrintFormat("[RBL_CampaignPersistence] Restored %1 missions", restoredCount);
		return restoredCount;
	}
	
	// Restore single mission
	protected bool RestoreMissionState(RBL_MissionManager missionMgr, RBL_MissionSaveData data)
	{
		if (!missionMgr || !data)
			return false;
		
		return missionMgr.RestoreMission(
			data.m_sMissionID,
			data.m_sMissionType,
			data.m_sMissionName,
			data.m_sTargetZone,
			data.m_vTargetPosition,
			data.m_fProgress,
			data.m_iObjectivesCompleted,
			data.m_iTotalObjectives,
			data.m_fTimeLimit,
			data.m_fTimeRemaining,
			data.m_fStartTime,
			data.m_iMoneyReward,
			data.m_iHRReward,
			data.m_aItemRewards
		);
	}
	
	// ========================================================================
	// UTILITIES
	// ========================================================================
	
	// Get current date string
	protected string GetCurrentDateString()
	{
		int year, month, day;
		System.GetYearMonthDay(year, month, day);
		return string.Format("%1-%2-%3", year, PadZero(month), PadZero(day));
	}
	
	protected string PadZero(int value)
	{
		if (value < 10)
			return "0" + value.ToString();
		return value.ToString();
	}
	
	// Validate campaign data
	bool ValidateCampaignData(RBL_CampaignSaveData data)
	{
		if (!data)
			return false;
		
		if (data.m_iWarLevel < 1 || data.m_iWarLevel > 10)
			return false;
		
		if (data.m_iAggression < 0 || data.m_iAggression > 100)
			return false;
		
		if (data.m_fTotalPlayTime < 0)
			return false;
		
		return true;
	}
	
	// Validate commander data
	bool ValidateCommanderData(RBL_CommanderSaveData data)
	{
		if (!data)
			return false;
		
		if (data.m_iCommanderResources < 0)
			return false;
		
		if (data.m_iThreatLevel < 0)
			return false;
		
		return true;
	}
}

// ============================================================================
// PLACEHOLDER CLASSES FOR INTERFACES
// ============================================================================

class RBL_Operation
{
	string GetOperationID() { return ""; }
	int GetOperationType() { return 0; }
	string GetTargetZone() { return ""; }
	int GetAssignedUnits() { return 0; }
	float GetStartTime() { return 0; }
	float GetProgress() { return 0; }
}

class RBL_Mission
{
	string GetMissionID() { return ""; }
	string GetMissionType() { return ""; }
	string GetMissionName() { return ""; }
	string GetTargetZone() { return ""; }
	vector GetTargetPosition() { return vector.Zero; }
	float GetProgress() { return 0; }
	int GetObjectivesCompleted() { return 0; }
	int GetTotalObjectives() { return 0; }
	float GetTimeLimit() { return 0; }
	float GetTimeRemaining() { return 0; }
	float GetStartTime() { return 0; }
	int GetMoneyReward() { return 0; }
	int GetHRReward() { return 0; }
	array<string> GetItemRewards() { return new array<string>(); }
	bool IsActive() { return false; }
	bool IsCompleted() { return false; }
	bool IsFailed() { return false; }
}

class RBL_MissionManager
{
	protected static ref RBL_MissionManager s_Instance;
	static RBL_MissionManager GetInstance() { if (!s_Instance) s_Instance = new RBL_MissionManager(); return s_Instance; }
	
	array<ref RBL_Mission> GetActiveMissions() { return new array<ref RBL_Mission>(); }
	void ClearAllMissions() {}
	bool RestoreMission(string id, string type, string name, string zone, vector pos, float progress,
		int objCompleted, int totalObj, float timeLimit, float timeRemaining, float startTime,
		int moneyReward, int hrReward, array<string> itemRewards) { return false; }
}

