#include "../include/mni/mni.h"

#include <shellapi.h>   // Shell_NotifyIconW

#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))

// ========================================================================== //

#define WM_NOTIFYICON                           (WM_USER + 0)
#define WM_DPICHANGED_DELAYED                   (WM_USER + 1)
#define WM_MNI_INIT                             (WM_USER + 2)
#define WM_MNI_RELEASE                          (WM_USER + 3)
#define WM_MNI_SHOW                             (WM_USER + 4)
#define WM_MNI_HIDE                             (WM_USER + 5)
#define WM_MNI_ICON_CHANGE                      (WM_USER + 6)
#define WM_MNI_MENU_CHANGE                      (WM_USER + 7)
#define WM_MNI_TIP_CHANGE                       (WM_USER + 8)
#define WM_MNI_TIP_TYPE_CHANGE                  (WM_USER + 9)

#define WM_APP_LAST                             (0xBFFF)

#define TIMER_LMB_DOUBLE_CLICK_CHECK            (1)
#define TIMER_PREVENT_DOUBLE_KEYSELECT          (2)

#define TIMER_LMB_DOUBLE_CLICK_CHECK_INTERVAL   (100)
#define TIMER_PREVENT_DOUBLE_KEYSELECT_INTERVAL (100)

#define MNI_TASKBAR_CREATED_WINDOW_MESSAGE      TEXT("TaskbarCreated")

// ========================================================================== //

#pragma region Macros

// ========================================================================== //
// MNI_ASSERT macro
// ========================================================================== //
#if defined(_DEBUG)
    #include <assert.h>
    #define MNI_ASSERT(_expr) assert(_expr)
#else
    #define MNI_ASSERT(_expr) do{}while(0)
#endif // _DEBUG

// ========================================================================== //
// MNI_TRACE macro
// ========================================================================== //
#define MNI_USE_TRACE 0
#define MNI_TRACE_WINDOW_MESSAGES

#if MNI_USE_TRACE > 0
    #include <stdarg.h>
    #include <stdio.h>
    #include <debugapi.h>
    
    #define TRACE_BUFFER_SIZE 1024

    static void _MniTrace(const wchar_t *format, ...) {
        if (IsDebuggerPresent()) {
            wchar_t buffer[TRACE_BUFFER_SIZE];
            memset(buffer, 0, sizeof(buffer));

            va_list args;
            va_start(args, format);
            int size = _vsnwprintf_s(buffer, TRACE_BUFFER_SIZE, _TRUNCATE, format, args);
            va_end(args);

            if (size > 0) {
                OutputDebugStringW(buffer);
                OutputDebugStringW(L"\n");
            }
            
            //static BOOL is_console = FALSE;
            //if (!is_console) {
            //    is_console = AllocConsole();
            //}

            //if (is_console) {
            //    FILE *out = NULL;
            //    freopen_s(&out, "CONOUT$", "w", stdout);
            //    if (out) {
            //        DWORD attr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            //        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attr);
            //        wprintf(L"%s\n", buffer);
            //        fclose(out);
            //    }
            //}
        }
    }

    #undef TRACE_BUFFER_SIZE

    #if defined(__GNUC__)
        #if MNI_USE_TRACE > 1
            #define MNI_TRACE(...) do{}while(0)
            #define MNI_TRACE2(fmt, ...) _MniTrace(fmt, " ", ##__VA_ARGS__)
        #else
            #define MNI_TRACE(fmt, ...) _MniTrace(fmt, " ", ##__VA_ARGS__)
            #define MNI_TRACE2(...) do{}while(0)
        #endif
    #else
        #if MNI_USE_TRACE > 1
            #define MNI_TRACE(...) do{}while(0)
            #define MNI_TRACE2(fmt, ...) _MniTrace(fmt, __VA_ARGS__)
        #else
            #define MNI_TRACE(fmt, ...) _MniTrace(fmt, __VA_ARGS__)
            #define MNI_TRACE2(...) do{}while(0)
        #endif
    #endif
#else
    #define MNI_TRACE(...) do{}while(0)
    #define MNI_TRACE2(...) do{}while(0)
#endif

#pragma endregion

// ========================================================================== //

#pragma region Helpers

// ========================================================================== //

static MniBool _IsLayoutRTL(void) {
    MniBool rtl = MNI_FALSE;
            
    DWORD layout = 0;
    if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IREADINGLAYOUT, (LPWSTR)&layout, 2))
    {
        // Returns one of the following 4 reading layout values:
        // 0 - Read from left to right
        // 1 - Read from right to left
        // 2 - Either read vertically from top to bottom with columns going from right to left,
        //     or read in horizontal rows from left to right
        // 3 - Read vertically from top to bottom with columns going from left to right
        if ((layout & 1) == 1)
        {
            rtl = MNI_TRUE;
        }
    }

    return rtl;
}

// ========================================================================== //

static DWORD _RegGetDword(const wchar_t *lpSubKey, const wchar_t *lpValue) {
    DWORD dwType = 0;
    DWORD dwData = 0;
    DWORD cbData = sizeof(dwData);

    LSTATUS status = RegGetValueW(
        HKEY_CURRENT_USER,
        lpSubKey,
        lpValue,
        RRF_RT_DWORD,
        &dwType,
        &dwData,
        &cbData
    );

    if (status == ERROR_SUCCESS) {        
        return dwData;
    }

    return 0;
}

// ========================================================================== //

static MniBool _AppsUseLightTheme(void) {
    DWORD ret = _RegGetDword(
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
        TEXT("AppsUseLightTheme")
    );

    return ret != 0;
}

// ========================================================================== //

static MniBool _SystemUsesLightTheme(void) {
    DWORD ret = _RegGetDword(
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize"),
        TEXT("SystemUsesLightTheme")
    );

    return ret != 0;
}

// ========================================================================== //

static MniBool _IsHighContrastThemeEnabled(void) {
    HIGHCONTRASTW hc;
    hc.cbSize = sizeof(hc);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0)) {
        // High contrast ON.
        if ((hc.dwFlags & HCF_HIGHCONTRASTON) != 0)
        {
            return MNI_TRUE;
        }
    }

    return MNI_FALSE;
}

// ========================================================================== //

static MniThemeInfo _GetAppsThemeInfo(void) {
    if (_AppsUseLightTheme()) {
        return (MniThemeInfo){
            .Theme              = MNI_THEME_LIGHT,
            .TextColor          = 0x00000000,
            .BackgroundColor    = 0x00FFFFFF,
        };
    }

    return (MniThemeInfo){
        .Theme              = MNI_THEME_DARK,
        .TextColor          = 0x00FFFFFF,
        .BackgroundColor    = 0x00000000,
    };
}

// ========================================================================== //

static MniThemeInfo _GetSystemThemeInfo(void) {
    if (_SystemUsesLightTheme()) {
        return (MniThemeInfo){
            .Theme              = MNI_THEME_LIGHT,
            .TextColor          = 0x00000000,
            .BackgroundColor    = 0x00FFFFFF,
        };
    }

    return (MniThemeInfo){
        .Theme              = MNI_THEME_DARK,
        .TextColor          = 0x00FFFFFF,
        .BackgroundColor    = 0x00000000,
    };
}

// ========================================================================== //

static MniThemeInfo _GetHighContrastThemeInfo(void) {
    DWORD fg = GetSysColor(COLOR_WINDOWTEXT);
    DWORD bg = GetSysColor(COLOR_WINDOW);

    return (MniThemeInfo){
        .Theme              = MNI_THEME_HIGHCONTRAST,
        .TextColor          = fg,
        .BackgroundColor    = bg,
    };
}

// ========================================================================== //

static HMONITOR _GetPrimaryMonitor(void) {
    POINT pt = {0, 0};
    return MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
}

// ========================================================================== //

static int _GetDpi(HWND hWnd) {
    typedef UINT (WINAPI *pfnGetDpiForWindow)(HWND);
    
    // This is available since Windows 10 1607
    pfnGetDpiForWindow GetDpiForWindowFn = 
        (pfnGetDpiForWindow)GetProcAddress(GetModuleHandleW(L"User32"), "GetDpiForWindow");

    int dpi = 96;

    if (GetDpiForWindowFn) {
        dpi = (int)GetDpiForWindowFn(hWnd);
    } else {
        HDC hDC = GetDC(NULL);
        if (hDC != NULL) {
            int logPixelsX = GetDeviceCaps(hDC, LOGPIXELSX);
            //int logPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
            ReleaseDC(NULL, hDC);

            dpi = logPixelsX;
        }
    }

    return dpi;
}

// ========================================================================== //

// dest must be valid memory location!
// This functions make sure that dest will be null terminated (if cch > 0).
static int _StringCopyW(wchar_t *dest, int cch, const wchar_t *src) {
    MNI_ASSERT(dest && "invalid dest ptr");
    MNI_ASSERT(cch > 0 && "cch is <= 0");

    if (cch <= 0) {
        return 0;
    }

    int i = 0;

    if (!src) {
        dest[0] = L'\0';
        i = 1;
    } else {
        for (i = 0; i < cch; i += 1) {
            dest[i] = src[i];

            if (src[i] == L'\0') {
                break;
            }
        }

        if (i > 0 && i == cch) {
            dest[i - 1] = L'\0';
        }
    }

    return i;
}

// ========================================================================== //

static int _StringLengthMaxW(const wchar_t *str, int max) {
    int i = 0;

    if (str) {
        for (i = 0; i < max; i += 1) {
            if (str[i] == L'\0') {
                break;
            }
        }
    }

    return i;
}

// ========================================================================== //

static int _StringLengthMaxA(const char *str, int max) {
    int i = 0;

    if (str) {
        for (i = 0; i < max; i += 1) {
            if (str[i] == '\0') {
                break;
            }
        }
    }

    return i;
}

// ========================================================================== //

static int _StringCompareW(const wchar_t *lhs, const wchar_t *rhs, int cch) {
    if (lhs == NULL && rhs != NULL) {
        return -1;
    }

    if (lhs != NULL && rhs == NULL) {
        return 1;
    }

    if (lhs == NULL && rhs == NULL) {
        return 0;
    }

    int i = 0;
    for (i = 0; i < cch; i += 1) {
        if (lhs[i] != rhs[i]) {
            return (unsigned char)lhs[i] < (unsigned char)rhs[i] ? -1 : 1;
        }

        if (lhs[i] == L'\0' || rhs[i] == L'\0') {
            break;
        }
    }

    return 0;
}

// ========================================================================== //

static MniBool _IsGuidEq(GUID guid1, GUID guid2) {
    return guid1.Data1 == guid2.Data1
        && guid1.Data2 == guid2.Data2
        && guid1.Data3 == guid2.Data3
        && guid1.Data4[0] == guid2.Data4[0]
        && guid1.Data4[1] == guid2.Data4[1]
        && guid1.Data4[2] == guid2.Data4[2]
        && guid1.Data4[3] == guid2.Data4[3]
        && guid1.Data4[4] == guid2.Data4[4]
        && guid1.Data4[5] == guid2.Data4[5]
        && guid1.Data4[6] == guid2.Data4[6]
        && guid1.Data4[7] == guid2.Data4[7]
        ;
}

// ========================================================================== //

static MniBool _IsThemeInfoChanged(MniThemeInfo lhs, MniThemeInfo rhs) {
    return lhs.Theme != rhs.Theme
        || lhs.TextColor != rhs.TextColor
        || lhs.BackgroundColor != rhs.BackgroundColor
        ;
}

// ========================================================================== //

// NOTE: if utf16 buffer is not big enough, the result will be truncated.
static MniBool _UTF8ToUTF16(const char *utf8, wchar_t *utf16, int cch) {
    MNI_ASSERT(cch > 0 && "cch is <= 0");

    if (cch <= 0) {
        return MNI_FALSE;
    }

    int len = _StringLengthMaxA(utf8, cch * 4);
    int ret = MultiByteToWideChar(CP_UTF8, 0, utf8, len, utf16, cch - 1);

    if (ret == 0) {
        DWORD err = GetLastError();
        if (err != ERROR_INSUFFICIENT_BUFFER) {
            return MNI_FALSE;
        }

        utf16[cch - 1] = L'\0';
    } else {
        utf16[ret] = L'\0';
    }

    return MNI_TRUE;
}

#pragma endregion

// ========================================================================== //

#pragma region Window Messages

static MniBool _MniWmWindowCreate(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmWindowCreate()");

    if (mni->on_window_create) {
        mni->on_window_create(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmWindowDestroy(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmWindowDestroy()");

    if (mni->on_window_destroy) {
        mni->on_window_destroy(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmInit(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmInit()");

    if (mni->on_init) {
        mni->on_init(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmRelease(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmRelease()");

    if (mni->on_release) {
        mni->on_release(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmIconShow(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmIconShow()");

    if (mni->on_show) {
        mni->on_show(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmIconHide(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmIconHide()");

    if (mni->on_hide) {
        mni->on_hide(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmIconChange(ModernNotifyIcon *mni, HICON icon) {
    MNI_TRACE(L"_MniWmIconChange()");

    if (mni->on_icon_change) {
        mni->on_icon_change(mni, icon);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmMenuChange(ModernNotifyIcon *mni, HMENU menu) {
    MNI_TRACE(L"_MniWmMenuChange()");

    if (mni->on_menu_change) {
        mni->on_menu_change(mni, menu);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmTipChange(ModernNotifyIcon *mni, const wchar_t *tip) {
    MNI_TRACE(L"_MniWmTipChange()");

    if (mni->on_tip_change) {
        mni->on_tip_change(mni, tip);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmTipTypeChange(ModernNotifyIcon *mni, MniTipType mtt) {
    MNI_TRACE(L"_MniWmTipTypeChange()");

    if (mni->on_tip_type_change) {
        mni->on_tip_type_change(mni, mtt);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmKeySelect(ModernNotifyIcon *mni, int x, int y) {
    MNI_TRACE(
        L"_MniWmKeySelect(x=%d, y=%d), prevent_double_key_select=%d",
        x,
        y,
        mni->prevent_double_key_select
    );

    if (!mni->prevent_double_key_select) {
        mni->prevent_double_key_select = MNI_TRUE;

        SetTimer(
            mni->window_handle,
            TIMER_PREVENT_DOUBLE_KEYSELECT,
            TIMER_PREVENT_DOUBLE_KEYSELECT_INTERVAL,
            NULL
        );

        if (mni->on_key_select) {
            mni->on_key_select(mni, x, y);
        }
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmLmbClick(ModernNotifyIcon *mni, int x, int y) {
    MNI_TRACE(L"_MniWmLmbClick(x=%d, y=%d)", x, y);

    if (mni->on_lmb_click) {
        mni->on_lmb_click(mni, x, y);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmLmbDoubleClick(ModernNotifyIcon *mni, int x, int y) {
    MNI_TRACE(L"_MniWmLmbDoubleClick(x=%d, y=%d)", x, y);
    
    if (mni->on_lmb_double_click) {
        mni->on_lmb_double_click(mni, x, y);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmMmbClick(ModernNotifyIcon *mni, int x, int y) {
    MNI_TRACE(L"_MniWmMmbClick(x=%d, y=%d)", x, y);

    if (mni->on_mmb_click) {
        mni->on_mmb_click(mni, x, y);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmContextMenu(ModernNotifyIcon *mni, int x, int y) {
    MNI_TRACE(L"_MniWmContextMenu(x=%d, y=%d)", x, y);
    if (mni->on_context_menu_open) {
        mni->on_context_menu_open(mni);
    }

    BOOL selected_item = 0;

    if (mni->menu) {
        HMENU hSubMenu = GetSubMenu(mni->menu, 0);
        if (hSubMenu) {
            // Our window must be foreground before calling TrackPopupMenu
            // or the menu will not disappear when the user clicks away.
            SetForegroundWindow(mni->window_handle);

            UINT uFlags = TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_TOPALIGN;

            int alignment = GetSystemMetrics(SM_MENUDROPALIGNMENT);

            // Respect menu drop alignment.
            if (alignment != 0) {
                uFlags |= TPM_RIGHTALIGN;
            } else {
                uFlags |= TPM_LEFTALIGN;
            }

            if (_IsLayoutRTL()) {
                uFlags = uFlags | TPM_LAYOUTRTL;                
            }

            {
                selected_item = ImmersiveTrackPopupMenu(
                    hSubMenu,
                    uFlags,
                    x,
                    y,
                    mni->window_handle,
                    NULL,
                    mni->icm_style,
                    mni->icm_theme
                );
            }

            // ??
            PostMessageW(mni->window_handle, WM_NULL, 0, 0);
        }
    }

    MNI_TRACE(L"\tTrackPopupMenu(): Selected Item: %d", selected_item);
    if (mni->on_context_menu_item_click && selected_item != 0) {
        mni->on_context_menu_item_click(mni, (int)selected_item);
    }
    
    if (mni->on_context_menu_close) {
        mni->on_context_menu_close(mni, (MniBool)(selected_item != 0));
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmBalloonShow(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmBalloonShow()");
    
    if (mni->on_balloon_show) {
        mni->on_balloon_show(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmBalloonHide(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmBalloonHide()");

    if (mni->on_balloon_hide) {
        mni->on_balloon_hide(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmBalloonTimeout(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmBalloonTimeout()");

    if (mni->on_balloon_timeout) {
        mni->on_balloon_timeout(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmBalloonUserClick(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmBalloonUserClick()");

    if (mni->on_balloon_click) {
        mni->on_balloon_click(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmRichPopupOpen(ModernNotifyIcon *mni, int x, int y) {
    MNI_TRACE(L"_MniWmRichPopupOpen(x=%d, y=%d)", x, y);
    
    if (mni->on_rich_popup_open) {
        mni->on_rich_popup_open(mni, x, y);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmRichPopupClose(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniWmRichPopupClose()");
    
    if (mni->on_rich_popup_close) {
        mni->on_rich_popup_close(mni);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmDpiChange(ModernNotifyIcon *mni, int dpi) {
    MNI_TRACE(L"_MniWmDpiChange(dpi=%d)", dpi);
    
    if (mni->dpi != dpi) {
        if (mni->on_dpi_change) {
            mni->on_dpi_change(mni, dpi);
        }

        mni->dpi = dpi;
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmThemeChange(ModernNotifyIcon *mni, MniBool is_hc) {
    MNI_TRACE(L"_MniWmThemeChange()");

    // When theme change this is invoked multiple times, using is_hc is unstable.
    UNREFERENCED_PARAMETER(is_hc);

    if (_IsHighContrastThemeEnabled()) {
        MniThemeInfo hcti = _GetHighContrastThemeInfo();

        // Check if high contrast theme changed.
        if (_IsThemeInfoChanged(mni->system_theme, hcti)) {
            if (mni->on_system_theme_change) {
                mni->on_system_theme_change(mni, hcti);
            }

            mni->system_theme = hcti;
        }

        // Check if high contrast theme changed.
        if (_IsThemeInfoChanged(mni->apps_theme, hcti)) {
            if (mni->on_apps_theme_change) {
                mni->on_apps_theme_change(mni, hcti);
            }

            mni->apps_theme = hcti;
        }
    } else {
        // Check if system theme changed.
        MniThemeInfo sti = _GetSystemThemeInfo();
        if (mni->system_theme.Theme != sti.Theme) {
            if (mni->on_system_theme_change) {
                mni->on_system_theme_change(mni, sti);
            }

            mni->system_theme = sti;
        }

        // Check if apps theme changed.
        MniThemeInfo ati = _GetAppsThemeInfo();
        if (mni->apps_theme.Theme != ati.Theme) {
            if (mni->on_apps_theme_change) {
                mni->on_apps_theme_change(mni, ati);
            }

            mni->apps_theme = ati;
        }
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmTaskbarCreated(ModernNotifyIcon *mni) {
    MNI_TRACE(
        L"_MniWmTaskbarCreated(), is_dpi_event=%d, primary_monitor=%p",
        mni->is_dpi_event,
        mni->primary_monitor
    );

    if (mni->is_dpi_event) {
        mni->is_dpi_event = MNI_FALSE;
    } else {
        // NOTE: Setting these two here can lead to undesirable effects
        //       if this was called from something else than explorer restart.
        //       It's better to force recreating icon when re-showing.
        // Icon no longer exists.
        //mni->icon_visible = MNI_FALSE;
        //mni->icon_created = MNI_FALSE;
        MNI_TRACE(L"\tEXPLORER RESTART");
        if (mni->on_taskbar_created) {
            mni->on_taskbar_created(mni);
        }
    }

    // We check for dpi change here.
    int dpi = _GetDpi(mni->window_handle);
    MNI_TRACE(L"\t_GetDpi(): %d", dpi);
    if (mni->dpi != dpi) {
        SendMessageW(mni->window_handle, WM_DPICHANGED_DELAYED, (WPARAM)dpi, 0);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmUserTimerTimeout(ModernNotifyIcon *mni, UINT id) {
    MNI_TRACE(L"_MniWmUserTimerTimeout(id=%d)", id);

    if (id >= MNI_USER_TIMER_ID) {
        if (mni->on_timer) {
            mni->on_timer(mni, id);
        }
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmInternalTimerTimeout(ModernNotifyIcon *mni, UINT id) {
    MNI_TRACE(L"_MniWmInternalTimerTimeout(id=%d)", id);

    if (id >= MNI_USER_TIMER_ID) {
        return MNI_FALSE;
    }

    if (id == TIMER_PREVENT_DOUBLE_KEYSELECT) {
        KillTimer(mni->window_handle, TIMER_PREVENT_DOUBLE_KEYSELECT);
        mni->prevent_double_key_select = MNI_FALSE;
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmCustomMessage(ModernNotifyIcon *mni, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    MNI_TRACE(L"_MniWmCustomMessage(uMsg=%d, wParam=%lld, lParam=%lld)", uMsg, wParam, lParam);

    if (mni->on_custom_message) {
        mni->on_custom_message(mni, uMsg, wParam, lParam);
    }

    return MNI_TRUE;
}

// ========================================================================== //

static MniBool _MniWmSystemMessage(ModernNotifyIcon *mni, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (mni->on_system_message) {
        return mni->on_system_message(mni, uMsg, wParam, lParam);
    }

    return MNI_FALSE; // we don't want to handle this message
}

#pragma endregion

// ========================================================================== //

#pragma region Window Procedure

static LRESULT _MniDispatch(
    ModernNotifyIcon    *mni,
    HWND                hWnd,
    UINT                uMsg,
    WPARAM              wParam,
    LPARAM              lParam
) {
    MNI_TRACE2(L"_MniDispatch(uMsg=0x%04x, wParam=%lld, lParam=%lld)", uMsg, wParam, lParam);

    // Register message when taskbar is created to get notified when explorer.exe gets restarted.
    if (mni->taskbar_created_message_id == 0) {
        mni->taskbar_created_message_id = RegisterWindowMessageW(MNI_TASKBAR_CREATED_WINDOW_MESSAGE);
    }

    // Handle notify icon messages.
    switch (uMsg) {
        case WM_CREATE:
            if (_MniWmWindowCreate(mni)) {
                return 0;
            }
            break;

        case WM_DESTROY:
            if (_MniWmWindowDestroy(mni)) {
                return 0;
            }
            break;

        case WM_NOTIFYICON:
            switch (LOWORD(lParam)) {
            // Left Click.
            //case NIN_SELECT:
            //    if (KeySelect(GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam)))
            //    {
            //        return 0;
            //    }
            //    break;

            // When you select icon with keyboard and press Space or Enter.
            // Pressing Enter triggers this twice.
            case NIN_KEYSELECT:
                if (_MniWmKeySelect(mni, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam)))
                {
                    return 0;
                }
                break;

            // When you Right Click on icon or Shift+F10 when it's selected with keyboard.
            case WM_CONTEXTMENU:
                if (_MniWmContextMenu(mni, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam))) {
                    return 0;
                }
                break;

            case WM_LBUTTONUP:
                if (_MniWmLmbClick(mni, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam))) {
                    return 0;
                }
                break;

            case WM_LBUTTONDBLCLK:
                if (_MniWmLmbDoubleClick(mni, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam))) {
                    return 0;
                }
                break;

            case WM_MBUTTONUP:
                if (_MniWmMmbClick(mni, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam))) {
                    return 0;
                }
                break;

            case NIN_BALLOONSHOW:
                if (_MniWmBalloonShow(mni)) {                
                    return 0;
                }
                break;
        
            case NIN_BALLOONHIDE:
                if (_MniWmBalloonHide(mni)) {
                    return 0;
                }
                break;

            case NIN_BALLOONTIMEOUT:
                if (_MniWmBalloonTimeout(mni)) {
                    return 0;
                }
                break;

            case NIN_BALLOONUSERCLICK:
                if (_MniWmBalloonUserClick(mni)) {
                    return 0;
                }
                break;

            case NIN_POPUPOPEN:
                if (_MniWmRichPopupOpen(mni, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam))) {
                    return 0;
                }
                break;

            case NIN_POPUPCLOSE:
                if (_MniWmRichPopupClose(mni)) {
                    return 0;
                }
                break;
            }
            break; // WM_NOTIFYICON

        case WM_DPICHANGED:
            // NOTE: Not calling message handler immediately, because
            //       changing dpi in system also trigger TaskbarCreated message.
            //       And if we call handler before TaskbarCreated, changing icons etc.
            //       doesn't work.
            mni->is_dpi_event = MNI_TRUE;
            MNI_TRACE(L"DPI CHANGED");
            return 0;
            //return _MniWmDpiChange(mni, LOWORD(wParam));

        case WM_DISPLAYCHANGE:
            {
                HMONITOR monitor = _GetPrimaryMonitor();
                if (mni->primary_monitor != monitor) {
                    MNI_TRACE(L"PRIMARY MONITOR CHANGE");
                    mni->primary_monitor = monitor;

                    // Move invisible window to primary monitor for accurate dpi value.
                    SetWindowPos(mni->window_handle, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
                }
                return 0;
            }
            break;

        case WM_SETTINGCHANGE:
            {
                if (wParam == SPI_SETHIGHCONTRAST) {
                    if (_MniWmThemeChange(mni, MNI_TRUE)) {
                        return 0;
                    }
                } else {
                    if ((const wchar_t *)lParam != NULL) {
                        const wchar_t name[] = L"ImmersiveColorSet";
                        const int len = ARRAYSIZE(name) - 1; // ARRAYSIZE includes '\0'
                        if (_StringCompareW((const wchar_t *)lParam, name, len) == 0) {
                            if (_MniWmThemeChange(mni, MNI_FALSE)) {
                                return 0;
                            }
                        }
                    }
                }
            }
            break;

        case WM_TIMER:
            if ((UINT)wParam >= MNI_USER_TIMER_ID) {
                if (_MniWmUserTimerTimeout(mni, (UINT)wParam)) {
                    return 0;
                }
            } else {
                if (_MniWmInternalTimerTimeout(mni, (UINT)wParam)) {
                    return 0;
                }
            }
            break;

        case WM_DPICHANGED_DELAYED:
            if (_MniWmDpiChange(mni, (int)wParam)) {
                return 0;
            }
            break;

        case WM_MNI_INIT:
            if (_MniWmInit(mni)) {
                return 0;
            }
            break;

        case WM_MNI_RELEASE:
            if (_MniWmRelease(mni)) {
                return 0;
            }
            break;

        case WM_MNI_SHOW:
            if (_MniWmIconShow(mni)) {
                return 0;
            }
            break;

        case WM_MNI_HIDE:
            if (_MniWmIconHide(mni)) {
                return 0;
            }
            break;

        case WM_MNI_ICON_CHANGE:
            if (_MniWmIconChange(mni, (HICON)wParam)) {
                return 0;
            }
            break;

        case WM_MNI_MENU_CHANGE:
            if (_MniWmMenuChange(mni, (HMENU)wParam)) {
                return 0;
            }
            break;

        case WM_MNI_TIP_CHANGE:
            if (_MniWmTipChange(mni, (const wchar_t *)wParam)) {
                return 0;
            }
            break;

        case WM_MNI_TIP_TYPE_CHANGE:
            if (_MniWmTipTypeChange(mni, (MniTipType)wParam)) {
                return 0;
            }
            break;
    } // switch (uMsg)

    // explorer.exe restart / dpi changed.
    if (uMsg == (UINT)mni->taskbar_created_message_id) {
        if (_MniWmTaskbarCreated(mni)) {
            return 0;
        }
    }

    // User and system messages.
    if (WM_APP <= uMsg && uMsg <= WM_APP_LAST) {
        if (_MniWmCustomMessage(mni, uMsg, wParam, lParam)) {
            return 0;
        }
    } else {
        if (_MniWmSystemMessage(mni, uMsg, wParam, lParam)) {
            return 0;
        }
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

// ========================================================================== //

static LRESULT CALLBACK _MniWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ModernNotifyIcon *mni = NULL;

    if (uMsg == WM_NCCREATE) {
        LPCREATESTRUCT lpCreateStruct = (LPCREATESTRUCT)lParam;

        mni = (ModernNotifyIcon *)lpCreateStruct->lpCreateParams;
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)mni);
        
        mni->window_handle = hWnd;
    } else {
        mni = (ModernNotifyIcon *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    }

    if (mni) {
        return _MniDispatch(mni, hWnd, uMsg, wParam, lParam);
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

#pragma endregion

// ========================================================================== //

#pragma region Internal Methods

static MniError _MniUpdateIcon(ModernNotifyIcon *mni, HICON icon) {
    MNI_TRACE(L"_MniUpdateIcon(icon=%p), icon_created=%d", icon, mni->icon_created);

    if (mni->icon_created) {
        NOTIFYICONDATAW nid = {
            .cbSize = sizeof(nid),
            .hWnd   = mni->window_handle,
            .uID    = 0,
            .uFlags = NIF_ICON,
            .hIcon  = icon,
        };

        if (mni->use_guid) {
            nid.uFlags |= NIF_GUID;
            nid.guidItem = mni->guid;
        }

        if (mni->tip_type == MNI_TIP_TYPE_STANDARD) {
            nid.uFlags |= NIF_SHOWTIP;
        }

        if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) {
            return MNI_ERROR_FAILED_TO_CHANGE_ICON;
        }
    }

    return MNI_OK;
}

// ========================================================================== //

static MniError _MniUpdateMenu(ModernNotifyIcon *mni, HMENU menu) {
    MNI_TRACE(L"_MniUpdateMenu(menu=%p)", menu);

    UNREFERENCED_PARAMETER(menu);
    UNREFERENCED_PARAMETER(mni);

    return MNI_OK;
}

// ========================================================================== //

static MniError _MniUpdateTip(ModernNotifyIcon *mni, const wchar_t *tip) {
    MNI_TRACE(L"_MniUpdateTip(tip=%p)", &tip);

    if (mni->icon_created) {
        NOTIFYICONDATAW nid = {
            .cbSize = sizeof(nid),
            .hWnd   = mni->window_handle,
            .uID    = 0,
            .uFlags = NIF_TIP
        };

        if (mni->use_guid) {
            nid.uFlags |= NIF_GUID;
            nid.guidItem = mni->guid;
        }

        if (mni->tip_type == MNI_TIP_TYPE_STANDARD) {
            nid.uFlags |= NIF_SHOWTIP;
            _StringCopyW(nid.szTip, ARRAYSIZE(nid.szTip), tip);
        }

        if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) {
            return MNI_ERROR_FAILED_TO_CHANGE_TIP;
        }
    }

    return MNI_OK;
}

// ========================================================================== //

static MniError _MniInternalCreateWindow(ModernNotifyIcon *mni, MniInfo info) {
    MNI_TRACE(L"_MniInternalCreateWindow()");

    if (mni->window_handle != NULL) {
        return MNI_WINDOW_ALREADY_CREATED;
    }

    // Get module handle.
    HINSTANCE hInstance = info.module_handle;
    if (hInstance == NULL) {
        hInstance = GetModuleHandleW(NULL);
        if (hInstance == NULL) {
            return MNI_ERROR_INVALID_MODULE_HANDLE;
        }
    }

    // Init window class.
    const wchar_t *class_name = L"MniWndClass";
    if (info.class_name != NULL && info.class_name[0] != L'\0') {
        class_name = info.class_name;
    }

    ATOM atom = 0;
    {
        WNDCLASSEX wcex = {
            .cbSize         = sizeof(wcex),
            .style          = CS_HREDRAW | CS_VREDRAW,
            .lpfnWndProc    = _MniWndProc,
            .cbClsExtra     = 0,
            .cbWndExtra     = 0,
            .hInstance      = hInstance,
            .hIcon          = NULL,
            .hCursor        = NULL,
            .hbrBackground  = (HBRUSH)COLOR_WINDOW,
            .lpszMenuName   = NULL,
            .lpszClassName  = class_name,
            .hIconSm        = NULL
        };

        atom = RegisterClassExW(&wcex);
        if (atom == 0) {
            return MNI_ERROR_FAILED_TO_REGISTER_WNDCLASS;
        }
    }

    // Create invisible window.
    HWND hWnd = NULL;
    {        
        hWnd = CreateWindowExW(
            0,
            class_name,
            info.window_title,
            info.window_style,
            0,                  // we want to put window on main monitor
            0,                  // we want to put window on main monitor
            100,
            40,
            info.parent_window,
            NULL,
            hInstance,
            (LPVOID)mni
        );

        if (hWnd == NULL) {
            UnregisterClassW(class_name, hInstance);
            return MNI_ERROR_FAILED_TO_CREATE_WINDOW;
        }

        UpdateWindow(hWnd);
    }

    mni->window_handle = hWnd;
    mni->module_handle = hInstance;
    mni->class_name = class_name;

    return MNI_OK;
}

// ========================================================================== //

static MniError _MniInternalDestroyWindow(ModernNotifyIcon *mni) {
    MNI_TRACE(L"_MniInternalDestroyWindow()");

    if (mni->window_handle == NULL) {
        return MNI_ERROR_INVALID_WINDOW_HANDLE;
    }

    DestroyWindow(mni->window_handle);
    mni->window_handle = NULL;

    UnregisterClassW(mni->class_name, mni->module_handle);
    
    return MNI_OK;
}

// ========================================================================== //

static MniError _MniInternalCreateNotifyIcon(ModernNotifyIcon *mni) {
    MNI_TRACE(
        L"_MniInternalCreateNotifyIcon(), icon_created=%d, window_handle=%p",
        mni->icon_created,
        mni->window_handle
    );

    if (mni->icon_created) {
        return MNI_ICON_ALREADY_CREATED;
    }

    if (mni->window_handle == NULL) {
        return MNI_ERROR_INVALID_WINDOW_HANDLE;
    }

    NOTIFYICONDATAW nid = {
        .cbSize           = sizeof(nid),
        .hWnd             = mni->window_handle,
        .uID              = 0,
        .uFlags           = NIF_TIP | NIF_ICON | NIF_MESSAGE, // NIF_TIP is required for
                                                              // standard tip and rich popup
        .uCallbackMessage = WM_NOTIFYICON,
        .hIcon            = mni->icon,
        .szTip            = L"",
        .dwState          = 0,
        .dwStateMask      = 0,
        .szInfo           = L"",
        .uVersion         = NOTIFYICON_VERSION_4,
        .szInfoTitle      = L"",
        .dwInfoFlags      = 0,
        .guidItem         = MNI_GUID_NULL,
        .hBalloonIcon     = 0,
    };

    if (mni->use_guid) {
        nid.uFlags |= NIF_GUID;
        nid.guidItem = mni->guid;
    }

    if (mni->tip_type == MNI_TIP_TYPE_STANDARD) {
        nid.uFlags |= NIF_SHOWTIP;
        _StringCopyW(nid.szTip, ARRAYSIZE(nid.szTip), mni->tip);
    }
    
    if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
        return MNI_ERROR_FAILED_TO_ADD_ICON;
    }

    if (!Shell_NotifyIconW(NIM_SETVERSION, &nid)) {
        if (!Shell_NotifyIconW(NIM_DELETE, &nid)) {
            return MNI_ERROR_FAILED_TO_DELETE_ICON;
        }

        return MNI_ERROR_UNSUPPORTED_VERSION;
    }

    mni->icon_created = MNI_TRUE;
    mni->icon_visible = MNI_TRUE;

    return MNI_OK;
}

// ========================================================================== //

static MniError _MniInternalDestroyNotifyIcon(ModernNotifyIcon *mni) {
    MNI_TRACE(
        L"_MniInternalDestroyNotifyIcon(), icon_created=%d, window_handle=%p",
        mni->icon_created,
        mni->window_handle
    );

    if (mni->icon_created == MNI_FALSE) {
        return MNI_ERROR_ICON_NOT_CREATED;
    }

    NOTIFYICONDATAW nid = {
        .cbSize = sizeof(nid),
        .hWnd   = mni->window_handle,
        .uID    = 0
    };

    if (mni->use_guid) {
        nid.uFlags |= NIF_GUID;
        nid.guidItem = mni->guid;
    }

    if (!Shell_NotifyIconW(NIM_DELETE, &nid)) {
        return MNI_ERROR_FAILED_TO_DELETE_ICON;
    }

    mni->icon_created = MNI_FALSE;
    mni->icon_visible = MNI_FALSE;

    return MNI_OK;
}

#pragma endregion

// ========================================================================== //

#pragma region Public API

MniError MniInit(ModernNotifyIcon *mni, MniInfo info) {
    MNI_TRACE(L"MniInit(mni=%p) info={", mni);
    MNI_TRACE(L"\t.module_handle=%p", info.module_handle);
    MNI_TRACE(L"\t.class_name=%p", info.class_name);
    MNI_TRACE(L"\t.guid=%p", &info.guid);
    MNI_TRACE(L"\t.icon=%p", info.icon);
    MNI_TRACE(L"\t.menu=%p", info.menu);
    MNI_TRACE(L"\t.tip=%p", info.tip);
    MNI_TRACE(L"\t.tip_type=%d", info.tip_type);
    MNI_TRACE(L"\t.window_title=%p", info.window_title);
    MNI_TRACE(L"\t.window_style=%x", info.window_style);
    MNI_TRACE(L"\t.parent_window=%p", info.parent_window);
    MNI_TRACE(L"\t.on_window_create=%p", info.on_window_create);
    MNI_TRACE(L"\t.on_window_destroy=%p", info.on_window_destroy);
    MNI_TRACE(L"\t.on_show=%p", info.on_show);
    MNI_TRACE(L"\t.on_hide=%p", info.on_hide);
    MNI_TRACE(L"\t.on_icon_change=%p", info.on_icon_change);
    MNI_TRACE(L"\t.on_menu_change=%p", info.on_menu_change);
    MNI_TRACE(L"\t.on_tip_change=%p", info.on_tip_change);
    MNI_TRACE(L"\t.on_tip_type_change=%p", info.on_tip_type_change);
    MNI_TRACE(L"\t.on_key_select=%p", info.on_key_select);
    MNI_TRACE(L"\t.on_lmb_click=%p", info.on_lmb_click);
    MNI_TRACE(L"\t.on_lmb_double_click=%p", info.on_lmb_double_click);
    MNI_TRACE(L"\t.on_mmb_click=%p", info.on_mmb_click);
    MNI_TRACE(L"\t.on_context_menu_open=%p", info.on_context_menu_open);
    MNI_TRACE(L"\t.on_context_menu_item_click =%p", info.on_context_menu_item_click );
    MNI_TRACE(L"\t.on_context_menu_close=%p", info.on_context_menu_close);
    MNI_TRACE(L"\t.on_balloon_show=%p", info.on_balloon_show);
    MNI_TRACE(L"\t.on_balloon_hide=%p", info.on_balloon_hide);
    MNI_TRACE(L"\t.on_balloon_timeout=%p", info.on_balloon_timeout);
    MNI_TRACE(L"\t.on_balloon_click=%p", info.on_balloon_click);
    MNI_TRACE(L"\t.on_rich_popup_open=%p", info.on_rich_popup_open);
    MNI_TRACE(L"\t.on_rich_popup_close=%p", info.on_rich_popup_close);
    MNI_TRACE(L"\t.on_dpi_change=%p", info.on_dpi_change);
    MNI_TRACE(L"\t.on_system_theme_change=%p", info.on_system_theme_change);
    MNI_TRACE(L"\t.on_apps_theme_change=%p", info.on_apps_theme_change);
    MNI_TRACE(L"\t.on_taskbar_created=%p", info.on_taskbar_created);
    MNI_TRACE(L"\t.on_timer=%p", info.on_timer);
    MNI_TRACE(L"\t.on_custom_message=%p", info.on_custom_message);
    MNI_TRACE(L"\t.on_system_message=%p", info.on_system_message);
    MNI_TRACE(L"}");
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }
    
    memset(mni, 0, sizeof(*mni));

    MniError ret = _MniInternalCreateWindow(mni, info);
    if (MNI_FAILED(ret)) {
        return ret;
    }

    if (!_IsGuidEq(info.guid, MNI_GUID_NULL)) {
        mni->use_guid = MNI_TRUE;
        mni->guid = info.guid;
    }

    mni->icon = info.icon;
    mni->menu = info.menu;
    _StringCopyW(mni->tip, ARRAYSIZE(mni->tip), info.tip);
    mni->tip_type = info.tip_type;

    mni->primary_monitor = _GetPrimaryMonitor();
    if (_IsHighContrastThemeEnabled()) {
        MniThemeInfo hcti = _GetHighContrastThemeInfo();
        mni->system_theme = hcti;
        mni->apps_theme = hcti;
    } else {
        mni->system_theme = _GetSystemThemeInfo();
        mni->apps_theme = _GetAppsThemeInfo();
    }
    mni->icm_style = info.icm_style;
    mni->icm_theme = info.icm_theme;
    mni->dpi = _GetDpi(mni->window_handle);

    mni->menu_position = info.menu_position;
    mni->menu_animation = info.menu_animation;

    mni->reserved1 = NULL;
    mni->reserved2 = NULL;

    mni->user_data1 = info.user_data1;
    mni->user_data2 = info.user_data2;

    mni->on_window_create           = info.on_window_create;
    mni->on_window_destroy          = info.on_window_destroy;
    mni->on_init                    = info.on_init;
    mni->on_release                 = info.on_release;
    mni->on_show                    = info.on_show;
    mni->on_hide                    = info.on_hide;
    mni->on_icon_change             = info.on_icon_change;
    mni->on_menu_change             = info.on_menu_change;
    mni->on_tip_change              = info.on_tip_change;
    mni->on_tip_type_change         = info.on_tip_type_change;
    mni->on_key_select              = info.on_key_select;
    mni->on_lmb_click               = info.on_lmb_click;
    mni->on_lmb_double_click        = info.on_lmb_double_click;
    mni->on_mmb_click               = info.on_mmb_click;
    mni->on_context_menu_open       = info.on_context_menu_open;
    mni->on_context_menu_item_click = info.on_context_menu_item_click;
    mni->on_context_menu_close      = info.on_context_menu_close;
    mni->on_balloon_show            = info.on_balloon_show;
    mni->on_balloon_hide            = info.on_balloon_hide;
    mni->on_balloon_timeout         = info.on_balloon_timeout;
    mni->on_balloon_click           = info.on_balloon_click;
    mni->on_rich_popup_open         = info.on_rich_popup_open;
    mni->on_rich_popup_close        = info.on_rich_popup_close;
    mni->on_dpi_change              = info.on_dpi_change;
    mni->on_system_theme_change     = info.on_system_theme_change;
    mni->on_apps_theme_change       = info.on_apps_theme_change;
    mni->on_taskbar_created         = info.on_taskbar_created;
    mni->on_timer                   = info.on_timer;
    mni->on_custom_message          = info.on_custom_message;
    mni->on_system_message          = info.on_system_message;

    if (mni->window_handle) {
        SendMessageW(mni->window_handle, WM_MNI_INIT, 0, 0);
    }

    return MNI_OK;
}

// ========================================================================== //

MniError MniRelease(ModernNotifyIcon *mni, MniBool destroy_icon, MniBool destroy_menu) {
    MNI_TRACE(L"MniRelease(mni=%p, destroy_icon=%d, destroy_menu=%d)", mni, destroy_icon, destroy_menu);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (mni->window_handle) {
        SendMessageW(mni->window_handle, WM_MNI_RELEASE, 0, 0);
    }

    _MniInternalDestroyNotifyIcon(mni);
    _MniInternalDestroyWindow(mni);

    if (destroy_icon && mni->icon) {
        DestroyIcon(mni->icon);
    }

    if (destroy_menu && mni->menu) {
        DestroyMenu(mni->menu);
    }

    memset(mni, 0, sizeof(*mni));

    return MNI_OK;
}

// ========================================================================== //

MniError MniShow(ModernNotifyIcon *mni, MniBool recreate) {
    MNI_TRACE(L"MniShow(mni=%p, recreate=%d)", mni, recreate);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    MNI_TRACE(
        L"\ticon_visible=%d, icon_created=%d, window_handle=%p",
        mni->icon_visible,
        mni->icon_created,
        mni->window_handle
    );

    if (mni->icon_visible && !recreate) {
        return MNI_ICON_ALREADY_SHOWN;
    }

    if (!mni->window_handle) {
        MniError result = _MniInternalCreateWindow(mni, (MniInfo){0});
        if (MNI_FAILED(result)) {
            return result;
        }
    }

    if (recreate) {
        // If this fails, probable cause was the fact that icon doesn't exists anymore.
        MniError result = _MniInternalDestroyNotifyIcon(mni);
        if (FAILED(result)) {
            mni->icon_visible = MNI_FALSE;
            mni->icon_created = MNI_FALSE;
        }
    }

    if (!mni->icon_created) {
        MniError result = _MniInternalCreateNotifyIcon(mni);
        if (MNI_FAILED(result)) {
            return result;
        }
    } else {
        NOTIFYICONDATAW nid = {
            .cbSize      = sizeof(nid),
            .hWnd        = mni->window_handle,
            .uID         = 0,
            .uFlags      = NIF_STATE,
            .dwState     = 0,
            .dwStateMask = NIS_HIDDEN
        };

        if (mni->use_guid) {
            nid.uFlags |= NIF_GUID;
            nid.guidItem = mni->guid;
        }

        if (mni->tip_type == MNI_TIP_TYPE_STANDARD) {
            nid.uFlags |= NIF_SHOWTIP;
        }

        if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) {
            return MNI_ERROR_FAILED_TO_SHOW_ICON;
        }
    }
    
    mni->icon_visible = MNI_TRUE;
    
    if (mni->window_handle) {
        SendMessageW(mni->window_handle, WM_MNI_SHOW, 0, 0);
    }

    return MNI_OK;
}

// ========================================================================== //

MniError MniHide(ModernNotifyIcon *mni) {
    MNI_TRACE(L"MniHide(mni=%p)", mni);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    MNI_TRACE(
        L"\ticon_visible=%d, icon_created=%d, window_handle=%p",
        mni->icon_visible,
        mni->icon_created,
        mni->window_handle
    );

    if (!mni->icon_visible) {
        return MNI_ICON_ALREADY_HIDDEN;
    }

    if (!mni->icon_created) {
        return MNI_ERROR_ICON_NOT_CREATED;
    }

    if (!mni->window_handle) {
        return MNI_ERROR_INVALID_WINDOW_HANDLE;
    }

    NOTIFYICONDATAW nid = {
        .cbSize      = sizeof(nid),
        .hWnd        = mni->window_handle,
        .uID         = 0,
        .uFlags      = NIF_STATE,
        .dwState     = NIS_HIDDEN,
        .dwStateMask = NIS_HIDDEN
    };

    if (mni->use_guid) {
        nid.uFlags |= NIF_GUID;
        nid.guidItem = mni->guid;
    }

    if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) {
        return MNI_ERROR_FAILED_TO_HIDE_ICON;
    }

    mni->icon_visible = MNI_FALSE;

    SendMessageW(mni->window_handle, WM_MNI_HIDE, 0, 0);

    return MNI_OK;
}

// ========================================================================== //

MniError MniSetIcon(ModernNotifyIcon *mni, HICON icon, MniBool destroy_current) {
    MNI_TRACE(L"MniSetIcon(mni=%p, icon=%p, destroy_current=%d)", mni, icon, destroy_current);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    // Only update if there is change.
    if (icon != mni->icon) {
        MniError result = _MniUpdateIcon(mni, icon);
        if (MNI_FAILED(result)) {
            return result;
        }
    
        SendMessageW(mni->window_handle, WM_MNI_ICON_CHANGE, (WPARAM)icon, 0);

        if (mni->icon && destroy_current) {
            DestroyIcon(mni->icon);
        }

        mni->icon = icon;
    }

    return MNI_OK;
}

// ========================================================================== //

MniError MniSetMenu(ModernNotifyIcon *mni, HMENU menu, MniBool destroy_current) {
    MNI_TRACE(L"MniSetMenu(mni=%p, menu=%p, destroy_current=%d)", mni, menu, destroy_current);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    // Only update if there is change.
    if (menu != mni->menu) {
        MniError result = _MniUpdateMenu(mni, menu); 
        if (MNI_FAILED(result)) {
            return result;
        }

        SendMessageW(mni->window_handle, WM_MNI_MENU_CHANGE, (WPARAM)menu, 0);

        if (mni->menu && destroy_current) {
            DestroyMenu(mni->menu);
        }

        mni->menu = menu;
    }

    return MNI_OK;
}

// ========================================================================== //

MniError MniSetTip(ModernNotifyIcon *mni, const wchar_t *tip) {
    MNI_TRACE(L"MniSetTip(mni=%p, tip=%p)", mni, tip);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    // Only update if there is change.
    if (_StringCompareW(mni->tip, tip, ARRAYSIZE(mni->tip)) != 0) {
        MniError result = _MniUpdateTip(mni, tip); 
        if (MNI_FAILED(result)) {
            return result;
        }

        SendMessageW(mni->window_handle, WM_MNI_TIP_CHANGE, (WPARAM)tip, 0);

        _StringCopyW(mni->tip, ARRAYSIZE(mni->tip), tip);
    }

    return MNI_OK;
}

// ========================================================================== //

MniError MniSetTipType(ModernNotifyIcon *mni, MniTipType mtt) {
    MNI_TRACE(L"MniSetTipType(mni=%p, mtt=%p)", mni, &mtt);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    // Only update if there is change.
    if (mtt != mni->tip_type)
    {
        const wchar_t *tip = L"";
        if (mni->tip_type == MNI_TIP_TYPE_STANDARD) {
            tip = mni->tip;
        }

        MniError result = _MniUpdateTip(mni, tip);
        if (MNI_FAILED(result)) {
            return result;
        }

        SendMessageW(mni->window_handle, WM_MNI_TIP_TYPE_CHANGE, (WPARAM)mtt, 0);

        mni->tip_type = mtt;
    }
    
    return MNI_OK;
}

// ========================================================================== //

MniError MniGetIcon(ModernNotifyIcon *mni, HICON *icon) {
    MNI_TRACE(L"MniGetIcon(mni=%p, icon=%p)", mni, icon);
    MNI_ASSERT(mni && "mni ptr is null");
    
    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!icon) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *icon = mni->icon;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetMenu(ModernNotifyIcon *mni, HMENU *menu) {
    MNI_TRACE(L"MniGetMenu(mni=%p, menu=%p)", mni, menu);
    MNI_ASSERT(mni && "mni ptr is null");
    
    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!menu) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *menu = mni->menu;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetTip(ModernNotifyIcon *mni, wchar_t *buffer, int *len) {
    MNI_TRACE(L"MniGetTip(mni=%p, buffer=%p, len=%p)", mni, buffer, len);
    MNI_ASSERT(mni && "mni ptr is null");
    
    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!buffer && !len) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    // cch should include '\0'.
    int cch = _StringLengthMaxW(mni->tip, ARRAYSIZE(mni->tip));
    if (cch == ARRAYSIZE(mni->tip)) {
        mni->tip[cch - 1] = L'\0';
    } else {
        cch += 1;
    }

    // Return required length (in characters including '\0').
    if (!buffer) {
        *len = cch;
        return MNI_OK;
    }

    // Check if buffer can fit the string.
    if (cch > *len) {
        return MNI_ERROR_INSUFFICIENT_BUFFER;
    }

    // Copy tip to destination buffer.
    _StringCopyW(buffer, cch, mni->tip);
    *len = cch;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetTipType(ModernNotifyIcon *mni, MniTipType *mtt) {
    MNI_TRACE(L"MniGetTip(mni=%p, mtt=%p)", mni, mtt);
    MNI_ASSERT(mni && "mni ptr is null");
    
    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!mtt) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *mtt = mni->tip_type;

    return MNI_OK;
}

// ========================================================================== //

MniError MniIsNotifyIconCreated(ModernNotifyIcon *mni, MniBool *is_created) {
    MNI_TRACE(L"MniIsNotifyIconCreated(mni=%p, is_create=%p)", mni, is_created);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!is_created) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *is_created = mni->icon_created;

    return MNI_OK;
}

// ========================================================================== //

MniError MniIsNotifyIconVisible(ModernNotifyIcon *mni, MniBool *is_visible) {
    MNI_TRACE(L"MniIsNotifyIconVisible(mni=%p, is_visible=%p)", mni, is_visible);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!is_visible) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *is_visible = mni->icon_visible;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetWindowHandle(ModernNotifyIcon *mni, HWND *window_handle) {
    MNI_TRACE(L"MniGetWindowHandle(mni=%p, window_handle=%p)", mni, window_handle);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!window_handle) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *window_handle = mni->window_handle;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetModuleHandle(ModernNotifyIcon *mni, HINSTANCE *module_handle) {
    MNI_TRACE(L"MniGetModuleHandle(mni=%p, module_handle=%p)", mni, module_handle);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!module_handle) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *module_handle = mni->module_handle;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetDpi(ModernNotifyIcon *mni, int *dpi) {
    MNI_TRACE(L"MniGetDpi(mni=%p, dpi=%p)", mni, dpi);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!dpi) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *dpi = mni->dpi;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetSystemThemeInfo(ModernNotifyIcon *mni, MniThemeInfo *system_theme) {
    MNI_TRACE(L"MniGetSystemThemeInfo(mni=%p, system_theme=%p)", mni, system_theme);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!system_theme) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *system_theme = mni->system_theme;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetAppsThemeInfo(ModernNotifyIcon *mni, MniThemeInfo *apps_theme) {
    MNI_TRACE(L"MniGetAppsThemeInfo(mni=%p, apps_theme=%p)", mni, apps_theme);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!apps_theme) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *apps_theme = mni->apps_theme;

    return MNI_OK;
}

// ========================================================================== //

MNI_API MniError MniSetIcmStyle(ModernNotifyIcon *mni, IcmStyle icm_style) {
    MNI_TRACE(L"MniSetIcmTheme(mni=%p, icm_style=%d)", mni, icm_style);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    mni->icm_style = icm_style;

    return MNI_OK;
}

// ========================================================================== //

MNI_API MniError MniSetIcmTheme(ModernNotifyIcon *mni, IcmTheme icm_theme) {
    MNI_TRACE(L"MniSetIcmTheme(mni=%p, icm_theme=%d)", mni, icm_theme);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    mni->icm_theme = icm_theme;

    return MNI_OK;
}

// ========================================================================== //

MNI_API MniError MniGetIcmStyle(ModernNotifyIcon *mni, IcmStyle *icm_style) {
    MNI_TRACE(L"MniGetIcmStyle(mni=%p, icm_style=%p)", mni, icm_style);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!icm_style) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *icm_style = mni->icm_style;

    return MNI_OK;
}

// ========================================================================== //

MNI_API MniError MniGetIcmTheme(ModernNotifyIcon *mni, IcmTheme *icm_theme) {
    MNI_TRACE(L"MniGetIcmTheme(mni=%p, icm_theme=%p)", mni, icm_theme);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!icm_theme) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *icm_theme = mni->icm_theme;

    return MNI_OK;
}

// ========================================================================== //

MniError MniSetUserData1(ModernNotifyIcon *mni, void *data) {
    MNI_TRACE(L"MniSetUserData1(mni=%p, data=%p)", mni, data);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    mni->user_data1 = data;

    return MNI_OK;
}

// ========================================================================== //

MniError MniSetUserData2(ModernNotifyIcon *mni, void *data) {
    MNI_TRACE(L"MniSetUserData2(mni=%p, data=%p)", mni, data);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    mni->user_data2 = data;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetUserData1(ModernNotifyIcon *mni, void **data) {
    MNI_TRACE(L"MniGetUserData1(mni=%p, data=%p)", mni, data);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!data) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *data = mni->user_data1;

    return MNI_OK;
}

// ========================================================================== //

MniError MniGetUserData2(ModernNotifyIcon *mni, void **data) {
    MNI_TRACE(L"MniGetUserData2(mni=%p, data=%p)", mni, data);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!data) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    *data = mni->user_data2;

    return MNI_OK;
}

// ========================================================================== //

MniError MniSendBalloonNotification(
    ModernNotifyIcon        *mni,
    const wchar_t           *title,
    const wchar_t           *text,
    MniBalloonIconType      icon_type,
    HICON                   icon,
    MniBalloonFlags         flags
) {
    MNI_TRACE(
        L"MniSendBalloonNotification(mni=%p, title=%p, text=%p, icon_type=%d, icon=%p, flags=%x)",
        mni,
        title,
        text,
        (UINT)icon_type,
        icon,
        (UINT)flags
    );
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    MNI_TRACE(
        L"\ticon_visible=%d, icon_created=%d, window_handle=%p",
        mni->icon_visible,
        mni->icon_created,
        mni->window_handle
    );

    if (!mni->icon_created) {
        return MNI_ERROR_ICON_NOT_CREATED;
    }

    NOTIFYICONDATAW nid = {
        .cbSize       = sizeof(nid),
        .hWnd         = mni->window_handle,
        .uID          = 0,
        .uFlags       = NIF_INFO,
        .dwInfoFlags  = NIIF_NONE,
        .hBalloonIcon = NULL
    };

    if (mni->use_guid) {
        nid.uFlags |= NIF_GUID;
        nid.guidItem = mni->guid;
    }

    if (mni->tip_type == MNI_TIP_TYPE_STANDARD) {
        nid.uFlags |= NIF_SHOWTIP;
    }

    if ((flags & MNI_BALLOON_FLAGS_REALTIME) == MNI_BALLOON_FLAGS_REALTIME) {
        nid.uFlags |= NIF_REALTIME;
    }

    switch (icon_type)
    {
    case MNI_BALLOON_ICON_TYPE_NONE:
        break;
    case MNI_BALLOON_ICON_TYPE_SYSTEM_INFO:
        nid.dwInfoFlags = NIIF_INFO | NIIF_LARGE_ICON;
        break;
    case MNI_BALLOON_ICON_TYPE_SYSTEM_WARNING:
        nid.dwInfoFlags = NIIF_WARNING | NIIF_LARGE_ICON;
        break;
    case MNI_BALLOON_ICON_TYPE_SYSTEM_ERROR:
        nid.dwInfoFlags = NIIF_ERROR | NIIF_LARGE_ICON;
        break;
    case MNI_BALLOON_ICON_TYPE_CUSTOM:
        nid.dwInfoFlags  = NIIF_USER | NIIF_LARGE_ICON;
        nid.hBalloonIcon = icon;
        break;
    }

    if ((flags & MNI_BALLOON_FLAGS_PLAY_SOUND) != MNI_BALLOON_FLAGS_PLAY_SOUND) {
        nid.dwInfoFlags |= NIIF_NOSOUND;
    }

    if ((flags & MNI_BALLOON_FLAGS_RESPECT_QUIET_TIME) == MNI_BALLOON_FLAGS_RESPECT_QUIET_TIME) {
        nid.dwInfoFlags |= NIIF_RESPECT_QUIET_TIME;
    }
    
    _StringCopyW(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), title);
    _StringCopyW(nid.szInfo, ARRAYSIZE(nid.szInfo), text);

    if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) {
        return MNI_ERROR_FAILED_TO_SHOW_BALLOON;
    }

    return MNI_OK;
}

// ========================================================================== //

MniError MniRemoveBalloonNotification(ModernNotifyIcon *mni) {
    MNI_TRACE(L"MniRemoveBalloonNotification(mni=%p)", mni);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    MNI_TRACE(L"\ticon_created=%d, window_handle=%p", mni->icon_created, mni->window_handle);

    if (!mni->icon_created) {
        return MNI_ERROR_ICON_NOT_CREATED;
    }

    NOTIFYICONDATAW nid = {
        .cbSize       = sizeof(nid),
        .hWnd         = mni->window_handle,
        .uID          = 0,
        .uFlags       = NIF_INFO,
        .szInfo       = L"",
        .szInfoTitle  = L"",
        .dwInfoFlags  = NIIF_NONE,
        .hBalloonIcon = NULL,
    };

    if (!Shell_NotifyIconW(NIM_MODIFY, &nid)) {
        return MNI_ERROR_FAILED_TO_REMOVE_BALLOON;
    }
    
    return MNI_OK;
}

// ========================================================================== //

MniError MniStartTimer(ModernNotifyIcon *mni, UINT timer_id, UINT interval) {
    MNI_TRACE(L"MniStartTimer(mni=%p, timer_id=%d, interval=%d)", mni, timer_id, interval);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!mni->window_handle) {
        return MNI_ERROR_INVALID_WINDOW_HANDLE;
    }

    if (timer_id < MNI_USER_TIMER_ID) {
        return MNI_ERROR_INVALID_TIMER_ID;
    }
    
    UINT_PTR ret = SetTimer(mni->window_handle, timer_id, interval, NULL);
    if (!ret) {
        return MNI_ERROR_FAILED_TO_START_TIMER;
    }
    
    return MNI_OK;
}

// ========================================================================== //

MniError MniStopTimer(ModernNotifyIcon *mni, UINT timer_id) {
    MNI_TRACE(L"MniStopTimer(mni=%p, timer_id=%d)", mni, timer_id);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!mni->window_handle) {
        return MNI_ERROR_INVALID_WINDOW_HANDLE;
    }

    if (timer_id < MNI_USER_TIMER_ID) {
        return MNI_ERROR_INVALID_TIMER_ID;
    }
    
    MniBool ret = KillTimer(mni->window_handle, timer_id);
    if (!ret) {
        return MNI_ERROR_FAILED_TO_STOP_TIMER;
    }
    
    return MNI_OK;
}

// ========================================================================== //

MniError MniSendCustomMessage(ModernNotifyIcon *mni, UINT msg, WPARAM wParam, LPARAM lParam) {
    MNI_TRACE(L"MniSendCustomMessage(mni=%p, msg=%d, wParam=%p, lParam=%p)", mni, msg, wParam, lParam);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!mni->window_handle) {
        return MNI_ERROR_INVALID_WINDOW_HANDLE;
    }
    
    if (WM_APP <= msg && msg <= WM_APP_LAST)
    {
        // This returns immediately if calling thread is different than window creation thread. 
        BOOL ret = SendNotifyMessageW(mni->window_handle, msg, wParam, lParam);
        if (ret == FALSE) {
            return MNI_ERROR_FAILED_TO_SEND_MESSAGE;
        }
    }
    
    return MNI_OK;
}

// ========================================================================== //

MniError MniPostCustomMessage(ModernNotifyIcon *mni, UINT msg, WPARAM wParam, LPARAM lParam) {
    MNI_TRACE(L"MniPostCustomMessage(mni=%p, msg=%d, wParam=%p, lParam=%p)", mni, msg, wParam, lParam);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!mni->window_handle) {
        return MNI_ERROR_INVALID_WINDOW_HANDLE;
    }
    
    if (WM_APP <= msg && msg <= WM_APP_LAST)
    {
        BOOL ret = PostMessageW(mni->window_handle, msg, wParam, lParam);
        if (ret == FALSE) {
            return MNI_ERROR_FAILED_TO_SEND_MESSAGE;
        }
    }
    
    return MNI_OK;
}

// ========================================================================== //

const wchar_t *MniErrorToString(MniError error) {
    switch (error) {
    case MNI_OK:                                    return L"MNI_OK";
    
    case MNI_WINDOW_ALREADY_CREATED:                return L"MNI_WINDOW_ALREADY_CREATED";
    case MNI_ICON_ALREADY_CREATED:                  return L"MNI_ICON_ALREADY_CREATED";
    case MNI_ICON_ALREADY_SHOWN:                    return L"MNI_ICON_ALREADY_SHOWN";
    case MNI_ICON_ALREADY_HIDDEN:                   return L"MNI_ICON_ALREADY_HIDDEN";

    case MNI_ERROR_MNI_PTR_IS_NULL:                 return L"MNI_ERROR_MNI_PTR_IS_NULL";
    case MNI_ERROR_UNSUPPORTED_VERSION:             return L"MNI_ERROR_UNSUPPORTED_VERSION";
    case MNI_ERROR_FAILED_TO_ADD_ICON:              return L"MNI_ERROR_FAILED_TO_ADD_ICON";
    case MNI_ERROR_FAILED_TO_DELETE_ICON:           return L"MNI_ERROR_FAILED_TO_DELETE_ICON";
    case MNI_ERROR_FAILED_TO_SHOW_ICON:             return L"MNI_ERROR_FAILED_TO_SHOW_ICON";
    case MNI_ERROR_FAILED_TO_HIDE_ICON:             return L"MNI_ERROR_FAILED_TO_HIDE_ICON";
    case MNI_ERROR_FAILED_TO_CHANGE_ICON:           return L"MNI_ERROR_FAILED_TO_CHANGE_ICON";
    case MNI_ERROR_FAILED_TO_CHANGE_TIP:            return L"MNI_ERROR_FAILED_TO_CHANGE_TIP";
    case MNI_ERROR_FAILED_TO_COPY_TIP:              return L"MNI_ERROR_FAILED_TO_COPY_TIP";
    case MNI_ERROR_FAILED_TO_COPY_BALLOON_TITLE:    return L"MNI_ERROR_FAILED_TO_COPY_BALLOON_TITLE";
    case MNI_ERROR_FAILED_TO_COPY_BALLOON_TEXT:     return L"MNI_ERROR_FAILED_TO_COPY_BALLOON_TEXT";
    case MNI_ERROR_FAILED_TO_SHOW_BALLOON:          return L"MNI_ERROR_FAILED_TO_SHOW_BALLOON";
    case MNI_ERROR_FAILED_TO_REMOVE_BALLOON:        return L"MNI_ERROR_FAILED_TO_REMOVE_BALLOON";
    case MNI_ERROR_FAILED_TO_REGISTER_WNDCLASS:     return L"MNI_ERROR_FAILED_TO_REGISTER_WNDCLASS";
    case MNI_ERROR_FAILED_TO_CREATE_WINDOW:         return L"MNI_ERROR_FAILED_TO_CREATE_WINDOW";
    case MNI_ERROR_FAILED_TO_START_TIMER:           return L"MNI_ERROR_FAILED_TO_START_TIMER";
    case MNI_ERROR_FAILED_TO_STOP_TIMER:            return L"MNI_ERROR_FAILED_TO_STOP_TIMER";
    case MNI_ERROR_INVALID_MODULE_HANDLE:           return L"MNI_ERROR_INVALID_MODULE_HANDLE";
    case MNI_ERROR_INVALID_WINDOW_HANDLE:           return L"MNI_ERROR_INVALID_WINDOW_HANDLE";
    case MNI_ERROR_INVALID_ICON:                    return L"MNI_ERROR_INVALID_ICON";
    case MNI_ERROR_INVALID_MENU:                    return L"MNI_ERROR_INVALID_MENU";
    case MNI_ERROR_INVALID_TIMER_ID:                return L"MNI_ERROR_INVALID_TIMER_ID";
    case MNI_ERROR_INVALID_ARGUMENT:                return L"MNI_ERROR_INVALID_ARGUMENT";
    case MNI_ERROR_ICON_NOT_CREATED:                return L"MNI_ERROR_ICON_NOT_CREATED";
    case MNI_ERROR_INSUFFICIENT_BUFFER:             return L"MNI_ERROR_INSUFFICIENT_BUFFER";
    case MNI_ERROR_FAILED_TO_CONVERT_TIP:           return L"MNI_ERROR_FAILED_TO_CONVERT_TIP";
    case MNI_ERROR_FAILED_TO_CONVERT_TITLE:         return L"MNI_ERROR_FAILED_TO_CONVERT_TITLE";
    case MNI_ERROR_FAILED_TO_CONVERT_TEXT:          return L"MNI_ERROR_FAILED_TO_CONVERT_TEXT";
    case MNI_ERROR_FAILED_TO_SEND_MESSAGE:          return L"MNI_ERROR_FAILED_TO_SEND_MESSAGE";
    case MNI_ERROR_FAILED_TO_POST_MESSAGE:          return L"MNI_ERROR_FAILED_TO_POST_MESSAGE";
    }

    return L"MNI_UNKNOWN_ERROR_CODE";
}

// ========================================================================== //

int MniRunMessageLoop(void) {
    MNI_TRACE(L"MniRunMessageLoop()");
    MSG msg;

    while (1) {
        MniBool ret = GetMessage(&msg, NULL, 0, 0);
        if (ret == -1) {
            return -1;
        }

        if (ret == MNI_FALSE) {
            break;
        } 
        
        TranslateMessage(&msg); 
        DispatchMessageW(&msg);
    }

    return (int)(msg.wParam);
}

// ========================================================================== //

void MniQuit(void) {
    MNI_TRACE(L"MniQuit()");
    PostQuitMessage(0);
}

// ========================================================================== //

MniError MniSetTipUTF8(ModernNotifyIcon *mni, const char *tip) {
    MNI_TRACE(L"MniSetTipUTF8(mni=%p, tip=%p)", mni, tip);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    // Convert tip.
    wchar_t tip_buffer[128];
    if (!tip) {
        tip_buffer[0] = L'\0';
    } else {
        MniBool ret = _UTF8ToUTF16(tip, tip_buffer, ARRAYSIZE(tip_buffer));
        if (ret == MNI_FALSE) {
            return MNI_ERROR_FAILED_TO_CONVERT_TIP;
        }
    }

    return MniSetTip(mni, tip_buffer);
}

// ========================================================================== //

MniError MniGetTipUTF8(ModernNotifyIcon *mni, char *buffer, int *len) {
    MNI_TRACE(L"MniGetTipUTF8(mni=%p, buffer=%p, len=%p)", mni, buffer, len);
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    if (!buffer && !len) {
        return MNI_ERROR_INVALID_ARGUMENT;
    }

    // cch should include '\0'.
    int cch = _StringLengthMaxW(mni->tip, ARRAYSIZE(mni->tip));
    if (cch == ARRAYSIZE(mni->tip)) {
        mni->tip[cch - 1] = L'\0';
    } else {
        cch += 1;
    }

    int buffer_len = *len;

    // Get required length of utf8 string.
    // utf8_len will include null character.
    int utf8_len = WideCharToMultiByte(CP_UTF8, 0, mni->tip, cch, NULL, 0, NULL, NULL);
    if (utf8_len == 0) {
        return MNI_ERROR_FAILED_TO_CONVERT_TIP;
    }

    // Return required length for utf8 string.
    if (!buffer) {
        *len = utf8_len;
        return MNI_OK;
    }

    // Check if buffer can fit the string.
    if (utf8_len > buffer_len) {
        return MNI_ERROR_INSUFFICIENT_BUFFER;
    }
    
    // Convert.
    int ret = WideCharToMultiByte(CP_UTF8, 0, mni->tip, cch, buffer, buffer_len, NULL, NULL);
    if (ret == 0) {
        return MNI_ERROR_FAILED_TO_CONVERT_TIP;
    }

    *len = ret;

    return MNI_OK;
}

// ========================================================================== //

MniError MniSendBalloonNotificationUTF8(
    ModernNotifyIcon        *mni,
    const char              *title,
    const char              *text,
    MniBalloonIconType      icon_type,
    HICON                   icon,
    MniBalloonFlags         flags
) {
    MNI_TRACE(
        L"MniSendBalloonNotificationUTF8(mni=%p, title=%p, text=%p, icon_type=%d, icon=%p, flags=%x)",
        mni,
        title,
        text,
        (UINT)icon_type,
        icon,
        (UINT)flags
    );
    MNI_ASSERT(mni && "mni ptr is null");

    if (!mni) {
        return MNI_ERROR_MNI_PTR_IS_NULL;
    }

    // Convert title.
    wchar_t title_buffer[64];
    if (!title) {
        title_buffer[0] = L'\0';
    } else {
        MniBool ret = _UTF8ToUTF16(title, title_buffer, ARRAYSIZE(title_buffer));
        if (ret == MNI_FALSE) {
            return MNI_ERROR_FAILED_TO_CONVERT_TITLE;
        }
    }

    // Convert text.
    wchar_t text_buffer[256];
    if (!text) {
        text_buffer[0] = L'\0';
    } else {
        MniBool ret = _UTF8ToUTF16(title, text_buffer, ARRAYSIZE(text_buffer));
        if (ret == MNI_FALSE) {
            return MNI_ERROR_FAILED_TO_CONVERT_TEXT;
        }
    }

    return MniSendBalloonNotification(mni, title_buffer, text_buffer, icon_type, icon, flags);
}

// ========================================================================== //

const char *MniErrorToStringUTF8(MniError error) {
    switch (error) {
    case MNI_OK:                                    return "MNI_OK";
    
    case MNI_WINDOW_ALREADY_CREATED:                return "MNI_WINDOW_ALREADY_CREATED";
    case MNI_ICON_ALREADY_CREATED:                  return "MNI_ICON_ALREADY_CREATED";
    case MNI_ICON_ALREADY_SHOWN:                    return "MNI_ICON_ALREADY_SHOWN";
    case MNI_ICON_ALREADY_HIDDEN:                   return "MNI_ICON_ALREADY_HIDDEN";

    case MNI_ERROR_MNI_PTR_IS_NULL:                 return "MNI_ERROR_MNI_PTR_IS_NULL";
    case MNI_ERROR_UNSUPPORTED_VERSION:             return "MNI_ERROR_UNSUPPORTED_VERSION";
    case MNI_ERROR_FAILED_TO_ADD_ICON:              return "MNI_ERROR_FAILED_TO_ADD_ICON";
    case MNI_ERROR_FAILED_TO_DELETE_ICON:           return "MNI_ERROR_FAILED_TO_DELETE_ICON";
    case MNI_ERROR_FAILED_TO_SHOW_ICON:             return "MNI_ERROR_FAILED_TO_SHOW_ICON";
    case MNI_ERROR_FAILED_TO_HIDE_ICON:             return "MNI_ERROR_FAILED_TO_HIDE_ICON";
    case MNI_ERROR_FAILED_TO_CHANGE_ICON:           return "MNI_ERROR_FAILED_TO_CHANGE_ICON";
    case MNI_ERROR_FAILED_TO_CHANGE_TIP:            return "MNI_ERROR_FAILED_TO_CHANGE_TIP";
    case MNI_ERROR_FAILED_TO_COPY_TIP:              return "MNI_ERROR_FAILED_TO_COPY_TIP";
    case MNI_ERROR_FAILED_TO_COPY_BALLOON_TITLE:    return "MNI_ERROR_FAILED_TO_COPY_BALLOON_TITLE";
    case MNI_ERROR_FAILED_TO_COPY_BALLOON_TEXT:     return "MNI_ERROR_FAILED_TO_COPY_BALLOON_TEXT";
    case MNI_ERROR_FAILED_TO_SHOW_BALLOON:          return "MNI_ERROR_FAILED_TO_SHOW_BALLOON";
    case MNI_ERROR_FAILED_TO_REMOVE_BALLOON:        return "MNI_ERROR_FAILED_TO_REMOVE_BALLOON";
    case MNI_ERROR_FAILED_TO_REGISTER_WNDCLASS:     return "MNI_ERROR_FAILED_TO_REGISTER_WNDCLASS";
    case MNI_ERROR_FAILED_TO_CREATE_WINDOW:         return "MNI_ERROR_FAILED_TO_CREATE_WINDOW";
    case MNI_ERROR_FAILED_TO_START_TIMER:           return "MNI_ERROR_FAILED_TO_START_TIMER";
    case MNI_ERROR_FAILED_TO_STOP_TIMER:            return "MNI_ERROR_FAILED_TO_STOP_TIMER";
    case MNI_ERROR_INVALID_MODULE_HANDLE:           return "MNI_ERROR_INVALID_MODULE_HANDLE";
    case MNI_ERROR_INVALID_WINDOW_HANDLE:           return "MNI_ERROR_INVALID_WINDOW_HANDLE";
    case MNI_ERROR_INVALID_ICON:                    return "MNI_ERROR_INVALID_ICON";
    case MNI_ERROR_INVALID_MENU:                    return "MNI_ERROR_INVALID_MENU";
    case MNI_ERROR_INVALID_TIMER_ID:                return "MNI_ERROR_INVALID_TIMER_ID";
    case MNI_ERROR_INVALID_ARGUMENT:                return "MNI_ERROR_INVALID_ARGUMENT";
    case MNI_ERROR_ICON_NOT_CREATED:                return "MNI_ERROR_ICON_NOT_CREATED";
    case MNI_ERROR_INSUFFICIENT_BUFFER:             return "MNI_ERROR_INSUFFICIENT_BUFFER";
    case MNI_ERROR_FAILED_TO_CONVERT_TIP:           return "MNI_ERROR_FAILED_TO_CONVERT_TIP";
    case MNI_ERROR_FAILED_TO_CONVERT_TITLE:         return "MNI_ERROR_FAILED_TO_CONVERT_TITLE";
    case MNI_ERROR_FAILED_TO_CONVERT_TEXT:          return "MNI_ERROR_FAILED_TO_CONVERT_TEXT";
    case MNI_ERROR_FAILED_TO_SEND_MESSAGE:          return "MNI_ERROR_FAILED_TO_SEND_MESSAGE";
    case MNI_ERROR_FAILED_TO_POST_MESSAGE:          return "MNI_ERROR_FAILED_TO_POST_MESSAGE";

    }

    return "MNI_UNKNOWN_ERROR_CODE";
}

#pragma endregion

// ========================================================================== //
