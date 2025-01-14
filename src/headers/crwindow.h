/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-05 23:22:54
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-05 23:34:07
 * @FilePath: \CrystalGraphic\src\headers\crwindow.h
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include "crvk.h"

typedef struct crwindow_inner
{
    #ifdef CR_WINDOWS
    HWND hWnd;
    #elif defined CR_LINUX
    Window win;
    Atom protocols_quit;
    CRPOINTU delta;
    #endif
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
    //
    CRUINT64 w, h;
    CRBOOL resized;
    //
    pcrvk vk;
}CRWINDOWINNER, *PCRWINDOWINNER;

CRCODE _inner_empty_callback_(PCRWINDOWMSG msg);
