#include "gameobjects.h"
#include <stdlib.h>

float randef()
{
	return (float)rand() / (float)RAND_MAX;
}

void gen_scene(Scene* sceneobj, float distance, float x, float y, int type)
{
	for (int i = 0; i < 1024; i++) {
		if (!sceneobj->obj_active[i]) {
			gen_object(sceneobj, distance, x, y, type, i);
			return;
		}
	}
}

/* generate new cube at distance distance, with grid position x, y (from bottom left) */
void gen_object(Scene* sceneobj, float distance, float x, float y, int type, int pos)
{
	//printf("Checking pos %i\n", pos);
	sceneobj->obj_modelmatrix[pos] = m4_identity();
	//obj_modelmatrix[i] = m4_mul(obj_modelmatrix[i], m4_rotation(degreesToRadians(i * 20), vec3(0, 1, 0)));
	// transpose from starting point
	sceneobj->obj_modelmatrix[pos] = m4_mul(sceneobj->obj_modelmatrix[pos], m4_translation(vec3(x, y, 0)));
	sceneobj->obj_modelmatrix[pos] = m4_mul(sceneobj->obj_modelmatrix[pos], m4_translation(vec3(-(spawn_grid_width / 2), floor_level, -distance)));
	//obj_modelmatrix[i] = m4_mul(obj_modelmatrix[i], m4_rotation(degreesToRadians(randf() * 360), vec3(randf(), randf(), randf())));
	switch (type) {
	case 0:
		sceneobj->obj_colors[pos] = vec3(1.0, 0.0, 0.0);
		break;
	case 1:
		sceneobj->obj_colors[pos] = vec3(0.0, 0.0, 1.0);
		break;
	case 2:
		sceneobj->obj_colors[pos] = vec3(randef(), randef(), randef());
		break;
	}

	/* change to proper size */
	sceneobj->obj_modelmatrix[pos] = m4_mul(sceneobj->obj_modelmatrix[pos], m4_scaling(vec3(0.5, 0.5, 0.5)));
	sceneobj->obj_alpha[pos] = 1.0; //randf();
	sceneobj->obj_active[pos] = 1;
	return;
}
