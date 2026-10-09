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

#include <wx/bmpbndl.h>
#include <wx/string.h>

// Single-color line icons for the main window toolbars, drawn from SVG and
// tinted for the current light or dark appearance. Returns an invalid bundle
// for an art id without a line icon, or when the "Classic" icon style is set.
wxBitmapBundle CreateLineIconBundle(const wxString& artId, int size);

// "Line" (default) or "Classic". UseLineIcons is read once, so a change saved
// by SetUseLineIcons applies on restart; SavedUseLineIcons reads the setting.
bool UseLineIcons();
bool SavedUseLineIcons();
void SetUseLineIcons(bool line);
