/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2025-01-02 22:49:27
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2025-01-12 22:27:36
 * @FilePath: \CrystalGraphic\src\headers\crmatrix.h
 * @Description: 
 * 
 * Copyright (c) 2025 by lizaterop@gmail.com, All Rights Reserved. 
 */
#ifndef _INCLUDE_CRMATRIX_H_
#define _INCLUDE_CRMATRIX_H_

#include <Defs.h>

//

void _inner_view_mat4_(CR_MAT4 *pMat4);
void _inner_setup_mat_4_(CR_MAT4 *pMat4);
//
void _inner_set_rotate_z_(CR_MAT4 *pMat4, float rad);
void _inner_set_move_xy_(CR_MAT4 *pMat4, float x, float y);
void _inner_set_zoom_(CR_MAT4 *pMat4, float zoom);
//
void _inner_left_mul_mat4_(CR_MAT4 *pDst, CR_MAT4 *pMat);
//
//下面的操作会自动生成矩阵然后左乘到现有矩阵

void crMat4RotateZ(CR_MAT4 *pMat4, float rad);
void crMat4MoveXY(CR_MAT4 *pMat4, float x, float y);
void crMat4Zoom(CR_MAT4 *pMat4, float zoom);

#endif