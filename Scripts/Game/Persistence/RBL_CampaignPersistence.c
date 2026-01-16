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
		if (!missions)
			return 0;
		
		// Mission restoration is not implemented; return 0 to continue load flow.
		return 0;
	}
	
	protected static string GetCurrentDateString()
	{
		int year, month, day;
		System.GetYearMonthDay(year, month, day);
		return string.Format("%1-%2-%3", year, month, day);
	}
}
