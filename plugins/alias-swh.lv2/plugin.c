
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Alias {
  float *level;
  float *input;
  float *output;
} Alias;

static void cleanupAlias(LV2_Handle instance)
{

  free(instance);
}

static void connectPortAlias(LV2_Handle instance, uint32_t port, void *data)
{
  Alias *plugin = (Alias *)instance;

  switch (port) {
  case 0:
    plugin->level = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateAlias(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Alias *plugin_data = (Alias *)malloc(sizeof(Alias));
  
  
  return (LV2_Handle)plugin_data;
}



static void runAlias(LV2_Handle instance, uint32_t sample_count)
{
  Alias *plugin_data = (Alias *)instance;

  const float level = *(plugin_data->level);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
      unsigned long pos;
      float coef = 1.0f - 2.0f * level;

      if (output != input) {
        for (pos = 0; pos < sample_count; pos+=2) {
          output[pos] = input[pos];
        }
      }
      for (pos = 1; pos < sample_count; pos+=2) {
        output[pos] = input[pos] * coef;
      }
    
}

static const LV2_Descriptor aliasDescriptor = {
  "http://plugin.org.uk/swh-plugins/alias",
  instantiateAlias,
  connectPortAlias,
  NULL,
  runAlias,
  NULL,
  cleanupAlias,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &aliasDescriptor;
  default:
    return NULL;
  }
}
