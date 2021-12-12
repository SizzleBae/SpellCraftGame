// HelloWindowsDesktop.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_PAINT: 
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // Here your application is laid out.
            TCHAR greeting[] = _T("Greetings, Spell Crafter!");
            TextOut(hdc, 50, 50, greeting, _tcslen(greeting));
            // End application-specific layout section.

            EndPaint(hWnd, &ps);
            break;
        } 
        case WM_DESTROY: 
        {
            PostQuitMessage(0);
            break;
        } 
        default: 
        {
            return DefWindowProc(hWnd, message, wParam, lParam);
            break;
        } 
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // The main window class name.
    TCHAR szWindowClass[] = _T("SpellCraftGame");

    // The string that appears in the application's title bar.
    TCHAR szTitle[] = _T("SpellCraft");

    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,             // An optional extended window style.
        szWindowClass,                      // The name of the application
        szTitle,                            // The text that appears in the title bar
        WS_OVERLAPPEDWINDOW,                // The type of window to create
        CW_USEDEFAULT, CW_USEDEFAULT,       // Initial position (x, y)  
        1000, 800,                          // Initial size (width, length)
        NULL,                               // The parent of this window
        NULL,                               // This application does not have a menu bar
        hInstance,                          // The first parameter from WinMain
        NULL                                // Not used in this application
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Windows Desktop Guided Tour"),
            NULL);

        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}