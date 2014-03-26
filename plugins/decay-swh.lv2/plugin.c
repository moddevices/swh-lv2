
      #include "ladspa-util.h"

      #define LOG001 -6.9077552789f
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Decay {
  float *in;
  float *out;
  float *decay_time;
float y;
float b;
float last_decay_time;
float sample_rate;
char first_time;
} Decay;

static void cleanupDecay(LV2_Handle instance)
{

  free(instance);
}

static void connectPortDecay(LV2_Handle instance, uint32_t port, void *data)
{
  Decay *plugin = (Decay *)instance;

  switch (port) {
  case 0:
    plugin->in = data;
    break;
  case 1:
    plugin->out = data;
    break;
  case 2:
    plugin->decay_time = data;
    break;
  }
}

static LV2_Handle instantiateDecay(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Decay *plugin_data = (Decay *)malloc(sizeof(Decay));
  float y = plugin_data->y;
  float b = plugin_data->b;
  float last_decay_time = plugin_data->last_decay_time;
  float sample_rate = plugin_data->sample_rate;
  char first_time = plugin_data->first_time;
  
      sample_rate = s_rate;
    
  plugin_data->y = y;
  plugin_data->b = b;
  plugin_data->last_decay_time = last_decay_time;
  plugin_data->sample_rate = sample_rate;
  plugin_data->first_time = first_time;
  
  return (LV2_Handle)plugin_data;
}


static void activateDecay(LV2_Handle instance)
{
  Decay *plugin_data = (Decay *)instance;
  float y __attribute__ ((unused)) = plugin_data->y;
  float b __attribute__ ((unused)) = plugin_data->b;
  float last_decay_time __attribute__ ((unused)) = plugin_data->last_decay_time;
  float sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  char first_time __attribute__ ((unused)) = plugin_data->first_time;
  
      plugin_data->b = 0.f;
      plugin_data->y = 0.f;
      plugin_data->last_decay_time = 0.f;
      plugin_data->first_time = 0;
    
}


static void runDecay(LV2_Handle instance, uint32_t sample_count)
{
  Decay *plugin_data = (Decay *)instance;

  const float * const in = plugin_data->in;
  float * const out = plugin_data->out;
  const float decay_time = *(plugin_data->decay_time);
  float y = plugin_data->y;
  float b = plugin_data->b;
  float last_decay_time = plugin_data->last_decay_time;
  float sample_rate = plugin_data->sample_rate;
  char first_time = plugin_data->first_time;
  
      int i;

      if (first_time) {
        plugin_data->last_decay_time = decay_time;
        plugin_data->b = decay_time == 0.f ? 0.f : exp (LOG001 / (decay_time * sample_rate));
        plugin_data->first_time = 0;
      }

      if (decay_time == plugin_data->last_decay_time) {
        if (b == 0.f)
          for (i=0; i<sample_count; i++)
            out[i] = y = in[i];
        else
          for (i=0; i<sample_count; i++)
            out[i] = y = in[i] + b * y;
      } else {
        float b_slope;

        plugin_data->b = decay_time == 0.f ? 0.f : exp (LOG001 / (decay_time * sample_rate));
        b_slope = (plugin_data->b - b) / sample_count;

        for (i=0; i<sample_count; i++) {
          buffer_write(out[i], y = in[i] + b * y);
          b += b_slope;
        }

        plugin_data->last_decay_time = decay_time;
      }
      
      plugin_data->y = y;
    
}

static const LV2_Descriptor decayDescriptor = {
  "http://plugin.org.uk/swh-plugins/decay",
  instantiateDecay,
  connectPortDecay,
  activateDecay,
  runDecay,
  NULL,
  cleanupDecay,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &decayDescriptor;
  default:
    return NULL;
  }
}
