
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Zm1 {
  float *input;
  float *output;
float xm1;
} Zm1;

static void cleanupZm1(LV2_Handle instance)
{

  free(instance);
}

static void connectPortZm1(LV2_Handle instance, uint32_t port, void *data)
{
  Zm1 *plugin = (Zm1 *)instance;

  switch (port) {
  case 0:
    plugin->input = data;
    break;
  case 1:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateZm1(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Zm1 *plugin_data = (Zm1 *)malloc(sizeof(Zm1));
  float xm1 = plugin_data->xm1;
  
      xm1 = 0.0f;
    
  plugin_data->xm1 = xm1;
  
  return (LV2_Handle)plugin_data;
}


static void activateZm1(LV2_Handle instance)
{
  Zm1 *plugin_data = (Zm1 *)instance;
  float xm1 __attribute__ ((unused)) = plugin_data->xm1;
  
      xm1 = 0.0f;
    
}


static void runZm1(LV2_Handle instance, uint32_t sample_count)
{
  Zm1 *plugin_data = (Zm1 *)instance;

  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float xm1 = plugin_data->xm1;
  
      unsigned long pos;
      float tmp;

      for (pos = 0; pos < sample_count; pos++) {
	tmp = input[pos];
        output[pos] = xm1;
	xm1 = tmp;
      }
      plugin_data->xm1 = xm1;
    
}

static const LV2_Descriptor zm1Descriptor = {
  "http://plugin.org.uk/swh-plugins/zm1",
  instantiateZm1,
  connectPortZm1,
  activateZm1,
  runZm1,
  NULL,
  cleanupZm1,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &zm1Descriptor;
  default:
    return NULL;
  }
}
