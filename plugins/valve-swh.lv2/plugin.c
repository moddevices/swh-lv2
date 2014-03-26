
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Valve {
  float *q_p;
  float *dist_p;
  float *input;
  float *output;
float itm1;
float otm1;
} Valve;

static void cleanupValve(LV2_Handle instance)
{

  free(instance);
}

static void connectPortValve(LV2_Handle instance, uint32_t port, void *data)
{
  Valve *plugin = (Valve *)instance;

  switch (port) {
  case 0:
    plugin->q_p = data;
    break;
  case 1:
    plugin->dist_p = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateValve(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Valve *plugin_data = (Valve *)malloc(sizeof(Valve));
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  
  plugin_data->itm1 = itm1;
  plugin_data->otm1 = otm1;
  
  return (LV2_Handle)plugin_data;
}


static void activateValve(LV2_Handle instance)
{
  Valve *plugin_data = (Valve *)instance;
  float itm1 __attribute__ ((unused)) = plugin_data->itm1;
  float otm1 __attribute__ ((unused)) = plugin_data->otm1;
  
      itm1 = 0.0f;
      otm1 = 0.0f;
    
}


static void runValve(LV2_Handle instance, uint32_t sample_count)
{
  Valve *plugin_data = (Valve *)instance;

  const float q_p = *(plugin_data->q_p);
  const float dist_p = *(plugin_data->dist_p);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  
unsigned long pos;
float fx;

const float q = q_p - 0.999f;
const float dist = dist_p * 40.0f + 0.1f;

if (q == 0.0f) {
	for (pos = 0; pos < sample_count; pos++) {
		if (input[pos] == q) {
			fx = 1.0f / dist;
		} else {
			fx = input[pos] / (1.0f - f_exp(-dist * input[pos]));
		}
		otm1 = 0.999f * otm1 + fx - itm1;
		round_to_zero(&otm1);
		itm1 = fx;
		buffer_write(output[pos], otm1);
	}
} else {
	for (pos = 0; pos < sample_count; pos++) {
		if (input[pos] == q) {
			fx = 1.0f / dist + q / (1.0f - f_exp(dist * q));
		} else {
			fx = (input[pos] - q) /
			 (1.0f - f_exp(-dist * (input[pos] - q))) +
			 q / (1.0f - f_exp(dist * q));
		}
		otm1 = 0.999f * otm1 + fx - itm1;
		round_to_zero(&otm1);
		itm1 = fx;
		buffer_write(output[pos], otm1);
	}
}

plugin_data->itm1 = itm1;
plugin_data->otm1 = otm1;
    
}

static const LV2_Descriptor valveDescriptor = {
  "http://plugin.org.uk/swh-plugins/valve",
  instantiateValve,
  connectPortValve,
  activateValve,
  runValve,
  NULL,
  cleanupValve,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &valveDescriptor;
  default:
    return NULL;
  }
}
