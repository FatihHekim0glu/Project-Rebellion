// ============================================================================
// PROJECT REBELLION - Save Restorer
// Orchestrates full game state restoration from save data
// ============================================================================

class RBL_SaveRestorer
{
	protected static ref RBL_SaveRestorer s_Instance;
	
	// Restoration state
	protected bool m_bIsRestoring;
	protected int m_iRestorePhase;
	protected ref RBL_SaveData m_PendingRestore;
	protected ref RBL_RestoreResult m_RestoreResult;
	
	// Callbacks
	protected ref ScriptInvoker m_OnRestoreStart;
	protected ref ScriptInvoker m_OnRestoreProgress;
	protected ref ScriptInvoker m_OnRestoreComplete;
	protected ref ScriptInvoker m_OnRestoreFailed;
	
	// Phases
	protected const int PHASE_VALIDATE = 0;
	protected const int PHASE_CAMPAIGN = 1;
	protected const int PHASE_ZONES = 2;
	protected const int PHASE_ECONOMY = 3;
	protected const int PHASE_PLAYERS = 4;
	protected const int PHASE_COMMANDER = 5;
	protected const int PHASE_MISSIONS = 6;
	protected const int PHASE_FINALIZE = 7;
	protected const int PHASE_COMPLETE = 8;
	
	// ========================================================================
	// SINGLETON
	// ========================================================================
	
	static RBL_SaveRestorer GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_SaveRestorer();
		return s_Instance;
	}
	
	void RBL_SaveRestorer()
	{
		m_bIsRestoring = false;
		m_iRestorePhase = 0;
		m_OnRestoreStart = new ScriptInvoker();
		m_OnRestoreProgress = new ScriptInvoker();
		m_OnRestoreComplete = new ScriptInvoker();
		m_OnRestoreFailed = new ScriptInvoker();
	}
	
	// ========================================================================
	// MAIN RESTORE API
	// ========================================================================
	
	// Start full restoration from save data
	bool StartRestore(RBL_SaveData saveData)
	{
		if (m_bIsRestoring)
		{
			PrintFormat("[RBL_SaveRestorer] Already restoring");
			return false;
		}
		
		if (!saveData || !saveData.IsValid())
		{
			PrintFormat("[RBL_SaveRestorer] Invalid save data");
			m_OnRestoreFailed.Invoke("Invalid save data");
			return false;
		}
		
		m_PendingRestore = saveData;
		m_bIsRestoring = true;
		m_iRestorePhase = PHASE_VALIDATE;
		m_RestoreResult = new RBL_RestoreResult();
		
		PrintFormat("[RBL_SaveRestorer] Starting restoration...");
		m_OnRestoreStart.Invoke();
		
		// Start phased restoration
		ProcessNextPhase();
		
		return true;
	}
	
	// Start restoration from slot
	bool StartRestoreFromSlot(int slotIndex)
	{
		RBL_SaveData saveData = RBL_SaveSlotManager.GetInstance().LoadFromSlot(slotIndex);
		if (!saveData)
		{
			m_OnRestoreFailed.Invoke("Failed to load slot " + slotIndex.ToString());
			return false;
		}
		
		return StartRestore(saveData);
	}
	
	// Start restoration from file
	bool StartRestoreFromFile(string filename)
	{
		RBL_SaveFileManager fileMgr = RBL_SaveFileManager.GetInstance();
		string json = fileMgr.ReadFromFile(filename);
		
		if (json.IsEmpty())
		{
			m_OnRestoreFailed.Invoke("Failed to read file: " + filename);
			return false;
		}
		
		RBL_SaveData saveData = RBL_SaveSerializer.DeserializeFromJson(json);
		if (!saveData)
		{
			m_OnRestoreFailed.Invoke("Failed to parse save file");
			return false;
		}
		
		return StartRestore(saveData);
	}
	
	// ========================================================================
	// PHASED RESTORATION
	// ========================================================================
	
	protected void ProcessNextPhase()
	{
		if (!m_bIsRestoring || !m_PendingRestore)
			return;
		
		bool success = true;
		string phaseName = GetPhaseName(m_iRestorePhase);
		
		PrintFormat("[RBL_SaveRestorer] Phase %1: %2", m_iRestorePhase, phaseName);
		
		switch (m_iRestorePhase)
		{
			case PHASE_VALIDATE:
				success = PhaseValidate();
				break;
			case PHASE_CAMPAIGN:
				success = PhaseCampaign();
				break;
			case PHASE_ZONES:
				success = PhaseZones();
				break;
			case PHASE_ECONOMY:
				success = PhaseEconomy();
				break;
			case PHASE_PLAYERS:
				success = PhasePlayers();
				break;
			case PHASE_COMMANDER:
				success = PhaseCommander();
				break;
			case PHASE_MISSIONS:
				success = PhaseMissions();
				break;
			case PHASE_FINALIZE:
				success = PhaseFinalize();
				break;
			case PHASE_COMPLETE:
				CompleteRestore();
				return;
		}
		
		// Invoke progress callback
		float progress = m_iRestorePhase / (float)PHASE_COMPLETE;
		m_OnRestoreProgress.Invoke(progress, phaseName);
		
		if (success)
		{
			// Move to next phase
			m_iRestorePhase++;
			// Schedule next phase (allows UI to update)
			GetGame().GetCallqueue().CallLater(ProcessNextPhase, 100, false);
		}
		else
		{
			// Phase failed
			FailRestore("Phase failed: " + phaseName);
		}
	}
	
	// ========================================================================
	// RESTORE PHASES
	// ========================================================================
	
	// Phase 0: Validate save data
	protected bool PhaseValidate()
	{
		if (!m_PendingRestore)
			return false;
		
		// Check version
		if (m_PendingRestore.NeedsMigration())
		{
			PrintFormat("[RBL_SaveRestorer] Save needs migration from v%1 to v%2",
				m_PendingRestore.m_iVersion, RBL_SAVE_VERSION);
			
			// Attempt migration
			if (!MigrateSaveData(m_PendingRestore))
			{
				m_RestoreResult.m_bMigrationFailed = true;
				return false;
			}
		}
		
		// Validate world match
		string currentWorld = GetCurrentWorldName();
		if (!currentWorld.IsEmpty() && !m_PendingRestore.m_sWorldName.IsEmpty())
		{
			if (currentWorld != m_PendingRestore.m_sWorldName)
			{
				PrintFormat("[RBL_SaveRestorer] World mismatch: %1 vs %2", currentWorld, m_PendingRestore.m_sWorldName);
				m_RestoreResult.m_bWorldMismatch = true;
				// Continue anyway but warn
			}
		}
		
		m_RestoreResult.m_bValidationPassed = true;
		return true;
	}
	
	// Phase 1: Restore campaign state
	protected bool PhaseCampaign()
	{
		if (!m_PendingRestore.m_Campaign)
			return true; // No campaign data, skip
		
		bool success = RBL_CampaignPersistence.GetInstance().RestoreCampaignState(m_PendingRestore.m_Campaign);
		m_RestoreResult.m_bCampaignRestored = success;
		
		return success;
	}
	
	// Phase 2: Restore zones
	protected bool PhaseZones()
	{
		if (!m_PendingRestore.m_aZones || m_PendingRestore.m_aZones.IsEmpty())
			return true; // No zone data, skip
		
		int restored = RBL_ZonePersistence.GetInstance().RestoreAllZoneStates(m_PendingRestore.m_aZones);
		m_RestoreResult.m_iZonesRestored = restored;
		m_RestoreResult.m_bZonesRestored = (restored > 0);
		
		return true; // Don't fail on partial zone restore
	}
	
	// Phase 3: Restore economy
	protected bool PhaseEconomy()
	{
		if (!m_PendingRestore.m_Economy)
			return true; // No economy data, skip
		
		bool success = RBL_EconomyPersistence.GetInstance().RestoreEconomyState(m_PendingRestore.m_Economy);
		m_RestoreResult.m_bEconomyRestored = success;
		
		return success;
	}
	
	// Phase 4: Restore players
	protected bool PhasePlayers()
	{
		if (!m_PendingRestore.m_aPlayers || m_PendingRestore.m_aPlayers.IsEmpty())
			return true; // No player data, skip
		
		int restored = RBL_PlayerPersistence.GetInstance().RestoreAllPlayerStates(m_PendingRestore.m_aPlayers);
		m_RestoreResult.m_iPlayersRestored = restored;
		m_RestoreResult.m_bPlayersRestored = true; // Don't fail on partial player restore
		
		return true;
	}
	
	// Phase 5: Restore commander AI
	protected bool PhaseCommander()
	{
		if (!m_PendingRestore.m_Commander)
			return true; // No commander data, skip
		
		bool success = RBL_CampaignPersistence.GetInstance().RestoreCommanderState(m_PendingRestore.m_Commander);
		m_RestoreResult.m_bCommanderRestored = success;
		
		return true; // Don't fail on commander restore
	}
	
	// Phase 6: Restore missions
	protected bool PhaseMissions()
	{
		if (!m_PendingRestore.m_aMissions || m_PendingRestore.m_aMissions.IsEmpty())
			return true; // No mission data, skip
		
		int restored = RBL_CampaignPersistence.GetInstance().RestoreActiveMissions(m_PendingRestore.m_aMissions);
		m_RestoreResult.m_iMissionsRestored = restored;
		m_RestoreResult.m_bMissionsRestored = true;
		
		return true;
	}
	
	// Phase 7: Finalize restoration
	protected bool PhaseFinalize()
	{
		// Refresh all systems
		RefreshAllSystems();
		
		// Notify systems of load completion
		BroadcastLoadComplete();
		
		m_RestoreResult.m_bFinalized = true;
		return true;
	}
	
	// ========================================================================
	// COMPLETION / FAILURE
	// ========================================================================
	
	protected void CompleteRestore()
	{
		m_bIsRestoring = false;
		m_RestoreResult.m_bSuccess = true;
		m_RestoreResult.m_fCompletionTime = GetGame().GetWorld().GetWorldTime();
		
		PrintFormat("[RBL_SaveRestorer] ========================================");
		PrintFormat("[RBL_SaveRestorer] RESTORATION COMPLETE");
		PrintFormat("[RBL_SaveRestorer] Zones: %1", m_RestoreResult.m_iZonesRestored);
		PrintFormat("[RBL_SaveRestorer] Players: %1", m_RestoreResult.m_iPlayersRestored);
		PrintFormat("[RBL_SaveRestorer] Missions: %1", m_RestoreResult.m_iMissionsRestored);
		PrintFormat("[RBL_SaveRestorer] ========================================");
		
		m_OnRestoreComplete.Invoke(m_RestoreResult);
		
		// Show notification
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ShowNotification("Game Loaded Successfully", RBL_UIColors.COLOR_ACCENT_GREEN, 3.0);
		}
		
		m_PendingRestore = null;
	}
	
	protected void FailRestore(string reason)
	{
		m_bIsRestoring = false;
		m_RestoreResult.m_bSuccess = false;
		m_RestoreResult.m_sFailureReason = reason;
		
		PrintFormat("[RBL_SaveRestorer] RESTORATION FAILED: %1", reason);
		
		m_OnRestoreFailed.Invoke(reason);
		
		// Show notification
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			uiMgr.ShowNotification("Load Failed: " + reason, RBL_UIColors.COLOR_ACCENT_RED, 4.0);
		}
		
		m_PendingRestore = null;
	}
	
	// ========================================================================
	// UTILITIES
	// ========================================================================
	
	// Get phase name
	protected string GetPhaseName(int phase)
	{
		switch (phase)
		{
			case PHASE_VALIDATE: return "Validating";
			case PHASE_CAMPAIGN: return "Campaign State";
			case PHASE_ZONES: return "Zone States";
			case PHASE_ECONOMY: return "Economy";
			case PHASE_PLAYERS: return "Player Data";
			case PHASE_COMMANDER: return "Commander AI";
			case PHASE_MISSIONS: return "Missions";
			case PHASE_FINALIZE: return "Finalizing";
			case PHASE_COMPLETE: return "Complete";
		}
		return "Unknown";
	}
	
	// Get current world name
	protected string GetCurrentWorldName()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return "";
		
		return world.GetName();
	}
	
	// Migrate save data to current version
	protected bool MigrateSaveData(RBL_SaveData saveData)
	{
		if (!saveData)
			return false;
		
		int fromVersion = saveData.m_iVersion;
		
		// Apply migrations sequentially
		while (saveData.m_iVersion < RBL_SAVE_VERSION)
		{
			bool migrated = false;
			
			// Version-specific migrations
			switch (saveData.m_iVersion)
			{
				case 0:
					migrated = MigrateV0ToV1(saveData);
					break;
				// Add more migrations as needed
			}
			
			if (!migrated)
			{
				PrintFormat("[RBL_SaveRestorer] Migration failed at v%1", saveData.m_iVersion);
				return false;
			}
			
			saveData.m_iVersion++;
		}
		
		PrintFormat("[RBL_SaveRestorer] Migrated save from v%1 to v%2", fromVersion, saveData.m_iVersion);
		return true;
	}
	
	// V0 to V1 migration (example)
	protected bool MigrateV0ToV1(RBL_SaveData saveData)
	{
		// Example migration - adjust fields as needed
		// In real implementation, handle field additions/removals/renames
		return true;
	}
	
	// Refresh all game systems
	protected void RefreshAllSystems()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.RefreshZoneStates();
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
			econMgr.RecalculateIncome();
		
		RBL_CommanderAI commander = RBL_CommanderAI.GetInstance();
		if (commander)
			commander.ReassessSituation();
	}
	
	// Broadcast load complete to all systems
	protected void BroadcastLoadComplete()
	{
		// Notify all relevant systems
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
			campaignMgr.OnGameLoaded();
	}
	
	// Check if currently restoring
	bool IsRestoring() { return m_bIsRestoring; }
	int GetCurrentPhase() { return m_iRestorePhase; }
	
	// Event accessors
	ScriptInvoker GetOnRestoreStart() { return m_OnRestoreStart; }
	ScriptInvoker GetOnRestoreProgress() { return m_OnRestoreProgress; }
	ScriptInvoker GetOnRestoreComplete() { return m_OnRestoreComplete; }
	ScriptInvoker GetOnRestoreFailed() { return m_OnRestoreFailed; }
}

// ============================================================================
// RESTORE RESULT
// ============================================================================
class RBL_RestoreResult
{
	bool m_bSuccess;
	string m_sFailureReason;
	float m_fCompletionTime;
	
	// Validation
	bool m_bValidationPassed;
	bool m_bMigrationFailed;
	bool m_bWorldMismatch;
	
	// Phase results
	bool m_bCampaignRestored;
	bool m_bZonesRestored;
	bool m_bEconomyRestored;
	bool m_bPlayersRestored;
	bool m_bCommanderRestored;
	bool m_bMissionsRestored;
	bool m_bFinalized;
	
	// Counts
	int m_iZonesRestored;
	int m_iPlayersRestored;
	int m_iMissionsRestored;
	
	void RBL_RestoreResult()
	{
		m_bSuccess = false;
		m_sFailureReason = "";
		m_fCompletionTime = 0;
		m_bValidationPassed = false;
		m_bMigrationFailed = false;
		m_bWorldMismatch = false;
		m_bCampaignRestored = false;
		m_bZonesRestored = false;
		m_bEconomyRestored = false;
		m_bPlayersRestored = false;
		m_bCommanderRestored = false;
		m_bMissionsRestored = false;
		m_bFinalized = false;
		m_iZonesRestored = 0;
		m_iPlayersRestored = 0;
		m_iMissionsRestored = 0;
	}
	
	string GetSummary()
	{
		if (!m_bSuccess)
			return "Failed: " + m_sFailureReason;
		
		return string.Format("Restored: %1 zones, %2 players, %3 missions",
			m_iZonesRestored, m_iPlayersRestored, m_iMissionsRestored);
	}
}

