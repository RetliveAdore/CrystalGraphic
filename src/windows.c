/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-23 00:38:41
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-26 16:37:58
 * @FilePath: \Crystal-Graphic\src\windows.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <GraphicDfs.h>

#ifdef CR_WINDOWS
#include <windows.h>
#include <CrystalThread.h>

static CRINT64 windowCounter = 0;
CRLOCK lock = NULL;

typedef struct crwindow_inner
{
    HWND hWnd;
    CRUINT32 fps;
    PCRWindowProperties prop;
    //
    CRWindowCallback funcs[CALLBACK_FUNCS_NUM];
    CRBOOL onProcess;
    //
    CRTHREAD eventThread;
    CRTHREAD paintThread;
    CRTHREAD moveControl;
}CRWINDOWINNER, *PCRWINDOWINNER;

static CRCODE _inner_empty_callback_(PCRWINDOWMSG msg)
{
    CR_LOG_IFO("auto", "empty callback");
    return 0;
}

static LRESULT AfterProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, PCRWINDOWINNER pInner)
{
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PCRWINDOWINNER pInner;
    CRWINDOWMSG inf;
    if (msg == WM_CREATE)
    {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(lpcs->lpCreateParams));
        pInner = lpcs->lpCreateParams;
        return 0;
    }
    pInner = (PCRWINDOWINNER)(CRUINT64)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (!pInner) return DefWindowProc(hWnd, msg, wParam, lParam);
    if (msg == WM_CLOSE)
    {
        if (!pInner->funcs[CRWINDOW_QUIT_CB](&inf))
            DestroyWindow(hWnd);
        CR_LOG_IFO("auto", "Close window");
        return 0;
    }
    else if (msg == WM_DESTROY)
    {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NULL);
        pInner->onProcess = CRFALSE;
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return AfterProc(hWnd, msg, wParam, lParam, pInner);
}

static void _inner_window_thread_(CRLVOID data, CRTHREAD idThis)
{
    PCRWINDOWINNER pInner = data;
    pInner->hWnd = CreateWindow(
        CR_WNDCLASS_NAME,
        pInner->prop->title, WS_OVERLAPPEDWINDOW,
        pInner->prop->x, pInner->prop->y,
        pInner->prop->w, pInner->prop->h,
        NULL, NULL, GetModuleHandle(NULL), pInner
    );
    UpdateWindow(pInner->hWnd);
    ShowWindow(pInner->hWnd, SW_SHOWDEFAULT);
    //
    CR_LOG_IFO("auto", "Create Window");
    MSG msg = {0};
    while (pInner->onProcess)
    {
        if (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    pInner->prop->window = (CRWINDOW)NULL;
    CRAlloc(pInner, 0);
    //
    CRLock(lock);
    windowCounter--;
    CRUnlock(lock);
}

CRAPI CRINT64 CRWindowCounter(void)
{
    return windowCounter;
}

CRAPI void CRCreateWindow(PCRWindowProperties prop)
{
    if (!prop) CR_LOG_ERR("auto", "nullptr");
    prop->window = (CRWINDOW)NULL;
    PCRWINDOWINNER pInner = CRAlloc(NULL, sizeof(CRWINDOWINNER));
    if (!pInner)
    {
        CR_LOG_ERR("auto", "bad alloc");
        return;
    }
    for (int i = 0; i < CALLBACK_FUNCS_NUM; i++)
        pInner->funcs[i] = _inner_empty_callback_;
    pInner->onProcess = CRTRUE;
    pInner->prop = prop;
    pInner->eventThread = CRThread(_inner_window_thread_, pInner);
    //
    //多线程操作需要加锁
    CRLock(lock);
    windowCounter++;
    CRUnlock(lock);
    //
    prop->window = (CRWINDOW)pInner;
}

CRAPI void CRCloseWindow(CRWINDOW window)
{
    if (!window)
    {
        CR_LOG_WAR("auto", "invalid window");
        return;
    }
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    CloseWindow(pInner->hWnd);
}

CRAPI void CRSetWindowCbk(CRWINDOW window, CRUINT8 type, CRWindowCallback cbk)
{
    if (!window)
    {
        CR_LOG_WAR("auto", "invalid window");
        return;
    }
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    if (cbk)
        pInner->funcs[type] = cbk;
    else
        pInner->funcs[type] = _inner_empty_callback_;
}

#endif