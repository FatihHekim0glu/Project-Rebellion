// ============================================================================
// PROJECT REBELLION - Core Enumerations
// ============================================================================

// Zone classification determining behavior, income, and strategic value
enum ERBLZoneType
{
	HQ,           // Player headquarters - lose condition if captured
	Outpost,      // Military garrison point
	Resource,     // Generates supplies/income
	Factory,      // Produces vehicles/equipment
	Airbase,      // Air asset spawn point, high strategic value
	Town,         // Civilian population center - affects support
	Seaport,      // Naval assets and supply routes
	Roadblock     // Minor checkpoint
}

// Faction identifiers matching Reforger's faction system
enum ERBLFactionKey
{
	NONE,         // Neutral/Unclaimed
	US,           // Occupier faction (NATO/US)
	USSR,         // Occupier faction (Soviet)
	FIA,          // Player resistance faction
	CIVILIAN      // Non-combatant
}

// War Level tiers determining available assets
enum ERBLWarLevel
{
	TIER_1 = 1,   // Militia: Trucks, basic rifles
	TIER_2,       // Light Infantry: APCs, LMGs
	TIER_3,       // Mechanized: IFVs, AT weapons
	TIER_4,       // Motorized Assault: Armed vehicles
	TIER_5,       // Heavy Mechanized: Tanks appear
	TIER_6,       // Air Support: Transport helis
	TIER_7,       // Combined Arms: Attack helis
	TIER_8,       // Full Mobilization: Heavy armor
	TIER_9,       // Total War: All conventional assets
	TIER_10       // Scorched Earth: SpecOps, unlimited
}

// QRF response types based on situation
enum ERBLQRFType
{
	PATROL,       // Light patrol response
	CONVOY,       // Ground vehicle convoy
	INFANTRY,     // Infantry reinforcement
	MECHANIZED,   // Armored response
	HELICOPTER,   // Air mobile QRF
	SPECOPS       // Elite counter-insurgency
}

// Alert states for zones and AI behavior
enum ERBLAlertState
{
	RELAXED,      // Normal patrols, low vigilance
	AWARE,        // Increased patrols
	ALERTED,      // Active search patterns
	COMBAT,       // Full combat engagement
	LOCKDOWN      // Maximum security response
}

// Undercover detection threat levels
enum ERBLCoverStatus
{
	HIDDEN,       // Fully undercover
	SUSPICIOUS,   // Being watched
	COMPROMISED,  // Cover blown, combat imminent
	HOSTILE       // Known enemy, shoot on sight
}

// Arsenal item availability states
enum ERBLItemAvailability
{
	LOCKED,       // Not yet discovered
	LIMITED,      // Available but consumes stock
	UNLIMITED     // Fully unlocked (count >= threshold)
}

// Campaign event types for logging and triggers
enum ERBLCampaignEvent
{
	ZONE_CAPTURED,
	ZONE_LOST,
	QRF_DISPATCHED,
	QRF_DESTROYED,
	PLAYER_KILLED,
	ENEMY_KILLED,
	ITEM_DEPOSITED,
	ITEM_UNLOCKED,
	WAR_LEVEL_CHANGED,
	SAVE_TRIGGERED,
	LOAD_TRIGGERED
}

