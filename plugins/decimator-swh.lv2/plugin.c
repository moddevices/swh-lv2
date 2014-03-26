
      #include <math.h>
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Decimator {
  float *bits;
  float *fs;
  float *input;
  float *output;
long sample_rate;
float count;
float last_out;
} Decimator;

static void cleanupDecimator(LV2_Handle instance)
{

  free(instance);
}

static void connectPortDecimator(LV2_Handle instance, uint32_t port, void *data)
{
  Decimator *plugin = (Decimator *)instance;

  switch (port) {
  case 0:
    plugin->bits = data;
    break;
  case 1:
    plugin->fs = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateDecimator(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Decimator *plugin_data = (Decimator *)malloc(sizeof(Decimator));
  long sample_rate = plugin_data->sample_rate;
  float count = plugin_data->count;
  float last_out = plugin_data->last_out;
  
sample_rate = s_rate;
count = 0.0f;
last_out = 0.0f;
    
  plugin_data->sample_rate = sample_rate;
  plugin_data->count = count;
  plugin_data->last_out = last_out;
  
  return (LV2_Handle)plugin_data;
}



static void runDecimator(LV2_Handle instance, uint32_t sample_count)
{
  Decimator *plugin_data = (Decimator *)instance;

  const float bits = *(plugin_data->bits);
  const float fs = *(plugin_data->fs);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  long sample_rate = plugin_data->sample_rate;
  float count = plugin_data->count;
  float last_out = plugin_data->last_out;
  
unsigned long pos;
float step, stepr, delta, ratio;
double dummy;

if (bits >= 31.0f || bits < 1.0f) {
	step = 0.0f;
	stepr = 1.0f;
} else {
	step = pow(0.5f, bits - 0.999f);
	stepr = 1/step;
}

if (fs >= sample_rate) {
	ratio = 1.0f;
} else {
	ratio = fs/sample_rate;
}

for (pos = 0; pos < sample_count; pos++) {
	count += ratio;

	if (count >= 1.0f) {
		count -= 1.0f;
		delta = modf((input[pos] + (input[pos]<0?-1.0:1.0)*step*0.5) * stepr, &dummy) * step;
		last_out = input[pos] - delta;
		buffer_write(output[pos], last_out);
	} else {
		buffer_write(output[pos], last_out);
	}
}

plugin_data->last_out = last_out;
plugin_data->count = count;
    
}

static const LV2_Descriptor decimatorDescriptor = {
  "http://plugin.org.uk/swh-plugins/decimator",
  instantiateDecimator,
  connectPortDecimator,
  NULL,
  runDecimator,
  NULL,
  cleanupDecimator,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &decimatorDescriptor;
  default:
    return NULL;
  }
}
