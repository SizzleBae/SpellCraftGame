#define WIN32_LEAN_AND_MEAN 
#include <windows.h>

#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <io.h>
#include <fcntl.h>

#include <Source/Platform.h>

game_memory GameMemory{};

DEBUG_LOG(DebugLog) {
    int i = 0;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
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

            TextOut(hdc, 50, 190, (LPCWSTR) &GameMemory.TestChar, 1);
            // End application-specific layout section.

            EndPaint(hWnd, &ps);
            break;
        } 
        case WM_KEYDOWN:
        {
            switch (wParam) {
            case 'B': 
            {
                GameMemory.TestChar = wParam;
                break;
            }
            default:
                break;
            }
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

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int nCmdShow)
{
    AllocConsole();

    // set the screen buffer to be big enough to let us scroll text
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = 500;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

    //TODO(sizzle): This is appearantly unsafe?
    freopen("CONOUT$", "wb", stdout);
    freopen("CONOUT$", "wb", stderr);

    HMODULE GameDLLHandle = LoadLibraryA(".\\WindowsGameDLL.dll");
    if (!GameDLLHandle)
    {
        MessageBox(NULL,
            _T("Failed to load game DLL!"),
            _T("SpellCraft Game"),
            NULL);

        return 1;
    }

    game_update_and_render *GameUpdateAndRender = 
        (game_update_and_render*)GetProcAddress(GameDLLHandle, "GameUpdateAndRender");

    game_input GameInput{};
    GameMemory.DebugLog = &DebugLog;

    GameUpdateAndRender(&GameMemory, &GameInput);

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
    wcex.hInstance = Instance;
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
            _T("SpellCraft Game"),
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
        Instance,                          // The first parameter from WinMain
        NULL                                // Not used in this application
    );

    if (!hWnd)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("SpellCraft Game"),
            NULL);

        return 1;
    }

    ShowWindow(hWnd, nCmdShow);

    bool Run = true;
    int ExitCode = 0;
    while (Run) {
        // Main message loop:
        MSG Message;
        while (PeekMessage(&Message, NULL, 0, 0, PM_REMOVE))
        {
            if (Message.message == WM_QUIT) {
                Run = false;
                ExitCode = Message.wParam;
                break;
            }
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        UpdateWindow(hWnd);

    }
    return ExitCode;
}