
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Foverdrive {
  float *drive;
  float *input;
  float *output;
} Foverdrive;

static void cleanupFoverdrive(LV2_Handle instance)
{

  free(instance);
}

static void connectPortFoverdrive(LV2_Handle instance, uint32_t port, void *data)
{
  Foverdrive *plugin = (Foverdrive *)instance;

  switch (port) {
  case 0:
    plugin->drive = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateFoverdrive(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Foverdrive *plugin_data = (Foverdrive *)malloc(sizeof(Foverdrive));
  
  
  return (LV2_Handle)plugin_data;
}



static void runFoverdrive(LV2_Handle instance, uint32_t sample_count)
{
  Foverdrive *plugin_data = (Foverdrive *)instance;

  const float drive = *(plugin_data->drive);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
			unsigned long pos;
			const float drivem1 = drive - 1.0f;

			for (pos = 0; pos < sample_count; pos++) {
				float x = input[pos];
				const float fx = fabs(x);
				output[pos] = x*(fx + drive)/(x*x + drivem1*fx + 1.0f);
			}
		
}

static const LV2_Descriptor foverdriveDescriptor = {
  "http://plugin.org.uk/swh-plugins/foverdrive",
  instantiateFoverdrive,
  connectPortFoverdrive,
  NULL,
  runFoverdrive,
  NULL,
  cleanupFoverdrive,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &foverdriveDescriptor;
  default:
    return NULL;
  }
}
