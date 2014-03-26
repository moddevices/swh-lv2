
      #include "util/biquad.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _SinglePara {
  float *gain;
  float *fc;
  float *bw;
  float *input;
  float *output;
float fs;
biquad * filter;
} SinglePara;

static void cleanupSinglePara(LV2_Handle instance)
{
SinglePara *plugin_data = (SinglePara *)instance;

      free(plugin_data->filter);
    
  free(instance);
}

static void connectPortSinglePara(LV2_Handle instance, uint32_t port, void *data)
{
  SinglePara *plugin = (SinglePara *)instance;

  switch (port) {
  case 0:
    plugin->gain = data;
    break;
  case 1:
    plugin->fc = data;
    break;
  case 2:
    plugin->bw = data;
    break;
  case 3:
    plugin->input = data;
    break;
  case 4:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateSinglePara(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  SinglePara *plugin_data = (SinglePara *)malloc(sizeof(SinglePara));
  float fs = plugin_data->fs;
  biquad * filter = plugin_data->filter;
  
      fs = (float)s_rate;
      filter = malloc(sizeof(biquad));
      biquad_init(filter);
    
  plugin_data->fs = fs;
  plugin_data->filter = filter;
  
  return (LV2_Handle)plugin_data;
}


static void activateSinglePara(LV2_Handle instance)
{
  SinglePara *plugin_data = (SinglePara *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  biquad * filter __attribute__ ((unused)) = plugin_data->filter;
  
      biquad_init(filter);
    
}


static void runSinglePara(LV2_Handle instance, uint32_t sample_count)
{
  SinglePara *plugin_data = (SinglePara *)instance;

  const float gain = *(plugin_data->gain);
  const float fc = *(plugin_data->fc);
  const float bw = *(plugin_data->bw);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float fs = plugin_data->fs;
  biquad * filter = plugin_data->filter;
  
      unsigned long pos;

      eq_set_params(filter, fc, gain, bw, fs);

      for (pos = 0; pos < sample_count; pos++) {
	buffer_write(output[pos], biquad_run(filter, input[pos]));
      }
    
}

static const LV2_Descriptor singleParaDescriptor = {
  "http://plugin.org.uk/swh-plugins/singlePara",
  instantiateSinglePara,
  connectPortSinglePara,
  activateSinglePara,
  runSinglePara,
  NULL,
  cleanupSinglePara,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &singleParaDescriptor;
  default:
    return NULL;
  }
}
