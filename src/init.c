/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-23 00:42:19
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-26 13:39:49
 * @FilePath: \Crystal-Graphic\src\init.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <GraphicDfs.h>
#include <CrystalThread.h>

void **CRCoreFunList = NULL;
void **CRThreadFunList = NULL;
extern CRLOCK lock;

#ifdef CR_WINDOWS
#include <Windows.h>

extern LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static void _inner_init_wndclass_(void)
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
    if (!RegisterClassEx(&wcex)) CR_LOG_ERR("auto", "Error registering window class");
}
static void _inner_uninit_wndclass_(void)
{
    if (!UnregisterClass(CR_WNDCLASS_NAME, GetModuleHandle(NULL))) CR_LOG_ERR("auto", "Failed unregister class");
}
#endif

CRAPI CRBOOL CRModInit(void **ptr)
{
    if (ptr[0] == ptr[1])
        return CRFALSE;
    CRCoreFunList = ptr;
    #ifdef CR_WINDOWS
    _inner_init_wndclass_();
    #endif
    return CRTRUE;
}

CRAPI void CRModUninit(void)
{
    #ifdef CR_WINDOWS
    _inner_uninit_wndclass_();
    #endif
    if (lock) CRLockRelease(lock);
}

CRAPI CRBOOL CrystalGraphicInit(void** thr)
{
    if (!thr)
    {
        CR_LOG_ERR("auto", "nullptr");
        return CRFALSE;
    }
    if (thr[0] == thr[2])
    {
        CR_LOG_ERR("auto", "CRThreadFunList not inited correctly");
        return CRFALSE;
    }
    CRThreadFunList = thr;
    lock = CRLockCreate();
    return CRTRUE;
}