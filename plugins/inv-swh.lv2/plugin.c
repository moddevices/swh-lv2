
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Inv {
  float *input;
  float *output;
} Inv;

static void cleanupInv(LV2_Handle instance)
{

  free(instance);
}

static void connectPortInv(LV2_Handle instance, uint32_t port, void *data)
{
  Inv *plugin = (Inv *)instance;

  switch (port) {
  case 0:
    plugin->input = data;
    break;
  case 1:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateInv(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Inv *plugin_data = (Inv *)malloc(sizeof(Inv));
  
  
  return (LV2_Handle)plugin_data;
}



static void runInv(LV2_Handle instance, uint32_t sample_count)
{
  Inv *plugin_data = (Inv *)instance;

  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
      unsigned long pos;

      for (pos = 0; pos < sample_count; pos++) {
        output[pos] = -input[pos];
      }
    
}

static const LV2_Descriptor invDescriptor = {
  "http://plugin.org.uk/swh-plugins/inv",
  instantiateInv,
  connectPortInv,
  NULL,
  runInv,
  NULL,
  cleanupInv,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &invDescriptor;
  default:
    return NULL;
  }
}
