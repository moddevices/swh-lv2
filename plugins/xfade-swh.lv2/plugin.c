
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Xfade {
  float *xfade;
  float *inputLA;
  float *inputRA;
  float *inputLB;
  float *inputRB;
  float *outputL;
  float *outputR;
} Xfade;

static void cleanupXfade(LV2_Handle instance)
{

  free(instance);
}

static void connectPortXfade(LV2_Handle instance, uint32_t port, void *data)
{
  Xfade *plugin = (Xfade *)instance;

  switch (port) {
  case 0:
    plugin->xfade = data;
    break;
  case 1:
    plugin->inputLA = data;
    break;
  case 2:
    plugin->inputRA = data;
    break;
  case 3:
    plugin->inputLB = data;
    break;
  case 4:
    plugin->inputRB = data;
    break;
  case 5:
    plugin->outputL = data;
    break;
  case 6:
    plugin->outputR = data;
    break;
  }
}

static LV2_Handle instantiateXfade(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Xfade *plugin_data = (Xfade *)malloc(sizeof(Xfade));
  
  
  return (LV2_Handle)plugin_data;
}



static void runXfade(LV2_Handle instance, uint32_t sample_count)
{
  Xfade *plugin_data = (Xfade *)instance;

  const float xfade = *(plugin_data->xfade);
  const float * const inputLA = plugin_data->inputLA;
  const float * const inputRA = plugin_data->inputRA;
  const float * const inputLB = plugin_data->inputLB;
  const float * const inputRB = plugin_data->inputRB;
  float * const outputL = plugin_data->outputL;
  float * const outputR = plugin_data->outputR;
  
      unsigned long pos;
      const float coefB = (xfade + 1.0f) * 0.5f;
      const float coefA = 1.0f - coefB;

      for (pos = 0; pos < sample_count; pos++) {
        buffer_write(outputL[pos], inputLA[pos] * coefA + inputLB[pos] * coefB);
        buffer_write(outputR[pos], inputRA[pos] * coefA + inputRB[pos] * coefB);
      }
    
}

static const LV2_Descriptor xfadeDescriptor = {
  "http://plugin.org.uk/swh-plugins/xfade",
  instantiateXfade,
  connectPortXfade,
  NULL,
  runXfade,
  NULL,
  cleanupXfade,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Xfade4 {
  float *xfade;
  float *inputLA;
  float *inputRA;
  float *inputLB;
  float *inputRB;
  float *outputLA;
  float *outputRA;
  float *outputLB;
  float *outputRB;
} Xfade4;

static void cleanupXfade4(LV2_Handle instance)
{

  free(instance);
}

static void connectPortXfade4(LV2_Handle instance, uint32_t port, void *data)
{
  Xfade4 *plugin = (Xfade4 *)instance;

  switch (port) {
  case 0:
    plugin->xfade = data;
    break;
  case 1:
    plugin->inputLA = data;
    break;
  case 2:
    plugin->inputRA = data;
    break;
  case 3:
    plugin->inputLB = data;
    break;
  case 4:
    plugin->inputRB = data;
    break;
  case 5:
    plugin->outputLA = data;
    break;
  case 6:
    plugin->outputRA = data;
    break;
  case 7:
    plugin->outputLB = data;
    break;
  case 8:
    plugin->outputRB = data;
    break;
  }
}

static LV2_Handle instantiateXfade4(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Xfade4 *plugin_data = (Xfade4 *)malloc(sizeof(Xfade4));
  
  
  return (LV2_Handle)plugin_data;
}



static void runXfade4(LV2_Handle instance, uint32_t sample_count)
{
  Xfade4 *plugin_data = (Xfade4 *)instance;

  const float xfade = *(plugin_data->xfade);
  const float * const inputLA = plugin_data->inputLA;
  const float * const inputRA = plugin_data->inputRA;
  const float * const inputLB = plugin_data->inputLB;
  const float * const inputRB = plugin_data->inputRB;
  float * const outputLA = plugin_data->outputLA;
  float * const outputRA = plugin_data->outputRA;
  float * const outputLB = plugin_data->outputLB;
  float * const outputRB = plugin_data->outputRB;
  
      unsigned long pos;
      const float coefB = (xfade + 1.0f) * 0.5f;
      const float coefA = 1.0f - coefB;

      for (pos = 0; pos < sample_count; pos++) {
        buffer_write(outputLA[pos], inputLA[pos] * coefA);
        buffer_write(outputRA[pos], inputRA[pos] * coefA);
        buffer_write(outputLB[pos], inputLB[pos] * coefB);
        buffer_write(outputRB[pos], inputRB[pos] * coefB);
      }
    
}

static const LV2_Descriptor xfade4Descriptor = {
  "http://plugin.org.uk/swh-plugins/xfade4",
  instantiateXfade4,
  connectPortXfade4,
  NULL,
  runXfade4,
  NULL,
  cleanupXfade4,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &xfadeDescriptor;
  case 1:
    return &xfade4Descriptor;
  default:
    return NULL;
  }
}
