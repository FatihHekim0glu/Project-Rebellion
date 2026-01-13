// ============================================================================
// PROJECT REBELLION - Network Manager
// Central coordinator for all multiplayer networking operations
// ============================================================================

class RBL_NetworkManagerClass : GenericEntityClass
{
}

class RBL_NetworkManager : GenericEntity
{
	protected static RBL_NetworkManager s_Instance;
	
	protected RplComponent m_RplComponent;
	protected bool m_bIsAuthority;
	protected bool m_bInitialized;
	
	// Replicated game state
	[RplProp(onRplName: "OnRpl_Money")]
	protected int m_iRplMoney;
	
	[RplProp(onRplName: "OnRpl_HR")]
	protected int m_iRplHR;
	
	[RplProp(onRplName: "OnRpl_WarLevel")]
	protected int m_iRplWarLevel;
	
	[RplProp(onRplName: "OnRpl_Aggression")]
	protected int m_iRplAggression;
	
	// Events
	protected ref ScriptInvoker m_OnNetworkStateChanged;
	protected ref ScriptInvoker m_OnPlayerConnected;
	protected ref ScriptInvoker m_OnPlayerDisconnected;
	protected ref ScriptInvoker m_OnPurchaseRequest;
	protected ref ScriptInvoker m_OnPurchaseResult;
	
	// Request tracking
	protected ref map<int, ref array<string>> m_mPendingRequests;
	protected int m_iNextRequestID;
	
	static RBL_NetworkManager GetInstance()
	{
		return s_Instance;
	}
	
	void RBL_NetworkManager(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
		
		m_OnNetworkStateChanged = new ScriptInvoker();
		m_OnPlayerConnected = new ScriptInvoker();
		m_OnPlayerDisconnected = new ScriptInvoker();
		m_OnPurchaseRequest = new ScriptInvoker();
		m_OnPurchaseResult = new ScriptInvoker();
		
		m_mPendingRequests = new map<int, ref array<string>>();
		m_iNextRequestID = 1;
		
		m_bInitialized = false;
		
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
	}
	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		UpdateAuthorityState();
		
		m_bInitialized = true;
		
		PrintFormat("[RBL_NetworkMgr] Initialized. Authority: %1", m_bIsAuthority);
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_bInitialized)
			return;
		
		if (m_bIsAuthority)
			SyncStateFromManagers();
	}
	
	protected void UpdateAuthorityState()
	{
		if (m_RplComponent)
			m_bIsAuthority = m_RplComponent.IsMaster();
		else
			m_bIsAuthority = RBL_NetworkUtils.IsSinglePlayer();
	}
	
	// ========================================================================
	// AUTHORITY CHECKS
	// ========================================================================
	
	bool IsAuthority()
	{
		return m_bIsAuthority;
	}
	
	bool IsServer()
	{
		return m_bIsAuthority;
	}
	
	bool IsClient()
	{
		return !m_bIsAuthority;
	}
	
	bool CanModifyState()
	{
		return m_bIsAuthority || RBL_NetworkUtils.IsSinglePlayer();
	}
	
	// ========================================================================
	// STATE SYNCHRONIZATION (Server -> Clients)
	// ========================================================================
	
	protected void SyncStateFromManagers()
	{
		if (!m_bIsAuthority)
			return;
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			int newMoney = econMgr.GetMoney();
			int newHR = econMgr.GetHR();
			
			if (newMoney != m_iRplMoney || newHR != m_iRplHR)
			{
				m_iRplMoney = newMoney;
				m_iRplHR = newHR;
				Replication.BumpMe();
			}
		}
		
		RBL_CampaignManager campMgr = RBL_CampaignManager.GetInstance();
		if (campMgr)
		{
			int newWarLevel = campMgr.GetWarLevel();
			int newAggression = campMgr.GetAggression();
			
			if (newWarLevel != m_iRplWarLevel || newAggression != m_iRplAggression)
			{
				m_iRplWarLevel = newWarLevel;
				m_iRplAggression = newAggression;
				Replication.BumpMe();
			}
		}
	}
	
	// Replication callbacks
	protected void OnRpl_Money()
	{
		if (!m_bIsAuthority)
		{
			RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
			if (econMgr)
				econMgr.SetMoney(m_iRplMoney);
		}
		m_OnNetworkStateChanged.Invoke("Money", m_iRplMoney);
	}
	
	protected void OnRpl_HR()
	{
		if (!m_bIsAuthority)
		{
			RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
			if (econMgr)
				econMgr.SetHR(m_iRplHR);
		}
		m_OnNetworkStateChanged.Invoke("HR", m_iRplHR);
	}
	
	protected void OnRpl_WarLevel()
	{
		m_OnNetworkStateChanged.Invoke("WarLevel", m_iRplWarLevel);
	}
	
	protected void OnRpl_Aggression()
	{
		m_OnNetworkStateChanged.Invoke("Aggression", m_iRplAggression);
	}
	
	// ========================================================================
	// PURCHASE SYSTEM (Client -> Server -> Client)
	// ========================================================================
	
	void RequestPurchase(int playerID, string itemID, int moneyCost, int hrCost)
	{
		if (m_bIsAuthority)
		{
			ProcessPurchaseOnServer(playerID, itemID, moneyCost, hrCost);
		}
		else
		{
			Rpc(RpcAsk_Purchase, playerID, itemID, moneyCost, hrCost);
		}
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_Purchase(int playerID, string itemID, int moneyCost, int hrCost)
	{
		ProcessPurchaseOnServer(playerID, itemID, moneyCost, hrCost);
	}
	
	protected void ProcessPurchaseOnServer(int playerID, string itemID, int moneyCost, int hrCost)
	{
		if (!m_bIsAuthority)
			return;
		
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (!econMgr)
		{
			SendPurchaseResult(playerID, itemID, false, "Economy system unavailable");
			return;
		}
		
		if (!econMgr.CanAffordPurchase(moneyCost, hrCost))
		{
			SendPurchaseResult(playerID, itemID, false, "Insufficient funds");
			return;
		}
		
		bool success = econMgr.TryPurchase(itemID, moneyCost, hrCost);
		
		if (success)
		{
			DeliverItemToPlayer(playerID, itemID);
			SendPurchaseResult(playerID, itemID, true, "Purchase successful");
		}
		else
		{
			SendPurchaseResult(playerID, itemID, false, "Purchase failed");
		}
	}
	
	protected void DeliverItemToPlayer(int playerID, string itemID)
	{
		RBL_ShopManager shopMgr = RBL_ShopManager.GetInstance();
		if (!shopMgr)
			return;
		
		RBL_ItemDelivery delivery = RBL_ItemDelivery.GetInstance();
		if (!delivery)
			return;
		
		// Use centralized lookup from ShopManager
		RBL_ShopItem item = shopMgr.GetItemByID(itemID);
		
		if (item)
		{
			// Use the new DeliverShopItem method that works with getter methods
			delivery.DeliverShopItem(item, playerID);
			PrintFormat("[RBL_Network] Delivered %1 to player %2", item.GetDisplayName(), playerID);
		}
		else
		{
			PrintFormat("[RBL_Network] Item not found for delivery: %1", itemID);
		}
	}
	
	protected void SendPurchaseResult(int playerID, string itemID, bool success, string message)
	{
		m_OnPurchaseResult.Invoke(playerID, itemID, success, message);
		
		Rpc(RpcDo_PurchaseResult, playerID, itemID, success, message);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_PurchaseResult(int playerID, string itemID, bool success, string message)
	{
		if (RBL_NetworkUtils.GetLocalPlayerID() == playerID)
		{
			m_OnPurchaseResult.Invoke(playerID, itemID, success, message);
			
			if (success)
				RBL_Notifications.ItemPurchased(itemID, 0);
			else
				RBL_Notifications.ErrorMessage(message);
		}
	}
	
	// ========================================================================
	// ZONE CAPTURE (Server -> Clients)
	// ========================================================================
	
	void BroadcastZoneCapture(string zoneID, int previousOwner, int newOwner)
	{
		if (!m_bIsAuthority)
			return;
		
		Rpc(RpcDo_ZoneCaptured, zoneID, previousOwner, newOwner);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_ZoneCaptured(string zoneID, int previousOwner, int newOwner)
	{
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (!zoneMgr)
			return;
		
		// Update zone on clients
		RBL_VirtualZone zone = zoneMgr.GetVirtualZoneByID(zoneID);
		if (zone && !m_bIsAuthority)
		{
			zone.SetOwnerFaction(newOwner);
		}
		
		// Show notification
		string zoneName = zoneID;
		if (zone)
			zoneName = zone.GetZoneName();
		
		if (newOwner == ERBLFactionKey.FIA)
			RBL_Notifications.ZoneCaptured(zoneName);
		else
			RBL_Notifications.ZoneLost(zoneName);
	}
	
	void BroadcastCaptureProgress(string zoneID, float progress, int capturingFaction)
	{
		if (!m_bIsAuthority)
			return;
		
		Rpc(RpcDo_CaptureProgress, zoneID, progress, capturingFaction);
	}
	
	[RplRpc(RplChannel.Unreliable, RplRcver.Broadcast)]
	protected void RpcDo_CaptureProgress(string zoneID, float progress, int capturingFaction)
	{
		// Update local capture UI
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.UpdateCaptureProgress(zoneID, progress, capturingFaction);
	}
	
	// ========================================================================
	// NOTIFICATION BROADCAST (Server -> Clients)
	// ========================================================================
	
	void BroadcastNotification(string message, int color, float duration)
	{
		if (!m_bIsAuthority)
			return;
		
		Rpc(RpcDo_Notification, message, color, duration, -1);
	}
	
	void SendNotificationToPlayer(int playerID, string message, int color, float duration)
	{
		if (!m_bIsAuthority)
			return;
		
		Rpc(RpcDo_Notification, message, color, duration, playerID);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Notification(string message, int color, float duration, int targetPlayerID)
	{
		// If targetPlayerID is -1, show to all; otherwise only to target
		if (targetPlayerID != -1 && RBL_NetworkUtils.GetLocalPlayerID() != targetPlayerID)
			return;
		
		RBL_UIManager uiMgr = RBL_UIManager.GetInstance();
		if (uiMgr)
			uiMgr.ShowNotification(message, color, duration);
	}
	
	// ========================================================================
	// MISSION SYNC (Server -> Clients)
	// ========================================================================
	
	void BroadcastMissionStarted(string missionID, string missionName)
	{
		if (!m_bIsAuthority)
			return;
		
		Rpc(RpcDo_MissionStarted, missionID, missionName);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_MissionStarted(string missionID, string missionName)
	{
		RBL_Notifications.MissionReceived(missionName);
	}
	
	void BroadcastMissionCompleted(string missionID, string missionName)
	{
		if (!m_bIsAuthority)
			return;
		
		Rpc(RpcDo_MissionCompleted, missionID, missionName);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_MissionCompleted(string missionID, string missionName)
	{
		RBL_Notifications.MissionComplete(missionName);
	}
	
	void BroadcastMissionFailed(string missionID, string missionName)
	{
		if (!m_bIsAuthority)
			return;
		
		Rpc(RpcDo_MissionFailed, missionID, missionName);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_MissionFailed(string missionID, string missionName)
	{
		RBL_Notifications.MissionFailed(missionName);
	}
	
	// ========================================================================
	// GAME STATE BROADCAST (Server -> Clients)
	// ========================================================================
	
	void BroadcastWarLevelChange(int newLevel)
	{
		if (!m_bIsAuthority)
			return;
		
		m_iRplWarLevel = newLevel;
		Replication.BumpMe();
		
		Rpc(RpcDo_WarLevelChanged, newLevel);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_WarLevelChanged(int newLevel)
	{
		RBL_Notifications.WarLevelIncreased(newLevel);
	}
	
	void BroadcastQRFIncoming(string qrfType, string targetZone)
	{
		if (!m_bIsAuthority)
			return;
		
		Rpc(RpcDo_QRFIncoming, qrfType, targetZone);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_QRFIncoming(string qrfType, string targetZone)
	{
		RBL_Notifications.QRFIncoming(qrfType, targetZone);
	}
	
	void BroadcastGameEnd(bool victory)
	{
		if (!m_bIsAuthority)
			return;
		
		Rpc(RpcDo_GameEnd, victory);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_GameEnd(bool victory)
	{
		RBL_VictoryManager victoryMgr = RBL_VictoryManager.GetInstance();
		if (victoryMgr)
		{
			if (victory)
				victoryMgr.TriggerVictory();
			else
				victoryMgr.TriggerDefeat();
		}
	}
	
	// ========================================================================
	// EVENT ACCESSORS
	// ========================================================================
	
	ScriptInvoker GetOnNetworkStateChanged() { return m_OnNetworkStateChanged; }
	ScriptInvoker GetOnPlayerConnected() { return m_OnPlayerConnected; }
	ScriptInvoker GetOnPlayerDisconnected() { return m_OnPlayerDisconnected; }
	ScriptInvoker GetOnPurchaseRequest() { return m_OnPurchaseRequest; }
	ScriptInvoker GetOnPurchaseResult() { return m_OnPurchaseResult; }
	
	// ========================================================================
	// DEBUG
	// ========================================================================
	
	void PrintStatus()
	{
		PrintFormat("[RBL_NetworkMgr] === STATUS ===");
		PrintFormat("  Authority: %1", m_bIsAuthority);
		PrintFormat("  Initialized: %1", m_bInitialized);
		PrintFormat("  Rpl Money: %1", m_iRplMoney);
		PrintFormat("  Rpl HR: %1", m_iRplHR);
		PrintFormat("  Rpl War Level: %1", m_iRplWarLevel);
		PrintFormat("  Rpl Aggression: %1", m_iRplAggression);
		RBL_NetworkUtils.PrintNetworkStatus();
	}
}

// ============================================================================
// SINGLETON WRAPPER (for non-entity access)
// ============================================================================
class RBL_Network
{
	protected static ref RBL_Network s_Instance;
	protected RBL_NetworkManager m_Manager;
	protected bool m_bFallbackMode;
	
	static RBL_Network GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_Network();
		return s_Instance;
	}
	
	void RBL_Network()
	{
		m_bFallbackMode = false;
	}
	
	protected RBL_NetworkManager GetManager()
	{
		if (!m_Manager)
			m_Manager = RBL_NetworkManager.GetInstance();
		return m_Manager;
	}
	
	bool IsServer()
	{
		RBL_NetworkManager mgr = GetManager();
		if (mgr)
			return mgr.IsServer();
		return RBL_NetworkUtils.IsServer();
	}
	
	bool IsClient()
	{
		return !IsServer();
	}
	
	bool CanModifyState()
	{
		RBL_NetworkManager mgr = GetManager();
		if (mgr)
			return mgr.CanModifyState();
		return RBL_NetworkUtils.IsSinglePlayer() || RBL_NetworkUtils.IsServer();
	}
	
	void RequestPurchase(string itemID, int moneyCost, int hrCost)
	{
		RBL_NetworkManager mgr = GetManager();
		if (mgr)
		{
			int playerID = RBL_NetworkUtils.GetLocalPlayerID();
			mgr.RequestPurchase(playerID, itemID, moneyCost, hrCost);
		}
	}
	
	void BroadcastNotification(string message, int color, float duration)
	{
		RBL_NetworkManager mgr = GetManager();
		if (mgr)
			mgr.BroadcastNotification(message, color, duration);
	}
	
	void BroadcastZoneCapture(string zoneID, int previousOwner, int newOwner)
	{
		RBL_NetworkManager mgr = GetManager();
		if (mgr)
			mgr.BroadcastZoneCapture(zoneID, previousOwner, newOwner);
	}
}

