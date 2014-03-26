
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _WaveTerrain {
  float *xb;
  float *yb;
  float *zb;
} WaveTerrain;

static void cleanupWaveTerrain(LV2_Handle instance)
{

  free(instance);
}

static void connectPortWaveTerrain(LV2_Handle instance, uint32_t port, void *data)
{
  WaveTerrain *plugin = (WaveTerrain *)instance;

  switch (port) {
  case 0:
    plugin->xb = data;
    break;
  case 1:
    plugin->yb = data;
    break;
  case 2:
    plugin->zb = data;
    break;
  }
}

static LV2_Handle instantiateWaveTerrain(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  WaveTerrain *plugin_data = (WaveTerrain *)malloc(sizeof(WaveTerrain));
  
  
  return (LV2_Handle)plugin_data;
}



static void runWaveTerrain(LV2_Handle instance, uint32_t sample_count)
{
  WaveTerrain *plugin_data = (WaveTerrain *)instance;

  const float * const xb = plugin_data->xb;
  const float * const yb = plugin_data->yb;
  float * const zb = plugin_data->zb;
  
      unsigned long pos;
      float x, y;

      for (pos = 0; pos < sample_count; pos++) {
	x = xb[pos];
	y = yb[pos];
        zb[pos] = (x - y) * (x - 1.0f) * (x + 1.0f) * (y - 1.0f) * (y + 1.0f);
      }
    
}

static const LV2_Descriptor waveTerrainDescriptor = {
  "http://plugin.org.uk/swh-plugins/waveTerrain",
  instantiateWaveTerrain,
  connectPortWaveTerrain,
  NULL,
  runWaveTerrain,
  NULL,
  cleanupWaveTerrain,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &waveTerrainDescriptor;
  default:
    return NULL;
  }
}
