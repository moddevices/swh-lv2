
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Diode {
  float *mode;
  float *input;
  float *output;
} Diode;

static void cleanupDiode(LV2_Handle instance)
{

  free(instance);
}

static void connectPortDiode(LV2_Handle instance, uint32_t port, void *data)
{
  Diode *plugin = (Diode *)instance;

  switch (port) {
  case 0:
    plugin->mode = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateDiode(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Diode *plugin_data = (Diode *)malloc(sizeof(Diode));
  
  
  return (LV2_Handle)plugin_data;
}



static void runDiode(LV2_Handle instance, uint32_t sample_count)
{
  Diode *plugin_data = (Diode *)instance;

  const float mode = *(plugin_data->mode);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
unsigned long pos;

if (mode >= 0.0f && mode < 1.0f) {
	for (pos = 0; pos < sample_count; pos++) {
		output[pos] = ((1.0f-mode) * input[pos]) +
		 (mode * (input[pos] > 0.0f ? input[pos] : 0.0f));
	}
} else if (mode >= 1.0f && mode < 2.0f) {
	float fac = mode - 1.0f;
	for (pos = 0; pos < sample_count; pos++) {
		output[pos] =((1.0f-fac) * (input[pos] > 0 ?
		 input[pos] : 0.0)) + (fac * fabs(input[pos]));
	}
} else if (mode >= 2) {
	float fac = mode < 3 ? mode - 2 : 1.0;
	for (pos = 0; pos < sample_count; pos++) {
		output[pos] = (1.0-fac) * fabs(input[pos]);
	}
} else {
	for (pos = 0; pos < sample_count; pos++) {
		output[pos] = input[pos];
	}
}
		
}

static const LV2_Descriptor diodeDescriptor = {
  "http://plugin.org.uk/swh-plugins/diode",
  instantiateDiode,
  connectPortDiode,
  NULL,
  runDiode,
  NULL,
  cleanupDiode,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &diodeDescriptor;
  default:
    return NULL;
  }
}
