
#include "ladspa-util.h"

#define D_SIZE 256
#define NZEROS 200

/* The non-zero taps of the Hilbert transformer */
static float xcoeffs[] = {
     +0.0008103736f, +0.0008457886f, +0.0009017196f, +0.0009793364f,
     +0.0010798341f, +0.0012044365f, +0.0013544008f, +0.0015310235f,
     +0.0017356466f, +0.0019696659f, +0.0022345404f, +0.0025318040f,
     +0.0028630784f, +0.0032300896f, +0.0036346867f, +0.0040788644f,
     +0.0045647903f, +0.0050948365f, +0.0056716186f, +0.0062980419f,
     +0.0069773575f, +0.0077132300f, +0.0085098208f, +0.0093718901f,
     +0.0103049226f, +0.0113152847f, +0.0124104218f, +0.0135991079f,
     +0.0148917649f, +0.0163008758f, +0.0178415242f, +0.0195321089f,
     +0.0213953037f, +0.0234593652f, +0.0257599469f, +0.0283426636f,
     +0.0312667947f, +0.0346107648f, +0.0384804823f, +0.0430224431f,
     +0.0484451086f, +0.0550553725f, +0.0633242001f, +0.0740128560f,
     +0.0884368322f, +0.1090816773f, +0.1412745301f, +0.1988673273f,
     +0.3326528346f, +0.9997730178f, -0.9997730178f, -0.3326528346f,
     -0.1988673273f, -0.1412745301f, -0.1090816773f, -0.0884368322f,
     -0.0740128560f, -0.0633242001f, -0.0550553725f, -0.0484451086f,
     -0.0430224431f, -0.0384804823f, -0.0346107648f, -0.0312667947f,
     -0.0283426636f, -0.0257599469f, -0.0234593652f, -0.0213953037f,
     -0.0195321089f, -0.0178415242f, -0.0163008758f, -0.0148917649f,
     -0.0135991079f, -0.0124104218f, -0.0113152847f, -0.0103049226f,
     -0.0093718901f, -0.0085098208f, -0.0077132300f, -0.0069773575f,
     -0.0062980419f, -0.0056716186f, -0.0050948365f, -0.0045647903f,
     -0.0040788644f, -0.0036346867f, -0.0032300896f, -0.0028630784f,
     -0.0025318040f, -0.0022345404f, -0.0019696659f, -0.0017356466f,
     -0.0015310235f, -0.0013544008f, -0.0012044365f, -0.0010798341f,
     -0.0009793364f, -0.0009017196f, -0.0008457886f, -0.0008103736f,
};
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Hilbert {
  float *input;
  float *output0;
  float *output90;
  float *latency;
float * delay;
unsigned int dptr;
} Hilbert;

static void cleanupHilbert(LV2_Handle instance)
{
Hilbert *plugin_data = (Hilbert *)instance;

      free(plugin_data->delay);
    
  free(instance);
}

static void connectPortHilbert(LV2_Handle instance, uint32_t port, void *data)
{
  Hilbert *plugin = (Hilbert *)instance;

  switch (port) {
  case 0:
    plugin->input = data;
    break;
  case 1:
    plugin->output0 = data;
    break;
  case 2:
    plugin->output90 = data;
    break;
  case 3:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiateHilbert(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Hilbert *plugin_data = (Hilbert *)malloc(sizeof(Hilbert));
  float * delay = plugin_data->delay;
  unsigned int dptr = plugin_data->dptr;
  
      delay = calloc(D_SIZE, sizeof(float));

      dptr = 0;
    
  plugin_data->delay = delay;
  plugin_data->dptr = dptr;
  
  return (LV2_Handle)plugin_data;
}



static void runHilbert(LV2_Handle instance, uint32_t sample_count)
{
  Hilbert *plugin_data = (Hilbert *)instance;

  const float * const input = plugin_data->input;
  float * const output0 = plugin_data->output0;
  float * const output90 = plugin_data->output90;
  float latency;
  float * delay = plugin_data->delay;
  unsigned int dptr = plugin_data->dptr;
  
      unsigned long pos;
      unsigned int i;
      float hilb;

      for (pos = 0; pos < sample_count; pos++) {
	delay[dptr] = input[pos];
	hilb = 0.0f;
	for (i = 0; i < NZEROS/2; i++) {
	  hilb += (xcoeffs[i] * delay[(dptr - i*2) & (D_SIZE - 1)]);
	}
        buffer_write(output0[pos], delay[(dptr - 99) & (D_SIZE - 1)]);
        buffer_write(output90[pos], hilb);
	dptr = (dptr + 1) & (D_SIZE - 1);
      }

      plugin_data->dptr = dptr;

      *(plugin_data->latency) = 99;
    
}

static const LV2_Descriptor hilbertDescriptor = {
  "http://plugin.org.uk/swh-plugins/hilbert",
  instantiateHilbert,
  connectPortHilbert,
  NULL,
  runHilbert,
  NULL,
  cleanupHilbert,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &hilbertDescriptor;
  default:
    return NULL;
  }
}
