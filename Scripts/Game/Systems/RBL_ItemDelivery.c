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
		
		// Check if valid target
		if (!IsValidDeliveryTarget(playerEntity))
			return ERBLDeliveryResult.FAILED_NO_PLAYER;
		
		// Spawn the weapon near player (not exactly at origin to avoid collision)
		vector spawnPos = playerEntity.GetOrigin() + Vector(0, 1, 0);
		IEntity weaponEntity = SpawnEntity(item.PrefabPath, spawnPos);
		if (!weaponEntity)
			return ERBLDeliveryResult.FAILED_SPAWN_ERROR;
		
		// Try to add to player inventory
		bool added = AddToPlayerInventory(playerEntity, weaponEntity);
		
		if (added)
		{
			PrintFormat("[RBL_Delivery] Weapon %1 added to inventory", item.DisplayName);
			
			// Try to equip the weapon
			TryEquipWeapon(playerEntity, weaponEntity);
		}
		else
		{
			// If can't add to inventory, drop at player's feet
			vector dropPos = GetWeaponDropPosition(playerEntity);
			weaponEntity.SetOrigin(dropPos);
			PrintFormat("[RBL_Delivery] Weapon %1 dropped at player's feet", item.DisplayName);
		}
		
		return ERBLDeliveryResult.SUCCESS;
	}
	
	// Try to equip a weapon after adding to inventory
	protected void TryEquipWeapon(IEntity playerEntity, IEntity weaponEntity)
	{
		if (!playerEntity || !weaponEntity)
			return;
		
		// Get weapon manager
		BaseWeaponManagerComponent weaponMgr = BaseWeaponManagerComponent.Cast(
			playerEntity.FindComponent(BaseWeaponManagerComponent));
		
		if (!weaponMgr)
			return;
		
		// Get weapon component from entity
		BaseWeaponComponent weaponComp = BaseWeaponComponent.Cast(
			weaponEntity.FindComponent(BaseWeaponComponent));
		
		if (weaponComp)
		{
			// Attempt to select this weapon
			// Note: This may not work for all inventory systems
			PrintFormat("[RBL_Delivery] Attempting to equip weapon");
		}
	}
	
	// Get position to drop a weapon
	protected vector GetWeaponDropPosition(IEntity playerEntity)
	{
		vector playerPos = playerEntity.GetOrigin();
		vector playerAngles = playerEntity.GetAngles();
		
		// Drop in front of player
		float yaw = playerAngles[1] * Math.DEG2RAD;
		vector forward = Vector(Math.Sin(yaw), 0, Math.Cos(yaw));
		
		vector dropPos = playerPos + forward * 1.5;
		
		BaseWorld world = GetGame().GetWorld();
		if (world)
			dropPos[1] = world.GetSurfaceY(dropPos[0], dropPos[2]) + 0.3;
		
		return dropPos;
	}
	
	// ========================================================================
	// EQUIPMENT DELIVERY
	// ========================================================================
	ERBLDeliveryResult DeliverEquipment(IEntity playerEntity, RBL_ShopItem item)
	{
		if (!playerEntity || item.PrefabPath.IsEmpty())
			return ERBLDeliveryResult.FAILED_NO_PREFAB;
		
		// Check if valid target
		if (!IsValidDeliveryTarget(playerEntity))
			return ERBLDeliveryResult.FAILED_NO_PLAYER;
		
		// Spawn the equipment near player
		vector spawnPos = playerEntity.GetOrigin() + Vector(0, 1, 0);
		IEntity equipEntity = SpawnEntity(item.PrefabPath, spawnPos);
		if (!equipEntity)
			return ERBLDeliveryResult.FAILED_SPAWN_ERROR;
		
		// Try to add to player inventory
		bool added = AddToPlayerInventory(playerEntity, equipEntity);
		
		if (added)
		{
			PrintFormat("[RBL_Delivery] Equipment %1 added to inventory", item.DisplayName);
		}
		else
		{
			// Check if it's a stackable item (like bandages, grenades)
			bool stacked = TryStackEquipment(playerEntity, equipEntity, item);
			
			if (!stacked)
			{
				// Drop at player's feet as last resort
				vector dropPos = GetEquipmentDropPosition(playerEntity);
				equipEntity.SetOrigin(dropPos);
				PrintFormat("[RBL_Delivery] Equipment %1 dropped at player's feet", item.DisplayName);
			}
		}
		
		return ERBLDeliveryResult.SUCCESS;
	}
	
	// Try to stack equipment with existing items
	protected bool TryStackEquipment(IEntity playerEntity, IEntity equipEntity, RBL_ShopItem item)
	{
		if (!playerEntity || !equipEntity)
			return false;
		
		// Get inventory manager
		SCR_InventoryStorageManagerComponent invMgr = SCR_InventoryStorageManagerComponent.Cast(
			playerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!invMgr)
			return false;
		
		// Try to find matching stack and add
		// Note: This is a simplified implementation
		// Full implementation would search inventory for matching prefab
		
		return false;
	}
	
	// Get position to drop equipment
	protected vector GetEquipmentDropPosition(IEntity playerEntity)
	{
		vector playerPos = playerEntity.GetOrigin();
		vector playerAngles = playerEntity.GetAngles();
		
		// Drop slightly to the side
		float yaw = (playerAngles[1] + 30) * Math.DEG2RAD;
		vector offset = Vector(Math.Sin(yaw), 0, Math.Cos(yaw));
		
		vector dropPos = playerPos + offset * 1.0;
		
		BaseWorld world = GetGame().GetWorld();
		if (world)
			dropPos[1] = world.GetSurfaceY(dropPos[0], dropPos[2]) + 0.2;
		
		return dropPos;
	}
	
	// ========================================================================
	// VEHICLE DELIVERY
	// ========================================================================
	ERBLDeliveryResult DeliverVehicle(IEntity playerEntity, RBL_ShopItem item)
	{
		if (!playerEntity || item.PrefabPath.IsEmpty())
			return ERBLDeliveryResult.FAILED_NO_PREFAB;
		
		// Check if valid target
		if (!IsValidDeliveryTarget(playerEntity))
			return ERBLDeliveryResult.FAILED_NO_PLAYER;
		
		// Calculate spawn position in front of player
		vector spawnPos = GetVehicleSpawnPosition(playerEntity);
		
		// Verify spawn position is valid (not inside terrain/buildings)
		if (!IsValidVehicleSpawnPosition(spawnPos))
		{
			// Try alternative positions
			spawnPos = FindAlternativeVehicleSpawn(playerEntity);
		}
		
		// Spawn the vehicle
		IEntity vehicleEntity = SpawnVehicle(item.PrefabPath, spawnPos, playerEntity.GetAngles()[1]);
		if (!vehicleEntity)
			return ERBLDeliveryResult.FAILED_SPAWN_ERROR;
		
		PrintFormat("[RBL_Delivery] Vehicle %1 spawned at %2", item.DisplayName, spawnPos.ToString());
		
		return ERBLDeliveryResult.SUCCESS;
	}
	
	// Spawn vehicle with proper orientation and physics
	protected IEntity SpawnVehicle(string prefab, vector position, float yaw)
	{
		if (prefab.IsEmpty())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return null;
		
		// Ensure vehicle is on terrain with slight offset
		position[1] = world.GetSurfaceY(position[0], position[2]) + VEHICLE_SPAWN_HEIGHT_OFFSET;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		
		// Set rotation to match player facing
		vector angles = Vector(0, yaw, 0);
		Math3D.AnglesToMatrix(angles, params.Transform);
		params.Transform[3] = position;
		
		Resource resource = Resource.Load(prefab);
		if (!resource.IsValid())
		{
			PrintFormat("[RBL_Delivery] Invalid vehicle prefab: %1", prefab);
			return null;
		}
		
		IEntity vehicle = GetGame().SpawnEntityPrefab(resource, world, params);
		
		// Initialize vehicle physics if needed
		if (vehicle)
		{
			Physics physics = vehicle.GetPhysics();
			if (physics)
			{
				physics.SetActive(ActiveState.ACTIVE);
			}
		}
		
		return vehicle;
	}
	
	// Check if vehicle spawn position is valid
	protected bool IsValidVehicleSpawnPosition(vector position)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return false;
		
		// Check terrain height
		float terrainY = world.GetSurfaceY(position[0], position[2]);
		if (Math.AbsFloat(position[1] - terrainY) > 5.0)
			return false;
		
		// Could add building/obstacle check here
		return true;
	}
	
	// Find alternative spawn position if primary is blocked
	protected vector FindAlternativeVehicleSpawn(IEntity playerEntity)
	{
		vector playerPos = playerEntity.GetOrigin();
		
		// Try different angles
		for (int i = 0; i < 8; i++)
		{
			float angle = i * 45.0 * Math.DEG2RAD;
			vector offset = Vector(Math.Sin(angle), 0, Math.Cos(angle)) * VEHICLE_SPAWN_DISTANCE;
			vector testPos = playerPos + offset;
			
			if (IsValidVehicleSpawnPosition(testPos))
				return testPos;
		}
		
		// Fallback: just return front position
		return GetVehicleSpawnPosition(playerEntity);
	}
	
	// ========================================================================
	// RECRUIT DELIVERY
	// ========================================================================
	ERBLDeliveryResult DeliverRecruit(IEntity playerEntity, RBL_ShopItem item, int playerID)
	{
		if (!playerEntity)
			return ERBLDeliveryResult.FAILED_NO_PLAYER;
		
		// Check if valid target
		if (!IsValidDeliveryTarget(playerEntity))
			return ERBLDeliveryResult.FAILED_NO_PLAYER;
		
		// Get recruit prefab
		string prefab = GetRecruitPrefab(item.ID);
		if (prefab.IsEmpty())
			return ERBLDeliveryResult.FAILED_NO_PREFAB;
		
		// Calculate spawn position behind player
		vector baseSpawnPos = GetRecruitSpawnPosition(playerEntity);
		
		// Spawn recruit(s) based on HR cost
		int recruitCount = item.HRCost;
		if (recruitCount <= 0)
			recruitCount = 1;
		
		int successCount = 0;
		ref array<IEntity> spawnedRecruits = new array<IEntity>();
		
		for (int i = 0; i < recruitCount; i++)
		{
			// Calculate formation position
			vector offsetPos = GetRecruitFormationPosition(baseSpawnPos, i, recruitCount);
			
			IEntity recruitEntity = SpawnRecruit(prefab, offsetPos, playerEntity.GetAngles()[1]);
			if (recruitEntity)
			{
				spawnedRecruits.Insert(recruitEntity);
				successCount++;
				PrintFormat("[RBL_Delivery] Recruit %1/%2 spawned", successCount, recruitCount);
			}
		}
		
		// Add all recruits to player's group
		if (successCount > 0)
		{
			for (int i = 0; i < spawnedRecruits.Count(); i++)
			{
				AddRecruitToPlayerGroup(playerEntity, spawnedRecruits[i]);
			}
			PrintFormat("[RBL_Delivery] %1 recruits added to player's group", successCount);
		}
		
		return (successCount > 0) ? ERBLDeliveryResult.SUCCESS : ERBLDeliveryResult.FAILED_SPAWN_ERROR;
	}
	
	// Get prefab for recruit type
	protected string GetRecruitPrefab(string recruitID)
	{
		string prefab;
		if (m_mRecruitPrefabs.Find(recruitID, prefab))
			return prefab;
		
		// Default to rifleman
		m_mRecruitPrefabs.Find("rifleman", prefab);
		return prefab;
	}
	
	// Spawn a single recruit with proper orientation
	protected IEntity SpawnRecruit(string prefab, vector position, float yaw)
	{
		if (prefab.IsEmpty())
			return null;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return null;
		
		// Ensure on terrain
		position[1] = world.GetSurfaceY(position[0], position[2]);
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		
		// Face same direction as player
		vector angles = Vector(0, yaw, 0);
		Math3D.AnglesToMatrix(angles, params.Transform);
		params.Transform[3] = position;
		
		Resource resource = Resource.Load(prefab);
		if (!resource.IsValid())
		{
			PrintFormat("[RBL_Delivery] Invalid recruit prefab: %1", prefab);
			return null;
		}
		
		return GetGame().SpawnEntityPrefab(resource, world, params);
	}
	
	// Calculate formation position for multiple recruits
	protected vector GetRecruitFormationPosition(vector basePos, int index, int totalCount)
	{
		BaseWorld world = GetGame().GetWorld();
		
		if (totalCount == 1)
		{
			if (world)
				basePos[1] = world.GetSurfaceY(basePos[0], basePos[2]);
			return basePos;
		}
		
		// Create a line formation behind player
		float spacing = 2.0;
		int row = index / 3;
		int col = index % 3;
		
		float offsetX = (col - 1) * spacing;
		float offsetZ = -row * spacing;
		
		vector formationPos = basePos + Vector(offsetX, 0, offsetZ);
		
		if (world)
			formationPos[1] = world.GetSurfaceY(formationPos[0], formationPos[2]);
		
		return formationPos;
	}
	
	// ========================================================================
	// HELPER METHODS - PLAYER ACCESS
	// ========================================================================
	
	protected IEntity GetPlayerEntity(int playerID)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		if (!pm)
			return null;
		
		return pm.GetPlayerControlledEntity(playerID);
	}
	
	// Get local player entity (for single player or local client)
	IEntity GetLocalPlayerEntity()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;
		
		return pc.GetControlledEntity();
	}
	
	// Get local player ID
	int GetLocalPlayerID()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return -1;
		
		return pc.GetPlayerId();
	}
	
	// Check if player entity is valid for delivery
	bool IsValidDeliveryTarget(IEntity playerEntity)
	{
		if (!playerEntity)
			return false;
		
		// Check if alive
		DamageManagerComponent dmgMgr = DamageManagerComponent.Cast(
			playerEntity.FindComponent(DamageManagerComponent));
		
		if (dmgMgr && dmgMgr.GetState() == EDamageState.DESTROYED)
			return false;
		
		return true;
	}
	
	// Get player position with terrain check
	vector GetPlayerPosition(IEntity playerEntity)
	{
		if (!playerEntity)
			return vector.Zero;
		
		vector pos = playerEntity.GetOrigin();
		
		BaseWorld world = GetGame().GetWorld();
		if (world)
		{
			float terrainY = world.GetSurfaceY(pos[0], pos[2]);
			if (pos[1] < terrainY)
				pos[1] = terrainY;
		}
		
		return pos;
	}
	
	// ========================================================================
	// HELPER METHODS - SPAWNING
	// ========================================================================
	
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
