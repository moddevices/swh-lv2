
      #include "util/waveguide_nl.h"

      #define LP_INNER 0.96f
      #define LP_OUTER 0.983f

      /* required for clang compilation */
      void waveguide_nl_process_lin(waveguide_nl *wg, float in0, float in1, float *out0, float *out1);

      #define RUN_WG(n, junct_a, junct_b) waveguide_nl_process_lin(w[n], junct_a - out[n*2+1], junct_b - out[n*2], out+n*2, out+n*2+1)
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Plate {
  float *time;
  float *damping;
  float *wet;
  float *input;
  float *outputl;
  float *outputr;
waveguide_nl ** w;
float * out;
} Plate;

static void cleanupPlate(LV2_Handle instance)
{
Plate *plugin_data = (Plate *)instance;

      unsigned int i;

      for (i = 0; i < 8; i++) {
	waveguide_nl_free(plugin_data->w[i]);
      }
      free(plugin_data->w);
      free(plugin_data->out);
    
  free(instance);
}

static void connectPortPlate(LV2_Handle instance, uint32_t port, void *data)
{
  Plate *plugin = (Plate *)instance;

  switch (port) {
  case 0:
    plugin->time = data;
    break;
  case 1:
    plugin->damping = data;
    break;
  case 2:
    plugin->wet = data;
    break;
  case 3:
    plugin->input = data;
    break;
  case 4:
    plugin->outputl = data;
    break;
  case 5:
    plugin->outputr = data;
    break;
  }
}

static LV2_Handle instantiatePlate(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Plate *plugin_data = (Plate *)malloc(sizeof(Plate));
  waveguide_nl ** w = plugin_data->w;
  float * out = plugin_data->out;
  
      w = malloc(8 * sizeof(waveguide_nl *));
      w[0] = waveguide_nl_new(2389, LP_INNER, 0.04f, 0.0f);
      w[1] = waveguide_nl_new(4742, LP_INNER, 0.17f, 0.0f);
      w[2] = waveguide_nl_new(4623, LP_INNER, 0.52f, 0.0f);
      w[3] = waveguide_nl_new(2142, LP_INNER, 0.48f, 0.0f);
      w[4] = waveguide_nl_new(5597, LP_OUTER, 0.32f, 0.0f);
      w[5] = waveguide_nl_new(3692, LP_OUTER, 0.89f, 0.0f);
      w[6] = waveguide_nl_new(5611, LP_OUTER, 0.28f, 0.0f);
      w[7] = waveguide_nl_new(3703, LP_OUTER, 0.29f, 0.0f);

      out = calloc(32, sizeof(float));
    
  plugin_data->w = w;
  plugin_data->out = out;
  
  return (LV2_Handle)plugin_data;
}


static void activatePlate(LV2_Handle instance)
{
  Plate *plugin_data = (Plate *)instance;
  waveguide_nl ** w __attribute__ ((unused)) = plugin_data->w;
  float * out __attribute__ ((unused)) = plugin_data->out;
  
      unsigned int i;

      for (i = 0; i < 8; i++) {
	waveguide_nl_reset(w[i]);
      }
    
}


static void runPlate(LV2_Handle instance, uint32_t sample_count)
{
  Plate *plugin_data = (Plate *)instance;

  const float time = *(plugin_data->time);
  const float damping = *(plugin_data->damping);
  const float wet = *(plugin_data->wet);
  const float * const input = plugin_data->input;
  float * const outputl = plugin_data->outputl;
  float * const outputr = plugin_data->outputr;
  waveguide_nl ** w = plugin_data->w;
  float * out = plugin_data->out;
  
      unsigned long pos;
      const float scale = powf(time * 0.117647f, 1.34f);
      const float lpscale = 1.0f - damping * 0.93;

      for (pos=0; pos<8; pos++) {
	waveguide_nl_set_delay(w[pos], w[pos]->size * scale);
      }
      for (pos=0; pos<4; pos++) {
	waveguide_nl_set_fc(w[pos], LP_INNER * lpscale);
      }
      for (; pos<8; pos++) {
	waveguide_nl_set_fc(w[pos], LP_OUTER * lpscale);
      }

      for (pos = 0; pos < sample_count; pos++) {
	const float alpha = (out[0] + out[2] + out[4] + out[6]) * 0.5f
			    + input[pos];
	const float beta = (out[1] + out[9] + out[14]) * 0.666666666f;
	const float gamma = (out[3] + out[8] + out[11]) * 0.666666666f;
	const float delta = (out[5] + out[10] + out[13]) * 0.666666666f;
	const float epsilon = (out[7] + out[12] + out[15]) * 0.666666666f;

	RUN_WG(0, beta, alpha);
	RUN_WG(1, gamma, alpha);
	RUN_WG(2, delta, alpha);
	RUN_WG(3, epsilon, alpha);
	RUN_WG(4, beta, gamma);
	RUN_WG(5, gamma, delta);
	RUN_WG(6, delta, epsilon);
	RUN_WG(7, epsilon, beta);

        outputl[pos] = beta * wet + input[pos] * (1.0f - wet);
        outputr[pos] = gamma * wet + input[pos] * (1.0f - wet);
      }
    
}

static const LV2_Descriptor plateDescriptor = {
  "http://plugin.org.uk/swh-plugins/plate",
  instantiatePlate,
  connectPortPlate,
  activatePlate,
  runPlate,
  NULL,
  cleanupPlate,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &plateDescriptor;
  default:
    return NULL;
  }
}
