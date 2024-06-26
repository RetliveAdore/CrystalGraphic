/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-06-24 16:15:22
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-06-24 16:15:28
 * @FilePath: \Crystal-Graphic\src\linux.c
 * @Description: 
 * Coptright (c) 2024 by RetliveAdore-lizaterop@gmail.com, All Rights Reserved. 
 */
#include <GraphicDfs.h>

#ifdef CR_LINUX
#include <X11/Xlib.h>
#include <CrystalThread.h>

static Display *pDisplay = NULL;
static Window rootWindow = 0;

static CRINT64 windowCounter = 0;
CRLOCK lock = NULL;

typedef struct crwindow_inner
{
    Window win;
    Atom protocols_quit;
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

static void _inner_process_msg_(PCRWINDOWINNER pInner)
{
    CRWINDOWMSG msg;
    XEvent event;
    while (pInner->onProcess)
    {
        XNextEvent(pDisplay, &event);
        if (event.xany.window != pInner->win)
        {
            XPutBackEvent(pDisplay, &event);
            CRSleep(1);
            continue;
        }
        switch (event.type)
        {
        case ClientMessage:
            if(event.xclient.data.l[0] == pInner->protocols_quit)
            {
                if (!pInner->funcs[CRWINDOW_QUIT_CB](&msg))
                {
                    XSelectInput(pDisplay, pInner->win, NoEventMask);
                    XDestroyWindow(pDisplay, pInner->win);
                    pInner->onProcess = CRFALSE;
                }
                CR_LOG_IFO("auto", "Close window");
            }
            break;
        default:
            break;
        }
        XFlush(pDisplay);
    }
    pInner->prop->window = (CRWINDOW)NULL;
    CRAlloc(pInner, 0);
    //
    CRLock(lock);
    windowCounter--;
    CRUnlock(lock);
}

static void _inner_window_thread_(CRLVOID data, CRTHREAD idThis)
{
    PCRWINDOWINNER pInner = data;
    XSetWindowAttributes swa;
    //选择关心的事件
    swa.event_mask = ExposureMask
	| KeyPressMask | ButtonPressMask
	| KeyReleaseMask | ButtonReleaseMask
	| PointerMotionMask
	| StructureNotifyMask;
    pInner->win = XCreateSimpleWindow(
        pDisplay, rootWindow,
        pInner->prop->x, pInner->prop->y,
        pInner->prop->w, pInner->prop->h,
        0, 0, 0
    );
    XMapWindow(pDisplay, pInner->win);
    pInner->protocols_quit = XInternAtom(pDisplay, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(pDisplay, pInner->win, &(pInner->protocols_quit), 1);
    CR_LOG_IFO("auto", "Create Window");
    /**/
    _inner_process_msg_(pInner);
    /**/
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
    pInner->prop = prop;
    pInner->eventThread = CRThread(_inner_window_thread_, pInner);
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
    return CRTRUE;
}

void _inner_x_uninit_()
{
    if (pDisplay) XCloseDisplay(pDisplay);
    pDisplay = NULL;
    rootWindow = 0;
}

#endif