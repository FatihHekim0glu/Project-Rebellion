// ============================================================================
// PROJECT REBELLION - Game Mode
// Plug-and-play: Just load this mod and play!
// ============================================================================

class RBL_GameModeClass : SCR_BaseGameModeClass
{
}

class RBL_GameMode : SCR_BaseGameMode
{
	[Attribute("1", UIWidgets.CheckBox, "Enable Rebellion systems")]
	protected bool m_bEnableRebellion;
	
	[Attribute("1", UIWidgets.CheckBox, "Auto-initialize zones from config")]
	protected bool m_bAutoInitialize;
	
	protected bool m_bSystemsInitialized;
	protected float m_fInitDelay;
	protected const float INIT_DELAY_TIME = 2.0;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (!m_bEnableRebellion)
			return;
		
		m_bSystemsInitialized = false;
		m_fInitDelay = 0;
		
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] PROJECT REBELLION");
		PrintFormat("[RBL] Guerrilla Warfare for Arma Reforger");
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] Waiting for world to load...");
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (!m_bEnableRebellion)
			return;
		
		// Delayed initialization to ensure world is ready
		if (!m_bSystemsInitialized)
		{
			m_fInitDelay += timeSlice;
			if (m_fInitDelay >= INIT_DELAY_TIME)
			{
				InitializeSystems();
				m_bSystemsInitialized = true;
			}
			return;
		}
		
		// Update systems
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
			commanderAI.Update(timeSlice);
		
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
			undercover.Update(timeSlice);
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.Update(timeSlice);
		
		RBL_PersistenceManager persistence = RBL_PersistenceManager.GetInstance();
		if (persistence)
			persistence.Update(timeSlice);
	}
	
	protected void InitializeSystems()
	{
		PrintFormat("[RBL] Initializing Rebellion systems...");
		
		if (m_bAutoInitialize)
		{
			// Use auto-initializer for plug-and-play
			RBL_AutoInitializer autoInit = RBL_AutoInitializer.GetInstance();
			autoInit.Initialize();
		}
		else
		{
			// Manual mode - just create managers
			RBL_ZoneManager.GetInstance();
			RBL_EconomyManager.GetInstance();
			RBL_CommanderAI.GetInstance();
			RBL_UndercoverSystem.GetInstance();
			RBL_PersistenceManager.GetInstance();
		}
		
		PrintZoneInfo();
	}
	
	protected void PrintZoneInfo()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] CAMPAIGN READY!");
		PrintFormat("[RBL] Total Zones: %1", zoneMgr.GetTotalZoneCount());
		PrintFormat("[RBL] FIA Controls: %1", zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA));
		PrintFormat("[RBL] Enemy Controls: %1", zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR));
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] Your mission: Liberate all zones!");
		PrintFormat("[RBL] ========================================");
	}
	
	// Debug command to print status
	void DebugPrintStatus()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.PrintZoneStatus();
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
			econMgr.PrintEconomyStatus();
	}
}

// ============================================================================
// MODDED GAME MODE - Hooks into ANY existing game mode
// ============================================================================
modded class SCR_BaseGameMode
{
	protected ref RBL_GameModeAddon m_RBLAddon;
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		// Auto-attach Rebellion to any game mode
		m_RBLAddon = new RBL_GameModeAddon();
		m_RBLAddon.OnGameModeInit(this);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);
		
		if (m_RBLAddon)
			m_RBLAddon.OnGameModeFrame(timeSlice);
	}
}

class RBL_GameModeAddon
{
	protected bool m_bInitialized;
	protected float m_fInitDelay;
	protected const float INIT_DELAY = 3.0;
	
	void RBL_GameModeAddon()
	{
		m_bInitialized = false;
		m_fInitDelay = 0;
	}
	
	void OnGameModeInit(SCR_BaseGameMode gameMode)
	{
		PrintFormat("[RBL] Rebellion addon attached to game mode");
	}
	
	void OnGameModeFrame(float timeSlice)
	{
		if (m_bInitialized)
		{
			UpdateSystems(timeSlice);
			return;
		}
		
		m_fInitDelay += timeSlice;
		if (m_fInitDelay >= INIT_DELAY)
		{
			Initialize();
			m_bInitialized = true;
		}
	}
	
	protected void Initialize()
	{
		PrintFormat("[RBL] ========================================");
		PrintFormat("[RBL] PROJECT REBELLION - ACTIVE");
		PrintFormat("[RBL] ========================================");
		
		RBL_AutoInitializer autoInit = RBL_AutoInitializer.GetInstance();
		autoInit.Initialize();
	}
	
	protected void UpdateSystems(float timeSlice)
	{
		RBL_CommanderAI commanderAI = RBL_CommanderAI.GetInstance();
		if (commanderAI)
			commanderAI.Update(timeSlice);
		
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.Update(timeSlice);
		
		RBL_PersistenceManager persistence = RBL_PersistenceManager.GetInstance();
		if (persistence)
			persistence.Update(timeSlice);
	}
}
