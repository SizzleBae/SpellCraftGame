#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

#include <tchar.h>
#include <iostream>
#include <wingdi.h>

#include <Source/Platform.h>
 
struct win32_screen_buffer {
    uint32* XRGB;
    int32 Width;
    int32 Height;
    BITMAPINFO Info;
};

global_variable win32_screen_buffer GlobalScreenBuffer;

DEBUG_LOG(DebugLog) {
    std::cout << Message << std::endl;
}

void DisplayScreenBufferInWindow(HWND Window, HDC DeviceContextHandle, win32_screen_buffer *ScreenBuffer) {
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    int32 ClientWidth = ClientRect.right - ClientRect.left;
	int32 ClientHeight = ClientRect.bottom - ClientRect.top;

    // Center and scale destination area such that it fits the window
    int32 DestX, DestY, DestWidth, DestHeight;
    if (ClientWidth <= ClientHeight) {
        DestWidth = ClientWidth;
        real32 Scale = real32(DestWidth) / real32(ScreenBuffer->Width);
        DestHeight = int32(Scale * real32(ScreenBuffer->Height));

        DestX = 0;
        DestY = (ClientHeight - DestHeight) / 2;
    }
    else {
        DestHeight = ClientHeight;
        real32 Scale = real32(DestHeight) / real32(ScreenBuffer->Height);
        DestWidth = int32(Scale * real32(ScreenBuffer->Width));

        DestX = (ClientWidth - DestWidth) / 2;
        DestY = 0;
    }

    // Fill background
    HRGN BgRegion = CreateRectRgnIndirect(&ClientRect);
    HBRUSH BgBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRgn(DeviceContextHandle, BgRegion, BgBrush);
    DeleteObject(BgRegion);
    DeleteObject(BgBrush);

    StretchDIBits(DeviceContextHandle,
        DestX, DestY, DestWidth, DestHeight,
        0, 0, ScreenBuffer->Width, ScreenBuffer->Height,
        ScreenBuffer->XRGB,
        &ScreenBuffer->Info,
        DIB_RGB_COLORS, SRCCOPY);

}

LRESULT CALLBACK WindowCallback(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message) {
        case WM_DESTROY: 
        {
            PostQuitMessage(0);
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT PaintStruct;
            HDC DeviceContextHandle = BeginPaint(WindowHandle, &PaintStruct);
            DisplayScreenBufferInWindow(WindowHandle, DeviceContextHandle, &GlobalScreenBuffer);
            EndPaint(WindowHandle, &PaintStruct);
        } break;
        default: 
        {
            return DefWindowProc(WindowHandle, Message, WParam, LParam);
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

    //TODO(sizzle): This is apparently unsafe?
    freopen("CONOUT$", "wb", stdout);
    freopen("CONOUT$", "wb", stderr);

    HMODULE GameDllHandle = LoadLibraryA(".\\WindowsGameDLL.dll");
    if (!GameDllHandle)
    {
        MessageBox(nullptr,
                   _T("Failed to load game DLL!"),
                   _T("SpellCraft Game"),
                   NULL);

        return 1;
    }

    game_update_and_render *GameUpdateAndRender = 
        reinterpret_cast<game_update_and_render*>(GetProcAddress(GameDllHandle, "GameUpdateAndRender"));

    game_output_sound *GameOutputSound =
        reinterpret_cast<game_output_sound*>(GetProcAddress(GameDllHandle, "GameOutputSound"));

    // The main window class name.
    TCHAR szWindowClass[] = _T("SpellCraftGame");

    // The string that appears in the application's title bar.
    TCHAR szTitle[] = _T("SpellCraft");

    WNDCLASSEX WindowClass;
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowCallback;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = Instance;
    WindowClass.hIcon = LoadIcon(WindowClass.hInstance, IDI_APPLICATION);
    WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WindowClass.lpszMenuName = nullptr;
    WindowClass.lpszClassName = szWindowClass;
    WindowClass.hIconSm = LoadIcon(WindowClass.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&WindowClass))
    {
        MessageBox(nullptr,
                   _T("Call to RegisterClassEx failed!"),
                   _T("SpellCraft Game"),
                   NULL);

        return 1;
    }

    HWND WindowHandle = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,             // An optional extended window style.
        szWindowClass,                      // The name of the application
        szTitle,                            // The text that appears in the title bar
        WS_OVERLAPPEDWINDOW,                // The type of window to create
        CW_USEDEFAULT, CW_USEDEFAULT,       // Initial position (x, y)  
        1000, 800,                          // Initial size (width, length)
        nullptr,                               // The parent of this window
        nullptr,                               // This application does not have a menu bar
        Instance,                          // The first parameter from WinMain
        nullptr        // Not used in this application
    );

    if (!WindowHandle)
    {
        MessageBox(nullptr,
                   _T("Call to CreateWindow failed!"),
                   _T("SpellCraft Game"),
                   NULL);

        return 1;
    }

    ShowWindow(WindowHandle, nCmdShow);

    game_memory GameMemory{};
    GameMemory.PermanentStorageSize = Megabytes(256);
    GameMemory.TransientStorageSize = Gigabytes(1);
    GameMemory.DebugLog = &DebugLog;

    uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    void *GameMemoryBlock = VirtualAlloc(0, TotalSize, //TODO(sizzle): use base address
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE);
    GameMemory.PermanentStorage = GameMemoryBlock;
    GameMemory.TransientStorage = (static_cast<uint8*>(GameMemory.PermanentStorage) +
        GameMemory.PermanentStorageSize);

    game_input GameInput{};

    GlobalScreenBuffer.Width = 300;
    GlobalScreenBuffer.Height = 300;
    GlobalScreenBuffer.XRGB = static_cast<uint32*>(VirtualAlloc(0, sizeof(uint32) * GlobalScreenBuffer.Width * GlobalScreenBuffer.Height,
                                                          MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    GlobalScreenBuffer.Info.bmiHeader.biSize = sizeof(GlobalScreenBuffer.Info.bmiHeader);
    GlobalScreenBuffer.Info.bmiHeader.biWidth = GlobalScreenBuffer.Width;
    GlobalScreenBuffer.Info.bmiHeader.biHeight = -GlobalScreenBuffer.Height;
    GlobalScreenBuffer.Info.bmiHeader.biPlanes = 1;
    GlobalScreenBuffer.Info.bmiHeader.biBitCount = 32;
    GlobalScreenBuffer.Info.bmiHeader.biCompression = BI_RGB;

    bool Run = true;
    int ExitCode = 0;
    while (Run) {
        // Main message loop:
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            switch (Message.message) {
                case WM_QUIT:
                {
                    Run = false;
                    ExitCode = static_cast<int>(Message.wParam);
                } break;
                case WM_KEYDOWN:
                {
                    //TODO(sizzle): Get this input by querying key state instead
                    switch (Message.wParam) {
                        case 'W':
                        {
                            GameInput.Up = 1.0f;
                        } break;
                        case 'S':
                        {
                            GameInput.Down = 1.0f;
                        } break;
                        case 'A':
                        {
                            GameInput.Left = 1.0f;
                        } break;
                        case 'D':
                        {
                            GameInput.Right = 1.0f;
                        } break;
                    default: break;
                    }
                } break;
                case WM_KEYUP:
                {
                    switch (Message.wParam) {
                    case 'W':
                    {
                        GameInput.Up = 0.0f;
                    } break;
                    case 'S':
                    {
                        GameInput.Down = 0.0f;
                    } break;
                    case 'A':
                    {
                        GameInput.Left = 0.0f;
                    } break;
                    case 'D':
                    {
                        GameInput.Right = 0.0f;
                    } break;
                    default: break;
                    }
                } break;
                default: 
                {
                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                } break;
            }
        }

        //TODO(sizzle): Should maybe not cast the screen buffer like this? :3
        GameUpdateAndRender(&GameMemory, &GameInput, reinterpret_cast<screen_buffer*>(&GlobalScreenBuffer));

        HDC DeviceContextHandle = GetDC(WindowHandle);
        DisplayScreenBufferInWindow(WindowHandle, DeviceContextHandle, &GlobalScreenBuffer);

        game_sound_output_buffer SoundBuffer;
        GameOutputSound(&GameMemory, &SoundBuffer);


        //TODO(sizzle): Do proper timing stuff 
        Sleep(16);

    }

    return ExitCode;
}