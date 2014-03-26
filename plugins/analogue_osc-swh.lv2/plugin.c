
      #include <math.h>

      #include "ladspa-util.h"
      #include "util/blo.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _AnalogueOsc {
  float *wave;
  float *freq;
  float *warm;
  float *instab;
  float *output;
blo_h_tables * tables;
blo_h_osc * osc;
float fs;
float itm1;
float otm1;
float otm2;
unsigned int rnda;
unsigned int rndb;
} AnalogueOsc;

static void cleanupAnalogueOsc(LV2_Handle instance)
{
AnalogueOsc *plugin_data = (AnalogueOsc *)instance;

      blo_h_tables_free(plugin_data->tables);
      blo_h_free(plugin_data->osc);
    
  free(instance);
}

static void connectPortAnalogueOsc(LV2_Handle instance, uint32_t port, void *data)
{
  AnalogueOsc *plugin = (AnalogueOsc *)instance;

  switch (port) {
  case 0:
    plugin->wave = data;
    break;
  case 1:
    plugin->freq = data;
    break;
  case 2:
    plugin->warm = data;
    break;
  case 3:
    plugin->instab = data;
    break;
  case 4:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateAnalogueOsc(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  AnalogueOsc *plugin_data = (AnalogueOsc *)malloc(sizeof(AnalogueOsc));
  blo_h_tables * tables = plugin_data->tables;
  blo_h_osc * osc = plugin_data->osc;
  float fs = plugin_data->fs;
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  float otm2 = plugin_data->otm2;
  unsigned int rnda = plugin_data->rnda;
  unsigned int rndb = plugin_data->rndb;
  
      tables = blo_h_tables_new(512);
      osc = blo_h_new(tables, BLO_SINE, (float)s_rate);
      fs = (float)s_rate;
      itm1 = 0.0f;
      otm1 = 0.0f;
      otm2 = 0.0f;
      rnda = 43437;
      rndb = 111145;
    
  plugin_data->tables = tables;
  plugin_data->osc = osc;
  plugin_data->fs = fs;
  plugin_data->itm1 = itm1;
  plugin_data->otm1 = otm1;
  plugin_data->otm2 = otm2;
  plugin_data->rnda = rnda;
  plugin_data->rndb = rndb;
  
  return (LV2_Handle)plugin_data;
}



static void runAnalogueOsc(LV2_Handle instance, uint32_t sample_count)
{
  AnalogueOsc *plugin_data = (AnalogueOsc *)instance;

  const float wave = *(plugin_data->wave);
  const float freq = *(plugin_data->freq);
  const float warm = *(plugin_data->warm);
  const float instab = *(plugin_data->instab);
  float * const output = plugin_data->output;
  blo_h_tables * tables = plugin_data->tables;
  blo_h_osc * osc = plugin_data->osc;
  float fs = plugin_data->fs;
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  float otm2 = plugin_data->otm2;
  unsigned int rnda = plugin_data->rnda;
  unsigned int rndb = plugin_data->rndb;
  
      unsigned long pos;
      float x, y;
      const float q = warm - 0.999f;
      const float leak = 1.0f - warm * 0.02f;
      const unsigned int max_jump = (unsigned int)f_round(instab * 30000.0f) + 1;

      osc->wave = LIMIT(f_round(wave) - 1, 0, BLO_N_WAVES-1);
      osc->nyquist = fs * (0.47f - f_clamp(warm, 0.0f, 1.0f) * 0.41f);
      blo_hd_set_freq(osc, freq);

      tables = tables; // So gcc doesn't think it's unused

      for (pos = 0; pos < sample_count; pos++) {
	x = blo_hd_run_cub(osc);
        rnda += 432577;
        rnda *= 47;
	rndb += 7643113;
        rnda *= 59;
        osc->ph.all += (((rnda + rndb)/2) % max_jump) - max_jump/2;
        osc->ph.all &= osc->ph_mask;
	y = (x - q) / (1.0f - f_exp(-1.2f * (x - q))) +
              q / (1.0f - f_exp(1.2f * q));
	/* Catch the case where x ~= q */
	if (fabs(y) > 1.0f) {
		y = 0.83333f + q / (1.0f - f_exp(1.2f * q));
	}
	otm2 = otm1;
        otm1 = leak * otm1 + y - itm1;
        itm1 = y;

        buffer_write(output[pos], (otm1 + otm2) * 0.5f);
      }

      plugin_data->itm1 = itm1;
      plugin_data->otm1 = otm1;
      plugin_data->otm2 = otm2;
      plugin_data->rnda = rnda;
      plugin_data->rndb = rndb;
    
}

static const LV2_Descriptor analogueOscDescriptor = {
  "http://plugin.org.uk/swh-plugins/analogueOsc",
  instantiateAnalogueOsc,
  connectPortAnalogueOsc,
  NULL,
  runAnalogueOsc,
  NULL,
  cleanupAnalogueOsc,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &analogueOscDescriptor;
  default:
    return NULL;
  }
}
