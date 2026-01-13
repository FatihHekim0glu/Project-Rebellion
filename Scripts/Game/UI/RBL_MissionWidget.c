// ============================================================================
// PROJECT REBELLION - Mission UI Widget
// Displays active missions and available missions
// ============================================================================

// ============================================================================
// MISSION LIST WIDGET (Top-left panel)
// ============================================================================
class RBL_MissionListWidget : RBL_PanelWidget
{
	protected ref array<ref RBL_Mission> m_aDisplayedMissions;
	protected int m_iSelectedIndex;
	protected bool m_bShowAvailable;
	
	void RBL_MissionListWidget()
	{
		m_fPosX = RBL_UISizes.HUD_MARGIN;
		m_fPosY = 100;
		m_fWidth = 280;
		m_fHeight = 200;
		
		m_iBackgroundColor = RBL_UIColors.COLOR_BG_DARK;
		m_iBorderColor = RBL_UIColors.COLOR_BORDER_LIGHT;
		
		m_aDisplayedMissions = new array<ref RBL_Mission>();
		m_iSelectedIndex = 0;
		m_bShowAvailable = false;
		
		m_fUpdateInterval = 0.5;
	}
	
	override void OnUpdate()
	{
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (!missionMgr)
			return;
		
		m_aDisplayedMissions.Clear();
		
		// Get active missions
		array<ref RBL_Mission> activeMissions = missionMgr.GetActiveMissions();
		for (int i = 0; i < activeMissions.Count(); i++)
		{
			if (activeMissions[i])
				m_aDisplayedMissions.Insert(activeMissions[i]);
		}
		
		// Optionally show available missions
		if (m_bShowAvailable)
		{
			array<ref RBL_Mission> availableMissions = missionMgr.GetAvailableMissions();
			for (int i = 0; i < availableMissions.Count(); i++)
			{
				if (availableMissions[i])
					m_aDisplayedMissions.Insert(availableMissions[i]);
			}
		}
		
		// Clamp selection
		if (m_iSelectedIndex >= m_aDisplayedMissions.Count())
			m_iSelectedIndex = Math.Max(0, m_aDisplayedMissions.Count() - 1);
	}
	
	override void Draw()
	{
		// Background
		int bgColor = ApplyAlpha(m_iBackgroundColor, m_fAlpha * 0.85);
		DrawRect(m_fPosX, m_fPosY, m_fWidth, m_fHeight, bgColor);
		
		// Border
		int borderColor = ApplyAlpha(m_iBorderColor, m_fAlpha);
		DrawRectOutline(m_fPosX, m_fPosY, m_fWidth, m_fHeight, borderColor, 1);
		
		// Header
		int headerColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("MissionHeader", m_fPosX + 8, m_fPosY + 8);
		DbgUI.Text("MISSIONS");
		DbgUI.End();
		
		// Separator
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(m_fPosX + 8, m_fPosY + 28, m_fWidth - 16, 1, lineColor);
		
		// Mission list
		float listY = m_fPosY + 36;
		
		if (m_aDisplayedMissions.Count() == 0)
		{
			int noMissionColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
			DbgUI.Begin("NoMissions", m_fPosX + 8, listY);
			DbgUI.Text("No active missions");
			DbgUI.End();
		}
		else
		{
			for (int i = 0; i < m_aDisplayedMissions.Count() && i < 5; i++)
			{
				DrawMissionEntry(m_aDisplayedMissions[i], m_fPosX + 8, listY, m_fWidth - 16, i == m_iSelectedIndex);
				listY += 32;
			}
		}
		
		// Controls hint
		int hintColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
		DbgUI.Begin("MissionHint", m_fPosX + 8, m_fPosY + m_fHeight - 20);
		DbgUI.Text("[TAB] Toggle Available");
		DbgUI.End();
	}
	
	protected void DrawMissionEntry(RBL_Mission mission, float x, float y, float width, bool selected)
	{
		if (!mission)
			return;
		
		// Selection highlight
		if (selected)
		{
			int selectColor = ApplyAlpha(RBL_UIColors.COLOR_BG_HIGHLIGHT, m_fAlpha);
			DrawRect(x - 4, y - 2, width + 8, 28, selectColor);
		}
		
		// Status indicator
		int statusColor = GetStatusColor(mission.GetStatus());
		statusColor = ApplyAlpha(statusColor, m_fAlpha);
		DrawRect(x, y, 3, 24, statusColor);
		
		// Mission name
		int nameColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_PRIMARY, m_fAlpha);
		DbgUI.Begin("Mission_" + mission.GetID(), x + 8, y + 2);
		DbgUI.Text(mission.GetName());
		DbgUI.End();
		
		// Progress or status
		string statusText = "";
		if (mission.IsActive())
		{
			int completed = mission.GetCompletedObjectiveCount();
			int total = mission.GetObjectiveCount();
			statusText = completed.ToString() + "/" + total.ToString();
			
			if (mission.HasTimeLimit())
			{
				float timeLeft = mission.GetTimeRemaining();
				int mins = timeLeft / 60;
				int secs = timeLeft % 60;
				string secPad1 = "";
				if (secs < 10) secPad1 = "0";
				statusText += " [" + mins.ToString() + ":" + secPad1 + secs.ToString() + "]";
			}
		}
		else if (mission.IsAvailable())
		{
			statusText = mission.GetDifficultyString();
		}
		
		int statusTextColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
		DbgUI.Begin("MissionStatus_" + mission.GetID(), x + 8, y + 14);
		DbgUI.Text(statusText);
		DbgUI.End();
	}
	
	protected int GetStatusColor(ERBLMissionStatus status)
	{
		switch (status)
		{
			case ERBLMissionStatus.ACTIVE:
				return RBL_UIColors.COLOR_ACCENT_BLUE;
			case ERBLMissionStatus.AVAILABLE:
				return RBL_UIColors.COLOR_TEXT_SECONDARY;
			case ERBLMissionStatus.COMPLETED:
				return RBL_UIColors.COLOR_ACCENT_GREEN;
			case ERBLMissionStatus.FAILED:
				return RBL_UIColors.COLOR_ACCENT_RED;
			case ERBLMissionStatus.EXPIRED:
				return RBL_UIColors.COLOR_TEXT_MUTED;
		}
		return RBL_UIColors.COLOR_TEXT_MUTED;
	}
	
	// Navigation
	void SelectNext()
	{
		m_iSelectedIndex++;
		if (m_iSelectedIndex >= m_aDisplayedMissions.Count())
			m_iSelectedIndex = 0;
	}
	
	void SelectPrevious()
	{
		m_iSelectedIndex--;
		if (m_iSelectedIndex < 0)
			m_iSelectedIndex = Math.Max(0, m_aDisplayedMissions.Count() - 1);
	}
	
	void ToggleShowAvailable()
	{
		m_bShowAvailable = !m_bShowAvailable;
	}
	
	void ActivateSelected()
	{
		if (m_iSelectedIndex >= m_aDisplayedMissions.Count())
			return;
		
		RBL_Mission mission = m_aDisplayedMissions[m_iSelectedIndex];
		if (!mission)
			return;
		
		if (mission.IsAvailable())
		{
			RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
			if (missionMgr)
				missionMgr.StartMission(mission.GetID());
		}
	}
	
	RBL_Mission GetSelectedMission()
	{
		if (m_iSelectedIndex >= m_aDisplayedMissions.Count())
			return null;
		return m_aDisplayedMissions[m_iSelectedIndex];
	}
}

// ============================================================================
// MISSION OBJECTIVE TRACKER (Mid-right panel)
// ============================================================================
class RBL_ObjectiveTrackerWidget : RBL_PanelWidget
{
	protected ref RBL_Mission m_pTrackedMission;
	
	void RBL_ObjectiveTrackerWidget()
	{
		vector pos = RBL_UILayout.GetCornerPosition(1, 250, 150, RBL_UISizes.HUD_MARGIN);
		m_fPosX = pos[0];
		m_fPosY = pos[1] + 100;
		m_fWidth = 250;
		m_fHeight = 150;
		
		m_iBackgroundColor = RBL_UIColors.COLOR_BG_DARK;
		m_iBorderColor = RBL_UIColors.COLOR_BORDER_DARK;
		
		m_fUpdateInterval = 0.25;
	}
	
	override void OnUpdate()
	{
		RBL_MissionManager missionMgr = RBL_MissionManager.GetInstance();
		if (!missionMgr)
		{
			m_pTrackedMission = null;
			return;
		}
		
		// Track first active mission
		array<ref RBL_Mission> activeMissions = missionMgr.GetActiveMissions();
		if (activeMissions.Count() > 0)
			m_pTrackedMission = activeMissions[0];
		else
			m_pTrackedMission = null;
	}
	
	override void Draw()
	{
		if (!m_pTrackedMission)
			return;
		
		// Background with transparency
		int bgColor = ApplyAlpha(m_iBackgroundColor, m_fAlpha * 0.75);
		DrawRect(m_fPosX, m_fPosY, m_fWidth, m_fHeight, bgColor);
		
		// Mission name
		int nameColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_AMBER, m_fAlpha);
		DbgUI.Begin("TrackedMissionName", m_fPosX + 8, m_fPosY + 8);
		DbgUI.Text(m_pTrackedMission.GetName());
		DbgUI.End();
		
		// Time remaining
		if (m_pTrackedMission.HasTimeLimit())
		{
			float timeLeft = m_pTrackedMission.GetTimeRemaining();
			int timeColor = RBL_UIColors.COLOR_TEXT_SECONDARY;
			if (timeLeft < 60) timeColor = RBL_UIColors.COLOR_ACCENT_RED;
			timeColor = ApplyAlpha(timeColor, m_fAlpha);
			
			int mins = timeLeft / 60;
			int secs = timeLeft % 60;
			string secPad2 = "";
			if (secs < 10) secPad2 = "0";
			string timeStr = mins.ToString() + ":" + secPad2 + secs.ToString();
			
			DbgUI.Begin("TrackedTime", m_fPosX + m_fWidth - 60, m_fPosY + 8);
			DbgUI.Text(timeStr);
			DbgUI.End();
		}
		
		// Objectives
		float objY = m_fPosY + 32;
		array<ref RBL_MissionObjective> objectives = m_pTrackedMission.GetObjectives();
		
		for (int i = 0; i < objectives.Count() && i < 4; i++)
		{
			RBL_MissionObjective obj = objectives[i];
			if (!obj)
				continue;
			
			// Checkbox
			string checkbox = "[ ]";
			if (obj.IsCompleted()) checkbox = "[X]";
			int checkColor = RBL_UIColors.COLOR_TEXT_MUTED;
			if (obj.IsCompleted()) checkColor = RBL_UIColors.COLOR_ACCENT_GREEN;
			checkColor = ApplyAlpha(checkColor, m_fAlpha);
			
			DbgUI.Begin("ObjCheck_" + i.ToString(), m_fPosX + 8, objY);
			DbgUI.Text(checkbox);
			DbgUI.End();
			
			// Description
			int descColor = RBL_UIColors.COLOR_TEXT_PRIMARY;
			if (obj.IsCompleted()) descColor = RBL_UIColors.COLOR_TEXT_MUTED;
			if (obj.IsOptional())
				descColor = RBL_UIColors.COLOR_TEXT_SECONDARY;
			descColor = ApplyAlpha(descColor, m_fAlpha);
			
			string desc = obj.GetDescription();
			if (desc.Length() > 30)
				desc = desc.Substring(0, 27) + "...";
			
			DbgUI.Begin("ObjDesc_" + i.ToString(), m_fPosX + 32, objY);
			DbgUI.Text(desc);
			DbgUI.End();
			
			// Progress for count objectives
			if (obj.GetTargetCount() > 1)
			{
				string progress = obj.GetCurrentCount().ToString() + "/" + obj.GetTargetCount().ToString();
				int progColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
				DbgUI.Begin("ObjProg_" + i.ToString(), m_fPosX + m_fWidth - 40, objY);
				DbgUI.Text(progress);
				DbgUI.End();
			}
			
			objY += 24;
		}
	}
	
	void SetTrackedMission(RBL_Mission mission)
	{
		m_pTrackedMission = mission;
	}
}

// ============================================================================
// MISSION BRIEFING POPUP
// ============================================================================
class RBL_MissionBriefingWidget : RBL_PanelWidget
{
	protected ref RBL_Mission m_pMission;
	protected float m_fFadeProgress;
	
	void RBL_MissionBriefingWidget()
	{
		vector pos = RBL_UILayout.GetCenteredPosition(400, 300);
		m_fPosX = pos[0];
		m_fPosY = pos[1];
		m_fWidth = 400;
		m_fHeight = 300;
		
		m_iBackgroundColor = RBL_UIColors.COLOR_BG_DARK;
		m_iBorderColor = RBL_UIColors.COLOR_ACCENT_AMBER;
		
		m_bVisible = false;
		m_fFadeProgress = 0;
	}
	
	void ShowBriefing(RBL_Mission mission)
	{
		m_pMission = mission;
		m_bVisible = true;
		m_fFadeProgress = 0;
	}
	
	override void Hide()
	{
		super.Hide();
	}
	
	override bool IsVisible()
	{
		return m_bVisible || m_fFadeProgress > 0;
	}
	
	override void Update(float timeSlice)
	{
		super.Update(timeSlice);
		
		if (m_bVisible && m_fFadeProgress < 1.0)
		{
			m_fFadeProgress += timeSlice * 3.0;
			if (m_fFadeProgress > 1.0)
				m_fFadeProgress = 1.0;
		}
		else if (!m_bVisible && m_fFadeProgress > 0)
		{
			m_fFadeProgress -= timeSlice * 3.0;
			if (m_fFadeProgress < 0)
				m_fFadeProgress = 0;
		}
		
		m_fAlpha = m_fFadeProgress;
	}
	
	override void Draw()
	{
		if (!m_pMission || m_fAlpha <= 0.01)
			return;
		
		// Backdrop
		int backdropColor = ApplyAlpha(ARGB(150, 0, 0, 0), m_fAlpha);
		DrawRect(0, 0, RBL_UISizes.REFERENCE_WIDTH, RBL_UISizes.REFERENCE_HEIGHT, backdropColor);
		
		// Panel
		int bgColor = ApplyAlpha(m_iBackgroundColor, m_fAlpha);
		DrawRect(m_fPosX, m_fPosY, m_fWidth, m_fHeight, bgColor);
		
		int borderColor = ApplyAlpha(m_iBorderColor, m_fAlpha);
		DrawRectOutline(m_fPosX, m_fPosY, m_fWidth, m_fHeight, borderColor, 2);
		
		// Header
		int headerColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_AMBER, m_fAlpha);
		DbgUI.Begin("BriefingHeader", m_fPosX + 16, m_fPosY + 16);
		DbgUI.Text("MISSION BRIEFING");
		DbgUI.End();
		
		// Mission name
		int nameColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_BRIGHT, m_fAlpha);
		DbgUI.Begin("BriefingName", m_fPosX + 16, m_fPosY + 44);
		DbgUI.Text(m_pMission.GetName());
		DbgUI.End();
		
		// Type and difficulty
		string typeInfo = m_pMission.GetTypeString() + " | " + m_pMission.GetDifficultyString();
		int infoColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_SECONDARY, m_fAlpha);
		DbgUI.Begin("BriefingInfo", m_fPosX + 16, m_fPosY + 64);
		DbgUI.Text(typeInfo);
		DbgUI.End();
		
		// Separator
		int lineColor = ApplyAlpha(RBL_UIColors.COLOR_BORDER_DARK, m_fAlpha);
		DrawRect(m_fPosX + 16, m_fPosY + 84, m_fWidth - 32, 1, lineColor);
		
		// Briefing text
		int briefColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_PRIMARY, m_fAlpha);
		DbgUI.Begin("BriefingText", m_fPosX + 16, m_fPosY + 96);
		DbgUI.Text(m_pMission.GetBriefing());
		DbgUI.End();
		
		// Objectives header
		DbgUI.Begin("BriefingObjHeader", m_fPosX + 16, m_fPosY + 160);
		DbgUI.Text("OBJECTIVES:");
		DbgUI.End();
		
		// Objectives list
		float objY = m_fPosY + 180;
		array<ref RBL_MissionObjective> objectives = m_pMission.GetObjectives();
		for (int i = 0; i < objectives.Count() && i < 3; i++)
		{
			RBL_MissionObjective obj = objectives[i];
			if (!obj)
				continue;
			
			string prefix = "  - ";
			if (obj.IsOptional()) prefix = "  (Optional) ";
			DbgUI.Begin("BriefingObj_" + i.ToString(), m_fPosX + 16, objY);
			DbgUI.Text(prefix + obj.GetDescription());
			DbgUI.End();
			objY += 18;
		}
		
		// Reward
		RBL_MissionReward reward = m_pMission.GetReward();
		if (reward)
		{
			int rewardColor = ApplyAlpha(RBL_UIColors.COLOR_ACCENT_GREEN, m_fAlpha);
			DbgUI.Begin("BriefingReward", m_fPosX + 16, m_fPosY + m_fHeight - 50);
			DbgUI.Text("REWARD: " + reward.GetRewardString());
			DbgUI.End();
		}
		
		// Controls
		int hintColor = ApplyAlpha(RBL_UIColors.COLOR_TEXT_MUTED, m_fAlpha);
		DbgUI.Begin("BriefingHint", m_fPosX + 16, m_fPosY + m_fHeight - 24);
		DbgUI.Text("[ENTER] Accept   [ESC] Decline");
		DbgUI.End();
	}
	
	RBL_Mission GetMission() { return m_pMission; }
	bool IsBriefingVisible() { return m_bVisible; }
}

