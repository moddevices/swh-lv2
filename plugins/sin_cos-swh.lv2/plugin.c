
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _SinCos {
  float *freq;
  float *pitch;
  float *sine;
  float *cosine;
double phi;
float fs;
double last_om;
} SinCos;

static void cleanupSinCos(LV2_Handle instance)
{

  free(instance);
}

static void connectPortSinCos(LV2_Handle instance, uint32_t port, void *data)
{
  SinCos *plugin = (SinCos *)instance;

  switch (port) {
  case 0:
    plugin->freq = data;
    break;
  case 1:
    plugin->pitch = data;
    break;
  case 2:
    plugin->sine = data;
    break;
  case 3:
    plugin->cosine = data;
    break;
  }
}

static LV2_Handle instantiateSinCos(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  SinCos *plugin_data = (SinCos *)malloc(sizeof(SinCos));
  double phi = plugin_data->phi;
  float fs = plugin_data->fs;
  double last_om = plugin_data->last_om;
  
      fs = (float)s_rate;
      phi = 0.0;
      last_om = 0.0;
    
  plugin_data->phi = phi;
  plugin_data->fs = fs;
  plugin_data->last_om = last_om;
  
  return (LV2_Handle)plugin_data;
}



static void runSinCos(LV2_Handle instance, uint32_t sample_count)
{
  SinCos *plugin_data = (SinCos *)instance;

  const float freq = *(plugin_data->freq);
  const float pitch = *(plugin_data->pitch);
  float * const sine = plugin_data->sine;
  float * const cosine = plugin_data->cosine;
  double phi = plugin_data->phi;
  float fs = plugin_data->fs;
  double last_om = plugin_data->last_om;
  
      unsigned long pos;
      const double target_om = 2.0 * M_PI * f_clamp(freq, 0.0f, 0.5f) * pow(2.0, f_clamp(pitch, 0.0f, 16.0f)) / fs;
      const double om_d = (target_om - last_om) / (double)sample_count;
      double om = last_om;

      for (pos = 0; pos < sample_count; pos++) {
        buffer_write(sine[pos], sin(phi));
        buffer_write(cosine[pos], cos(phi));
	om += om_d;
	phi += om;
      }
      while (phi > 2.0 * M_PI) {
	phi -= 2.0 * M_PI;
      }

      plugin_data->phi = phi;
      plugin_data->last_om = target_om;
    
}

static const LV2_Descriptor sinCosDescriptor = {
  "http://plugin.org.uk/swh-plugins/sinCos",
  instantiateSinCos,
  connectPortSinCos,
  NULL,
  runSinCos,
  NULL,
  cleanupSinCos,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &sinCosDescriptor;
  default:
    return NULL;
  }
}
