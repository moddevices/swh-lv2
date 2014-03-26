
      #include <math.h>
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _HardLimiter {
  float *limit_db;
  float *wet_gain;
  float *res_gain;
  float *input;
  float *output;
} HardLimiter;

static void cleanupHardLimiter(LV2_Handle instance)
{

  free(instance);
}

static void connectPortHardLimiter(LV2_Handle instance, uint32_t port, void *data)
{
  HardLimiter *plugin = (HardLimiter *)instance;

  switch (port) {
  case 0:
    plugin->limit_db = data;
    break;
  case 1:
    plugin->wet_gain = data;
    break;
  case 2:
    plugin->res_gain = data;
    break;
  case 3:
    plugin->input = data;
    break;
  case 4:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateHardLimiter(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  HardLimiter *plugin_data = (HardLimiter *)malloc(sizeof(HardLimiter));
  
  
  return (LV2_Handle)plugin_data;
}



static void runHardLimiter(LV2_Handle instance, uint32_t sample_count)
{
  HardLimiter *plugin_data = (HardLimiter *)instance;

  const float limit_db = *(plugin_data->limit_db);
  const float wet_gain = *(plugin_data->wet_gain);
  const float res_gain = *(plugin_data->res_gain);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
	unsigned long i;
	for (i = 0; i < sample_count; i++)
	{
		float limit_g = pow(10, limit_db / 20);
		float sign = input[i] < 0.0 ? -1.0 : 1.0;
		float data = input[i] * sign;
		float residue = data > limit_g ? data - limit_g : 0.0;
		data -= residue;
		buffer_write(output[i],
			 sign * (wet_gain * data + res_gain * residue));
	}
    
}

static const LV2_Descriptor hardLimiterDescriptor = {
  "http://plugin.org.uk/swh-plugins/hardLimiter",
  instantiateHardLimiter,
  connectPortHardLimiter,
  NULL,
  runHardLimiter,
  NULL,
  cleanupHardLimiter,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &hardLimiterDescriptor;
  default:
    return NULL;
  }
}
