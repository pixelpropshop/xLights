/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "LineIcons.h"

#include "settings/XLightsConfigAdapter.h"
#include "wxUtilities.h"

#include <wx/artprov.h>

namespace {
enum class Tint { Normal, Accent };

struct LineIcon {
    const char* id;
    const char* path;
    bool filled;
    Tint tint;
    // Size used when the caller asks for none (wxART_BUTTON), matching the
    // bitmap the icon replaces.
    int defaultSize = 0;
};

// 24x24 grid, 1.8 stroke, round caps and joins.
const LineIcon ICONS[] = {
    // Main
    { wxART_FOLDER_OPEN, "M3 7a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z", false, Tint::Normal },
    { wxART_NEW, "M14 3H7a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V8zM14 3v5h5M12 11v6M9 14h6", false, Tint::Normal },
    { wxART_FILE_OPEN, "M4 20h13l3-9H7zM4 20V6a1 1 0 0 1 1-1h4l2 2h6a1 1 0 0 1 1 1v3", false, Tint::Normal },
    { wxART_FILE_SAVE, "M5 3h11l3 3v13a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2zM8 3v5h7V3M7 21v-7h10v7", false, Tint::Normal },
    { wxART_FILE_SAVE_AS, "M14 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h9l3 3v3M7 3v4h6V3M13 21l1-3 5.5-5.5 2 2L16 20z", false, Tint::Normal },
    { "xlART_RENDER_ALL", "M13 2L3 14h9l-1 8 10-12h-9l1-8z", false, Tint::Normal },
    // Playback
    { "xlART_PLAY", "M8 5.5v13a1 1 0 0 0 1.5.86l10.5-6.5a1 1 0 0 0 0-1.72L9.5 4.64A1 1 0 0 0 8 5.5z", true, Tint::Accent },
    { "xlART_PAUSE", "M7 5h3v14H7zM14 5h3v14h-3z", true, Tint::Normal },
    { "xlART_STOP", "M6 6h12v12H6z", true, Tint::Normal },
    { "xlART_BACKWARD", "M19 20L9 12l10-8zM5 19V5", false, Tint::Normal },
    { "xlART_FORWARD", "M5 4l10 8-10 8zM19 5v14", false, Tint::Normal },
    { "xlART_REPLAY", "M17 2l4 4-4 4M3 11V9a4 4 0 0 1 4-4h14M7 22l-4-4 4-4M21 13v2a4 4 0 0 1-4 4H3", false, Tint::Normal },
    { "xlART_OUTPUT_LIGHTS", "M9 18h6M10 22h4M12 2a7 7 0 0 0-4 12.7V17h8v-2.3A7 7 0 0 0 12 2z", false, Tint::Normal },
    { "xlART_OUTPUT_LIGHTS_ON", "M9 18h6M10 22h4M12 6a5 5 0 0 0-3 9V17h6v-2a5 5 0 0 0-3-9zM12 1v2M4.2 4.2l1.4 1.4M18.4 5.6l1.4-1.4M2 12h2M20 12h2", false, Tint::Accent },
    // Windows
    { "xlART_EFFECTSETTINGS", "M4 6h9M17 6h3M4 12h3M11 12h9M4 18h11M19 18h1M15 6a2 2 0 1 0 0 .01M9 12a2 2 0 1 0 0 .01M17 18a2 2 0 1 0 0 .01", false, Tint::Normal },
    { "xlART_COLORS", "M12 3a9 9 0 1 0 0 18c1 0 1.6-.8 1.6-1.6 0-.5-.2-.8-.5-1.2-.3-.3-.4-.7-.4-1.1 0-.9.7-1.6 1.6-1.6H16a5 5 0 0 0 5-5C21 6.2 17 3 12 3zM7.5 11.5h.01M10 7.5h.01M15 8h.01", false, Tint::Normal },
    { "xlART_LAYERS", "M12 3l9 5-9 5-9-5zM3 13l9 5 9-5", false, Tint::Normal },
    { "xlART_LAYERS2", "M9 12m-6 0a6 6 0 1 0 12 0a6 6 0 1 0-12 0M15 12m-6 0a6 6 0 1 0 12 0a6 6 0 1 0-12 0", false, Tint::Normal },
    { "xlART_MODEL_PREVIEW", "M3 4h18v13H3zM8 21h8M12 7l-3.5 7h7z", false, Tint::Normal },
    { "xlART_HOUSE_PREVIEW", "M3 11l9-7 9 7M5 10v10h14V10M10 20v-5h4v5", false, Tint::Normal },
    { "xlART_SEQUENCE_ELEMENTS", "M8 6h13M8 12h13M8 18h13M3 6h.01M3 12h.01M3 18h.01", false, Tint::Normal },
    { "xlART_EFFECTS", "M12 3l1.8 4.6L18 9l-4.2 1.4L12 15l-1.8-4.6L6 9l4.2-1.4zM18.5 14.5l.8 2 2 .8-2 .8-.8 2-.8-2-2-.8 2-.8z", false, Tint::Normal },
    { "xlART_EFFECTASSISTANT", "M15 4V2M15 16v-2M8 9h2M20 9h2M17.8 11.8L19 13M17.8 6.2L19 5M3 21l9-9M12.2 6.2L11 5", false, Tint::Normal },
    { "xlART_SELECTEFFECTS", "M3 8V3h5M16 3h5v5M21 16v5h-5M8 21H3v-5M9 9h6v6H9z", false, Tint::Normal },
    // Edit
    { "xlART_PASTE_BY_TIME", "M9 3h6v3H9zM8 4.5H6a1 1 0 0 0-1 1V20a1 1 0 0 0 1 1h5M16 4.5h2a1 1 0 0 1 1 1V10M17 17m-4 0a4 4 0 1 0 8 0a4 4 0 1 0-8 0M17 15v2l1.5 1", false, Tint::Normal },
    { "xlART_PASTE_BY_CELL", "M9 3h6v3H9zM8 4.5H6a1 1 0 0 0-1 1V20a1 1 0 0 0 1 1h5M16 4.5h2a1 1 0 0 1 1 1V10M13 13h8v8h-8zM17 13v8M13 17h8", false, Tint::Normal },
    // View
    { "xlART_ZOOM_IN", "M11 11m-7 0a7 7 0 1 0 14 0a7 7 0 1 0-14 0M21 21l-4.3-4.3M11 8v6M8 11h6", false, Tint::Normal },
    { "xlART_ZOOM_OUT", "M11 11m-7 0a7 7 0 1 0 14 0a7 7 0 1 0-14 0M21 21l-4.3-4.3M8 11h6", false, Tint::Normal },
    { "xlART_SETTINGS", "M12 12m-3 0a3 3 0 1 0 6 0a3 3 0 1 0-6 0M12 2v3M12 19v3M4.9 4.9l2.1 2.1M17 17l2.1 2.1M2 12h3M19 12h3M4.9 19.1L7 17M17 7l2.1-2.1", false, Tint::Normal },
    // Tools
    { "xlART_TOOLS_TEST", "M9 3h6M10 3v6L4.5 18.5A1.6 1.6 0 0 0 6 21h12a1.6 1.6 0 0 0 1.5-2.5L14 9V3M7 15h10", false, Tint::Normal },
    { "xlART_TOOLS_CHECKSEQUENCE", "M9 3h6v3H9zM8 4.5H6a1 1 0 0 0-1 1V20a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1V5.5a1 1 0 0 0-1-1h-2M9 14l2 2 4-4", false, Tint::Normal },
    { "xlART_TOOLS_PACKAGESEQUENCE", "M21 8l-9-5-9 5v8l9 5 9-5zM3 8l9 5 9-5M12 13v8", false, Tint::Normal },
    { "xlART_TOOLS_BATCHRENDER", "M3 6h9M3 12h9M3 18h9M18 3l-3 7h5l-3 8", false, Tint::Normal },
    { "xlART_TOOLS_BULKUPLOAD", "M7 18a4 4 0 0 1-.6-8A6 6 0 0 1 18 9a4 4 0 0 1 0 9M12 12v9M9 15l3-3 3 3", false, Tint::Normal },
    { "xlART_TOOLS_VIEWLOG", "M14 3H7a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V8zM14 3v5h5M9 13h6M9 17h6", false, Tint::Normal },
    // Auto-complete (AC) toolbar
    { "xlAC_DISABLED", "M9 2v5M15 2v5M6 7h12v4a6 6 0 0 1-12 0zM12 17v5M3 3l18 18", false, Tint::Normal },
    { "xlAC_ENABLED", "M9 2v5M15 2v5M6 7h12v4a6 6 0 0 1-12 0zM12 17v5", false, Tint::Accent },
    { "xlAC_SELECT", "M5 3l14 8-6 2-3 6z", false, Tint::Normal },
    { "xlAC_OFF", "M12 12m-8 0a8 8 0 1 0 16 0a8 8 0 1 0-16 0", false, Tint::Normal },
    { "xlAC_ON", "M12 12m-8 0a8 8 0 1 0 16 0a8 8 0 1 0-16 0", true, Tint::Normal },
    { "xlAC_SHIMMER", "M3 12l3-6 3 12 3-12 3 12 3-12 3 6", false, Tint::Normal },
    { "xlAC_TWINKLE", "M8 4l1.2 3.3L12.5 8.5 9.2 9.7 8 13l-1.2-3.3L3.5 8.5l3.3-1.2zM17 12l.9 2.1 2.1.9-2.1.9L17 18l-.9-2.1L14 15l2.1-.9z", false, Tint::Normal },
    { "xlAC_INTENSITY", "M4 18a8 8 0 1 1 16 0M12 18l4-6", false, Tint::Normal },
    { "xlAC_RAMPUP", "M3 20L21 5v15z", false, Tint::Normal },
    { "xlAC_RAMPDOWN", "M3 5l18 15H3z", false, Tint::Normal },
    { "xlAC_RAMPUPDOWN", "M3 20l9-15 9 15z", false, Tint::Normal },
    { "xlAC_FILL", "M5 11l7-7 8 8-7 7zM5 11h15M20 16s2 2.2 2 3.5a2 2 0 0 1-4 0c0-1.3 2-3.5 2-3.5", false, Tint::Normal },
    { "xlAC_CASCADE", "M3 20h4v-4h4v-4h4V8h4V4", false, Tint::Normal },
    { "xlAC_FOREGROUND", "M3 3h11v4M3 3v11h4M10 10h11v11H10z", false, Tint::Normal },
    { "xlAC_BACKGROUND", "M3 3h11v11H3zM17 10h4v11H10v-4", false, Tint::Normal },
    // Effect and color panel buttons
    { "xlART_valuecurve_notselected", "M3 17c4 0 4-10 9-10s5 10 9 10", false, Tint::Normal, 24 },
    { "xlART_valuecurve_selected", "M3 17c4 0 4-10 9-10s5 10 9 10", false, Tint::Accent, 24 },
    { "xlART_PADLOCK_OPEN", "M7 11V7a5 5 0 0 1 9.9-1M5 11h14v10H5z", false, Tint::Normal, 14 },
    { "xlART_PADLOCK_CLOSED", "M7 11V7a5 5 0 0 1 10 0v4M5 11h14v10H5z", false, Tint::Accent, 14 },
    { "xlART_DICE_ICON", "M6 3h12a3 3 0 0 1 3 3v12a3 3 0 0 1-3 3H6a3 3 0 0 1-3-3V6a3 3 0 0 1 3-3zM8.5 8.5h.01M15.5 8.5h.01M12 12h.01M8.5 15.5h.01M15.5 15.5h.01", false, Tint::Normal, 24 },
    { "xlART_colorpanel_reverse_xpm", "M4 8h15l-4-4M20 16H5l4 4", false, Tint::Normal, 16 },
    { "xlART_colorpanel_left_shift_xpm", "M20 12H5M11 6l-6 6 6 6", false, Tint::Normal, 16 },
    { "xlART_colorpanel_right_shift_xpm", "M4 12h15M13 6l6 6-6 6", false, Tint::Normal, 16 },
    // Model types (Layout tab buttons and model lists)
    { "xlART_Arches_ICON", "M2 20a5 5 0 0 1 10 0M12 20a5 5 0 0 1 10 0", false, Tint::Normal },
    { "xlART_ARCH_ICON", "M2 20a5 5 0 0 1 10 0M12 20a5 5 0 0 1 10 0", false, Tint::Normal },
    { "xlART_Candy Canes_ICON", "M9 21V8a3.5 3.5 0 0 1 7 0v1", false, Tint::Normal },
    { "xlART_CANE_ICON", "M9 21V8a3.5 3.5 0 0 1 7 0v1", false, Tint::Normal },
    { "xlART_Channel Block_ICON", "M3 8h4v8H3zM10 8h4v8h-4zM17 8h4v8h-4z", false, Tint::Normal },
    { "xlART_CHANNELBLOCK_ICON", "M3 8h4v8H3zM10 8h4v8h-4zM17 8h4v8h-4z", false, Tint::Normal },
    { "xlART_Circle_ICON", "M12 12m-8 0a8 8 0 1 0 16 0a8 8 0 1 0-16 0", false, Tint::Normal },
    { "xlART_CIRCLE_ICON", "M12 12m-8 0a8 8 0 1 0 16 0a8 8 0 1 0-16 0", false, Tint::Normal },
    { "xlART_Cube_ICON", "M12 2l9 5v10l-9 5-9-5V7zM3 7l9 5 9-5M12 12v10", false, Tint::Normal },
    { "xlART_CUBE_ICON", "M12 2l9 5v10l-9 5-9-5V7zM3 7l9 5 9-5M12 12v10", false, Tint::Normal },
    { "xlART_Custom_ICON", "M4 4h6v6H4zM14 4h6v6h-6zM4 14h6v6H4zM14 14h6v6h-6z", false, Tint::Normal },
    { "xlART_CUSTOM_ICON", "M4 4h6v6H4zM14 4h6v6h-6zM4 14h6v6H4zM14 14h6v6h-6z", false, Tint::Normal },
    { "xlART_DMX_ICON", "M8 21h8M12 17v4M7 4h10l-2 10H9z", false, Tint::Normal },
    { "xlART_Icicles_ICON", "M3 5h18M5 5v6M9 5v10M13 5v5M17 5v9M21 5v4", false, Tint::Normal },
    { "xlART_ICICLE_ICON", "M3 5h18M5 5v6M9 5v10M13 5v5M17 5v9M21 5v4", false, Tint::Normal },
    { "xlART_Image_ICON", "M3 5h18v14H3zM3 16l5-5 4 4 3-3 6 6M15.5 9h.01", false, Tint::Normal },
    { "xlART_IMAGE_ICON", "M3 5h18v14H3zM3 16l5-5 4 4 3-3 6 6M15.5 9h.01", false, Tint::Normal },
    { "xlART_Label_ICON", "M4 6h16M12 6v13M8 19h8", false, Tint::Normal },
    { "xlART_LABEL_ICON", "M4 6h16M12 6v13M8 19h8", false, Tint::Normal },
    { "xlART_Matrix_ICON", "M4 4h16v16H4zM4 9.3h16M4 14.7h16M9.3 4v16M14.7 4v16", false, Tint::Normal },
    { "xlART_MATRIX_ICON", "M4 4h16v16H4zM4 9.3h16M4 14.7h16M9.3 4v16M14.7 4v16", false, Tint::Normal },
    { "xlART_Poly Line_ICON", "M3 18l5-9 5 5 8-10", false, Tint::Normal },
    { "xlART_POLY_ICON", "M3 18l5-9 5 5 8-10", false, Tint::Normal },
    { "xlART_Single Line_ICON", "M3 18L21 6", false, Tint::Normal },
    { "xlART_LINE_ICON", "M3 18L21 6", false, Tint::Normal },
    { "xlART_Sphere_ICON", "M12 12m-9 0a9 9 0 1 0 18 0a9 9 0 1 0-18 0M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18", false, Tint::Normal },
    { "xlART_SPHERE_ICON", "M12 12m-9 0a9 9 0 1 0 18 0a9 9 0 1 0-18 0M3 12h18M12 3a14 14 0 0 1 0 18M12 3a14 14 0 0 0 0 18", false, Tint::Normal },
    { "xlART_Spinner_ICON", "M12 12m-9 0a9 9 0 1 0 18 0a9 9 0 1 0-18 0M12 3v18M3 12h18M5.6 5.6l12.8 12.8M18.4 5.6L5.6 18.4", false, Tint::Normal },
    { "xlART_SPINNER_ICON", "M12 12m-9 0a9 9 0 1 0 18 0a9 9 0 1 0-18 0M12 3v18M3 12h18M5.6 5.6l12.8 12.8M18.4 5.6L5.6 18.4", false, Tint::Normal },
    { "xlART_Star_ICON", "M12 3l2.6 5.6 6 .7-4.5 4.1 1.2 6L12 16.5 6.7 19.4l1.2-6-4.5-4.1 6-.7z", false, Tint::Normal },
    { "xlART_STAR_ICON", "M12 3l2.6 5.6 6 .7-4.5 4.1 1.2 6L12 16.5 6.7 19.4l1.2-6-4.5-4.1 6-.7z", false, Tint::Normal },
    { "xlART_Tree_ICON", "M12 2L4 20h16zM12 2v18", false, Tint::Normal },
    { "xlART_TREE_ICON", "M12 2L4 20h16zM12 2v18", false, Tint::Normal },
    { "xlART_Window Frame_ICON", "M4 4h16v16H4zM8 8h8v8H8z", false, Tint::Normal },
    { "xlART_WINDOW_ICON", "M4 4h16v16H4zM8 8h8v8H8z", false, Tint::Normal },
    { "xlART_Download_ICON", "M12 3v12M7 10l5 5 5-5M5 21h14", false, Tint::Normal },
    { "xlART_Import Custom_ICON", "M14 3H7a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V8zM14 3v5h5M12 11v6M9 14l3 3 3-3", false, Tint::Normal },
    { "xlART_Add Object_ICON", "M12 2l8 4.5v9L12 20l-8-4.5v-9zM12 8v6M9 11h6", false, Tint::Normal },
    // Preferences pages
    { "xlART_PREF_BACKUP", "M3 3h18v4H3zM4 7v13h16V7M10 12h4", false, Tint::Normal },
    { "xlART_PREF_VIEW", "M2 12s4-7 10-7 10 7 10 7-4 7-10 7S2 12 2 12zM12 12m-3 0a3 3 0 1 0 6 0a3 3 0 1 0-6 0", false, Tint::Normal },
    { "xlART_PREF_GRID", "M3 5h18v14H3zM3 10h18M3 15h18M9 5v14M15 5v14", false, Tint::Normal },
    { "xlART_PREF_SEQUENCES", "M14 3H7a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h10a2 2 0 0 0 2-2V8zM14 3v5h5M10 11v6l5-3z", false, Tint::Normal },
    { "xlART_PREF_OUTPUT", "M9 18h6M10 22h4M12 2a7 7 0 0 0-4 12.7V17h8v-2.3A7 7 0 0 0 12 2z", false, Tint::Normal },
    { "xlART_PREF_CHECK", "M9 3h6v3H9zM8 4.5H6a1 1 0 0 0-1 1V20a1 1 0 0 0 1 1h12a1 1 0 0 0 1-1V5.5a1 1 0 0 0-1-1h-2M9 14l2 2 4-4", false, Tint::Normal },
    { "xlART_PREF_RANDOM", "M6 3h12a3 3 0 0 1 3 3v12a3 3 0 0 1-3 3H6a3 3 0 0 1-3-3V6a3 3 0 0 1 3-3zM8.5 8.5h.01M15.5 8.5h.01M12 12h.01M8.5 15.5h.01M15.5 15.5h.01", false, Tint::Normal },
    { "xlART_PREF_COLORS", "M12 3a9 9 0 1 0 0 18c1 0 1.6-.8 1.6-1.6 0-.5-.2-.8-.5-1.2-.3-.3-.4-.7-.4-1.1 0-.9.7-1.6 1.6-1.6H16a5 5 0 0 0 5-5C21 6.2 17 3 12 3zM7.5 11.5h.01M10 7.5h.01M15 8h.01", false, Tint::Normal },
    { "xlART_PREF_OTHER", "M4 6h9M17 6h3M4 12h3M11 12h9M4 18h11M19 18h1M15 6a2 2 0 1 0 0 .01M9 12a2 2 0 1 0 0 .01M17 18a2 2 0 1 0 0 .01", false, Tint::Normal },
    { "xlART_PREF_TOOLBARS", "M3 4h18v6H3zM6 7h.01M10 7h.01M14 7h.01M3 14h11v6H3z", false, Tint::Normal },
    { "xlART_PREF_SERVICES", "M9 2v5M15 2v5M6 7h12v4a6 6 0 0 1-12 0zM12 17v5", false, Tint::Normal },
};

wxString Hex(const wxColour& c) {
    return wxString::Format("#%02X%02X%02X", c.Red(), c.Green(), c.Blue());
}

int s_useLineIcons = -1;
} // namespace

bool SavedUseLineIcons() {
    auto* config = GetXLightsConfig();
    return config == nullptr || config->Read("xLightsToolbarIconStyle", std::string("Line")) != "Classic";
}

bool UseLineIcons() {
    if (s_useLineIcons == -1) {
        s_useLineIcons = SavedUseLineIcons() ? 1 : 0;
    }
    return s_useLineIcons == 1;
}

void SetUseLineIcons(bool line) {
    auto* config = GetXLightsConfig();
    if (config != nullptr) {
        config->Write("xLightsToolbarIconStyle", std::string(line ? "Line" : "Classic"));
    }
}

wxBitmapBundle CreateLineIconBundle(const wxString& artId, int size) {
    if (!UseLineIcons()) {
        return wxBitmapBundle();
    }
    for (const auto& icon : ICONS) {
        if (artId != icon.id) {
            continue;
        }
        if (size <= 0) {
            size = icon.defaultSize;
            if (size <= 0) {
                return wxBitmapBundle();
            }
        }
        const bool dark = IsDarkMode();
        const wxColour color = icon.tint == Tint::Accent ? (dark ? wxColour(242, 169, 59) : wxColour(201, 133, 18))
                                                           : (dark ? wxColour(213, 217, 225) : wxColour(58, 64, 76));
        const wxString paint = icon.filled ? wxString::Format("fill=\"%s\" stroke=\"%s\" stroke-width=\"1\"", Hex(color), Hex(color))
                                           : wxString::Format("fill=\"none\" stroke=\"%s\" stroke-width=\"1.8\"", Hex(color));
        const wxString svg = wxString::Format("<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 24 24\"><path d=\"%s\" %s stroke-linecap=\"round\" stroke-linejoin=\"round\"/></svg>",
                                              icon.path, paint);
        const wxScopedCharBuffer utf8 = svg.utf8_str();
        return wxBitmapBundle::FromSVG(utf8.data(), wxSize(size, size));
    }
    return wxBitmapBundle();
}
