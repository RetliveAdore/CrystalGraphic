/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-08 16:39:10
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-13 21:45:46
 * @FilePath: \CrystalGraphic\include\CrystalGraphic.h
 * @Description: 
 * 负责渲染和视窗的模块
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CRYSTAL_GRAPHIC_H_
#define _INCLUDE_CRYSTAL_GRAPHIC_H_

#include "Defs.h"

/**
 * 初始化图形模块
 * 参数1：CrystalCore模块清单（已初始化）
 * 返回值：
 * 0：初始化成功。
 */
// typedef CRCODE(*CRGRAPHICMODULEINIT)(void **coreList);
// #define CRGraphicModuleInit ((CRGRAPHICMODULEINIT)CRGraphicFunList[0])
/**
 * 获取当前视窗计数
 * 返回值：数量
 * 用于判断是否有正在使用的窗口，便于程序退出条件判断。
 */
typedef CRUINT64(*CRWINDOWCOUNTER)(void);
#define CRWindowCounter ((CRWINDOWCOUNTER)CRGraphicFunList[2])
/**
 * 创建窗口
 * 参数1：指向窗口配置结构体
 * 返回值：创建的窗口。
 * 当返回0时，创建失败。
 * 配置结构体可以用于实时更新窗体配置，在销毁窗体之前不可释放此结构体。
 */
typedef void(*CRCREATEWINDOW)(PCRWindowProperties prop);
#define CRCreateWindow ((CRCREATEWINDOW)CRGraphicFunList[4])
/**
 * 关闭（销毁）窗口
 * 参数1：要关闭的窗口。
 * 在窗口关闭时，如果设置了关闭回调，那么此回调将被执行。
 * 使用CRSetWindowCbk来设置回调。
 * 假如此回调打断了关闭，那窗口将不会被关闭（销毁），而是继续之前的状态运行。
 */
typedef void(*CRCLOSEWINDOW)(CRWINDOW window);
#define CRCloseWindow ((CRCLOSEWINDOW)CRGraphicFunList[6])
/**
 * 设置事件回调
 * 参数1：设置回调的窗体；
 * 参数2：回调类型；
 * 参数3：回调函数。
 */
typedef void(*CRSETWINDOWCBK)(CRWINDOW window, CRUINT8 type, CRWindowCallback cbk);
#define CRSetWindowCbk ((CRSETWINDOWCBK)CRGraphicFunList[8])

/**
 * 添加图形实体
 * 参数1：实体所属的窗口；
 * 参数2：实体数量
 * 参数3：指向数据的指针；
 * 参数4：数据大小（字节）；
 * 参数5：指向索引数据的指针；
 * 参数6：索引大小（字节）；
 * 参数7：用于返回实体句柄的地址；
 * 参数8：指向实体属性的指针。
 */
typedef void(*CRADDGRAPHICITEM)(
    CRWINDOW window,
    CRUINT32 itemCount,
    CRGRAPHICITEM *pItemBack,
    CRITEMPROP *pProps
);
#define CRAddGraphicItem ((CRADDGRAPHICITEM)CRGraphicFunList[10])
/**
 * 移除图形实体
 * 参数1：实体所属的窗口；
 * 参数2：实体数量；
 * 参数3：指向实体首地址集合的指针。
 */
typedef void(*CRREMOVEGRAPHICITEM)(CRWINDOW window, CRUINT32 itemCount, CRGRAPHICITEM *pItem);
#define CRRemoveGraphicItem ((CRREMOVEGRAPHICITEM)CRGraphicFunList[12])
/**
 * 更新图形实体的动态属性
 * 参数1：实体所属的窗口；
 * 参数2：实体数量；
 */
typedef void(*CRUPDATEITEMPROP)(CRUINT32 itemCount, CRGRAPHICITEM *pItems, CRITEMPROP *pProps);
#define CRUpdateItemProp ((CRUPDATEITEMPROP)CRGraphicFunList[14])
/**
 * 添加纹理
 */
typedef void(*CRADDTEXTURE)(
    CRWINDOW window,
    CRUINT32 count,
    CRUINT32 *pW,
    CRUINT32 *pH,
    CRUINT64 *pSizes,
    CRDYNAMIC *pData,
    CRGRAPHICTEXTURE *pBackTextures
);
#define CRAddTexture ((CRADDTEXTURE)CRGraphicFunList[16])
/**
 * 移除纹理
 */
typedef void(*CRREMOVETEXTURE)(CRWINDOW window, CRUINT32 count, CRGRAPHICTEXTURE *pTextures);
#define CRRemoveTexture ((CRREMOVETEXTURE)CRGraphicFunList[18])
/**
 * 添加网格
 */
typedef void(*CRADDMESH)(
    CRWINDOW window,
    CRUINT32 count,
    CRDYNAMIC *pVertexData,
    CRUINT64 *pVertexSize,
    CRDYNAMIC *pIndexData,
    CRUINT64 *pIndexSize,
    CRGRAPHICMESH *pBackMeshes
);
#define CRAddMesh ((CRADDMESH)CRGraphicFunList[20])
/**
 * 移除网格
 */
typedef void(*CRREMOVEMESH)(CRWINDOW window, CRUINT32 count, CRGRAPHICMESH *pMeshes);
#define CRRemoveMesh ((CRREMOVEMESH)CRGraphicFunList[22])
/**
 * 获取屏幕大小（像素）
 * 参数1：指向宽度数据的指针；
 * 参数2：指向高度数据的指针。
 */
typedef void(*CRGETSCREENSIZE)(CRUINT32 *w, CRUINT32 *h);
#define CRGetScreenSize ((CRGETSCREENSIZE)CRGraphicFunList[24])

/**
 * 设置全局缩放率
 * 参数1：指定窗口
 * 参数2：缩放倍率
 */
typedef void(*CRSETZOOM)(CRWINDOW window, float zoom);
#define CRSetZoom ((CRSETZOOM)CRGraphicFunList[26])

/**
 * 添加渲染组
 * 参数1：指定窗口；
 * 参数2：渲染组数量；
 * 参数3：指向渲染组的指针。
 * 当用户设定渲染组之后，将替换掉默认的渲染组，此替换不可逆。
 * 默认的渲染组将以较为随机的顺序渲染所有实体，
 * 当用户有需求时，可以自行设定渲染组，比如只需要绘制部分实体或者
 * 需要对透明物体进行排序时。
 * 每个渲染组中的实体将以从前往后的顺序进行渲染，而渲染组则按照等级从小到大进行渲染。
 * 需要注意的是，每个等级只允许有一个渲染组，如果添加渲染组到一个已经添加过的等级，
 * 将替换掉原来的渲染组。
 */
typedef void(*CRADDBLENDGROUP)(CRWINDOW window, CRUINT64 count, CRBLENDGROUP *pGroups);
#define CRAddBlendGroup ((CRADDBLENDGROUP)CRGraphicFunList[28])
/**
 * 移除渲染组
 * 参数1：指定窗口；
 * 参数2：渲染组数量；
 * 参数3：指向等级的指针。
 */
typedef void(*CRREMOVEBLENDGROUP)(CRWINDOW window, CRUINT64 count, CRUINT64 *pLevels);
#define CRRemoveBlendGroup ((CRREMOVEBLENDGROUP)CRGraphicFunList[30])

#endif