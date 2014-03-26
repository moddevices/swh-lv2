
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _SinusWavewrapper {
  float *wrap;
  float *input;
  float *output;
} SinusWavewrapper;

static void cleanupSinusWavewrapper(LV2_Handle instance)
{

  free(instance);
}

static void connectPortSinusWavewrapper(LV2_Handle instance, uint32_t port, void *data)
{
  SinusWavewrapper *plugin = (SinusWavewrapper *)instance;

  switch (port) {
  case 0:
    plugin->wrap = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateSinusWavewrapper(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  SinusWavewrapper *plugin_data = (SinusWavewrapper *)malloc(sizeof(SinusWavewrapper));
  
  
  return (LV2_Handle)plugin_data;
}



static void runSinusWavewrapper(LV2_Handle instance, uint32_t sample_count)
{
  SinusWavewrapper *plugin_data = (SinusWavewrapper *)instance;

  const float wrap = *(plugin_data->wrap);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
			float coef = wrap * M_PI;
			unsigned long pos;

			if (coef < 0.05f) {
				coef = 0.05f;
			}

			for (pos = 0; pos < sample_count; pos++) {
				output[pos] = sin(input[pos] * coef);
			}
		
}

static const LV2_Descriptor sinusWavewrapperDescriptor = {
  "http://plugin.org.uk/swh-plugins/sinusWavewrapper",
  instantiateSinusWavewrapper,
  connectPortSinusWavewrapper,
  NULL,
  runSinusWavewrapper,
  NULL,
  cleanupSinusWavewrapper,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &sinusWavewrapperDescriptor;
  default:
    return NULL;
  }
}
