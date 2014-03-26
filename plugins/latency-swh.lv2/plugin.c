
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _ArtificialLatency {
  float *delay;
  float *input;
  float *output;
  float *latency;
float fs;
} ArtificialLatency;

static void cleanupArtificialLatency(LV2_Handle instance)
{

  free(instance);
}

static void connectPortArtificialLatency(LV2_Handle instance, uint32_t port, void *data)
{
  ArtificialLatency *plugin = (ArtificialLatency *)instance;

  switch (port) {
  case 0:
    plugin->delay = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  case 3:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiateArtificialLatency(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  ArtificialLatency *plugin_data = (ArtificialLatency *)malloc(sizeof(ArtificialLatency));
  float fs = plugin_data->fs;
  
      fs = s_rate;
    
  plugin_data->fs = fs;
  
  return (LV2_Handle)plugin_data;
}



static void runArtificialLatency(LV2_Handle instance, uint32_t sample_count)
{
  ArtificialLatency *plugin_data = (ArtificialLatency *)instance;

  const float delay = *(plugin_data->delay);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float latency;
  float fs = plugin_data->fs;
  
      unsigned long pos;
      const int delay_fr = f_round(delay * 0.001 * fs);

      if (input != output) {
        for (pos = 0; pos < sample_count; pos++) {
          buffer_write(output[pos], input[pos]);
        }
      }
      *(plugin_data->latency) = (float)delay_fr;
    
}

static const LV2_Descriptor artificialLatencyDescriptor = {
  "http://plugin.org.uk/swh-plugins/artificialLatency",
  instantiateArtificialLatency,
  connectPortArtificialLatency,
  NULL,
  runArtificialLatency,
  NULL,
  cleanupArtificialLatency,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &artificialLatencyDescriptor;
  default:
    return NULL;
  }
}
