// ============================================================================
// PROJECT REBELLION - Mission Data Structures
// Core data classes for the mission system
// ============================================================================

// ============================================================================
// MISSION OBJECTIVE
// ============================================================================
class RBL_MissionObjective
{
	protected string m_sID;
	protected string m_sDescription;
	protected ERBLObjectiveType m_eType;
	protected bool m_bCompleted;
	protected bool m_bOptional;
	
	// Objective parameters
	protected string m_sTargetZoneID;
	protected string m_sTargetEntityID;
	protected vector m_vTargetPosition;
	protected int m_iTargetCount;
	protected int m_iCurrentCount;
	protected float m_fTimeLimit;
	protected float m_fTimeRemaining;
	
	void RBL_MissionObjective()
	{
		m_sID = "";
		m_sDescription = "";
		m_eType = ERBLObjectiveType.CAPTURE_ZONE;
		m_bCompleted = false;
		m_bOptional = false;
		m_iTargetCount = 1;
		m_iCurrentCount = 0;
		m_fTimeLimit = 0;
		m_fTimeRemaining = 0;
		m_vTargetPosition = vector.Zero;
	}
	
	// Initializers for different objective types
	static RBL_MissionObjective CreateCaptureZone(string zoneID, string description)
	{
		RBL_MissionObjective obj = new RBL_MissionObjective();
		obj.m_sID = "obj_capture_" + zoneID;
		obj.m_sDescription = description;
		obj.m_eType = ERBLObjectiveType.CAPTURE_ZONE;
		obj.m_sTargetZoneID = zoneID;
		return obj;
	}
	
	static RBL_MissionObjective CreateDefendZone(string zoneID, float duration, string description)
	{
		RBL_MissionObjective obj = new RBL_MissionObjective();
		obj.m_sID = "obj_defend_" + zoneID;
		obj.m_sDescription = description;
		obj.m_eType = ERBLObjectiveType.DEFEND_ZONE;
		obj.m_sTargetZoneID = zoneID;
		obj.m_fTimeLimit = duration;
		obj.m_fTimeRemaining = duration;
		return obj;
	}
	
	static RBL_MissionObjective CreateKillCount(int count, string description)
	{
		RBL_MissionObjective obj = new RBL_MissionObjective();
		obj.m_sID = "obj_killcount_" + count.ToString();
		obj.m_sDescription = description;
		obj.m_eType = ERBLObjectiveType.KILL_COUNT;
		obj.m_iTargetCount = count;
		return obj;
	}
	
	static RBL_MissionObjective CreateReachLocation(vector position, string description)
	{
		RBL_MissionObjective obj = new RBL_MissionObjective();
		obj.m_sID = "obj_reach_" + position.ToString(false);
		obj.m_sDescription = description;
		obj.m_eType = ERBLObjectiveType.REACH_LOCATION;
		obj.m_vTargetPosition = position;
		return obj;
	}
	
	static RBL_MissionObjective CreateSurviveTime(float duration, string description)
	{
		RBL_MissionObjective obj = new RBL_MissionObjective();
		obj.m_sID = "obj_survive_" + duration.ToString();
		obj.m_sDescription = description;
		obj.m_eType = ERBLObjectiveType.SURVIVE_TIME;
		obj.m_fTimeLimit = duration;
		obj.m_fTimeRemaining = duration;
		return obj;
	}
	
	// Update
	void Update(float timeSlice)
	{
		if (m_bCompleted)
			return;
		
		// Update time-based objectives
		if (m_fTimeRemaining > 0)
		{
			m_fTimeRemaining -= timeSlice;
			if (m_fTimeRemaining <= 0)
			{
				m_fTimeRemaining = 0;
				
				// Survive objectives complete when time runs out
				if (m_eType == ERBLObjectiveType.SURVIVE_TIME || 
				    m_eType == ERBLObjectiveType.DEFEND_ZONE)
				{
					m_bCompleted = true;
				}
			}
		}
	}
	
	// Progress tracking
	void IncrementProgress(int amount = 1)
	{
		m_iCurrentCount += amount;
		if (m_iCurrentCount >= m_iTargetCount)
		{
			m_iCurrentCount = m_iTargetCount;
			m_bCompleted = true;
		}
	}
	
	void SetCompleted(bool completed)
	{
		m_bCompleted = completed;
	}
	
	// Getters
	string GetID() { return m_sID; }
	string GetDescription() { return m_sDescription; }
	ERBLObjectiveType GetType() { return m_eType; }
	bool IsCompleted() { return m_bCompleted; }
	bool IsOptional() { return m_bOptional; }
	string GetTargetZoneID() { return m_sTargetZoneID; }
	vector GetTargetPosition() { return m_vTargetPosition; }
	int GetTargetCount() { return m_iTargetCount; }
	int GetCurrentCount() { return m_iCurrentCount; }
	float GetTimeLimit() { return m_fTimeLimit; }
	float GetTimeRemaining() { return m_fTimeRemaining; }
	
	float GetProgress()
	{
		if (m_iTargetCount <= 0)
		{
			if (m_bCompleted)
				return 1.0;
			return 0.0;
		}
		return m_iCurrentCount / m_iTargetCount;
	}
	
	// Setters
	void SetOptional(bool optional) { m_bOptional = optional; }
}

// ============================================================================
// MISSION REWARD
// ============================================================================
class RBL_MissionReward
{
	int m_iMoney;
	int m_iHR;
	int m_iAggression;
	ref array<string> m_aUnlockItems;
	
	void RBL_MissionReward()
	{
		m_iMoney = 0;
		m_iHR = 0;
		m_iAggression = 0;
		m_aUnlockItems = new array<string>();
	}
	
	static RBL_MissionReward Create(int money, int hr, int aggression = 0)
	{
		RBL_MissionReward reward = new RBL_MissionReward();
		reward.m_iMoney = money;
		reward.m_iHR = hr;
		reward.m_iAggression = aggression;
		return reward;
	}
	
	void AddUnlockItem(string itemID)
	{
		m_aUnlockItems.Insert(itemID);
	}
	
	int GetMoney() { return m_iMoney; }
	int GetHR() { return m_iHR; }
	int GetAggression() { return m_iAggression; }
	array<string> GetUnlockItems() { return m_aUnlockItems; }
	
	string GetRewardString()
	{
		string result = "";
		if (m_iMoney > 0)
			result += "$" + m_iMoney.ToString();
		if (m_iHR > 0)
		{
			if (!result.IsEmpty()) result += ", ";
			result += m_iHR.ToString() + " HR";
		}
		if (result.IsEmpty())
			result = "None";
		return result;
	}
}

// ============================================================================
// MISSION
// ============================================================================
class RBL_Mission
{
	protected string m_sID;
	protected string m_sName;
	protected string m_sDescription;
	protected string m_sBriefing;
	
	protected ERBLMissionType m_eType;
	protected ERBLMissionStatus m_eStatus;
	protected ERBLMissionDifficulty m_eDifficulty;
	
	protected ref array<ref RBL_MissionObjective> m_aObjectives;
	protected ref RBL_MissionReward m_pReward;
	
	protected float m_fTimeLimit;
	protected float m_fTimeRemaining;
	protected float m_fTimeElapsed;
	
	protected string m_sTargetZoneID;
	protected vector m_vMissionArea;
	protected float m_fMissionRadius;
	
	protected int m_iMinWarLevel;
	protected bool m_bRepeatable;
	
	void RBL_Mission()
	{
		m_sID = "";
		m_sName = "";
		m_sDescription = "";
		m_sBriefing = "";
		
		m_eType = ERBLMissionType.NONE;
		m_eStatus = ERBLMissionStatus.AVAILABLE;
		m_eDifficulty = ERBLMissionDifficulty.MEDIUM;
		
		m_aObjectives = new array<ref RBL_MissionObjective>();
		m_pReward = new RBL_MissionReward();
		
		m_fTimeLimit = 0;
		m_fTimeRemaining = 0;
		m_fTimeElapsed = 0;
		
		m_vMissionArea = vector.Zero;
		m_fMissionRadius = 500;
		
		m_iMinWarLevel = 1;
		m_bRepeatable = false;
	}
	
	// ========================================================================
	// SETUP
	// ========================================================================
	
	void SetID(string id) { m_sID = id; }
	void SetName(string name) { m_sName = name; }
	void SetDescription(string desc) { m_sDescription = desc; }
	void SetBriefing(string briefing) { m_sBriefing = briefing; }
	void SetType(ERBLMissionType type) { m_eType = type; }
	void SetDifficulty(ERBLMissionDifficulty diff) { m_eDifficulty = diff; }
	void SetReward(RBL_MissionReward reward) { m_pReward = reward; }
	void SetTimeLimit(float time) { m_fTimeLimit = time; m_fTimeRemaining = time; }
	void SetTargetZone(string zoneID) { m_sTargetZoneID = zoneID; }
	void SetMissionArea(vector center, float radius) { m_vMissionArea = center; m_fMissionRadius = radius; }
	void SetMinWarLevel(int level) { m_iMinWarLevel = level; }
	void SetRepeatable(bool repeatable) { m_bRepeatable = repeatable; }
	
	void AddObjective(RBL_MissionObjective objective)
	{
		if (objective)
			m_aObjectives.Insert(objective);
	}
	
	// ========================================================================
	// STATUS MANAGEMENT
	// ========================================================================
	
	void Start()
	{
		if (m_eStatus != ERBLMissionStatus.AVAILABLE)
			return;
		
		m_eStatus = ERBLMissionStatus.ACTIVE;
		m_fTimeElapsed = 0;
		
		PrintFormat("[RBL_Mission] Started: %1", m_sName);
	}
	
	void Complete()
	{
		m_eStatus = ERBLMissionStatus.COMPLETED;
		PrintFormat("[RBL_Mission] Completed: %1", m_sName);
	}
	
	void Fail()
	{
		m_eStatus = ERBLMissionStatus.FAILED;
		PrintFormat("[RBL_Mission] Failed: %1", m_sName);
	}
	
	void Expire()
	{
		m_eStatus = ERBLMissionStatus.EXPIRED;
		PrintFormat("[RBL_Mission] Expired: %1", m_sName);
	}
	
	// ========================================================================
	// UPDATE
	// ========================================================================
	
	void Update(float timeSlice)
	{
		if (m_eStatus != ERBLMissionStatus.ACTIVE)
			return;
		
		m_fTimeElapsed += timeSlice;
		
		// Update time limit
		if (m_fTimeLimit > 0)
		{
			m_fTimeRemaining -= timeSlice;
			if (m_fTimeRemaining <= 0)
			{
				m_fTimeRemaining = 0;
				Fail();
				return;
			}
		}
		
		// Update objectives
		bool allComplete = true;
		bool anyRequired = false;
		
		for (int i = 0; i < m_aObjectives.Count(); i++)
		{
			RBL_MissionObjective obj = m_aObjectives[i];
			if (!obj)
				continue;
			
			obj.Update(timeSlice);
			
			if (!obj.IsOptional())
			{
				anyRequired = true;
				if (!obj.IsCompleted())
					allComplete = false;
			}
		}
		
		// Complete if all required objectives done
		if (anyRequired && allComplete)
		{
			Complete();
		}
	}
	
	// ========================================================================
	// OBJECTIVE MANAGEMENT
	// ========================================================================
	
	RBL_MissionObjective GetObjectiveByID(string objectiveID)
	{
		for (int i = 0; i < m_aObjectives.Count(); i++)
		{
			if (m_aObjectives[i] && m_aObjectives[i].GetID() == objectiveID)
				return m_aObjectives[i];
		}
		return null;
	}
	
	void OnZoneCaptured(string zoneID, ERBLFactionKey byFaction)
	{
		if (m_eStatus != ERBLMissionStatus.ACTIVE)
			return;
		
		for (int i = 0; i < m_aObjectives.Count(); i++)
		{
			RBL_MissionObjective obj = m_aObjectives[i];
			if (!obj || obj.IsCompleted())
				continue;
			
			if (obj.GetType() == ERBLObjectiveType.CAPTURE_ZONE)
			{
				if (obj.GetTargetZoneID() == zoneID && byFaction == ERBLFactionKey.FIA)
				{
					obj.SetCompleted(true);
				}
			}
			else if (obj.GetType() == ERBLObjectiveType.DEFEND_ZONE)
			{
				if (obj.GetTargetZoneID() == zoneID && byFaction != ERBLFactionKey.FIA)
				{
					Fail();
					return;
				}
			}
		}
	}
	
	void OnEnemyKilled()
	{
		if (m_eStatus != ERBLMissionStatus.ACTIVE)
			return;
		
		for (int i = 0; i < m_aObjectives.Count(); i++)
		{
			RBL_MissionObjective obj = m_aObjectives[i];
			if (!obj || obj.IsCompleted())
				continue;
			
			if (obj.GetType() == ERBLObjectiveType.KILL_COUNT)
			{
				obj.IncrementProgress(1);
			}
		}
	}
	
	void OnPlayerReachedLocation(vector position)
	{
		if (m_eStatus != ERBLMissionStatus.ACTIVE)
			return;
		
		for (int i = 0; i < m_aObjectives.Count(); i++)
		{
			RBL_MissionObjective obj = m_aObjectives[i];
			if (!obj || obj.IsCompleted())
				continue;
			
			if (obj.GetType() == ERBLObjectiveType.REACH_LOCATION)
			{
				float dist = vector.Distance(position, obj.GetTargetPosition());
				if (dist <= 50.0)
				{
					obj.SetCompleted(true);
				}
			}
		}
	}
	
	// ========================================================================
	// GETTERS
	// ========================================================================
	
	string GetID() { return m_sID; }
	string GetName() { return m_sName; }
	string GetDescription() { return m_sDescription; }
	string GetBriefing() { return m_sBriefing; }
	ERBLMissionType GetType() { return m_eType; }
	ERBLMissionStatus GetStatus() { return m_eStatus; }
	ERBLMissionDifficulty GetDifficulty() { return m_eDifficulty; }
	RBL_MissionReward GetReward() { return m_pReward; }
	array<ref RBL_MissionObjective> GetObjectives() { return m_aObjectives; }
	int GetObjectiveCount() { return m_aObjectives.Count(); }
	float GetTimeLimit() { return m_fTimeLimit; }
	float GetTimeRemaining() { return m_fTimeRemaining; }
	float GetTimeElapsed() { return m_fTimeElapsed; }
	string GetTargetZoneID() { return m_sTargetZoneID; }
	vector GetMissionArea() { return m_vMissionArea; }
	float GetMissionRadius() { return m_fMissionRadius; }
	int GetMinWarLevel() { return m_iMinWarLevel; }
	bool IsRepeatable() { return m_bRepeatable; }
	
	bool IsActive() { return m_eStatus == ERBLMissionStatus.ACTIVE; }
	bool IsAvailable() { return m_eStatus == ERBLMissionStatus.AVAILABLE; }
	bool IsCompleted() { return m_eStatus == ERBLMissionStatus.COMPLETED; }
	bool IsFailed() { return m_eStatus == ERBLMissionStatus.FAILED; }
	
	bool HasTimeLimit() { return m_fTimeLimit > 0; }
	
	int GetCompletedObjectiveCount()
	{
		int count = 0;
		for (int i = 0; i < m_aObjectives.Count(); i++)
		{
			if (m_aObjectives[i] && m_aObjectives[i].IsCompleted())
				count++;
		}
		return count;
	}
	
	float GetOverallProgress()
	{
		int total = m_aObjectives.Count();
		if (total == 0)
		{
			if (m_eStatus == ERBLMissionStatus.COMPLETED)
				return 1.0;
			return 0.0;
		}
		
		return GetCompletedObjectiveCount() / total;
	}
	
	string GetDifficultyString()
	{
		switch (m_eDifficulty)
		{
			case ERBLMissionDifficulty.EASY: return "Easy";
			case ERBLMissionDifficulty.MEDIUM: return "Medium";
			case ERBLMissionDifficulty.HARD: return "Hard";
			case ERBLMissionDifficulty.EXTREME: return "Extreme";
		}
		return "Unknown";
	}
	
	string GetStatusString()
	{
		switch (m_eStatus)
		{
			case ERBLMissionStatus.AVAILABLE: return "Available";
			case ERBLMissionStatus.ACTIVE: return "Active";
			case ERBLMissionStatus.COMPLETED: return "Completed";
			case ERBLMissionStatus.FAILED: return "Failed";
			case ERBLMissionStatus.EXPIRED: return "Expired";
		}
		return "Unknown";
	}
	
	string GetTypeString()
	{
		switch (m_eType)
		{
			case ERBLMissionType.ATTACK: return "Attack";
			case ERBLMissionType.DEFEND: return "Defend";
			case ERBLMissionType.PATROL: return "Patrol";
			case ERBLMissionType.RESUPPLY: return "Resupply";
			case ERBLMissionType.REINFORCE: return "Reinforce";
			case ERBLMissionType.AMBUSH: return "Ambush";
			case ERBLMissionType.SABOTAGE: return "Sabotage";
			case ERBLMissionType.RESCUE: return "Rescue";
			case ERBLMissionType.ASSASSINATION: return "Assassination";
			case ERBLMissionType.INTEL: return "Intelligence";
		}
		return "Unknown";
	}
}

