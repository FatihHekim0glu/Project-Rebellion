// ============================================================================
// PROJECT REBELLION - Campaign Persistence
// Handles campaign state save/restore
// ============================================================================

class RBL_CampaignPersistence
{
	static void CollectCampaignState(RBL_CampaignState outData)
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
	
	static void CollectCommanderState(RBL_CommanderState outData)
	{
		RBL_CommanderAI commander = RBL_CommanderAI.GetInstance();
		if (!commander)
			return;
		
		outData.m_iCommanderResources = commander.GetFactionResources();
		PrintFormat("[RBL_CampaignPersistence] Collected commander state");
	}
	
	static bool RestoreCampaignState(RBL_CampaignState data)
	{
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (!campaignMgr)
			return false;
		
		campaignMgr.SetWarLevel(data.m_iWarLevel);
		campaignMgr.SetAggression(data.m_iAggression);
		
		PrintFormat("[RBL_CampaignPersistence] Restored campaign state: WL%1", data.m_iWarLevel);
		return true;
	}
	
	static bool RestoreCommanderState(RBL_CommanderState data)
	{
		RBL_CommanderAI commander = RBL_CommanderAI.GetInstance();
		if (!commander)
			return false;
		
		PrintFormat("[RBL_CampaignPersistence] Restored commander state");
		return true;
	}
	
	protected static string GetCurrentDateString()
	{
		int year, month, day;
		System.GetYearMonthDay(year, month, day);
		return string.Format("%1-%2-%3", year, month, day);
	}
}
