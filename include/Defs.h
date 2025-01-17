/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-12 23:21:33
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-15 22:22:58
 * @FilePath: \CrystalGraphic\include\Defs.h
 * @Description: 
 * 
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CRYSTAL_GRAPHIC_DFS_H_
#define _INCLUDE_CRYSTAL_GRAPHIC_DFS_H_

#include <CrystalCore.h>

extern void **CRGraphicFunList;

#define CR_WNDCLASS_NAME "CrystalWindow"
#define CRUI_TITLEBAR_PIXEL 25

typedef CRLVOID CRGRAPHICMESH;
typedef CRLVOID CRGRAPHICTEXTURE;
typedef CRLVOID CRGRAPHICITEM;

typedef struct crcoloru
{
    CRUINT8 r;
    CRUINT8 g;
    CRUINT8 b;
    CRUINT8 a;
}CRCOLORU;

typedef struct cr_pointu
{
    CRINT64 x, y;
}CRPOINTU;

typedef struct crwindow_properties
{
    CRCHAR* title;
    CRINT64 x, y;
    CRINT64 w, h;
    //
    CRBOOL modified;
    //
    CRWINDOW window;
}CRWindowProperties, *PCRWindowProperties;

typedef struct crwindow_msg
{
    CRWINDOW window;
    union
    {
        CRINT64 x;
        CRUINT64 w;
        CRINT64 z;
    };
    union
    {
        CRINT64 y;
        CRUINT64 h;
    };
    CRCHAR keycode;
    //CRW_STAT_XX
    CRUINT8 status;
    CRLVOID lp;
}CRWINDOWMSG, *PCRWINDOWMSG;
#define CRUI_STAT_OTHER  0x00
#define CRUI_STAT_UP     0x01
#define CRUI_STAT_DOWN   0x02
#define CRUI_STAT_MOVE   0x04
#define CRUI_STAT_SCROLL 0x08
#define CRUI_STAT_LEFT   0x10
#define CRUI_STAT_MIDD   0x20
#define CRUI_STAT_RIGHT  0x40

#define CRWINDOW_QUIT_CB   0
#define CRWINDOW_PAINT_CB  1
#define CRWINDOW_MOUSE_CB  2
#define CRWINDOW_KEY_CB    3
#define CRWINDOW_FOCUS_CB  4
#define CRWINDOW_SIZE_CB   5
#define CRWINDOW_MOVE_CB   6
#define CRWINDOW_ENTITY_CB 7
#define CALLBACK_FUNCS_NUM 8

typedef struct cr_pos_2d{
    float x;
    float y;
}CRPOS2D;

typedef struct cr_pos_3d{
    float x;
    float y;
    float z;
}CRPOS3D;

typedef struct cr_color_rgba{
    float r;
    float g;
    float b;
    float a;
}CRCOLOR_RGBA;

typedef struct cr_vertex_3d_alpha_uv{
    CRPOS3D pos;
    CRCOLOR_RGBA color;
    CRPOS2D uv;
}CRVERTEX3D_ALPHA_UV;

//

typedef struct cr_item_properties{
    float angle;
    float x, y;
    float uvX, uvY;
    float zoom;
    CRCOLOR_RGBA color;
    CRGRAPHICMESH mesh;
    CRGRAPHICTEXTURE texture;
}CRITEMPROP;

typedef struct cr_global_properties{
    float anglr;
    float x, y;
    float zoom;
    CRCOLOR_RGBA color;
}CRGLOBALPROP;

typedef struct cr_blend_group{
    CRUINT64 level;
    CRUINT64 count;
    CRGRAPHICITEM *itemList;
}CRBLENDGROUP;

#pragma pack(push)
#pragma pack(1)
typedef struct {
    CRUINT8 r;
    CRUINT8 g;
    CRUINT8 b;
    CRUINT8 a;
}CR_PIXEL;

#pragma pack(4)
typedef float CR_VEC4[4];
typedef float CR_VEC2[2];

#pragma pack(16)
typedef CR_VEC4 CR_MAT4[4];

typedef struct cr_ubo {
    CR_MAT4 model;
    CR_VEC4 color;
    CR_VEC2 uvPos;
} CR_UBO;

typedef struct cr_global_ubo {
    CR_MAT4 view;
    CR_VEC4 ratio;
    CR_VEC4 colorFlag;
} CR_GLOBAL_UBO;

#pragma pack(pop)

#define GET_INSTANCE_COUNT(size, array) (size / sizeof(array))
#define CR_ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


/**
 * 由事件触发的回调函数
 */
typedef CRCODE(*CRWindowCallback)(PCRWINDOWMSG msg);

//键码定义
#ifdef CR_WINDOWS

#define CR_VK_SPACE 0x20

#define CR_VK_LEFT  0x25
#define CR_VK_UP    0x26
#define CR_VK_RIGHT 0x27
#define CR_VK_DOWN  0x28

#define CR_VK_ESC   0x1b

#define CR_VK_A 'A'
#define CR_VK_B
#define CR_VK_C
#define CR_VK_D 'D'
#define CR_VK_E
#define CR_VK_F
#define CR_VK_G
#define CR_VK_H
#define CR_VK_I
#define CR_VK_J
#define CR_VK_K
#define CR_VK_L
#define CR_VK_M
#define CR_VK_N
#define CR_VK_O
#define CR_VK_P
#define CR_VK_Q
#define CR_VK_R
#define CR_VK_S 'S'
#define CR_VK_T
#define CR_VK_U
#define CR_VK_V
#define CR_VK_W 'W'
#define CR_VK_X
#define CR_VK_Y
#define CR_VK_Z

#elif defined CR_LINUX

#define CR_VK_SPACE 0x41

#define CR_VK_LEFT  0x71
#define CR_VK_UP    0x6f
#define CR_VK_RIGHT 0x72
#define CR_VK_DOWN  0x74

#define CR_VK_ESC   0x09

#define CR_VK_A 0x26
#define CR_VK_B 0x38
#define CR_VK_C 0x36
#define CR_VK_D 0x28
#define CR_VK_E 0x1a
#define CR_VK_F 0x29
#define CR_VK_G
#define CR_VK_H
#define CR_VK_I
#define CR_VK_J
#define CR_VK_K
#define CR_VK_L
#define CR_VK_M
#define CR_VK_N
#define CR_VK_O
#define CR_VK_P
#define CR_VK_Q
#define CR_VK_R
#define CR_VK_S 0x27
#define CR_VK_T
#define CR_VK_U
#define CR_VK_V
#define CR_VK_W 0x19
#define CR_VK_X
#define CR_VK_Y
#define CR_VK_Z 0x34

#endif

#endif