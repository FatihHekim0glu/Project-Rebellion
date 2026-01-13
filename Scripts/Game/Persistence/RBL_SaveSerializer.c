// ============================================================================
// PROJECT REBELLION - Save Serializer
// JSON serialization and deserialization utilities
// ============================================================================

class RBL_SaveSerializer
{
	// ========================================================================
	// MAIN SERIALIZATION
	// ========================================================================
	
	// Serialize full save data to JSON string
	static string SerializeToJson(RBL_SaveData saveData)
	{
		if (!saveData)
			return "";
		
		string json = "{";
		
		// Header
		json += SerializeHeader(saveData);
		
		// Campaign
		json += ",\"campaign\":" + SerializeCampaign(saveData.m_Campaign);
		
		// Zones
		json += ",\"zones\":" + SerializeZoneArray(saveData.m_aZones);
		
		// Economy
		json += ",\"economy\":" + SerializeEconomy(saveData.m_Economy);
		
		// Players
		json += ",\"players\":" + SerializePlayerArray(saveData.m_aPlayers);
		
		// Commander
		json += ",\"commander\":" + SerializeCommander(saveData.m_Commander);
		
		// Missions
		json += ",\"missions\":" + SerializeMissionArray(saveData.m_aMissions);
		
		json += "}";
		
		return json;
	}
	
	// Deserialize JSON string to save data
	static RBL_SaveData DeserializeFromJson(string json)
	{
		if (json.IsEmpty())
			return null;
		
		RBL_SaveData saveData = new RBL_SaveData();
		
		// Parse JSON using built-in parser
		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
		if (!ctx.ImportFromString(json))
		{
			PrintFormat("[RBL_SaveSerializer] Failed to parse JSON");
			return null;
		}
		
		// Header
		ctx.ReadValue("magic", saveData.m_sMagic);
		ctx.ReadValue("version", saveData.m_iVersion);
		ctx.ReadValue("saveTime", saveData.m_sSaveTime);
		ctx.ReadValue("playerId", saveData.m_sPlayerId);
		ctx.ReadValue("worldName", saveData.m_sWorldName);
		
		// Validate
		if (!saveData.IsValid())
		{
			PrintFormat("[RBL_SaveSerializer] Invalid save file header");
			return null;
		}
		
		// Deserialize sections
		DeserializeCampaign(ctx, saveData.m_Campaign);
		DeserializeZones(ctx, saveData.m_aZones);
		DeserializeEconomy(ctx, saveData.m_Economy);
		DeserializePlayers(ctx, saveData.m_aPlayers);
		DeserializeCommander(ctx, saveData.m_Commander);
		DeserializeMissions(ctx, saveData.m_aMissions);
		
		return saveData;
	}
	
	// ========================================================================
	// HEADER SERIALIZATION
	// ========================================================================
	
	protected static string SerializeHeader(RBL_SaveData saveData)
	{
		string json = "";
		json += "\"magic\":\"" + saveData.m_sMagic + "\"";
		json += ",\"version\":" + saveData.m_iVersion.ToString();
		json += ",\"saveTime\":\"" + saveData.m_sSaveTime + "\"";
		json += ",\"playerId\":\"" + saveData.m_sPlayerId + "\"";
		json += ",\"worldName\":\"" + saveData.m_sWorldName + "\"";
		return json;
	}
	
	// ========================================================================
	// CAMPAIGN SERIALIZATION
	// ========================================================================
	
	protected static string SerializeCampaign(RBL_CampaignSaveData campaign)
	{
		if (!campaign)
			return "null";
		
		string json = "{";
		json += "\"warLevel\":" + campaign.m_iWarLevel.ToString();
		json += ",\"aggression\":" + campaign.m_iAggression.ToString();
		json += ",\"civilianSupport\":" + campaign.m_iCivilianSupport.ToString();
		json += ",\"totalPlayTime\":" + campaign.m_fTotalPlayTime.ToString();
		json += ",\"gameDays\":" + campaign.m_iGameDays.ToString();
		json += ",\"currentDayTime\":" + campaign.m_fCurrentDayTime.ToString();
		json += ",\"alertState\":" + campaign.m_iAlertState.ToString();
		json += ",\"alertDecayTimer\":" + campaign.m_fAlertDecayTimer.ToString();
		json += ",\"zonesCaptured\":" + campaign.m_iZonesCaptured.ToString();
		json += ",\"zonesLost\":" + campaign.m_iZonesLost.ToString();
		json += ",\"enemiesKilled\":" + campaign.m_iEnemiesKilled.ToString();
		json += ",\"friendliesLost\":" + campaign.m_iFriendliesLost.ToString();
		json += ",\"startDate\":\"" + campaign.m_sStartDate + "\"";
		json += ",\"lastPlayDate\":\"" + campaign.m_sLastPlayDate + "\"";
		json += "}";
		return json;
	}
	
	protected static void DeserializeCampaign(SCR_JsonLoadContext ctx, RBL_CampaignSaveData campaign)
	{
		if (!ctx.StartObject("campaign"))
			return;
		
		ctx.ReadValue("warLevel", campaign.m_iWarLevel);
		ctx.ReadValue("aggression", campaign.m_iAggression);
		ctx.ReadValue("civilianSupport", campaign.m_iCivilianSupport);
		ctx.ReadValue("totalPlayTime", campaign.m_fTotalPlayTime);
		ctx.ReadValue("gameDays", campaign.m_iGameDays);
		ctx.ReadValue("currentDayTime", campaign.m_fCurrentDayTime);
		ctx.ReadValue("alertState", campaign.m_iAlertState);
		ctx.ReadValue("alertDecayTimer", campaign.m_fAlertDecayTimer);
		ctx.ReadValue("zonesCaptured", campaign.m_iZonesCaptured);
		ctx.ReadValue("zonesLost", campaign.m_iZonesLost);
		ctx.ReadValue("enemiesKilled", campaign.m_iEnemiesKilled);
		ctx.ReadValue("friendliesLost", campaign.m_iFriendliesLost);
		ctx.ReadValue("startDate", campaign.m_sStartDate);
		ctx.ReadValue("lastPlayDate", campaign.m_sLastPlayDate);
		
		ctx.EndObject();
	}
	
	// ========================================================================
	// ZONE SERIALIZATION
	// ========================================================================
	
	protected static string SerializeZoneArray(array<ref RBL_ZoneSaveData> zones)
	{
		if (!zones || zones.IsEmpty())
			return "[]";
		
		string json = "[";
		for (int i = 0; i < zones.Count(); i++)
		{
			if (i > 0) json += ",";
			json += SerializeZone(zones[i]);
		}
		json += "]";
		return json;
	}
	
	protected static string SerializeZone(RBL_ZoneSaveData zone)
	{
		if (!zone)
			return "null";
		
		string json = "{";
		json += "\"zoneID\":\"" + zone.m_sZoneID + "\"";
		json += ",\"zoneName\":\"" + zone.m_sZoneName + "\"";
		json += ",\"zoneType\":" + zone.m_iZoneType.ToString();
		json += ",\"position\":" + SerializeVector(zone.m_vPosition);
		json += ",\"ownerFaction\":" + zone.m_iOwnerFaction.ToString();
		json += ",\"previousOwner\":" + zone.m_iPreviousOwner.ToString();
		json += ",\"garrisonStrength\":" + zone.m_iGarrisonStrength.ToString();
		json += ",\"maxGarrison\":" + zone.m_iMaxGarrison.ToString();
		json += ",\"garrisonUnitTypes\":" + SerializeStringArray(zone.m_aGarrisonUnitTypes);
		json += ",\"supportLevel\":" + zone.m_iSupportLevel.ToString();
		json += ",\"supportTrend\":" + zone.m_iSupportTrend.ToString();
		json += ",\"captureProgress\":" + zone.m_fCaptureProgress.ToString();
		json += ",\"capturingFaction\":" + zone.m_iCapturingFaction.ToString();
		json += ",\"resourceOutput\":" + zone.m_iResourceOutput.ToString();
		json += ",\"resourceTimer\":" + zone.m_fResourceTimer.ToString();
		json += ",\"isUnderAttack\":" + BoolToJson(zone.m_bIsUnderAttack);
		json += ",\"isContested\":" + BoolToJson(zone.m_bIsContested);
		json += ",\"isDestroyed\":" + BoolToJson(zone.m_bIsDestroyed);
		json += ",\"lastCaptureTime\":" + zone.m_fLastCaptureTime.ToString();
		json += ",\"lastAttackTime\":" + zone.m_fLastAttackTime.ToString();
		json += "}";
		return json;
	}
	
	protected static void DeserializeZones(SCR_JsonLoadContext ctx, array<ref RBL_ZoneSaveData> zones)
	{
		zones.Clear();
		
		if (!ctx.StartArray("zones"))
			return;
		
		int count = ctx.GetChildCount();
		for (int i = 0; i < count; i++)
		{
			if (!ctx.StartArrayItem(i))
				continue;
			
			RBL_ZoneSaveData zone = new RBL_ZoneSaveData();
			
			ctx.ReadValue("zoneID", zone.m_sZoneID);
			ctx.ReadValue("zoneName", zone.m_sZoneName);
			ctx.ReadValue("zoneType", zone.m_iZoneType);
			DeserializeVector(ctx, "position", zone.m_vPosition);
			ctx.ReadValue("ownerFaction", zone.m_iOwnerFaction);
			ctx.ReadValue("previousOwner", zone.m_iPreviousOwner);
			ctx.ReadValue("garrisonStrength", zone.m_iGarrisonStrength);
			ctx.ReadValue("maxGarrison", zone.m_iMaxGarrison);
			DeserializeStringArray(ctx, "garrisonUnitTypes", zone.m_aGarrisonUnitTypes);
			ctx.ReadValue("supportLevel", zone.m_iSupportLevel);
			ctx.ReadValue("supportTrend", zone.m_iSupportTrend);
			ctx.ReadValue("captureProgress", zone.m_fCaptureProgress);
			ctx.ReadValue("capturingFaction", zone.m_iCapturingFaction);
			ctx.ReadValue("resourceOutput", zone.m_iResourceOutput);
			ctx.ReadValue("resourceTimer", zone.m_fResourceTimer);
			ctx.ReadValue("isUnderAttack", zone.m_bIsUnderAttack);
			ctx.ReadValue("isContested", zone.m_bIsContested);
			ctx.ReadValue("isDestroyed", zone.m_bIsDestroyed);
			ctx.ReadValue("lastCaptureTime", zone.m_fLastCaptureTime);
			ctx.ReadValue("lastAttackTime", zone.m_fLastAttackTime);
			
			zones.Insert(zone);
			ctx.EndArrayItem();
		}
		
		ctx.EndArray();
	}
	
	// ========================================================================
	// ECONOMY SERIALIZATION
	// ========================================================================
	
	protected static string SerializeEconomy(RBL_EconomySaveData economy)
	{
		if (!economy)
			return "null";
		
		string json = "{";
		json += "\"fiaMoney\":" + economy.m_iFIAMoney.ToString();
		json += ",\"fiaHR\":" + economy.m_iFIAHumanResources.ToString();
		json += ",\"fiaFuel\":" + economy.m_iFIAFuel.ToString();
		json += ",\"fiaAmmo\":" + economy.m_iFIAAmmo.ToString();
		json += ",\"moneyPerMinute\":" + economy.m_iMoneyPerMinute.ToString();
		json += ",\"hrPerMinute\":" + economy.m_iHRPerMinute.ToString();
		json += ",\"arsenalItems\":" + SerializeArsenalArray(economy.m_aArsenalItems);
		json += ",\"unlockedItems\":" + SerializeStringArray(economy.m_aUnlockedItems);
		json += ",\"vehiclePool\":" + SerializeVehicleArray(economy.m_aVehiclePool);
		json += ",\"totalMoneyEarned\":" + economy.m_iTotalMoneyEarned.ToString();
		json += ",\"totalMoneySpent\":" + economy.m_iTotalMoneySpent.ToString();
		json += ",\"totalItemsBought\":" + economy.m_iTotalItemsBought.ToString();
		json += ",\"totalItemsSold\":" + economy.m_iTotalItemsSold.ToString();
		json += "}";
		return json;
	}
	
	protected static void DeserializeEconomy(SCR_JsonLoadContext ctx, RBL_EconomySaveData economy)
	{
		if (!ctx.StartObject("economy"))
			return;
		
		ctx.ReadValue("fiaMoney", economy.m_iFIAMoney);
		ctx.ReadValue("fiaHR", economy.m_iFIAHumanResources);
		ctx.ReadValue("fiaFuel", economy.m_iFIAFuel);
		ctx.ReadValue("fiaAmmo", economy.m_iFIAAmmo);
		ctx.ReadValue("moneyPerMinute", economy.m_iMoneyPerMinute);
		ctx.ReadValue("hrPerMinute", economy.m_iHRPerMinute);
		DeserializeArsenalArray(ctx, economy.m_aArsenalItems);
		DeserializeStringArray(ctx, "unlockedItems", economy.m_aUnlockedItems);
		DeserializeVehicleArray(ctx, economy.m_aVehiclePool);
		ctx.ReadValue("totalMoneyEarned", economy.m_iTotalMoneyEarned);
		ctx.ReadValue("totalMoneySpent", economy.m_iTotalMoneySpent);
		ctx.ReadValue("totalItemsBought", economy.m_iTotalItemsBought);
		ctx.ReadValue("totalItemsSold", economy.m_iTotalItemsSold);
		
		ctx.EndObject();
	}
	
	protected static string SerializeArsenalArray(array<ref RBL_ArsenalItemSave> items)
	{
		if (!items || items.IsEmpty())
			return "[]";
		
		string json = "[";
		for (int i = 0; i < items.Count(); i++)
		{
			if (i > 0) json += ",";
			json += "{\"id\":\"" + items[i].m_sItemID + "\",\"qty\":" + items[i].m_iQuantity.ToString() + "}";
		}
		json += "]";
		return json;
	}
	
	protected static void DeserializeArsenalArray(SCR_JsonLoadContext ctx, array<ref RBL_ArsenalItemSave> items)
	{
		items.Clear();
		
		if (!ctx.StartArray("arsenalItems"))
			return;
		
		int count = ctx.GetChildCount();
		for (int i = 0; i < count; i++)
		{
			if (!ctx.StartArrayItem(i))
				continue;
			
			string itemID;
			int qty;
			ctx.ReadValue("id", itemID);
			ctx.ReadValue("qty", qty);
			
			items.Insert(new RBL_ArsenalItemSave(itemID, qty));
			ctx.EndArrayItem();
		}
		
		ctx.EndArray();
	}
	
	protected static string SerializeVehicleArray(array<ref RBL_VehicleSave> vehicles)
	{
		if (!vehicles || vehicles.IsEmpty())
			return "[]";
		
		string json = "[";
		for (int i = 0; i < vehicles.Count(); i++)
		{
			if (i > 0) json += ",";
			RBL_VehicleSave v = vehicles[i];
			json += "{\"id\":\"" + v.m_sVehicleID + "\",\"type\":\"" + v.m_sVehicleType + "\"";
			json += ",\"fuel\":" + v.m_fFuel.ToString() + ",\"health\":" + v.m_fHealth.ToString();
			json += ",\"deployed\":" + BoolToJson(v.m_bIsDeployed);
			json += ",\"pos\":" + SerializeVector(v.m_vPosition) + "}";
		}
		json += "]";
		return json;
	}
	
	protected static void DeserializeVehicleArray(SCR_JsonLoadContext ctx, array<ref RBL_VehicleSave> vehicles)
	{
		vehicles.Clear();
		
		if (!ctx.StartArray("vehiclePool"))
			return;
		
		int count = ctx.GetChildCount();
		for (int i = 0; i < count; i++)
		{
			if (!ctx.StartArrayItem(i))
				continue;
			
			RBL_VehicleSave v = new RBL_VehicleSave();
			ctx.ReadValue("id", v.m_sVehicleID);
			ctx.ReadValue("type", v.m_sVehicleType);
			ctx.ReadValue("fuel", v.m_fFuel);
			ctx.ReadValue("health", v.m_fHealth);
			ctx.ReadValue("deployed", v.m_bIsDeployed);
			DeserializeVector(ctx, "pos", v.m_vPosition);
			
			vehicles.Insert(v);
			ctx.EndArrayItem();
		}
		
		ctx.EndArray();
	}
	
	// ========================================================================
	// PLAYER SERIALIZATION
	// ========================================================================
	
	protected static string SerializePlayerArray(array<ref RBL_PlayerSaveData> players)
	{
		if (!players || players.IsEmpty())
			return "[]";
		
		string json = "[";
		for (int i = 0; i < players.Count(); i++)
		{
			if (i > 0) json += ",";
			json += SerializePlayer(players[i]);
		}
		json += "]";
		return json;
	}
	
	protected static string SerializePlayer(RBL_PlayerSaveData player)
	{
		if (!player)
			return "null";
		
		string json = "{";
		json += "\"uid\":\"" + player.m_sPlayerUID + "\"";
		json += ",\"name\":\"" + EscapeString(player.m_sPlayerName) + "\"";
		json += ",\"money\":" + player.m_iMoney.ToString();
		json += ",\"hr\":" + player.m_iHumanResources.ToString();
		json += ",\"kills\":" + player.m_iKills.ToString();
		json += ",\"deaths\":" + player.m_iDeaths.ToString();
		json += ",\"zonesCaptured\":" + player.m_iZonesCaptured.ToString();
		json += ",\"missionsCompleted\":" + player.m_iMissionsCompleted.ToString();
		json += ",\"playTime\":" + player.m_fPlayTime.ToString();
		json += ",\"rank\":" + player.m_iRank.ToString();
		json += ",\"experience\":" + player.m_iExperience.ToString();
		json += ",\"unlocks\":" + SerializeStringArray(player.m_aUnlocks);
		json += ",\"achievements\":" + SerializeStringArray(player.m_aAchievements);
		json += ",\"inventory\":" + SerializeStringArray(player.m_aInventoryItems);
		json += ",\"lastPosition\":" + SerializeVector(player.m_vLastPosition);
		json += ",\"lastDirection\":" + player.m_fLastDirection.ToString();
		json += ",\"coverStatus\":" + player.m_iCoverStatus.ToString();
		json += ",\"suspicionLevel\":" + player.m_fSuspicionLevel.ToString();
		json += "}";
		return json;
	}
	
	protected static void DeserializePlayers(SCR_JsonLoadContext ctx, array<ref RBL_PlayerSaveData> players)
	{
		players.Clear();
		
		if (!ctx.StartArray("players"))
			return;
		
		int count = ctx.GetChildCount();
		for (int i = 0; i < count; i++)
		{
			if (!ctx.StartArrayItem(i))
				continue;
			
			RBL_PlayerSaveData player = new RBL_PlayerSaveData();
			
			ctx.ReadValue("uid", player.m_sPlayerUID);
			ctx.ReadValue("name", player.m_sPlayerName);
			ctx.ReadValue("money", player.m_iMoney);
			ctx.ReadValue("hr", player.m_iHumanResources);
			ctx.ReadValue("kills", player.m_iKills);
			ctx.ReadValue("deaths", player.m_iDeaths);
			ctx.ReadValue("zonesCaptured", player.m_iZonesCaptured);
			ctx.ReadValue("missionsCompleted", player.m_iMissionsCompleted);
			ctx.ReadValue("playTime", player.m_fPlayTime);
			ctx.ReadValue("rank", player.m_iRank);
			ctx.ReadValue("experience", player.m_iExperience);
			DeserializeStringArray(ctx, "unlocks", player.m_aUnlocks);
			DeserializeStringArray(ctx, "achievements", player.m_aAchievements);
			DeserializeStringArray(ctx, "inventory", player.m_aInventoryItems);
			DeserializeVector(ctx, "lastPosition", player.m_vLastPosition);
			ctx.ReadValue("lastDirection", player.m_fLastDirection);
			ctx.ReadValue("coverStatus", player.m_iCoverStatus);
			ctx.ReadValue("suspicionLevel", player.m_fSuspicionLevel);
			
			players.Insert(player);
			ctx.EndArrayItem();
		}
		
		ctx.EndArray();
	}
	
	// ========================================================================
	// COMMANDER SERIALIZATION
	// ========================================================================
	
	protected static string SerializeCommander(RBL_CommanderSaveData commander)
	{
		if (!commander)
			return "null";
		
		string json = "{";
		json += "\"resources\":" + commander.m_iCommanderResources.ToString();
		json += ",\"reinforcementPool\":" + commander.m_iReinforcementPool.ToString();
		json += ",\"threatLevel\":" + commander.m_iThreatLevel.ToString();
		json += ",\"highestThreatZone\":\"" + commander.m_sHighestThreatZone + "\"";
		json += ",\"operations\":" + SerializeOperationArray(commander.m_aActiveOperations);
		json += ",\"qrfCooldown\":" + commander.m_fQRFCooldown.ToString();
		json += ",\"patrolCooldown\":" + commander.m_fPatrolCooldown.ToString();
		json += ",\"offensiveCooldown\":" + commander.m_fOffensiveCooldown.ToString();
		json += ",\"knownPlayerPositions\":" + SerializeStringArray(commander.m_aKnownPlayerPositions);
		json += ",\"recentlyAttackedZones\":" + SerializeStringArray(commander.m_aRecentlyAttackedZones);
		json += "}";
		return json;
	}
	
	protected static void DeserializeCommander(SCR_JsonLoadContext ctx, RBL_CommanderSaveData commander)
	{
		if (!ctx.StartObject("commander"))
			return;
		
		ctx.ReadValue("resources", commander.m_iCommanderResources);
		ctx.ReadValue("reinforcementPool", commander.m_iReinforcementPool);
		ctx.ReadValue("threatLevel", commander.m_iThreatLevel);
		ctx.ReadValue("highestThreatZone", commander.m_sHighestThreatZone);
		DeserializeOperationArray(ctx, commander.m_aActiveOperations);
		ctx.ReadValue("qrfCooldown", commander.m_fQRFCooldown);
		ctx.ReadValue("patrolCooldown", commander.m_fPatrolCooldown);
		ctx.ReadValue("offensiveCooldown", commander.m_fOffensiveCooldown);
		DeserializeStringArray(ctx, "knownPlayerPositions", commander.m_aKnownPlayerPositions);
		DeserializeStringArray(ctx, "recentlyAttackedZones", commander.m_aRecentlyAttackedZones);
		
		ctx.EndObject();
	}
	
	protected static string SerializeOperationArray(array<ref RBL_OperationSave> operations)
	{
		if (!operations || operations.IsEmpty())
			return "[]";
		
		string json = "[";
		for (int i = 0; i < operations.Count(); i++)
		{
			if (i > 0) json += ",";
			RBL_OperationSave op = operations[i];
			json += "{\"id\":\"" + op.m_sOperationID + "\",\"type\":" + op.m_iOperationType.ToString();
			json += ",\"target\":\"" + op.m_sTargetZone + "\",\"units\":" + op.m_iAssignedUnits.ToString();
			json += ",\"start\":" + op.m_fStartTime.ToString() + ",\"progress\":" + op.m_fProgress.ToString() + "}";
		}
		json += "]";
		return json;
	}
	
	protected static void DeserializeOperationArray(SCR_JsonLoadContext ctx, array<ref RBL_OperationSave> operations)
	{
		operations.Clear();
		
		if (!ctx.StartArray("operations"))
			return;
		
		int count = ctx.GetChildCount();
		for (int i = 0; i < count; i++)
		{
			if (!ctx.StartArrayItem(i))
				continue;
			
			RBL_OperationSave op = new RBL_OperationSave();
			ctx.ReadValue("id", op.m_sOperationID);
			ctx.ReadValue("type", op.m_iOperationType);
			ctx.ReadValue("target", op.m_sTargetZone);
			ctx.ReadValue("units", op.m_iAssignedUnits);
			ctx.ReadValue("start", op.m_fStartTime);
			ctx.ReadValue("progress", op.m_fProgress);
			
			operations.Insert(op);
			ctx.EndArrayItem();
		}
		
		ctx.EndArray();
	}
	
	// ========================================================================
	// MISSION SERIALIZATION
	// ========================================================================
	
	protected static string SerializeMissionArray(array<ref RBL_MissionSaveData> missions)
	{
		if (!missions || missions.IsEmpty())
			return "[]";
		
		string json = "[";
		for (int i = 0; i < missions.Count(); i++)
		{
			if (i > 0) json += ",";
			json += SerializeMission(missions[i]);
		}
		json += "]";
		return json;
	}
	
	protected static string SerializeMission(RBL_MissionSaveData mission)
	{
		if (!mission)
			return "null";
		
		string json = "{";
		json += "\"id\":\"" + mission.m_sMissionID + "\"";
		json += ",\"type\":\"" + mission.m_sMissionType + "\"";
		json += ",\"name\":\"" + EscapeString(mission.m_sMissionName) + "\"";
		json += ",\"targetZone\":\"" + mission.m_sTargetZone + "\"";
		json += ",\"targetPos\":" + SerializeVector(mission.m_vTargetPosition);
		json += ",\"progress\":" + mission.m_fProgress.ToString();
		json += ",\"objectivesCompleted\":" + mission.m_iObjectivesCompleted.ToString();
		json += ",\"totalObjectives\":" + mission.m_iTotalObjectives.ToString();
		json += ",\"timeLimit\":" + mission.m_fTimeLimit.ToString();
		json += ",\"timeRemaining\":" + mission.m_fTimeRemaining.ToString();
		json += ",\"startTime\":" + mission.m_fStartTime.ToString();
		json += ",\"moneyReward\":" + mission.m_iMoneyReward.ToString();
		json += ",\"hrReward\":" + mission.m_iHRReward.ToString();
		json += ",\"itemRewards\":" + SerializeStringArray(mission.m_aItemRewards);
		json += ",\"isActive\":" + BoolToJson(mission.m_bIsActive);
		json += ",\"isCompleted\":" + BoolToJson(mission.m_bIsCompleted);
		json += ",\"isFailed\":" + BoolToJson(mission.m_bIsFailed);
		json += "}";
		return json;
	}
	
	protected static void DeserializeMissions(SCR_JsonLoadContext ctx, array<ref RBL_MissionSaveData> missions)
	{
		missions.Clear();
		
		if (!ctx.StartArray("missions"))
			return;
		
		int count = ctx.GetChildCount();
		for (int i = 0; i < count; i++)
		{
			if (!ctx.StartArrayItem(i))
				continue;
			
			RBL_MissionSaveData mission = new RBL_MissionSaveData();
			
			ctx.ReadValue("id", mission.m_sMissionID);
			ctx.ReadValue("type", mission.m_sMissionType);
			ctx.ReadValue("name", mission.m_sMissionName);
			ctx.ReadValue("targetZone", mission.m_sTargetZone);
			DeserializeVector(ctx, "targetPos", mission.m_vTargetPosition);
			ctx.ReadValue("progress", mission.m_fProgress);
			ctx.ReadValue("objectivesCompleted", mission.m_iObjectivesCompleted);
			ctx.ReadValue("totalObjectives", mission.m_iTotalObjectives);
			ctx.ReadValue("timeLimit", mission.m_fTimeLimit);
			ctx.ReadValue("timeRemaining", mission.m_fTimeRemaining);
			ctx.ReadValue("startTime", mission.m_fStartTime);
			ctx.ReadValue("moneyReward", mission.m_iMoneyReward);
			ctx.ReadValue("hrReward", mission.m_iHRReward);
			DeserializeStringArray(ctx, "itemRewards", mission.m_aItemRewards);
			ctx.ReadValue("isActive", mission.m_bIsActive);
			ctx.ReadValue("isCompleted", mission.m_bIsCompleted);
			ctx.ReadValue("isFailed", mission.m_bIsFailed);
			
			missions.Insert(mission);
			ctx.EndArrayItem();
		}
		
		ctx.EndArray();
	}
	
	// ========================================================================
	// UTILITY FUNCTIONS
	// ========================================================================
	
	protected static string SerializeVector(vector v)
	{
		return "[" + v[0].ToString() + "," + v[1].ToString() + "," + v[2].ToString() + "]";
	}
	
	protected static void DeserializeVector(SCR_JsonLoadContext ctx, string name, out vector v)
	{
		if (!ctx.StartArray(name))
			return;
		
		float x, y, z;
		ctx.ReadValue("0", x);
		ctx.ReadValue("1", y);
		ctx.ReadValue("2", z);
		v = Vector(x, y, z);
		
		ctx.EndArray();
	}
	
	protected static string SerializeStringArray(array<string> arr)
	{
		if (!arr || arr.IsEmpty())
			return "[]";
		
		string json = "[";
		for (int i = 0; i < arr.Count(); i++)
		{
			if (i > 0) json += ",";
			json += "\"" + EscapeString(arr[i]) + "\"";
		}
		json += "]";
		return json;
	}
	
	protected static void DeserializeStringArray(SCR_JsonLoadContext ctx, string name, array<string> arr)
	{
		arr.Clear();
		
		if (!ctx.StartArray(name))
			return;
		
		int count = ctx.GetChildCount();
		for (int i = 0; i < count; i++)
		{
			string value;
			ctx.ReadValue(i.ToString(), value);
			arr.Insert(value);
		}
		
		ctx.EndArray();
	}
	
	protected static string BoolToJson(bool value)
	{
		if (value)
			return "true";
		return "false";
	}
	
	protected static string EscapeString(string str)
	{
		// Escape special characters for JSON
		string backslash = "\\";
		string quote = "\"";
		str = str.Replace(backslash, backslash + backslash);
		str = str.Replace(quote, backslash + quote);
		return str;
	}
}

