#include <Windows.h>
#include <dsound.h>
#include <wingdi.h>

#include <tchar.h>
#include <iostream>
#include <format>

#include <Source/Platform.h>
 
struct win32_screen_buffer {
    int32 Width;
    int32 Height;
    uint32* XRGB;
    BITMAPINFO Info;
};

global_variable win32_screen_buffer GlobalScreenBuffer;

DEBUG_LOG(DebugLog) {
    std::cout << Message << std::endl;
}

void LogLastWindowsError()
{
    LPSTR ErrorDescription;
    DWORD ErrorCode = GetLastError();

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        ErrorCode,
        0x0409, // NOTE(sizzle): This is the id for en-US language
        (LPSTR)&ErrorDescription,
        0, nullptr);

    std::cerr << "Windows Error (" << ErrorCode << ") " << ErrorDescription << std::endl;

    LocalFree(ErrorDescription);
}

bool WriteEntireFile(const char *Path, void *Buffer, uint32 ByteCount)
{
    HANDLE FileHandle = CreateFileA(Path, 
        GENERIC_WRITE, 
        0, 
        nullptr, 
        CREATE_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        nullptr);

    if(FileHandle == INVALID_HANDLE_VALUE)
    {
        LogLastWindowsError();
        return false;
    }

    DWORD BytesWritten;
    if(!WriteFile(FileHandle,
        Buffer,    
        ByteCount,  
        &BytesWritten, 
        NULL) || BytesWritten != ByteCount)
    {
        LogLastWindowsError();
        return false;
    }

    CloseHandle(FileHandle);
    return true;
}

bool ReadEntireFile(const char* Path, void* Buffer, uint32 MaxBytes)
{
    HANDLE FileHandle = CreateFileA(Path,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);

    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        LogLastWindowsError();
        return false;
    }

    DWORD BytesRead;
    if (!ReadFile(FileHandle, Buffer, MaxBytes, &BytesRead, nullptr))
    {
        LogLastWindowsError();
        return false;
    }

    CloseHandle(FileHandle);
    return true;
}

//TODO(sizzle): This could me moved to game code by having it call platform methods for writing to file
void SaveGameState(int32 Slot, game_memory *State)
{
    std::string FileName = std::format("save_{}", Slot);

    std::cout << "SAVE: " << FileName << std::endl;

    Assert(State->PermanentStorageSize < MAXUINT32)
    WriteEntireFile(FileName.c_str(), State->PermanentStorage, State->PermanentStorageSize);
}

void LoadGameState(int32 Slot, game_memory *State)
{
    std::string FileName = std::format("save_{}", Slot);

    std::cout << "LOAD: " << FileName << std::endl;

    Assert(State->PermanentStorageSize < MAXUINT32)
        ReadEntireFile(FileName.c_str(), State->PermanentStorage, State->PermanentStorageSize);
}

void DisplayScreenBufferInWindow(HWND Window, HDC DeviceContext, win32_screen_buffer *ScreenBuffer)
{
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
    FillRgn(DeviceContext, BgRegion, BgBrush);
    DeleteObject(BgRegion);
    DeleteObject(BgBrush);

    StretchDIBits(DeviceContext,
        DestX, DestY, DestWidth, DestHeight,
        0, 0, ScreenBuffer->Width, ScreenBuffer->Height,
        ScreenBuffer->XRGB,
        &ScreenBuffer->Info,
        DIB_RGB_COLORS, SRCCOPY);

}

LRESULT CALLBACK WindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message) {
        case WM_DESTROY: 
        {
            PostQuitMessage(0);
        } break;
        case WM_PAINT:
        {
            PAINTSTRUCT PaintStruct;
            HDC DeviceContext = BeginPaint(Window, &PaintStruct);
            DisplayScreenBufferInWindow(Window, DeviceContext, &GlobalScreenBuffer);
            EndPaint(Window, &PaintStruct);
        } break;
        default: 
        {
            return DefWindowProc(Window, Message, WParam, LParam);
        } 
    }

    return 0;
}

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int nCmdShow)
{
    // The main window class name.
    TCHAR WindowClassName[] = _T("SpellCraftGame");

    // The string that appears in the application's title bar.
    TCHAR ApplicationName[] = _T("SpellCraft");

    AllocConsole();

    // set the screen buffer to be big enough to let us scroll text
    CONSOLE_SCREEN_BUFFER_INFO ConsoleInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ConsoleInfo);
    ConsoleInfo.dwSize.Y = 500;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), ConsoleInfo.dwSize);

    //TODO(sizzle): This is apparently unsafe?
    freopen("CONOUT$", "wb", stdout);
    freopen("CONOUT$", "wb", stderr);

    HMODULE GameDll = LoadLibraryA(".\\WindowsGameDLL.dll");
    if (!GameDll)
    {
        MessageBox(nullptr, _T("Failed to load game DLL!"),ApplicationName,NULL);
        return 1;
    }

#pragma warning(push)
#pragma warning(disable:4191)

    game_update_and_render *GameUpdateAndRender = 
        reinterpret_cast<game_update_and_render*>(GetProcAddress(GameDll, "GameUpdateAndRender"));

    game_output_sound *GameOutputSound =
        reinterpret_cast<game_output_sound*>(GetProcAddress(GameDll, "GameOutputSound"));

#pragma warning(pop)

    WNDCLASSEX WindowClass{};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = WindowCallback;
    WindowClass.cbClsExtra = 0;
    WindowClass.cbWndExtra = 0;
    WindowClass.hInstance = Instance;
    WindowClass.hIcon = LoadIcon(WindowClass.hInstance, IDI_APPLICATION);
    WindowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    WindowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    WindowClass.lpszMenuName = nullptr;
    WindowClass.lpszClassName = WindowClassName;
    WindowClass.hIconSm = LoadIcon(WindowClass.hInstance, IDI_APPLICATION);

    if (!RegisterClassEx(&WindowClass))
    {
        MessageBox(nullptr, _T("Call to RegisterClassEx failed!"), ApplicationName, NULL);
        return 1;
    }

    HWND Window = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW,             // An optional extended window style.
        WindowClassName,                      // The name of the application
        ApplicationName,                            // The text that appears in the title bar
        WS_OVERLAPPEDWINDOW,                // The type of window to create
        CW_USEDEFAULT, CW_USEDEFAULT,       // Initial position (x, y)  
        1000, 800,                          // Initial size (width, length)
        nullptr,                               // The parent of this window
        nullptr,                               // This application does not have a menu bar
        Instance,                          // The first parameter from WinMain
        nullptr        // Not used in this application
    );

    if (!Window)
    {
        MessageBox(nullptr, _T("Call to CreateWindow failed!"), ApplicationName, NULL);
        return 1;
    }

    ShowWindow(Window, nCmdShow);

    game_memory GameMemory{};
    GameMemory.PermanentStorageSize = Megabytes(256);
    GameMemory.TransientStorageSize = Gigabytes(1);
    GameMemory.DebugLog = &DebugLog;

    uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    void *GameMemoryBlock = VirtualAlloc(nullptr, TotalSize, //TODO(sizzle): use base address
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE);
    GameMemory.PermanentStorage = GameMemoryBlock;
    GameMemory.TransientStorage = (static_cast<uint8*>(GameMemory.PermanentStorage) +
        GameMemory.PermanentStorageSize);

    game_input GameInput{};

    GlobalScreenBuffer = {
    	.Width = 300,
    	.Height = 300
    };

    GlobalScreenBuffer.XRGB = static_cast<uint32*>(VirtualAlloc(nullptr, sizeof(uint32) * GlobalScreenBuffer.Width * GlobalScreenBuffer.Height,
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
        while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
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
	                    case '1': case '2': case '3': case '4':
	                    {
                            int32 Slot = (int32)Message.wParam - 49;
                            if(GetKeyState(VK_CONTROL) & 0x8000)
                            {
                                SaveGameState(Slot, &GameMemory);
                            } else
                            {
                                LoadGameState(Slot, &GameMemory);
                            }
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

        screen_buffer OutScreenBuffer = { .XRGB = GlobalScreenBuffer.XRGB, .Width = GlobalScreenBuffer.Width, .Height = GlobalScreenBuffer.Height };
        GameUpdateAndRender(&GameMemory, &GameInput, &OutScreenBuffer);

        HDC DeviceContext = GetDC(Window);
        DisplayScreenBufferInWindow(Window, DeviceContext, &GlobalScreenBuffer);

        game_sound_output_buffer SoundBuffer{ .SamplesPerSecond = 44000, .SampleCount = 1 };
        GameOutputSound(&GameMemory, &SoundBuffer);


        //TODO(sizzle): Do proper timing stuff 
        Sleep(16);

    }

    return ExitCode;
}