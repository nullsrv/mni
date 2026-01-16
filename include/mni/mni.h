// The MIT License
// ===============
// 
// Copyright (c) 2023-2024 nullsrv
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// SPDX-License-Identifier: MIT

/**
 * @file mni.h
 * Wrapper around Windows notify icon.
 */
#ifndef MNI_H
#define MNI_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @defgroup version Version information.
 * @{
 */
#define MNI_VERSION_MAJOR       4                       ///< Major version constant.
#define MNI_VERSION_MINOR       0                       ///< Minor version constant.
#define MNI_VERSION_REVISON     2                       ///< Revision version constant.
#define MNI_VERSION             0x4002                  ///< Version number constant.
#define MNI_VERSION_STRING      "4.0.2"                 ///< Version string.
/** @} */

/**
 * @brief   Api linkage specifier.
 * @details If you want to use this library as DLL, define **MNI_DLL** before
 *          including mni.h
 */
#if defined(MNI_DLL) || defined(MNI_USE_DLL)
    #if defined(MNI_EXPORTS)
        #define MNI_API extern __declspec(dllexport)
    #else
        #define MNI_API extern __declspec(dllimport)
    #endif // MNI_EXPORTS
#else
    #define MNI_API extern
#endif // MNI_DLL

/**
 * @defgroup error Error checking macros.
 * @{
 */
#define MNI_SUCCEEDED(_err)     (_err >= 0)             ///< Check if result succeeded.
#define MNI_FAILED(_err)        (_err < 0)              ///< Check if result failed.
/** @} */

// Boolean macros
#define MNI_FALSE               ((MniBool)0)            ///< False value.
#define MNI_TRUE                ((MniBool)1)            ///< True value.

/**
 * @brief   User messages start id.
 * @details If you want to send custom messages and receive them through callback,
 *          use this as a starting id.
 */
#define MNI_USER_MESSAGE_ID     WM_APP

/**
 * @brief   Timer start id.
 * @details If you want to start custom timers and receive them through callback,
 *          use this as a starting id.
 */
#define MNI_USER_TIMER_ID       (0)

/**
 * @brief   Null guid.
 * @details Set MniInfo::guid to this value to not use guid.
 */
#define MNI_GUID_NULL ((GUID){0x00000000L, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}})

/**
 * @typedef MniBool
 * @brief Boolean type.
 */
typedef int MniBool;

// Forward declaration.
struct Mni4;

/**
 * @enum MniError
 * @brief List of error codes.
 */
typedef enum MniError {
    // Success code:
    MNI_OK                                  = 0,        ///< No error.

    // Warning codes:
    MNI_WINDOW_ALREADY_CREATED              = 1,        ///< Tried to create window, but it is already created.
    MNI_ICON_ALREADY_CREATED                = 2,        ///< Notify icon is already created.
    MNI_ICON_ALREADY_SHOWN                  = 3,        ///< Notify icon is already show.
    MNI_ICON_ALREADY_HIDDEN                 = 4,        ///< Notify icon is already hidden.

    // Error codes:
    MNI_ERROR_MNI_PTR_IS_NULL               = -1,       ///< mni ptr argument is NULL.
    MNI_ERROR_UNSUPPORTED_VERSION           = -2,       ///< Call to Shell_NotifyIcon(NIM_SETVERSION, ...) failed.
    MNI_ERROR_FAILED_TO_ADD_ICON            = -3,       ///< Call to Shell_NotifyIcon(NIM_ADD, ...) failed.
    MNI_ERROR_FAILED_TO_DELETE_ICON         = -4,       ///< Call to Shell_NotifyIcon(NIM_DELETE, ...) failed.
    MNI_ERROR_FAILED_TO_SHOW_ICON           = -5,       ///< Call to Shell_NotifyIcon(NIM_MODIFY, ...) failed.
    MNI_ERROR_FAILED_TO_HIDE_ICON           = -6,       ///< Call to Shell_NotifyIcon(NIM_MODIFY, ...) failed.
    MNI_ERROR_FAILED_TO_CHANGE_ICON         = -7,       ///< Call to Shell_NotifyIcon(NIM_MODIFY, ...) failed.
    MNI_ERROR_FAILED_TO_CHANGE_TIP          = -8,       ///< Call to Shell_NotifyIcon(NIM_MODIFY, ...) failed.
    MNI_ERROR_FAILED_TO_COPY_TIP            = -9,       ///< Unused code.
    MNI_ERROR_FAILED_TO_COPY_BALLOON_TITLE  = -10,      ///< Unused code.
    MNI_ERROR_FAILED_TO_COPY_BALLOON_TEXT   = -11,      ///< Unused code.
    MNI_ERROR_FAILED_TO_SHOW_BALLOON        = -12,      ///< Call to Shell_NotifyIcon(NIM_MODIFY, ...) failed.
    MNI_ERROR_FAILED_TO_REMOVE_BALLOON      = -13,      ///< Call to Shell_NotifyIcon(NIM_MODIFY, ...) failed.
    MNI_ERROR_FAILED_TO_REGISTER_WNDCLASS   = -14,      ///< Call to RegisterClassEx(...) failed.
    MNI_ERROR_FAILED_TO_CREATE_WINDOW       = -15,      ///< Call to CreateWindowEx(...) failed.
    MNI_ERROR_FAILED_TO_START_TIMER         = -16,      ///< Call to SetTimer(...) failed.
    MNI_ERROR_FAILED_TO_STOP_TIMER          = -17,      ///< Call to KillTimer(...) failed.
    MNI_ERROR_INVALID_MODULE_HANDLE         = -18,      ///< Failed to obtain module handle.
    MNI_ERROR_INVALID_WINDOW_HANDLE         = -19,      ///< mni->window_handle == NULL.
    MNI_ERROR_INVALID_ICON                  = -20,      ///< Unused code.
    MNI_ERROR_INVALID_MENU                  = -21,      ///< Unused code.
    MNI_ERROR_INVALID_TIMER_ID              = -22,      ///< Invalid timer id.
    MNI_ERROR_INVALID_ARGUMENT              = -23,      ///< Argument passed to function is invalid.
    MNI_ERROR_ICON_NOT_CREATED              = -24,      ///< Icon is not created.
    MNI_ERROR_INSUFFICIENT_BUFFER           = -25,      ///< Buffer is too small to fit the data.
    MNI_ERROR_FAILED_TO_CONVERT_TIP         = -26,      ///< UTF-8 to UTF-16 conversion failed.
    MNI_ERROR_FAILED_TO_CONVERT_TITLE       = -27,      ///< UTF-8 to UTF-16 conversion failed.
    MNI_ERROR_FAILED_TO_CONVERT_TEXT        = -28,      ///< UTF-8 to UTF-16 conversion failed.
    MNI_ERROR_FAILED_TO_SEND_MESSAGE        = -29,      ///< Call to SendNotifyMessage(...) failed.
    MNI_ERROR_FAILED_TO_POST_MESSAGE        = -30,      ///< Call to PostMessage(...) failed.
} MniError;

/**
 * @enum MniBalloonFlags
 * @brief Balloon notification bit flags.
 */
typedef enum MniBalloonFlags {
    MNI_BALLOON_FLAGS_DEFAULT               = (1 << 0), ///< Default flags.
    MNI_BALLOON_FLAGS_REALTIME              = (1 << 1), ///< Use this flag for notifications that represent
                                                        ///< real-time information which would be meaningless or
                                                        ///< misleading if displayed at a later time.
    MNI_BALLOON_FLAGS_PLAY_SOUND            = (1 << 2), ///< Play sound when showing notification.
    MNI_BALLOON_FLAGS_RESPECT_QUIET_TIME    = (1 << 3), ///< See https://learn.microsoft.com/en-us/windows/win32/api/shellapi/ns-shellapi-notifyicondataa#niif_respect_quiet_time-0x00000080
} MniBalloonFlags;

/**
 * @enum MniBalloonIconType
 * @brief Icon used on balloon notification.
 */
typedef enum MniBalloonIconType {
    MNI_BALLOON_ICON_TYPE_NONE              = 0,        ///< Notification would be shown without icon.
    MNI_BALLOON_ICON_TYPE_SYSTEM_INFO       = 1,        ///< Use information icon.
    MNI_BALLOON_ICON_TYPE_SYSTEM_WARNING    = 2,        ///< Use warning icon.
    MNI_BALLOON_ICON_TYPE_SYSTEM_ERROR      = 3,        ///< Use error icon.
    MNI_BALLOON_ICON_TYPE_CUSTOM            = 4,        ///< Use custom icon on notification toast.
} MniBalloonIconType;

/**
 * @enum MniTipType
 * @brief Tip type that should be shown when hover over icon.
 */
typedef enum MniTipType {
    MNI_TIP_TYPE_STANDARD                   = 0,        ///< Standard, text tip.
    MNI_TIP_TYPE_RICH_POPUP                 = 1,        ///< Rich Popup.
} MniTipType;

/**
 * @enum MniTheme
 * @brief Apps/System theme enum.
 */
typedef enum MniTheme {
    MNI_THEME_DARK                          = 0,        ///< Dark theme.
    MNI_THEME_LIGHT                         = 1,        ///< Light theme.
    MNI_THEME_HIGHCONTRAST                  = 2,        ///< High Contrast theme.
} MniTheme;

/**
 * @enum MniThemeInfo
 * @brief Information about theme.
 */
typedef struct MniThemeInfo {
    MniTheme        theme;                              ///< Theme.
    DWORD           text_color;         // BGR          ///< Text color.
    DWORD           background_color;   // BGR          ///< Background color.
} MniThemeInfo;

/**
 * @enum MniIcmStyle
 * @brief Style of popup menu. Unused.
 */
typedef enum MniIcmStyle {
    MNI_ICM_STYLE_AUTO                      = 0,        ///< Automatically choose style.
    MNI_ICM_STYLE_IMMERSIVE                 = 1,        ///< Use immersive style.
    MNI_ICM_STYLE_CLASSIC                   = 2,        ///< Use classic style.
    MNI_ICM_STYLE_WIN32                     = 3,        ///< Use win32 style.
} MniIcmStyle;

/**
 * @enum MniIcmTheme
 * @brief Theme of popup menu. Unused.
 */
typedef enum MniIcmTheme {
    MNI_ICM_THEME_AUTO                      = 0,        ///< Use system theme.
    MNI_ICM_THEME_LIGHT                     = 1,        ///< Force light theme.
    MNI_ICM_THEME_DARK                      = 2,        ///< Force dark theme.
} MniIcmTheme;

/**
 * @enum MniIcmPosition
 * @brief Position where popup menu is shown. Unused.
 */
typedef enum MniIcmPosition {
    MNI_ICM_POSITION_DEFAULT                = 0,        ///< Default position.
} MniIcmPosition;

/**
 * @enum MniIcmAnimation
 * @brief Animation used when showing popup. Unused.
 */
typedef enum MniIcmAnimation {
    MNI_ICM_ANIMATION_DEFAULT               = 0,        ///< Default animation.
} MniIcmAnimation;

/**
 * @defgroup callbacks Callbacks prototypes.
 * @{
 */
typedef void (*MniOnWindowCreateFn)         (struct Mni4 *mni);
typedef void (*MniOnWindowDestroyFn)        (struct Mni4 *mni);
typedef void (*MniOnInitFn)                 (struct Mni4 *mni);
typedef void (*MniOnReleaseFn)              (struct Mni4 *mni);
typedef void (*MniOnShowFn)                 (struct Mni4 *mni);
typedef void (*MniOnHideFn)                 (struct Mni4 *mni);
typedef void (*MniOnIconChangeFn)           (struct Mni4 *mni, HICON icon);
typedef void (*MniOnMenuChangeFn)           (struct Mni4 *mni, HMENU menu);
typedef void (*MniOnTipChangeFn)            (struct Mni4 *mni, const wchar_t *tip);
typedef void (*MniOnTipTypeChangeFn)        (struct Mni4 *mni, MniTipType mtt);
typedef void (*MniOnKeySelectFn)            (struct Mni4 *mni, int x, int y);
typedef void (*MniOnLmbClickFn)             (struct Mni4 *mni, int x, int y);
typedef void (*MniOnLmbDoubleClickFn)       (struct Mni4 *mni, int x, int y);
typedef void (*MniOnMmbClickFn)             (struct Mni4 *mni, int x, int y);
typedef void (*MniOnContextMenuOpenFn)      (struct Mni4 *mni);
typedef void (*MniOnContextMenuItemClickFn) (struct Mni4 *mni, int selected_item);
typedef void (*MniOnContextMenuCloseFn)     (struct Mni4 *mni, MniBool was_item_selected);
typedef void (*MniOnBalloonShowFn)          (struct Mni4 *mni);
typedef void (*MniOnBalloonHideFn)          (struct Mni4 *mni);
typedef void (*MniOnBalloonTimeoutFn)       (struct Mni4 *mni);
typedef void (*MniOnBalloonClickFn)         (struct Mni4 *mni);
typedef void (*MniOnRichPopupOpenFn)        (struct Mni4 *mni, int x, int y);
typedef void (*MniOnRichPopupCloseFn)       (struct Mni4 *mni);
typedef void (*MniOnDpiChangeFn)            (struct Mni4 *mni, int dpi);
typedef void (*MniOnSystemThemeChangeFn)    (struct Mni4 *mni, MniThemeInfo mti);
typedef void (*MniOnAppsThemeChangeFn)      (struct Mni4 *mni, MniThemeInfo mti);
typedef void (*MniOnTaskbarCreatedFn)       (struct Mni4 *mni);
typedef void (*MniOnTimerFn)                (struct Mni4 *mni, unsigned int id);

typedef void (*MniOnCustomMessageFn)(struct Mni4 *mni, UINT msg, WPARAM wParam, LPARAM lParam);
typedef BOOL (*MniOnSystemMessageFn)(struct Mni4 *mni, UINT msg, WPARAM wParam, LPARAM lParam);
/** @} */

/**
 * @struct MniInfo
 * @brief Information used to initialize Mni4.
 */
typedef struct MniInfo {
    HINSTANCE                   instance_handle;
    const wchar_t               *class_name;
    GUID                        guid;
    HICON                       icon;
    HMENU                       menu;
    const wchar_t               *tip;
    MniTipType                  tip_type;
    MniIcmStyle                 icm_style;
    MniIcmTheme                 icm_theme;
    DWORD                       window_style;
    const wchar_t               *window_title;
    MniIcmPosition              icm_position;
    MniIcmAnimation             icm_animation;
    HWND                        parent_window;
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
} MniInfo;

/**
 * @struct Mni4
 * @brief Structure representing icon in notification area.
 */
typedef struct Mni4 {
    HWND                        window_handle;
    HINSTANCE                   instance_handle;
    HICON                       icon;                   ///< icon handle
    HMENU                       menu;                   ///< menu handle
    wchar_t                     tip[128];
    MniTipType                  tip_type;
    MniBool                     use_guid;
    GUID                        guid;
    MniThemeInfo                system_theme;
    MniThemeInfo                apps_theme;
    MniIcmStyle                 icm_style;
    MniIcmTheme                 icm_theme;
    int                         dpi;
    MniBool                     icon_created;
    MniBool                     icon_visible;
    MniIcmPosition              icm_position;
    MniIcmAnimation             icm_animation;
    MniBool                     is_dpi_event;
    MniBool                     prevent_double_key_select;
    int                         taskbar_created_message_id;
    wchar_t                     class_name[32];
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
} Mni4;

/**
 * @brief       Initialize Mni4 structure.
 * @details     Perform all the initialization needed to create icon in notification area
 *              i.e. register window class, create invisible window and set some of the
 *              mni struct properties using info argument.
 *              Call this before any other calls.
 *              Note: this doesn't show the icon in notification area.
 * @param       mni             pointer to Mni4 struct
 * @param       info            information used to initialize Mni4 struct
 * @return      status code, see #MniError
 */
MNI_API MniError MniInit(Mni4 *mni, MniInfo info);

/**
 * @brief       Release all the resources.
 * @details     Delete notify icon, destroy window, unregister class.
 *              If destroy_icon is set to MNI_TRUE call DestroyIcon(mni->icon).
 *              If destroy_menu is set to MNI_TRUE call DestroyMenu(mni->menu).
 * @param       mni             pointer to Mni4 struct
 * @param       destroy_icon    set to MNI_TRUE to release Mni4::icon
 * @param       destroy_menu    set to MNI_TRUE to release Mni4::menu
 * @return      status code, see #MniError
 */
MNI_API MniError MniRelease(Mni4 *mni, MniBool destroy_icon, MniBool destroy_menu);

/**
 * @brief       Show the icon in notification area.
 * @details     
 * @param       mni             pointer to Mni4 struct
 * @param       recreate        set to MNI_TRUE to recreate icon
 * @return      status code, see #MniError
 */
MNI_API MniError MniShow(Mni4 *mni, MniBool recreate);

/**
 * @brief       Hide the icon in notification area.
 * @details     If icon is already hidden it returns #MNI_ICON_ALREADY_HIDDEN.
 * @param       mni             pointer to Mni4 struct
 * @return      status code, see #MniError
 */
MNI_API MniError MniHide(Mni4 *mni);

/**
 * @brief       Set icon that is visible in notification area.
 * @param       mni             pointer to Mni4 struct
 * @param       icon            handle to icon
 * @param       destroy_current destroy current icon before setting new one
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetIcon(Mni4 *mni, HICON icon, MniBool destroy_current);

/**
 * @brief       Set menu that is used when right clicking on notify icon.
 * @param       mni             pointer to Mni4 struct
 * @param       icon            handle to menu
 * @param       destroy_current destroy current menu before setting new one
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetMenu(Mni4 *mni, HMENU menu, MniBool destroy_current);

/**
 * @brief       Set tip that is visible in notification area.
 * @details     This is only displayed when Mni4::tip_type is set to #MNI_TIP_TYPE_STANDARD.
 * @param       mni             pointer to Mni4 struct
 * @param       tip             pointer to string with new tip
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetTip(Mni4 *mni, const wchar_t *tip);

/**
 * @brief       Set tip type.
 * @details     See: #MniTipType.
 * @param       mni             pointer to Mni4 struct
 * @param       mtt             tip type
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetTipType(Mni4 *mni, MniTipType mtt);

/**
 * @brief       Get current icon handle.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  icon            pointer to HICON that receive icon handle
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetIcon(Mni4 *mni, HICON *icon);

/**
 * @brief       Get current menu handle.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  menu            pointer to HMENU that receive menu handle
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetMenu(Mni4 *mni, HMENU *menu);

/**
 * @brief           Get current tip string.
 * @remarks         If \p buffer != **NULL**, \p len indicate \p buffer size.
 * @remarks         If \p buffer == **NULL**, \p len receive required buffer length in
 *                  characters (including '\0').
 * @param           mni         pointer to Mni4 struct
 * @param[out]      buffer      pointer to string that receive tip, see remarks
 * @param[in, out]  len         pointer to int, see remarks
 * @return          status code, see #MniError
 */
MNI_API MniError MniGetTip(Mni4 *mni, wchar_t *buffer, int *len);

/**
 * @brief       Get current tip type.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  mtt             pointer to MniTipType that receive current tip type
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetTipType(Mni4 *mni, MniTipType *mtt);

/**
 * @brief       Check if icon is created.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  is_created      pointer to MniBool that receive information
 * @return      status code, see #MniError
 */
MNI_API MniError MniIsNotifyIconCreated(Mni4 *mni, MniBool *is_created);

/**
 * @brief       Check if icon is visible in.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  is_visible      pointer to MniBool that receive information
 * @return      status code, see #MniError
 */
MNI_API MniError MniIsNotifyIconVisible(Mni4 *mni, MniBool *is_visible);

/**
 * @brief       Get handle of window that was used to create notify icon.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  window_handle   pointer to HWND that receive window handle
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetWindowHandle(Mni4 *mni, HWND *window_handle);

/**
 * @brief       Get handle of module instance that was used to create notify icon.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  window_handle   pointer to HINSTANCE that receive instance handle
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetInstanceHandle(Mni4 *mni, HINSTANCE *instance_handle);

/**
 * @brief       Get current system dpi.
 * @details     This value is dpi of primary monitor.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  dpi             pointer to int that receive dpi
 * @return  status code, see #MniError
 */
MNI_API MniError MniGetDpi(Mni4 *mni, int *dpi);

/**
 * @brief       Get current system theme.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  system_theme    pointer to MniThemeInfo that receive theme information
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetSystemThemeInfo(Mni4 *mni, MniThemeInfo *system_theme);

/**
 * @brief       Get current application theme.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  system_theme    pointer to MniThemeInfo that receive theme information
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetAppsThemeInfo(Mni4 *mni, MniThemeInfo *apps_theme);

/**
 * @brief       Set style of context menu.
 * @param       mni             pointer to Mni4 struct
 * @param       icm_style       context menu style
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetIcmStyle(Mni4 *mni, MniIcmStyle icm_style);

/**
 * @brief       Set theme of context menu.
 * @param       mni             pointer to Mni4 struct
 * @param       icm_theme       context menu theme
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetIcmTheme(Mni4 *mni, MniIcmTheme icm_theme);

/**
 * @brief       Set position of where context menu will be shown.
 * @param       mni             pointer to Mni4 struct
 * @param       icm_pos         context menu position
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetIcmPosition(Mni4 *mni, MniIcmPosition icm_pos);

/**
 * @brief       Set animation of context menu.
 * @param       mni             pointer to Mni4 struct
 * @param       icm_anim        context menu animation
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetIcmAnimation(Mni4 *mni, MniIcmAnimation icm_anim);

/**
 * @brief       Get style of context menu.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  icm_style       pointer to MniIcmStyle that receive style
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetIcmStyle(Mni4 *mni, MniIcmStyle *icm_style);

/**
 * @brief       Get style of context menu.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  icm_theme       pointer to MniIcmTheme that receive theme
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetIcmTheme(Mni4 *mni, MniIcmTheme *icm_theme);

/**
 * @brief       Get position of where context menu will be shown.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  icm_pos         pointer to MniIcmPosition that receive position information
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetIcmPosition(Mni4 *mni, MniIcmPosition *icm_pos);

/**
 * @brief       Get animation of context menu.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  icm_anim        pointer to MniIcmAnimation that receive animation
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetIcmAnimation(Mni4 *mni, MniIcmAnimation *icm_anim);

/**
 * @brief       Set first user data in Mni4 struct.
 * @details     Setting user data is useful if you want to access
 *              some information in callbacks.
 * @param       mni             pointer to Mni4 struct
 * @param       data            pointer to data
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetUserData1(Mni4 *mni, void *data);

/**
 * @brief       Set second user data in Mni4 struct.
 * @details     Setting user data is useful if you want to access
 *              some information in callbacks.
 * @param       mni             pointer to Mni4 struct
 * @param       data            pointer to data
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetUserData2(Mni4 *mni, void *data);

/**
 * @brief       Get first user data.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  data            pointer to receive user data
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetUserData1(Mni4 *mni, void **data);

/**
 * @brief       Get first user data.
 * @param       mni             pointer to Mni4 struct
 * @param[out]  data            pointer to receive user data
 * @return      status code, see #MniError
 */
MNI_API MniError MniGetUserData2(Mni4 *mni, void **data);

/**
 * @brief       Send balloon notification.
 * @param       mni             pointer to Mni4 struct
 * @param       title           notification title
 * @param       title           notification content
 * @param       icon_type       notification icon
 * @param       icon            handle to icon (or **NULL**)
 * @param       flags           notificaion flags
 * @return      status code, see #MniError
 */
MNI_API MniError MniSendBalloonNotification(
    Mni4                    *mni,
    const wchar_t           *title,
    const wchar_t           *text,
    MniBalloonIconType      icon_type,
    HICON                   icon,
    MniBalloonFlags         flags
);

/**
 * @brief       Remove notificaion.
 * @param       mni             pointer to Mni4 struct
 * @return      status code, see #MniError
 */
MNI_API MniError MniRemoveBalloonNotification(Mni4 *mni);

/**
 * @brief       Start timer.
 * @param       mni             pointer to Mni4 struct
 * @param       timer_id        timer id
 * @param       interval        timer timeout interval
 * @return      status code, see #MniError
 */
MNI_API MniError MniStartTimer(Mni4 *mni, unsigned int timer_id, unsigned int interval);

/**
 * @brief       Stop timer.
 * @param       mni             pointer to Mni4 struct
 * @param       timer_id        timer id
 * @return      status code, see #MniError
 */
MNI_API MniError MniStopTimer(Mni4 *mni, unsigned int timer_id);

/**
 * @brief       Send custom massage to queue.
 * @details     If calling thread is the same as window creation thread then blocks.
 *              If calling thread is different then return immediately.
 * @param       mni             pointer to Mni4 struct
 * @param       msg             message id
 * @param       wParam          message specific information
 * @param       lParam          message specific information
 * @return      status code, see #MniError
 */
MNI_API MniError MniSendCustomMessage(Mni4 *mni, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * @brief       Post custom massage to queue and return immediately.
 * @param       mni             pointer to Mni4 struct
 * @param       msg             message id
 * @param       wParam          message specific information
 * @param       lParam          message specific information
 * @return      status code, see #MniError
 */
MNI_API MniError MniPostCustomMessage(Mni4 *mni, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * @brief       Convert error code to string.
 * @param       error           error id
 * @return      error string
 */
MNI_API const wchar_t *MniErrorToString(MniError error);

/**
 * @brief       Process messages.
 * @return      exit code
 */
MNI_API int MniRunMessageLoop(void);

/**
 * @brief       Post quit message to message queue.
 */
MNI_API void MniQuit(void);

/**
 * @brief       Set notify icon tip using UTF-8 string.
 * @param       mni             pointer to Mni4 struct
 * @param       tip             tip text
 * @return      status code, see #MniError
 */
MNI_API MniError MniSetTipUTF8(Mni4 *mni, const char *tip);

/**
 * @brief           Get current tip in UTF-8 string.
 * @remarks         If \p buffer != **NULL**, \p len indicate \p buffer size.
 * @remarks         If \p buffer == **NULL**, \p len receive required buffer length in
 *                  characters (including '\0').
 * @param           mni         pointer to Mni4 struct
 * @param[out]      buffer      pointer to string that receive tip, see remarks
 * @param[in, out]  len         pointer to int, see remarks
 * @return          status code, see #MniError
 */
MNI_API MniError MniGetTipUTF8(Mni4 *mni, char *buffer, int *len);

/**
 * @brief       Send balloon notification using UTF-8 string.
 * @param       mni             pointer to Mni4 struct
 * @param       title           notification title
 * @param       title           notification content
 * @param       icon_type       notification icon
 * @param       icon            handle to icon (or **NULL**)
 * @param       flags           notificaion flags
 * @return      status code, see #MniError
 */
MNI_API MniError MniSendBalloonNotificationUTF8(
    Mni4                    *mni,
    const char              *title,
    const char              *text,
    MniBalloonIconType      icon_type,
    HICON                   icon,
    MniBalloonFlags         flags
);

/**
 * @brief       Convert error code to UTF-8 string.
 * @param       error           error id
 * @return      error string
 */
MNI_API const char *MniErrorToStringUTF8(MniError error);

#if defined(__cplusplus)
}
#endif

#endif // MNI_H
