
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Ulaw {
  float *input;
  float *output;
} Ulaw;

static void cleanupUlaw(LV2_Handle instance)
{

  free(instance);
}

static void connectPortUlaw(LV2_Handle instance, uint32_t port, void *data)
{
  Ulaw *plugin = (Ulaw *)instance;

  switch (port) {
  case 0:
    plugin->input = data;
    break;
  case 1:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateUlaw(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Ulaw *plugin_data = (Ulaw *)malloc(sizeof(Ulaw));
  
  
  return (LV2_Handle)plugin_data;
}



static void runUlaw(LV2_Handle instance, uint32_t sample_count)
{
  Ulaw *plugin_data = (Ulaw *)instance;

  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
      /* float u = 255.0; */
      float uloginv = 0.18033688011112; /* 1.0 / log(1.0 + u) */
      unsigned long pos;
      for (pos = 0; pos < sample_count; pos++) {
          float s = input[pos];
          float sabs = fabs(s);
          if (s >= 0)
              s =  uloginv * log(1.0 + 255.0 * sabs);
          else
              s = -uloginv * log(1.0 + 255.0 * sabs);
          buffer_write(output[pos], s);
      }
    
}

static const LV2_Descriptor ulawDescriptor = {
  "http://plugin.org.uk/swh-plugins/ulaw",
  instantiateUlaw,
  connectPortUlaw,
  NULL,
  runUlaw,
  NULL,
  cleanupUlaw,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &ulawDescriptor;
  default:
    return NULL;
  }
}
