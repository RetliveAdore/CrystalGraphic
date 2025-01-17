/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-26 10:00:59
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-12 21:55:01
 * @FilePath: \CrystalGraphic\src\crwindow.c
 * @Description: 
 * 
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include "headers/crwindow.h"

CRCODE _inner_empty_callback_(PCRWINDOWMSG msg)
{
    return 0;
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

CRAPI void CRSetGlobalProp(CRWINDOW window, CRGLOBALPROP *pProp)
{
    if (!window) return;
    PCRWINDOWINNER pInner = (PCRWINDOWINNER)window;
    if (!pInner->vk) return;
    _inner_set_vk_global_prop_(pInner->vk, pProp);
}