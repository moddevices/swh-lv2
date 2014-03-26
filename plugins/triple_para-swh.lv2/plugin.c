
      #include "util/biquad.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _TriplePara {
  float *gain_L;
  float *fc_L;
  float *bw_L;
  float *gain_1;
  float *fc_1;
  float *bw_1;
  float *gain_2;
  float *fc_2;
  float *bw_2;
  float *gain_3;
  float *fc_3;
  float *bw_3;
  float *gain_H;
  float *fc_H;
  float *bw_H;
  float *input;
  float *output;
float fs;
biquad * filters;
} TriplePara;

static void cleanupTriplePara(LV2_Handle instance)
{
TriplePara *plugin_data = (TriplePara *)instance;

free(plugin_data->filters);
    
  free(instance);
}

static void connectPortTriplePara(LV2_Handle instance, uint32_t port, void *data)
{
  TriplePara *plugin = (TriplePara *)instance;

  switch (port) {
  case 0:
    plugin->gain_L = data;
    break;
  case 1:
    plugin->fc_L = data;
    break;
  case 2:
    plugin->bw_L = data;
    break;
  case 3:
    plugin->gain_1 = data;
    break;
  case 4:
    plugin->fc_1 = data;
    break;
  case 5:
    plugin->bw_1 = data;
    break;
  case 6:
    plugin->gain_2 = data;
    break;
  case 7:
    plugin->fc_2 = data;
    break;
  case 8:
    plugin->bw_2 = data;
    break;
  case 9:
    plugin->gain_3 = data;
    break;
  case 10:
    plugin->fc_3 = data;
    break;
  case 11:
    plugin->bw_3 = data;
    break;
  case 12:
    plugin->gain_H = data;
    break;
  case 13:
    plugin->fc_H = data;
    break;
  case 14:
    plugin->bw_H = data;
    break;
  case 15:
    plugin->input = data;
    break;
  case 16:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateTriplePara(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  TriplePara *plugin_data = (TriplePara *)malloc(sizeof(TriplePara));
  float fs = plugin_data->fs;
  biquad * filters = plugin_data->filters;
  
fs = s_rate;

filters = calloc(5, sizeof(biquad));
biquad_init(&filters[0]);
biquad_init(&filters[1]);
biquad_init(&filters[2]);
biquad_init(&filters[3]);
biquad_init(&filters[4]);
    
  plugin_data->fs = fs;
  plugin_data->filters = filters;
  
  return (LV2_Handle)plugin_data;
}


static void activateTriplePara(LV2_Handle instance)
{
  TriplePara *plugin_data = (TriplePara *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  biquad * filters __attribute__ ((unused)) = plugin_data->filters;
  
biquad_init(&filters[0]);
biquad_init(&filters[1]);
biquad_init(&filters[2]);
biquad_init(&filters[3]);
biquad_init(&filters[4]);
    
}


static void runTriplePara(LV2_Handle instance, uint32_t sample_count)
{
  TriplePara *plugin_data = (TriplePara *)instance;

  const float gain_L = *(plugin_data->gain_L);
  const float fc_L = *(plugin_data->fc_L);
  const float bw_L = *(plugin_data->bw_L);
  const float gain_1 = *(plugin_data->gain_1);
  const float fc_1 = *(plugin_data->fc_1);
  const float bw_1 = *(plugin_data->bw_1);
  const float gain_2 = *(plugin_data->gain_2);
  const float fc_2 = *(plugin_data->fc_2);
  const float bw_2 = *(plugin_data->bw_2);
  const float gain_3 = *(plugin_data->gain_3);
  const float fc_3 = *(plugin_data->fc_3);
  const float bw_3 = *(plugin_data->bw_3);
  const float gain_H = *(plugin_data->gain_H);
  const float fc_H = *(plugin_data->fc_H);
  const float bw_H = *(plugin_data->bw_H);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float fs = plugin_data->fs;
  biquad * filters = plugin_data->filters;
  
unsigned long pos;
float in;

ls_set_params(&filters[0], fc_L, gain_L, bw_L, fs);
eq_set_params(&filters[1], fc_1, gain_1, bw_1, fs);
eq_set_params(&filters[2], fc_2, gain_2, bw_2, fs);
eq_set_params(&filters[3], fc_3, gain_3, bw_3, fs);
hs_set_params(&filters[4], fc_H, gain_H, bw_H, fs);

for (pos = 0; pos < sample_count; pos++) {
	in = biquad_run(&filters[0], input[pos]);
	in = biquad_run(&filters[1], in);
	in = biquad_run(&filters[2], in);
	in = biquad_run(&filters[3], in);
	in = biquad_run(&filters[4], in);
	buffer_write(output[pos], in);
}
    
}

static const LV2_Descriptor tripleParaDescriptor = {
  "http://plugin.org.uk/swh-plugins/triplePara",
  instantiateTriplePara,
  connectPortTriplePara,
  activateTriplePara,
  runTriplePara,
  NULL,
  cleanupTriplePara,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &tripleParaDescriptor;
  default:
    return NULL;
  }
}
