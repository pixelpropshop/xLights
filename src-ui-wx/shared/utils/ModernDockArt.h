#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/aui/dockart.h>
#include <wx/aui/auibar.h>
#include <wx/aui/framemanager.h>

// Flat dock art shared by the app's AUI managers: solid captions in the normal
// UI font, colors derived from the system theme (so dark mode stays dark), and
// a single-line gripper instead of the stock dotted one.
class ModernDockArt : public wxAuiDefaultDockArt {
public:
    ModernDockArt();

    wxNODISCARD wxAuiDockArt* Clone() override;
    void UpdateColoursFromSystem() override;
    void DrawGripper(wxDC& dc, wxWindow* window, const wxRect& rect, wxAuiPaneInfo& pane) override;
};

// Toolbar art to match: a plain background instead of the stock gradient, and
// the same single-line gripper.
class ModernToolBarArt : public wxAuiDefaultToolBarArt {
public:
    wxNODISCARD wxAuiToolBarArt* Clone() override;
    void DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override;
    void DrawGripper(wxDC& dc, wxWindow* wnd, const wxRect& rect) override;
};
