
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Split {
  float *input;
  float *out2;
  float *out1;
} Split;

static void cleanupSplit(LV2_Handle instance)
{

  free(instance);
}

static void connectPortSplit(LV2_Handle instance, uint32_t port, void *data)
{
  Split *plugin = (Split *)instance;

  switch (port) {
  case 0:
    plugin->input = data;
    break;
  case 1:
    plugin->out2 = data;
    break;
  case 2:
    plugin->out1 = data;
    break;
  }
}

static LV2_Handle instantiateSplit(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Split *plugin_data = (Split *)malloc(sizeof(Split));
  
  
  return (LV2_Handle)plugin_data;
}



static void runSplit(LV2_Handle instance, uint32_t sample_count)
{
  Split *plugin_data = (Split *)instance;

  const float * const input = plugin_data->input;
  float * const out2 = plugin_data->out2;
  float * const out1 = plugin_data->out1;
  
			unsigned long pos;

			for (pos = 0; pos < sample_count; pos++) {
				const float in = input[pos];

				out1[pos] = in;
				out2[pos] = in;
			}
		
}

static const LV2_Descriptor splitDescriptor = {
  "http://plugin.org.uk/swh-plugins/split",
  instantiateSplit,
  connectPortSplit,
  NULL,
  runSplit,
  NULL,
  cleanupSplit,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &splitDescriptor;
  default:
    return NULL;
  }
}
