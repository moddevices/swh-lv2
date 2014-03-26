
			#define MAX_AMP 1.0f
			#define CLIP 0.8f
			#define CLIP_A ((MAX_AMP - CLIP) * (MAX_AMP - CLIP))
			#define CLIP_B (MAX_AMP - 2.0f * CLIP)
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Declip {
  float *input;
  float *output;
} Declip;

static void cleanupDeclip(LV2_Handle instance)
{

  free(instance);
}

static void connectPortDeclip(LV2_Handle instance, uint32_t port, void *data)
{
  Declip *plugin = (Declip *)instance;

  switch (port) {
  case 0:
    plugin->input = data;
    break;
  case 1:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateDeclip(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Declip *plugin_data = (Declip *)malloc(sizeof(Declip));
  
  
  return (LV2_Handle)plugin_data;
}



static void runDeclip(LV2_Handle instance, uint32_t sample_count)
{
  Declip *plugin_data = (Declip *)instance;

  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  
			unsigned long pos;

			for (pos = 0; pos < sample_count; pos++) {
				const float in = input[pos];

				if((in < CLIP) && (in > -CLIP)) {
					output[pos] = in;
				} else if (in > 0.0f) {
					output[pos] = MAX_AMP - (CLIP_A / (CLIP_B + in));
				} else {
					output[pos] = -(MAX_AMP - (CLIP_A / (CLIP_B - in)));
				}
			}
		
}

static const LV2_Descriptor declipDescriptor = {
  "http://plugin.org.uk/swh-plugins/declip",
  instantiateDeclip,
  connectPortDeclip,
  NULL,
  runDeclip,
  NULL,
  cleanupDeclip,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &declipDescriptor;
  default:
    return NULL;
  }
}
