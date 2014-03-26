
      #include "ladspa-util.h"
      #include "util/ls_filter.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _LsFilter {
  float *type;
  float *cutoff;
  float *resonance;
  float *input;
  float *output;
float fs;
ls_filt * filt;
} LsFilter;

static void cleanupLsFilter(LV2_Handle instance)
{
LsFilter *plugin_data = (LsFilter *)instance;

      free(plugin_data->filt);
    
  free(instance);
}

static void connectPortLsFilter(LV2_Handle instance, uint32_t port, void *data)
{
  LsFilter *plugin = (LsFilter *)instance;

  switch (port) {
  case 0:
    plugin->type = data;
    break;
  case 1:
    plugin->cutoff = data;
    break;
  case 2:
    plugin->resonance = data;
    break;
  case 3:
    plugin->input = data;
    break;
  case 4:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateLsFilter(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  LsFilter *plugin_data = (LsFilter *)malloc(sizeof(LsFilter));
  float fs = plugin_data->fs;
  ls_filt * filt = plugin_data->filt;
  
      filt = malloc(sizeof(ls_filt));
      fs = s_rate;
    
  plugin_data->fs = fs;
  plugin_data->filt = filt;
  
  return (LV2_Handle)plugin_data;
}


static void activateLsFilter(LV2_Handle instance)
{
  LsFilter *plugin_data = (LsFilter *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  ls_filt * filt __attribute__ ((unused)) = plugin_data->filt;
  
      ls_filt_init(filt);
    
}


static void runLsFilter(LV2_Handle instance, uint32_t sample_count)
{
  LsFilter *plugin_data = (LsFilter *)instance;

  const float type = *(plugin_data->type);
  const float cutoff = *(plugin_data->cutoff);
  const float resonance = *(plugin_data->resonance);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float fs = plugin_data->fs;
  ls_filt * filt = plugin_data->filt;
  
      unsigned long pos;
      const ls_filt_type t = (ls_filt_type)f_round(type);

      ls_filt_setup(filt, t, cutoff, resonance, fs);

      for (pos = 0; pos < sample_count; pos++) {
        buffer_write(output[pos], ls_filt_run(filt, input[pos]));
      }
	
    
}

static const LV2_Descriptor lsFilterDescriptor = {
  "http://plugin.org.uk/swh-plugins/lsFilter",
  instantiateLsFilter,
  connectPortLsFilter,
  activateLsFilter,
  runLsFilter,
  NULL,
  cleanupLsFilter,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &lsFilterDescriptor;
  default:
    return NULL;
  }
}
