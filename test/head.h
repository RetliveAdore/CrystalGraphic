/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-15 19:24:44
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-15 22:54:43
 * @FilePath: \CrystalGraphic\test\head.h
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_HEAD_H_
#define _INCLUDE_HEAD_H_

#include <CrystalGraphic.h>

//最大内存为10MB
#define MAX_MEM (1024 * 1024 * 10)
#define MAP_W 12
#define MAP_H 12
#define SPEED_F 1.0f
#define SPEED_F_MAX 10.0f
#define SPEED_UP_RATIO 2.0f

typedef struct gameStruct {
    CRMODULE core;
    CRMODULE graphic;
    CRUINT32 screenW, screenH;
    float ratio;
    CRWindowProperties windowProperty;
    //
    CRGRAPHICMESH rectMesh;
    CRITEMPROP rectProp;
    CRGRAPHICITEM rectItem;
    //
}GAMESTRUCT;
extern GAMESTRUCT game;

typedef struct statusStruct {
    CRTIMER timer;
    CRBOOL pressed;
    CRBOOL pl, pr, pu, pd;
    float oldvX, oldvY;
    float vX, vY;
    float posX, oldX;
    float posY, oldY;
    float zoom;
    float angle;
}STATUSSTRUCT;
extern STATUSSTRUCT status;

CRBOOL initWindow();
CRBOOL initResources(char *argv_0);

//功能性函数定义
void _inner_pixel_to_float_(CRINT64 x, CRINT64 y, float *pX, float *pY);
void _delta_pixel_to_float_(CRINT64 dx, CRINT64 dy, float *pDx, float *pDy);

//回调函数定义
CRCODE mouseCallback(PCRWINDOWMSG msg);
CRCODE focusCallback(PCRWINDOWMSG msg);
CRCODE keyCallback(PCRWINDOWMSG msg);

//状态循环函数定义
void mainloop();

#endif