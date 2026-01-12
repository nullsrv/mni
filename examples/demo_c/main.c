#define MNI_USE_DLL
#include <mni/mni.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <strsafe.h>

#define MENU_HELLO                  1000
#define MENU_EMPTY                  1001
#define MENU_DISABLED               1002
#define MENU_COUNTER                1003
#define MENU_IMG_ITEM               1004
#define MENU_HIDE_SHOW              1005
#define MENU_CLICKME                1006
#define MENU_CHECKME                1007
#define MENU_RADIOCHECK             1008
#define MENU_CUSTOMCHECK            1009
#define MENU_IMG_CHECK              1010
#define MENU_SHOW_BALLOON           1011
#define MENU_EXIT                   1012

#define MENU_STYLE_AUTO             1100
#define MENU_STYLE_IMMERSIVE        1101
#define MENU_STYLE_CLASSIC          1102

#define MENU_THEME_AUTO             1200
#define MENU_THEME_LIGHT            1201
#define MENU_THEME_DARK             1202

#define MENU_BALLOON_ICON_NONE      1300
#define MENU_BALLOON_ICON_INFO      1301
#define MENU_BALLOON_ICON_WARN      1302
#define MENU_BALLOON_ICON_ERROR     1303
#define MENU_BALLOON_ICON_USER      1304
#define MENU_BALLOON_PLAY_SOUND     1305
#define MENU_BALLOON_RESPECT_QUIET  1306

#define BALLOON_ICON_NONE           0
#define BALLOON_ICON_INFO           1
#define BALLOON_ICON_WARN           2
#define BALLOON_ICON_ERROR          3
#define BALLOON_ICON_USER           4

#define TIMER_HIDE_FOR_1_SEC        MNI_USER_TIMER_ID

typedef struct UINT2 {
    UINT first;
    UINT second;
} UINT2;

typedef struct Demo {
    int         counter;
    int         balloon_icon_type;
    BOOL        play_sound;
    BOOL        respect_quiet_time;
    HMENU       menu_popup;
    HMENU       menu_main;
    HMENU       menu_balloon;
    HMENU       menu_style;
    HMENU       menu_theme;
    HICON       ico_balloon;
    HBITMAP     bmp_checked;
    HBITMAP     bmp_unchecked;
    HBITMAP     bmp_custom;
} Demo;

UINT2 StyleItems[] = {
    { MENU_STYLE_AUTO,      MNI_ICM_STYLE_AUTO },
    { MENU_STYLE_IMMERSIVE, MNI_ICM_STYLE_IMMERSIVE },
    { MENU_STYLE_CLASSIC,   MNI_ICM_STYLE_CLASSIC }
};

UINT2 ThemeItems[] = {
    { MENU_THEME_AUTO,      MNI_ICM_THEME_AUTO },
    { MENU_THEME_LIGHT,     MNI_ICM_THEME_LIGHT },
    { MENU_THEME_DARK,      MNI_ICM_THEME_DARK }
};

UINT2 BalloonItems[] = {
    { MENU_BALLOON_ICON_NONE,  MNI_BALLOON_ICON_TYPE_NONE },
    { MENU_BALLOON_ICON_INFO,  MNI_BALLOON_ICON_TYPE_SYSTEM_INFO },
    { MENU_BALLOON_ICON_WARN,  MNI_BALLOON_ICON_TYPE_SYSTEM_WARNING },
    { MENU_BALLOON_ICON_ERROR, MNI_BALLOON_ICON_TYPE_SYSTEM_ERROR },
    { MENU_BALLOON_ICON_USER,  MNI_BALLOON_ICON_TYPE_CUSTOM }
};

#pragma region "Helper Methods"

static BOOL _IsColorLight(DWORD color) {
    BYTE r = GetRValue(color);
    BYTE g = GetGValue(color);
    BYTE b = GetBValue(color);

    return (((5 * g) + (2 * r) + b) > (8 * 128));
}

static void SetItemBitmap(HMENU hMenu, UINT id, HBITMAP hBmp) {
    MENUITEMINFOW referenceMii;
    memset(&referenceMii, 0, sizeof(referenceMii));
    referenceMii.cbSize = sizeof(referenceMii);
    referenceMii.fMask = MIIM_BITMAP;

    BOOL bGetRet = GetMenuItemInfoW(hMenu, id, FALSE, &referenceMii);
    if (!bGetRet) {
        MessageBoxW(0, L"Failed to get menu item info!", L"Error", MB_OK);
    } else {
        MENUITEMINFOW updatedMii;
        memset(&updatedMii, 0, sizeof(updatedMii));
        updatedMii.cbSize = sizeof(updatedMii);
        updatedMii.fMask = MIIM_BITMAP;
        updatedMii.hbmpItem = hBmp;

        BOOL bSetRet = SetMenuItemInfoW(hMenu, id, FALSE, &updatedMii);
        if (!bSetRet) {
            MessageBoxW(0, L"Failed to set menu item image!", L"Error", MB_OK);
        }
    }
}

static void SetItemText(HMENU hMenu, UINT id, const wchar_t *text, int len) {
    MENUITEMINFOW mii;
    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = (LPWSTR)text;
    mii.cch = len;

    BOOL bSetRet = SetMenuItemInfoW(hMenu, id, FALSE, &mii);
    if (!bSetRet) {
        MessageBoxW(0, L"Failed to set menu item text!", L"Error", MB_OK);
    }
}

static void SetItemCheckState(HMENU hMenu, UINT id, BOOL state) {
    MENUITEMINFOW referenceMii;
    memset(&referenceMii, 0, sizeof(referenceMii));
    referenceMii.cbSize = sizeof(referenceMii);
    referenceMii.fMask = MIIM_STATE;

    BOOL bGetRet = GetMenuItemInfoW(hMenu, id, FALSE, &referenceMii);
    if (!bGetRet) {
        MessageBoxW(0, L"Failed to get menu item info!", L"Error", MB_OK);
    } else {
        MENUITEMINFOW updatedMii;
        memset(&updatedMii, 0, sizeof(updatedMii));
        updatedMii.cbSize = sizeof(updatedMii);
        updatedMii.fMask = MIIM_STATE;

        if (state) {
            updatedMii.fState = referenceMii.fState | MFS_CHECKED;
        } else {
            updatedMii.fState = referenceMii.fState & ~MFS_CHECKED;
        }

        BOOL bSetRet = SetMenuItemInfoW(hMenu, id, FALSE, &updatedMii);
        if (!bSetRet) {
            MessageBoxW(0, L"Failed to set menu item state!", L"Error", MB_OK);
        }
    }
}

static BOOL IsItemChecked(HMENU hMenu, UINT id) {
    MENUITEMINFOW mii;
    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;

    BOOL bGetRet = GetMenuItemInfoW(hMenu, id, FALSE, &mii);
    if (bGetRet) {
        return (mii.fState & MFS_CHECKED) != 0;
    }

    return FALSE;
}

static UINT UpdateRadioCheckmarks(HMENU hMenu, UINT checked, UINT2 *items, int len) {
    UINT type = 0;

    for (int i = 0; i < len; i += 1) {
        if (checked == items[i].first) {
            type = items[i].second;
            break;
        }
    }

    for (int i = 0; i < len; i += 1) {
        if (checked == items[i].first) {
            SetItemCheckState(hMenu, items[i].first, TRUE);
        } else {
            SetItemCheckState(hMenu, items[i].first, FALSE);
        }
    }

    return type;
}

static HICON _LoadIcon(MniThemeInfo mti, int dpi) {
    BOOL use_light_icon = TRUE;
    if (mti.theme == MNI_THEME_LIGHT) {
         use_light_icon = FALSE;
    } else {
        if (mti.theme == MNI_THEME_HIGHCONTRAST && !_IsColorLight(mti.text_color)) {
            use_light_icon = FALSE;
        }
    }

    const wchar_t *path = use_light_icon ? L"data/icon_light.ico" : L"data/icon_dark.ico";
    const int size = MulDiv(16, dpi, 96);

    return (HICON)LoadImageW(0, path, IMAGE_ICON, size, size, LR_LOADFROMFILE);// | LR_DEFAULTSIZE);
}

#pragma endregion

#pragma region "Event Handlers"

void OnInit(Mni4 *MNI) {
    Demo *demo = NULL;
    if (MNI_FAILED(MniGetUserData1(MNI, &demo))) {
        return;
    }

    // If you want transparency you need 32-bit BMP.
    DWORD loadFlags = LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE;

    demo->bmp_custom    = (HBITMAP)LoadImageW(0, L"data/item.bmp", IMAGE_BITMAP, 0, 0, loadFlags);
    demo->bmp_checked   = (HBITMAP)LoadImageW(0, L"data/checked.bmp", IMAGE_BITMAP, 0, 0, loadFlags);
    demo->bmp_unchecked = (HBITMAP)LoadImageW(0, L"data/unchecked.bmp", IMAGE_BITMAP, 0, 0, loadFlags);
    demo->ico_balloon   = (HICON)LoadImageW(0, L"data/balloon.ico", IMAGE_ICON, 64, 64, LR_LOADFROMFILE | LR_DEFAULTSIZE);

    // Create menus.
    demo->menu_style   = CreateMenu();
    demo->menu_theme   = CreateMenu();
    demo->menu_balloon = CreateMenu();
    demo->menu_main    = CreateMenu();
    demo->menu_popup   = CreateMenu();

    AppendMenuW(demo->menu_style, MF_CHECKED   | MF_STRING | MFT_RADIOCHECK, MENU_STYLE_AUTO, L"Auto");
    AppendMenuW(demo->menu_style, MF_UNCHECKED | MF_STRING | MFT_RADIOCHECK, MENU_STYLE_IMMERSIVE, L"Immersive");
    AppendMenuW(demo->menu_style, MF_UNCHECKED | MF_STRING | MFT_RADIOCHECK, MENU_STYLE_CLASSIC, L"Classic");

    AppendMenuW(demo->menu_theme, MF_CHECKED   | MF_STRING | MFT_RADIOCHECK, MENU_THEME_AUTO, L"Auto");
    AppendMenuW(demo->menu_theme, MF_UNCHECKED | MF_STRING | MFT_RADIOCHECK, MENU_THEME_LIGHT, L"Light");
    AppendMenuW(demo->menu_theme, MF_UNCHECKED | MF_STRING | MFT_RADIOCHECK, MENU_THEME_DARK, L"Dark");

    AppendMenuW(demo->menu_balloon, MF_CHECKED | MF_STRING | MFT_RADIOCHECK, MENU_BALLOON_ICON_NONE, L"None");
    AppendMenuW(demo->menu_balloon, MF_STRING | MFT_RADIOCHECK, MENU_BALLOON_ICON_INFO, L"Info");
    AppendMenuW(demo->menu_balloon, MF_STRING | MFT_RADIOCHECK, MENU_BALLOON_ICON_WARN, L"Warning");
    AppendMenuW(demo->menu_balloon, MF_STRING | MFT_RADIOCHECK, MENU_BALLOON_ICON_ERROR, L"Error");
    AppendMenuW(demo->menu_balloon, MF_STRING | MFT_RADIOCHECK, MENU_BALLOON_ICON_USER, L"User");
    AppendMenuW(demo->menu_balloon, MF_SEPARATOR, 0, NULL);
    AppendMenuW(demo->menu_balloon, MF_STRING | MF_CHECKED, MENU_BALLOON_PLAY_SOUND, L"Play sound");
    AppendMenuW(demo->menu_balloon, MF_STRING | MF_CHECKED, MENU_BALLOON_RESPECT_QUIET, L"Respect quiet time");
    
    AppendMenuW(demo->menu_main, MF_STRING, MENU_HELLO, L"Hello");
    AppendMenuW(demo->menu_main, MF_STRING, MENU_EMPTY, L"");
    AppendMenuW(demo->menu_main, MF_STRING | MF_DISABLED, MENU_DISABLED, L"Disabled");
    AppendMenuW(demo->menu_main, MF_STRING | MF_GRAYED , MENU_COUNTER, L"Click count: 0");
    AppendMenuW(demo->menu_main, MF_STRING , MENU_IMG_ITEM, L"Item with image");
    AppendMenuW(demo->menu_main, MF_SEPARATOR, 0, NULL);
    AppendMenuW(demo->menu_main, MF_STRING, MENU_HIDE_SHOW, L"Hide for 1 second");
    AppendMenuW(demo->menu_main, MF_STRING, MENU_CLICKME, L"Click me");
    AppendMenuW(demo->menu_main, MF_STRING, MENU_CHECKME, L"Check me");
    AppendMenuW(demo->menu_main, MF_STRING | MFT_RADIOCHECK, MENU_RADIOCHECK, L"Radio check");
    AppendMenuW(demo->menu_main, MF_STRING, MENU_CUSTOMCHECK, L"Custom Checkmarks");
    AppendMenuW(demo->menu_main, MF_STRING, MENU_IMG_CHECK, L"Image and Checkmark");
    AppendMenuW(demo->menu_main, MF_SEPARATOR, 0, NULL);
    AppendMenuW(demo->menu_main, MF_POPUP | MF_STRING | MF_DISABLED, (UINT_PTR)demo->menu_style, L"Style");
    AppendMenuW(demo->menu_main, MF_POPUP | MF_STRING | MF_DISABLED, (UINT_PTR)demo->menu_theme, L"Theme");
    AppendMenuW(demo->menu_main, MF_POPUP | MF_STRING, (UINT_PTR)demo->menu_balloon, L"Balloon options");
    AppendMenuW(demo->menu_main, MF_SEPARATOR, 0, NULL);
    AppendMenuW(demo->menu_main, MF_STRING, MENU_SHOW_BALLOON, L"Show notification");
    AppendMenuW(demo->menu_main, MF_STRING, MENU_EXIT, L"Exit");
    
    AppendMenuW(demo->menu_popup, MF_POPUP, (UINT_PTR)demo->menu_main, L"");

    // Set custom checkmarks.
    SetMenuItemBitmaps(demo->menu_main, MENU_CUSTOMCHECK, MF_BYCOMMAND, demo->bmp_unchecked, demo->bmp_checked);

    // Set item icon. Using SetMenuItemInfoW() is recommended way.
    SetItemBitmap(demo->menu_main, MENU_IMG_ITEM, demo->bmp_custom);
    SetItemBitmap(demo->menu_main, MENU_IMG_CHECK, demo->bmp_custom);

    HICON ico = _LoadIcon(MNI->system_theme, MNI->dpi);

    MniSetIcon(MNI, ico, MNI_FALSE);
    MniSetMenu(MNI, demo->menu_popup, MNI_FALSE);
    MniSetTip(MNI, L"Demo Tip");
}

void OnRelease(Mni4 *MNI) {
    Demo *demo = NULL;
    if (MNI_FAILED(MniGetUserData1(MNI, &demo))) {
        return;
    }

    DeleteObject(demo->bmp_checked);
    DeleteObject(demo->bmp_unchecked);
    DeleteObject(demo->bmp_custom);
}

void OnLmbClick(Mni4 *MNI, int x, int y) {
    Demo *demo = NULL;
    if (MNI_FAILED(MniGetUserData1(MNI, &demo))) {
        return;
    }

    demo->counter++;

    wchar_t buffer[32];
    HRESULT hr = StringCchPrintfW(buffer, ARRAYSIZE(buffer), L"Click count: %d", demo->counter);
    if (FAILED(hr)) {
        const wchar_t str[] = L"Click count: X";
        SetItemText(demo->menu_main, MENU_COUNTER, str, ARRAYSIZE(str));
    } else {
        SetItemText(demo->menu_main, MENU_COUNTER, buffer, wcsnlen(buffer, ARRAYSIZE(buffer)));
    }
}

void OnLmbDoubleClick(Mni4 *MNI, int x, int y) {
    MessageBoxW(0, L"Double clicked on icon", L"Event", MB_OK);
}

void OnTaskbarCreated(Mni4 *MNI) {
    // Set 'recreate' paramater to MNI_TRUE, so icon get recreated.
    MniShow(MNI, MNI_TRUE);
}

void OnDpiChange(Mni4 *MNI, int dpi) {
    // Load icon with updated size.
    HICON ico = _LoadIcon(MNI->system_theme, dpi);
    MniSetIcon(MNI, ico, MNI_TRUE);
}

void OnSystemThemeChange(Mni4 *MNI, MniThemeInfo mti) {
    // Load icon depending on theme.
    HICON ico = _LoadIcon(mti, MNI->dpi);
    MniSetIcon(MNI, ico, TRUE);
}

void OnTimer(Mni4 *MNI, unsigned int timer_id) {
    if (timer_id == TIMER_HIDE_FOR_1_SEC) {
        MniShow(MNI, MNI_FALSE);
        MniStopTimer(MNI, TIMER_HIDE_FOR_1_SEC);
    }
}

void OnContextMenuClick(Mni4 *MNI, int selectedItem) {
    Demo *demo = NULL;
    if (MNI_FAILED(MniGetUserData1(MNI, &demo))) {
        return;
    }

    HMENU menu = MNI->menu;

    switch (selectedItem) {
    case MENU_EMPTY:
        SetItemText(menu, MENU_EMPTY, L"Text\tCtrl+N", 4);
        break;
    
    case MENU_HIDE_SHOW:
        MniHide(MNI);
        MniStartTimer(MNI, TIMER_HIDE_FOR_1_SEC, 1000);
        break;
    
    case MENU_CLICKME:
        MessageBoxW(NULL, L"Hello World.", L"Click Me", MB_OK);
        break;
    
    case MENU_CHECKME:
        SetItemCheckState(menu, MENU_CHECKME, !IsItemChecked(menu, MENU_CHECKME));
        break;
    
    case MENU_RADIOCHECK:
        SetItemCheckState(menu, MENU_RADIOCHECK, !IsItemChecked(menu, MENU_RADIOCHECK));
        break;
    
    case MENU_CUSTOMCHECK:
        SetItemCheckState(menu, MENU_CUSTOMCHECK, !IsItemChecked(menu, MENU_CUSTOMCHECK));
        break;
    
    case MENU_IMG_CHECK:
        SetItemCheckState(menu, MENU_IMG_CHECK, !IsItemChecked(menu, MENU_IMG_CHECK));
        break;
    
    case MENU_SHOW_BALLOON:
        {
            MniBalloonIconType bi = MNI_BALLOON_ICON_TYPE_NONE;
            switch (demo->balloon_icon_type) {
            case BALLOON_ICON_INFO:
                bi = MNI_BALLOON_ICON_TYPE_SYSTEM_INFO;
                break;
            case BALLOON_ICON_WARN:
                bi = MNI_BALLOON_ICON_TYPE_SYSTEM_WARNING;
                break;
            case BALLOON_ICON_ERROR:
                bi = MNI_BALLOON_ICON_TYPE_SYSTEM_ERROR;
                break;
            case BALLOON_ICON_USER:
                bi = MNI_BALLOON_ICON_TYPE_SYSTEM_ERROR;
                break;
            }
    
            MniBalloonFlags flags = MNI_BALLOON_FLAGS_DEFAULT;
            if (IsItemChecked(demo->menu_balloon, MENU_BALLOON_PLAY_SOUND)) {
                flags |= MNI_BALLOON_FLAGS_PLAY_SOUND;
            }
            
            if (IsItemChecked(demo->menu_balloon, MENU_BALLOON_RESPECT_QUIET)) {
                flags |= MNI_BALLOON_FLAGS_RESPECT_QUIET_TIME;
            }
    
            MniSendBalloonNotification(MNI, L"Title", L"Text", bi, demo->ico_balloon, flags);
        }
        break;
    
    case MENU_STYLE_AUTO:
        if (!IsItemChecked(demo->menu_style, MENU_STYLE_AUTO)) {
            SetItemCheckState(demo->menu_style, MENU_STYLE_AUTO, TRUE);
            SetItemCheckState(demo->menu_style, MENU_STYLE_IMMERSIVE, FALSE);
            SetItemCheckState(demo->menu_style, MENU_STYLE_CLASSIC, FALSE);
            MniSetIcmStyle(MNI, MNI_ICM_STYLE_AUTO);
        }
        break;
    case MENU_STYLE_IMMERSIVE:
        if (!IsItemChecked(demo->menu_style, MENU_STYLE_IMMERSIVE)) {
            SetItemCheckState(demo->menu_style, MENU_STYLE_AUTO, FALSE);
            SetItemCheckState(demo->menu_style, MENU_STYLE_IMMERSIVE, TRUE);
            SetItemCheckState(demo->menu_style, MENU_STYLE_CLASSIC, FALSE);
            MniSetIcmStyle(MNI, MNI_ICM_STYLE_IMMERSIVE);
        }
        break;
    case MENU_STYLE_CLASSIC:
        if (!IsItemChecked(demo->menu_style, MENU_STYLE_CLASSIC)) {
            SetItemCheckState(demo->menu_style, MENU_STYLE_AUTO, FALSE);
            SetItemCheckState(demo->menu_style, MENU_STYLE_IMMERSIVE, FALSE);
            SetItemCheckState(demo->menu_style, MENU_STYLE_CLASSIC, TRUE);
            MniSetIcmStyle(MNI, MNI_ICM_STYLE_CLASSIC);
        }
        break;
    
    case MENU_THEME_AUTO:
        if (!IsItemChecked(demo->menu_theme, MENU_THEME_AUTO)) {
            SetItemCheckState(demo->menu_theme, MENU_THEME_AUTO, TRUE);
            SetItemCheckState(demo->menu_theme, MENU_THEME_LIGHT, FALSE);
            SetItemCheckState(demo->menu_theme, MENU_THEME_DARK, FALSE);
            MniSetIcmTheme(MNI, MNI_ICM_THEME_AUTO);
        }
        break;
    case MENU_THEME_LIGHT:
        if (!IsItemChecked(demo->menu_theme, MENU_THEME_LIGHT)) {
            SetItemCheckState(demo->menu_theme, MENU_THEME_AUTO, FALSE);
            SetItemCheckState(demo->menu_theme, MENU_THEME_LIGHT, TRUE);
            SetItemCheckState(demo->menu_theme, MENU_THEME_DARK, FALSE);
            MniSetIcmTheme(MNI, MNI_ICM_THEME_LIGHT);
        }
        break;
    case MENU_THEME_DARK:
        if (!IsItemChecked(demo->menu_theme, MENU_THEME_DARK)) {
            SetItemCheckState(demo->menu_theme, MENU_THEME_AUTO, FALSE);
            SetItemCheckState(demo->menu_theme, MENU_THEME_LIGHT, FALSE);
            SetItemCheckState(demo->menu_theme, MENU_THEME_DARK, TRUE);
            MniSetIcmTheme(MNI, MNI_ICM_THEME_DARK);
        }
        break;
    
    case MENU_BALLOON_ICON_NONE:
        demo->balloon_icon_type = UpdateRadioCheckmarks(demo->menu_balloon, MENU_BALLOON_ICON_NONE, BalloonItems, ARRAYSIZE(BalloonItems));
        break;
    case MENU_BALLOON_ICON_INFO:
        demo->balloon_icon_type = UpdateRadioCheckmarks(demo->menu_balloon, MENU_BALLOON_ICON_INFO, BalloonItems, ARRAYSIZE(BalloonItems));
        break;
    case MENU_BALLOON_ICON_WARN:
        demo->balloon_icon_type = UpdateRadioCheckmarks(demo->menu_balloon, MENU_BALLOON_ICON_WARN, BalloonItems, ARRAYSIZE(BalloonItems));
        break;
    case MENU_BALLOON_ICON_ERROR:
        demo->balloon_icon_type = UpdateRadioCheckmarks(demo->menu_balloon, MENU_BALLOON_ICON_ERROR, BalloonItems, ARRAYSIZE(BalloonItems));
        break;
    case MENU_BALLOON_ICON_USER:
        demo->balloon_icon_type = UpdateRadioCheckmarks(demo->menu_balloon, MENU_BALLOON_ICON_USER, BalloonItems, ARRAYSIZE(BalloonItems));
        break;
    
    case MENU_BALLOON_PLAY_SOUND:
        if (!IsItemChecked(demo->menu_balloon, MENU_BALLOON_PLAY_SOUND)) {
            SetItemCheckState(demo->menu_balloon, MENU_BALLOON_PLAY_SOUND, TRUE);
            demo->play_sound = TRUE;
        } else {
            SetItemCheckState(demo->menu_balloon, MENU_BALLOON_PLAY_SOUND, FALSE);
            demo->play_sound = FALSE;
        }
        break;
    
    case MENU_BALLOON_RESPECT_QUIET:
        if (!IsItemChecked(demo->menu_theme, MENU_BALLOON_RESPECT_QUIET)) {
            SetItemCheckState(demo->menu_balloon, MENU_BALLOON_RESPECT_QUIET, TRUE);
            demo->respect_quiet_time = TRUE;
        } else {
            SetItemCheckState(demo->menu_balloon, MENU_BALLOON_RESPECT_QUIET, FALSE);
            demo->respect_quiet_time = FALSE;
        }
        break;
    
    case MENU_EXIT:
        MniQuit();
        break;
    }
}

#pragma endregion

int WINAPI wWinMain(
    _In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nShowCmd
) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nShowCmd);

    Demo demo = {
        .play_sound = TRUE,
        .respect_quiet_time = TRUE,
    };

    // Setup MniInfo.
    MniInfo info;
    memset(&info, 0, sizeof(info));
    //info.module_handle = hInstance;
    info.user_data1 = (void *)&demo;
    
    // Uncomment to use guid.
    //info.guid = (GUID){0xc527ebdc, 0xec8b, 0x4c1b, 0xbd, 0x76, 0xfd, 0xbf, 0xaa, 0x79, 0xfc, 0xa6};
    
    // Load all user resources.
    info.on_init = OnInit;

    // Release all user resources.
    info.on_release = OnRelease;

    // After explorer.exe restart, we want to show icon again.
    info.on_taskbar_created = OnTaskbarCreated;
    
    // After dpi change we want to load higher resolution icons.
    info.on_dpi_change = OnDpiChange;

    // After theme change we want to change icons colors.
    info.on_system_theme_change = OnSystemThemeChange;

    // Handle context menu events.
    info.on_context_menu_item_click = OnContextMenuClick;

    // Handle timer timeouts.
    info.on_timer = OnTimer;

    // It's no recommended to have events for both single and double click,
    // because single click callback will be called twice.
    info.on_lmb_click = OnLmbClick;
    info.on_lmb_double_click = OnLmbDoubleClick;

    // Init ModernNotifyIcon.
    Mni4 mni;
    if (MNI_FAILED(MniInit(&mni, info)))
    {
        MessageBoxW(NULL, L"Failed to initialize Notify Icon!", L"Error", MB_OK);
        return 2;
    }

    // Show the icon in Notification Area.
    if (MNI_FAILED(MniShow(&mni, MNI_FALSE)))
    {
        MessageBoxW(NULL, L"Failed to show Notify Icon!", L"Error", MB_OK);
        return 1;
    }

    int r = MniRunMessageLoop();
    MniRelease(&mni, MNI_TRUE, MNI_TRUE);

    return r;
}
