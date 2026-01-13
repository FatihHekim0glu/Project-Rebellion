// ============================================================================
// PROJECT REBELLION - On-Screen HUD (Debug Text)
// Shows status info on screen using DbgUI
// ============================================================================

class RBL_ScreenHUD
{
	protected static ref RBL_ScreenHUD s_Instance;
	protected bool m_bEnabled;
	protected float m_fUpdateTimer;
	protected const float UPDATE_INTERVAL = 0.25;

	static RBL_ScreenHUD GetInstance()
	{
		if (!s_Instance)
			s_Instance = new RBL_ScreenHUD();
		return s_Instance;
	}

	void RBL_ScreenHUD()
	{
		m_bEnabled = true;
		m_fUpdateTimer = 0;
	}

	void Update(float timeSlice)
	{
		if (!m_bEnabled)
			return;

		m_fUpdateTimer += timeSlice;
		if (m_fUpdateTimer < UPDATE_INTERVAL)
			return;

		m_fUpdateTimer = 0;
		DrawHUD();
	}

	protected void DrawHUD()
	{
		// Use DbgUI for simple on-screen text
		DbgUI.Begin("Project Rebellion", 10, 10);

		// Economy
		RBL_EconomyManager econMgr = RBL_EconomyManager.GetInstance();
		if (econMgr)
		{
			DbgUI.Text("Money: $" + econMgr.GetMoney().ToString());
			DbgUI.Text("HR: " + econMgr.GetHR().ToString());
		}

		DbgUI.Text("---");

		// Zones
		RBL_ZoneManager zoneMgr = RBL_ZoneManager.GetInstance();
		if (zoneMgr)
		{
			int fia = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.FIA);
			int enemy = zoneMgr.GetZoneCountByFaction(ERBLFactionKey.USSR);
			DbgUI.Text("FIA Zones: " + fia.ToString());
			DbgUI.Text("Enemy Zones: " + enemy.ToString());

			// Nearest zone info (with null safety)
			IEntity controlled = null;
			if (GetGame())
			{
				PlayerController playerController = GetGame().GetPlayerController();
				if (playerController)
					controlled = playerController.GetControlledEntity();
			}
			
			if (controlled)
			{
				RBL_VirtualZone nearest = zoneMgr.GetNearestVirtualZone(controlled.GetOrigin());
				if (nearest)
				{
					DbgUI.Text("---");
					DbgUI.Text("Nearest: " + nearest.GetZoneID());
					float dist = vector.Distance(controlled.GetOrigin(), nearest.GetZonePosition());
					DbgUI.Text("Distance: " + Math.Round(dist).ToString() + "m");
					string owner = typename.EnumToString(ERBLFactionKey, nearest.GetOwnerFaction());
					DbgUI.Text("Owner: " + owner);

					// Capture progress
					RBL_CaptureManager capMgr = RBL_CaptureManager.GetInstance();
					if (capMgr)
					{
						float progress = capMgr.GetCaptureProgress(nearest.GetZoneID());
						if (progress > 0)
						{
							DbgUI.Text("CAPTURING: " + Math.Round(progress).ToString() + "%");
						}
						else if (dist <= nearest.GetCaptureRadius())
						{
							if (nearest.GetOwnerFaction() == ERBLFactionKey.FIA)
								DbgUI.Text("[FRIENDLY ZONE]");
							else
								DbgUI.Text("[ENTER TO CAPTURE]");
						}
					}
				}
			}
		}

		DbgUI.Text("---");

		// War status
		RBL_CampaignManager campaignMgr = RBL_CampaignManager.GetInstance();
		if (campaignMgr)
		{
			DbgUI.Text("War Level: " + campaignMgr.GetWarLevel().ToString() + "/10");
			DbgUI.Text("Alert: " + campaignMgr.GetAggression().ToString() + "%");
		}

		// Undercover status
		DbgUI.Text("---");
		RBL_UndercoverSystem undercover = RBL_UndercoverSystem.GetInstance();
		if (undercover && GetGame())
		{
			PlayerController pc = GetGame().GetPlayerController();
			if (pc)
			{
				int playerID = pc.GetPlayerId();
				RBL_PlayerCoverState coverState = undercover.GetPlayerState(playerID);
				if (coverState)
				{
					string statusColor = "";
					switch (coverState.m_eCurrentStatus)
					{
						case ERBLCoverStatus.HIDDEN: statusColor = "[SAFE] "; break;
						case ERBLCoverStatus.SUSPICIOUS: statusColor = "[!] "; break;
						case ERBLCoverStatus.SPOTTED: statusColor = "[!!] "; break;
						case ERBLCoverStatus.COMPROMISED: statusColor = "[!!!] "; break;
					}
					DbgUI.Text("Cover: " + statusColor + coverState.GetStatusString());
					DbgUI.Text("Suspicion: " + Math.Round(coverState.m_fSuspicionLevel * 100).ToString() + "%");
					
					if (coverState.m_bNearEnemy)
						DbgUI.Text("Enemy: " + Math.Round(coverState.m_fNearestEnemyDistance).ToString() + "m");
					
					string factors = coverState.GetFactorsString();
					if (factors != "None")
						DbgUI.Text("Risks: " + factors);
				}
				else
				{
					DbgUI.Text("Cover: HIDDEN");
				}
			}
		}

		DbgUI.Text("---");
		DbgUI.Text("Console: RBL_DebugCommands");
		DbgUI.Text("  .PrintStatus()");
		DbgUI.Text("  .OpenShop()");
		DbgUI.Text("  .Buy(\"itemid\")");

		DbgUI.End();
	}

	void SetEnabled(bool enabled) { m_bEnabled = enabled; }
	bool IsEnabled() { return m_bEnabled; }
	void Toggle() { m_bEnabled = !m_bEnabled; }
}
