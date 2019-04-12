/*
 * OpenslasHMD
 * Copyright (C) 2019 Joey Ferwerda
 * Distributed under the Boost 1.0 licence, see LICENSE for full text.
 */
#include "math_3d.h"

/* Resource Loader usng GLTF2*/
#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#define MATH_3D_IMPLEMENTATION
#include <stdbool.h>
#define MAX_OBJ_NUM	1024

//temporary declarations
//void mat4_4_t(void);
//void vec3_t(void);

struct renderable_object {
	char name[64];
	mat4_t obj_modelmatrix;
	vec3_t obj_colors;
	float obj_alpha;
	bool obj_active;
};

typedef struct Scene {
	mat4_t obj_modelmatrix[MAX_OBJ_NUM];
	bool obj_active[MAX_OBJ_NUM];
	float obj_alpha[MAX_OBJ_NUM];
	vec3_t obj_colors[MAX_OBJ_NUM];
} Scene;

/* 4x3 grid as in beat saber */
#define grid_x			4
#define grid_y			3
#define spawn_grid_height	1.8
#define spawn_grid_width	2.0
#define spawn_z			50
#define playerzone_grid_height	spawn_grid_height
#define playerzone_grid_width	spawn_grid_width
#define floor_level		-1.8
#define floor_thickness		0.1
#define playerzone_z		1.8
#define spawn_grid_cell_width	spawn_grid_width / grid_x
#define spawn_grid_cell_height	spawn_grid_height / grid_y

void gen_scene(Scene* sceneobj, float distance, float x, float y, int type);
void gen_object(Scene* sceneobj, float distance, float x, float y, int type, int pos);

#endif