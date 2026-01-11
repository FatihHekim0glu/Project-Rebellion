// ============================================================================
// PROJECT REBELLION - Configuration Constants
// Central configuration for tweaking gameplay balance
// ============================================================================

class RBL_Config
{
	// ========================================================================
	// CAPTURE MECHANICS
	// ========================================================================
	
	static const float CAPTURE_BASE_RATE = 1.0;           // % per second base
	static const float CAPTURE_UNIT_BONUS = 0.5;          // Additional % per attacker
	static const float CAPTURE_MAX_RATE = 5.0;            // Maximum % per second
	static const float CAPTURE_DECAY_RATE = 2.0;          // Progress decay when contested
	
	// ========================================================================
	// AGGRESSION
	// ========================================================================
	
	static const float AGGRESSION_DECAY_PER_MINUTE = 0.5;
	static const int AGGRESSION_PER_KILL = 2;
	static const int AGGRESSION_PER_VEHICLE_KILL = 5;
	static const int AGGRESSION_PER_ZONE_CAPTURE = 15;
	static const int AGGRESSION_PER_FAILED_QRF = -10;
	
	// ========================================================================
	// WAR LEVEL THRESHOLDS
	// (Score = Kills + ZonesCaptured * 10)
	// ========================================================================
	
	static const int WAR_LEVEL_2_THRESHOLD = 20;
	static const int WAR_LEVEL_3_THRESHOLD = 50;
	static const int WAR_LEVEL_4_THRESHOLD = 100;
	static const int WAR_LEVEL_5_THRESHOLD = 175;
	static const int WAR_LEVEL_6_THRESHOLD = 275;
	static const int WAR_LEVEL_7_THRESHOLD = 400;
	static const int WAR_LEVEL_8_THRESHOLD = 550;
	static const int WAR_LEVEL_9_THRESHOLD = 750;
	static const int WAR_LEVEL_10_THRESHOLD = 1000;
	
	// ========================================================================
	// QRF COSTS (AI Resource Pool)
	// ========================================================================
	
	static const int QRF_COST_PATROL = 50;
	static const int QRF_COST_INFANTRY = 100;
	static const int QRF_COST_CONVOY = 150;
	static const int QRF_COST_MECHANIZED = 300;
	static const int QRF_COST_SPECOPS = 400;
	static const int QRF_COST_HELICOPTER = 500;
	
	// ========================================================================
	// QRF TIMING
	// ========================================================================
	
	static const float QRF_COOLDOWN_SECONDS = 180.0;      // 3 minutes
	static const int QRF_MAX_CONCURRENT = 3;
	static const float QRF_DECISION_INTERVAL = 30.0;      // AI thinks every 30s
	
	// ========================================================================
	// ECONOMY - INCOME
	// ========================================================================
	
	static const float RESOURCE_TICK_INTERVAL = 600.0;    // 10 minutes
	
	static const int INCOME_RESOURCE_BASE = 100;
	static const int INCOME_FACTORY_BASE = 150;
	static const int INCOME_SEAPORT_BASE = 125;
	static const int INCOME_AIRBASE_BASE = 75;
	static const int INCOME_TOWN_BASE = 50;
	static const int INCOME_OUTPOST_BASE = 25;
	
	// Support multiplier ranges from 0.5x (0% support) to 1.5x (100% support)
	static const float INCOME_SUPPORT_MIN_MULT = 0.5;
	static const float INCOME_SUPPORT_MAX_MULT = 1.5;
	
	// ========================================================================
	// ECONOMY - ARSENAL
	// ========================================================================
	
	static const int ARSENAL_UNLOCK_THRESHOLD = 25;       // Items for unlimited
	
	// ========================================================================
	// ECONOMY - RECRUITMENT
	// ========================================================================
	
	static const int RECRUIT_HR_COST = 1;
	static const int RECRUIT_MONEY_COST = 50;
	
	// ========================================================================
	// UNDERCOVER DETECTION
	// ========================================================================
	
	static const float UNDERCOVER_CHECK_INTERVAL = 1.0;
	static const float UNDERCOVER_DETECTION_RADIUS = 50.0;
	static const float UNDERCOVER_CLOSE_RANGE = 10.0;
	static const float UNDERCOVER_SUSPICION_BUILD = 0.15; // Per second
	static const float UNDERCOVER_SUSPICION_DECAY = 0.05; // Per second
	static const float UNDERCOVER_COMPROMISE_THRESHOLD = 1.0;
	
	// Threat values for different factors
	static const float THREAT_WEAPON_IN_HANDS = 0.8;
	static const float THREAT_MILITARY_UNIFORM = 0.7;
	static const float THREAT_TACTICAL_VEST = 0.5;
	static const float THREAT_MILITARY_VEHICLE = 1.0;
	static const float THREAT_CLOSE_INSPECTION = 0.6;
	static const float THREAT_BEING_WATCHED = 0.9;
	
	// ========================================================================
	// PERSISTENCE
	// ========================================================================
	
	static const float AUTOSAVE_INTERVAL = 300.0;         // 5 minutes
	static const string SAVE_FILE_PREFIX = "RBL_Campaign";
	
	// ========================================================================
	// ZONE STRATEGIC VALUES
	// ========================================================================
	
	static const int STRATEGIC_VALUE_HQ = 1000;
	static const int STRATEGIC_VALUE_AIRBASE = 500;
	static const int STRATEGIC_VALUE_FACTORY = 300;
	static const int STRATEGIC_VALUE_SEAPORT = 250;
	static const int STRATEGIC_VALUE_RESOURCE = 200;
	static const int STRATEGIC_VALUE_TOWN = 150;
	static const int STRATEGIC_VALUE_OUTPOST = 100;
	static const int STRATEGIC_VALUE_ROADBLOCK = 50;
	
	// ========================================================================
	// AI COMMANDER
	// ========================================================================
	
	static const float AI_INTEL_DECAY_RATE = 0.01;        // Per second
	static const int AI_STARTING_RESOURCES = 1000;
	static const int AI_MAX_RESOURCES = 10000;
	
	// Response thresholds (100 - Aggression = threshold)
	// Higher aggression = lower threshold = faster response
	
	// ========================================================================
	// DISTANCES
	// ========================================================================
	
	static const float DISTANCE_CLOSE = 1000.0;           // < 1km
	static const float DISTANCE_MEDIUM = 3000.0;          // < 3km
	// >= 3km is considered far
	
	// ========================================================================
	// STARTING VALUES
	// ========================================================================
	
	static const int STARTING_WAR_LEVEL = 1;
	static const int STARTING_AGGRESSION = 25;
	static const int STARTING_MONEY = 500;
	static const int STARTING_HR = 10;
}

