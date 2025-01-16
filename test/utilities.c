/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-15 19:24:44
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-15 22:59:32
 * @FilePath: \CrystalGraphic\test\utilities.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include "head.h"

const CRVERTEX3D_ALPHA_UV rectVertex[] = {
    -0.5f, -0.5f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 0.0f,
    0.5f, -0.5f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f,
    0.5f, 0.5f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,
    -0.5f, 0.5f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f,
};

const CRUINT32 rectIndex[] = {
    0, 1, 2,
    0, 2, 3
};

static void _inner_pixel_to_float_(CRINT64 x, CRINT64 y, float *pX, float *pY)
{
    *pX = ((float)(x * 2) / (float)game.windowProperty.w - 1.0f) * game.ratio;
    *pY = (float)(y * 2) / (float)game.windowProperty.h - 1.0f;
}

static void _delta_pixel_to_float_(CRINT64 dx, CRINT64 dy, float *pDx, float *pDy)
{
    *pDx = (float)(dx * 2) / (float)game.windowProperty.w * game.ratio;
    *pDy = (float)(dy * 2) / (float)game.windowProperty.h;
}

static CRCODE mouseCallback(PCRWINDOWMSG msg)
{
    static CRINT64 oldX, oldY;
    if (msg->status & CRUI_STAT_MOVE)
    {
        if (status.pressed)
        {
            float dx, dy;
            _delta_pixel_to_float_(msg->x - oldX, msg->y - oldY, &dx, &dy);
            status.posX = status.oldX + dx;
            status.posY = status.oldY + dy;
        }
    }
    else if (msg->status & CRUI_STAT_DOWN)
    {
        oldX = msg->x;
        oldY = msg->y;
        status.oldX = status.posX;
        status.oldY = status.posY;
        status.pressed = CRTRUE;
    }
    else if (msg->status & CRUI_STAT_UP)
    {
        status.pressed = CRFALSE;
    }
    else if ((msg->status & CRUI_STAT_MIDD) && (msg->status & CRUI_STAT_SCROLL))
    {
        status.zoom += 0.001f * (float)msg->z;
        if (status.zoom < 0.05f) status.zoom = 0.05f;
        else if (status.zoom > 1.5f) status.zoom = 1.5f;
    }
    return 0;
}

static CRCODE focusCallback(PCRWINDOWMSG msg)
{
    if (msg->status & CRUI_STAT_DOWN)
    {
        status.pressed = CRFALSE;
    }
}

CRBOOL initWindow()
{
    CRGetScreenSize(&game.screenW, &game.screenH);
    game.windowProperty.title = "PixelCrystal";
    game.windowProperty.w = game.screenW >> 1;
    game.windowProperty.h = (game.windowProperty.w << 1) / 3;
    game.windowProperty.x = game.screenW >> 2;
    game.windowProperty.y = (game.screenH - game.windowProperty.h) >> 1;
    CRCreateWindow(&game.windowProperty);
    if (!game.windowProperty.window)
        return CRFALSE;
    game.ratio = (float)game.windowProperty.w / (float)game.windowProperty.h;
    status.zoom = 1.0f;
    //设置回调
    CRSetWindowCbk(game.windowProperty.window, CRWINDOW_MOUSE_CB, mouseCallback);
    CRSetWindowCbk(game.windowProperty.window, CRWINDOW_FOCUS_CB, focusCallback);
    return CRTRUE;
}

CRBOOL initResources(char *argv_0)
{
    //创建正方形网格
    CRUINT64 vertexSize = sizeof(rectVertex);
    CRUINT64 indexSize = sizeof(rectIndex);
    //
    CRDYNAMIC vertexData = CRDyn(0);
    if (!vertexData)
    {
        CR_LOG_ERR("auto", "faied to create vertex data");
        return CRFALSE;
    }
    CRDYNAMIC indexData =CRDyn(0);
    if (!indexData)
    {
        CR_LOG_ERR("auto", "faied to create index data");
        CRFreeDyn(vertexData);
        return CRFALSE;
    }
    char *tmpData = (char*)rectVertex;
    for (CRUINT32 i = 0; i < vertexSize; i++)
        CRDynPush(vertexData, &tmpData[i], DYN_MODE_8);
    tmpData = (char*)rectIndex;
    for (CRUINT32 i = 0; i < indexSize; i++)
        CRDynPush(indexData, &tmpData[i], DYN_MODE_8);
    //
    CRAddMesh(game.windowProperty.window, 1, &vertexData, &vertexSize, &indexData, &indexSize, &game.rectMesh);
    //
    CRFreeDyn(vertexData);
    CRFreeDyn(indexData);

    //创建正方形图形实体
    if (!game.rectMesh)
    {
        CR_LOG_ERR("auto", "failed to create rect  mesh");
        return CRFALSE;
    }
    game.rectProp.angle = 0.0f;
    game.rectProp.x = 0.0f;
    game.rectProp.y = 0.0f;
    game.rectProp.uvX = 0.0f;
    game.rectProp.uvY = 0.0f;
    game.rectProp.zoom = 1.0f;
    game.rectProp.color.r = 1.0f;
    game.rectProp.color.g = 1.0f;
    game.rectProp.color.b = 1.0f;
    game.rectProp.color.a = 1.0f;
    game.rectProp.mesh = game.rectMesh;
    game.rectProp.texture = NULL;
    CRAddGraphicItem(game.windowProperty.window, 1, &game.rectItem, &game.rectProp);
    //
    //
    return CRTRUE;
}

STATUSSTRUCT status;

void mainloop()
{
    //
    game.rectProp.x = status.posX;
    game.rectProp.y = status.posY;
    game.rectProp.angle = status.angle;
    game.rectProp.zoom = status.zoom;
    CRUpdateItemProp(1, &game.rectItem, &game.rectProp);
}