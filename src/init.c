/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-11 22:18:44
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-12 23:32:22
 * @FilePath: \CrystalGraphic\src\init.c
 * @Description: 
 * 
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <Defs.h>

void **CRCoreFunList;
CRLOCK windowCounterLock = NULL;

#ifdef CR_WINDOWS
#include <Windows.h>
extern LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static CRBOOL _inner_init_wndclass_(void)
{
    WNDCLASSEX wcex = {0};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_VREDRAW | CS_HREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = CR_WNDCLASS_NAME;
    wcex.hIconSm = NULL;
    if (!RegisterClassEx(&wcex))
    {
        CR_LOG_ERR("auto", "Error registering window class");
        return CRFALSE;
    }
    return CRTRUE;
}
static void _inner_uninit_wndclass_(void)
{
    if (!UnregisterClass(CR_WNDCLASS_NAME, GetModuleHandle(NULL))) CR_LOG_ERR("auto", "Failed unregister class");
}
#elif defined CR_LINUX
extern CRBOOL _inner_x_init_();
extern void _inner_x_uninit_();
#endif

CRAPI CRCODE CRModInit(void** list)
{
    if (!list || list[0] == list[2]) return 1;
    CRCoreFunList = list;
    windowCounterLock = CRLockCreate();
    if (!windowCounterLock) return 1;
    #ifdef CR_WINDOWS
    if (!_inner_init_wndclass_()) return 2;
    #elif defined CR_LINUX
    if (!_inner_x_init_()) return 3;
    #endif
    return 0;
}

CRAPI CRCODE CRModUninit(void)
{
    CRLockRelease(windowCounterLock);
    #ifdef CR_WINDOWS
    _inner_uninit_wndclass_();
    #elif defined CR_LINUX
    _inner_x_uninit_();
    #endif
	return 0;
}

CRAPI CRCODE CRGraphicModuleInit(void **coreList)
{
    return 0;
}