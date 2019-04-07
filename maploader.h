// https://pastebin.com/cTPGrxWY -> explains json layout
#include <stdio.h>
#include <json-c/json.h>
#include <string.h>		// for strnlen

#define MAX_NOTES	4096
#define MAX_OBSTACLES	4096
#define MAX_BUFFER	16777216	// about 16M == 2^24; doubt the json would be longer

struct note {
	int cutDirection;
	int lineIndex;	// 0 to 3, start from left
	int lineLayer;	// 0 to 2, start from bottom
	double time;	// or double? unsigned? note time position in beats
	int type;	// 0 = red, 1 = blue, 3 = bomb
};

struct obstacle {
	double duration;	// length in beats, incl. 0.5 etc.
	int lineIndex;	// horizontal position 0 to 3, start from left
	double time;	// obstacle time position in beats
	int type;	// 0 = wall, 1 = ceiling
	int width;	// width in lines (extends to the right)
};

// TODO: struct event {...

// variables to get

char *_version;		// "_version": "1.5.0"
extern int _beatsPerBar;	// "_beatsPerBar": 16 - ONLY USED BY EDITOR - TODO: remove/ignore?
extern int _beatsPerMinute;	// "_beatsPerMinute": 140
// array events -> array of what?
// 	time: event time position in beats
// 	type: event type
// 	value: event value
// noteJumpSpeed -> what is this?? - movement speed of notes; unused
// shuffle: how random notes are colored in no direction mode
// shufflePeriod: how often notes should change color in no direction mode
// _notes: array of note structs
// TODO: is _notes sorted by _time???

struct note *_notes;
struct obstacle *_obstacles;
// TODO: struct event *_events;

extern int num_notes;
extern int num_obst;
extern int num_events;

void init_map(char *filename, struct note *_notes, struct obstacle *_obstacles);

void maploader_cleanup();
