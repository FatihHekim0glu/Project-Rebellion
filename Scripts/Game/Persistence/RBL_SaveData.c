// ============================================================================
// PROJECT REBELLION - Save Data Structures
// JSON-serializable data containers for persistence
// ============================================================================

// Save file format version - increment on breaking changes
const int RBL_SAVE_VERSION = 1;
const string RBL_SAVE_MAGIC = "RBL_SAVE";

// ============================================================================
// ROOT SAVE DATA - Contains everything
// ============================================================================
class RBL_SaveData
{
	// Header
	string m_sMagic;
	int m_iVersion;
	string m_sSaveTime;
	string m_sPlayerId;
	string m_sWorldName;
	
	// Campaign state
	ref RBL_CampaignSaveData m_Campaign;
	
	// Zone states
	ref array<ref RBL_ZoneSaveData> m_aZones;
	
	// Economy state
	ref RBL_EconomySaveData m_Economy;
	
	// Player data
	ref array<ref RBL_PlayerSaveData> m_aPlayers;
	
	// AI state
	ref RBL_CommanderSaveData m_Commander;
	
	// Active missions
	ref array<ref RBL_MissionSaveData> m_aMissions;
	
	void RBL_SaveData()
	{
		m_sMagic = RBL_SAVE_MAGIC;
		m_iVersion = RBL_SAVE_VERSION;
		m_sSaveTime = "";
		m_sPlayerId = "";
		m_sWorldName = "";
		
		m_Campaign = new RBL_CampaignSaveData();
		m_aZones = new array<ref RBL_ZoneSaveData>();
		m_Economy = new RBL_EconomySaveData();
		m_aPlayers = new array<ref RBL_PlayerSaveData>();
		m_Commander = new RBL_CommanderSaveData();
		m_aMissions = new array<ref RBL_MissionSaveData>();
	}
	
	bool IsValid()
	{
		return m_sMagic == RBL_SAVE_MAGIC && m_iVersion > 0;
	}
	
	bool NeedsMigration()
	{
		return m_iVersion < RBL_SAVE_VERSION;
	}
}

// ============================================================================
// CAMPAIGN STATE - Global campaign progress
// ============================================================================
class RBL_CampaignSaveData
{
	// Core stats
	int m_iWarLevel;
	int m_iAggression;
	int m_iCivilianSupport;
	
	// Time tracking
	float m_fTotalPlayTime;
	int m_iGameDays;
	float m_fCurrentDayTime;
	
	// Alert state
	int m_iAlertState;
	float m_fAlertDecayTimer;
	
	// Victory conditions
	int m_iZonesCaptured;
	int m_iZonesLost;
	int m_iEnemiesKilled;
	int m_iFriendliesLost;
	
	// Timestamps
	string m_sStartDate;
	string m_sLastPlayDate;
	
	void RBL_CampaignSaveData()
	{
		m_iWarLevel = 1;
		m_iAggression = 0;
		m_iCivilianSupport = 50;
		m_fTotalPlayTime = 0;
		m_iGameDays = 0;
		m_fCurrentDayTime = 0;
		m_iAlertState = 0;
		m_fAlertDecayTimer = 0;
		m_iZonesCaptured = 0;
		m_iZonesLost = 0;
		m_iEnemiesKilled = 0;
		m_iFriendliesLost = 0;
		m_sStartDate = "";
		m_sLastPlayDate = "";
	}
}

// ============================================================================
// ZONE STATE - Individual zone data
// ============================================================================
class RBL_ZoneSaveData
{
	// Identity
	string m_sZoneID;
	string m_sZoneName;
	int m_iZoneType;
	
	// Position (for validation)
	vector m_vPosition;
	
	// Ownership
	int m_iOwnerFaction;
	int m_iPreviousOwner;
	
	// Garrison
	int m_iGarrisonStrength;
	int m_iMaxGarrison;
	ref array<string> m_aGarrisonUnitTypes;
	
	// Support
	int m_iSupportLevel;
	int m_iSupportTrend;
	
	// Capture state
	float m_fCaptureProgress;
	int m_iCapturingFaction;
	
	// Resources
	int m_iResourceOutput;
	float m_fResourceTimer;
	
	// Status flags
	bool m_bIsUnderAttack;
	bool m_bIsContested;
	bool m_bIsDestroyed;
	
	// Timestamps
	float m_fLastCaptureTime;
	float m_fLastAttackTime;
	
	void RBL_ZoneSaveData()
	{
		m_sZoneID = "";
		m_sZoneName = "";
		m_iZoneType = 0;
		m_vPosition = vector.Zero;
		m_iOwnerFaction = 0;
		m_iPreviousOwner = 0;
		m_iGarrisonStrength = 0;
		m_iMaxGarrison = 0;
		m_aGarrisonUnitTypes = new array<string>();
		m_iSupportLevel = 50;
		m_iSupportTrend = 0;
		m_fCaptureProgress = 0;
		m_iCapturingFaction = -1;
		m_iResourceOutput = 0;
		m_fResourceTimer = 0;
		m_bIsUnderAttack = false;
		m_bIsContested = false;
		m_bIsDestroyed = false;
		m_fLastCaptureTime = 0;
		m_fLastAttackTime = 0;
	}
}

// ============================================================================
// ECONOMY STATE - Faction resources and arsenal
// ============================================================================
class RBL_EconomySaveData
{
	// FIA resources
	int m_iFIAMoney;
	int m_iFIAHumanResources;
	int m_iFIAFuel;
	int m_iFIAAmmo;
	
	// Income rates
	int m_iMoneyPerMinute;
	int m_iHRPerMinute;
	
	// Arsenal items (itemID -> quantity)
	ref array<ref RBL_ArsenalItemSave> m_aArsenalItems;
	
	// Unlocked items
	ref array<string> m_aUnlockedItems;
	
	// Vehicle pool
	ref array<ref RBL_VehicleSave> m_aVehiclePool;
	
	// Trade history
	int m_iTotalMoneyEarned;
	int m_iTotalMoneySpent;
	int m_iTotalItemsBought;
	int m_iTotalItemsSold;
	
	void RBL_EconomySaveData()
	{
		m_iFIAMoney = 500;
		m_iFIAHumanResources = 10;
		m_iFIAFuel = 100;
		m_iFIAAmmo = 100;
		m_iMoneyPerMinute = 0;
		m_iHRPerMinute = 0;
		m_aArsenalItems = new array<ref RBL_ArsenalItemSave>();
		m_aUnlockedItems = new array<string>();
		m_aVehiclePool = new array<ref RBL_VehicleSave>();
		m_iTotalMoneyEarned = 0;
		m_iTotalMoneySpent = 0;
		m_iTotalItemsBought = 0;
		m_iTotalItemsSold = 0;
	}
}

class RBL_ArsenalItemSave
{
	string m_sItemID;
	int m_iQuantity;
	
	void RBL_ArsenalItemSave(string itemID, int quantity)
	{
		m_sItemID = itemID;
		m_iQuantity = quantity;
	}
}

class RBL_VehicleSave
{
	string m_sVehicleID;
	string m_sVehicleType;
	float m_fFuel;
	float m_fHealth;
	bool m_bIsDeployed;
	vector m_vPosition;
	
	void RBL_VehicleSave()
	{
		m_sVehicleID = "";
		m_sVehicleType = "";
		m_fFuel = 100;
		m_fHealth = 100;
		m_bIsDeployed = false;
		m_vPosition = vector.Zero;
	}
}

// ============================================================================
// PLAYER STATE - Individual player data
// ============================================================================
class RBL_PlayerSaveData
{
	// Identity
	string m_sPlayerUID;
	string m_sPlayerName;
	
	// Resources
	int m_iMoney;
	int m_iHumanResources;
	
	// Stats
	int m_iKills;
	int m_iDeaths;
	int m_iZonesCaptured;
	int m_iMissionsCompleted;
	float m_fPlayTime;
	
	// Progression
	int m_iRank;
	int m_iExperience;
	ref array<string> m_aUnlocks;
	ref array<string> m_aAchievements;
	
	// Inventory snapshot
	ref array<string> m_aInventoryItems;
	
	// Position (for reconnect)
	vector m_vLastPosition;
	float m_fLastDirection;
	
	// Undercover state
	int m_iCoverStatus;
	float m_fSuspicionLevel;
	
	void RBL_PlayerSaveData()
	{
		m_sPlayerUID = "";
		m_sPlayerName = "";
		m_iMoney = 0;
		m_iHumanResources = 0;
		m_iKills = 0;
		m_iDeaths = 0;
		m_iZonesCaptured = 0;
		m_iMissionsCompleted = 0;
		m_fPlayTime = 0;
		m_iRank = 0;
		m_iExperience = 0;
		m_aUnlocks = new array<string>();
		m_aAchievements = new array<string>();
		m_aInventoryItems = new array<string>();
		m_vLastPosition = vector.Zero;
		m_fLastDirection = 0;
		m_iCoverStatus = 0;
		m_fSuspicionLevel = 0;
	}
}

// ============================================================================
// COMMANDER AI STATE - Enemy AI state
// ============================================================================
class RBL_CommanderSaveData
{
	// Resources
	int m_iCommanderResources;
	int m_iReinforcementPool;
	
	// Threat assessment
	int m_iThreatLevel;
	string m_sHighestThreatZone;
	
	// Active operations
	ref array<ref RBL_OperationSave> m_aActiveOperations;
	
	// Cooldowns
	float m_fQRFCooldown;
	float m_fPatrolCooldown;
	float m_fOffensiveCooldown;
	
	// Memory
	ref array<string> m_aKnownPlayerPositions;
	ref array<string> m_aRecentlyAttackedZones;
	
	void RBL_CommanderSaveData()
	{
		m_iCommanderResources = 100;
		m_iReinforcementPool = 50;
		m_iThreatLevel = 0;
		m_sHighestThreatZone = "";
		m_aActiveOperations = new array<ref RBL_OperationSave>();
		m_fQRFCooldown = 0;
		m_fPatrolCooldown = 0;
		m_fOffensiveCooldown = 0;
		m_aKnownPlayerPositions = new array<string>();
		m_aRecentlyAttackedZones = new array<string>();
	}
}

class RBL_OperationSave
{
	string m_sOperationID;
	int m_iOperationType;
	string m_sTargetZone;
	int m_iAssignedUnits;
	float m_fStartTime;
	float m_fProgress;
	
	void RBL_OperationSave()
	{
		m_sOperationID = "";
		m_iOperationType = 0;
		m_sTargetZone = "";
		m_iAssignedUnits = 0;
		m_fStartTime = 0;
		m_fProgress = 0;
	}
}

// ============================================================================
// MISSION STATE - Active missions
// ============================================================================
class RBL_MissionSaveData
{
	string m_sMissionID;
	string m_sMissionType;
	string m_sMissionName;
	
	// Target
	string m_sTargetZone;
	vector m_vTargetPosition;
	
	// Progress
	float m_fProgress;
	int m_iObjectivesCompleted;
	int m_iTotalObjectives;
	
	// Timing
	float m_fTimeLimit;
	float m_fTimeRemaining;
	float m_fStartTime;
	
	// Rewards
	int m_iMoneyReward;
	int m_iHRReward;
	ref array<string> m_aItemRewards;
	
	// Status
	bool m_bIsActive;
	bool m_bIsCompleted;
	bool m_bIsFailed;
	
	void RBL_MissionSaveData()
	{
		m_sMissionID = "";
		m_sMissionType = "";
		m_sMissionName = "";
		m_sTargetZone = "";
		m_vTargetPosition = vector.Zero;
		m_fProgress = 0;
		m_iObjectivesCompleted = 0;
		m_iTotalObjectives = 0;
		m_fTimeLimit = 0;
		m_fTimeRemaining = 0;
		m_fStartTime = 0;
		m_iMoneyReward = 0;
		m_iHRReward = 0;
		m_aItemRewards = new array<string>();
		m_bIsActive = true;
		m_bIsCompleted = false;
		m_bIsFailed = false;
	}
}

// ============================================================================
// SAVE SLOT METADATA - For save slot UI
// ============================================================================
class RBL_SaveSlotInfo
{
	int m_iSlotIndex;
	string m_sSlotName;
	string m_sSaveTime;
	string m_sWorldName;
	float m_fPlayTime;
	int m_iWarLevel;
	int m_iZonesControlled;
	int m_iTotalZones;
	bool m_bIsEmpty;
	bool m_bIsCorrupted;
	
	void RBL_SaveSlotInfo(int slotIndex)
	{
		m_iSlotIndex = slotIndex;
		m_sSlotName = "Empty Slot";
		m_sSaveTime = "";
		m_sWorldName = "";
		m_fPlayTime = 0;
		m_iWarLevel = 0;
		m_iZonesControlled = 0;
		m_iTotalZones = 0;
		m_bIsEmpty = true;
		m_bIsCorrupted = false;
	}
	
	string GetDisplayText()
	{
		if (m_bIsEmpty)
			return "- Empty Slot -";
		
		if (m_bIsCorrupted)
			return "! Corrupted Save !";
		
		string playTimeStr = FormatPlayTime(m_fPlayTime);
		return string.Format("%1 - WL%2 - %3/%4 zones - %5", 
			m_sSlotName, m_iWarLevel, m_iZonesControlled, m_iTotalZones, playTimeStr);
	}
	
	protected string FormatPlayTime(float seconds)
	{
		int hours = seconds / 3600;
		int mins = (seconds % 3600) / 60;
		return string.Format("%1h %2m", hours, mins);
	}
}

