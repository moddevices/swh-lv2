
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Shaper {
  float *shapep;
  float *input;
  float *output;
} Shaper;

static void cleanupShaper(LV2_Handle instance)
{

  free(instance);
}

static void connectPortShaper(LV2_Handle instance, uint32_t port, void *data)
{
  Shaper *plugin = (Shaper *)instance;

  switch (port) {
  case 0:
    plugin->shapep = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateShaper(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Shaper *plugin_data = (Shaper *)malloc(sizeof(Shaper));
  
  
  return (LV2_Handle)plugin_data;
}



static void runShaper(LV2_Handle instance, uint32_t sample_count)
{
  Shaper *plugin_data = (Shaper *)instance;

  const float shapep = *(plugin_data->shapep);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
int pos;
float shape = 0.0f;

if (shapep < 1.0f && shapep > -1.0f) {
	shape = 1.0f;
} else if (shapep < 0) {
	shape = -1.0f / shape;
} else {
	shape = shapep;
}

for (pos = 0; pos < sample_count; pos++) {
	if (input[pos] < 0.0f) {
		output[pos] = -pow(-input[pos], shape);
	} else {
		output[pos] = pow(input[pos], shape);
	}
}
		
}

static const LV2_Descriptor shaperDescriptor = {
  "http://plugin.org.uk/swh-plugins/shaper",
  instantiateShaper,
  connectPortShaper,
  NULL,
  runShaper,
  NULL,
  cleanupShaper,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &shaperDescriptor;
  default:
    return NULL;
  }
}
