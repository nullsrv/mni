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

#include "cpp/mni.hpp"
#include "mni.h"

#include <type_traits>

#pragma region "Type Checks"

static_assert(std::is_same<std::underlying_type_t<mni::TipType>, std::underlying_type_t<MniTipType>>::value, "Incorrect Type!");
// TODO: more chceks

#pragma endregion

namespace mni
{
    extern const int Version = MNI_VERSION;
    extern const int VersionMajor = MNI_VERSION_MAJOR;
    extern const int VersionMinor = MNI_VERSION_MINOR;
    extern const int VersionRevision = MNI_VERSION_REVISON;
    extern const char* VersionString = MNI_VERSION_STRING;

    extern const unsigned int UserMessageId = MNI_USER_MESSAGE_ID;
    extern const unsigned int UserTimerId = MNI_USER_TIMER_ID;
}

namespace mni
{
    Error::Error(int error)
        : mError(error)
    {
    }

    bool Error::IsError() const
    {
        return MNI_FAILED(mError);
    }

    bool Error::IsOk() const
    {
        return MNI_SUCCEEDED(mError);
    }
}

#pragma region "Callbacks"

namespace mni
{
    static void OnWindowCreateCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnWindowCreate)
        {
            pNotifyIcon->OnWindowCreate();
        }
    }

    static void OnWindowDestroyCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnWindowDestroy)
        {
            pNotifyIcon->OnWindowDestroy();
        }
    }

    static void OnInitCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnInit)
        {
            pNotifyIcon->OnInit();
        }
    }

    static void OnReleaseCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnRelease)
        {
            pNotifyIcon->OnRelease();
        }
    }

    static void OnShowCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnShow)
        {
            pNotifyIcon->OnShow();
        }
    }

    static void OnHideCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnHide)
        {
            pNotifyIcon->OnHide();
        }
    }

    static void OnIconChangeCallback(Mni4* mni, HICON icon)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnIconChange)
        {
            pNotifyIcon->OnIconChange(icon);
        }
    }

    static void OnMenuChangeCallback(Mni4* mni, HMENU menu)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnMenuChange)
        {
            pNotifyIcon->OnMenuChange(menu);
        }
    }

    static void OnTipChangeCallback(Mni4* mni, const wchar_t* tip)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnTipChange)
        {
            pNotifyIcon->OnTipChange(tip);
        }
    }

    static void OnTipTypeChangeCallback(Mni4* mni, MniTipType mtt)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnTipTypeChange)
        {
            pNotifyIcon->OnTipTypeChange(static_cast<TipType>(mtt));
        }
    }

    static void OnKeySelectCallback(Mni4* mni, int x, int y)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnKeySelect)
        {
            pNotifyIcon->OnKeySelect(x, y);
        }
    }

    static void OnLmbClickCallback(Mni4* mni, int x, int y)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnLmbClick)
        {
            pNotifyIcon->OnLmbClick(x, y);
        }
    }

    static void OnLmbDoubleClickCallback(Mni4* mni, int x, int y)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnLmbDoubleClick)
        {
            pNotifyIcon->OnLmbDoubleClick(x, y);
        }
    }

    static void OnMmbClickCallback(Mni4* mni, int x, int y)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnMmbClick)
        {
            pNotifyIcon->OnMmbClick(x, y);
        }
    }

    static void OnContextMenuOpenCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnContextMenuOpen)
        {
            pNotifyIcon->OnContextMenuOpen();
        }
    }

    static void OnContextMenuItemClickCallback(Mni4* mni, int selected_item)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnContextMenuItemClick)
        {
            pNotifyIcon->OnContextMenuItemClick(selected_item);
        }
    }

    static void OnContextMenuCloseCallback(Mni4* mni, MniBool was_item_selected)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnContextMenuClose)
        {
            pNotifyIcon->OnContextMenuClose(was_item_selected == MNI_TRUE);
        }
    }

    static void OnBalloonShowCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnBalloonShow)
        {
            pNotifyIcon->OnBalloonShow();
        }
    }

    static void OnBalloonHideCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnBalloonHide)
        {
            pNotifyIcon->OnBalloonHide();
        }
    }

    static void OnBalloonTimeoutCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnBalloonTimeout)
        {
            pNotifyIcon->OnBalloonTimeout();
        }
    }

    static void OnBalloonClickCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnBalloonClick)
        {
            pNotifyIcon->OnBalloonClick();
        }
    }

    static void OnRichPopupOpenCallback(Mni4* mni, int x, int y)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnRichPopupOpen)
        {
            pNotifyIcon->OnRichPopupOpen(x, y);
        }
    }

    static void OnRichPopupCloseCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnRichPopupClose)
        {
            pNotifyIcon->OnRichPopupClose();
        }
    }

    static void OnDpiChangeCallback(Mni4* mni, int dpi)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnDpiChange)
        {
            pNotifyIcon->OnDpiChange(dpi);
        }
    }

    static void OnSystemThemeChangeCallback(Mni4* mni, MniThemeInfo mti)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnSystemThemeChange)
        {
            pNotifyIcon->OnSystemThemeChange(ThemeInfo{static_cast<Theme>(mti.theme), mti.text_color, mti.background_color});
        }
    }

    static void OnAppsThemeChangeCallback(Mni4* mni, MniThemeInfo mti)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnAppsThemeChange)
        {
            pNotifyIcon->OnAppsThemeChange(ThemeInfo{static_cast<Theme>(mti.theme), mti.text_color, mti.background_color});
        }
    }

    static void OnTaskbarCreatedCallback(Mni4* mni)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnTaskbarCreated)
        {
            pNotifyIcon->OnTaskbarCreated();
        }
    }

    static void OnTimerCallback(Mni4* mni, unsigned int id)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnTimer)
        {
            pNotifyIcon->OnTimer(id);
        }
    }

    static void OnCustomMessageCallback(Mni4* mni, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnCustomMessage)
        {
            pNotifyIcon->OnCustomMessage(msg, wParam, lParam);
        }
    }

    static BOOL OnSystemMessageCallback(Mni4* mni, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        NotifyIcon* pNotifyIcon = reinterpret_cast<NotifyIcon*>(mni->reserved1);
        if (pNotifyIcon && pNotifyIcon->OnSystemMessage)
        {
            return pNotifyIcon->OnSystemMessage(msg, wParam, lParam);
        }

        return FALSE;
    }
} // namespace mni

#pragma endregion

#pragma region "Impl"

class mni::NotifyIcon::CImpl
{
public:
    CImpl()
    {
        pMni = new Mni4();
    }

    ~CImpl()
    {
        if (pMni)
        {
            delete pMni;
        }
    }

    MniError Init(MniInfo info)
    {
        return MniInit(pMni, info);
    }

    MniError Release(MniBool destroyIcon, MniBool destroyMenu)
    {
        return MniRelease(pMni, destroyIcon, destroyMenu);
    }

    MniError Show(MniBool recreate)
    {
        return MniShow(pMni, recreate);
    }

    MniError Hide()
    {
        return MniHide(pMni);
    }

    MniError SetIcon(HICON icon, MniBool destroy_current)
    {
        return MniSetIcon(pMni, icon, destroy_current);
    }

    MniError SetMenu(HMENU menu, MniBool destroy_current)
    {
        return MniSetMenu(pMni, menu, destroy_current);
    }

    MniError SetTip(const wchar_t* tip)
    {
        return MniSetTip(pMni, tip);
    }

    MniError SetTipType(MniTipType mtt)
    {
        return MniSetTipType(pMni, mtt);
    }

    MniError GetIcon(HICON* icon)
    {
        return MniGetIcon(pMni, icon);
    }

    MniError GetMenu(HMENU* menu)
    {
        return MniGetMenu(pMni, menu);
    }

    MniError GetTip(wchar_t* buffer, int* len)
    {
        return MniGetTip(pMni, buffer, len);
    }

    MniError GetTipType(MniTipType* mtt)
    {
        return MniGetTipType(pMni, mtt);
    }

    MniError IsNotifyIconCreated(MniBool* is_created)
    {
        return MniIsNotifyIconCreated(pMni, is_created);
    }

    MniError IsNotifyIconVisible(MniBool* is_visible)
    {
        return MniIsNotifyIconVisible(pMni, is_visible);
    }

    MniError GetWindowHandle(HWND* window_handle)
    {
        return MniGetWindowHandle(pMni, window_handle);
    }

    MniError GetInstanceHandle(HINSTANCE* instance_handle)
    {
        return MniGetInstanceHandle(pMni, instance_handle);
    }

    MniError GetDpi(int* dpi)
    {
        return MniGetDpi(pMni, dpi);
    }

    MniError GetSystemThemeInfo(MniThemeInfo* system_theme)
    {
        return MniGetSystemThemeInfo(pMni, system_theme);
    }

    MniError GetAppsThemeInfo(MniThemeInfo* apps_theme)
    {
        return MniGetAppsThemeInfo(pMni, apps_theme);
    }

    MniError SetIcmStyle(MniIcmStyle icm_style)
    {
        return MniSetIcmStyle(pMni, icm_style);
    }

    MniError SetIcmTheme(MniIcmTheme icm_theme)
    {
        return MniSetIcmTheme(pMni, icm_theme);
    }

    MniError SetIcmPosition(MniIcmPosition icm_pos)
    {
        return MniSetIcmPosition(pMni, icm_pos);
    }

    MniError SetIcmAnimation(MniIcmAnimation icm_anim)
    {
        return MniSetIcmAnimation(pMni, icm_anim);
    }

    MniError GetIcmStyle(MniIcmStyle* icm_style)
    {
        return MniGetIcmStyle(pMni, icm_style);
    }

    MniError GetIcmTheme(MniIcmTheme* icm_theme)
    {
        return MniGetIcmTheme(pMni, icm_theme);
    }

    MniError GetIcmPosition(MniIcmPosition* icm_pos)
    {
        return MniGetIcmPosition(pMni, icm_pos);
    }

    MniError GetIcmAnimation(MniIcmAnimation* icm_anim)
    {
        return MniGetIcmAnimation(pMni, icm_anim);
    }

    MniError SetUserData1(void* data)
    {
        return MniSetUserData1(pMni, data);
    }

    MniError SetUserData2(void* data)
    {
        return MniSetUserData2(pMni, data);
    }

    MniError GetUserData1(void** data)
    {
        return MniGetUserData1(pMni, data);
    }

    MniError GetUserData2(void** data)
    {
        return MniGetUserData2(pMni, data);
    }

    MniError SendBalloonNotification(
        const wchar_t           *title,
        const wchar_t           *text,
        MniBalloonIconType      icon_type,
        HICON                   icon,
        MniBalloonFlags         flags
    ) {
        return MniSendBalloonNotification(pMni, title, text, icon_type, icon, flags);
    }

    MniError RemoveBalloonNotification()
    {
        return MniRemoveBalloonNotification(pMni);
    }

    MniError StartTimer(unsigned int timer_id, unsigned int interval)
    {
        return MniStartTimer(pMni, timer_id, interval);
    }

    MniError StopTimer(unsigned int timer_id)
    {
        return MniStopTimer(pMni, timer_id);
    }

    MniError SendCustomMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return MniSendCustomMessage(pMni, msg, wParam, lParam);
    }

    MniError PostCustomMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return MniPostCustomMessage(pMni, msg, wParam, lParam);
    }

    const wchar_t* ErrorToString(MniError error)
    {
        return MniErrorToString(error);
    }

    int RunMessageLoop(void)
    {
        return MniRunMessageLoop();
    }

    void Quit(void)
    {
        return MniQuit();
    }

    MniError SetTipUTF8(const char* tip)
    {
        return MniSetTipUTF8(pMni, tip);
    }

    MniError GetTipUTF8(char* buffer, int* len)
    {
        return MniGetTipUTF8(pMni, buffer, len);
    }

    MniError SendBalloonNotificationUTF8(
        const char              *title,
        const char              *text,
        MniBalloonIconType      icon_type,
        HICON                   icon,
        MniBalloonFlags         flags
    ) {
        return MniSendBalloonNotificationUTF8(pMni, title, text, icon_type, icon, flags);
    }

    const char* ErrorToStringUTF8(MniError error)
    {
        return MniErrorToStringUTF8(error);
    }

    Mni4* GetMniPtr()
    {
        return pMni;
    }

private:
    Mni4* pMni;
};

#pragma endregion

namespace mni
{
    NotifyIcon::NotifyIcon()
        : mIconDestroyPolicy (ResourceDestroyPolicy::Auto)
        , mMenuDestroyPolicy (ResourceDestroyPolicy::Auto)
    {
        pImpl = new CImpl();
        if (!pImpl)
        {
            InternalThrow();
        }
    }

    NotifyIcon::~NotifyIcon()
    {
        if (pImpl)
        {
            pImpl->Release(
                mIconDestroyPolicy == RDP::Auto ? MNI_TRUE : MNI_FALSE,
                mMenuDestroyPolicy == RDP::Auto ? MNI_TRUE : MNI_FALSE
            );
            delete pImpl;
        }
    }

    Error NotifyIcon::Init(Info info)
    {
        if (!pImpl)
        {
            return Error::MniPtrIsNull;
        }

        MniInfo mnii;
        memset(&mnii, 0, sizeof(mnii));    
    
        mnii.instance_handle = info.instanceHandle;
        mnii.class_name = info.className;
        mnii.guid = info.guid;
        mnii.icon = info.icon;
        mnii.menu = info.menu;
        mnii.tip = info.tip;
        mnii.tip_type = static_cast<MniTipType>(info.tipType);
        mnii.icm_style = static_cast<MniIcmStyle>(info.icmStyle);
        mnii.icm_theme = static_cast<MniIcmTheme>(info.icmTheme);
        mnii.window_style = info.windowStyle;
        mnii.window_title = info.windowTitle;
        mnii.icm_position = static_cast<MniIcmPosition>(info.icmPosition);
        mnii.icm_animation = static_cast<MniIcmAnimation>(info.icmAnimation);
        mnii.parent_window = info.parentWindow;
        mnii.user_data1 = info.userData1;
        mnii.user_data2 = info.userData2;

        mnii.reserved1 = this;
        mnii.reserved2 = nullptr;

        mnii.on_window_create = OnWindowCreateCallback;
        mnii.on_window_destroy = OnWindowDestroyCallback;
        mnii.on_init = OnInitCallback;
        mnii.on_release = OnReleaseCallback;
        mnii.on_show = OnShowCallback;
        mnii.on_hide = OnHideCallback;
        mnii.on_icon_change = OnIconChangeCallback;
        mnii.on_menu_change = OnMenuChangeCallback;
        mnii.on_tip_change = OnTipChangeCallback;
        mnii.on_tip_type_change = OnTipTypeChangeCallback;
        mnii.on_key_select = OnKeySelectCallback;
        mnii.on_lmb_click = OnLmbClickCallback;
        mnii.on_lmb_double_click = OnLmbDoubleClickCallback;
        mnii.on_mmb_click = OnMmbClickCallback;
        mnii.on_context_menu_open = OnContextMenuOpenCallback;
        mnii.on_context_menu_item_click = OnContextMenuItemClickCallback;
        mnii.on_context_menu_close = OnContextMenuCloseCallback;
        mnii.on_balloon_show = OnBalloonShowCallback;
        mnii.on_balloon_hide = OnBalloonHideCallback;
        mnii.on_balloon_timeout = OnBalloonTimeoutCallback;
        mnii.on_balloon_click = OnBalloonClickCallback;
        mnii.on_rich_popup_open = OnRichPopupOpenCallback;
        mnii.on_rich_popup_close = OnRichPopupCloseCallback;
        mnii.on_dpi_change = OnDpiChangeCallback;
        mnii.on_system_theme_change = OnSystemThemeChangeCallback;
        mnii.on_apps_theme_change = OnAppsThemeChangeCallback;
        mnii.on_taskbar_created = OnTaskbarCreatedCallback;
        mnii.on_timer = OnTimerCallback;
        mnii.on_custom_message = OnCustomMessageCallback;
        mnii.on_system_message = OnSystemMessageCallback;

        OnWindowCreate = info.OnWindowCreate;
        OnWindowDestroy = info.OnWindowDestroy;
        OnInit = info.OnInit;
        OnRelease = info.OnRelease;
        OnShow = info.OnShow;
        OnHide = info.OnHide;
        OnIconChange = info.OnIconChange;
        OnMenuChange = info.OnMenuChange;
        OnTipChange = info.OnTipChange;
        OnTipTypeChange = info.OnTipTypeChange;
        OnKeySelect = info.OnKeySelect;
        OnLmbClick = info.OnLmbClick;
        OnLmbDoubleClick = info.OnLmbDoubleClick;
        OnMmbClick = info.OnMmbClick;
        OnContextMenuOpen = info.OnContextMenuOpen;
        OnContextMenuItemClick = info.OnContextMenuItemClick;
        OnContextMenuClose = info.OnContextMenuClose;
        OnBalloonShow = info.OnBalloonShow;
        OnBalloonHide = info.OnBalloonHide;
        OnBalloonTimeout = info.OnBalloonTimeout;
        OnBalloonClick = info.OnBalloonClick;
        OnRichPopupOpen = info.OnRichPopupOpen;
        OnRichPopupClose = info.OnRichPopupClose;
        OnDpiChange = info.OnDpiChange;
        OnSystemThemeChange = info.OnSystemThemeChange;
        OnAppsThemeChange = info.OnAppsThemeChange;
        OnTaskbarCreated = info.OnTaskbarCreated;
        OnTimer = info.OnTimer;
        OnCustomMessage = info.OnCustomMessage;
        OnSystemMessage = info.OnSystemMessage;

        mMenuDestroyPolicy = info.menuDestroyPolicy;
        mIconDestroyPolicy = info.iconDestroyPolicy;
    
        return pImpl->Init(mnii);
    }

    Error NotifyIcon::Release(bool destroyIcon, bool destroyMenu)
    {
        return pImpl->Release(
            destroyIcon ? MNI_TRUE : MNI_FALSE,
            destroyMenu ? MNI_TRUE : MNI_FALSE
        );
    }

    Error NotifyIcon::Show(bool recreate)
    {
        return pImpl->Show(recreate ? MNI_TRUE : MNI_FALSE);
    }

    Error NotifyIcon::Hide()
    {
        return pImpl->Hide();
    }

    HICON NotifyIcon::GetIcon() const
    {
        return pImpl->GetMniPtr()->icon;
    }

    HMENU NotifyIcon::GetMenu() const
    {
        return pImpl->GetMniPtr()->menu;
    }

    std::wstring NotifyIcon::GetTip() const
    {
        return std::wstring(pImpl->GetMniPtr()->tip);
    }

    TipType NotifyIcon::GetTipType() const
    {
        return static_cast<TipType>(pImpl->GetMniPtr()->tip_type);
    }

    Error NotifyIcon::SetIcon(HICON icon, ResourceDestroyPolicy rdp)
    {
        MniBool destroy = (mIconDestroyPolicy == ResourceDestroyPolicy::Auto) ? MNI_TRUE : MNI_FALSE;
        mIconDestroyPolicy = rdp;
        return pImpl->SetIcon(icon, destroy);
    }

    Error NotifyIcon::SetMenu(HMENU menu, ResourceDestroyPolicy rdp)
    {
        MniBool destroy = (mMenuDestroyPolicy == ResourceDestroyPolicy::Auto) ? MNI_TRUE : MNI_FALSE;
        mMenuDestroyPolicy = rdp;
        return pImpl->SetMenu(menu, destroy);
    }

    Error NotifyIcon::SetTip(const std::wstring& tip)
    {
        return pImpl->SetTip(tip.c_str());
    }

    Error NotifyIcon::SetTipType(mni::TipType tipType)
    {
        return pImpl->SetTipType(static_cast<MniTipType>(tipType));
    }

    bool NotifyIcon::IsCreated() const
    {
        return pImpl->GetMniPtr()->icon_created == MNI_TRUE;
    }

    bool NotifyIcon::IsVisible() const
    {
        return pImpl->GetMniPtr()->icon_visible == MNI_TRUE;
    }

    HINSTANCE NotifyIcon::GetInstanceHandle() const
    {
        return pImpl->GetMniPtr()->instance_handle;
    }

    HWND NotifyIcon::GetWindowHandle() const
    {
        return pImpl->GetMniPtr()->window_handle;
    }

    int NotifyIcon::GetDpi()
    {
        return pImpl->GetMniPtr()->dpi;
    }

    mni::ThemeInfo NotifyIcon::GetSystemThemeInfo() const
    {
        return mni::ThemeInfo{
            static_cast<mni::Theme>(pImpl->GetMniPtr()->system_theme.theme),
            pImpl->GetMniPtr()->system_theme.text_color,
            pImpl->GetMniPtr()->system_theme.background_color,
        };
    }

    mni::ThemeInfo NotifyIcon::GetAppsThemeInfo() const
    {
        return mni::ThemeInfo{
            static_cast<mni::Theme>(pImpl->GetMniPtr()->apps_theme.theme),
            pImpl->GetMniPtr()->apps_theme.text_color,
            pImpl->GetMniPtr()->apps_theme.background_color,
        };
    }

    void NotifyIcon::SetIcmStyle(mni::IcmStyle style)
    {
        pImpl->GetMniPtr()->icm_style = static_cast<MniIcmStyle>(style);
    }

    void NotifyIcon::SetIcmTheme(mni::IcmTheme theme)
    {
        pImpl->GetMniPtr()->icm_theme = static_cast<MniIcmTheme>(theme);
    }

    void NotifyIcon::SetIcmPosition(mni::IcmPosition position)
    {
        pImpl->GetMniPtr()->icm_position = static_cast<MniIcmPosition>(position);
    }

    void NotifyIcon::SetIcmAnimation(mni::IcmAnimation anim)
    {
        pImpl->GetMniPtr()->icm_animation = static_cast<MniIcmAnimation>(anim);
    }

    mni::IcmStyle NotifyIcon::GetIcmStyle() const
    {
        return static_cast<mni::IcmStyle>(pImpl->GetMniPtr()->icm_style);
    }

    mni::IcmTheme NotifyIcon::GetIcmTheme() const
    {
        return static_cast<mni::IcmTheme>(pImpl->GetMniPtr()->icm_theme);
    }

    mni::IcmPosition NotifyIcon::GetIcmPosition() const
    {
        return static_cast<mni::IcmPosition>(pImpl->GetMniPtr()->icm_position);
    }

    mni::IcmAnimation NotifyIcon::GetIcmAnimation() const
    {
        return static_cast<mni::IcmAnimation>(pImpl->GetMniPtr()->icm_animation);
    }

    void* NotifyIcon::GetUserData1() const
    {
        return pImpl->GetMniPtr()->user_data1;
    }

    void* NotifyIcon::GetUserData2() const
    {
        return pImpl->GetMniPtr()->user_data2;
    }

    void NotifyIcon::SetUserData1(void* data) const
    {
        pImpl->GetMniPtr()->user_data1 = data;
    }

    void NotifyIcon::SetUserData2(void* data) const
    {
        pImpl->GetMniPtr()->user_data2 = data;
    }

    Error NotifyIcon::SendBalloonNotification(
        const std::wstring& title,
        const std::wstring& text,
        BalloonIconType     iconType,
        HICON               icon,
        BalloonFlags        flags
    ) {
        return pImpl->SendBalloonNotification(
            title.c_str(),
            text.c_str(),
            static_cast<MniBalloonIconType>(iconType),
            icon,
            static_cast<MniBalloonFlags>(flags)
        );
    }

    Error NotifyIcon::RemoveBalloonNotification()
    {
        return pImpl->RemoveBalloonNotification();
    }

    Error NotifyIcon::StartTimer(unsigned int id, unsigned int interval)
    {
        return pImpl->StartTimer(id, interval);
    }

    Error NotifyIcon::StopTimer(unsigned int id)
    {
        return pImpl->StopTimer(id);
    }

    Error NotifyIcon::SendCustomMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return pImpl->SendCustomMessage(msg, wParam, lParam);
    }

    Error NotifyIcon::PostCustomMessage(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return pImpl->PostCustomMessage(msg, wParam, lParam);
    }

    Error NotifyIcon::SetTipUTF8(const std::string& str)
    {
        return pImpl->SetTipUTF8(str.c_str());
    }

    Error NotifyIcon::GetTipUTF8(std::string& str)
    {
        char buffer[1024];
        int size = ARRAYSIZE(buffer);

        MniError result = pImpl->GetTipUTF8(buffer, &size);
        if (MNI_FAILED(result))
        {
            return result;
        }

        str = std::string(buffer);
        return result;
    }

    Error NotifyIcon::SendBalloonNotificationUTF8(
        const std::string&  title,
        const std::string&  text,
        BalloonIconType     iconType,
        HICON               icon,
        BalloonFlags        flags
    ) {
        return pImpl->SendBalloonNotificationUTF8(
            title.c_str(),
            text.c_str(),
            static_cast<MniBalloonIconType>(iconType),
            icon,
            static_cast<MniBalloonFlags>(flags)
        );
    }

    int NotifyIcon::RunMessageLoop()
    {
        return MniRunMessageLoop();
    }

    void NotifyIcon::Quit()
    {
        MniQuit();
    }
} // namespace mni
