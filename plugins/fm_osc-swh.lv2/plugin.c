
      #include "ladspa-util.h"
      #include "util/blo.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _FmOsc {
  float *wave;
  float *fm;
  float *output;
blo_h_tables * tables;
blo_h_osc * osc;
} FmOsc;

static void cleanupFmOsc(LV2_Handle instance)
{
FmOsc *plugin_data = (FmOsc *)instance;

      blo_h_tables_free(plugin_data->tables);
      blo_h_free(plugin_data->osc);
    
  free(instance);
}

static void connectPortFmOsc(LV2_Handle instance, uint32_t port, void *data)
{
  FmOsc *plugin = (FmOsc *)instance;

  switch (port) {
  case 0:
    plugin->wave = data;
    break;
  case 1:
    plugin->fm = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateFmOsc(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  FmOsc *plugin_data = (FmOsc *)malloc(sizeof(FmOsc));
  blo_h_tables * tables = plugin_data->tables;
  blo_h_osc * osc = plugin_data->osc;
  
      tables = blo_h_tables_new(1024);
      osc = blo_h_new(tables, BLO_SINE, (float)s_rate);
    
  plugin_data->tables = tables;
  plugin_data->osc = osc;
  
  return (LV2_Handle)plugin_data;
}



static void runFmOsc(LV2_Handle instance, uint32_t sample_count)
{
  FmOsc *plugin_data = (FmOsc *)instance;

  const float wave = *(plugin_data->wave);
  const float * const fm = plugin_data->fm;
  float * const output = plugin_data->output;
  blo_h_tables * tables = plugin_data->tables;
  blo_h_osc * osc = plugin_data->osc;
  
      unsigned long pos;
      osc->wave = LIMIT(f_round(wave) - 1, 0, BLO_N_WAVES-1);

      tables = tables; // So gcc doesn't think it's unused

      for (pos = 0; pos < sample_count; pos++) {
	blo_hd_set_freq(osc, fm[pos]);
        buffer_write(output[pos], blo_hd_run_cub(osc));
      }
    
}

static const LV2_Descriptor fmOscDescriptor = {
  "http://plugin.org.uk/swh-plugins/fmOsc",
  instantiateFmOsc,
  connectPortFmOsc,
  NULL,
  runFmOsc,
  NULL,
  cleanupFmOsc,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &fmOscDescriptor;
  default:
    return NULL;
  }
}
