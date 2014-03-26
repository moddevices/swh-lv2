
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Const {
  float *amplitude;
  float *input;
  float *output;
float last_amp;
} Const;

static void cleanupConst(LV2_Handle instance)
{

  free(instance);
}

static void connectPortConst(LV2_Handle instance, uint32_t port, void *data)
{
  Const *plugin = (Const *)instance;

  switch (port) {
  case 0:
    plugin->amplitude = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateConst(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Const *plugin_data = (Const *)malloc(sizeof(Const));
  float last_amp = plugin_data->last_amp;
  
  plugin_data->last_amp = last_amp;
  
  return (LV2_Handle)plugin_data;
}


static void activateConst(LV2_Handle instance)
{
  Const *plugin_data = (Const *)instance;
  float last_amp __attribute__ ((unused)) = plugin_data->last_amp;
  
      last_amp = 0.0f;
    
}


static void runConst(LV2_Handle instance, uint32_t sample_count)
{
  Const *plugin_data = (Const *)instance;

  const float amplitude = *(plugin_data->amplitude);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float last_amp = plugin_data->last_amp;
  
      unsigned long pos;
      const float delta = (amplitude - last_amp) / (sample_count - 1);
      float amp = last_amp;

      for (pos = 0; pos < sample_count; pos++) {
	amp += delta;
        output[pos] = input[pos] + amp;
      }

      plugin_data->last_amp = amp;
    
}

static const LV2_Descriptor constDescriptor = {
  "http://plugin.org.uk/swh-plugins/const",
  instantiateConst,
  connectPortConst,
  activateConst,
  runConst,
  NULL,
  cleanupConst,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &constDescriptor;
  default:
    return NULL;
  }
}
