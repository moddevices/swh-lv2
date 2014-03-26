
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Foldover {
  float *drive_p;
  float *push;
  float *input;
  float *output;
} Foldover;

static void cleanupFoldover(LV2_Handle instance)
{

  free(instance);
}

static void connectPortFoldover(LV2_Handle instance, uint32_t port, void *data)
{
  Foldover *plugin = (Foldover *)instance;

  switch (port) {
  case 0:
    plugin->drive_p = data;
    break;
  case 1:
    plugin->push = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateFoldover(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Foldover *plugin_data = (Foldover *)malloc(sizeof(Foldover));
  
  
  return (LV2_Handle)plugin_data;
}



static void runFoldover(LV2_Handle instance, uint32_t sample_count)
{
  Foldover *plugin_data = (Foldover *)instance;

  const float drive_p = *(plugin_data->drive_p);
  const float push = *(plugin_data->push);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
      unsigned long pos;
      float x;
      const float drive = drive_p + 1.0f;

      for (pos = 0; pos < sample_count; pos++) {
	x = input[pos] * drive + push;
        output[pos] = 1.5f * x - 0.5f * x * x * x;
      }
    
}

static const LV2_Descriptor foldoverDescriptor = {
  "http://plugin.org.uk/swh-plugins/foldover",
  instantiateFoldover,
  connectPortFoldover,
  NULL,
  runFoldover,
  NULL,
  cleanupFoldover,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &foldoverDescriptor;
  default:
    return NULL;
  }
}
