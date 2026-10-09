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

#include <wx/aui/framemanager.h>
#include <wx/timer.h>
#include <wx/window.h>

#include <functional>
#include <vector>

class wxMenu;

// A tab strip docked in the sequencer's wxAuiManager. Windows added to it share
// the strip's dock slot and only the selected one is shown. Nothing is
// reparented: every window stays a pane of the manager, so lookups by pane name,
// the View menu and saved perspectives keep working unchanged.
class SequencerWindowTabs : public wxWindow {
public:
    static const wxString PANE_NAME;

    SequencerWindowTabs(wxWindow* parent, wxAuiManager* mgr);

    static bool CanBeTabbed(const wxString& name);

    // Adds the "Window Tabs" entries to a menu. The ids are shared by every
    // menu built here, so one handler serves the View menu and the popup.
    static void AppendMenuItems(wxMenu* menu);
    static bool IsMenuCommand(int id);
    void UpdateMenuChecks(wxMenu* menu) const;
    void HandleMenuCommand(int id);

    void SetOnChanged(std::function<void()> cb) {
        _onChanged = std::move(cb);
    }
    void SetOnSelected(std::function<void(const wxString&)> cb) {
        _onSelected = std::move(cb);
    }

    bool Contains(const wxString& name) const;
    void AddWindow(const wxString& name);
    void SelectWindow(const wxString& name);
    // Takes the window out of the tabs and leaves it showing where it is.
    void RemoveWindow(const wxString& name);
    // Takes the window out of the tabs and hides it.
    void CloseWindow(const wxString& name);
    // Takes the window out of the tabs and floats it.
    void DetachWindow(const wxString& name);
    void TabSettingsWindows();
    void RemoveAll();

    // The tabs are part of a perspective: "selected|name,name,...", empty for
    // none. SetState only records it; Reapply puts the panes in that state.
    std::string GetState() const;
    void SetState(const std::string& state);
    // Puts the tabbed windows back in the strip's slot after a perspective load
    // has moved or re-shown them.
    void Reapply();
    void Shutdown();

private:
    struct Slot {
        int direction = wxAUI_DOCK_LEFT;
        int layer = 2;
        int row = 0;
        bool operator==(const Slot& o) const {
            return direction == o.direction && layer == o.layer && row == o.row;
        }
        bool operator!=(const Slot& o) const {
            return !(*this == o);
        }
    };

    static Slot SlotOf(const wxAuiPaneInfo& pane);
    wxAuiPaneInfo& Bar();
    wxAuiPaneInfo& Pane(const wxString& name);
    wxString LabelFor(const wxString& name);
    wxString FullLabelFor(const wxString& name);
    bool IsVertical() const;

    bool Arrange();
    void Commit();
    void QueueSync();
    void Sync();
    void PickActive(size_t removedIndex);

    void UpdateBarSize();
    std::vector<wxRect> TabRects(int extent, wxSize* needed);
    int HitTest(const wxPoint& pt);

    void OnRender(wxAuiManagerEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnMotion(wxMouseEvent& event);
    void OnLeave(wxMouseEvent& event);
    void OnWheel(wxMouseEvent& event);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnMenu(wxCommandEvent& event);

    wxAuiManager* _mgr = nullptr;
    std::vector<wxString> _members;
    wxString _active;
    Slot _slot;
    bool _vertical = false;
    bool _syncQueued = false;
    bool _sizeCheckQueued = false;
    bool _inUpdate = false;
    bool _shutdown = false;
    int _hover = -1;
    wxString _contextTab;
    wxTimer _retryTimer;
    std::function<void()> _onChanged;
    std::function<void(const wxString&)> _onSelected;
};
