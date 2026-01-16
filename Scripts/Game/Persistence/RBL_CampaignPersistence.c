// ============================================================================
// PROJECT REBELLION - Campaign Persistence
// Handles campaign state save/restore
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
	// COLLECTION
	// ========================================================================
	
	static void CollectCampaignState(RBL_CampaignSaveData outData)
	{
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (!campaignMgr)
			return;
		
		outData.m_iWarLevel = campaignMgr.GetWarLevel();
		outData.m_iAggression = campaignMgr.GetAggression();
		outData.m_fTotalPlayTime = campaignMgr.GetTotalPlayTime();
		outData.m_sLastPlayDate = GetCurrentDateString();
		
		PrintFormat("[RBL_CampaignPersistence] Collected campaign state: WL%1", outData.m_iWarLevel);
	}
	
	static void CollectCommanderState(RBL_CommanderSaveData outData)
	{
		RBL_CommanderAI commander = RBL_CommanderAI.GetInstance();
		if (!commander)
			return;
		
		outData.m_iCommanderResources = commander.GetFactionResources();
		PrintFormat("[RBL_CampaignPersistence] Collected commander state");
	}
	
	RBL_CampaignSaveData CollectCampaignState()
	{
		RBL_CampaignSaveData data = new RBL_CampaignSaveData();
		CollectCampaignState(data);
		return data;
	}
	
	RBL_CommanderSaveData CollectCommanderState()
	{
		RBL_CommanderSaveData data = new RBL_CommanderSaveData();
		CollectCommanderState(data);
		return data;
	}
	
	array<ref RBL_MissionSaveData> CollectActiveMissions()
	{
		array<ref RBL_MissionSaveData> missions = new array<ref RBL_MissionSaveData>();
		
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (!missionMgr)
			return missions;
		
		array<ref RBL_Mission> active = missionMgr.GetActiveMissions();
		if (!active)
			return missions;
		
		for (int i = 0; i < active.Count(); i++)
		{
			RBL_Mission mission = active[i];
			if (!mission)
				continue;
			
			RBL_MissionSaveData data = new RBL_MissionSaveData();
			data.m_sMissionID = mission.GetID();
			data.m_sMissionName = mission.GetName();
			data.m_sMissionType = mission.GetTypeString();
			data.m_sTargetZone = mission.GetTargetZoneID();
			data.m_vTargetPosition = mission.GetMissionArea();
			data.m_fProgress = mission.GetOverallProgress();
			data.m_iObjectivesCompleted = mission.GetCompletedObjectiveCount();
			data.m_iTotalObjectives = mission.GetObjectiveCount();
			data.m_fTimeLimit = mission.GetTimeLimit();
			data.m_fTimeRemaining = mission.GetTimeRemaining();
			data.m_fStartTime = mission.GetTimeElapsed();
			
			RBL_MissionReward reward = mission.GetReward();
			if (reward)
			{
				data.m_iMoneyReward = reward.GetMoney();
				data.m_iHRReward = reward.GetHR();
				data.m_aItemRewards = reward.GetUnlockItems();
			}
			
			data.m_bIsActive = mission.IsActive();
			data.m_bIsCompleted = mission.IsCompleted();
			data.m_bIsFailed = mission.IsFailed();
			
			missions.Insert(data);
		}
		
		return missions;
	}
	
	// ========================================================================
	// RESTORE
	// ========================================================================
	
	static bool RestoreCampaignState(RBL_CampaignSaveData data)
	{
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (!campaignMgr)
			return false;
		
		campaignMgr.SetWarLevel(data.m_iWarLevel);
		campaignMgr.SetAggression(data.m_iAggression);
		
		PrintFormat("[RBL_CampaignPersistence] Restored campaign state: WL%1", data.m_iWarLevel);
		return true;
	}
	
	static bool RestoreCommanderState(RBL_CommanderSaveData data)
	{
		RBL_CommanderAI commander = RBL_CommanderAI.GetInstance();
		if (!commander)
			return false;
		
		PrintFormat("[RBL_CampaignPersistence] Restored commander state");
		return true;
	}
	
	int RestoreActiveMissions(array<ref RBL_MissionSaveData> missions)
	{
		if (!missions || missions.Count() == 0)
			return 0;
		
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (!missionMgr)
		{
			PrintFormat("[RBL_CampaignPersistence] Mission Manager not available for restoration");
			return 0;
		}
		
		int restoredCount = 0;
		
		for (int i = 0; i < missions.Count(); i++)
		{
			RBL_MissionSaveData saveData = missions[i];
			if (!saveData)
				continue;
			
			// Only restore active missions
			if (!saveData.m_bIsActive || saveData.m_bIsCompleted || saveData.m_bIsFailed)
				continue;
			
			// Create mission from save data
			RBL_Mission mission = RestoreMissionFromSaveData(saveData);
			if (mission)
			{
				// Add to active missions
				missionMgr.RestoreActiveMission(mission);
				restoredCount++;
				PrintFormat("[RBL_CampaignPersistence] Restored mission: %1", mission.GetName());
			}
		}
		
		PrintFormat("[RBL_CampaignPersistence] Restored %1/%2 active missions", restoredCount, missions.Count());
		return restoredCount;
	}
	
	protected static RBL_Mission RestoreMissionFromSaveData(RBL_MissionSaveData saveData)
	{
		if (!saveData)
			return null;
		
		// Create mission object
		RBL_Mission mission = new RBL_Mission();
		
		// Restore basic info
		mission.SetID(saveData.m_sMissionID);
		mission.SetName(saveData.m_sMissionName);
		
		// Restore mission type from string
		ERBLMissionType missionType = ParseMissionType(saveData.m_sMissionType);
		mission.SetType(missionType);
		
		// Restore target zone
		if (saveData.m_sTargetZone.Length() > 0)
			mission.SetTargetZone(saveData.m_sTargetZone);
		
		// Restore mission area
		if (saveData.m_vTargetPosition != vector.Zero)
			mission.SetMissionArea(saveData.m_vTargetPosition, 500.0);
		
		// Restore time limit
		if (saveData.m_fTimeLimit > 0)
		{
			mission.SetTimeLimit(saveData.m_fTimeLimit);
			// Restore remaining time
			mission.SetTimeRemaining(saveData.m_fTimeRemaining);
		}
		
		// Restore reward
		if (saveData.m_iMoneyReward > 0 || saveData.m_iHRReward > 0)
		{
			RBL_MissionReward reward = RBL_MissionReward.Create(
				saveData.m_iMoneyReward,
				saveData.m_iHRReward
			);
			mission.SetReward(reward);
		}
		
		// Restore objectives based on mission type
		RestoreMissionObjectives(mission, saveData);
		
		// Start the mission (it was active when saved)
		mission.Start();
		
		// Restore progress
		mission.SetTimeElapsed(saveData.m_fStartTime);
		
		return mission;
	}
	
	protected static ERBLMissionType ParseMissionType(string typeString)
	{
		if (typeString == "Attack") return ERBLMissionType.ATTACK;
		if (typeString == "Defend") return ERBLMissionType.DEFEND;
		if (typeString == "Patrol") return ERBLMissionType.PATROL;
		if (typeString == "Ambush") return ERBLMissionType.AMBUSH;
		if (typeString == "Sabotage") return ERBLMissionType.SABOTAGE;
		if (typeString == "Rescue") return ERBLMissionType.RESCUE;
		if (typeString == "Assassination") return ERBLMissionType.ASSASSINATION;
		if (typeString == "Intelligence") return ERBLMissionType.INTEL;
		return ERBLMissionType.PATROL;
	}
	
	protected static void RestoreMissionObjectives(RBL_Mission mission, RBL_MissionSaveData saveData)
	{
		if (!mission || !saveData)
			return;
		
		// Restore objectives based on mission type and progress
		ERBLMissionType type = mission.GetType();
		int completed = saveData.m_iObjectivesCompleted;
		int total = saveData.m_iTotalObjectives;
		
		// Create objectives based on mission type
		switch (type)
		{
			case ERBLMissionType.ATTACK:
			{
				if (saveData.m_sTargetZone.Length() > 0)
				{
					RBL_MissionObjective obj = RBL_MissionObjective.CreateCaptureZone(
						saveData.m_sTargetZone,
						"Capture " + saveData.m_sTargetZone
					);
					if (completed > 0)
						obj.SetCompleted(true);
					mission.AddObjective(obj);
				}
				break;
			}
			case ERBLMissionType.DEFEND:
			{
				if (saveData.m_sTargetZone.Length() > 0)
				{
					RBL_MissionObjective obj = RBL_MissionObjective.CreateDefendZone(
						saveData.m_sTargetZone,
						saveData.m_fTimeLimit,
						"Defend " + saveData.m_sTargetZone
					);
					if (completed > 0)
						obj.SetCompleted(true);
					mission.AddObjective(obj);
				}
				break;
			}
			case ERBLMissionType.PATROL:
			{
				if (saveData.m_vTargetPosition != vector.Zero)
				{
					RBL_MissionObjective obj = RBL_MissionObjective.CreateReachLocation(
						saveData.m_vTargetPosition,
						"Reach patrol location"
					);
					if (completed > 0)
						obj.SetCompleted(true);
					mission.AddObjective(obj);
				}
				break;
			}
		}
	}
	
	protected static string GetCurrentDateString()
	{
		int year, month, day;
		System.GetYearMonthDay(year, month, day);
		return string.Format("%1-%2-%3", year, month, day);
	}
}
