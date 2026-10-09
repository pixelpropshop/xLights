/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "sequencer/SequencerWindowTabs.h"

#include <wx/dcbuffer.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/tokenzr.h>
#include <wx/utils.h>

#include <algorithm>

const wxString SequencerWindowTabs::PANE_NAME = "WindowTabs";

namespace {
struct TabbableWindow {
    const char* name;
    const char* label;
    bool settings;
    // Shorter text for the tab itself; the pane's own caption keeps the full name.
    const char* tab = nullptr;
};

// The previews and the video stay out: they are watched while editing, and a
// tab hides everything but the selected window.
const TabbableWindow TABBABLE[] = {
    { "Effect", "Effect Settings", true, "Effect" },
    { "Color", "Color", true },
    { "LayerTiming", "Layer Blending", true, "Blending" },
    { "LayerSettings", "Layer Settings", true, "Layer" },
    { "EffectAssist", "Effect Assist", false },
    { "EffectPresets", "Effect Presets", false },
    { "SelectEffect", "Select Effects", false },
    { "SearchPanel", "Search Effects", false },
    { "FindData", "Find Effect Data", false },
    { "DisplayElements", "Display Elements", false },
    { "Perspectives", "Perspectives", false },
    { "EffectDropper", "Effect Dropper", false },
    { "ValueCurveDropper", "Value Curves", false },
    { "ColourDropper", "Color Dropper", false },
    { "Jukebox", "Jukebox", false },
};
constexpr size_t TABBABLE_COUNT = sizeof(TABBABLE) / sizeof(TABBABLE[0]);

const std::vector<int>& MenuIds() {
    static std::vector<int> ids;
    if (ids.empty()) {
        for (size_t i = 0; i < TABBABLE_COUNT + 4; i++) {
            ids.push_back(wxNewId());
        }
    }
    return ids;
}

int TabSettingsId() {
    return MenuIds()[TABBABLE_COUNT];
}

int RemoveAllId() {
    return MenuIds()[TABBABLE_COUNT + 1];
}

int DetachTabId() {
    return MenuIds()[TABBABLE_COUNT + 2];
}

int CloseTabId() {
    return MenuIds()[TABBABLE_COUNT + 3];
}

wxColour Mix(const wxColour& a, const wxColour& b, double amount) {
    auto m = [amount](unsigned char x, unsigned char y) {
        return (unsigned char)(x + (y - x) * amount);
    };
    return wxColour(m(a.Red(), b.Red()), m(a.Green(), b.Green()), m(a.Blue(), b.Blue()));
}
} // namespace

SequencerWindowTabs::SequencerWindowTabs(wxWindow* parent, wxAuiManager* mgr) :
    wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE),
    _mgr(mgr),
    _retryTimer(this) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    Bind(wxEVT_PAINT, &SequencerWindowTabs::OnPaint, this);
    Bind(wxEVT_SIZE, &SequencerWindowTabs::OnSize, this);
    Bind(wxEVT_LEFT_DOWN, &SequencerWindowTabs::OnLeftDown, this);
    Bind(wxEVT_MOTION, &SequencerWindowTabs::OnMotion, this);
    Bind(wxEVT_LEAVE_WINDOW, &SequencerWindowTabs::OnLeave, this);
    Bind(wxEVT_MOUSEWHEEL, &SequencerWindowTabs::OnWheel, this);
    Bind(wxEVT_CONTEXT_MENU, &SequencerWindowTabs::OnContextMenu, this);
    Bind(wxEVT_MENU, &SequencerWindowTabs::OnMenu, this);
    Bind(wxEVT_TIMER, [this](wxTimerEvent&) { Sync(); });
    _mgr->Bind(wxEVT_AUI_RENDER, &SequencerWindowTabs::OnRender, this);

    // Floatable only so the gripper can drag it between docks; Sync puts it
    // back if it is dropped away from one.
    _mgr->AddPane(this, wxAuiPaneInfo()
                            .Name(PANE_NAME)
                            .Caption("Window Tabs")
                            .CaptionVisible(false)
                            .CloseButton(false)
                            .Gripper(true)
                            .Fixed()
                            .PaneBorder(false)
                            .Left()
                            .Layer(_slot.layer)
                            .Row(_slot.row)
                            .BestSize(FromDIP(wxSize(80, 24)))
                            .Hide());
}

bool SequencerWindowTabs::CanBeTabbed(const wxString& name) {
    for (const auto& t : TABBABLE) {
        if (name == t.name) {
            return true;
        }
    }
    return false;
}

void SequencerWindowTabs::AppendMenuItems(wxMenu* menu) {
    const auto& ids = MenuIds();
    for (size_t i = 0; i < TABBABLE_COUNT; i++) {
        menu->AppendCheckItem(ids[i], TABBABLE[i].label);
    }
    menu->AppendSeparator();
    menu->Append(TabSettingsId(), "Tab the Settings Windows");
    menu->Append(RemoveAllId(), "Untab All");
}

bool SequencerWindowTabs::IsMenuCommand(int id) {
    const auto& ids = MenuIds();
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

void SequencerWindowTabs::UpdateMenuChecks(wxMenu* menu) const {
    const auto& ids = MenuIds();
    for (size_t i = 0; i < TABBABLE_COUNT; i++) {
        wxMenuItem* item = menu->FindItem(ids[i]);
        if (item != nullptr) {
            item->Enable(_mgr->GetPane(TABBABLE[i].name).IsOk());
            item->Check(Contains(TABBABLE[i].name));
        }
    }
    if (wxMenuItem* item = menu->FindItem(RemoveAllId()); item != nullptr) {
        item->Enable(!_members.empty());
    }
}

void SequencerWindowTabs::HandleMenuCommand(int id) {
    const auto& ids = MenuIds();
    for (size_t i = 0; i < TABBABLE_COUNT; i++) {
        if (ids[i] == id) {
            if (Contains(TABBABLE[i].name)) {
                RemoveWindow(TABBABLE[i].name);
            } else {
                AddWindow(TABBABLE[i].name);
            }
            return;
        }
    }
    if (id == TabSettingsId()) {
        TabSettingsWindows();
    } else if (id == RemoveAllId()) {
        RemoveAll();
    } else if (id == DetachTabId()) {
        DetachWindow(_contextTab);
    } else if (id == CloseTabId()) {
        CloseWindow(_contextTab);
    }
}

bool SequencerWindowTabs::Contains(const wxString& name) const {
    return std::find(_members.begin(), _members.end(), name) != _members.end();
}

SequencerWindowTabs::Slot SequencerWindowTabs::SlotOf(const wxAuiPaneInfo& pane) {
    Slot s;
    s.direction = pane.dock_direction;
    s.layer = pane.dock_layer;
    s.row = pane.dock_row;
    return s;
}

wxAuiPaneInfo& SequencerWindowTabs::Bar() {
    return _mgr->GetPane(this);
}

wxAuiPaneInfo& SequencerWindowTabs::Pane(const wxString& name) {
    return _mgr->GetPane(name);
}

wxString SequencerWindowTabs::LabelFor(const wxString& name) {
    for (const auto& t : TABBABLE) {
        if (name == t.name) {
            return t.tab != nullptr ? t.tab : t.label;
        }
    }
    return Pane(name).caption;
}

wxString SequencerWindowTabs::FullLabelFor(const wxString& name) {
    for (const auto& t : TABBABLE) {
        if (name == t.name) {
            return t.label;
        }
    }
    return Pane(name).caption;
}

bool SequencerWindowTabs::IsVertical() const {
    return _slot.direction == wxAUI_DOCK_TOP || _slot.direction == wxAUI_DOCK_BOTTOM;
}

void SequencerWindowTabs::AddWindow(const wxString& name) {
    if (!CanBeTabbed(name) || Contains(name)) {
        return;
    }
    wxAuiPaneInfo& pane = Pane(name);
    if (!pane.IsOk()) {
        return;
    }
    if (_members.empty()) {
        // The strip takes the first window's place, or opens a column of its
        // own when that window is floating or hidden.
        wxAuiPaneInfo& bar = Bar();
        if (pane.IsShown() && pane.IsDocked() && pane.dock_direction != wxAUI_DOCK_CENTER) {
            bar.Dock().Direction(pane.dock_direction).Layer(pane.dock_layer).Row(pane.dock_row).Position(pane.dock_pos);
        } else {
            bar.Dock().Left().Layer(2).Row(0).Position(0);
        }
        _slot = SlotOf(bar);
    }
    _members.push_back(name);
    _active = name;
    Arrange();
    Commit();
    if (_onSelected) {
        _onSelected(name);
    }
}

void SequencerWindowTabs::SelectWindow(const wxString& name) {
    if (!Contains(name) || name == _active) {
        return;
    }
    _active = name;
    Arrange();
    Commit();
    if (_onSelected) {
        _onSelected(name);
    }
}

void SequencerWindowTabs::PickActive(size_t removedIndex) {
    if (_members.empty()) {
        _active.clear();
    } else {
        _active = _members[std::min(removedIndex, _members.size() - 1)];
    }
}

void SequencerWindowTabs::RemoveWindow(const wxString& name) {
    auto it = std::find(_members.begin(), _members.end(), name);
    if (it == _members.end()) {
        return;
    }
    size_t index = it - _members.begin();
    _members.erase(it);
    wxAuiPaneInfo& pane = Pane(name);
    if (pane.IsOk()) {
        pane.Show();
    }
    if (name == _active) {
        PickActive(index);
    }
    Arrange();
    Commit();
}

void SequencerWindowTabs::CloseWindow(const wxString& name) {
    auto it = std::find(_members.begin(), _members.end(), name);
    if (it == _members.end()) {
        return;
    }
    size_t index = it - _members.begin();
    _members.erase(it);
    wxAuiPaneInfo& pane = Pane(name);
    if (pane.IsOk()) {
        pane.Hide();
    }
    if (name == _active) {
        PickActive(index);
    }
    Arrange();
    Commit();
}

void SequencerWindowTabs::DetachWindow(const wxString& name) {
    if (!Contains(name)) {
        return;
    }
    RemoveWindow(name);
    wxAuiPaneInfo& pane = Pane(name);
    if (pane.IsOk() && pane.IsFloatable()) {
        wxPoint pos = ClientToScreen(wxPoint(0, GetClientSize().y));
        pane.Float().FloatingPosition(pos).Show();
        _mgr->Update();
    }
}

void SequencerWindowTabs::TabSettingsWindows() {
    wxString first;
    for (const auto& t : TABBABLE) {
        if (t.settings && Pane(t.name).IsOk()) {
            if (first.empty()) {
                first = t.name;
            }
            AddWindow(t.name);
        }
    }
    if (!first.empty()) {
        SelectWindow(first);
    }
}

void SequencerWindowTabs::RemoveAll() {
    if (_members.empty()) {
        return;
    }
    for (const auto& name : _members) {
        wxAuiPaneInfo& pane = Pane(name);
        if (pane.IsOk()) {
            pane.Show();
        }
    }
    _members.clear();
    _active.clear();
    Arrange();
    Commit();
}

std::string SequencerWindowTabs::GetState() const {
    if (_members.empty()) {
        return std::string();
    }
    wxString value = _active + "|";
    for (size_t i = 0; i < _members.size(); i++) {
        if (i > 0) {
            value += ",";
        }
        value += _members[i];
    }
    return value.ToStdString();
}

void SequencerWindowTabs::SetState(const std::string& state) {
    wxString value = wxString::FromUTF8(state);
    _members.clear();
    _active.clear();
    wxString active = value.BeforeFirst('|');
    wxStringTokenizer tokens(value.AfterFirst('|'), ",");
    while (tokens.HasMoreTokens()) {
        wxString name = tokens.GetNextToken();
        if (CanBeTabbed(name) && !Contains(name) && Pane(name).IsOk()) {
            _members.push_back(name);
        }
    }
    if (Contains(active)) {
        _active = active;
    } else if (!_members.empty()) {
        _active = _members.front();
    }
}

void SequencerWindowTabs::Reapply() {
    if (_shutdown || !Bar().IsOk()) {
        return;
    }
    if (!Bar().IsFloating()) {
        _slot = SlotOf(Bar());
    }
    if (Arrange()) {
        Commit();
    } else {
        Refresh();
    }
}

void SequencerWindowTabs::Shutdown() {
    if (_shutdown) {
        return;
    }
    _shutdown = true;
    _retryTimer.Stop();
    _mgr->Unbind(wxEVT_AUI_RENDER, &SequencerWindowTabs::OnRender, this);
}

// Puts the panes in the state the tabs describe. Returns whether anything changed.
bool SequencerWindowTabs::Arrange() {
    bool changed = false;
    wxAuiPaneInfo& bar = Bar();
    if (_members.empty()) {
        if (bar.IsShown()) {
            bar.Hide();
            changed = true;
        }
        return changed;
    }
    if (_active.empty() || !Contains(_active)) {
        _active = _members.front();
    }
    if (bar.IsFloating()) {
        bar.Dock().Direction(_slot.direction).Layer(_slot.layer).Row(_slot.row);
        changed = true;
    }
    if (!bar.IsShown()) {
        bar.Show();
        changed = true;
    }
    Slot slot = SlotOf(bar);
    _slot = slot;
    if (_vertical != IsVertical()) {
        _vertical = IsVertical();
        bar.GripperTop(_vertical);
        changed = true;
    }

    for (const auto& name : _members) {
        wxAuiPaneInfo& pane = Pane(name);
        if (!pane.IsOk()) {
            continue;
        }
        if (pane.IsFloating() || SlotOf(pane) != slot) {
            pane.Dock().Direction(slot.direction).Layer(slot.layer).Row(slot.row);
            changed = true;
        }
        bool show = name == _active;
        if (pane.IsShown() != show) {
            pane.Show(show);
            changed = true;
        }
    }

    // Number the shown panes in the slot so the selected window sits right
    // after the strip and anything else docked there keeps its order.
    wxAuiPaneInfo& active = Pane(_active);
    std::vector<wxAuiPaneInfo*> order;
    wxAuiPaneInfoArray& panes = _mgr->GetAllPanes();
    for (size_t i = 0; i < panes.size(); i++) {
        wxAuiPaneInfo& p = panes[i];
        if (&p == &bar || &p == &active || !p.IsShown() || !p.IsDocked() || SlotOf(p) != slot) {
            continue;
        }
        order.push_back(&p);
    }
    std::stable_sort(order.begin(), order.end(), [](const wxAuiPaneInfo* a, const wxAuiPaneInfo* b) {
        return a->dock_pos < b->dock_pos;
    });
    auto barAt = std::find_if(order.begin(), order.end(), [&bar](const wxAuiPaneInfo* p) {
        return p->dock_pos >= bar.dock_pos;
    });
    size_t barIndex = barAt - order.begin();
    order.insert(barAt, &bar);
    if (active.IsOk()) {
        order.insert(order.begin() + barIndex + 1, &active);
    }
    for (size_t i = 0; i < order.size(); i++) {
        if (order[i]->dock_pos != (int)i) {
            order[i]->dock_pos = (int)i;
            changed = true;
        }
    }
    return changed;
}

void SequencerWindowTabs::Commit() {
    UpdateBarSize();
    _inUpdate = true;
    _mgr->Update();
    _inUpdate = false;
    Refresh();
    if (_onChanged) {
        _onChanged();
    }
}

void SequencerWindowTabs::OnRender(wxAuiManagerEvent& event) {
    event.Skip();
    if (!_inUpdate && !_members.empty()) {
        QueueSync();
    }
}

void SequencerWindowTabs::QueueSync() {
    if (_syncQueued || _shutdown) {
        return;
    }
    _syncQueued = true;
    CallAfter([this]() {
        _syncQueued = false;
        Sync();
    });
}

// Catches changes made by dragging, which the manager does not report: the
// strip moved to another dock, or the selected window dragged out of it.
void SequencerWindowTabs::Sync() {
    if (_shutdown || _members.empty()) {
        return;
    }
    // A dragged pane floats until it is dropped; judge the layout after the drop.
    if (wxGetMouseState().LeftIsDown()) {
        _retryTimer.StartOnce(150);
        return;
    }
    wxAuiPaneInfoArray& panes = _mgr->GetAllPanes();
    for (size_t i = 0; i < panes.size(); i++) {
        if (panes[i].IsMaximized()) {
            return;
        }
    }
    wxAuiPaneInfo& bar = Bar();
    if (!bar.IsOk()) {
        return;
    }

    bool changed = false;
    bool barMoved = !bar.IsFloating() && SlotOf(bar) != _slot;
    if (!barMoved && !bar.IsFloating() && !_active.empty()) {
        wxAuiPaneInfo& active = Pane(_active);
        if (active.IsOk() && active.IsShown() && (active.IsFloating() || SlotOf(active) != _slot)) {
            // Dragged away from the strip: it leaves the tabs and stays where it was dropped.
            auto it = std::find(_members.begin(), _members.end(), _active);
            size_t index = it - _members.begin();
            _members.erase(it);
            PickActive(index);
            changed = true;
        }
    }
    // A tabbed window shown by something other than its tab becomes the selected one.
    for (const auto& name : _members) {
        if (name != _active && Pane(name).IsShown()) {
            _active = name;
            changed = true;
            break;
        }
    }
    changed |= Arrange();
    if (changed) {
        Commit();
    } else {
        Refresh();
    }
}

std::vector<wxRect> SequencerWindowTabs::TabRects(int extent, wxSize* needed) {
    wxClientDC dc(this);
    wxFont bold = GetFont().Bold();
    dc.SetFont(bold);
    const int padX = FromDIP(12);
    const int padY = FromDIP(6);
    const int margin = FromDIP(2);
    const int height = dc.GetCharHeight() + 2 * padY;

    // Widths are measured in bold so a tab does not grow when it is selected.
    std::vector<wxRect> rects;
    if (!_vertical) {
        int x = margin;
        int y = 0;
        for (const auto& name : _members) {
            int w = dc.GetTextExtent(LabelFor(name)).x + 2 * padX;
            if (x > margin && x + w > extent - margin) {
                x = margin;
                y += height;
            }
            rects.emplace_back(x, y, w, height);
            x += w;
        }
        if (needed != nullptr) {
            *needed = wxSize(FromDIP(80), y + height + margin);
        }
    } else {
        int w = 0;
        for (const auto& name : _members) {
            w = std::max(w, dc.GetTextExtent(LabelFor(name)).x + 2 * padX);
        }
        int x = 0;
        int y = margin;
        for (size_t i = 0; i < _members.size(); i++) {
            if (y > margin && y + height > extent - margin) {
                y = margin;
                x += w;
            }
            rects.emplace_back(x, y, w, height);
            y += height;
        }
        if (needed != nullptr) {
            *needed = wxSize(x + w + margin, height + 2 * margin);
        }
    }
    return rects;
}

void SequencerWindowTabs::UpdateBarSize() {
    wxAuiPaneInfo& bar = Bar();
    if (!bar.IsOk()) {
        return;
    }
    wxSize client = GetClientSize();
    int extent = _vertical ? client.y : client.x;
    if (extent <= FromDIP(20)) {
        extent = FromDIP(_vertical ? 400 : 250);
    }
    wxSize needed;
    TabRects(extent, &needed);
    bar.BestSize(needed);
}

void SequencerWindowTabs::OnSize(wxSizeEvent& event) {
    event.Skip();
    Refresh();
    if (_members.empty() || _shutdown || _sizeCheckQueued) {
        return;
    }
    // Wrapping depends on the strip's length, so a resize can change its depth.
    // Checked afterwards, as the resize usually comes from inside an Update.
    _sizeCheckQueued = true;
    CallAfter([this]() {
        _sizeCheckQueued = false;
        if (_shutdown || _members.empty()) {
            return;
        }
        wxSize before = Bar().best_size;
        UpdateBarSize();
        if (Bar().best_size != before) {
            _inUpdate = true;
            _mgr->Update();
            _inUpdate = false;
        }
    });
}

int SequencerWindowTabs::HitTest(const wxPoint& pt) {
    wxSize client = GetClientSize();
    auto rects = TabRects(_vertical ? client.y : client.x, nullptr);
    for (size_t i = 0; i < rects.size(); i++) {
        if (rects[i].Contains(pt)) {
            return (int)i;
        }
    }
    return -1;
}

void SequencerWindowTabs::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    const wxColour bg = GetBackgroundColour();
    const wxColour text = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    const wxColour accent = wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT);
    dc.SetBackground(wxBrush(bg));
    dc.Clear();

    const wxSize client = GetClientSize();
    const int line = FromDIP(1);
    const int thickness = FromDIP(2);
    const int padX = FromDIP(12);

    // A hairline under the whole strip (or beside it, when vertical) separates
    // it from the window below; the selected tab's accent bar sits on it.
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(Mix(bg, text, 0.15)));
    if (_vertical) {
        dc.DrawRectangle(client.x - line, 0, line, client.y);
    } else {
        dc.DrawRectangle(0, client.y - line, client.x, line);
    }

    const wxFont normal = GetFont();
    const wxFont bold = normal.Bold();
    auto rects = TabRects(_vertical ? client.y : client.x, nullptr);
    for (size_t i = 0; i < rects.size(); i++) {
        const wxRect& r = rects[i];
        const bool active = _members[i] == _active;
        const bool hover = (int)i == _hover;
        if (hover && !active) {
            dc.SetBrush(wxBrush(Mix(bg, text, 0.08)));
            dc.DrawRectangle(r);
        }
        if (active) {
            dc.SetBrush(wxBrush(accent));
            if (_vertical) {
                dc.DrawRectangle(r.GetRight() + 1 - thickness, r.y, thickness, r.height);
            } else {
                dc.DrawRectangle(r.x, r.GetBottom() + 1 - thickness, r.width, thickness);
            }
        }
        dc.SetFont(active ? bold : normal);
        dc.SetTextForeground(active || hover ? text : Mix(text, bg, 0.35));
        const wxString label = LabelFor(_members[i]);
        const wxSize extent = dc.GetTextExtent(label);
        dc.DrawText(label, _vertical ? r.x + padX : r.x + (r.width - extent.x) / 2, r.y + (r.height - extent.y) / 2);
    }
}

void SequencerWindowTabs::OnLeftDown(wxMouseEvent& event) {
    int index = HitTest(event.GetPosition());
    if (index >= 0) {
        SelectWindow(_members[index]);
    }
}

void SequencerWindowTabs::OnMotion(wxMouseEvent& event) {
    int index = HitTest(event.GetPosition());
    if (index != _hover) {
        _hover = index;
        Refresh();
    }
}

void SequencerWindowTabs::OnLeave(wxMouseEvent& event) {
    if (_hover != -1) {
        _hover = -1;
        Refresh();
    }
}

void SequencerWindowTabs::OnWheel(wxMouseEvent& event) {
    if (_members.size() < 2 || event.GetWheelRotation() == 0) {
        return;
    }
    auto it = std::find(_members.begin(), _members.end(), _active);
    int index = it == _members.end() ? 0 : (int)(it - _members.begin());
    int count = (int)_members.size();
    index = (index + (event.GetWheelRotation() < 0 ? 1 : count - 1)) % count;
    SelectWindow(_members[index]);
}

void SequencerWindowTabs::OnContextMenu(wxContextMenuEvent& event) {
    wxPoint pt = event.GetPosition() == wxDefaultPosition ? wxPoint(0, 0) : ScreenToClient(event.GetPosition());
    int index = HitTest(pt);
    _contextTab = index >= 0 ? _members[index] : wxString();

    wxMenu menu;
    if (!_contextTab.empty()) {
        const wxString label = FullLabelFor(_contextTab);
        menu.Append(DetachTabId(), "Detach " + label);
        menu.Append(CloseTabId(), "Close " + label);
        menu.AppendSeparator();
    }
    AppendMenuItems(&menu);
    UpdateMenuChecks(&menu);
    PopupMenu(&menu);
}

void SequencerWindowTabs::OnMenu(wxCommandEvent& event) {
    if (IsMenuCommand(event.GetId())) {
        HandleMenuCommand(event.GetId());
    } else {
        event.Skip();
    }
}
