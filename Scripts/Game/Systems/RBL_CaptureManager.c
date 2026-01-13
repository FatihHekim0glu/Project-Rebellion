// ============================================================================
// PROJECT REBELLION - Capture Manager
// Handles zone capture when players stand in zones
// Server-authoritative capture calculations with progress broadcast
// ============================================================================

class RBL_CaptureManager
{
	protected static ref RBL_CaptureManager s_Instance;

	// Using RBL_Config constants for maintainability
	protected const float MAX_CAPTURE_PROGRESS = 100.0;

	protected float m_fTimeSinceCheck;
	protected ref map<string, float> m_mCaptureProgress;
	protected ref map<string, ERBLFactionKey> m_mCapturingFaction;

	protected ref ScriptInvoker m_OnCaptureStarted;
	protected ref ScriptInvoker m_OnCaptureProgress;
	protected ref ScriptInvoker m_OnCaptureComplete;
	
	// Network: progress broadcast timer
	protected float m_fTimeSinceBroadcast;
	protected const float BROADCAST_INTERVAL = 0.5;

	static RBL_CaptureManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_CaptureManager();
		return s_Instance;
	}

	void RBL_CaptureManager()
	{
		m_fTimeSinceCheck = 0;
		m_fTimeSinceBroadcast = 0;
		m_mCaptureProgress = new map<string, float>();
		m_mCapturingFaction = new map<string, ERBLFactionKey>();

		m_OnCaptureStarted = new ScriptInvoker();
		m_OnCaptureProgress = new ScriptInvoker();
		m_OnCaptureComplete = new ScriptInvoker();
	}
	
	// ========================================================================
	// NETWORK HELPERS
	// ========================================================================
	
	protected bool IsServer()
	{
		return RBL_NetworkUtils.IsSinglePlayer() || RBL_NetworkUtils.IsServer();
	}

	void Update(float timeSlice)
	{
		// Capture calculations are server-authoritative
		if (!IsServer())
			return;
		
		m_fTimeSinceCheck += timeSlice;
		// Check zones every second
		if (m_fTimeSinceCheck < 1.0)
			return;

		m_fTimeSinceCheck = 0;
		CheckAllZones();
		
		// Broadcast progress to clients
		m_fTimeSinceBroadcast += m_fTimeSinceCheck;
		if (m_fTimeSinceBroadcast >= BROADCAST_INTERVAL)
		{
			m_fTimeSinceBroadcast = 0;
			BroadcastAllCaptureProgress();
		}
	}
	
	protected void BroadcastAllCaptureProgress()
	{
		RBL_NetworkManager netMgr = RBL_NetworkManager.GetInstance();
		if (!netMgr)
			return;
		
		array<string> zoneIDs = new array<string>();
		m_mCaptureProgress.GetKeyArray(zoneIDs);
		
		for (int i = 0; i < zoneIDs.Count(); i++)
		{
			string zoneID = zoneIDs[i];
			float progress = 0;
			m_mCaptureProgress.Find(zoneID, progress);
			
			if (progress > 0)
			{
				ERBLFactionKey capturingFaction = ERBLFactionKey.NONE;
				m_mCapturingFaction.Find(zoneID, capturingFaction);
				
				netMgr.BroadcastCaptureProgress(zoneID, progress, capturingFaction);
			}
		}
	}

	protected void CheckAllZones()
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;

		array<ref RBL_VirtualZone> zones = zoneMgr.GetAllVirtualZones();

		for (int i = 0; i < zones.Count(); i++)
		{
			CheckZoneCapture(zones[i]);
		}
	}

	protected void CheckZoneCapture(RBL_VirtualZone zone)
	{
		if (!zone)
			return;

		string zoneID = zone.GetZoneID();
		vector zonePos = zone.GetZonePosition();
		float captureRadius = zone.GetCaptureRadius();
		ERBLFactionKey currentOwner = zone.GetOwnerFaction();

		// Count players in zone
		int fiaPlayersInZone = CountPlayersInZone(zonePos, captureRadius, ERBLFactionKey.FIA);
		int enemyPresence = zone.GetCurrentGarrison();

		// Determine capture state
		if (fiaPlayersInZone > 0 && currentOwner != ERBLFactionKey.FIA)
		{
			// FIA trying to capture
			if (fiaPlayersInZone > enemyPresence)
			{
				ProcessCapture(zone, ERBLFactionKey.FIA, fiaPlayersInZone - enemyPresence);
			}
			else
			{
				// Contested - decay progress
				DecayCaptureProgress(zoneID);
			}
		}
		else if (fiaPlayersInZone == 0)
		{
			// No FIA presence - decay progress
			DecayCaptureProgress(zoneID);
		}
	}

	protected int CountPlayersInZone(vector zonePos, float radius, ERBLFactionKey faction)
	{
		int count = 0;

		// Get all player controllers
		array<int> playerIds = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(playerIds);

		for (int i = 0; i < playerIds.Count(); i++)
		{
			IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerIds[i]);
			if (!playerEntity)
				continue;

			vector playerPos = playerEntity.GetOrigin();
			float dist = vector.Distance(playerPos, zonePos);

			if (dist <= radius)
				count++;
		}

		return count;
	}

	protected void ProcessCapture(RBL_VirtualZone zone, ERBLFactionKey capturingFaction, int netStrength)
	{
		string zoneID = zone.GetZoneID();

		// Check if starting new capture
		ERBLFactionKey previousCapturing;
		if (!m_mCapturingFaction.Find(zoneID, previousCapturing) || previousCapturing != capturingFaction)
		{
			// Starting fresh capture
			m_mCaptureProgress.Set(zoneID, 0);
			m_mCapturingFaction.Set(zoneID, capturingFaction);
			m_OnCaptureStarted.Invoke(zone, capturingFaction);
			PrintFormat("[RBL] Capture started: %1 by %2", zoneID, typename.EnumToString(ERBLFactionKey, capturingFaction));
		}

		// Progress capture
		float currentProgress = 0;
		m_mCaptureProgress.Find(zoneID, currentProgress);

		float progressGain = RBL_Config.CAPTURE_BASE_RATE + (RBL_Config.CAPTURE_UNIT_BONUS * (netStrength - 1));
		progressGain = Math.Min(progressGain, RBL_Config.CAPTURE_MAX_RATE);
		currentProgress += progressGain;

		m_mCaptureProgress.Set(zoneID, currentProgress);
		m_OnCaptureProgress.Invoke(zone, currentProgress, MAX_CAPTURE_PROGRESS);

		// Check for capture complete
		if (currentProgress >= MAX_CAPTURE_PROGRESS)
		{
			CompleteCapture(zone, capturingFaction);
		}
	}

	protected void CompleteCapture(RBL_VirtualZone zone, ERBLFactionKey newOwner)
	{
		if (!IsServer())
			return;
		
		string zoneID = zone.GetZoneID();
		string zoneName = zone.GetZoneName();
		ERBLFactionKey previousOwner = zone.GetOwnerFaction();
		
		// Clear enemy garrison before transfer
		RBL_GarrisonManager garMgr = RBL_GarrisonManager.GetInstance();
		if (garMgr)
			garMgr.ClearGarrison(zoneID);
		
		// Transfer ownership using network-aware method
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
			zoneMgr.SetZoneOwner(zoneID, newOwner);
		
		// Reset progress
		m_mCaptureProgress.Set(zoneID, 0);
		m_mCapturingFaction.Remove(zoneID);
		
		// Notify UI of capture completion
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.OnCaptureComplete(zoneID, zoneName, newOwner);

		// Award resources (server only)
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr && newOwner == ERBLFactionKey.FIA)
		{
			int moneyReward = zone.GetStrategicValue() / 2;
			int hrReward = 2;
			econMgr.AddMoney(moneyReward);
			econMgr.AddHR(hrReward);
			PrintFormat("[RBL] Capture reward: $%1, +%2 HR", moneyReward, hrReward);
		}

		// Trigger events
		m_OnCaptureComplete.Invoke(zone, previousOwner, newOwner);

		// Notify campaign manager
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
			campaignMgr.AddAggression(RBL_Config.AGGRESSION_PER_ZONE_CAPTURE);

		PrintFormat("[RBL] *** ZONE CAPTURED: %1 ***", zoneID);
		PrintFormat("[RBL] New owner: %1", typename.EnumToString(ERBLFactionKey, newOwner));
	}
	
	// ========================================================================
	// CLIENT: RECEIVE PROGRESS UPDATES
	// ========================================================================
	
	void SetCaptureProgressLocal(string zoneID, float progress, ERBLFactionKey capturingFaction)
	{
		m_mCaptureProgress.Set(zoneID, progress);
		
		if (capturingFaction != ERBLFactionKey.NONE)
			m_mCapturingFaction.Set(zoneID, capturingFaction);
		else
			m_mCapturingFaction.Remove(zoneID);
		
		// Update UI with capture progress
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
		{
			if (progress > 0)
				uiMgr.UpdateCaptureProgress(zoneID, progress, capturingFaction);
			else
				uiMgr.ClearCaptureProgress(zoneID);
		}
		
		// Trigger progress event for other listeners
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			RBL_VirtualZone zone = zoneMgr.GetVirtualZoneByID(zoneID);
			if (zone)
				m_OnCaptureProgress.Invoke(zone, progress, MAX_CAPTURE_PROGRESS);
		}
	}

	protected void DecayCaptureProgress(string zoneID)
	{
		float currentProgress = 0;
		if (!m_mCaptureProgress.Find(zoneID, currentProgress))
			return;

		if (currentProgress <= 0)
			return;

		currentProgress -= RBL_Config.CAPTURE_DECAY_RATE;
		if (currentProgress < 0)
			currentProgress = 0;

		m_mCaptureProgress.Set(zoneID, currentProgress);

		if (currentProgress == 0)
		{
			m_mCapturingFaction.Remove(zoneID);
		}
	}

	float GetCaptureProgress(string zoneID)
	{
		float progress = 0;
		m_mCaptureProgress.Find(zoneID, progress);
		return progress;
	}

	bool IsZoneBeingCaptured(string zoneID)
	{
		return m_mCapturingFaction.Contains(zoneID);
	}

	ScriptInvoker GetOnCaptureStarted() { return m_OnCaptureStarted; }
	ScriptInvoker GetOnCaptureProgress() { return m_OnCaptureProgress; }
	ScriptInvoker GetOnCaptureComplete() { return m_OnCaptureComplete; }
}
