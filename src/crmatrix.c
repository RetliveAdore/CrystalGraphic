/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-27 13:35:59
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-12 22:43:03
 * @FilePath: \CrystalGraphic\src\crmatrix.c
 * @Description: 
 * 
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <crmatrix.h>
#include <math.h>
#include <string.h>

void _inner_view_mat4_(CR_MAT4 *pMat4)
{
    for (CRUINT8 i = 0; i < 4; i++)
    {
        for (CRUINT8 j = 0; j < 4; j++)
        {
            CRPrint(CR_TC_GREEN, "%.3f\t", (*pMat4)[i][j]);
        }
        CRPrint(CR_TC_GREEN, "\n");
    }
}

void _inner_setup_mat_4_(CR_MAT4 *pMat4)
{
    memset(pMat4, 0, sizeof(CR_MAT4));
    (*pMat4)[0][0] = 1.0f;
    (*pMat4)[1][1] = 1.0f;
    (*pMat4)[2][2] = 1.0f;
    (*pMat4)[3][3] = 1.0f;
}

void _inner_set_move_xy_(CR_MAT4 *pMat4, float x, float y)
{
    memset(pMat4, 0, sizeof(CR_MAT4));
    (*pMat4)[3][0] = x;
    (*pMat4)[3][1] = y;
    //
    (*pMat4)[0][0] = 1.0f;
    (*pMat4)[1][1] = 1.0f;
    (*pMat4)[2][2] = 1.0f;
    (*pMat4)[3][3] = 1.0f;
}

void _inner_set_zoom_(CR_MAT4 *pMat4, float zoom)
{
    memset(pMat4, 0, sizeof(CR_MAT4));
    (*pMat4)[0][0] = zoom;
    (*pMat4)[1][1] = zoom;
    (*pMat4)[2][2] = zoom;
    (*pMat4)[3][3] = 1.0f;
}

static inline float _mul_i_line_j_row_(CR_MAT4 *pLeft, CR_MAT4 *pRight, CRUINT8 i, CRUINT8 j)
{
    float back = 0.0f;
    for (CRUINT8 k = 0; k < 4; k++)
        back += (*pLeft)[i][k] * (*pRight)[k][j];
    return back;
}

//变换矩阵按转换次序依次左乘
void _inner_left_mul_mat4_(CR_MAT4 *pDst, CR_MAT4 *pMat)
{
    CR_MAT4 tmp;
    for (CRUINT8 i = 0; i < 4; i++)
    {
        for (CRUINT8 j = 0; j < 4; j++)
            tmp[i][j] = _mul_i_line_j_row_(pDst, pMat, i, j);
    }
    memcpy(pDst, &tmp, sizeof(CR_MAT4));
}

void _inner_set_rotate_z_(CR_MAT4 *pMat4, float rad)
{
    memset(pMat4, 0, sizeof(CR_MAT4));
    (*pMat4)[0][0] = cos(rad);
    (*pMat4)[0][1] = sin(rad);
    //
    (*pMat4)[1][0] = -sin(rad);
    (*pMat4)[1][1] = cos(rad);
    //
    (*pMat4)[2][2] = 1.0f;
    (*pMat4)[3][3] = 1.0f;
}

void crMat4RotateZ(CR_MAT4 *pMat4, float rad)
{
    CR_MAT4 left;
    _inner_set_rotate_z_(&left, rad);
    _inner_left_mul_mat4_(pMat4, &left);
}

void crMat4MoveXY(CR_MAT4 *pMat4, float x, float y)
{
    CR_MAT4 left;
    _inner_set_move_xy_(&left, x, y);
    _inner_left_mul_mat4_(pMat4, &left);
}

void crMat4Zoom(CR_MAT4 *pMat4, float zoom)
{
    CR_MAT4 left;
    _inner_set_zoom_(&left, zoom);
    _inner_left_mul_mat4_(pMat4, &left);
}