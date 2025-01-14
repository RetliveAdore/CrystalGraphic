/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-08 16:36:22
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-13 13:09:24
 * @FilePath: \CrystalGraphic\core\CrystalGraphic.c
 * @Description: 
 * 清单文件
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */

#include <CrystalGraphic.h>

static void* CRGraphicFunListArr[] =
{
    _cr_inner_do_nothing_, "CRGraphicModuleInit",//0
    _cr_inner_do_nothing_, "CRWindowCounter",    //2
    _cr_inner_do_nothing_, "CRCreateWindow",     //4
    _cr_inner_do_nothing_, "CRCloseWindow",      //6
    _cr_inner_do_nothing_, "CRSetWindowCbk",     //8
    //
    _cr_inner_do_nothing_, "CRAddGraphicItem",   //10
    _cr_inner_do_nothing_, "CRRemoveGraphicItem",//12
    _cr_inner_do_nothing_, "CRUpdateItemProp",   //14
    _cr_inner_do_nothing_, "CRAddTexture",       //16
    _cr_inner_do_nothing_, "CRRemoveTexture",    //18
    _cr_inner_do_nothing_, "CRAddMesh",          //20
    _cr_inner_do_nothing_, "CRRemoveMesh",       //22
    //
    _cr_inner_do_nothing_, "CRGetScreenSize",    //24
    _cr_inner_do_nothing_, "CRSetZoom",          //26
    //
    _cr_inner_do_nothing_, "CRAddBlendGroup",    //28
    _cr_inner_do_nothing_, "CRRemoveBlendGroup", //30
    0
};

CRAPI void** CRGraphicFunList = CRGraphicFunListArr;