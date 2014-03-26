
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Alaw {
  float *input;
  float *output;
} Alaw;

static void cleanupAlaw(LV2_Handle instance)
{

  free(instance);
}

static void connectPortAlaw(LV2_Handle instance, uint32_t port, void *data)
{
  Alaw *plugin = (Alaw *)instance;

  switch (port) {
  case 0:
    plugin->input = data;
    break;
  case 1:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateAlaw(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Alaw *plugin_data = (Alaw *)malloc(sizeof(Alaw));
  
  
  return (LV2_Handle)plugin_data;
}



static void runAlaw(LV2_Handle instance, uint32_t sample_count)
{
  Alaw *plugin_data = (Alaw *)instance;

  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
      /* float a = 87.7; */
      float ainv = 0.011402508551881; /* 1.0 / a */
      float aloginv = 0.182684374819744; /* 1.0 / (1.0 + log(a)) */
      float aexp = 16.021419671691538; /* a * aloginv */
      unsigned long pos;
      for (pos = 0; pos < sample_count; pos++) {
          float s = input[pos];
          float sabs = fabs(s);
          if (sabs < ainv) {
              s *= aexp;
          } else {
              if (s >= 0)
                   s = 1.0 + log(sabs) * aloginv;
              else
                   s = -1.0 - log(sabs) * aloginv;
          }
          buffer_write(output[pos], s);
      }
    
}

static const LV2_Descriptor alawDescriptor = {
  "http://plugin.org.uk/swh-plugins/alaw",
  instantiateAlaw,
  connectPortAlaw,
  NULL,
  runAlaw,
  NULL,
  cleanupAlaw,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &alawDescriptor;
  default:
    return NULL;
  }
}
