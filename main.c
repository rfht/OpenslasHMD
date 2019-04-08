/*
 * OpenslasHMD - open source ryhthm block slasher
 * Copyright (C) 2019 Thomas Frohwein 11335318+rfht@users.noreply.github.com
 * Distributed under the ISC license. See LICENSE.
 */


#include <openhmd.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include "gl.h"
#define MATH_3D_IMPLEMENTATION
#include "math_3d.h"
#include "resourceloader.h"
#include "maploader.h"

#define degreesToRadians(angleDegrees) ((angleDegrees) * M_PI / 180.0)
#define radiansToDegrees(angleRadians) ((angleRadians) * 180.0 / M_PI)

#define OVERSAMPLE_SCALE 2.0
#define MAX_OBJ_NUM	1024
#define MAX_SONG_NOTES		32000		// Shrek entire movie: 11,765 notes
#define MAX_SONG_OBSTACLES	32000

void GLAPIENTRY
gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                  GLsizei length, const GLchar* message, const void* userParam)
{
	fprintf(stderr, "GL DEBUG CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
	        type, severity, message);
}

float randf()
{
	return (float)rand() / (float)RAND_MAX;
}

mat4_t obj_modelmatrix[MAX_OBJ_NUM];
vec3_t obj_colors[MAX_OBJ_NUM];
float obj_alpha[MAX_OBJ_NUM];
bool obj_active[MAX_OBJ_NUM];

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

#define spawn_grid_lower_left		vec3(-(spawn_grid_width / 2), floor_level, - spawn_z)
#define playerzone_grid_lower_left	vec3(-(playerzone_grid_width / 2), floor_level, -playerzone_z)

/* http://lazyfoo.net/tutorials/SDL/21_sound_effects_and_music/index.php */
Mix_Music *gMusic = NULL;

/* generate new cube at distance distance, with grid position x, y (from bottom left) */
void gen_object(float distance, float x, float y, int type)
{
	for (int i = 0; i < MAX_OBJ_NUM; i++) {
		if (!obj_active[i]) {
			obj_modelmatrix[i] = m4_identity();
			//obj_modelmatrix[i] = m4_mul(obj_modelmatrix[i], m4_rotation(degreesToRadians(i * 20), vec3(0, 1, 0)));
			// transpose from starting point
			obj_modelmatrix[i] = m4_mul(obj_modelmatrix[i], m4_translation(vec3(x, y, 0)));
			obj_modelmatrix[i] = m4_mul(obj_modelmatrix[i], m4_translation(vec3(-(spawn_grid_width / 2), floor_level, -distance)));
			//obj_modelmatrix[i] = m4_mul(obj_modelmatrix[i], m4_rotation(degreesToRadians(randf() * 360), vec3(randf(), randf(), randf())));
			switch (type) {
				case 0:
					obj_colors[i] = vec3(1.0, 0.0, 0.0);
					break;
				case 1:
					obj_colors[i] = vec3(0.0, 0.0, 1.0);
					break;
				case 2:
					obj_colors[i] = vec3(randf(), randf(), randf());
					break;
				//default:
					/* ? need to error in this case? */
			}
			//obj_colors[i] = vec3(randf(), randf(), randf());
			
			/* change to proper size */
			obj_modelmatrix[i] = m4_mul(obj_modelmatrix[i], m4_scaling(vec3(0.5, 0.5, 0.5)));
			obj_alpha[i] = 1.0; //randf();
			obj_active[i] = 1;
			return;
		}
	}
}

void grid_spawn(float lineIndex, float lineLayer, int type, float distance)
{
	float x = lineIndex * spawn_grid_cell_width;
	float y = -floor_level + (lineLayer * spawn_grid_cell_height);
	gen_object(distance, x, y, type);
}

// TODO: Is this framerate-dependent??
void move_all_objects(float speed)
{
	for (int i = 0; i < MAX_OBJ_NUM; i++) {
		if (obj_active[i]) {
			obj_modelmatrix[i] = m4_mul(obj_modelmatrix[i], m4_translation(vec3(0, 0, speed)));
		}
	}
}

void destroy_object(int objnum)
{
	// TODO: implement/use
	// set obj_active[objnum] to FALSE or 0
}

void draw_cubes(GLuint shader)
{
	int modelLoc = glGetUniformLocation(shader, "model");
	int colorLoc = glGetUniformLocation(shader, "uniformColor");
	for(int i = 0; i < MAX_OBJ_NUM; i ++) {
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*) obj_modelmatrix[i].m);
		glUniform4f(colorLoc, obj_colors[i].x, obj_colors[i].y, obj_colors[i].z, obj_alpha[i]);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	// floor is 10x10m, 0.1m thick
	mat4_t floor = m4_identity();
	floor = m4_mul(floor, m4_scaling(vec3(10, floor_thickness, 10)));
	// TODO - WORKAROUND: 
	// move the floor to -1.8m height if the HMD tracker sits at zero
	floor = m4_mul(floor, m4_translation(vec3(0, floor_level - floor_thickness, 0)));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*) floor.m);
	glUniform4f(colorLoc, 0, .4f, .25f, .9f);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}
/*
void draw_controllers(GLuint shader, ohmd_device *lc, ohmd_device *rc)
{
	int modelLoc = glGetUniformLocation(shader, "model");
	int colorLoc = glGetUniformLocation(shader, "uniformColor");

	mat4_t lcmodel;
	ohmd_device_getf(lc, OHMD_GL_MODEL_MATRIX, (float*) lcmodel.m);
	lcmodel = m4_mul(lcmodel, m4_scaling(vec3(0.03, 0.03, 0.1)));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*) lcmodel.m);
	glUniform4f(colorLoc, 1.0, 0.0, 0.0, 1.0);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	mat4_t rcmodel;
	ohmd_device_getf(rc, OHMD_GL_MODEL_MATRIX, (float*) rcmodel.m);
	rcmodel = m4_mul(rcmodel, m4_scaling(vec3(0.03, 0.03, 0.1)));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (float*) rcmodel.m);
	glUniform4f(colorLoc, 0.0, 1.0, 0.0, 1.0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}*/

long clock_to_msec(clock_t duration)
{
	/* https://stackoverflow.com/questions/17167949/how-to-use-timer-in-c */
	/* TODO: Magic number - this should be 1000, why not here??? */
	return (long)(duration * 3520.0 / CLOCKS_PER_SEC);
}

void spawn_note(struct note snote, float distance)
{
	printf("spawning note at distance: %.2f\n", distance);
	grid_spawn(snote.lineIndex, snote.lineLayer, snote.type,
			distance);
}

void print_help()
{
	printf("|=========================================================|\n");
	printf("| OpenslasHMD Help                                        |\n");
	printf("|=========================================================|\n");
	printf("| -chart mysong/mysong.json  - json chart of the song     |\n");
	printf("| -song mysong/mysong.ogg    - audio file of the song     |\n");
	printf("|                                                         |\n");
	printf("| - Example -                                             |\n"); 
	printf("| ./OpenslasHMD -chart test/easy.json -song test/song.ogg |\n");
	printf("|=========================================================|\n");
	printf("| Github: https://github.com/rfht/OpenslasHMD             |\n");
	printf("|---------------------------------------------------------|\n");
}

int main(int argc, char** argv)
{
	/* intialize SDL_mixer */
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		printf("SDL_mixer failed init. Error: %s\n", Mix_GetError());
	}

	int hmddev = -1;
	int lcontrollerdev = -1;
	int rcontrollerdev = -1;
	char* song;
	char* chart;

	if (argc <= 1)
	{
		print_help();
		exit(0);
	}
	/* User can set devices to use with flags. There is no validation that -lc is actually a left controller*/
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-hmd") == 0 && i < argc - 1) {
			hmddev = strtol(argv[i+1], NULL, 10);
			printf("Using HMD device %d\n", hmddev);
		}
		if (strcmp(argv[i], "-lc") == 0 && i < argc - 1) {
			lcontrollerdev = strtol(argv[i+1], NULL, 10);
			printf("Using left controller device %d\n", lcontrollerdev);
		}
		if (strcmp(argv[i], "-rc") == 0 && i < argc - 1) {
			rcontrollerdev = strtol(argv[i+1], NULL, 10);
			printf("Using right controller device %d\n", rcontrollerdev);
		}
		if (strcmp(argv[i], "-song") == 0 && i < argc - 1) {
			song = argv[i+1];
			printf("Song: %s\n", song);
		}
		if (strcmp(argv[i], "-chart") == 0 && i < argc - 1) {
			chart = argv[i+1];
			printf("Chart %s\n", chart);
		}
		if (strcmp(argv[i], "?") == 0 || strcmp(argv[i], "-?") == 0 || strcmp(argv[i], "--?") == 0)
		{
			print_help();
			exit(0);
		}
	}

	int hmd_w, hmd_h;

	ohmd_context* ctx = ohmd_ctx_create();
	int num_devices = ohmd_ctx_probe(ctx);
	if(num_devices < 0) {
		printf("failed to probe devices: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}

	/* For all devices that are not set by the user, choose first available */
	for(int i = 0; i < num_devices; i++) {
		int device_class = 0, device_flags = 0;
		ohmd_list_geti(ctx, i, OHMD_DEVICE_CLASS, &device_class);
		ohmd_list_geti(ctx, i, OHMD_DEVICE_FLAGS, &device_flags);
		if (hmddev == -1 && (device_class == OHMD_DEVICE_CLASS_HMD)) {
			hmddev = i;
		}
		if (lcontrollerdev == -1 && (device_class == OHMD_DEVICE_CLASS_CONTROLLER) && (device_flags & OHMD_DEVICE_FLAGS_LEFT_CONTROLLER)) {
			lcontrollerdev = i;
		}
		if (rcontrollerdev == -1 && (device_class == OHMD_DEVICE_CLASS_CONTROLLER) && (device_flags & OHMD_DEVICE_FLAGS_RIGHT_CONTROLLER)) {
			rcontrollerdev = i;
		}
	}

	ohmd_device_settings* settings = ohmd_device_settings_create(ctx);

	// If OHMD_IDS_AUTOMATIC_UPDATE is set to 0, ohmd_ctx_update() must be called at least 10 times per second.
	// It is enabled by default.

	int auto_update = 1;
	ohmd_device_settings_seti(settings, OHMD_IDS_AUTOMATIC_UPDATE, &auto_update);



	ohmd_device* hmd = ohmd_list_open_device_s(ctx, hmddev, settings);
	if(!hmd){
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}

	ohmd_device* lc = ohmd_list_open_device_s(ctx, lcontrollerdev, settings);
	if(!lc){
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}
	
	ohmd_device* rc = ohmd_list_open_device_s(ctx, rcontrollerdev, settings);
	if(!rc){
		printf("failed to open device: %s\n", ohmd_ctx_get_error(ctx));
		return 1;
	}
	
	printf("HMD:\n");
	printf("\t%s\n", ohmd_list_gets(ctx, hmddev, OHMD_PRODUCT));

	printf("Left controller:\n");
	printf("\t%s\n", ohmd_list_gets(ctx, lcontrollerdev, OHMD_PRODUCT));

	printf("Right controller:\n");
	printf("\t%s\n", ohmd_list_gets(ctx, rcontrollerdev, OHMD_PRODUCT));

	ohmd_device_geti(hmd, OHMD_SCREEN_HORIZONTAL_RESOLUTION, &hmd_w);
	ohmd_device_geti(hmd, OHMD_SCREEN_VERTICAL_RESOLUTION, &hmd_h);
	float ipd;
	ohmd_device_getf(hmd, OHMD_EYE_IPD, &ipd);
	float viewport_scale[2];
	float distortion_coeffs[4];
	float aberr_scale[3];
	float sep;
	float left_lens_center[2];
	float right_lens_center[2];
	//viewport is half the screen
	ohmd_device_getf(hmd, OHMD_SCREEN_HORIZONTAL_SIZE, &(viewport_scale[0]));
	viewport_scale[0] /= 2.0f;
	ohmd_device_getf(hmd, OHMD_SCREEN_VERTICAL_SIZE, &(viewport_scale[1]));
	//distortion coefficients
	ohmd_device_getf(hmd, OHMD_UNIVERSAL_DISTORTION_K, &(distortion_coeffs[0]));
	ohmd_device_getf(hmd, OHMD_UNIVERSAL_ABERRATION_K, &(aberr_scale[0]));
	//calculate lens centers (assuming the eye separation is the distance between the lens centers)
	ohmd_device_getf(hmd, OHMD_LENS_HORIZONTAL_SEPARATION, &sep);
	ohmd_device_getf(hmd, OHMD_LENS_VERTICAL_POSITION, &(left_lens_center[1]));
	ohmd_device_getf(hmd, OHMD_LENS_VERTICAL_POSITION, &(right_lens_center[1]));
	left_lens_center[0] = viewport_scale[0] - sep/2.0f;
	right_lens_center[0] = sep/2.0f;
	//assume calibration was for lens view to which ever edge of screen is further away from lens center
	float warp_scale = (left_lens_center[0] > right_lens_center[0]) ? left_lens_center[0] : right_lens_center[0];
	float warp_adj = 1.0f;

	ohmd_device_settings_destroy(settings);

	gl_ctx gl;
	GLuint VAOs[2];
	GLuint appshader;
	init_gl(&gl, hmd_w, hmd_h, VAOs, &appshader);

	//glEnable(GL_DEBUG_OUTPUT);
	//glDebugMessageCallback(gl_debug_callback, 0);

	int eye_w = hmd_w/2*OVERSAMPLE_SCALE;
	int eye_h = hmd_h*OVERSAMPLE_SCALE;

	GLuint textures[2];
	GLuint framebuffers[2];
	GLuint depthbuffers[2];
	for (int i = 0; i < 2; i++)
		create_fbo(eye_w, eye_h, &framebuffers[i], &textures[i], &depthbuffers[i]);

	SDL_ShowCursor(SDL_DISABLE);

	const char* vertex;
	ohmd_gets(OHMD_GLSL_330_DISTORTION_VERT_SRC, &vertex);
	const char* fragment;
	ohmd_gets(OHMD_GLSL_330_DISTORTION_FRAG_SRC, &fragment);
	GLuint distortionshader = compile_shader(vertex, fragment);

	/* load map and music */
	struct note *_notes = malloc(sizeof(struct note) * MAX_SONG_NOTES);
	struct obstacle *_obstacles = malloc(sizeof(struct obstacle) * MAX_SONG_OBSTACLES);
	init_map(chart, _notes, _obstacles);
	printf("_version: %s\n", _version);

	gMusic = Mix_LoadMUS(song);
	if (gMusic == NULL)
	{
		printf("Failed to load music. Error: %s\n", Mix_GetError());
	}

	bool done = false;
	bool crosshair_overlay = false;
	long song_time;

	/* variables to not read full arrays on each iteration
	 * Strategy: whenever a block/obstacle/event is spawned, move progress index past it.
	 * On each iteration, start on progress index for notes/obstacles/
	 * events so that those that have already been spawned are skipped.
	 * Only read array elements until one is found with spawn time in the future
	 *
	 * ASSUMPTION: notes/obstacles/events are sorted in ascending order based on
	 * TODO: verify this assumption; maybe somewhere in init_map()... ?
	 */

	int next_note = 0;
	int next_obst = 0;
	int next_event = 0; // TODO: array for events hasn't been implemented yet
	int i;

	/* set up variables for timing tracking */
	printf("_beatsPerMinute: %d\n", _beatsPerMinute);
	int beat_length = (int)((60 * 1000) / _beatsPerMinute); // length of each beat in msec
	printf("beat_length: %d milliseconds\n", beat_length);

	/* create array note_spawn_time */
	long note_spawn_time[num_notes];
	for (i = 0; i < num_notes; i++) {
		note_spawn_time[i] = (long)(_notes[i].time * beat_length);
		printf("%.2f, ", note_spawn_time[i] / 1000.0);
	}
	printf("\n");

	int offset = -3650;	// TODO: magic number

	printf("first note spawn time: %ld\n", note_spawn_time[0]);
	printf("spawn_z: %d, note_spawn_time + offset: %ld\n",
			spawn_z, note_spawn_time[next_note] + offset);

	/* spawn any notes that should already be on the way */
	/*
	while (note_spawn_time[next_note] + offset < 0
			&& next_note < num_notes)
	{
		printf("early note_spawn time %d: %ld, position: %ld\n",
				next_note, note_spawn_time[next_note],
				(long)(((float)spawn_z * (((float)note_spawn_time[next_note] +
				(float)offset)
				/ (float)offset))));
		spawn_note(_notes[next_note], (long)((float)spawn_z *
				(((float)note_spawn_time[next_note] + (float)offset) / (float)offset)));
		next_note++;
	}
	*/

	clock_t start_time = clock();
	/* start Music */
	Mix_PlayMusic(gMusic, -1);
	// DEBUG
	int iter = 0;

	load_gltf("resources/ArrowCube.gltf");

	while(!done){
		/* update current time */
		/* TODO: account for pauses once implemented */
		song_time = clock_to_msec(clock() - start_time);

		iter++;
		if (iter % 1000 == 0)
			printf("song_time: %.1f\n", song_time / 1000.0);

		ohmd_ctx_update(ctx);
		move_all_objects(0.5);

		/* check if notes/obstacles/events need to spawn */
		// TODO: fix the temporary correction factor of 3 in this while loop
		while (note_spawn_time[next_note] + offset < song_time  //* 3.55
				&& next_note < num_notes)
		{
			float relPos = ((float)spawn_z * (
				1 - ((((float)note_spawn_time[next_note] + (float)offset) - (float)song_time)
				/ (float)offset)));
			printf("relPos: %.1f, from %.1f / %.1f * %d\n",
				relPos, ((float)note_spawn_time[next_note] + (float)offset) - (float)song_time, (float)offset, spawn_z);
			spawn_note(_notes[next_note], relPos);
			printf("note spawned at song_time: %.1f s\n", song_time / 1000.0);
			next_note++;
		}

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_KEYDOWN){
				switch(event.key.keysym.sym){
				case SDLK_ESCAPE:
					done = true;
					break;
				case SDLK_b:
					grid_spawn(0, 0, 0, spawn_z);
					grid_spawn(4, 3, 1, spawn_z);
					//gen_object(50.0, 0.0, 0.0);
					break;
				case SDLK_m:
					printf("song_time: %.1f s, next note_spawn_time: %.1f s\n",
							song_time / 1000.0,
							note_spawn_time[next_note] / 1000.0);
					break;
				default:
					break;
				}
			}
		}

		glViewport(0, 0, eye_w, eye_h);
		glScissor(0, 0, eye_w, eye_h);

		glUseProgram(appshader);
		for (int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[i], 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthbuffers[i], 0);

			glClearColor(0.0, 0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(VAOs[0]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_SCISSOR_TEST);

			float projectionmatrix[16];
			ohmd_device_getf(hmd, OHMD_LEFT_EYE_GL_PROJECTION_MATRIX, projectionmatrix);
			glUniformMatrix4fv(glGetUniformLocation(appshader, "proj"), 1, GL_FALSE, projectionmatrix);

			float viewmatrix[16];
			ohmd_device_getf(hmd, OHMD_LEFT_EYE_GL_MODELVIEW_MATRIX, viewmatrix);
			glUniformMatrix4fv(glGetUniformLocation(appshader, "view"), 1, GL_FALSE, viewmatrix);

			draw_cubes(appshader);
			//draw_controllers(appshader, lc, rc);
		}

		// draw the textures to the screen, applying the distortion shader
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glUseProgram(distortionshader);
		glUniform1i(glGetUniformLocation(distortionshader, "warpTexture"), 0);
		glUniform2fv(glGetUniformLocation(distortionshader, "ViewportScale"), 1, viewport_scale);
		glUniform3fv(glGetUniformLocation(distortionshader, "aberr"), 1, aberr_scale);
		glUniform1f(glGetUniformLocation(distortionshader, "WarpScale"), warp_scale*warp_adj);
		glUniform4fv(glGetUniformLocation(distortionshader, "HmdWarpParam"), 1, distortion_coeffs);
		// The shader is set up to render starting at the middle of the viewport
		// and half its size. Move it to the bottom left and double its size.
		float mvp[16] = {
			2.0, 0.0, 0.0, 0.0,
			0.0, 2.0, 0.0, 0.0,
			0.0, 0.0, 2.0, 0.0,
			-1.0, -1.0, 0.0, 1.0
		};
		glUniformMatrix4fv(glGetUniformLocation(distortionshader, "mvp"), 1, GL_FALSE, mvp);

		for (int i = 0; i < 2; i++)
		{
			if (i == 0) {
				glViewport(0, 0, hmd_w / 2, hmd_h);
				glScissor(0, 0, hmd_w / 2, hmd_h);
				glClearColor(0.5, 0, 0.0, 1.0);
			} else {
				glViewport(hmd_w / 2, 0, hmd_w / 2, hmd_h);
				glScissor(hmd_w / 2, 0, hmd_w / 2, hmd_h);
				glClearColor(0, 0.5, 0.0, 1.0);
			}
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glUniform2fv(glGetUniformLocation(distortionshader, "LensCenter"), 1, i == 0 ? left_lens_center : right_lens_center);

			glBindVertexArray(VAOs[1]);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textures[i]);

			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		// Da swap-dawup!
		SDL_GL_SwapWindow(gl.window);
		SDL_Delay(10);
	}

	ohmd_ctx_destroy(ctx);
	maploader_cleanup();
	free(_notes);
	free(_obstacles);

	/* SDL2_mixer cleanup */
	Mix_FreeMusic(gMusic);
	gMusic = NULL;
	Mix_Quit();

	return 0;
}
