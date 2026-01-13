// ============================================================================
// PROJECT REBELLION - Enumerations
// ============================================================================

enum ERBLZoneType
{
	Unknown = 0,
	Town,
	Outpost,
	Airbase,
	HQ,
	Factory,
	Resource,
	Seaport,
	Roadblock,
	Watchpost
}

enum ERBLFactionKey
{
	NONE = 0,
	FIA,
	USSR,
	US,
	CIVILIAN
}

enum ERBLAlertState
{
	RELAXED = 0,
	AWARE,
	ALERTED,
	COMBAT
}

enum ERBLQRFType
{
	PATROL = 0,
	CONVOY,
	INFANTRY,
	MECHANIZED,
	HELICOPTER,
	SPECOPS
}

enum ERBLItemAvailability
{
	LOCKED = 0,
	LIMITED,
	UNLIMITED
}

enum ERBLCampaignEvent
{
	GAME_START = 0,
	GAME_END,
	GAME_LOADED,
	GAME_SAVED,
	ZONE_CAPTURED,
	ZONE_LOST,
	QRF_LAUNCHED,
	QRF_ARRIVED,
	WAR_LEVEL_CHANGED,
	PLAYER_KILLED,
	ENEMY_KILLED,
	RESOURCE_TICK,
	SAVE_COMPLETE,
	LOAD_COMPLETE
}

enum ERBLCoverStatus
{
	HIDDEN = 0,
	SUSPICIOUS,
	SPOTTED,
	COMPROMISED,
	HOSTILE
}

enum ERBLMissionType
{
	NONE = 0,
	ATTACK,
	DEFEND,
	PATROL,
	RESUPPLY,
	REINFORCE,
	AMBUSH,
	SABOTAGE,
	RESCUE,
	ASSASSINATION,
	INTEL
}

enum ERBLMissionStatus
{
	AVAILABLE = 0,
	ACTIVE,
	COMPLETED,
	FAILED,
	EXPIRED
}

enum ERBLMissionDifficulty
{
	EASY = 0,
	MEDIUM,
	HARD,
	EXTREME
}

enum ERBLObjectiveType
{
	CAPTURE_ZONE = 0,
	DEFEND_ZONE,
	KILL_TARGET,
	KILL_COUNT,
	REACH_LOCATION,
	SURVIVE_TIME,
	COLLECT_ITEM,
	DELIVER_ITEM,
	ESCORT_UNIT,
	DESTROY_TARGET
}

enum ERBLGarrisonState
{
	EMPTY = 0,
	PARTIAL,
	FULL,
	REINFORCED
}

enum ERBLResourceType
{
	MONEY = 0,
	HUMAN_RESOURCES,
	AMMUNITION,
	FUEL,
	WEAPONS,
	VEHICLES
}

enum ERBLVehicleClass
{
	CIVILIAN = 0,
	TRANSPORT,
	ARMED,
	ARMORED,
	HELICOPTER,
	BOAT
}
