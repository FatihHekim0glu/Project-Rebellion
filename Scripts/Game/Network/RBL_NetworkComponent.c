// ============================================================================
// PROJECT REBELLION - Network Component
// Base replication component with authority helpers for multiplayer
// ============================================================================

class RBL_NetworkComponentClass : ScriptComponentClass
{
}

class RBL_NetworkComponent : ScriptComponent
{
	protected RplComponent m_RplComponent;
	protected bool m_bIsAuthority;
	protected bool m_bIsProxy;
	protected bool m_bNetworkReady;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		UpdateAuthorityState();
		
		m_bNetworkReady = true;
		PrintFormat("[RBL_Network] Component initialized. Authority: %1, Proxy: %2", m_bIsAuthority, m_bIsProxy);
	}
	
	protected void UpdateAuthorityState()
	{
		if (m_RplComponent)
		{
			m_bIsAuthority = m_RplComponent.IsMaster();
			m_bIsProxy = m_RplComponent.IsProxy();
		}
		else
		{
			m_bIsAuthority = IsSinglePlayer();
			m_bIsProxy = false;
		}
	}
	
	// ========================================================================
	// AUTHORITY CHECKS
	// ========================================================================
	
	bool IsAuthority()
	{
		UpdateAuthorityState();
		return m_bIsAuthority;
	}
	
	bool IsProxy()
	{
		UpdateAuthorityState();
		return m_bIsProxy;
	}
	
	bool IsServer()
	{
		return IsAuthority();
	}
	
	bool IsClient()
	{
		return !IsAuthority();
	}
	
	bool IsSinglePlayer()
	{
		RplSession session = RplSession.Global();
		if (!session)
			return true;
		
		return !session.IsActive();
	}
	
	bool IsMultiplayer()
	{
		return !IsSinglePlayer();
	}
	
	bool IsNetworkReady()
	{
		return m_bNetworkReady;
	}
	
	RplComponent GetRplComponent()
	{
		return m_RplComponent;
	}
	
	// ========================================================================
	// PLAYER HELPERS
	// ========================================================================
	
	static int GetLocalPlayerID()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
			return pc.GetPlayerId();
		return -1;
	}
	
	static IEntity GetLocalPlayerEntity()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
			return pc.GetControlledEntity();
		return null;
	}
	
	static bool IsLocalPlayer(int playerID)
	{
		return playerID == GetLocalPlayerID();
	}
	
	static array<int> GetAllPlayerIDs()
	{
		array<int> playerIDs = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		return playerIDs;
	}
	
	static int GetPlayerCount()
	{
		array<int> playerIDs = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		return playerIDs.Count();
	}
	
	// ========================================================================
	// RPC HELPERS
	// ========================================================================
	
	protected void BroadcastToAllClients(string methodName, Managed context = null)
	{
		if (!m_RplComponent)
			return;
		
		Rpc(RpcDo_BroadcastMethod, methodName);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_BroadcastMethod(string methodName)
	{
		// Override in subclasses to handle specific broadcasts
		PrintFormat("[RBL_Network] Received broadcast: %1", methodName);
	}
}

// ============================================================================
// STATIC NETWORK UTILITY FUNCTIONS
// ============================================================================
class RBL_NetworkUtils
{
	static bool IsServer()
	{
		RplSession session = RplSession.Global();
		if (!session || !session.IsActive())
			return true;
		
		return session.IsDedicated() || !session.IsProxy();
	}
	
	static bool IsClient()
	{
		return !IsServer();
	}
	
	static bool IsSinglePlayer()
	{
		RplSession session = RplSession.Global();
		if (!session)
			return true;
		
		return !session.IsActive();
	}
	
	static bool IsMultiplayer()
	{
		return !IsSinglePlayer();
	}
	
	static bool IsDedicatedServer()
	{
		RplSession session = RplSession.Global();
		if (!session)
			return false;
		
		return session.IsDedicated();
	}
	
	static bool IsListenServer()
	{
		return IsServer() && !IsDedicatedServer() && IsMultiplayer();
	}
	
	static int GetLocalPlayerID()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
			return pc.GetPlayerId();
		return -1;
	}
	
	static IEntity GetPlayerEntity(int playerID)
	{
		return GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
	}
	
	static string GetPlayerName(int playerID)
	{
		return GetGame().GetPlayerManager().GetPlayerName(playerID);
	}
	
	static void GetAllPlayers(out array<int> playerIDs)
	{
		if (!playerIDs)
			playerIDs = new array<int>();
		else
			playerIDs.Clear();
		
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
	}
	
	static int GetPlayerCount()
	{
		array<int> players = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(players);
		return players.Count();
	}
	
	static void PrintNetworkStatus()
	{
		PrintFormat("[RBL_Network] === NETWORK STATUS ===");
		PrintFormat("  Server: %1", IsServer());
		PrintFormat("  Client: %1", IsClient());
		PrintFormat("  SinglePlayer: %1", IsSinglePlayer());
		PrintFormat("  Multiplayer: %1", IsMultiplayer());
		PrintFormat("  Dedicated: %1", IsDedicatedServer());
		PrintFormat("  Players: %1", GetPlayerCount());
		PrintFormat("  Local Player ID: %1", GetLocalPlayerID());
	}
}

// ============================================================================
// NETWORK EVENT DATA STRUCTURES
// ============================================================================

class RBL_NetworkEventData
{
	int SenderPlayerID;
	float Timestamp;
	
	void RBL_NetworkEventData()
	{
		SenderPlayerID = RBL_NetworkUtils.GetLocalPlayerID();
		Timestamp = System.GetTickCount();
	}
}

class RBL_EconomyNetworkEvent : RBL_NetworkEventData
{
	int Money;
	int HR;
	string ItemID;
	int Cost;
	bool Success;
}

class RBL_ZoneNetworkEvent : RBL_NetworkEventData
{
	string ZoneID;
	int OwnerFaction;
	float CaptureProgress;
	int CapturingFaction;
}

class RBL_MissionNetworkEvent : RBL_NetworkEventData
{
	string MissionID;
	int Status;
	float Progress;
}

class RBL_NotificationNetworkEvent : RBL_NetworkEventData
{
	string Message;
	int Color;
	float Duration;
	int TargetPlayerID;
}

