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
    CRBOOL pressed;
    float posX, oldX;
    float posY, oldY;
    float zoom;
    float angle;
}STATUSSTRUCT;
extern STATUSSTRUCT status;

CRBOOL initWindow();
CRBOOL initResources(char *argv_0);

void mainloop();

#endif