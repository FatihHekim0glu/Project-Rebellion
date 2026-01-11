// ============================================================================
// PROJECT REBELLION - Game Mode
// ============================================================================

class RBL_GameModeClass : SCR_BaseGameModeClass
{
}

class RBL_GameMode : SCR_BaseGameMode
{
	[Attribute("1", UIWidgets.CheckBox, "Enable Rebellion systems")]
	protected bool m_bEnableRebellion;
	
	[Attribute("1", UIWidgets.Slider, "Starting War Level", "1 10 1")]
	protected int m_iStartingWarLevel;
	
	[Attribute("25", UIWidgets.Slider, "Starting Aggression", "0 100 1")]
	protected int m_iStartingAggression;
	
	protected RBL_ZoneManager m_ZoneManager;
	protected RBL_EconomyManager m_EconomyManager;
	protected RBL_CommanderAI m_CommanderAI;
	protected RBL_UndercoverSystem m_UndercoverSystem;
	protected RBL_PersistenceManager m_PersistenceManager;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!m_bEnableRebellion)
			return;
		
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] PROJECT REBELLION - Initializing...");
		PrintFormat("[RBL] ========================================");
		
		InitializeManagers();
	}
	
	protected void InitializeManagers()
	{
		m_ZoneManager = RBL_ZoneManager.GetInstance();
		m_EconomyManager = RBL_EconomyManager.GetInstance();
		m_CommanderAI = RBL_CommanderAI.GetInstance();
		m_UndercoverSystem = RBL_UndercoverSystem.GetInstance();
		m_PersistenceManager = RBL_PersistenceManager.GetInstance();
		
		PrintFormat("[RBL] Managers initialized:");
		PrintFormat("[RBL]   - ZoneManager: %1", m_ZoneManager != null);
		PrintFormat("[RBL]   - EconomyManager: %1", m_EconomyManager != null);
		PrintFormat("[RBL]   - CommanderAI: %1", m_CommanderAI != null);
		PrintFormat("[RBL]   - UndercoverSystem: %1", m_UndercoverSystem != null);
		PrintFormat("[RBL]   - PersistenceManager: %1", m_PersistenceManager != null);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (!m_bEnableRebellion)
			return;
		
		if (m_CommanderAI)
			m_CommanderAI.Update(timeSlice);
		
		if (m_UndercoverSystem)
			m_UndercoverSystem.Update(timeSlice);
		
		if (m_ZoneManager)
			m_ZoneManager.Update(timeSlice);
		
		if (m_PersistenceManager)
			m_PersistenceManager.Update(timeSlice);
	}
	
	RBL_ZoneManager GetZoneManager() { return m_ZoneManager; }
	RBL_EconomyManager GetEconomyManager() { return m_EconomyManager; }
	RBL_CommanderAI GetCommanderAI() { return m_CommanderAI; }
	RBL_UndercoverSystem GetUndercoverSystem() { return m_UndercoverSystem; }
	
	void DebugPrintStatus()
	{
		if (m_ZoneManager)
			m_ZoneManager.PrintZoneStatus();
		
		if (m_EconomyManager)
			m_EconomyManager.PrintEconomyStatus();
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			PrintFormat("[RBL] War Level: %1", campaignMgr.GetWarLevel());
			PrintFormat("[RBL] Aggression: %1", campaignMgr.GetAggression());
			PrintFormat("[RBL] Day: %1", campaignMgr.GetDayNumber());
		}
	}
}
