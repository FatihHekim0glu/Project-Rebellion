// ============================================================================
// PROJECT REBELLION - Game Mode
// Entry point that initializes all systems when the mission starts
// ============================================================================

class RBL_GameModeClass : SCR_BaseGameModeClass
{
}

class RBL_GameMode : SCR_BaseGameMode
{
	// ========================================================================
	// CONFIGURATION
	// ========================================================================
	
	[Attribute("1", UIWidgets.CheckBox, "Enable Rebellion systems")]
	protected bool m_bEnableRebellion;
	
	[Attribute("1", UIWidgets.Slider, "Starting War Level", "1 10 1")]
	protected int m_iStartingWarLevel;
	
	[Attribute("25", UIWidgets.Slider, "Starting Aggression", "0 100 1")]
	protected int m_iStartingAggression;
	
	// ========================================================================
	// MANAGER REFERENCES
	// ========================================================================
	
	protected RBL_ZoneManager m_ZoneManager;
	protected RBL_EconomyManager m_EconomyManager;
	protected RBL_CommanderAI m_CommanderAI;
	protected RBL_UndercoverSystem m_UndercoverSystem;
	protected RBL_PersistenceManager m_PersistenceManager;
	
	// ========================================================================
	// INITIALIZATION
	// ========================================================================
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!m_bEnableRebellion)
			return;
		
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] PROJECT REBELLION - Initializing...");
		PrintFormat("[RBL] ========================================");
		
		// Initialize singletons (server only)
		if (Replication.IsServer())
		{
			InitializeManagers();
		}
	}
	
	protected void InitializeManagers()
	{
		// Create manager instances
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
		
		// Note: RBL_CampaignManager is a GenericEntity, it will self-register
		// when placed in the world or spawned
		
		PrintFormat("[RBL] Waiting for CampaignManager entity...");
	}
	
	// ========================================================================
	// GAME STATE HOOKS
	// ========================================================================
	
	override void OnGameStateChanged(SCR_EGameModeState state)
	{
		super.OnGameStateChanged(state);
		
		if (!m_bEnableRebellion)
			return;
		
		switch (state)
		{
			case SCR_EGameModeState.GAME:
				OnGameStarted();
				break;
				
			case SCR_EGameModeState.POSTGAME:
				OnGameEnded();
				break;
		}
	}
	
	protected void OnGameStarted()
	{
		PrintFormat("[RBL] Game started - Campaign active");
		
		// Trigger auto-save on game start
		if (m_PersistenceManager)
			m_PersistenceManager.TriggerAutoSave();
	}
	
	protected void OnGameEnded()
	{
		PrintFormat("[RBL] Game ended - Final save");
		
		// Save on game end
		if (m_PersistenceManager)
			m_PersistenceManager.SaveCampaign();
	}
	
	// ========================================================================
	// PLAYER HOOKS
	// ========================================================================
	
	override void OnPlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator killer)
	{
		super.OnPlayerKilled(playerId, playerEntity, killerEntity, killer);
		
		if (!m_bEnableRebellion)
			return;
		
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
			campaignMgr.OnPlayerKilled();
		
		// Reset undercover status on death
		if (m_UndercoverSystem)
			m_UndercoverSystem.ResetPlayerCover(playerId);
	}
	
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);
		
		if (!m_bEnableRebellion)
			return;
		
		PrintFormat("[RBL] Player %1 spawned", playerId);
	}
	
	// ========================================================================
	// CONSOLE COMMANDS (Debug)
	// ========================================================================
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_DebugPrintStatus()
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
	
	// ========================================================================
	// API
	// ========================================================================
	
	RBL_ZoneManager GetZoneManager() { return m_ZoneManager; }
	RBL_EconomyManager GetEconomyManager() { return m_EconomyManager; }
	RBL_CommanderAI GetCommanderAI() { return m_CommanderAI; }
	RBL_UndercoverSystem GetUndercoverSystem() { return m_UndercoverSystem; }
}

