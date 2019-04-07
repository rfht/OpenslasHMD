// https://pastebin.com/cTPGrxWY -> explains json layout
#include "maploader.h"

int _beatsPerBar;
int _beatsPerMinute;

int num_notes;
int num_obst;
int num_events;

void init_map(char *filename, struct note *_notes, struct obstacle *_obstacles)
{
	// https://progur.com/2018/12/how-to-parse-json-in-c.html
	FILE *fp;
	char *buffer = malloc(sizeof(char) * MAX_BUFFER);
	
	struct json_object *parsed_json;
	struct json_object *jversion;	// _version
	struct json_object *jbpb;	// _beatsPerBar
	struct json_object *jbpm;	// _beatsPerMinute

	/* _notes */
	struct json_object *jnotes;	// _notes
	size_t n_notes;			// number of _notes
	struct json_object *jn_note;	// individual note
	struct json_object *jn_cutDir;	// _cutDirection
	struct json_object *jn_li;	// _lineIndex
	struct json_object *jn_ll;	// _lineLayer
	struct json_object *jn_time;	// _time
	struct json_object *jn_type;	// _type

	/* _obstacles */
	struct json_object *jobst;	// _obstacles
	size_t n_obst;			// number of obstacles
	struct json_object *jo_obst;	// individual obstacle
	struct json_object *jo_duration;	// _duration
	struct json_object *jo_li;	// _lineIndex
	struct json_object *jo_time;	// _time
	struct json_object *jo_type;	// _type
	struct json_object *jo_width;	// _width

	//_notes = malloc(sizeof(struct note) * MAX_NOTES);
	//_obstacles = malloc(sizeof(struct obstacle) * MAX_OBSTACLES);

	int i;

	fp = fopen(filename, "r");
	// TODO: throw error if json exceeds buffer length
	fread(buffer, MAX_BUFFER, 1, fp);
	fclose(fp);

	printf("length of the json: %ld of max %d characters/bytes\n", strnlen(buffer, MAX_BUFFER), MAX_BUFFER);

	parsed_json = json_tokener_parse(buffer);

	json_object_object_get_ex(parsed_json, "_version", &jversion);
	json_object_object_get_ex(parsed_json, "_beatsPerBar", &jbpb);
	json_object_object_get_ex(parsed_json, "_beatsPerMinute", &jbpm);

	json_object_object_get_ex(parsed_json, "_notes", &jnotes);
	n_notes = json_object_array_length(jnotes);

	/* get notes */
	for (i = 0; i < n_notes; i++) {
		jn_note = json_object_array_get_idx(jnotes, i);
		json_object_object_get_ex(jn_note, "_cutDirection", &jn_cutDir);
		json_object_object_get_ex(jn_note, "_lineIndex", &jn_li);
		json_object_object_get_ex(jn_note, "_lineLayer", &jn_ll);
		json_object_object_get_ex(jn_note, "_time", &jn_time);
		json_object_object_get_ex(jn_note, "_type", &jn_type);
		/*
		printf("note: %d, cutDir: %d, gridPos: %.1f, %.1f, time: %.6f, type: %d\n",
			i,
			json_object_get_int(jn_cutDir),
			json_object_get_double(jn_li),
			json_object_get_double(jn_ll),
			json_object_get_double(jn_time),
			json_object_get_int(jn_type)
		);
		*/
		_notes[i].cutDirection = json_object_get_int(jn_cutDir);
		_notes[i].lineIndex = json_object_get_double(jn_li);
		_notes[i].lineLayer = json_object_get_double(jn_ll);
		_notes[i].time = json_object_get_double(jn_time);
		_notes[i].type = json_object_get_int(jn_type);
	}


	json_object_object_get_ex(parsed_json, "_obstacles", &jobst);
	n_obst = json_object_array_length(jobst);

	/* get obstacles */
	for (i = 0; i < n_obst; i++) {
		jo_obst = json_object_array_get_idx(jobst, i);
		json_object_object_get_ex(jo_obst, "_duration", &jo_duration);
		json_object_object_get_ex(jo_obst, "_lineIndex", &jo_li);
		json_object_object_get_ex(jo_obst, "_time", &jo_time);
		json_object_object_get_ex(jo_obst, "_type", &jo_type);
		json_object_object_get_ex(jo_obst, "_width", &jo_width);

		_obstacles[i].duration = json_object_get_double(jo_duration);
		_obstacles[i].lineIndex = json_object_get_int(jo_li);
		_obstacles[i].time = json_object_get_double(jo_time);
		_obstacles[i].type = json_object_get_int(jo_type);
		_obstacles[i].width = json_object_get_int(jo_width);
	}

	/* store values in shared variables */
	_version = json_object_get_string(jversion);
	num_notes = n_notes;
	num_obst = n_obst;
	_beatsPerMinute = json_object_get_int(jbpm);
	_beatsPerBar = json_object_get_int(jbpb);
	printf("bpb: %d\n", json_object_get_int(jbpb));
	printf("bpm: %d\n", json_object_get_int(jbpm));
	printf("n_notes: %ld\n", n_notes);
	printf("n_obst: %ld\n", n_obst);

	/*
	for (i = 0; i < n_obst; i++) {
		printf("%.2f, ", _obstacles[i].time);
	}
	printf("\n");
	*/

	free(buffer);
}

void maploader_cleanup()
{
	//free(_notes);
	//free(_obstacles);
}
