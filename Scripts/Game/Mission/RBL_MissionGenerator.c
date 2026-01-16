// ============================================================================
// PROJECT REBELLION - Mission Generator
// Generates context-aware missions based on game state
// ============================================================================

class RBL_MissionGenerator
{
	// Mission ID counter for uniqueness
	protected static int s_iMissionCounter = 0;
	
	// ========================================================================
	// RANDOM MISSION GENERATION
	// ========================================================================
	
	static RBL_Mission GenerateRandomMission()
	{
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		int warLevel = 1;
		if (campMgr)
			warLevel = campMgr.GetWarLevel();
		
		// Weight mission types based on game state
		array<ERBLMissionType> possibleTypes = GetAvailableMissionTypes(warLevel);
		
		if (possibleTypes.Count() == 0)
			possibleTypes.Insert(ERBLMissionType.PATROL);
		
		int typeIndex = Math.RandomInt(0, possibleTypes.Count());
		ERBLMissionType selectedType = possibleTypes[typeIndex];
		
		return GenerateMissionByType(selectedType, warLevel);
	}
	
	static RBL_Mission GenerateMissionByType(ERBLMissionType type, int warLevel)
	{
		switch (type)
		{
			case ERBLMissionType.ATTACK:
				return GenerateAttackMission(warLevel);
			case ERBLMissionType.DEFEND:
				return GenerateDefendMission(warLevel);
			case ERBLMissionType.PATROL:
				return GeneratePatrolMission(warLevel);
			case ERBLMissionType.AMBUSH:
				return GenerateAmbushMission(warLevel);
			case ERBLMissionType.SABOTAGE:
				return GenerateSabotageMission(warLevel);
			case ERBLMissionType.INTEL:
				return GenerateIntelMission(warLevel);
		}
		
		return GeneratePatrolMission(warLevel);
	}
	
	protected static array<ERBLMissionType> GetAvailableMissionTypes(int warLevel)
	{
		array<ERBLMissionType> types = new array<ERBLMissionType>();
		
		// Always available
		types.Insert(ERBLMissionType.PATROL);
		types.Insert(ERBLMissionType.ATTACK);
		
		// War level 2+
		if (warLevel >= 2)
		{
			types.Insert(ERBLMissionType.DEFEND);
			types.Insert(ERBLMissionType.AMBUSH);
		}
		
		// War level 3+
		if (warLevel >= 3)
		{
			types.Insert(ERBLMissionType.SABOTAGE);
			types.Insert(ERBLMissionType.INTEL);
		}
		
		// War level 5+
		if (warLevel >= 5)
		{
			types.Insert(ERBLMissionType.ASSASSINATION);
			types.Insert(ERBLMissionType.RESCUE);
		}
		
		return types;
	}
	
	// ========================================================================
	// ATTACK MISSION
	// ========================================================================
	
	static RBL_Mission GenerateAttackMission(int warLevel)
	{
		s_iMissionCounter++;
		
		// Find enemy zone to attack
		RBL_VirtualZone targetZone = FindEnemyZone(warLevel);
		if (!targetZone)
			return GeneratePatrolMission(warLevel);
		
		string zoneID = targetZone.GetZoneID();
		string zoneName = targetZone.GetZoneName();
		ERBLZoneType zoneType = targetZone.GetZoneType();
		
		// Create mission
		RBL_Mission mission = new RBL_Mission();
		mission.SetID("mission_attack_" + s_iMissionCounter.ToString());
		mission.SetName("Capture " + zoneName);
		mission.SetDescription("Launch an attack to capture " + zoneName + " from enemy forces.");
		mission.SetBriefing("Intelligence reports indicate " + zoneName + " is a strategic target. Capture and hold the zone to expand our territory.");
		mission.SetType(ERBLMissionType.ATTACK);
		mission.SetDifficulty(CalculateDifficulty(targetZone, warLevel));
		mission.SetTargetZone(zoneID);
		mission.SetMissionArea(targetZone.GetZonePosition(), targetZone.GetCaptureRadius() + 100);
		
		// Time limit based on difficulty
		float timeLimit = GetTimeLimitForZone(zoneType, warLevel);
		if (timeLimit > 0)
			mission.SetTimeLimit(timeLimit);
		
		// Add objective
		RBL_MissionObjective captureObj = RBL_MissionObjective.CreateCaptureZone(
			zoneID, 
			"Capture " + zoneName
		);
		mission.AddObjective(captureObj);
		
		// Optional kill objective
		if (warLevel >= 3)
		{
			int killCount = 5 + warLevel;
			RBL_MissionObjective killObj = RBL_MissionObjective.CreateKillCount(
				killCount,
				"Eliminate " + killCount.ToString() + " enemy soldiers"
			);
			killObj.SetOptional(true);
			mission.AddObjective(killObj);
		}
		
		// Calculate rewards
		int moneyReward = CalculateMoneyReward(zoneType, warLevel);
		int hrReward = CalculateHRReward(zoneType);
		mission.SetReward(RBL_MissionReward.Create(moneyReward, hrReward, 10));
		
		return mission;
	}
	
	// ========================================================================
	// DEFEND MISSION
	// ========================================================================
	
	static RBL_Mission GenerateDefendMission(int warLevel)
	{
		s_iMissionCounter++;
		
		// Find friendly zone to defend
		RBL_VirtualZone targetZone = FindFriendlyZone();
		if (!targetZone)
			return GeneratePatrolMission(warLevel);
		
		string zoneID = targetZone.GetZoneID();
		string zoneName = targetZone.GetZoneName();
		
		// Create mission
		RBL_Mission mission = new RBL_Mission();
		mission.SetID("mission_defend_" + s_iMissionCounter.ToString());
		mission.SetName("Defend " + zoneName);
		mission.SetDescription("Enemy forces are preparing to attack " + zoneName + ". Hold the zone at all costs.");
		mission.SetBriefing("Intelligence suggests an imminent enemy attack on " + zoneName + ". Reinforce the position and repel all attackers.");
		mission.SetType(ERBLMissionType.DEFEND);
		mission.SetDifficulty(CalculateDifficulty(targetZone, warLevel));
		mission.SetTargetZone(zoneID);
		mission.SetMissionArea(targetZone.GetZonePosition(), targetZone.GetCaptureRadius() + 150);
		
		// Defend duration
		float defendTime = 180.0 + (warLevel * 30);
		
		// Add objectives
		RBL_MissionObjective defendObj = RBL_MissionObjective.CreateDefendZone(
			zoneID,
			defendTime,
			"Hold " + zoneName + " for " + (defendTime / 60.0).ToString(1, 0) + " minutes"
		);
		mission.AddObjective(defendObj);
		
		// Kill count objective
		int killCount = 3 + (warLevel * 2);
		RBL_MissionObjective killObj = RBL_MissionObjective.CreateKillCount(
			killCount,
			"Eliminate " + killCount.ToString() + " attackers"
		);
		mission.AddObjective(killObj);
		
		// Rewards
		int moneyReward = 200 + (warLevel * 50);
		int hrReward = 1;
		mission.SetReward(RBL_MissionReward.Create(moneyReward, hrReward, -5));
		
		return mission;
	}
	
	// ========================================================================
	// PATROL MISSION
	// ========================================================================
	
	static RBL_Mission GeneratePatrolMission(int warLevel)
	{
		s_iMissionCounter++;
		
		// Get random waypoints
		array<vector> waypoints = GeneratePatrolWaypoints(3);
		
		RBL_Mission mission = new RBL_Mission();
		mission.SetID("mission_patrol_" + s_iMissionCounter.ToString());
		mission.SetName("Patrol Route " + Math.RandomInt(100, 999).ToString());
		mission.SetDescription("Conduct a patrol through the designated area and report any enemy activity.");
		mission.SetBriefing("Command needs eyes on the ground. Patrol the area and engage any hostiles you encounter.");
		mission.SetType(ERBLMissionType.PATROL);
		mission.SetDifficulty(ERBLMissionDifficulty.EASY);
		mission.SetTimeLimit(900);
		
		// Add reach objectives for each waypoint
		for (int i = 0; i < waypoints.Count(); i++)
		{
			RBL_MissionObjective reachObj = RBL_MissionObjective.CreateReachLocation(
				waypoints[i],
				"Reach waypoint " + (i + 1).ToString()
			);
			mission.AddObjective(reachObj);
		}
		
		// Optional kill objective
		int killCount = 2 + warLevel;
		RBL_MissionObjective killObj = RBL_MissionObjective.CreateKillCount(
			killCount,
			"Eliminate " + killCount.ToString() + " enemies (optional)"
		);
		killObj.SetOptional(true);
		mission.AddObjective(killObj);
		
		// Modest rewards
		mission.SetReward(RBL_MissionReward.Create(100 + (warLevel * 25), 0, 2));
		
		return mission;
	}
	
	// ========================================================================
	// AMBUSH MISSION
	// ========================================================================
	
	static RBL_Mission GenerateAmbushMission(int warLevel)
	{
		s_iMissionCounter++;
		
		// Pick a road/path location
		vector ambushPos = FindAmbushLocation();
		
		RBL_Mission mission = new RBL_Mission();
		mission.SetID("mission_ambush_" + s_iMissionCounter.ToString());
		mission.SetName("Ambush Convoy");
		mission.SetDescription("Set up an ambush position and eliminate the enemy convoy.");
		mission.SetBriefing("Enemy supply convoy detected. Move to the ambush point and destroy the convoy when it arrives.");
		mission.SetType(ERBLMissionType.AMBUSH);
		mission.SetDifficulty(ERBLMissionDifficulty.MEDIUM);
		mission.SetMissionArea(ambushPos, 200);
		mission.SetTimeLimit(600);
		
		// Reach ambush point
		RBL_MissionObjective reachObj = RBL_MissionObjective.CreateReachLocation(
			ambushPos,
			"Reach ambush position"
		);
		mission.AddObjective(reachObj);
		
		// Kill enemies
		int killCount = 5 + warLevel;
		RBL_MissionObjective killObj = RBL_MissionObjective.CreateKillCount(
			killCount,
			"Eliminate " + killCount.ToString() + " convoy troops"
		);
		mission.AddObjective(killObj);
		
		// Survive
		RBL_MissionObjective surviveObj = RBL_MissionObjective.CreateSurviveTime(
			120,
			"Survive enemy reinforcements"
		);
		surviveObj.SetOptional(true);
		mission.AddObjective(surviveObj);
		
		mission.SetReward(RBL_MissionReward.Create(300 + (warLevel * 50), 0, 8));
		
		return mission;
	}
	
	// ========================================================================
	// SABOTAGE MISSION
	// ========================================================================
	
	static RBL_Mission GenerateSabotageMission(int warLevel)
	{
		s_iMissionCounter++;
		
		RBL_VirtualZone targetZone = FindEnemyZone(warLevel);
		if (!targetZone)
			return GeneratePatrolMission(warLevel);
		
		string zoneName = targetZone.GetZoneName();
		vector targetPos = targetZone.GetZonePosition();
		
		RBL_Mission mission = new RBL_Mission();
		mission.SetID("mission_sabotage_" + s_iMissionCounter.ToString());
		mission.SetName("Sabotage " + zoneName);
		mission.SetDescription("Infiltrate and sabotage enemy operations at " + zoneName + ".");
		mission.SetBriefing("Sneak into " + zoneName + ", destroy enemy equipment, and escape without raising the alarm.");
		mission.SetType(ERBLMissionType.SABOTAGE);
		mission.SetDifficulty(ERBLMissionDifficulty.HARD);
		mission.SetTargetZone(targetZone.GetZoneID());
		mission.SetMissionArea(targetPos, 300);
		mission.SetTimeLimit(720);
		
		// Reach target
		RBL_MissionObjective reachObj = RBL_MissionObjective.CreateReachLocation(
			targetPos,
			"Infiltrate " + zoneName
		);
		mission.AddObjective(reachObj);
		
		// Small kill count
		RBL_MissionObjective killObj = RBL_MissionObjective.CreateKillCount(
			3,
			"Eliminate key personnel"
		);
		mission.AddObjective(killObj);
		
		mission.SetReward(RBL_MissionReward.Create(400 + (warLevel * 75), 1, 5));
		
		return mission;
	}
	
	// ========================================================================
	// INTEL MISSION
	// ========================================================================
	
	static RBL_Mission GenerateIntelMission(int warLevel)
	{
		s_iMissionCounter++;
		
		RBL_VirtualZone targetZone = FindEnemyZone(1);
		if (!targetZone)
			return GeneratePatrolMission(warLevel);
		
		string zoneName = targetZone.GetZoneName();
		vector targetPos = targetZone.GetZonePosition();
		
		RBL_Mission mission = new RBL_Mission();
		mission.SetID("mission_intel_" + s_iMissionCounter.ToString());
		mission.SetName("Recon " + zoneName);
		mission.SetDescription("Gather intelligence on enemy positions at " + zoneName + ".");
		mission.SetBriefing("Command needs detailed intel on " + zoneName + ". Get close, observe enemy movements, and return safely.");
		mission.SetType(ERBLMissionType.INTEL);
		mission.SetDifficulty(ERBLMissionDifficulty.EASY);
		mission.SetTargetZone(targetZone.GetZoneID());
		mission.SetMissionArea(targetPos, 400);
		mission.SetTimeLimit(600);
		
		// Reach and observe
		RBL_MissionObjective reachObj = RBL_MissionObjective.CreateReachLocation(
			targetPos + Vector(50, 0, 50),
			"Reach observation point near " + zoneName
		);
		mission.AddObjective(reachObj);
		
		// Survive observation time
		RBL_MissionObjective surviveObj = RBL_MissionObjective.CreateSurviveTime(
			60,
			"Observe enemy activity"
		);
		mission.AddObjective(surviveObj);
		
		mission.SetReward(RBL_MissionReward.Create(150 + (warLevel * 30), 0, 3));
		
		return mission;
	}
	
	// ========================================================================
	// HELPER METHODS
	// ========================================================================
	
	protected static RBL_VirtualZone FindEnemyZone(int maxGarrison)
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return null;
		
		array<ref RBL_VirtualZone> enemyZones = zoneMgr.GetVirtualZonesByFaction(ERBLFactionKey.USSR);
		if (!enemyZones || enemyZones.Count() == 0)
			return null;
		
		// Filter by garrison size
		array<ref RBL_VirtualZone> validZones = new array<ref RBL_VirtualZone>();
		for (int i = 0; i < enemyZones.Count(); i++)
		{
			RBL_VirtualZone zone = enemyZones[i];
			if (zone && zone.GetMaxGarrison() <= maxGarrison * 5)
				validZones.Insert(zone);
		}
		
		if (validZones.Count() == 0)
			return enemyZones[Math.RandomInt(0, enemyZones.Count())];
		
		return validZones[Math.RandomInt(0, validZones.Count())];
	}
	
	protected static RBL_VirtualZone FindFriendlyZone()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return null;
		
		array<ref RBL_VirtualZone> friendlyZones = zoneMgr.GetVirtualZonesByFaction(ERBLFactionKey.FIA);
		if (!friendlyZones || friendlyZones.Count() == 0)
			return null;
		
		return friendlyZones[Math.RandomInt(0, friendlyZones.Count())];
	}
	
	protected static array<vector> GeneratePatrolWaypoints(int count)
	{
		array<vector> waypoints = new array<vector>();
		
		// Get player position as starting reference
		vector basePos = Vector(5000, 0, 5000);
		
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
		{
			IEntity player = pc.GetControlledEntity();
			if (player)
				basePos = player.GetOrigin();
		}
		
		// Generate waypoints in a patrol pattern
		for (int i = 0; i < count; i++)
		{
			float angle = (i / count) * 360.0 * Math.DEG2RAD;
			float dist = 200 + Math.RandomFloat(100, 300);
			
			float x = basePos[0] + Math.Cos(angle) * dist;
			float z = basePos[2] + Math.Sin(angle) * dist;
			
			vector wp = Vector(x, 0, z);
			
			BaseWorld world = GetGame().GetWorld();
			if (world)
				wp[1] = world.GetSurfaceY(wp[0], wp[2]);
			
			waypoints.Insert(wp);
		}
		
		return waypoints;
	}
	
	protected static vector FindAmbushLocation()
	{
		vector basePos = Vector(4500, 0, 5500);
		
		float offsetX = Math.RandomFloat(-500, 500);
		float offsetZ = Math.RandomFloat(-500, 500);
		
		vector pos = basePos + Vector(offsetX, 0, offsetZ);
		
		BaseWorld world = GetGame().GetWorld();
		if (world)
			pos[1] = world.GetSurfaceY(pos[0], pos[2]);
		
		return pos;
	}
	
	protected static ERBLMissionDifficulty CalculateDifficulty(RBL_VirtualZone zone, int warLevel)
	{
		if (!zone)
			return ERBLMissionDifficulty.MEDIUM;
		
		int garrison = zone.GetMaxGarrison();
		int score = garrison + (warLevel * 2);
		
		if (score <= 10)
			return ERBLMissionDifficulty.EASY;
		if (score <= 20)
			return ERBLMissionDifficulty.MEDIUM;
		if (score <= 35)
			return ERBLMissionDifficulty.HARD;
		
		return ERBLMissionDifficulty.EXTREME;
	}
	
	protected static float GetTimeLimitForZone(ERBLZoneType type, int warLevel)
	{
		float baseTime = 600;
		
		switch (type)
		{
			case ERBLZoneType.Roadblock:
			case ERBLZoneType.Watchpost:
				baseTime = 300;
				break;
			case ERBLZoneType.Outpost:
				baseTime = 600;
				break;
			case ERBLZoneType.Town:
				baseTime = 900;
				break;
			case ERBLZoneType.Factory:
			case ERBLZoneType.Resource:
				baseTime = 720;
				break;
			case ERBLZoneType.Airbase:
			case ERBLZoneType.HQ:
				baseTime = 1200;
				break;
		}
		
		return baseTime + (warLevel * 60);
	}
	
	protected static int CalculateMoneyReward(ERBLZoneType type, int warLevel)
	{
		int baseReward = 100;
		
		switch (type)
		{
			case ERBLZoneType.Roadblock:
			case ERBLZoneType.Watchpost:
				baseReward = 100;
				break;
			case ERBLZoneType.Outpost:
				baseReward = 200;
				break;
			case ERBLZoneType.Town:
				baseReward = 350;
				break;
			case ERBLZoneType.Factory:
				baseReward = 400;
				break;
			case ERBLZoneType.Resource:
				baseReward = 300;
				break;
			case ERBLZoneType.Airbase:
				baseReward = 600;
				break;
			case ERBLZoneType.HQ:
				baseReward = 1000;
				break;
		}
		
		return baseReward + (warLevel * 50);
	}
	
	protected static int CalculateHRReward(ERBLZoneType type)
	{
		switch (type)
		{
			case ERBLZoneType.Town:
				return 2;
			case ERBLZoneType.Airbase:
			case ERBLZoneType.HQ:
				return 3;
			default:
				return 1;
		}
		return 1;
	}
}

