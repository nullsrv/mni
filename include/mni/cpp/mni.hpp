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

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <functional>
#include <string>

#if defined(MNI_CPP_USE_EXCEPTIONS)
    #include <stdexcept>
    #define MNI_CPP_THROW(e) std::runtime_error(e)
#else
    #if defined(_DEBUG)
        #include <cassert>
        #define MNI_CPP_THROW(e) assert(0 && e)
    #else
        #define MNI_CPP_THROW(e) do{}while(0)
    #endif // _DEBUG
#endif // MNI_CPP_USE_EXCEPTIONS

namespace mni
{
    extern const int Version;
    extern const int VersionMajor;
    extern const int VersionMinor;
    extern const int VersionRevision;
    extern const char* VersionString;

    extern const unsigned int UserMessageId;
    extern const unsigned int UserTimerId;

    enum class TipType : int
    {
        Standard            = 0,
        Rich                = 1,
    };

    enum class BalloonIconType : int
    {
        None                = 0,
        SystemInfo          = 1,
        SystemWarning       = 2,
        SystemError         = 3,
        Custom              = 4,
    };

    enum class BalloonFlags : int
    {
        Default             = (1 << 0),
        RealTime            = (1 << 1),
        PlaySound           = (1 << 2),
        RespectQuietTime    = (1 << 3),
    };
    DEFINE_ENUM_FLAG_OPERATORS(BalloonFlags)

    enum class IcmStyle : int
    {
        Auto                = 0,
        Immersive           = 1,
        Classic             = 2,
        Win32               = 3,
    };

    enum class IcmTheme : int
    {
        Auto                = 0,
        Light               = 1,
        Dark                = 2,
    };

    enum class IcmPosition : int
    {
        Default             = 0,
    };

    enum class IcmAnimation : int
    {
        Default             = 0,
    };

    enum class Theme : int
    {
        Dark                = 0,
        Light               = 1,
        HighContrast        = 2,
    };

    struct ThemeInfo
    {
        Theme theme;
        DWORD text_color;
        DWORD background_color;
    };

    enum class ResourceDestroyPolicy
    {
        Auto,
        Manual
    };

    struct Error
    {
        enum
        {
            Ok                         = 0,
            WindowAlreadyCreated       = 1,
            IconAlreadyCreated         = 2,
            IconAlreadyShown           = 3,
            IconAlreadyHidden          = 4,
            MniPtrIsNull               = -1,
            UnsupportedVersion         = -2,
            FailedToAddIcon            = -3,
            FailedToDeleteIcon         = -4,
            FailedToShowIcon           = -5,
            FailedToHideIcon           = -6,
            FailedToChangeIcon         = -7,
            FailedToChangeTip          = -8,
            FailedToCopyTip            = -9,
            FailedToCopyBalloonTitle   = -10,
            FailedToCopyBalloonText    = -11,
            FailedToShowBalloon        = -12,
            FailedToRemoveBalloon      = -13,
            FailedToRegisterWndclass   = -14,
            FailedToCreateWindow       = -15,
            FailedToStartTimer         = -16,
            FailedToStopTimer          = -17,
            InvalidModuleHandle        = -18,
            InvalidWindowHandle        = -19,
            InvalidIcon                = -20,
            InvalidMenu                = -21,
            InvalidTimerId             = -22,
            InvalidArgument            = -23,
            IconNotCreated             = -24,
            InsufficientBuffer         = -25,
            FailedToConvertTip         = -26,
            FailedToConvertTitle       = -27,
            FailedToConvertText        = -28,
            FailedToSendMessage        = -29,
            FailedToPostMessage        = -30,
        };

        Error(int error);
        bool IsError() const;
        bool IsOk() const;

        operator bool() const
        {
            return IsOk();
        }

    private:
        int mError;
    };

    class NotifyIcon
    {
    public:
        using RDP = ResourceDestroyPolicy;

        struct Info
        {
            HINSTANCE           instanceHandle          = NULL;
            const wchar_t*      className               = nullptr;
            GUID                guid                    = GUID{0};
            HICON               icon                    = NULL;
            HMENU               menu                    = NULL;
            const wchar_t*      tip                     = nullptr;
            mni::TipType        tipType                 = mni::TipType::Standard;
            mni::IcmStyle       icmStyle                = mni::IcmStyle::Auto;
            mni::IcmTheme       icmTheme                = mni::IcmTheme::Auto;
            DWORD               windowStyle             = 0;
            const wchar_t*      windowTitle             = L"";
            mni::IcmPosition    icmPosition             = mni::IcmPosition::Default;
            mni::IcmAnimation   icmAnimation            = mni::IcmAnimation::Default;
            HWND                parentWindow            = NULL;
            void*               userData1               = nullptr;
            void*               userData2               = nullptr;
            RDP                 menuDestroyPolicy       = RDP::Auto;
            RDP                 iconDestroyPolicy       = RDP::Auto;

            std::function<void()>                       OnWindowCreate;
            std::function<void()>                       OnWindowDestroy;
            std::function<void()>                       OnInit;
            std::function<void()>                       OnRelease;
            std::function<void()>                       OnShow;
            std::function<void()>                       OnHide;
            std::function<void(HICON)>                  OnIconChange;
            std::function<void(HMENU)>                  OnMenuChange;
            std::function<void(const std::wstring&)>    OnTipChange;
            std::function<void(mni::TipType)>           OnTipTypeChange;
            std::function<void(int, int)>               OnKeySelect;
            std::function<void(int, int)>               OnLmbClick;
            std::function<void(int, int)>               OnLmbDoubleClick;
            std::function<void(int, int)>               OnMmbClick;
            std::function<void()>                       OnContextMenuOpen;
            std::function<void(int)>                    OnContextMenuItemClick;
            std::function<void(bool)>                   OnContextMenuClose;
            std::function<void()>                       OnBalloonShow;
            std::function<void()>                       OnBalloonHide;
            std::function<void()>                       OnBalloonTimeout;
            std::function<void()>                       OnBalloonClick;
            std::function<void(int, int)>               OnRichPopupOpen;
            std::function<void()>                       OnRichPopupClose;
            std::function<void(int)>                    OnDpiChange;
            std::function<void(mni::ThemeInfo)>         OnSystemThemeChange;
            std::function<void(mni::ThemeInfo)>         OnAppsThemeChange;
            std::function<void()>                       OnTaskbarCreated;
            std::function<void(int)>                    OnTimer;
            std::function<void(UINT, WPARAM, LPARAM)>   OnCustomMessage;
            std::function<BOOL(UINT, WPARAM, LPARAM)>   OnSystemMessage;
        };

    public:
        std::function<void()>                       OnWindowCreate;
        std::function<void()>                       OnWindowDestroy;
        std::function<void()>                       OnInit;
        std::function<void()>                       OnRelease;
        std::function<void()>                       OnShow;
        std::function<void()>                       OnHide;
        std::function<void(HICON)>                  OnIconChange;
        std::function<void(HMENU)>                  OnMenuChange;
        std::function<void(const std::wstring&)>    OnTipChange;
        std::function<void(mni::TipType)>           OnTipTypeChange;
        std::function<void(int, int)>               OnKeySelect;
        std::function<void(int, int)>               OnLmbClick;
        std::function<void(int, int)>               OnLmbDoubleClick;
        std::function<void(int, int)>               OnMmbClick;
        std::function<void()>                       OnContextMenuOpen;
        std::function<void(int)>                    OnContextMenuItemClick;
        std::function<void(bool)>                   OnContextMenuClose;
        std::function<void()>                       OnBalloonShow;
        std::function<void()>                       OnBalloonHide;
        std::function<void()>                       OnBalloonTimeout;
        std::function<void()>                       OnBalloonClick;
        std::function<void(int, int)>               OnRichPopupOpen;
        std::function<void()>                       OnRichPopupClose;
        std::function<void(int)>                    OnDpiChange;
        std::function<void(mni::ThemeInfo)>         OnSystemThemeChange;
        std::function<void(mni::ThemeInfo)>         OnAppsThemeChange;
        std::function<void()>                       OnTaskbarCreated;
        std::function<void(int)>                    OnTimer;
        std::function<void(UINT, WPARAM, LPARAM)>   OnCustomMessage;
        std::function<BOOL(UINT, WPARAM, LPARAM)>   OnSystemMessage;

    public:
        NotifyIcon();
        ~NotifyIcon();

        NotifyIcon(const NotifyIcon&) = delete;
        NotifyIcon& operator=(const NotifyIcon&) = delete;

        Error Init(Info info);
        Error Release(bool destroyIcon, bool destroyMenu);
        Error Show(bool recreate = false);
        Error Hide();

        HICON GetIcon() const;
        HMENU GetMenu() const;
        std::wstring GetTip() const;
        TipType GetTipType() const;

        Error SetIcon(HICON icon, ResourceDestroyPolicy rdp = ResourceDestroyPolicy::Auto);
        Error SetMenu(HMENU menu, ResourceDestroyPolicy rdp = ResourceDestroyPolicy::Auto);
        Error SetTip(const std::wstring& tip);
        Error SetTipType(TipType tipType);
    
        bool IsCreated() const;
        bool IsVisible() const;

        HINSTANCE GetInstanceHandle() const;
        HWND GetWindowHandle() const;

        int GetDpi();

        ThemeInfo GetSystemThemeInfo() const;
        ThemeInfo GetAppsThemeInfo() const;

        void SetIcmStyle(IcmStyle style);
        void SetIcmTheme(IcmTheme theme);
        void SetIcmPosition(IcmPosition position);
        void SetIcmAnimation(IcmAnimation anim);

        IcmStyle GetIcmStyle() const;
        IcmTheme GetIcmTheme() const;
        IcmPosition GetIcmPosition() const;
        IcmAnimation GetIcmAnimation() const;

        void* GetUserData1() const;
        void* GetUserData2() const;

        void SetUserData1(void* data) const;
        void SetUserData2(void* data) const;

        Error SendBalloonNotification(
            const std::wstring& title,
            const std::wstring& text,
            BalloonIconType     iconType,
            HICON               icon,
            BalloonFlags        flags
        );
        Error RemoveBalloonNotification();

        Error StartTimer(unsigned int id, unsigned int interval);
        Error StopTimer(unsigned int id);

        Error SendCustomMessage(UINT msg, WPARAM wParam, LPARAM lParam);
        Error PostCustomMessage(UINT msg, WPARAM wParam, LPARAM lParam);

        Error SetTipUTF8(const std::string& str);
        Error GetTipUTF8(std::string& str);

        Error SendBalloonNotificationUTF8(
            const std::string&  title,
            const std::string&  text,
            BalloonIconType     iconType,
            HICON               icon,
            BalloonFlags        flags
        );

        int RunMessageLoop();
        void Quit();

    private:
        void InternalThrow()
        {
            MNI_CPP_THROW("failed to allocate memory for mni");
        }

    private:
        class CImpl;

        CImpl*  pImpl;
        RDP     mIconDestroyPolicy;
        RDP     mMenuDestroyPolicy;
    };
} // namespace mni
