/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-08 16:57:03
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-15 22:16:11
 * @FilePath: \CrystalGraphic\src\windows.c
 * @Description: 
 * 
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crwindow.h>

#ifdef CR_WINDOWS
#include <windowsx.h>

static CRINT64 windowCounter = 0;
extern CRLOCK windowCounterLock;

static LRESULT AfterProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, PCRWINDOWINNER pInner)
{
    CRWINDOWMSG inf = {0};
    inf.window = (CRWINDOW)pInner;
    inf.x = GET_X_LPARAM(lParam);
    inf.y = GET_Y_LPARAM(lParam);
    inf.keycode = wParam & 0xff;
    inf.status = CRUI_STAT_OTHER;
    switch (msg)
    {
        case WM_PAINT:
            break;
        case WM_MOUSEMOVE:
            if (inf.y > CRUI_TITLEBAR_PIXEL)
            {
                inf.status = CRUI_STAT_MOVE;
                pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
            }
            pInner->cursor.x = inf.x;
            pInner->cursor.y = inf.y;
            return 0;
        case WM_SETCURSOR:
            if (pInner->cursor.y < CRUI_TITLEBAR_PIXEL && pInner->cursor.x < CRUI_TITLEBAR_PIXEL)
                SetCursor(LoadCursor(NULL, IDC_HAND));
            else
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            return 0;
        case WM_LBUTTONDOWN:
            if (inf.y <= CRUI_TITLEBAR_PIXEL)
            {
                if (inf.x > CRUI_TITLEBAR_PIXEL * 2)
                {
                    SendMessage(hWnd, WM_SYSCOMMAND, SC_MOVE|HTCAPTION, 0);
                    pInner->drag = CRTRUE;
                }
                else if (inf.x > CRUI_TITLEBAR_PIXEL)
                {}
                else
                    pInner->preClose = CRTRUE;
            }
            else
            {
                inf.status = CRUI_STAT_DOWN | CRUI_STAT_LEFT;
                pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
                //
            }
            return 0;
        case WM_LBUTTONUP:
            if (inf.y <= CRUI_TITLEBAR_PIXEL)
            {
                if (inf.x > CRUI_TITLEBAR_PIXEL * 3)
                {}
                else if (inf.x > CRUI_TITLEBAR_PIXEL * 2)
                {}
                else if (pInner->preClose)
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
            }
            else
            {
                inf.status = CRUI_STAT_UP | CRUI_STAT_LEFT;
                pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
            }
            pInner->preClose = CRFALSE;
            pInner->drag = CRFALSE;
            return 0;
        case WM_MBUTTONDOWN:
            inf.status = CRUI_STAT_DOWN | CRUI_STAT_MIDD;
            pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
            return 0;
        case WM_MBUTTONUP:
            inf.status = CRUI_STAT_UP | CRUI_STAT_MIDD;
            pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
            return 0;
        case WM_MOUSEWHEEL:
            inf.z = GET_WHEEL_DELTA_WPARAM(wParam);
            inf.status = CRUI_STAT_SCROLL | CRUI_STAT_MIDD;
            pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
            return 0;
        case WM_SETFOCUS:
            inf.status = CRUI_STAT_UP;
            pInner->funcs[CRWINDOW_FOCUS_CB](&inf);
            return 0;
        case WM_KILLFOCUS:
            inf.status = CRUI_STAT_DOWN;
            pInner->funcs[CRWINDOW_FOCUS_CB](&inf);
            return 0;
        case WM_KEYDOWN:
            inf.status = CRUI_STAT_DOWN;
            if (pInner->funcs[CRWINDOW_KEY_CB](&inf))
                return 0;
            break;
        case WM_KEYUP:
            inf.status = CRUI_STAT_UP;
            if (pInner->funcs[CRWINDOW_KEY_CB](&inf))
                return 0;
            break;
        case WM_SIZE:
            pInner->w = inf.w;
            pInner->h = inf.h;
            pInner->resized = CRTRUE;
            if (pInner->vk) _inner_resize_vk_(pInner->vk, inf.w, inf.h);
            if (pInner->funcs[CRWINDOW_SIZE_CB](&inf))
                return 0;
            break;
        case WM_MOVE:
            pInner->drag = CRTRUE;
            break;
        default:
            break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PCRWINDOWINNER pInner;
    CRWINDOWMSG inf = {0};
    if (msg == WM_CREATE)
    {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(lpcs->lpCreateParams));
        pInner = lpcs->lpCreateParams;
        return 0;
    }
    pInner = (PCRWINDOWINNER)(CRUINT64)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (!pInner) return DefWindowProc(hWnd, msg, wParam, lParam);
    inf.window = (CRWINDOW)pInner;
    if (msg == WM_CLOSE)
    {
        if (!pInner->funcs[CRWINDOW_QUIT_CB](&inf))
            DestroyWindow(hWnd);
        CR_LOG_DBG("auto", "Close window");
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
//
static void _inner_paint_thread_(CRLVOID data, CRTHREAD idThis);
//
static void _inner_window_thread_(CRLVOID data, CRTHREAD idThis)
{
    PCRWINDOWINNER pInner = data;
    //
    DWORD style = WS_POPUP;
    // style = WS_OVERLAPPEDWINDOW;
    pInner->hWnd = CreateWindow(
        CR_WNDCLASS_NAME,
        pInner->prop->title, style,
        pInner->prop->x, pInner->prop->y,
        pInner->prop->w, pInner->prop->h,
        NULL, NULL, GetModuleHandle(NULL), pInner
    );
    UpdateWindow(pInner->hWnd);
    ShowWindow(pInner->hWnd, SW_SHOWDEFAULT);
    //
    pInner->vk = _inner_create_crvk_(pInner->hWnd, pInner->w, pInner->h, CRUI_TITLEBAR_PIXEL, CRFALSE);
    if (!pInner->vk)
    {
        CR_LOG_ERR("auto", "failed to create vulkan");
        CloseWindow(pInner->hWnd);
        pInner->onProcess = CRFALSE;
        return;
    }
    //
    MSG msg = {0};
    pInner->drag = CRFALSE;
    //
    pInner->paintThread = CRThread(_inner_paint_thread_, pInner);
    //
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
    _inner_destroy_crvk_(pInner->vk);
    //
    CRLock(windowCounterLock);
    windowCounter--;
    CRUnlock(windowCounterLock);
}

static void _inner_paint_thread_(CRLVOID data, CRTHREAD idThis)
{
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)data;
    while (pInner->onProcess)
    {
        _inner_paint_(pInner->vk);
        CRSleep(10);
    }
}

CRAPI CRINT64 CRWindowCounter(void)
{
    return windowCounter;
}

CRAPI void CRCreateWindow(PCRWindowProperties prop)
{
    if (!prop)
    {
        CR_LOG_ERR("auto", "nullptr");
        return;
    }
    prop->window = (CRWINDOW)NULL;
    PCRWINDOWINNER pInner = CRAlloc(NULL, sizeof(CRWINDOWINNER));
    if (!pInner)
    {
        CR_LOG_ERR("auto", "bad alloc");
        prop->window = (CRWINDOW)NULL;
        return;
    }
    for (int i = 0; i < CALLBACK_FUNCS_NUM; i++)
        pInner->funcs[i] = _inner_empty_callback_;
    pInner->onProcess = CRTRUE;
    pInner->preClose = CRFALSE;
    pInner->prop = prop;
    pInner->w = prop->w;
    pInner->h = prop->h;
    pInner->vk = NULL;
    //
    pInner->eventThread = CRThread(_inner_window_thread_, pInner);
    if (!pInner->eventThread) CR_LOG_ERR("auto", "failed to create window thread");
    while (!pInner->vk)
    {
        if (!pInner->onProcess)
        {
            CRAlloc(pInner, 0);
            prop->window = (CRWINDOW)NULL;
            return;
        }
        CRSleep(1);
    }
    //
    CR_LOG_DBG("auto", "Create window succeed");
    //
    //多线程操作需要加锁
    CRLock(windowCounterLock);
    windowCounter++;
    CRUnlock(windowCounterLock);
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

CRAPI void CRGetScreenSize(CRUINT32 *w, CRUINT32 *h)
{
    *w = GetSystemMetrics(SM_CXSCREEN);
    *h = GetSystemMetrics(SM_CYSCREEN);
}

#endif