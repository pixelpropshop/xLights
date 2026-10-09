/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ModernDockArt.h"

#include <wx/dc.h>
#include <wx/settings.h>
#include <wx/window.h>

#include <algorithm>

namespace {
wxColour Mix(const wxColour& a, const wxColour& b, double amount) {
    auto m = [amount](unsigned char x, unsigned char y) {
        return (unsigned char)(x + (y - x) * amount);
    };
    return wxColour(m(a.Red(), b.Red()), m(a.Green(), b.Green()), m(a.Blue(), b.Blue()));
}

void DrawGripLine(wxDC& dc, wxWindow* window, const wxRect& rect, const wxColour& face, bool horizontal) {
    const wxColour text = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(Mix(face, text, 0.35)));
    const int thickness = wxWindow::FromDIP(2, window);
    const wxPoint center(rect.x + rect.width / 2, rect.y + rect.height / 2);
    if (horizontal) {
        const int half = std::min(wxWindow::FromDIP(14, window), rect.width / 3);
        dc.DrawRoundedRectangle(center.x - half, center.y - thickness / 2, half * 2, thickness, thickness / 2.0);
    } else {
        const int half = std::min(wxWindow::FromDIP(14, window), rect.height / 3);
        dc.DrawRoundedRectangle(center.x - thickness / 2, center.y - half, thickness, half * 2, thickness / 2.0);
    }
}
} // namespace

ModernDockArt::ModernDockArt() {
    SetMetric(wxAUI_DOCKART_GRADIENT_TYPE, wxAUI_GRADIENT_NONE);
    SetMetric(wxAUI_DOCKART_CAPTION_SIZE, 22);
    SetFont(wxAUI_DOCKART_CAPTION_FONT, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
    // The base constructor's call cannot reach this override.
    UpdateColoursFromSystem();
}

wxAuiDockArt* ModernDockArt::Clone() {
    return new ModernDockArt(*this);
}

void ModernDockArt::UpdateColoursFromSystem() {
    wxAuiDefaultDockArt::UpdateColoursFromSystem();

    const wxColour face = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
    const wxColour text = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    const wxColour caption = Mix(face, text, 0.07);
    SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_COLOUR, caption);
    SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_GRADIENT_COLOUR, caption);
    SetColour(wxAUI_DOCKART_INACTIVE_CAPTION_TEXT_COLOUR, Mix(text, face, 0.25));
    SetColour(wxAUI_DOCKART_BORDER_COLOUR, Mix(face, text, 0.12));
    SetColour(wxAUI_DOCKART_SASH_COLOUR, face);
    SetColour(wxAUI_DOCKART_BACKGROUND_COLOUR, face);
    SetColour(wxAUI_DOCKART_GRIPPER_COLOUR, face);
}

void ModernDockArt::DrawGripper(wxDC& dc, wxWindow* window, const wxRect& rect, wxAuiPaneInfo& pane) {
    const wxColour face = GetColour(wxAUI_DOCKART_GRIPPER_COLOUR);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(face));
    dc.DrawRectangle(rect);
    DrawGripLine(dc, window, rect, face, pane.HasGripperTop());
}

wxAuiToolBarArt* ModernToolBarArt::Clone() {
    return new ModernToolBarArt(*this);
}

void ModernToolBarArt::DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) {
    DrawPlainBackground(dc, wnd, rect);
}

void ModernToolBarArt::DrawGripper(wxDC& dc, wxWindow* wnd, const wxRect& rect) {
    DrawGripLine(dc, wnd, rect, wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE), (m_flags & wxAUI_TB_VERTICAL) != 0);
}
