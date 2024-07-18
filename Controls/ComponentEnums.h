#ifndef COMPONENT_ENUMS_H
#define COMPONENT_ENUMS_H



namespace GUI
{
  enum ComponentFlagType
  {
    COMPFT_DEFAULT            = 0x00000000,
    COMPFT_INVISIBLE          = 0x00000001,
    COMPFT_DISABLED           = 0x00000002,
    COMPFT_FOCUSED            = 0x00000004,
    COMPFT_MOUSE_INSIDE       = 0x00000008,
    COMPFT_DRAG_ENABLED       = 0x00000010,
    COMPFT_DRAGGED            = 0x00000020,
    COMPFT_DESTROYED          = 0x00000040,   // die Komponente ist nicht mehr anzusprechen
    COMPFT_TAB_STOP           = 0x00000080,   // kann per Tastatur fokussiert werden
    COMPFT_ACTIVE             = 0x00000200,   // aktiv
    COMPFT_NOT_FOCUSABLE      = 0x00000400,   // bekommt nicht automatisch den Fokus bei Draufklicken
    COMPFT_DONT_CATCH_MOUSE   = 0x00000800,   // wird von Maus ignoriert (Maus klickt durch)
    COMPFT_CAPTION_FROM_DBASE = 0x00001000,   // Text wird beim Erzeugen aus der Datenbank übernommen
    COMPFT_NOT_SERIALIZABLE   = 0x00002000,   // Komponente ist eine interne Subkomponente
    COMPFT_OWNER_DRAW         = 0x00004000,   // Komponente wird über Event gezeichnet
    COMPFT_DRAG_CONTENT       = 0x00008000,   // hat drag-baren Content
    COMPFT_DRAG_CONTENT_TARGET= 0x00010000,   // kann drag-baren Content annehmen
    COMPFT_OWNER_DRAW_ITEM    = 0x00020000,   // Subitems werden über Event gezeichnet
    COMPFT_ROOT_WINDOW        = 0x00040000,   // is a independent root window (receives native "parent")
    COMPFT_NATIVE_CAPTION     = 0x00080000,
    COMPFT_NATIVE_RESIZABLE   = 0x00100000,
    COMPFT_NATIVE_MINIMIZABLE = 0x00200000,
    COMPFT_NATIVE_MAXIMIZABLE = 0x00400000,
    COMPFT_NATIVE_CLOSEABLE   = 0x00800000
  };

  enum VisualFlagType
  {
    VFT_DEFAULT               = 0,
    VFT_SUNKEN_BORDER         = 0x00000001,
    VFT_FLAT_BORDER           = 0x00000002,
    VFT_RAISED_BORDER         = 0x00000004,
    VFT_TRANSPARENT_BKGND     = 0x00000008,   // es wird kein Hintergrund dargestellt
    VFT_REPEAT_BKGND          = 0x00000010,   // Hintergrund wird gekachelt
    VFT_REPEAT_BORDER         = 0x00000020,   // border is repeated to fill instead of scaled
    VFT_HIDE_FOCUS_RECT       = 0x00000100    // Focusrect nicht anzeigen
  };

  enum AlignmentFlagType
  {
    AF_LEFT                   = 0x00000001,
    AF_RIGHT                  = 0x00000002,
    AF_CENTER                 = 0x00000003,
    AF_TOP                    = 0x00000004,
    AF_BOTTOM                 = 0x00000008,
    AF_VCENTER                = 0x0000000c,
    AF_MULTILINE              = 0x00000010,
    AF_AUTOWRAP               = 0x00000020,
    AF_SHADOW_OFFSET          = 0x00000040,
    AF_DEFAULT                = AF_CENTER | AF_VCENTER,
  };

  enum eBorderType
  {
    BT_EDGE_TOP_LEFT = 0,
    BT_EDGE_TOP,
    BT_EDGE_TOP_RIGHT,
    BT_EDGE_LEFT,
    BT_EDGE_RIGHT,
    BT_EDGE_BOTTOM_LEFT,
    BT_EDGE_BOTTOM,
    BT_EDGE_BOTTOM_RIGHT,

    BT_SUNKEN_EDGE_TOP_LEFT,
    BT_SUNKEN_EDGE_TOP,
    BT_SUNKEN_EDGE_TOP_RIGHT,
    BT_SUNKEN_EDGE_LEFT,
    BT_SUNKEN_EDGE_RIGHT,
    BT_SUNKEN_EDGE_BOTTOM_LEFT,
    BT_SUNKEN_EDGE_BOTTOM,
    BT_SUNKEN_EDGE_BOTTOM_RIGHT,

    BT_BACKGROUND,

    BT_EDGE_LAST_ENTRY,
  };

  enum eColorIndex
  {
    COL_FIRST_ENTRY   = 0,
    COL_SCROLLBAR     = 0,
    COL_BACKGROUND,
    COL_ACTIVECAPTION,
    COL_INACTIVECAPTION,
    COL_MENU,
    COL_WINDOW,
    COL_WINDOWFRAME,
    COL_MENUTEXT,
    COL_WINDOWTEXT,
    COL_CAPTIONTEXT,
    COL_ACTIVEBORDER,
    COL_INACTIVEBORDER,
    COL_APPWORKSPACE,
    COL_HIGHLIGHT,
    COL_HIGHLIGHTTEXT,
    COL_BTNFACE,
    COL_BTNSHADOW,
    COL_GRAYTEXT,
    COL_BTNTEXT,
    COL_INACTIVECAPTIONTEXT,
    COL_BTNHIGHLIGHT,
    COL_3DDKSHADOW,
    COL_3DLIGHT,
    COL_INFOTEXT,
    COL_INFOBK,
    COL_HOTLIGHT,
    COL_GRADIENTACTIVECAPTION,
    COL_GRADIENTINACTIVECAPTION,
    COL_MENUHILIGHT,
    COL_MENUBAR,
    COL_NC_AREA,
    COL_THIN_BORDER,
    COL_CURSOR,
    COL_HOTLIGHTTEXT,
    COL_HOTTIP_BACKGROUND,
    COL_HOTTIP_TEXT,

    COL_LAST_ENTRY
  };

  enum eCustomTextureSections
  {
    CTS_ARROW_UP = 0,
    CTS_ARROW_DOWN,
    CTS_ARROW_LEFT,
    CTS_ARROW_RIGHT,
    CTS_TOGGLE_MINUS,
    CTS_TOGGLE_PLUS,
    CTS_CHECKBOX_UNCHECKED,
    CTS_CHECKBOX_CHECKED,
    CTS_CHECKBOX_CHECK,       // nur der Haken an sich
    CTS_SLIDER,
    CTS_BUTTON,
    CTS_BUTTON_MOUSEOVER,
    CTS_BUTTON_PUSHED,
    CTS_COMBO_BUTTON,
    CTS_RADIO_UNCHECKED,
    CTS_RADIO_CHECKED,
    CTS_BUTTON_DISABLED,
    CTS_ARROW_UP_MOUSEOVER,
    CTS_ARROW_UP_PUSHED,
    CTS_ARROW_UP_DISABLED,
    CTS_ARROW_DOWN_MOUSEOVER,
    CTS_ARROW_DOWN_PUSHED,
    CTS_ARROW_DOWN_DISABLED,
    CTS_ARROW_LEFT_MOUSEOVER,
    CTS_ARROW_LEFT_PUSHED,
    CTS_ARROW_LEFT_DISABLED,
    CTS_ARROW_RIGHT_MOUSEOVER,
    CTS_ARROW_RIGHT_PUSHED,
    CTS_ARROW_RIGHT_DISABLED,
    CTS_SLIDER_TOP,
    CTS_SLIDER_VCENTER,
    CTS_SLIDER_BOTTOM,
    CTS_SLIDER_LEFT,
    CTS_SLIDER_HCENTER,
    CTS_SLIDER_RIGHT,

    CTS_USER_FIRST = 10000,
  };

};




#endif // COMPONENT_ENUMS_H



