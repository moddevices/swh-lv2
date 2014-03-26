
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Divider {
  float *denominator;
  float *input;
  float *output;
float last;
float amp;
float lamp;
int zeroxs;
float count;
float out;
} Divider;

static void cleanupDivider(LV2_Handle instance)
{

  free(instance);
}

static void connectPortDivider(LV2_Handle instance, uint32_t port, void *data)
{
  Divider *plugin = (Divider *)instance;

  switch (port) {
  case 0:
    plugin->denominator = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateDivider(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Divider *plugin_data = (Divider *)malloc(sizeof(Divider));
  float last = plugin_data->last;
  float amp = plugin_data->amp;
  float lamp = plugin_data->lamp;
  int zeroxs = plugin_data->zeroxs;
  float count = plugin_data->count;
  float out = plugin_data->out;
  
out = 1.0f;
amp = 0.0f;
count = 0.0f;
lamp = 0.0f;
last = 0.0f;
zeroxs = 0;
		
  plugin_data->last = last;
  plugin_data->amp = amp;
  plugin_data->lamp = lamp;
  plugin_data->zeroxs = zeroxs;
  plugin_data->count = count;
  plugin_data->out = out;
  
  return (LV2_Handle)plugin_data;
}



static void runDivider(LV2_Handle instance, uint32_t sample_count)
{
  Divider *plugin_data = (Divider *)instance;

  const float denominator = *(plugin_data->denominator);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float last = plugin_data->last;
  float amp = plugin_data->amp;
  float lamp = plugin_data->lamp;
  int zeroxs = plugin_data->zeroxs;
  float count = plugin_data->count;
  float out = plugin_data->out;
  
/* Integer version of denominator */
int den = (int)denominator;

unsigned long pos;

for (pos = 0; pos < sample_count; pos++) {
	count += 1.0f;
	if ((input[pos] > 0.0f && last <= 0.0f) ||
	 (input[pos] < 0.0f && last >= 0.0)) {
		zeroxs++;
		if (den == 1) {
			out = out > 0.0f ? -1.0f : 1.0f;
			lamp = amp / count;
			zeroxs = 0;
			count = 0;
			amp = 0;
		}
	}
	amp += fabs(input[pos]);
	if (den > 1 && (zeroxs % den) == den-1) {
		out = out > 0.0f ? -1.0f : 1.0f;
		lamp = amp / count;
		zeroxs = 0;
		count = 0;
		amp = 0;
	}
	last = input[pos];
	output[pos] = out * lamp;
}

plugin_data->last = last;
plugin_data->amp = amp;
plugin_data->lamp = lamp;
plugin_data->zeroxs = zeroxs;
plugin_data->count = count;
plugin_data->out = out;
		
}

static const LV2_Descriptor dividerDescriptor = {
  "http://plugin.org.uk/swh-plugins/divider",
  instantiateDivider,
  connectPortDivider,
  NULL,
  runDivider,
  NULL,
  cleanupDivider,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &dividerDescriptor;
  default:
    return NULL;
  }
}
