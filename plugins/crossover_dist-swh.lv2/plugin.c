
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _CrossoverDist {
  float *amp;
  float *smooth;
  float *input;
  float *output;
} CrossoverDist;

static void cleanupCrossoverDist(LV2_Handle instance)
{

  free(instance);
}

static void connectPortCrossoverDist(LV2_Handle instance, uint32_t port, void *data)
{
  CrossoverDist *plugin = (CrossoverDist *)instance;

  switch (port) {
  case 0:
    plugin->amp = data;
    break;
  case 1:
    plugin->smooth = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateCrossoverDist(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  CrossoverDist *plugin_data = (CrossoverDist *)malloc(sizeof(CrossoverDist));
  
  
  return (LV2_Handle)plugin_data;
}



static void runCrossoverDist(LV2_Handle instance, uint32_t sample_count)
{
  CrossoverDist *plugin_data = (CrossoverDist *)instance;

  const float amp = *(plugin_data->amp);
  const float smooth = *(plugin_data->smooth);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
      unsigned long pos;
      float sig;
      const float fade = fabs(amp * smooth) + 0.0001;

      for (pos = 0; pos < sample_count; pos++) {
	sig = fabs(input[pos]) - amp;

        if (sig < 0.0f) {
          sig *= (1.0f + sig/fade) * smooth;
        }

	if (input[pos] < 0.0f) {
	  buffer_write(output[pos], -sig);
        } else {
	  buffer_write(output[pos], sig);
        }
      }
    
}

static const LV2_Descriptor crossoverDistDescriptor = {
  "http://plugin.org.uk/swh-plugins/crossoverDist",
  instantiateCrossoverDist,
  connectPortCrossoverDist,
  NULL,
  runCrossoverDist,
  NULL,
  cleanupCrossoverDist,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &crossoverDistDescriptor;
  default:
    return NULL;
  }
}
