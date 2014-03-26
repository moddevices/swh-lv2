
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Amp {
  float *gain;
  float *input;
  float *output;
} Amp;

static void cleanupAmp(LV2_Handle instance)
{

  free(instance);
}

static void connectPortAmp(LV2_Handle instance, uint32_t port, void *data)
{
  Amp *plugin = (Amp *)instance;

  switch (port) {
  case 0:
    plugin->gain = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateAmp(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Amp *plugin_data = (Amp *)malloc(sizeof(Amp));
  
  
  return (LV2_Handle)plugin_data;
}



static void runAmp(LV2_Handle instance, uint32_t sample_count)
{
  Amp *plugin_data = (Amp *)instance;

  const float gain = *(plugin_data->gain);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
      unsigned long pos;
      float coef = DB_CO(gain);

      for (pos = 0; pos < sample_count; pos++) {
        buffer_write(output[pos], input[pos] * coef);
      }
    
}

static const LV2_Descriptor ampDescriptor = {
  "http://plugin.org.uk/swh-plugins/amp",
  instantiateAmp,
  connectPortAmp,
  NULL,
  runAmp,
  NULL,
  cleanupAmp,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &ampDescriptor;
  default:
    return NULL;
  }
}
