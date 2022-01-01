#ifndef UNICODE
#define UNICODE
#endif

#include <Windows.h>
#include <strsafe.h>
#include <sstream>
#include <iomanip>

#include "resource.h"

const wchar_t szClass[] = L"RunCatCloneClass";
const wchar_t szTitle[] = L"RunCat Clone";

const UINT WM_APP_ANIMATE = WM_APP + 1;

HINSTANCE hInst;
NOTIFYICONDATA nid;

DWORD dwMilliseconds = 100;
std::wstringstream wss;

UINT64 previousIdleTime = 0;
UINT64 previousUsedTime = 0;
double previousPercent = 0;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    HANDLE hMutex = CreateMutex(NULL, FALSE, L"RunCatCloneMutex");

    if (!hMutex || GetLastError() == ERROR_ALREADY_EXISTS)
    {
        MessageBox(NULL, L"RunCat Clone is already running.", szTitle, MB_ICONWARNING);
        return 1;
    }

    hInst = hInstance;

    WNDCLASS wc{};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szClass;

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, L"Call to RegisterClass failed.", szTitle, MB_ICONWARNING);
        return 1;
    }

    HWND hwnd = CreateWindow(
        szClass,
        szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInst,
        NULL
    );

    if (!hwnd)
    {
        MessageBox(NULL, L"Call to CreateWindow failed.", szTitle, MB_ICONWARNING);
        return 1;
    }

    ShowWindow(hwnd, SW_HIDE);
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ReleaseMutex(hMutex);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hwnd;
            nid.uFlags = NIF_ICON | NIF_TIP | NIF_SHOWTIP;
            nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP));
            StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), szTitle);
            Shell_NotifyIcon(NIM_ADD, &nid);

            nid.uVersion = NOTIFYICON_VERSION_4;
            Shell_NotifyIcon(NIM_SETVERSION, &nid);

            PostMessage(hwnd, WM_APP_ANIMATE, 0, 0);
            return 0;
        }
        case WM_APP_ANIMATE:
        {
            nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WHITE_CAT_0));
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            Sleep(dwMilliseconds);

            nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WHITE_CAT_1));
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            Sleep(dwMilliseconds);

            nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WHITE_CAT_2));
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            Sleep(dwMilliseconds);

            nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WHITE_CAT_3));
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            Sleep(dwMilliseconds);

            nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WHITE_CAT_4));
            Shell_NotifyIcon(NIM_MODIFY, &nid);
            Sleep(dwMilliseconds);

            FILETIME idleTime, kernelTime, userTime;
            double currentPercent = previousPercent;

            if (GetSystemTimes(&idleTime, &kernelTime, &userTime))
            {
                UINT64 currentIdleTime
                    = (UINT64) idleTime.dwHighDateTime << 32 | (UINT64) idleTime.dwLowDateTime;
                UINT64 currentUsedTime
                    = ((UINT64) kernelTime.dwHighDateTime << 32 | (UINT64) kernelTime.dwLowDateTime)
                    + ((UINT64) userTime.dwHighDateTime << 32 | (UINT64) userTime.dwLowDateTime);

                UINT64 idleTimeChange = currentIdleTime - previousIdleTime;
                UINT64 usedTimeChange = currentUsedTime - previousUsedTime;

                currentPercent = (usedTimeChange - idleTimeChange) * 100.0 / usedTimeChange;

                previousIdleTime = currentIdleTime;
                previousUsedTime = currentUsedTime;
            }

            previousPercent = (previousPercent + currentPercent) / 2;
            dwMilliseconds = static_cast<DWORD>(2000 / (previousPercent + 20));

            wss << std::fixed << std::setprecision(1) << currentPercent;
            std::wstring wPercent(wss.str());
            wss.str(L"");
            wss.clear();
            StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), (wPercent + L"%").c_str());

            PostMessage(hwnd, WM_APP_ANIMATE, 0, 0);
            return 0;
        }
        case WM_DESTROY:
        {
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            return 0;
        }
        default:
        {
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
    }
}
