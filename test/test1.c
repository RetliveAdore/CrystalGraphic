/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-06 11:28:40
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-14 12:58:09
 * @FilePath: \CrystalGraphic\test\test1.c
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalCore.h>
#include <CrystalGraphic.h>
#include <stdlib.h>
#include <math.h>

#define TEX_W 128
#define TEX_H 128

const CRVERTEX3D_ALPHA_UV rectVertexBuffer[4] = {
    -0.5f, -0.5f, 0.0f,
    1.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 1.0f,
    //
    0.5f, -0.5f, 0.0f,
    0.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,
    //
    0.5f, 0.5f, 0.0f,
    1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f,
    //
    -0.5f, 0.5f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 0.0f
};

const CRVERTEX3D_ALPHA_UV rectVertexBufferII[4] = {
    -0.5f, -0.5f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f,
    //
    0.5f, -0.5f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,
    //
    0.5f, 0.5f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 0.0f,
    //
    -0.5f, 0.5f, 0.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 0.0f
};

const CRUINT32 indexBuffer[6] = {
    0, 1, 2,
    0, 2, 3,
};

static CRMODULE core, graphic;

static struct demo
{
    CRWindowProperties windowProp;
    CRUINT32 sw, sh, ww, wh;
    //实体句柄
    CRGRAPHICMESH meshes[2];
    CRGRAPHICTEXTURE textures[1];
    CRGRAPHICITEM items[2];
    //
    CRITEMPROP itemProps[2];
} demo;

static CRBOOL _inner_setup_module_(const char *argv_0)
{
    core = CRImport("CrystalCore.so", CRCoreFunList, argv_0);
    if (!core) return 1;
    CRMemSetup(1024 * 1024);
    CRThreadInit();
    //
    graphic = CRImport("CrystalGraphic.so", CRGraphicFunList, argv_0);
    if (!graphic)
    {
        CR_LOG_ERR("auto", "failed importing graphic module!");
        return CRFALSE;
    }
    //
    CR_LOG_DBG("auto", "graphic inited");
    return CRTRUE;
}

static void _inner_setdown_module_()
{
    CRUnload(graphic);
    CRThreadUninit();
    CRPrint(CR_TC_LIGHTMAGENTA, "memory leak check:\n");
    CRMemIterator();
    CRMemClear();
    CRUnload(core);
}

static CRBOOL _inner_create_window_()
{
    CRGetScreenSize(&demo.sw, &demo.sh);
    CR_LOG_DBG("auto", "屏幕大小（像素）：%d, %d", demo.sw, demo.sh);
    if (demo.sw > ((demo.sh << 1) / 3))
    {
        demo.wh = demo.sh >> 1;
        demo.ww = (demo.wh * 3) >> 1;
    }
    else
    {
        demo.ww = demo.sw >> 1;
        demo.wh = (demo.ww << 1) / 3;
    }
    demo.windowProp.w = demo.ww;
    demo.windowProp.h = demo.wh;
    demo.windowProp.x = (demo.sw - demo.ww) >> 1;
    demo.windowProp.y = (demo.sh - demo.wh) >> 1;
    CRCreateWindow(&demo.windowProp);
    if (!demo.windowProp.window)
        return CRFALSE;
    return CRTRUE;
}

static void _inner_prepare_data_()
{
    CRUINT64 indexSize =sizeof(indexBuffer);
    //
    CRUINT64 vertexSize = sizeof(rectVertexBuffer);
    CRUINT64 vertexSizeII = sizeof(rectVertexBuffer);
    CRDYNAMIC indexData = CRDyn(0);
    CRDYNAMIC vertexData = CRDyn(0);
    CRDYNAMIC vertexDataII = CRDyn(0);
    //
    CRUINT64 vertexSizes[2] = {vertexSize, vertexSizeII};
    CRDYNAMIC vertexDatas[2] = {vertexData, vertexDataII};
    CRUINT64 indexSizes[2] = {indexSize, indexSize};
    CRDYNAMIC indexDatas[2] = {indexData, indexData};
    //
    char *tmpIndex = (char*)indexBuffer;
    for (CRUINT64 i = 0; i < indexSize; i++)
        CRDynPush(indexData, &tmpIndex[i], DYN_MODE_8);
    //
    char *tmpVertex = (char*)rectVertexBuffer;
    for (CRUINT64 i = 0; i < vertexSize; i++)
        CRDynPush(vertexData, &tmpVertex[i], DYN_MODE_8);
    tmpVertex = (char*)rectVertexBufferII;
    for (CRUINT64 i = 0; i < vertexSizeII; i++)
        CRDynPush(vertexDataII, &tmpVertex[i], DYN_MODE_8);
    //创建实体
    CRAddMesh(demo.windowProp.window, 2, vertexDatas, vertexSizes, indexDatas, indexSizes, demo.meshes);
    //清除临时数据
    CRFreeDyn(vertexDataII);
    CRFreeDyn(vertexData);
    CRFreeDyn(indexData);

    CRDYNAMIC texData1 = CRDyn(TEX_W * TEX_H * sizeof(CR_PIXEL));
    if (!texData1) CR_LOG_ERR("auto", "bad alloc");
    srand(0);
    CRUINT8 pixel;
    for (CRUINT32 i = 0; i < TEX_W * TEX_H * sizeof(CR_PIXEL); i += 4)
    {
        pixel = rand() % 100 + 150;
        CRDynSet(texData1, &pixel, i, DYN_MODE_8);
        pixel = pixel - 5 + rand() % 5;
        CRDynSet(texData1, &pixel, i + 1, DYN_MODE_8);
        pixel = pixel - 50 - rand() % 30;
        CRDynSet(texData1, &pixel, i + 2, DYN_MODE_8);
        pixel = 255;
        CRDynSet(texData1, &pixel, i + 3, DYN_MODE_8);
    }
    CRUINT32 w = TEX_W;
    CRUINT32 h = TEX_H;
    CRUINT64 size = w * h * sizeof(CR_PIXEL);
    CRAddTexture(demo.windowProp.window, 1, &w, &h, &size, &texData1, demo.textures);
    //清除临时纹理
    CRFreeDyn(texData1);

    //创建绘制实体
    demo.itemProps[0].angle = 0.0f;
    demo.itemProps[0].x = 0.0f;
    demo.itemProps[0].y = 0.0f;
    demo.itemProps[0].uvX = 0.0f;
    demo.itemProps[0].uvY = 0.0f;
    demo.itemProps[0].color.r = 1.0f;
    demo.itemProps[0].color.g = 1.0f;
    demo.itemProps[0].color.b = 1.0f;
    demo.itemProps[0].color.a = 1.0f;
    demo.itemProps[0].zoom = 1.0f;
    demo.itemProps[0].texture = demo.textures[0];
    demo.itemProps[0].mesh = demo.meshes[1];
    //
    demo.itemProps[1].angle = 0.0f;
    demo.itemProps[1].x = 0.0f;
    demo.itemProps[1].y = 0.0f;
    demo.itemProps[1].uvX = 0.0f;
    demo.itemProps[1].uvY = 0.0f;
    demo.itemProps[1].color.r = 1.0f;
    demo.itemProps[1].color.g = 1.0f;
    demo.itemProps[1].color.b = 1.0f;
    demo.itemProps[1].color.a = 1.0f;
    demo.itemProps[1].zoom = 1.0f;
    demo.itemProps[1].texture = NULL;
    demo.itemProps[1].mesh = demo.meshes[0];
    CRAddGraphicItem(demo.windowProp.window, 2, demo.items, demo.itemProps);
}

static void _inner_update_item_(double t)
{
    float ratioX = (float)demo.windowProp.w / (float)demo.windowProp.h;
    float dx = 0.3 * sin(t * 1.1f) * ratioX;
    float dy = 0.3 * cos(t * 1.3f);
    //
    demo.itemProps[0].angle = 0.17 * t;
    demo.itemProps[0].x = dx + 0.5 * sin(t);
    demo.itemProps[0].y = dy + 0.5 * cos(t * 1.9 + 3.1415926);
    demo.itemProps[0].zoom = 0.8f + 0.2f * sin(1.4 * t);
    demo.itemProps[0].color.a = 0.7f + 0.3f * sin(t);
    demo.itemProps[0].uvX = cos(t);
    //
    demo.itemProps[1].angle = -0.7 * t;
    demo.itemProps[1].x = dx + 0.5 * cos(t);
    demo.itemProps[1].y = dy + 0.5 * sin(t + 3.1415926);
    demo.itemProps[1].zoom = 0.8f + 0.2f * cos(1.11 * t);
}

int main(int argc, char **argv)
{
    if (!_inner_setup_module_(argv[0]))
        return 1;

    if (!_inner_create_window_())
        goto FailedtoCreateWindow;

    _inner_prepare_data_();

    CRGRAPHICITEM tmpItems[2];
    tmpItems[0] = demo.items[1];
    tmpItems[1] = demo.items[0];
    CRBLENDGROUP group1 = {
        .count = 2,
        .itemList = tmpItems,
        .level = 0
    };
    CRAddBlendGroup(demo.windowProp.window, 1, &group1);

    CRTIMER timer1;
    CRTimerMark(&timer1);
    while (CRWindowCounter())
    {
        CRSleep(10);
        double t = CRTimerPeek(&timer1);
        _inner_update_item_(t);
        //
        CRUpdateItemProp(2, demo.items, demo.itemProps);
        //
        CRSetZoom(demo.windowProp.window, 1.0f / (1.2f + 0.5f * cos(0.5 * t)));
    }

    //
    _inner_setdown_module_();
    return 0;
FailedtoCreateWindow:
    _inner_setdown_module_();
    return 2;
}