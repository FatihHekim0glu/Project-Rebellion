// ============================================================================
// PROJECT REBELLION - Map Menu Extension
// Injects shop/settings buttons into the map menu hierarchy
// TODO: Re-enable when SCR_MapMenu class name is verified
// ============================================================================

/* DISABLED - SCR_MapMenu class not found in current Arma Reforger version
modded class SCR_MapMenu
{
	protected Widget m_RBLOverlayRoot;
	
protected const ResourceName LAYOUT_PATH_GUID = "{685F68F508FD0CAE}UI/layouts/RBL_MapOverlay.layout";
protected const ResourceName LAYOUT_PATH_ALIAS = "$NewEnfusionProject:UI/layouts/RBL_MapOverlay.layout";
protected const ResourceName LAYOUT_PATH_RELATIVE = "UI/layouts/RBL_MapOverlay.layout";
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		PrintFormat("[RBL_MapOverlay] Map menu opened");
		CreateOverlay();
	}
	
	override void OnMenuClose()
	{
		super.OnMenuClose();
		m_RBLOverlayRoot = null;
	}
	
	protected void CreateOverlay()
	{
		if (m_RBLOverlayRoot)
			return;
		
		Widget root = GetRootWidget();
		if (!root)
		{
			PrintFormat("[RBL_MapOverlay] No root widget");
			return;
		}
		
		Widget existing = root.FindAnyWidget("RBL_MapOverlayRoot");
		if (existing)
		{
			m_RBLOverlayRoot = existing;
			m_RBLOverlayRoot.SetVisible(true);
			m_RBLOverlayRoot.SetEnabled(true);
			return;
		}
		
		Widget parent = FindOverlayParent(root);
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
		{
			PrintFormat("[RBL_MapOverlay] No workspace");
			return;
		}
		
		m_RBLOverlayRoot = TryCreateLayout(workspace, parent, LAYOUT_PATH_GUID);
		if (!m_RBLOverlayRoot)
			m_RBLOverlayRoot = TryCreateLayout(workspace, parent, LAYOUT_PATH_ALIAS);
		if (!m_RBLOverlayRoot)
			m_RBLOverlayRoot = TryCreateLayout(workspace, parent, LAYOUT_PATH_RELATIVE);
		if (!m_RBLOverlayRoot && parent != root)
			m_RBLOverlayRoot = TryCreateLayout(workspace, root, LAYOUT_PATH_GUID);
		if (!m_RBLOverlayRoot && parent != root)
			m_RBLOverlayRoot = TryCreateLayout(workspace, root, LAYOUT_PATH_ALIAS);
		if (!m_RBLOverlayRoot && parent != root)
			m_RBLOverlayRoot = TryCreateLayout(workspace, root, LAYOUT_PATH_RELATIVE);
		
		if (m_RBLOverlayRoot)
		{
			m_RBLOverlayRoot.SetVisible(true);
			m_RBLOverlayRoot.SetEnabled(true);
			PrintFormat("[RBL_MapOverlay] Overlay created");
		}
		else
		{
			PrintFormat("[RBL_MapOverlay] Overlay creation failed");
		}
	}
	
	protected Widget TryCreateLayout(WorkspaceWidget workspace, Widget parent, ResourceName path)
	{
		if (!workspace || path.IsEmpty())
			return null;
		
		if (parent)
			return workspace.CreateWidgets(path, parent);
		
		return workspace.CreateWidgets(path);
	}
	
	protected Widget FindOverlayParent(Widget root)
	{
		array<string> candidates = {};
		candidates.Insert("MapToolbarPanel");
		candidates.Insert("MapToolbar");
		candidates.Insert("MapButtons");
		candidates.Insert("MapControls");
		candidates.Insert("ToolbarPanel");
		candidates.Insert("ControlsPanel");
		candidates.Insert("LeftPanel");
		candidates.Insert("LeftToolbar");
		candidates.Insert("MapLeftPanel");
		candidates.Insert("MapLeftBar");
		
		foreach (string name : candidates)
		{
			Widget found = root.FindAnyWidget(name);
			if (found)
				return found;
		}
		
		return root;
	}
}
*/