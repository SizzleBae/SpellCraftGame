#define WIN32_LEAN_AND_MEAN           
#include <Windows.h>

#include <Source/SpellCraftGame.h>

BOOL APIENTRY DllMain(HMODULE HModule, DWORD UlReasonForCall, LPVOID LpReserved)
{
    switch (UlReasonForCall)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

