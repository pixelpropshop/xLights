/***************************************************************
* This source files comes from the xLights project
* https://www.xlights.org
* https://github.com/xLightsSequencer/xLights
* See the github commit history for a record of contributing
* developers.
* Copyright claimed based on commit dates recorded in Github
* License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
**************************************************************/

#include <functional>
#include <vector>

#include <wx/preferences.h>
#include <wx/artprov.h>
#include <wx/bmpbndl.h>
#include <wx/simplebook.h>
#include <wx/statline.h>
#include <wx/vlbox.h>
#include <wx/scrolwin.h>

#include "xLightsMain.h"
#include "shared/utils/LineIcons.h"
#include "shared/utils/wxUtilities.h"

#include "ViewSettingsPanel.h"
#include "EffectsGridSettingsPanel.h"
#include "SequenceFileSettingsPanel.h"
#include "BackupSettingsPanel.h"
#include "OutputSettingsPanel.h"
#include "RandomEffectsSettingsPanel.h"
#include "ColorManagerSettingsPanel.h"
#include "OtherSettingsPanel.h"
#include "CheckSequenceSettingsPanel.h"
#include "ToolbarsSettingsPanel.h"
#include "ServicesPanel.h"

#include "grid_icon.xpm"
#include "settings_panel_icon.xpm"

namespace {
// Shared description of a preferences page so the macOS native editor and the
// desktop list dialog stay in lockstep when pages are added or reordered.
struct PrefPageDef {
    wxString name;
    wxBitmapBundle nativeIcon; // larger icon for the native macOS toolbar
    wxBitmapBundle listIcon;   // uniform small icon for the left-hand list
    std::function<wxWindow*(wxWindow*)> factory;
};
}

class xLightsPreferencesPage : public wxPreferencesPage {
public:
    xLightsPreferencesPage(const wxString &n, const wxBitmapBundle &i, std::function<wxWindow*(wxWindow*)> & f) : wxPreferencesPage(), m_icon(i), m_name(n), m_createFunction(f) {
    }

    virtual wxString GetName() const override {
        return m_name;
    }

    virtual wxBitmapBundle GetIcon() const override {
        return m_icon;
    }
    virtual wxWindow *CreateWindow (wxWindow *parent) override {
#ifdef __WXMSW__
        auto *scrolledWindow = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL);
        scrolledWindow->SetScrollRate(10, 10);

        wxWindow *content = m_createFunction(scrolledWindow);
        auto *sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(content, 1, wxEXPAND | wxALL, 2);

        scrolledWindow->SetSizer(sizer);
        scrolledWindow->FitInside();

        const wxSize screenSize = wxGetDisplaySize();
        int screenWidth = screenSize.GetWidth() * 0.90;
        int screenHeight = screenSize.GetHeight() * 0.45;

        int minWidth = std::min(screenWidth, 850);

        int minHeight = std::min(screenHeight, 375);
        int maxHeight = std::max(screenHeight, 250);

        scrolledWindow->SetMinSize(wxSize(minWidth, minHeight));
        scrolledWindow->SetMaxSize(wxSize(screenWidth, maxHeight));
        scrolledWindow->Layout();

        return scrolledWindow;
#else
        wxWindow *w = m_createFunction(parent);
#ifdef __WXOSX__
        //need to set a minimum width or the icons get moved into a flyout menu
        //which is more confusing
        w->SetMinSize(wxSize(500, -1));
#endif
        return w;
#endif
    }

private:
    wxBitmapBundle m_icon;
    wxString m_name;
    std::function<wxWindow*(wxWindow*)> m_createFunction;
};

#ifndef __WXOSX__
namespace {
wxColour MixColour(const wxColour& a, const wxColour& b, double amount) {
    auto m = [amount](unsigned char x, unsigned char y) {
        return (unsigned char)(x + (y - x) * amount);
    };
    return wxColour(m(a.Red(), b.Red()), m(a.Green(), b.Green()), m(a.Blue(), b.Blue()));
}

// The page list: one row per page, icon and name, the selected row tinted
// with the accent. Owner-drawn so it looks the same on Windows and Linux.
class PrefPageList : public wxVListBox {
public:
    PrefPageList(wxWindow* parent, const std::vector<PrefPageDef>& pages) :
        wxVListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE), _pages(pages) {
        SetItemCount(_pages.size());
        SetBackgroundColour(MixColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE), wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT), 0.04));
        int width = 0;
        wxClientDC dc(this);
        dc.SetFont(GetFont().Bold());
        for (const auto& p : _pages) {
            width = std::max(width, dc.GetTextExtent(p.name).x);
        }
        // Tall enough to list every page without scrolling.
        SetMinSize(wxSize(width + FromDIP(64), (int)_pages.size() * FromDIP(36) + FromDIP(12)));
    }

protected:
    wxCoord OnMeasureItem(size_t) const override {
        return FromDIP(36);
    }

    void OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const override {
        if (!IsSelected(n)) {
            return;
        }
        const bool dark = IsDarkMode();
        wxColour accent = dark ? wxColour(242, 169, 59) : wxColour(201, 133, 18);
        wxRect r = rect.Deflate(FromDIP(6), FromDIP(2));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(MixColour(GetBackgroundColour(), accent, dark ? 0.22 : 0.18)));
        dc.DrawRoundedRectangle(r, FromDIP(6));
    }

    void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const override {
        const PrefPageDef& page = _pages[n];
        const bool selected = IsSelected(n);
        int x = rect.x + FromDIP(16);
        const wxBitmap icon = page.listIcon.GetBitmapFor(this);
        if (icon.IsOk()) {
            const wxSize sz = icon.GetLogicalSize();
            dc.DrawBitmap(icon, x, rect.y + (rect.height - sz.y) / 2, true);
            x += sz.x + FromDIP(10);
        }
        dc.SetFont(selected ? GetFont().Bold() : GetFont());
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
        const wxSize text = dc.GetTextExtent(page.name);
        dc.DrawText(page.name, x, rect.y + (rect.height - text.y) / 2);
    }

private:
    const std::vector<PrefPageDef>& _pages;
};
} // namespace

// Preferences on Windows/Linux: a list of pages on the left selects the page
// shown on the right, each under its own title. macOS keeps the native
// preferences window.
class xlPreferencesListDialog : public wxDialog {
public:
    xlPreferencesListDialog(wxWindow* parent, const std::vector<PrefPageDef>& pages, const wxString& initialPage = wxEmptyString)
        : wxDialog(parent, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), _pages(pages) {
        // Ensure every page's TransferDataTo/FromWindow runs on open and OK.
        SetExtraStyle(GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);

        auto* topSizer = new wxBoxSizer(wxVERTICAL);
        auto* body = new wxBoxSizer(wxHORIZONTAL);
        auto* list = new PrefPageList(this, _pages);
        auto* book = new wxSimplebook(this, wxID_ANY);

        const wxSize screenSize = wxGetDisplaySize();
        int minWidth = std::min((int)(screenSize.GetWidth() * 0.90), 850);
        int minHeight = std::min((int)(screenSize.GetHeight() * 0.45), 375);

        int idx = 0;
        int selectIdx = 0;
        for (const auto& p : _pages) {
            auto* page = new wxPanel(book, wxID_ANY);
            auto* pageSizer = new wxBoxSizer(wxVERTICAL);
            auto* title = new wxStaticText(page, wxID_ANY, p.name);
            wxFont titleFont = title->GetFont().Bold();
            titleFont.SetFractionalPointSize(titleFont.GetFractionalPointSize() * 1.35);
            title->SetFont(titleFont);
            pageSizer->Add(title, 0, wxLEFT | wxRIGHT | wxTOP, FromDIP(14));
            pageSizer->Add(new wxStaticLine(page, wxID_ANY), 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP | wxBOTTOM, FromDIP(10));

            // Wrap each panel in a scrolled window so tall panels stay usable.
            auto* scrolledWindow = new wxScrolledWindow(page, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL);
            scrolledWindow->SetScrollRate(10, 10);
            wxWindow* content = p.factory(scrolledWindow);
            auto* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(content, 1, wxEXPAND | wxALL, 2);
            scrolledWindow->SetSizer(sizer);
            scrolledWindow->FitInside();
            scrolledWindow->SetMinSize(wxSize(minWidth, minHeight));
            pageSizer->Add(scrolledWindow, 1, wxEXPAND | wxLEFT | wxRIGHT, FromDIP(8));
            page->SetSizer(pageSizer);

            book->AddPage(page, p.name, false);
            if (!initialPage.IsEmpty() && p.name == initialPage) {
                selectIdx = idx;
            }
            ++idx;
        }
        book->SetSelection(selectIdx);
        list->SetSelection(selectIdx);
        list->Bind(wxEVT_LISTBOX, [book, list](wxCommandEvent&) {
            if (list->GetSelection() != wxNOT_FOUND) {
                book->SetSelection(list->GetSelection());
            }
        });

        body->Add(list, 0, wxEXPAND);
        body->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND);
        body->Add(book, 1, wxEXPAND);
        topSizer->Add(body, 1, wxEXPAND);
        topSizer->Add(new wxStaticLine(this, wxID_ANY), 0, wxEXPAND);
        topSizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, FromDIP(8));

        SetSizer(topSizer);
        topSizer->SetSizeHints(this);
        Fit();
        CentreOnParent();
        FitWindowToDisplay(this);
    }

private:
    std::vector<PrefPageDef> _pages;
};
#endif

void xLightsFrame::OnMenuItemPreferencesSelected(wxCommandEvent& event)
{
    ShowPreferencesDialog();
}

// Right-click on the Effects toolbar jumps straight to Preferences >
// Toolbars instead of making the user hunt for it in the general
// Preferences list.
void xLightsFrame::OnEffectsToolBarContextMenu(wxContextMenuEvent& event)
{
    ShowPreferencesDialog("Toolbars");
}

void xLightsFrame::ShowPreferencesDialog(const wxString& initialPage)
{
    if (readOnlyMode) {
        wxMessageBox("Preferences are not available in read only mode", "Read Only Mode", wxICON_INFORMATION | wxOK);
        return;
    }

    auto ld = _lowDefinitionRender;

    wxImage gridImage(GRID_ICON_64);
    wxBitmap gridIcon(gridImage);
    wxImage settingsImage(SETTINGS_PANEL_ICON);
    wxBitmap settingIcon(settingsImage);

    const wxSize iconSize(64, 64);
    const wxSize listIconSize(24, 24);

    auto scaledBundle = [](const wxImage& img, const wxSize& sz) {
        return wxBitmapBundle(wxBitmap(img.Scale(sz.GetWidth(), sz.GetHeight(), wxIMAGE_QUALITY_HIGH)));
    };
    // Each page's own line icon in the page list, or the classic icon.
    auto listIcon = [&listIconSize](const char* lineId, const wxBitmapBundle& classic) {
        wxBitmapBundle line = CreateLineIconBundle(lineId, listIconSize.GetWidth());
        return line.IsOk() ? line : classic;
    };

    std::vector<PrefPageDef> pages;
    pages.push_back({ "Backup",
                      wxArtProvider::GetBitmapBundle(wxART_HARDDISK, wxART_BUTTON, wxSize(28, 28)),
                      listIcon("xlART_PREF_BACKUP", wxArtProvider::GetBitmapBundle(wxART_HARDDISK, wxART_BUTTON, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new BackupSettingsPanel(p, this)); } });
    pages.push_back({ "View",
                      wxArtProvider::GetBitmapBundle(wxART_FULL_SCREEN, wxART_BUTTON, iconSize),
                      listIcon("xlART_PREF_VIEW", wxArtProvider::GetBitmapBundle(wxART_FULL_SCREEN, wxART_BUTTON, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new ViewSettingsPanel(p, this)); } });
    pages.push_back({ "Effects Grid",
                      wxBitmapBundle(gridIcon),
                      listIcon("xlART_PREF_GRID", scaledBundle(gridImage, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new EffectsGridSettingsPanel(p, this)); } });
    pages.push_back({ "Sequences",
                      wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, iconSize),
                      listIcon("xlART_PREF_SEQUENCES", wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new SequenceFileSettingsPanel(p, this)); } });
    pages.push_back({ "Output",
                      wxArtProvider::GetBitmapBundle("xlART_OUTPUT_LIGHTS_ON", wxART_BUTTON, iconSize),
                      listIcon("xlART_PREF_OUTPUT", wxArtProvider::GetBitmapBundle("xlART_OUTPUT_LIGHTS_ON", wxART_BUTTON, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new OutputSettingsPanel(p, this)); } });
    pages.push_back({ "Check Sequence",
                      wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, iconSize),
                      listIcon("xlART_PREF_CHECK", wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new CheckSequenceSettingsPanel(p, this)); } });
    pages.push_back({ "Random Effects",
                      wxArtProvider::GetBitmapBundle("xlART_DICE_ICON", wxART_BUTTON, wxSize(28, 28)),
                      listIcon("xlART_PREF_RANDOM", wxArtProvider::GetBitmapBundle("xlART_DICE_ICON", wxART_BUTTON, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new RandomEffectsSettingsPanel(p, this)); } });
    pages.push_back({ "Colors",
                      wxArtProvider::GetBitmapBundle("xlART_RENDER_ALL", wxART_BUTTON, iconSize),
                      listIcon("xlART_PREF_COLORS", wxArtProvider::GetBitmapBundle("xlART_RENDER_ALL", wxART_BUTTON, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new ColorManagerSettingsPanel(p, this)); } });
    pages.push_back({ "Other",
                      wxBitmapBundle(settingIcon),
                      listIcon("xlART_PREF_OTHER", scaledBundle(settingsImage, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new OtherSettingsPanel(p, this)); } });
    pages.push_back({ "Toolbars",
                      wxArtProvider::GetBitmapBundle(wxART_LIST_VIEW, wxART_BUTTON, iconSize),
                      listIcon("xlART_PREF_TOOLBARS", wxArtProvider::GetBitmapBundle(wxART_LIST_VIEW, wxART_BUTTON, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new ToolbarsSettingsPanel(p, this)); } });
#ifdef ENABLE_SERVICES
    pages.push_back({ "Services",
                      wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, iconSize),
                      listIcon("xlART_PREF_SERVICES", wxArtProvider::GetBitmapBundle("xlART_SETTINGS", wxART_BUTTON, listIconSize)),
                      [this](wxWindow* p) { return (wxWindow*)(new ServicesPanel(p, _serviceManager.get())); } });
#endif

#ifdef __WXOSX__
    // initialPage is intentionally unused here: wxPreferencesEditor (the
    // native macOS toolbar-style preferences window) exposes no page-select
    // API, only AddPage()/Show(). Right-clicking the Effects toolbar still
    // opens Preferences on macOS, just not pre-selected to Toolbars - the
    // user has one extra click there versus the Windows/Linux list dialog.
    if (!mPreferencesEditor.get()) {
        mPreferencesEditor.reset(new wxPreferencesEditor("Preferences"));
        for (auto& p : pages) {
            std::function<wxWindow*(wxWindow*)> f = p.factory;
            mPreferencesEditor->AddPage(new xLightsPreferencesPage(p.name, p.nativeIcon, f));
        }
    }
    mPreferencesEditor->Show(this);
#else
    xlPreferencesListDialog dlg(this, pages, initialPage);
    dlg.ShowModal();
#endif

    if (mRenderOnSave) {
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVE, _("Render All and Save"));
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVEAS, _("Render All and Save As"));
        MainToolBar->Realize();
    } else {
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVE, _("Save"));
        MainToolBar->SetToolShortHelp(ID_AUITOOLBAR_SAVEAS, _("Save As"));
        MainToolBar->Realize();
    }

    ResizeMainSequencer(); // just in case row height has changed

    if (ld != _lowDefinitionRender) {
            // just in case the user changes the low resolution renderer
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_RELOAD_ALLMODELS, "Preferences Change");
        _outputModelManager.AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Preferences Change");
    }
}
