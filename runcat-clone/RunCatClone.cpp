#ifndef UNICODE
#define UNICODE
#endif

#include <thread>

#include <Windows.h>
#include <strsafe.h>

#include "resource.h"

WCHAR title[] = L"RunCat Clone 2025.2";
HINSTANCE instance;

LRESULT CALLBACK processMessages(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
NOTIFYICONDATA notification;
const int icons_length = 5;
HICON icons[icons_length];
std::thread thread;

void animate();
bool run_thread = true;
DWORD sleep_milliseconds = 100;
double previous_percent = 0;
UINT64 previous_idle_time = 0;
UINT64 previous_used_time = 0;

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE not_used, LPWSTR arguments, int show_flag) {
    HANDLE mutex = CreateMutex(NULL, FALSE, L"RunCat Clone Mutex");

    if (!mutex || GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBox(NULL, L"RunCat Clone is already running.", title, MB_ICONWARNING);
        return 1;
    }

    ::instance = instance;
    WCHAR class_name[] = L"RunCat Clone Class";

    WNDCLASS window_class{};
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = processMessages;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = instance;
    window_class.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_APP));
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    window_class.lpszMenuName = NULL;
    window_class.lpszClassName = class_name;

    if (!RegisterClass(&window_class)) {
        MessageBox(NULL, L"Call to RegisterClass failed.", title, MB_ICONWARNING);
        return 2;
    }

    HWND window = CreateWindow(
        class_name,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        instance,
        NULL
    );

    if (!window) {
        MessageBox(NULL, L"Call to CreateWindow failed.", title, MB_ICONWARNING);
        return 3;
    }

    ShowWindow(window, SW_HIDE);
    MSG message;

    while (GetMessage(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    ReleaseMutex(mutex);
    return 0;
}

LRESULT CALLBACK processMessages(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    switch (message) {
        case WM_CREATE:
        {
            notification.cbSize = sizeof(NOTIFYICONDATA);
            notification.hWnd = window;
            notification.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP;
            notification.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_APP));
            StringCchCopy(notification.szTip, ARRAYSIZE(notification.szTip), title);
            Shell_NotifyIcon(NIM_ADD, &notification);

            notification.uVersion = NOTIFYICON_VERSION_4;
            Shell_NotifyIcon(NIM_SETVERSION, &notification);

            for (int index = 0; index < icons_length; index++) {
                icons[index] = LoadIcon(instance, MAKEINTRESOURCE(IDI_WHITE_CAT_0 + index));
            }

            thread = std::thread(animate);
            break;
        }
        case WM_DESTROY:
        {
            run_thread = false;
            thread.join();

            Shell_NotifyIcon(NIM_DELETE, &notification);
            PostQuitMessage(0);
            break;
        }
        default:
        {
            return DefWindowProc(window, message, w_param, l_param);
        }

        return 0;
    }
}

void animate() {
    while (run_thread) {
        for (int index = 0; index < icons_length; index++) {
            notification.hIcon = icons[index];
            Shell_NotifyIcon(NIM_MODIFY, &notification);
            Sleep(sleep_milliseconds);
        }

        FILETIME idle_time, kernel_time, user_time;
        double current_cercent = previous_percent;

        if (GetSystemTimes(&idle_time, &kernel_time, &user_time)) {
            UINT64 current_idle_time
                = (UINT64) idle_time.dwHighDateTime << 32 | (UINT64) idle_time.dwLowDateTime;

            UINT64 current_used_time
                = ((UINT64) kernel_time.dwHighDateTime << 32 | (UINT64) kernel_time.dwLowDateTime)
                + ((UINT64) user_time.dwHighDateTime << 32 | (UINT64) user_time.dwLowDateTime);

            UINT64 idle_time_change = current_idle_time - previous_idle_time;
            UINT64 used_time_change = current_used_time - previous_used_time;
            current_cercent = (used_time_change - idle_time_change) * 100.0 / used_time_change;

            previous_idle_time = current_idle_time;
            previous_used_time = current_used_time;
        }

        previous_percent = (previous_percent + current_cercent) / 2;
        sleep_milliseconds = static_cast<DWORD>(2000 / (previous_percent + 20));
    }
}
