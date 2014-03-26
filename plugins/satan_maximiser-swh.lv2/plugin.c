
      #include <math.h>
      #include "ladspa-util.h"

      #define BUFFER_SIZE 16
      #define BUFFER_MASK 15
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _SatanMaximiser {
  float *env_time_p;
  float *knee_point;
  float *input;
  float *output;
float env;
float * buffer;
unsigned int buffer_pos;
} SatanMaximiser;

static void cleanupSatanMaximiser(LV2_Handle instance)
{
SatanMaximiser *plugin_data = (SatanMaximiser *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortSatanMaximiser(LV2_Handle instance, uint32_t port, void *data)
{
  SatanMaximiser *plugin = (SatanMaximiser *)instance;

  switch (port) {
  case 0:
    plugin->env_time_p = data;
    break;
  case 1:
    plugin->knee_point = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateSatanMaximiser(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  SatanMaximiser *plugin_data = (SatanMaximiser *)malloc(sizeof(SatanMaximiser));
  float env = plugin_data->env;
  float * buffer = plugin_data->buffer;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  
      env = 0.0f;
      buffer = malloc(sizeof(float) * BUFFER_SIZE);
      buffer_pos = 0;
    
  plugin_data->env = env;
  plugin_data->buffer = buffer;
  plugin_data->buffer_pos = buffer_pos;
  
  return (LV2_Handle)plugin_data;
}


static void activateSatanMaximiser(LV2_Handle instance)
{
  SatanMaximiser *plugin_data = (SatanMaximiser *)instance;
  float env __attribute__ ((unused)) = plugin_data->env;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  
      env = 0.0f;
      memset(buffer, 0, sizeof(float) * BUFFER_SIZE);
      buffer_pos = 0;
    
}


static void runSatanMaximiser(LV2_Handle instance, uint32_t sample_count)
{
  SatanMaximiser *plugin_data = (SatanMaximiser *)instance;

  const float env_time_p = *(plugin_data->env_time_p);
  const float knee_point = *(plugin_data->knee_point);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float env = plugin_data->env;
  float * buffer = plugin_data->buffer;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  
      unsigned long pos;
      int delay;
      float env_tr, env_sc, knee;
      float env_time = env_time_p;

      if (env_time < 2.0f) {
	env_time = 2.0f;
      }
      knee = DB_CO(knee_point);
      delay = f_round(env_time * 0.5f);
      env_tr = 1.0f / env_time;

      for (pos = 0; pos < sample_count; pos++) {
	if (fabs(input[pos]) > env) {
	  env = fabs(input[pos]);
	} else {
	  env = fabs(input[pos]) * env_tr + env * (1.0f - env_tr);
	}
	if (env <= knee) {
	  env_sc = 1.0f / knee;
	} else {
	  env_sc = 1.0f / env;
	}
	buffer[buffer_pos] = input[pos];
	output[pos] = buffer[(buffer_pos - delay) & BUFFER_MASK] * env_sc;
	buffer_pos = (buffer_pos + 1) & BUFFER_MASK;
      }

      plugin_data->env = env;
      plugin_data->buffer_pos = buffer_pos;
    
}

static const LV2_Descriptor satanMaximiserDescriptor = {
  "http://plugin.org.uk/swh-plugins/satanMaximiser",
  instantiateSatanMaximiser,
  connectPortSatanMaximiser,
  activateSatanMaximiser,
  runSatanMaximiser,
  NULL,
  cleanupSatanMaximiser,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &satanMaximiserDescriptor;
  default:
    return NULL;
  }
}
