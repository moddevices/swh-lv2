
      #include "util/waveguide_nl.h"

      /* required for clang compilation */
      void waveguide_nl_process(waveguide_nl *wg, float in0, float in1, float *out0, float *out1);

      #define RUN_WG(n, junct_a, junct_b) waveguide_nl_process(w[n], junct_a - out[n*2+1], junct_b - out[n*2], out+n*2, out+n*2+1)
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Gong {
  float *damp_i;
  float *damp_o;
  float *micpos;
  float *scale0;
  float *apa0;
  float *apb0;
  float *scale1;
  float *apa1;
  float *apb1;
  float *scale2;
  float *apa2;
  float *apb2;
  float *scale3;
  float *apa3;
  float *apb3;
  float *scale4;
  float *apa4;
  float *apb4;
  float *scale5;
  float *apa5;
  float *apb5;
  float *scale6;
  float *apa6;
  float *apb6;
  float *scale7;
  float *apa7;
  float *apb7;
  float *input;
  float *output;
waveguide_nl ** w;
float * out;
int maxsize_i;
int maxsize_o;
} Gong;

static void cleanupGong(LV2_Handle instance)
{
Gong *plugin_data = (Gong *)instance;

      unsigned int i;

      for (i = 0; i < 8; i++) {
        waveguide_nl_free(plugin_data->w[i]);
      }
      free(plugin_data->w);
      free(plugin_data->out);
    
  free(instance);
}

static void connectPortGong(LV2_Handle instance, uint32_t port, void *data)
{
  Gong *plugin = (Gong *)instance;

  switch (port) {
  case 0:
    plugin->damp_i = data;
    break;
  case 1:
    plugin->damp_o = data;
    break;
  case 2:
    plugin->micpos = data;
    break;
  case 3:
    plugin->scale0 = data;
    break;
  case 4:
    plugin->apa0 = data;
    break;
  case 5:
    plugin->apb0 = data;
    break;
  case 6:
    plugin->scale1 = data;
    break;
  case 7:
    plugin->apa1 = data;
    break;
  case 8:
    plugin->apb1 = data;
    break;
  case 9:
    plugin->scale2 = data;
    break;
  case 10:
    plugin->apa2 = data;
    break;
  case 11:
    plugin->apb2 = data;
    break;
  case 12:
    plugin->scale3 = data;
    break;
  case 13:
    plugin->apa3 = data;
    break;
  case 14:
    plugin->apb3 = data;
    break;
  case 15:
    plugin->scale4 = data;
    break;
  case 16:
    plugin->apa4 = data;
    break;
  case 17:
    plugin->apb4 = data;
    break;
  case 18:
    plugin->scale5 = data;
    break;
  case 19:
    plugin->apa5 = data;
    break;
  case 20:
    plugin->apb5 = data;
    break;
  case 21:
    plugin->scale6 = data;
    break;
  case 22:
    plugin->apa6 = data;
    break;
  case 23:
    plugin->apb6 = data;
    break;
  case 24:
    plugin->scale7 = data;
    break;
  case 25:
    plugin->apa7 = data;
    break;
  case 26:
    plugin->apb7 = data;
    break;
  case 27:
    plugin->input = data;
    break;
  case 28:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateGong(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Gong *plugin_data = (Gong *)malloc(sizeof(Gong));
  waveguide_nl ** w = plugin_data->w;
  float * out = plugin_data->out;
  int maxsize_i = plugin_data->maxsize_i;
  int maxsize_o = plugin_data->maxsize_o;
  
      /* Max delay length for inner waveguides */
      maxsize_i = (float)s_rate * 0.03643242f;
      /* Max delay length for outer waveguides */
      maxsize_o = (float)s_rate * 0.05722782f;

      /* The waveguide structures */
      w = malloc(8 * sizeof(waveguide_nl *));
      w[0] = waveguide_nl_new(maxsize_i, 0.5, 0.0f, 0.0f);
      w[1] = waveguide_nl_new(maxsize_i, 0.5, 0.0f, 0.0f);
      w[2] = waveguide_nl_new(maxsize_i, 0.5, 0.0f, 0.0f);
      w[3] = waveguide_nl_new(maxsize_i, 0.5, 0.0f, 0.0f);
      w[4] = waveguide_nl_new(maxsize_o, 0.5, 0.0f, 0.0f);
      w[5] = waveguide_nl_new(maxsize_o, 0.5, 0.0f, 0.0f);
      w[6] = waveguide_nl_new(maxsize_o, 0.5, 0.0f, 0.0f);
      w[7] = waveguide_nl_new(maxsize_o, 0.5, 0.0f, 0.0f);

      /* Buffers to hold the currect deflections */
      out = calloc(32, sizeof(float));
    
  plugin_data->w = w;
  plugin_data->out = out;
  plugin_data->maxsize_i = maxsize_i;
  plugin_data->maxsize_o = maxsize_o;
  
  return (LV2_Handle)plugin_data;
}


static void activateGong(LV2_Handle instance)
{
  Gong *plugin_data = (Gong *)instance;
  waveguide_nl ** w __attribute__ ((unused)) = plugin_data->w;
  float * out __attribute__ ((unused)) = plugin_data->out;
  int maxsize_i __attribute__ ((unused)) = plugin_data->maxsize_i;
  int maxsize_o __attribute__ ((unused)) = plugin_data->maxsize_o;
  
      unsigned int i;

      for (i = 0; i < 8; i++) {
        waveguide_nl_reset(w[i]);
      }
    
}


static void runGong(LV2_Handle instance, uint32_t sample_count)
{
  Gong *plugin_data = (Gong *)instance;

  const float damp_i = *(plugin_data->damp_i);
  const float damp_o = *(plugin_data->damp_o);
  const float micpos = *(plugin_data->micpos);
  const float scale0 = *(plugin_data->scale0);
  const float apa0 = *(plugin_data->apa0);
  const float apb0 = *(plugin_data->apb0);
  const float scale1 = *(plugin_data->scale1);
  const float apa1 = *(plugin_data->apa1);
  const float apb1 = *(plugin_data->apb1);
  const float scale2 = *(plugin_data->scale2);
  const float apa2 = *(plugin_data->apa2);
  const float apb2 = *(plugin_data->apb2);
  const float scale3 = *(plugin_data->scale3);
  const float apa3 = *(plugin_data->apa3);
  const float apb3 = *(plugin_data->apb3);
  const float scale4 = *(plugin_data->scale4);
  const float apa4 = *(plugin_data->apa4);
  const float apb4 = *(plugin_data->apb4);
  const float scale5 = *(plugin_data->scale5);
  const float apa5 = *(plugin_data->apa5);
  const float apb5 = *(plugin_data->apb5);
  const float scale6 = *(plugin_data->scale6);
  const float apa6 = *(plugin_data->apa6);
  const float apb6 = *(plugin_data->apb6);
  const float scale7 = *(plugin_data->scale7);
  const float apa7 = *(plugin_data->apa7);
  const float apb7 = *(plugin_data->apb7);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  waveguide_nl ** w = plugin_data->w;
  float * out = plugin_data->out;
  int maxsize_i = plugin_data->maxsize_i;
  int maxsize_o = plugin_data->maxsize_o;
  
      unsigned long pos;
      /* The a coef of the inner lowpass */
      const float lpi = 1.0f - damp_i * 0.1423f;
      /* The a coef of the outer lowpass */
      const float lpo = 1.0f - damp_o * 0.19543f;

      /* Set the parameters of the waveguides */
      waveguide_nl_set_delay(w[0], maxsize_i * scale0);
      waveguide_nl_set_ap(w[0], apa0, apb0);
      waveguide_nl_set_delay(w[1], maxsize_i * scale1);
      waveguide_nl_set_ap(w[1], apa1, apb1);
      waveguide_nl_set_delay(w[2], maxsize_i * scale2);
      waveguide_nl_set_ap(w[2], apa2, apb2);
      waveguide_nl_set_delay(w[3], maxsize_i * scale3);
      waveguide_nl_set_ap(w[3], apa3, apb3);
      waveguide_nl_set_delay(w[4], maxsize_o * scale4);
      waveguide_nl_set_ap(w[4], apa4, apb4);
      waveguide_nl_set_delay(w[5], maxsize_o * scale5);
      waveguide_nl_set_ap(w[5], apa5, apb5);
      waveguide_nl_set_delay(w[6], maxsize_o * scale6);
      waveguide_nl_set_ap(w[6], apa6, apb6);
      waveguide_nl_set_delay(w[7], maxsize_o * scale7);
      waveguide_nl_set_ap(w[7], apa7, apb7);

      for (pos=0; pos<4; pos++) {
	waveguide_nl_set_fc(w[pos], lpi);
      }
      for (; pos<8; pos++) {
	waveguide_nl_set_fc(w[pos], lpo);
      }

      for (pos = 0; pos < sample_count; pos++) {
        /* Calcualte the deflections at the wavejunctions
           alpha is the centre, beta is north, gamma is east,
           delta is south and epsilon is west */
	const float alpha = (out[0] + out[2] + out[4] + out[6]) * 0.5f
			    + input[pos];
	const float beta = (out[1] + out[9] + out[14]) * 0.666666666666f;
	const float gamma = (out[3] + out[8] + out[11]) * 0.666666666666f;
	const float delta = (out[5] + out[10] + out[13]) * 0.666666666666f;
	const float epsilon = (out[7] + out[12] + out[15]) * 0.666666666666f;

        /* Inject the energy at the junctions + reflections into the
           waveguides (the macro gives the reflection calcs) */
	RUN_WG(0, beta, alpha);
	RUN_WG(1, gamma, alpha);
	RUN_WG(2, delta, alpha);
	RUN_WG(3, epsilon, alpha);
	RUN_WG(4, beta, gamma);
	RUN_WG(5, gamma, delta);
	RUN_WG(6, delta, epsilon);
	RUN_WG(7, epsilon, beta);

        output[pos] = (1.0f - micpos) * alpha + micpos * delta;
      }
    
}

static const LV2_Descriptor gongDescriptor = {
  "http://plugin.org.uk/swh-plugins/gong",
  instantiateGong,
  connectPortGong,
  activateGong,
  runGong,
  NULL,
  cleanupGong,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &gongDescriptor;
  default:
    return NULL;
  }
}
