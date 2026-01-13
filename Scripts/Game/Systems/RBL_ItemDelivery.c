// ============================================================================
// PROJECT REBELLION - Item Delivery System
// Handles spawning and delivering purchased items to players
// ============================================================================

// Delivery result
enum ERBLDeliveryResult
{
	SUCCESS,
	FAILED_NO_PLAYER,
	FAILED_NO_PREFAB,
	FAILED_SPAWN_ERROR,
	FAILED_INVENTORY_FULL,
	FAILED_NO_SPACE
}

// Delivery event data
class RBL_DeliveryEventData
{
	int PlayerID;
	string ItemID;
	string Category;
	ERBLDeliveryResult Result;
	IEntity DeliveredEntity;
	
	void RBL_DeliveryEventData()
	{
		PlayerID = -1;
		DeliveredEntity = null;
		Result = ERBLDeliveryResult.SUCCESS;
	}
}

// ============================================================================
// ITEM DELIVERY SYSTEM
// ============================================================================
class RBL_ItemDelivery
{
	protected static ref RBL_ItemDelivery s_Instance;
	
	// Spawn offsets
	protected const float VEHICLE_SPAWN_DISTANCE = 10.0;
	protected const float RECRUIT_SPAWN_DISTANCE = 5.0;
	protected const float VEHICLE_SPAWN_HEIGHT_OFFSET = 0.5;
	
	// Events
	protected ref ScriptInvoker m_OnItemDelivered;
	protected ref ScriptInvoker m_OnDeliveryFailed;
	
	// Recruit prefabs
	protected ref map<string, string> m_mRecruitPrefabs;
	
	static RBL_ItemDelivery GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_ItemDelivery();
		return s_Instance;
	}
	
	void RBL_ItemDelivery()
	{
		m_OnItemDelivered = new ScriptInvoker();
		m_OnDeliveryFailed = new ScriptInvoker();
		
		InitializeRecruitPrefabs();
		
		PrintFormat("[RBL_Delivery] Item Delivery System initialized");
	}
	
	protected void InitializeRecruitPrefabs()
	{
		m_mRecruitPrefabs = new map<string, string>();
		
		// FIA faction prefabs
		m_mRecruitPrefabs.Set("rifleman", "{84CE36A610A57C88}Prefabs/Characters/Factions/FIA/Character_FIA_Rifleman.et");
		m_mRecruitPrefabs.Set("medic", "{5A26B77C6A35E8D1}Prefabs/Characters/Factions/FIA/Character_FIA_Medic.et");
		m_mRecruitPrefabs.Set("mg", "{3D94F1B2E8C7A456}Prefabs/Characters/Factions/FIA/Character_FIA_MG.et");
		m_mRecruitPrefabs.Set("at", "{2F81D3C5A9B64E12}Prefabs/Characters/Factions/FIA/Character_FIA_AT.et");
		m_mRecruitPrefabs.Set("sniper", "{7B3E9F4D1C8A2567}Prefabs/Characters/Factions/FIA/Character_FIA_Sniper.et");
	}
	
	// ========================================================================
	// MAIN DELIVERY METHODS
	// ========================================================================
	
	ERBLDeliveryResult DeliverItem(RBL_ShopItem item, int playerID)
	{
		if (!item)
			return ERBLDeliveryResult.FAILED_NO_PREFAB;
		
		IEntity playerEntity = GetPlayerEntity(playerID);
		if (!playerEntity)
			return ERBLDeliveryResult.FAILED_NO_PLAYER;
		
		ERBLDeliveryResult result;
		
		// Route to appropriate delivery method
		if (item.Category == "Weapons")
			result = DeliverWeapon(playerEntity, item);
		else if (item.Category == "Equipment")
			result = DeliverEquipment(playerEntity, item);
		else if (item.Category == "Vehicles")
			result = DeliverVehicle(playerEntity, item);
		else if (item.Category == "Recruits")
			result = DeliverRecruit(playerEntity, item, playerID);
		else
			result = ERBLDeliveryResult.FAILED_NO_PREFAB;
		
		// Fire events
		RBL_DeliveryEventData eventData = new RBL_DeliveryEventData();
		eventData.PlayerID = playerID;
		eventData.ItemID = item.ID;
		eventData.Category = item.Category;
		eventData.Result = result;
		
		if (result == ERBLDeliveryResult.SUCCESS)
		{
			m_OnItemDelivered.Invoke(eventData);
			PrintFormat("[RBL_Delivery] Delivered %1 to player %2", item.DisplayName, playerID);
		}
		else
		{
			m_OnDeliveryFailed.Invoke(eventData);
			PrintFormat("[RBL_Delivery] Failed to deliver %1: %2", item.DisplayName, typename.EnumToString(ERBLDeliveryResult, result));
		}
		
		return result;
	}
	
	// ========================================================================
	// WEAPON DELIVERY
	// ========================================================================
	ERBLDeliveryResult DeliverWeapon(IEntity playerEntity, RBL_ShopItem item)
	{
		if (!playerEntity || item.PrefabPath.IsEmpty())
			return ERBLDeliveryResult.FAILED_NO_PREFAB;
		
		// Spawn the weapon
		IEntity weaponEntity = SpawnEntity(item.PrefabPath, playerEntity.GetOrigin());
		if (!weaponEntity)
			return ERBLDeliveryResult.FAILED_SPAWN_ERROR;
		
		// Try to add to player inventory
		bool added = AddToPlayerInventory(playerEntity, weaponEntity);
		
		if (!added)
		{
			// If can't add to inventory, drop at player's feet
			vector dropPos = playerEntity.GetOrigin() + Vector(0, 0.5, 1);
			BaseWorld world = GetGame().GetWorld();
			if (world)
				dropPos[1] = world.GetSurfaceY(dropPos[0], dropPos[2]) + 0.5;
			
			weaponEntity.SetOrigin(dropPos);
			PrintFormat("[RBL_Delivery] Weapon dropped at player's feet (inventory full)");
		}
		
		return ERBLDeliveryResult.SUCCESS;
	}
	
	// ========================================================================
	// EQUIPMENT DELIVERY
	// ========================================================================
	ERBLDeliveryResult DeliverEquipment(IEntity playerEntity, RBL_ShopItem item)
	{
		if (!playerEntity || item.PrefabPath.IsEmpty())
			return ERBLDeliveryResult.FAILED_NO_PREFAB;
		
		// Spawn the equipment
		IEntity equipEntity = SpawnEntity(item.PrefabPath, playerEntity.GetOrigin());
		if (!equipEntity)
			return ERBLDeliveryResult.FAILED_SPAWN_ERROR;
		
		// Try to add to player inventory
		bool added = AddToPlayerInventory(playerEntity, equipEntity);
		
		if (!added)
		{
			// Drop at player's feet
			vector dropPos = playerEntity.GetOrigin() + Vector(0, 0.5, 1);
			BaseWorld world = GetGame().GetWorld();
			if (world)
				dropPos[1] = world.GetSurfaceY(dropPos[0], dropPos[2]) + 0.5;
			
			equipEntity.SetOrigin(dropPos);
			PrintFormat("[RBL_Delivery] Equipment dropped at player's feet (inventory full)");
		}
		
		return ERBLDeliveryResult.SUCCESS;
	}
	
	// ========================================================================
	// VEHICLE DELIVERY
	// ========================================================================
	ERBLDeliveryResult DeliverVehicle(IEntity playerEntity, RBL_ShopItem item)
	{
		if (!playerEntity || item.PrefabPath.IsEmpty())
			return ERBLDeliveryResult.FAILED_NO_PREFAB;
		
		// Calculate spawn position in front of player
		vector spawnPos = GetVehicleSpawnPosition(playerEntity);
		
		// Spawn the vehicle
		IEntity vehicleEntity = SpawnEntity(item.PrefabPath, spawnPos);
		if (!vehicleEntity)
			return ERBLDeliveryResult.FAILED_SPAWN_ERROR;
		
		// Orient vehicle to face same direction as player
		vector playerAngles = playerEntity.GetAngles();
		vehicleEntity.SetAngles(playerAngles);
		
		PrintFormat("[RBL_Delivery] Vehicle spawned at %1", spawnPos.ToString());
		
		return ERBLDeliveryResult.SUCCESS;
	}
	
	// ========================================================================
	// RECRUIT DELIVERY
	// ========================================================================
	ERBLDeliveryResult DeliverRecruit(IEntity playerEntity, RBL_ShopItem item, int playerID)
	{
		if (!playerEntity)
			return ERBLDeliveryResult.FAILED_NO_PLAYER;
		
		// Get recruit prefab
		string prefab;
		if (!m_mRecruitPrefabs.Find(item.ID, prefab))
		{
			// Use default rifleman
			prefab = m_mRecruitPrefabs.Get("rifleman");
		}
		
		if (prefab.IsEmpty())
			return ERBLDeliveryResult.FAILED_NO_PREFAB;
		
		// Calculate spawn position behind player
		vector spawnPos = GetRecruitSpawnPosition(playerEntity);
		
		// Spawn recruit(s) based on HR cost
		int recruitCount = item.HRCost;
		if (recruitCount <= 0)
			recruitCount = 1;
		
		for (int i = 0; i < recruitCount; i++)
		{
			// Offset each recruit slightly
			vector offsetPos = spawnPos + Vector(Math.RandomFloat(-2, 2), 0, Math.RandomFloat(-2, 2));
			BaseWorld world = GetGame().GetWorld();
			if (world)
				offsetPos[1] = world.GetSurfaceY(offsetPos[0], offsetPos[2]);
			
			IEntity recruitEntity = SpawnEntity(prefab, offsetPos);
			if (recruitEntity)
			{
				// Add to player's group
				AddRecruitToPlayerGroup(playerEntity, recruitEntity);
				PrintFormat("[RBL_Delivery] Recruit spawned at %1", offsetPos.ToString());
			}
		}
		
		return ERBLDeliveryResult.SUCCESS;
	}
	
	// ========================================================================
	// HELPER METHODS
	// ========================================================================
	
	protected IEntity GetPlayerEntity(int playerID)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return null;
		
		return pm.GetPlayerControlledEntity(playerID);
	}
	
	protected IEntity SpawnEntity(string prefab, vector position)
	{
		if (prefab.IsEmpty())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return null;
		
		// Adjust to terrain
		position[1] = world.GetSurfaceY(position[0], position[2]);
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = position;
		
		Resource resource = Resource.Load(prefab);
		if (!resource.IsValid())
		{
			PrintFormat("[RBL_Delivery] Invalid prefab: %1", prefab);
			return null;
		}
		
		return GetGame().SpawnEntityPrefab(resource, world, params);
	}
	
	protected bool AddToPlayerInventory(IEntity playerEntity, IEntity itemEntity)
	{
		if (!playerEntity || !itemEntity)
			return false;
		
		// Get inventory manager
		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(
			playerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!invMgr)
		{
			// Try base inventory component
			InventoryStorageManagerComponent baseInvMgr = InventoryStorageManagerComponent.Cast(
				playerEntity.FindComponent(InventoryStorageManagerComponent));
			
			if (!baseInvMgr)
				return false;
			
			return baseInvMgr.TryInsertItem(itemEntity);
		}
		
		return invMgr.TryInsertItem(itemEntity);
	}
	
	protected vector GetVehicleSpawnPosition(IEntity playerEntity)
	{
		vector playerPos = playerEntity.GetOrigin();
		vector playerForward = playerEntity.GetAngles();
		
		// Get forward direction
		float yaw = playerForward[1] * Math.DEG2RAD;
		vector forward = Vector(Math.Sin(yaw), 0, Math.Cos(yaw));
		
		vector spawnPos = playerPos + forward * VEHICLE_SPAWN_DISTANCE;
		
		// Adjust to terrain
		BaseWorld world = GetGame().GetWorld();
		if (world)
			spawnPos[1] = world.GetSurfaceY(spawnPos[0], spawnPos[2]) + VEHICLE_SPAWN_HEIGHT_OFFSET;
		
		return spawnPos;
	}
	
	protected vector GetRecruitSpawnPosition(IEntity playerEntity)
	{
		vector playerPos = playerEntity.GetOrigin();
		vector playerForward = playerEntity.GetAngles();
		
		// Get backward direction
		float yaw = playerForward[1] * Math.DEG2RAD;
		vector backward = Vector(-Math.Sin(yaw), 0, -Math.Cos(yaw));
		
		vector spawnPos = playerPos + backward * RECRUIT_SPAWN_DISTANCE;
		
		// Adjust to terrain
		BaseWorld world = GetGame().GetWorld();
		if (world)
			spawnPos[1] = world.GetSurfaceY(spawnPos[0], spawnPos[2]);
		
		return spawnPos;
	}
	
	protected void AddRecruitToPlayerGroup(IEntity playerEntity, IEntity recruitEntity)
	{
		if (!playerEntity || !recruitEntity)
			return;
		
		// Get AI control of recruit
		AIControlComponent aiControl = AIControlComponent.Cast(recruitEntity.FindComponent(AIControlComponent));
		if (!aiControl)
			return;
		
		AIAgent recruitAgent = aiControl.GetAIAgent();
		if (!recruitAgent)
			return;
		
		// Get player's group or create one
		SCR_PlayerControllerGroupComponent groupComp = SCR_PlayerControllerGroupComponent.Cast(
			GetGame().GetPlayerController().FindComponent(SCR_PlayerControllerGroupComponent));
		
		if (groupComp)
		{
			SCR_AIGroup playerGroup = groupComp.GetPlayersGroup();
			if (playerGroup)
			{
				playerGroup.AddAgent(recruitAgent);
				PrintFormat("[RBL_Delivery] Recruit added to player's group");
				return;
			}
		}
		
		// Fallback: Just make recruit follow player via waypoint
		PrintFormat("[RBL_Delivery] Recruit spawned (no group system available)");
	}
	
	// ========================================================================
	// PUBLIC API
	// ========================================================================
	
	ScriptInvoker GetOnItemDelivered() { return m_OnItemDelivered; }
	ScriptInvoker GetOnDeliveryFailed() { return m_OnDeliveryFailed; }
}
