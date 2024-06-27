/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-23 00:38:41
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-27 21:37:46
 * @FilePath: \CrystalGraphic\src\windows.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <GraphicDfs.h>

#ifdef CR_WINDOWS
#include <windows.h>
#include <windowsx.h>
#include <CrystalThread.h>

static CRINT64 windowCounter = 0;
CRLOCK lock = NULL;

typedef struct crwindow_inner
{
    HWND hWnd;
    CRUINT32 fps;
    PCRWindowProperties prop;
    CRPOINTU cursor;
    //
    CRWindowCallback funcs[CALLBACK_FUNCS_NUM];
    CRBOOL onProcess;
    CRBOOL drag;
    CRBOOL preClose;
    //
    CRTHREAD eventThread;
    CRTHREAD paintThread;
}CRWINDOWINNER, *PCRWINDOWINNER;

static CRCODE _inner_empty_callback_(PCRWINDOWMSG msg)
{
    return 0;
}

static LRESULT AfterProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, PCRWINDOWINNER pInner)
{
    CRWINDOWMSG inf = {0};
    inf.window = (CRWINDOW)pInner;
    inf.x = GET_X_LPARAM(lParam);
    inf.y = GET_Y_LPARAM(lParam) - CRUI_TITLEBAR_PIXEL;
    inf.keycode = wParam & 0xff;
    inf.status = CRUI_STAT_OTHER;
    switch (msg)
    {
        case WM_PAINT:
            break;
        case WM_MOUSEMOVE:
            if (inf.y > 0)
            {
                inf.status = CRUI_STAT_MOVE;
                pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
            }
            pInner->cursor.x = inf.x;
            pInner->cursor.y = inf.y;
            return 0;
        case WM_SETCURSOR:
            if (pInner->cursor.y < 0 && pInner->cursor.x < CRUI_TITLEBAR_PIXEL)
                SetCursor(LoadCursor(NULL, IDC_HAND));
            else
                SetCursor(LoadCursor(NULL, IDC_ARROW));
            return 0;
        case WM_LBUTTONDOWN:
            if (inf.y < 0)
            {
                if (inf.x > CRUI_TITLEBAR_PIXEL * 3)
                    SendMessage(hWnd, WM_SYSCOMMAND, SC_MOVE|HTCAPTION, 0);
                else if (inf.x > CRUI_TITLEBAR_PIXEL * 2)
                {}
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
            if (inf.y < 0)
            {
                if (inf.x > CRUI_TITLEBAR_PIXEL * 3)
                {}
                else if (inf.x > CRUI_TITLEBAR_PIXEL * 2)
                {}
                else if (inf.x > CRUI_TITLEBAR_PIXEL)
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
        return;
    }
    for (int i = 0; i < CALLBACK_FUNCS_NUM; i++)
        pInner->funcs[i] = _inner_empty_callback_;
    pInner->onProcess = CRTRUE;
    pInner->drag = CRFALSE;
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