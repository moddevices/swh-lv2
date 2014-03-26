
      #include "ladspa-util.h"

      #define LOG001 -6.9077552789f
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Impulse_fc {
  float *frequency;
  float *out;
float sample_rate;
float phase;
} Impulse_fc;

static void cleanupImpulse_fc(LV2_Handle instance)
{

  free(instance);
}

static void connectPortImpulse_fc(LV2_Handle instance, uint32_t port, void *data)
{
  Impulse_fc *plugin = (Impulse_fc *)instance;

  switch (port) {
  case 0:
    plugin->frequency = data;
    break;
  case 1:
    plugin->out = data;
    break;
  }
}

static LV2_Handle instantiateImpulse_fc(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Impulse_fc *plugin_data = (Impulse_fc *)malloc(sizeof(Impulse_fc));
  float sample_rate = plugin_data->sample_rate;
  float phase = plugin_data->phase;
  
      sample_rate = s_rate;
      phase = 0.f;
    
  plugin_data->sample_rate = sample_rate;
  plugin_data->phase = phase;
  
  return (LV2_Handle)plugin_data;
}


static void activateImpulse_fc(LV2_Handle instance)
{
  Impulse_fc *plugin_data = (Impulse_fc *)instance;
  float sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float phase __attribute__ ((unused)) = plugin_data->phase;
  
      phase = 0.f;
    
}


static void runImpulse_fc(LV2_Handle instance, uint32_t sample_count)
{
  Impulse_fc *plugin_data = (Impulse_fc *)instance;

  const float frequency = *(plugin_data->frequency);
  float * const out = plugin_data->out;
  float sample_rate = plugin_data->sample_rate;
  float phase = plugin_data->phase;
  
      int i;
      float phase_step = frequency / sample_rate;

      for (i=0; i<sample_count; i++) {
        if (phase > 1.f) {
          phase -= 1.f;
          buffer_write(out[i], 1.f);
        } else {
          buffer_write(out[i], 0.f);
        }
        phase += phase_step;
      }

      plugin_data->phase = phase;
    
}

static const LV2_Descriptor impulse_fcDescriptor = {
  "http://plugin.org.uk/swh-plugins/impulse_fc",
  instantiateImpulse_fc,
  connectPortImpulse_fc,
  activateImpulse_fc,
  runImpulse_fc,
  NULL,
  cleanupImpulse_fc,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &impulse_fcDescriptor;
  default:
    return NULL;
  }
}
