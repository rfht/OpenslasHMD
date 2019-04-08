#include "resourceloader.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

int load_gltf(char* filename)
{
	cgltf_options options = {0};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, filename, &data);
	if (result == cgltf_result_success)
	{
		/* TODO make awesome stuff */
		if (data->nodes_count > 0)
		{
			printf("--| Resource Loading |--\n");
			printf("--| Found object %s with node count of %li\n", data->nodes->name, data->nodes_count);
		}
		else
			printf("No nodes found, freeing resource]\n");

		cgltf_free(data);
	}
}