
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Karaoke {
  float *gain;
  float *lin;
  float *rin;
  float *lout;
  float *rout;
} Karaoke;

static void cleanupKaraoke(LV2_Handle instance)
{

  free(instance);
}

static void connectPortKaraoke(LV2_Handle instance, uint32_t port, void *data)
{
  Karaoke *plugin = (Karaoke *)instance;

  switch (port) {
  case 0:
    plugin->gain = data;
    break;
  case 1:
    plugin->lin = data;
    break;
  case 2:
    plugin->rin = data;
    break;
  case 3:
    plugin->lout = data;
    break;
  case 4:
    plugin->rout = data;
    break;
  }
}

static LV2_Handle instantiateKaraoke(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Karaoke *plugin_data = (Karaoke *)malloc(sizeof(Karaoke));
  
  
  return (LV2_Handle)plugin_data;
}



static void runKaraoke(LV2_Handle instance, uint32_t sample_count)
{
  Karaoke *plugin_data = (Karaoke *)instance;

  const float gain = *(plugin_data->gain);
  const float * const lin = plugin_data->lin;
  const float * const rin = plugin_data->rin;
  float * const lout = plugin_data->lout;
  float * const rout = plugin_data->rout;
  
      unsigned long pos;
      float coef = pow(10.0f, gain * 0.05f) * 0.5f;
      float m, s;

      for (pos = 0; pos < sample_count; pos++) {
	m = lin[pos] + rin[pos];
	s = lin[pos] - rin[pos];
        lout[pos] = m * coef + s * 0.5f;
        rout[pos] = m * coef - s * 0.5f;
      }
    
}

static const LV2_Descriptor karaokeDescriptor = {
  "http://plugin.org.uk/swh-plugins/karaoke",
  instantiateKaraoke,
  connectPortKaraoke,
  NULL,
  runKaraoke,
  NULL,
  cleanupKaraoke,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &karaokeDescriptor;
  default:
    return NULL;
  }
}
