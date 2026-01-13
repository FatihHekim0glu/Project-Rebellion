// ============================================================================
// PROJECT REBELLION - Network Component
// Replication and network state management
// ============================================================================

class RBL_NetworkComponentClass : ScriptComponentClass
{
}

class RBL_NetworkComponent : ScriptComponent
{
	protected RplComponent m_RplComponent;
	protected bool m_bNetworkReady;
	
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_bNetworkReady = true;
		
		PrintFormat("[RBL_Network] Network component initialized");
	}
	
	bool IsAuthority()
	{
		if (!m_RplComponent)
			return true;
		return m_RplComponent.Role() == RplRole.Authority;
	}
	
	bool IsProxy()
	{
		return !IsAuthority();
	}
	
	bool IsSinglePlayer()
	{
		// Check if replication is active
		return !Replication.IsRunning();
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
}

// ============================================================================
// STATIC NETWORK UTILITY FUNCTIONS
// ============================================================================
class RBL_NetworkUtils
{
	static bool IsServer()
	{
		// In singleplayer, we're always the server
		if (!Replication.IsRunning())
			return true;
		
		// Check replication role
		return Replication.IsServer();
	}
	
	static bool IsClient()
	{
		return !IsServer();
	}
	
	static bool IsSinglePlayer()
	{
		return !Replication.IsRunning();
	}
	
	static bool IsMultiplayer()
	{
		return Replication.IsRunning();
	}
	
	static bool IsDedicatedServer()
	{
		// Use GetGame to check
		return System.IsCLIParam("-server");
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
		PlayerManager pm = GetGame().GetPlayerManager();
		if (pm)
			return pm.GetPlayerControlledEntity(playerID);
		return null;
	}
}

// ============================================================================
// NETWORK EVENT DATA
// ============================================================================
class RBL_NetworkEventData
{
	int m_iEventType;
	float m_fTimestamp;
	
	void RBL_NetworkEventData()
	{
		m_iEventType = 0;
		m_fTimestamp = 0;
	}
}

class RBL_ZoneNetworkEvent : RBL_NetworkEventData
{
	string ZoneID;
	int NewOwner;
	int PreviousOwner;
	float CaptureProgress;
}

class RBL_EconomyNetworkEvent : RBL_NetworkEventData
{
	int EventFactionKey;
	int MoneyDelta;
	int HRDelta;
}

class RBL_MissionNetworkEvent : RBL_NetworkEventData
{
	string MissionID;
	int MissionType;
	string TargetZone;
	bool IsCompleted;
	bool IsFailed;
}

class RBL_NotificationNetworkEvent : RBL_NetworkEventData
{
	string Message;
	int NotificationColor;
	float Duration;
	int TargetPlayerID;
}
