// ============================================================================
// PROJECT REBELLION - Player Persistence
// Saves and restores player data (stats, progression, inventory)
// ============================================================================

class RBL_PlayerPersistence
{
	protected static ref RBL_PlayerPersistence s_Instance;
	
	static RBL_PlayerPersistence GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_PlayerPersistence();
		return s_Instance;
	}
	
	// ========================================================================
	// COLLECT PLAYER DATA
	// ========================================================================
	
	// Collect all players' data
	array<ref RBL_PlayerSaveData> CollectAllPlayerStates()
	{
		array<ref RBL_PlayerSaveData> playerStates = new array<ref RBL_PlayerSaveData>();
		
		// Get all connected players
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
		{
			PrintFormat("[RBL_PlayerPersistence] PlayerManager not available");
			return playerStates;
		}
		
		array<int> playerIds = new array<int>();
		pm.GetPlayers(playerIds);
		
		for (int i = 0; i < playerIds.Count(); i++)
		{
			int playerId = playerIds[i];
			RBL_PlayerSaveData playerData = CollectPlayerState(playerId);
			if (playerData)
				playerStates.Insert(playerData);
		}
		
		PrintFormat("[RBL_PlayerPersistence] Collected %1 player states", playerStates.Count());
		return playerStates;
	}
	
	// Collect single player's data
	RBL_PlayerSaveData CollectPlayerState(int playerId)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return null;
		
		IEntity playerEntity = pm.GetPlayerControlledEntity(playerId);
		if (!playerEntity)
			return null;
		
		RBL_PlayerSaveData data = new RBL_PlayerSaveData();
		
		// Identity
		data.m_sPlayerUID = playerId.ToString();
		data.m_sPlayerName = pm.GetPlayerName(playerId);
		
		// Resources (from economy manager)
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			data.m_iMoney = econMgr.GetPlayerMoney(playerId);
			data.m_iHumanResources = econMgr.GetPlayerHR(playerId);
		}
		
		// Stats (from stats tracker)
		RBL_StatsTracker stats = RBL_StatsTracker.GetInstance();
		if (stats)
		{
			data.m_iKills = stats.GetKills(playerId);
			data.m_iDeaths = stats.GetDeaths(playerId);
			data.m_iZonesCaptured = stats.GetZonesCaptured(playerId);
			data.m_iMissionsCompleted = stats.GetMissionsCompleted(playerId);
			data.m_fPlayTime = stats.GetPlayTime(playerId);
		}
		
		// Progression
		RBL_ProgressionManager progression = RBL_ProgressionManager.GetInstance();
		if (progression)
		{
			data.m_iRank = progression.GetRank(playerId);
			data.m_iExperience = progression.GetExperience(playerId);
			CollectPlayerUnlocks(playerId, data.m_aUnlocks);
			CollectPlayerAchievements(playerId, data.m_aAchievements);
		}
		
		// Inventory
		CollectPlayerInventory(playerEntity, data.m_aInventoryItems);
		
		// Position
		data.m_vLastPosition = playerEntity.GetOrigin();
		data.m_fLastDirection = GetPlayerDirection(playerEntity);
		
		// Undercover state
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
		{
			data.m_iCoverStatus = undercover.GetCoverStatus(playerId);
			data.m_fSuspicionLevel = undercover.GetSuspicionLevel(playerId);
		}
		
		return data;
	}
	
	// Collect player unlocks
	protected void CollectPlayerUnlocks(int playerId, array<string> outUnlocks)
	{
		outUnlocks.Clear();
		
		RBL_ProgressionManager progression = RBL_ProgressionManager.GetInstance();
		if (!progression)
			return;
		
		array<string> unlocks = progression.GetPlayerUnlocks(playerId);
		if (unlocks)
		{
			for (int i = 0; i < unlocks.Count(); i++)
			{
				outUnlocks.Insert(unlocks[i]);
			}
		}
	}
	
	// Collect player achievements
	protected void CollectPlayerAchievements(int playerId, array<string> outAchievements)
	{
		outAchievements.Clear();
		
		RBL_ProgressionManager progression = RBL_ProgressionManager.GetInstance();
		if (!progression)
			return;
		
		array<string> achievements = progression.GetPlayerAchievements(playerId);
		if (achievements)
		{
			for (int i = 0; i < achievements.Count(); i++)
			{
				outAchievements.Insert(achievements[i]);
			}
		}
	}
	
	// Collect player inventory
	protected void CollectPlayerInventory(IEntity playerEntity, array<string> outItems)
	{
		outItems.Clear();
		
		if (!playerEntity)
			return;
		
		// Get inventory component
		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(
			playerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!invMgr)
			return;
		
		// Collect all item prefab names
		array<IEntity> items = new array<IEntity>();
		invMgr.GetItems(items);
		
		for (int i = 0; i < items.Count(); i++)
		{
			IEntity item = items[i];
			if (!item)
				continue;
			
			// Get prefab name
			ResourceName prefab = item.GetPrefabData().GetPrefabName();
			outItems.Insert(prefab);
		}
	}
	
	// Get player direction (yaw)
	protected float GetPlayerDirection(IEntity playerEntity)
	{
		if (!playerEntity)
			return 0;
		
		vector angles = playerEntity.GetAngles();
		return angles[1]; // Yaw
	}
	
	// ========================================================================
	// RESTORE PLAYER DATA
	// ========================================================================
	
	// Restore all players from save data
	int RestoreAllPlayerStates(array<ref RBL_PlayerSaveData> playerStates)
	{
		if (!playerStates || playerStates.IsEmpty())
		{
			PrintFormat("[RBL_PlayerPersistence] No player states to restore");
			return 0;
		}
		
		int restoredCount = 0;
		
		for (int i = 0; i < playerStates.Count(); i++)
		{
			RBL_PlayerSaveData playerData = playerStates[i];
			if (!playerData)
				continue;
			
			// Find player by UID
			int playerId = FindPlayerByUID(playerData.m_sPlayerUID);
			
			if (playerId >= 0)
			{
				if (RestorePlayerState(playerId, playerData))
					restoredCount++;
			}
			else
			{
				// Player not connected - store for later restoration
				StoreOfflinePlayerData(playerData);
			}
		}
		
		PrintFormat("[RBL_PlayerPersistence] Restored %1 player states", restoredCount);
		return restoredCount;
	}
	
	// Restore single player's state
	bool RestorePlayerState(int playerId, RBL_PlayerSaveData data)
	{
		if (!data)
			return false;
		
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return false;
		
		// Restore resources
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			econMgr.SetPlayerMoney(playerId, data.m_iMoney);
			econMgr.SetPlayerHR(playerId, data.m_iHumanResources);
		}
		
		// Restore stats
		RBL_StatsTracker stats = RBL_StatsTracker.GetInstance();
		if (stats)
		{
			stats.SetKills(playerId, data.m_iKills);
			stats.SetDeaths(playerId, data.m_iDeaths);
			stats.SetZonesCaptured(playerId, data.m_iZonesCaptured);
			stats.SetMissionsCompleted(playerId, data.m_iMissionsCompleted);
			stats.SetPlayTime(playerId, data.m_fPlayTime);
		}
		
		// Restore progression
		RBL_ProgressionManager progression = RBL_ProgressionManager.GetInstance();
		if (progression)
		{
			progression.SetRank(playerId, data.m_iRank);
			progression.SetExperience(playerId, data.m_iExperience);
			RestorePlayerUnlocks(playerId, data.m_aUnlocks);
			RestorePlayerAchievements(playerId, data.m_aAchievements);
		}
		
		// Restore position (if valid)
		if (data.m_vLastPosition != vector.Zero)
		{
			TeleportPlayerToPosition(playerId, data.m_vLastPosition, data.m_fLastDirection);
		}
		
		// Restore inventory
		RestorePlayerInventory(playerId, data.m_aInventoryItems);
		
		// Restore undercover state
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover)
		{
			undercover.SetCoverStatus(playerId, data.m_iCoverStatus);
			undercover.SetSuspicionLevel(playerId, data.m_fSuspicionLevel);
		}
		
		return true;
	}
	
	// Restore player unlocks
	protected void RestorePlayerUnlocks(int playerId, array<string> unlocks)
	{
		if (!unlocks)
			return;
		
		RBL_ProgressionManager progression = RBL_ProgressionManager.GetInstance();
		if (!progression)
			return;
		
		progression.ClearPlayerUnlocks(playerId);
		
		for (int i = 0; i < unlocks.Count(); i++)
		{
			progression.AddPlayerUnlock(playerId, unlocks[i]);
		}
	}
	
	// Restore player achievements
	protected void RestorePlayerAchievements(int playerId, array<string> achievements)
	{
		if (!achievements)
			return;
		
		RBL_ProgressionManager progression = RBL_ProgressionManager.GetInstance();
		if (!progression)
			return;
		
		progression.ClearPlayerAchievements(playerId);
		
		for (int i = 0; i < achievements.Count(); i++)
		{
			progression.AddPlayerAchievement(playerId, achievements[i]);
		}
	}
	
	// Restore player inventory
	protected void RestorePlayerInventory(int playerId, array<string> items)
	{
		if (!items || items.IsEmpty())
			return;
		
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;
		
		IEntity playerEntity = pm.GetPlayerControlledEntity(playerId);
		if (!playerEntity)
			return;
		
		// Get inventory component
		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(
			playerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!invMgr)
			return;
		
		// Clear current inventory
		array<IEntity> currentItems = new array<IEntity>();
		invMgr.GetItems(currentItems);
		
		for (int i = 0; i < currentItems.Count(); i++)
		{
			IEntity item = currentItems[i];
			if (!item)
				continue;
			
			invMgr.TryDeleteItem(item);
		}
		
		// Add saved items
		for (int i = 0; i < items.Count(); i++)
		{
			string prefabName = items[i];
			if (!prefabName.IsEmpty())
			{
				// Spawn item and add to inventory
				IEntity newItem = SpawnItem(prefabName);
				if (newItem)
					invMgr.TryInsertItem(newItem);
			}
		}
	}
	
	// Teleport player to position
	protected void TeleportPlayerToPosition(int playerId, vector position, float direction)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return;
		
		IEntity playerEntity = pm.GetPlayerControlledEntity(playerId);
		if (!playerEntity)
			return;
		
		// Set position
		playerEntity.SetOrigin(position);
		
		// Set direction
		vector angles = playerEntity.GetAngles();
		angles[1] = direction;
		playerEntity.SetAngles(angles);
	}
	
	// ========================================================================
	// OFFLINE PLAYER HANDLING
	// ========================================================================
	
	protected ref map<string, ref RBL_PlayerSaveData> m_mOfflinePlayerData;
	
	// Store data for offline player (to restore when they reconnect)
	void StoreOfflinePlayerData(RBL_PlayerSaveData data)
	{
		if (!data || data.m_sPlayerUID.IsEmpty())
			return;
		
		if (!m_mOfflinePlayerData)
			m_mOfflinePlayerData = new map<string, ref RBL_PlayerSaveData>();
		
		m_mOfflinePlayerData.Set(data.m_sPlayerUID, data);
		PrintFormat("[RBL_PlayerPersistence] Stored offline data for: %1", data.m_sPlayerName);
	}
	
	// Check and restore data when player connects
	bool TryRestoreOnConnect(int playerId)
	{
		if (!m_mOfflinePlayerData)
			return false;
		
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return false;
		
		string uid = playerId.ToString();
		if (uid.IsEmpty())
			return false;
		
		RBL_PlayerSaveData savedData = m_mOfflinePlayerData.Get(uid);
		if (!savedData)
			return false;
		
		// Restore with delay to ensure entity is ready
		GetGame().GetCallqueue().CallLater(DelayedRestore, 2000, false, playerId, savedData);
		
		// Remove from offline storage
		m_mOfflinePlayerData.Remove(uid);
		
		return true;
	}
	
	protected void DelayedRestore(int playerId, RBL_PlayerSaveData data)
	{
		RestorePlayerState(playerId, data);
		PrintFormat("[RBL_PlayerPersistence] Restored data for reconnecting player: %1", data.m_sPlayerName);
	}
	
	// ========================================================================
	// UTILITIES
	// ========================================================================
	
	// Find player ID by UID
	protected int FindPlayerByUID(string uid)
	{
		if (uid.IsEmpty())
			return -1;
		
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return -1;
		
		array<int> playerIds = new array<int>();
		pm.GetPlayers(playerIds);
		
		for (int i = 0; i < playerIds.Count(); i++)
		{
			if (playerIds[i].ToString() == uid)
				return playerIds[i];
		}
		
		return -1;
	}
	
	// Spawn item by prefab name
	protected IEntity SpawnItem(string prefabName)
	{
		if (prefabName.IsEmpty())
			return null;
		
		// Spawn at origin (will be moved to inventory)
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = vector.Zero;
		
		Resource resource = Resource.Load(prefabName);
		if (!resource)
			return null;
		
		return GetGame().SpawnEntityPrefab(resource, null, params);
	}
	
	// Validate player data
	bool ValidatePlayerData(RBL_PlayerSaveData data)
	{
		if (!data)
			return false;
		
		if (data.m_sPlayerUID.IsEmpty())
			return false;
		
		if (data.m_iMoney < 0 || data.m_iHumanResources < 0)
			return false;
		
		if (data.m_iKills < 0 || data.m_iDeaths < 0)
			return false;
		
		if (data.m_fPlayTime < 0)
			return false;
		
		return true;
	}
}

// ============================================================================
// PLACEHOLDER CLASSES FOR INTERFACES
// ============================================================================

class RBL_StatsTracker
{
	protected static ref RBL_StatsTracker s_Instance;
	static RBL_StatsTracker GetInstance() { if (!s_Instance) s_Instance = new RBL_StatsTracker(); return s_Instance; }
	
	int GetKills(int playerId) { return 0; }
	int GetDeaths(int playerId) { return 0; }
	int GetZonesCaptured(int playerId) { return 0; }
	int GetMissionsCompleted(int playerId) { return 0; }
	float GetPlayTime(int playerId) { return 0; }
	
	void SetKills(int playerId, int kills) {}
	void SetDeaths(int playerId, int deaths) {}
	void SetZonesCaptured(int playerId, int count) {}
	void SetMissionsCompleted(int playerId, int count) {}
	void SetPlayTime(int playerId, float time) {}
}

class RBL_ProgressionManager
{
	protected static ref RBL_ProgressionManager s_Instance;
	static RBL_ProgressionManager GetInstance() { if (!s_Instance) s_Instance = new RBL_ProgressionManager(); return s_Instance; }
	
	int GetRank(int playerId) { return 0; }
	int GetExperience(int playerId) { return 0; }
	array<string> GetPlayerUnlocks(int playerId) { return new array<string>(); }
	array<string> GetPlayerAchievements(int playerId) { return new array<string>(); }
	
	void SetRank(int playerId, int rank) {}
	void SetExperience(int playerId, int xp) {}
	void ClearPlayerUnlocks(int playerId) {}
	void AddPlayerUnlock(int playerId, string unlock) {}
	void ClearPlayerAchievements(int playerId) {}
	void AddPlayerAchievement(int playerId, string achievement) {}
}

