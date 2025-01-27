﻿/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-08 16:57:09
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-12 17:53:34
 * @FilePath: \CrystalGraphic\src\linux.c
 * @Description: 
 * 
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crwindow.h>

#ifdef CR_LINUX
#include <X11/cursorfont.h>
// #include <GL/glx.h>

static Display *pDisplay = NULL;
static Window rootWindow = 0;
static Cursor cursors[4];
#define CR_X_CURSOR_DEFAULT 0
#define CR_X_CURSOR_HAND    1

static CRINT64 windowCounter = 0;
CRLOCK lock = NULL;

static void _inner_process_msg_(PCRWINDOWINNER pInner)
{
    CRWINDOWMSG inf;
    XEvent event;
    while (!pInner->vk) CRSleep(1);
    while (pInner->onProcess)
    {
        inf.window = pInner->win;
        XNextEvent(pDisplay, &event);
        if (event.xany.window != pInner->win)
        {
            XPutBackEvent(pDisplay, &event);
            CRSleep(1);
            continue;
        }
        switch (event.type)
        {
        case Expose:
            pInner->funcs[CRWINDOW_PAINT_CB](&inf);
            break;
        case ConfigureNotify:
            inf.w = event.xconfigure.width;
            inf.h = event.xconfigure.height - CRUI_TITLEBAR_PIXEL;
            break;
        case MotionNotify:
            if (event.xbutton.x < CRUI_TITLEBAR_PIXEL && event.xbutton.y < CRUI_TITLEBAR_PIXEL)
                XDefineCursor(pDisplay, pInner->win, cursors[CR_X_CURSOR_HAND]);
            else
            {
                XDefineCursor(pDisplay, pInner->win, cursors[CR_X_CURSOR_DEFAULT]);
                if (pInner->drag)
                    XMoveWindow(pDisplay, pInner->win, event.xmotion.x_root - pInner->delta.x, event.xmotion.y_root - pInner->delta.y);
                else
                {
                    inf.status = CRUI_STAT_MOVE;
                    inf.x = event.xbutton.x;
                    inf.y = event.xbutton.y - CRUI_TITLEBAR_PIXEL;
                    pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
                }
            }
            break;
        case ButtonPress:
            inf.x = event.xbutton.x;
            inf.y = event.xbutton.y;
            if (inf.y > CRUI_TITLEBAR_PIXEL)
            {
                inf.keycode = event.xbutton.button;
                inf.status = CRUI_STAT_DOWN;
                if (event.xbutton.button == 1)
                    inf.status |= CRUI_STAT_LEFT;
                else if (event.xbutton.button == 2)
                    inf.status |= CRUI_STAT_MIDD;
                else if (event.xbutton.button == 3)
                    inf.status |= CRUI_STAT_RIGHT;
                else if (event.xbutton.button == 4)
                {
                    inf.z = 1;
                    inf.status = CRUI_STAT_MIDD | CRUI_STAT_SCROLL;
                }
                else if (event.xbutton.button == 5)
                {
                    inf.z = -1;
                    inf.status = CRUI_STAT_MIDD | CRUI_STAT_SCROLL;
                }
                pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
            }
            else if (event.xbutton.button == 1)
			{
				if (inf.x > CRUI_TITLEBAR_PIXEL * 2)
				{
					pInner->delta.x = event.xbutton.x;
					pInner->delta.y = event.xbutton.y;
					pInner->drag = CRTRUE;
				}
				else if (inf.x > CRUI_TITLEBAR_PIXEL)
				{}
				else
					pInner->preClose = CRTRUE;
			}
            break;
        case ButtonRelease:
            inf.x = event.xbutton.x;
            inf.y = event.xbutton.y;
            if (inf.y > CRUI_TITLEBAR_PIXEL)
            {
                inf.keycode = event.xbutton.button;
                inf.status = CRUI_STAT_UP;
                if (event.xbutton.button == 1)
                    inf.status |= CRUI_STAT_LEFT;
                else if (event.xbutton.button == 2)
                    inf.status |= CRUI_STAT_MIDD;
                else if (event.xbutton.button == 3)
                    inf.status |= CRUI_STAT_RIGHT;
                pInner->funcs[CRWINDOW_MOUSE_CB](&inf);
            }
            else if (event.xbutton.button == 1)
			{
				if (inf.x > CRUI_TITLEBAR_PIXEL * 2)
				{}
				else if (inf.x > CRUI_TITLEBAR_PIXEL)
				{}
				else if (pInner->preClose)
                {
                    event.type = ClientMessage;
                    event.xany.window = pInner->win;
                    event.xclient.data.l[0] = pInner->protocols_quit;
                    XPutBackEvent(pDisplay, &event);
                }
			}
            pInner->drag = CRFALSE;
			pInner->preClose = CRTRUE;
            break;
        case KeyPress:
			inf.status = CRUI_STAT_DOWN;
			inf.keycode = event.xkey.keycode;
			pInner->funcs[CRWINDOW_KEY_CB](&inf);
			break;
		case KeyRelease:
			inf.status = CRUI_STAT_UP;
			inf.keycode = event.xkey.keycode;
			pInner->funcs[CRWINDOW_KEY_CB](&inf);
			break;
        case FocusIn:
            inf.status = CRUI_STAT_UP;
            pInner->funcs[CRWINDOW_FOCUS_CB](&inf);
            break;
        case FocusOut:
            inf.status = CRUI_STAT_DOWN;
            pInner->funcs[CRWINDOW_FOCUS_CB](&inf);
            break;
        case ClientMessage:
            if(event.xclient.data.l[0] == pInner->protocols_quit)
            {
                if (!pInner->funcs[CRWINDOW_QUIT_CB](&inf))
                {
                    XSelectInput(pDisplay, pInner->win, NoEventMask);
                    pInner->onProcess = CRFALSE;
                    XFlush(pDisplay);
                    XDestroyWindow(pDisplay, pInner->win);
                }
                CR_LOG_DBG("auto", "Close window");
            }
            break;
        default:
            break;
        }
        XFlush(pDisplay);
    }
    pInner->prop->window = (CRWINDOW)NULL;
    //
    _inner_destroy_crvk_(pInner->vk);
    //
    CRAlloc(pInner, 0);
    CRLock(lock);
    windowCounter--;
    CRUnlock(lock);
}

/*
* 无边框窗口需要的东西
*
* 这些是用来创建无边框窗口使用的
* 内部的一些数据结构
*/

#define MWM_HINTS_FUNCTIONS (1L << 0)
#define MWM_HINTS_DECORATIONS  (1L << 1)

#define MWM_FUNC_ALL (1L << 0)
#define MWM_FUNC_RESIZE (1L << 1)
#define MWM_FUNC_MOVE (1L << 2)
#define MWM_FUNC_MINIMIZE (1L << 3)
#define MWM_FUNC_MAXIMIZE (1L << 4)
#define MWM_FUNC_CLOSE (1L << 5)

#define PROP_MWM_HINTS_ELEMENTS 5
typedef struct _mwmhints
{
    uint32_t flags;
    uint32_t functions;
    uint32_t decorations;
    int32_t  input_mode;
    uint32_t status;
}MWMHints;
//
static void _inner_paint_thread_(CRLVOID data, CRTHREAD idThis);
//
static void _inner_window_thread_(CRLVOID data, CRTHREAD idThis)
{
    PCRWINDOWINNER pInner = data;

    //选择关心的事件
    long eventMask = ExposureMask
	| KeyPressMask | ButtonPressMask
	| KeyReleaseMask | ButtonReleaseMask
	| PointerMotionMask | FocusChangeMask
	| StructureNotifyMask;

    pInner->win = XCreateSimpleWindow(
        pDisplay, rootWindow,
        pInner->prop->x, pInner->prop->y,
        pInner->prop->w, pInner->prop->h,
        0, 0, 0
    );
    XSelectInput(pDisplay, pInner->win, eventMask);
    XStoreName(pDisplay, pInner->win, pInner->prop->title);
    //制作无边框窗口
    MWMHints mwmhints;
    Atom prop;
    memset(&mwmhints,0, sizeof(MWMHints));
    prop = XInternAtom(pDisplay, "_MOTIF_WM_HINTS", False);
    mwmhints.flags = MWM_HINTS_DECORATIONS;
    mwmhints.decorations = 0;
    XChangeProperty(
        pDisplay, pInner->win, prop, prop, 32,
        PropModeReplace, (unsigned char*)&mwmhints,
        PROP_MWM_HINTS_ELEMENTS
    );

    XMapWindow(pDisplay, pInner->win);
    pInner->protocols_quit = XInternAtom(pDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(pDisplay, pInner->win, &(pInner->protocols_quit), 1);
    pInner->vk = _inner_create_crvk_(pDisplay, pInner->win, pInner->w, pInner->h, CRUI_TITLEBAR_PIXEL, CRFALSE);
    if (!pInner->vk)
    {
        CR_LOG_ERR("auto", "failed to create vulkan");
        XDestroyWindow(pDisplay, pInner->win);
        pInner->onProcess = CRFALSE;
        return;
    }
    CR_LOG_DBG("auto", "Create window succeed");
    /**/
    pInner->paintThread = CRThread(_inner_paint_thread_, pInner);
    _inner_process_msg_(pInner);
    /**/
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
    pInner->drag = CRFALSE;
    pInner->preClose = CRFALSE;
    pInner->prop = prop;
    pInner->w = prop->w;
    pInner->h = prop->h;
    pInner->vk = NULL;
    pInner->eventThread = CRThread(_inner_window_thread_, pInner);
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
    XEvent event;
    event.type = ClientMessage;
    event.xany.window = pInner->win;
    event.xclient.data.l[0] = pInner->protocols_quit;
    XPutBackEvent(pDisplay, &event);
}

CRBOOL _inner_x_init_()
{
    if (pDisplay) return CRTRUE;
    pDisplay = XOpenDisplay(NULL);
    if (!pDisplay)
    {
        CR_LOG_ERR("auto", "Failed open display");
        return CRFALSE;
    }
    rootWindow = DefaultRootWindow(pDisplay);
    if (!rootWindow)
    {
        CR_LOG_ERR("auto", "Can't find default window");
        return CRFALSE;
    }
    cursors[CR_X_CURSOR_DEFAULT] = XCreateFontCursor(pDisplay, XC_left_ptr);
    cursors[CR_X_CURSOR_HAND] = XCreateFontCursor(pDisplay, XC_hand2);
    return CRTRUE;
}

void _inner_x_uninit_()
{
    if (pDisplay) XCloseDisplay(pDisplay);
    pDisplay = NULL;
    rootWindow = 0;
}

CRAPI void CRGetScreenSize(CRUINT32 *w, CRUINT32 *h)
{
    Screen *screen = DefaultScreenOfDisplay(pDisplay);
    *w = screen->width;
    *h = screen->height;
}

#endif
