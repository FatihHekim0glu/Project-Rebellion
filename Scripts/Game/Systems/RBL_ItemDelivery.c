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
	
	// Prefab mappings
	protected ref map<string, string> m_mRecruitPrefabs;
	protected ref map<string, string> m_mEquipmentPrefabs;
	protected ref map<string, string> m_mWeaponPrefabs;
	protected ref map<string, string> m_mVehiclePrefabs;
	
	// State
	protected bool m_bInitialized;
	protected int m_iDeliveriesTotal;
	protected int m_iDeliveriesSuccessful;
	protected int m_iDeliveriesFailed;
	
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
		
		m_mRecruitPrefabs = new map<string, string>();
		m_mEquipmentPrefabs = new map<string, string>();
		m_mWeaponPrefabs = new map<string, string>();
		m_mVehiclePrefabs = new map<string, string>();
		
		m_bInitialized = false;
		m_iDeliveriesTotal = 0;
		m_iDeliveriesSuccessful = 0;
		m_iDeliveriesFailed = 0;
	}
	
	void Initialize()
	{
		if (m_bInitialized)
			return;
		
		InitializeRecruitPrefabs();
		InitializeEquipmentPrefabs();
		InitializeWeaponPrefabs();
		InitializeVehiclePrefabs();
		
		m_bInitialized = true;
		PrintFormat("[RBL_Delivery] Item Delivery System initialized");
	}
	
	protected void InitializeEquipmentPrefabs()
	{
		m_mEquipmentPrefabs.Set("bandage", "{000CD338713F2B5A}Prefabs/Items/Medicine/Bandage_01.et");
		m_mEquipmentPrefabs.Set("medkit", "{000CD338713F2B5A}Prefabs/Items/Medicine/Bandage_01.et");
		m_mEquipmentPrefabs.Set("grenade_rgd", "{3805931BE86F827D}Prefabs/Weapons/Grenades/RGD5/Grenade_RGD5.et");
		m_mEquipmentPrefabs.Set("grenade_smoke", "{E629F77EC23B3C32}Prefabs/Weapons/Grenades/ANM8/Grenade_ANM8.et");
		m_mEquipmentPrefabs.Set("binocs", "{25E4B05E79B0E658}Prefabs/Items/Equipment/Binoculars_M/Binoculars_M.et");
		PrintFormat("[RBL_Delivery] Equipment prefabs initialized");
	}
	
	protected void InitializeWeaponPrefabs()
	{
		m_mWeaponPrefabs.Set("makarov", "{3E413771E1834D2E}Prefabs/Weapons/Handguns/PM/Weapon_PM.et");
		m_mWeaponPrefabs.Set("akm", "{1BC151E0D4DE3D99}Prefabs/Weapons/Rifles/AKM/Weapon_AKM.et");
		m_mWeaponPrefabs.Set("ak74", "{9B3DFBAE74363E7A}Prefabs/Weapons/Rifles/AK74/Weapon_AK74.et");
		m_mWeaponPrefabs.Set("svd", "{3E36C5A1DDAE0CF8}Prefabs/Weapons/Rifles/SVD/Weapon_SVD.et");
		m_mWeaponPrefabs.Set("rpg7", "{519E924C1C8C5FB4}Prefabs/Weapons/Launchers/RPG7/Weapon_RPG7.et");
		m_mWeaponPrefabs.Set("pkm", "{5C3AD3CD9F747118}Prefabs/Weapons/MachineGuns/PKM/Weapon_PKM.et");
		PrintFormat("[RBL_Delivery] Weapon prefabs initialized");
	}
	
	protected void InitializeVehiclePrefabs()
	{
		m_mVehiclePrefabs.Set("uaz", "{5E74787BB083789B}Prefabs/Vehicles/Wheeled/UAZ469/UAZ469.et");
		m_mVehiclePrefabs.Set("ural", "{91B01E6C0D20E1D1}Prefabs/Vehicles/Wheeled/Ural4320/Ural4320.et");
		m_mVehiclePrefabs.Set("btr70", "{D85A504DF4E2C128}Prefabs/Vehicles/Wheeled/BTR70/BTR70.et");
		m_mVehiclePrefabs.Set("bmp1", "{BB0E7CE42F0F3E19}Prefabs/Vehicles/Tracked/BMP1/BMP1.et");
		PrintFormat("[RBL_Delivery] Vehicle prefabs initialized");
	}
	
	protected void InitializeRecruitPrefabs()
	{
		// NOTE: These prefabs need to match your game installation
		// You can find valid prefabs in Workbench under:
		// Prefabs/Characters/Factions/BLUFOR/US_Army/
		// or Prefabs/Characters/Factions/INDFOR/FIA/ (if using FIA mod)
		
		// Vanilla Arma Reforger US Army characters
		// Format: {GUID}Path/To/Prefab.et
		// Get GUIDs from Workbench by right-clicking prefab -> Copy Resource Name
		
		// Default fallback prefabs - US Army (vanilla)
		string usRifleman = "{DCB41E1B7E697DE8}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Rifleman.et";
		
		m_mRecruitPrefabs.Set("rifleman", usRifleman);
		m_mRecruitPrefabs.Set("medic", usRifleman);    // Fallback to rifleman
		m_mRecruitPrefabs.Set("mg", usRifleman);       // Fallback to rifleman  
		m_mRecruitPrefabs.Set("at", usRifleman);       // Fallback to rifleman
		m_mRecruitPrefabs.Set("sniper", usRifleman);   // Fallback to rifleman
		m_mRecruitPrefabs.Set("squad", usRifleman);    // Squad uses riflemen
		
		PrintFormat("[RBL_Delivery] Recruit prefabs initialized (using US Army fallback)");
		PrintFormat("[RBL_Delivery] To customize, call SetRecruitPrefab() with valid prefab paths");
	}
	
	// Allow runtime configuration of recruit prefabs
	void SetRecruitPrefab(string recruitType, string prefabPath)
	{
		if (recruitType.IsEmpty() || prefabPath.IsEmpty())
			return;
		
		m_mRecruitPrefabs.Set(recruitType, prefabPath);
		PrintFormat("[RBL_Delivery] Set %1 prefab to: %2", recruitType, prefabPath);
	}
	
	// Get current prefab for a recruit type
	string GetRecruitPrefabPath(string recruitType)
	{
		string prefab;
		if (m_mRecruitPrefabs.Find(recruitType, prefab))
			return prefab;
		return "";
	}
	
	// Get equipment prefab
	string GetEquipmentPrefabPath(string equipmentID)
	{
		string prefab;
		if (m_mEquipmentPrefabs.Find(equipmentID, prefab))
			return prefab;
		return "";
	}
	
	// Get weapon prefab
	string GetWeaponPrefabPath(string weaponID)
	{
		string prefab;
		if (m_mWeaponPrefabs.Find(weaponID, prefab))
			return prefab;
		return "";
	}
	
	// Get vehicle prefab
	string GetVehiclePrefabPath(string vehicleID)
	{
		string prefab;
		if (m_mVehiclePrefabs.Find(vehicleID, prefab))
			return prefab;
		return "";
	}
	
	// Set custom prefabs at runtime
	void SetEquipmentPrefab(string equipmentID, string prefabPath)
	{
		m_mEquipmentPrefabs.Set(equipmentID, prefabPath);
	}
	
	void SetWeaponPrefab(string weaponID, string prefabPath)
	{
		m_mWeaponPrefabs.Set(weaponID, prefabPath);
	}
	
	void SetVehiclePrefab(string vehicleID, string prefabPath)
	{
		m_mVehiclePrefabs.Set(vehicleID, prefabPath);
	}
	
	// Statistics
	bool IsInitialized() { return m_bInitialized; }
	int GetTotalDeliveries() { return m_iDeliveriesTotal; }
	int GetSuccessfulDeliveries() { return m_iDeliveriesSuccessful; }
	int GetFailedDeliveries() { return m_iDeliveriesFailed; }
	
	float GetSuccessRate()
	{
		if (m_iDeliveriesTotal == 0)
			return 100.0;
		return (m_iDeliveriesSuccessful / m_iDeliveriesTotal) * 100.0;
	}
	
	// ========================================================================
	// MAIN DELIVERY METHODS
	// ========================================================================
	
	ERBLDeliveryResult DeliverItem(RBL_ShopItem item, int playerID)
	{
		// Ensure initialized
		if (!m_bInitialized)
			Initialize();
		
		m_iDeliveriesTotal++;
		
		if (!item)
		{
			m_iDeliveriesFailed++;
			return ERBLDeliveryResult.FAILED_NO_PREFAB;
		}
		
		IEntity playerEntity = GetPlayerEntity(playerID);
		if (!playerEntity)
		{
			m_iDeliveriesFailed++;
			return ERBLDeliveryResult.FAILED_NO_PLAYER;
		}
		
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
		
		// Track statistics
		if (result == ERBLDeliveryResult.SUCCESS)
			m_iDeliveriesSuccessful++;
		else
			m_iDeliveriesFailed++;
		
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
			
			// Show category-specific notifications
			NotifyDeliverySuccess(item.DisplayName, item.Category);
		}
		else
		{
			m_OnDeliveryFailed.Invoke(eventData);
			PrintFormat("[RBL_Delivery] Failed to deliver %1: %2", item.DisplayName, typename.EnumToString(ERBLDeliveryResult, result));
			
			// Show failure notification
			RBL_Notifications.DeliveryFailed(item.DisplayName);
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
		{
			PrintFormat("[RBL_Delivery] Recruit has no AIControlComponent");
			return;
		}
		
		AIAgent recruitAgent = aiControl.GetAIAgent();
		if (!recruitAgent)
		{
			PrintFormat("[RBL_Delivery] Recruit has no AIAgent");
			return;
		}
		
		// Try multiple methods to add to player's group
		bool addedToGroup = false;
		
		// Method 1: Try SCR_PlayerControllerGroupComponent
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			SCR_PlayerControllerGroupComponent groupComp = SCR_PlayerControllerGroupComponent.Cast(
				pc.FindComponent(SCR_PlayerControllerGroupComponent));
			
			if (groupComp)
			{
				SCR_AIGroup playerGroup = groupComp.GetPlayersGroup();
				if (playerGroup)
				{
					playerGroup.AddAgent(recruitAgent);
					addedToGroup = true;
					PrintFormat("[RBL_Delivery] Recruit added via SCR_PlayerControllerGroupComponent");
				}
			}
		}
		
		// Method 2: Try to find/create AI group
		if (!addedToGroup)
		{
			AIGroup group = GetOrCreatePlayerAIGroup(playerEntity);
			if (group)
			{
				group.AddAgent(recruitAgent);
				addedToGroup = true;
				PrintFormat("[RBL_Delivery] Recruit added to created AI group");
			}
		}
		
		// Method 3: Create follow waypoint as fallback
		if (!addedToGroup)
		{
			CreateFollowWaypoint(recruitEntity, playerEntity);
			PrintFormat("[RBL_Delivery] Recruit set to follow player via waypoint");
		}
		
		// Set recruit faction to FIA
		SetRecruitFaction(recruitEntity);
	}
	
	// Get or create an AI group for the player
	protected AIGroup GetOrCreatePlayerAIGroup(IEntity playerEntity)
	{
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (!aiWorld)
			return null;
		
		// Create a new group
		AIGroup group = aiWorld.CreateGroup();
		return group;
	}
	
	// Create a waypoint for recruit to follow player
	protected void CreateFollowWaypoint(IEntity recruitEntity, IEntity playerEntity)
	{
		if (!recruitEntity || !playerEntity)
			return;
		
		AIControlComponent aiControl = AIControlComponent.Cast(recruitEntity.FindComponent(AIControlComponent));
		if (!aiControl)
			return;
		
		AIAgent agent = aiControl.GetAIAgent();
		if (!agent)
			return;
		
		// Get agent's group
		AIGroup group = agent.GetParentGroup();
		if (!group)
		{
			// Create group for this agent
			AIWorld aiWorld = GetGame().GetAIWorld();
			if (aiWorld)
			{
				group = aiWorld.CreateGroup();
				group.AddAgent(agent);
			}
		}
		
		if (!group)
			return;
		
		// Create follow waypoint
		BaseWorld world = GetGame().GetWorld();
		vector playerPos = playerEntity.GetOrigin();
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = playerPos;
		
		string wpPrefab = "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
		Resource resource = Resource.Load(wpPrefab);
		if (resource.IsValid())
		{
			IEntity wpEntity = GetGame().SpawnEntityPrefab(resource, world, params);
			AIWaypoint wp = AIWaypoint.Cast(wpEntity);
			if (wp)
			{
				group.AddWaypoint(wp);
			}
		}
	}
	
	// Set recruit to FIA faction
	protected void SetRecruitFaction(IEntity recruitEntity)
	{
		if (!recruitEntity)
			return;
		
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(
			recruitEntity.FindComponent(FactionAffiliationComponent));
		
		if (factionComp)
		{
			Faction fiaFaction = GetGame().GetFactionManager().GetFactionByKey("FIA");
			if (fiaFaction)
			{
				factionComp.SetAffiliatedFaction(fiaFaction);
			}
		}
	}
	
	// ========================================================================
	// PUBLIC API
	// ========================================================================
	
	ScriptInvoker GetOnItemDelivered() { return m_OnItemDelivered; }
	ScriptInvoker GetOnDeliveryFailed() { return m_OnDeliveryFailed; }
	
	// ========================================================================
	// NOTIFICATIONS
	// ========================================================================
	
	// Show notification for successful delivery
	void NotifyDeliverySuccess(string itemName, string category)
	{
		// Use RBL_Notifications for consistent UI
		if (category == "Weapons")
		{
			RBL_Notifications.ItemDelivered(itemName);
		}
		else if (category == "Equipment")
		{
			RBL_Notifications.ItemDelivered(itemName);
		}
		else if (category == "Vehicles")
		{
			RBL_Notifications.VehicleDelivered(itemName);
		}
		else if (category == "Recruits")
		{
			RBL_Notifications.RecruitJoined(itemName);
		}
		else
		{
			RBL_Notifications.ItemDelivered(itemName);
		}
	}
	
	// Show notification for failed delivery
	void NotifyDeliveryFailed(string itemName, ERBLDeliveryResult reason)
	{
		string message = "Delivery failed: " + itemName;
		
		switch (reason)
		{
			case ERBLDeliveryResult.FAILED_NO_PLAYER:
				message += " (No valid player)";
				break;
			case ERBLDeliveryResult.FAILED_NO_PREFAB:
				message += " (Invalid item)";
				break;
			case ERBLDeliveryResult.FAILED_SPAWN_ERROR:
				message += " (Spawn error)";
				break;
			case ERBLDeliveryResult.FAILED_INVENTORY_FULL:
				message += " (Inventory full)";
				break;
			case ERBLDeliveryResult.FAILED_NO_SPACE:
				message += " (No space)";
				break;
		}
		
		ShowNotification(message, false);
	}
	
	// Show on-screen notification
	protected void ShowNotification(string message, bool success)
	{
		// Log to console
		if (success)
			PrintFormat("[RBL_Delivery] SUCCESS: %1", message);
		else
			PrintFormat("[RBL_Delivery] FAILED: %1", message);
		
		// Try to show HUD notification
		SCR_HintUIInfo hint = SCR_HintUIInfo.CreateInfo(message, success ? "Delivery" : "Error", 3.0);
		if (hint)
		{
			SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();
			if (hintMgr)
				hintMgr.Show(hint);
		}
	}
}

// ============================================================================
// DEBUG COMMANDS
// ============================================================================
class RBL_DeliveryCommands
{
	// Test weapon delivery
	static void TestWeapon()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.PurchaseItem("akm");
	}
	
	// Test vehicle delivery
	static void TestVehicle()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.PurchaseItem("uaz");
	}
	
	// Test recruit delivery
	static void TestRecruit()
	{
		RBL_ShopManager shop = RBL_ShopManager.GetInstance();
		if (shop)
			shop.PurchaseItem("rifleman");
	}
	
	// Give player free money for testing
	static void GiveTestMoney()
	{
		RBL_EconomyManager econ = RBL_EconomyManager.GetInstance();
		if (econ)
		{
			econ.AddMoney(10000);
			econ.AddHR(50);
			PrintFormat("[RBL_Delivery] Added $10000 and 50 HR for testing");
		}
	}
	
	// Print delivery system status
	static void PrintStatus()
	{
		RBL_ItemDelivery delivery = RBL_ItemDelivery.GetInstance();
		if (!delivery)
		{
			PrintFormat("[RBL_Delivery] System not initialized");
			return;
		}
		
		IEntity player = delivery.GetLocalPlayerEntity();
		if (player)
		{
			PrintFormat("[RBL_Delivery] Player found at %1", player.GetOrigin().ToString());
			PrintFormat("[RBL_Delivery] Valid target: %1", delivery.IsValidDeliveryTarget(player));
		}
		else
		{
			PrintFormat("[RBL_Delivery] No local player found");
		}
	}
}
