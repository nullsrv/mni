#ifndef MNI_H
#define MNI_H

#include "../../deps/icm/include/icm/icm.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if defined(__cplusplus)
extern "C" {
#endif

// Version
#define MNI_VERSION_MAJOR       3
#define MNI_VERSION_MINOR       9
#define MNI_VERSION_REVISON     9
#define MNI_VERSION             0x3909
#define MNI_VERSION_STRING      "3.9.9"

// MNI_API
#if defined(MNI_DLL)
    #if defined(MNI_EXPORTS)
        #define MNI_API extern __declspec(dllexport)
    #else
        #define MNI_API extern __declspec(dllimport)
    #endif // MNI_EXPORTS
#else
    #define MNI_API extern
#endif // MNI_DLL

// Error check macros
#define MNI_SUCCEEDED(_err)     (_err >= 0)
#define MNI_FAILED(_err)        (_err < 0)

// Boolean macros
#define MNI_FALSE               ((MniBool)0)
#define MNI_TRUE                ((MniBool)1)

// User defined message ids
#define MNI_USER_MESSAGE_ID     WM_APP

// User defined timer ids
#define MNI_USER_TIMER_ID       (1000)

// Null guid
#define MNI_GUID_NULL ((GUID){0x00000000L, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}})

// MniBool
typedef int MniBool;

// Forward declaration.
struct ModernNotifyIcon;

// MniError
typedef enum MniError {
    // Success code:
    MNI_OK                                  = 0,
    MNI_WINDOW_ALREADY_CREATED              = 1,
    MNI_ICON_ALREADY_CREATED                = 2,
    MNI_ICON_ALREADY_SHOWN                  = 3,
    MNI_ICON_ALREADY_HIDDEN                 = 4,

    // Error codes:
    MNI_ERROR_MNI_PTR_IS_NULL               = -1,
    MNI_ERROR_UNSUPPORTED_VERSION           = -2,
    MNI_ERROR_FAILED_TO_ADD_ICON            = -3,
    MNI_ERROR_FAILED_TO_DELETE_ICON         = -4,
    MNI_ERROR_FAILED_TO_SHOW_ICON           = -5,
    MNI_ERROR_FAILED_TO_HIDE_ICON           = -6,
    MNI_ERROR_FAILED_TO_CHANGE_ICON         = -7,
    MNI_ERROR_FAILED_TO_CHANGE_TIP          = -8,
    MNI_ERROR_FAILED_TO_COPY_TIP            = -9,
    MNI_ERROR_FAILED_TO_COPY_BALLOON_TITLE  = -10,
    MNI_ERROR_FAILED_TO_COPY_BALLOON_TEXT   = -11,
    MNI_ERROR_FAILED_TO_SHOW_BALLOON        = -12,
    MNI_ERROR_FAILED_TO_REMOVE_BALLOON      = -13,
    MNI_ERROR_FAILED_TO_REGISTER_WNDCLASS   = -14,
    MNI_ERROR_FAILED_TO_CREATE_WINDOW       = -15,
    MNI_ERROR_FAILED_TO_START_TIMER         = -16,
    MNI_ERROR_FAILED_TO_STOP_TIMER          = -17,
    MNI_ERROR_INVALID_MODULE_HANDLE         = -18,
    MNI_ERROR_INVALID_WINDOW_HANDLE         = -19,
    MNI_ERROR_INVALID_ICON                  = -20,
    MNI_ERROR_INVALID_MENU                  = -21,
    MNI_ERROR_INVALID_TIMER_ID              = -22,
    MNI_ERROR_INVALID_ARGUMENT              = -23,
    MNI_ERROR_ICON_NOT_CREATED              = -24,
    MNI_ERROR_INSUFFICIENT_BUFFER           = -25,
    MNI_ERROR_FAILED_TO_CONVERT_TIP         = -26,
    MNI_ERROR_FAILED_TO_CONVERT_TITLE       = -27,
    MNI_ERROR_FAILED_TO_CONVERT_TEXT        = -28,
    MNI_ERROR_FAILED_TO_SEND_MESSAGE        = -29,
    MNI_ERROR_FAILED_TO_POST_MESSAGE        = -30,
} MniError;

// MniBalloonFlags
typedef enum MniBalloonFlags {
    MNI_BALLOON_FLAGS_DEFAULT               = (1 << 0),
    MNI_BALLOON_FLAGS_REALTIME              = (1 << 1),
    MNI_BALLOON_FLAGS_PLAY_SOUND            = (1 << 2),
    MNI_BALLOON_FLAGS_RESPECT_QUIET_TIME    = (1 << 3),
} MniBalloonFlags;

// MniBalloonIconType
typedef enum MniBalloonIconType {
    MNI_BALLOON_ICON_TYPE_NONE              = 0,
    MNI_BALLOON_ICON_TYPE_SYSTEM_INFO       = 1,
    MNI_BALLOON_ICON_TYPE_SYSTEM_WARNING    = 2,
    MNI_BALLOON_ICON_TYPE_SYSTEM_ERROR      = 3,
    MNI_BALLOON_ICON_TYPE_CUSTOM            = 4,
} MniBalloonIconType;

// MniTipType
typedef enum MniTipType {
    MNI_TIP_TYPE_STANDARD                   = 0,
    MNI_TIP_TYPE_RICH_POPUP                 = 1,
} MniTipType;

// MniMenuPosition
typedef enum MniMenuPosition {
    MNI_MENU_POSITION_DEFAULT               = 0,
} MniMenuPosition;

// MniMenuAnimation
typedef enum MniMenuAnimation {
    MNI_MENU_ANIMATION_DEFAULT              = 0,
} MniMenuAnimation;

// MniTheme
typedef enum MniTheme {
    MNI_THEME_DARK                          = 0,
    MNI_THEME_LIGHT                         = 1,
    MNI_THEME_HIGHCONTRAST                  = 2,
} MniTheme;

// MniThemeInfo
typedef struct MniThemeInfo {
    MniTheme        Theme;
    DWORD           TextColor;          // BGR
    DWORD           BackgroundColor;    // BGR
} MniThemeInfo;

// Callbacks typedefs.
typedef void (*MniOnWindowCreateFn)         (struct ModernNotifyIcon *mni);
typedef void (*MniOnWindowDestroyFn)        (struct ModernNotifyIcon *mni);
typedef void (*MniOnInitFn)                 (struct ModernNotifyIcon *mni);
typedef void (*MniOnReleaseFn)              (struct ModernNotifyIcon *mni);
typedef void (*MniOnShowFn)                 (struct ModernNotifyIcon *mni);
typedef void (*MniOnHideFn)                 (struct ModernNotifyIcon *mni);
typedef void (*MniOnIconChangeFn)           (struct ModernNotifyIcon *mni, HICON icon);
typedef void (*MniOnMenuChangeFn)           (struct ModernNotifyIcon *mni, HMENU menu);
typedef void (*MniOnTipChangeFn)            (struct ModernNotifyIcon *mni, const wchar_t *tip);
typedef void (*MniOnTipTypeChangeFn)        (struct ModernNotifyIcon *mni, MniTipType mtt);
typedef void (*MniOnKeySelectFn)            (struct ModernNotifyIcon *mni, int x, int y);
typedef void (*MniOnLmbClickFn)             (struct ModernNotifyIcon *mni, int x, int y);
typedef void (*MniOnLmbDoubleClickFn)       (struct ModernNotifyIcon *mni, int x, int y);
typedef void (*MniOnMmbClickFn)             (struct ModernNotifyIcon *mni, int x, int y);
typedef void (*MniOnContextMenuOpenFn)      (struct ModernNotifyIcon *mni);
typedef void (*MniOnContextMenuItemClickFn) (struct ModernNotifyIcon *mni, int selected_item);
typedef void (*MniOnContextMenuCloseFn)     (struct ModernNotifyIcon *mni, MniBool was_item_selected);
typedef void (*MniOnBalloonShowFn)          (struct ModernNotifyIcon *mni);
typedef void (*MniOnBalloonHideFn)          (struct ModernNotifyIcon *mni);
typedef void (*MniOnBalloonTimeoutFn)       (struct ModernNotifyIcon *mni);
typedef void (*MniOnBalloonClickFn)         (struct ModernNotifyIcon *mni);
typedef void (*MniOnRichPopupOpenFn)        (struct ModernNotifyIcon *mni, int x, int y);
typedef void (*MniOnRichPopupCloseFn)       (struct ModernNotifyIcon *mni);
typedef void (*MniOnDpiChangeFn)            (struct ModernNotifyIcon *mni, int dpi);
typedef void (*MniOnSystemThemeChangeFn)    (struct ModernNotifyIcon *mni, MniThemeInfo mti);
typedef void (*MniOnAppsThemeChangeFn)      (struct ModernNotifyIcon *mni, MniThemeInfo mti);
typedef void (*MniOnTaskbarCreatedFn)       (struct ModernNotifyIcon *mni);
typedef void (*MniOnTimerFn)                (struct ModernNotifyIcon *mni, UINT id);

typedef void (*MniOnCustomMessageFn)(struct ModernNotifyIcon *mni, UINT msg, WPARAM wParam, LPARAM lParam);
typedef BOOL (*MniOnSystemMessageFn)(struct ModernNotifyIcon *mni, UINT msg, WPARAM wParam, LPARAM lParam);

// MniInfo
typedef struct MniInfo {
    HINSTANCE                   module_handle;
    const wchar_t               *class_name;
    GUID                        guid;
    HICON                       icon;
    HMENU                       menu;
    const wchar_t               *tip;
    MniTipType                  tip_type;
    IcmStyle                    icm_style;
    IcmTheme                    icm_theme;
    DWORD                       window_style;
    const wchar_t               *window_title;
    MniMenuPosition             menu_position;
    MniMenuAnimation            menu_animation;
    HWND                        parent_window;
    void                        *user_data1;
    void                        *user_data2;

    MniOnWindowCreateFn         on_window_create;
    MniOnWindowDestroyFn        on_window_destroy;
    MniOnInitFn                 on_init;
    MniOnReleaseFn              on_release;
    MniOnShowFn                 on_show;
    MniOnHideFn                 on_hide;
    MniOnIconChangeFn           on_icon_change;
    MniOnMenuChangeFn           on_menu_change;
    MniOnTipChangeFn            on_tip_change;
    MniOnTipTypeChangeFn        on_tip_type_change;
    MniOnKeySelectFn            on_key_select;
    MniOnLmbClickFn             on_lmb_click;
    MniOnLmbDoubleClickFn       on_lmb_double_click;
    MniOnMmbClickFn             on_mmb_click;
    MniOnContextMenuOpenFn      on_context_menu_open;
    MniOnContextMenuItemClickFn on_context_menu_item_click;
    MniOnContextMenuCloseFn     on_context_menu_close;
    MniOnBalloonShowFn          on_balloon_show;
    MniOnBalloonHideFn          on_balloon_hide;
    MniOnBalloonTimeoutFn       on_balloon_timeout;
    MniOnBalloonClickFn         on_balloon_click;
    MniOnRichPopupOpenFn        on_rich_popup_open;
    MniOnRichPopupCloseFn       on_rich_popup_close;
    MniOnDpiChangeFn            on_dpi_change;
    MniOnSystemThemeChangeFn    on_system_theme_change;
    MniOnAppsThemeChangeFn      on_apps_theme_change;
    MniOnTaskbarCreatedFn       on_taskbar_created;
    MniOnTimerFn                on_timer;
    MniOnCustomMessageFn        on_custom_message;
    MniOnSystemMessageFn        on_system_message;
} MniInfo;

// ModernNotifyIcon
typedef struct ModernNotifyIcon {
    HWND                        window_handle;
    HINSTANCE                   module_handle;
    HICON                       icon;
    HMENU                       menu;
    wchar_t                     tip[128];
    MniTipType                  tip_type;
    MniBool                     use_guid;
    GUID                        guid;
    MniThemeInfo                system_theme;
    MniThemeInfo                apps_theme;
    IcmStyle                    icm_style;
    IcmTheme                    icm_theme;
    int                         dpi;
    MniBool                     icon_created;
    MniBool                     icon_visible;
    MniMenuPosition             menu_position;
    MniMenuAnimation            menu_animation;
    MniBool                     is_dpi_event;
    MniBool                     prevent_double_key_select;
    int                         taskbar_created_message_id;
    const wchar_t               *class_name;
    HMONITOR                    primary_monitor;
    void                        *user_data1;
    void                        *user_data2;
    void                        *reserved1;
    void                        *reserved2;

    MniOnWindowCreateFn         on_window_create;
    MniOnWindowDestroyFn        on_window_destroy;
    MniOnInitFn                 on_init;
    MniOnReleaseFn              on_release;
    MniOnShowFn                 on_show;
    MniOnHideFn                 on_hide;
    MniOnIconChangeFn           on_icon_change;
    MniOnMenuChangeFn           on_menu_change;
    MniOnTipChangeFn            on_tip_change;
    MniOnTipTypeChangeFn        on_tip_type_change;
    MniOnKeySelectFn            on_key_select;
    MniOnLmbClickFn             on_lmb_click;
    MniOnLmbDoubleClickFn       on_lmb_double_click;
    MniOnMmbClickFn             on_mmb_click;
    MniOnContextMenuOpenFn      on_context_menu_open;
    MniOnContextMenuItemClickFn on_context_menu_item_click;
    MniOnContextMenuCloseFn     on_context_menu_close;
    MniOnBalloonShowFn          on_balloon_show;
    MniOnBalloonHideFn          on_balloon_hide;
    MniOnBalloonTimeoutFn       on_balloon_timeout;
    MniOnBalloonClickFn         on_balloon_click;
    MniOnRichPopupOpenFn        on_rich_popup_open;
    MniOnRichPopupCloseFn       on_rich_popup_close;
    MniOnDpiChangeFn            on_dpi_change;
    MniOnSystemThemeChangeFn    on_system_theme_change;
    MniOnAppsThemeChangeFn      on_apps_theme_change;
    MniOnTaskbarCreatedFn       on_taskbar_created;
    MniOnTimerFn                on_timer;
    MniOnCustomMessageFn        on_custom_message;
    MniOnSystemMessageFn        on_system_message;
} ModernNotifyIcon;

MNI_API MniError MniInit(ModernNotifyIcon *mni, MniInfo info);
MNI_API MniError MniRelease(ModernNotifyIcon *mni, MniBool destroy_icon, MniBool destroy_menu);
MNI_API MniError MniShow(ModernNotifyIcon *mni, MniBool recreate);
MNI_API MniError MniHide(ModernNotifyIcon *mni);

MNI_API MniError MniSetIcon(ModernNotifyIcon *mni, HICON icon, MniBool destroy_current);
MNI_API MniError MniSetMenu(ModernNotifyIcon *mni, HMENU menu, MniBool destroy_current);
MNI_API MniError MniSetTip(ModernNotifyIcon *mni, const wchar_t *tip);
MNI_API MniError MniSetTipType(ModernNotifyIcon *mni, MniTipType mtt);
MNI_API MniError MniGetIcon(ModernNotifyIcon *mni, HICON *icon);
MNI_API MniError MniGetMenu(ModernNotifyIcon *mni, HMENU *menu);
MNI_API MniError MniGetTip(ModernNotifyIcon *mni, wchar_t *buffer, int *len);
MNI_API MniError MniGetTipType(ModernNotifyIcon *mni, MniTipType *mtt);

MNI_API MniError MniIsNotifyIconCreated(ModernNotifyIcon *mni, MniBool *is_created);
MNI_API MniError MniIsNotifyIconVisible(ModernNotifyIcon *mni, MniBool *is_visible);
MNI_API MniError MniGetWindowHandle(ModernNotifyIcon *mni, HWND *window_handle);
MNI_API MniError MniGetModuleHandle(ModernNotifyIcon *mni, HINSTANCE *module_handle);
MNI_API MniError MniGetDpi(ModernNotifyIcon *mni, int *dpi);
MNI_API MniError MniGetSystemThemeInfo(ModernNotifyIcon *mni, MniThemeInfo *system_theme);
MNI_API MniError MniGetAppsThemeInfo(ModernNotifyIcon *mni, MniThemeInfo *apps_theme);

MNI_API MniError MniSetIcmStyle(ModernNotifyIcon *mni, IcmStyle icm_style);
MNI_API MniError MniSetIcmTheme(ModernNotifyIcon *mni, IcmTheme icm_theme);
MNI_API MniError MniGetIcmStyle(ModernNotifyIcon *mni, IcmStyle *icm_style);
MNI_API MniError MniGetIcmTheme(ModernNotifyIcon *mni, IcmTheme *icm_theme);

MNI_API MniError MniSetUserData1(ModernNotifyIcon *mni, void *data);
MNI_API MniError MniSetUserData2(ModernNotifyIcon *mni, void *data);
MNI_API MniError MniGetUserData1(ModernNotifyIcon *mni, void **data);
MNI_API MniError MniGetUserData2(ModernNotifyIcon *mni, void **data);

MNI_API MniError MniSendBalloonNotification(
    ModernNotifyIcon        *mni,
    const wchar_t           *title,
    const wchar_t           *text,
    MniBalloonIconType      icon_type,
    HICON                   icon,
    MniBalloonFlags         flags
);
MNI_API MniError MniRemoveBalloonNotification(ModernNotifyIcon *mni);

MNI_API MniError MniStartTimer(ModernNotifyIcon *mni, UINT timer_id, UINT interval);
MNI_API MniError MniStopTimer(ModernNotifyIcon *mni, UINT timer_id);

MNI_API MniError MniSendCustomMessage(ModernNotifyIcon *mni, UINT msg, WPARAM wParam, LPARAM lParam);
MNI_API MniError MniPostCustomMessage(ModernNotifyIcon *mni, UINT msg, WPARAM wParam, LPARAM lParam);

MNI_API const wchar_t *MniErrorToString(MniError error);

MNI_API int MniRunMessageLoop(void);
MNI_API void MniQuit(void);

MNI_API MniError MniSetTipUTF8(ModernNotifyIcon *mni, const char *tip);
MNI_API MniError MniGetTipUTF8(ModernNotifyIcon *mni, char *buffer, int *len);
MNI_API MniError MniSendBalloonNotificationUTF8(
    ModernNotifyIcon        *mni,
    const char              *title,
    const char              *text,
    MniBalloonIconType      icon_type,
    HICON                   icon,
    MniBalloonFlags         flags
);
MNI_API const char *MniErrorToStringUTF8(MniError error);

#if defined(__cplusplus)
}
#endif

#endif // MNI_H
