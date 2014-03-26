
			#include "util/iir.h"
                        
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Bandpass_a_iir {
  float *center;
  float *width;
  float *input;
  float *output;
iirf_t* iirf;
iir_stage_t* gt;
long sample_rate;
} Bandpass_a_iir;

static void cleanupBandpass_a_iir(LV2_Handle instance)
{
Bandpass_a_iir *plugin_data = (Bandpass_a_iir *)instance;

                  free_iirf_t(plugin_data->iirf, plugin_data->gt);
                  free_iir_stage(plugin_data->gt);
                
  free(instance);
}

static void connectPortBandpass_a_iir(LV2_Handle instance, uint32_t port, void *data)
{
  Bandpass_a_iir *plugin = (Bandpass_a_iir *)instance;

  switch (port) {
  case 0:
    plugin->center = data;
    break;
  case 1:
    plugin->width = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateBandpass_a_iir(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Bandpass_a_iir *plugin_data = (Bandpass_a_iir *)malloc(sizeof(Bandpass_a_iir));
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  sample_rate = s_rate;
                
  plugin_data->iirf = iirf;
  plugin_data->gt = gt;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activateBandpass_a_iir(LV2_Handle instance)
{
  Bandpass_a_iir *plugin_data = (Bandpass_a_iir *)instance;
  iirf_t* iirf __attribute__ ((unused)) = plugin_data->iirf;
  iir_stage_t* gt __attribute__ ((unused)) = plugin_data->gt;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
                    
                  plugin_data->gt = init_iir_stage(IIR_STAGE_LOWPASS,1,3,2);
                  plugin_data->iirf = init_iirf_t(plugin_data->gt);
                  calc_2polebandpass(iirf, plugin_data->gt, *(plugin_data->center), *(plugin_data->width), sample_rate);
                
}


static void runBandpass_a_iir(LV2_Handle instance, uint32_t sample_count)
{
  Bandpass_a_iir *plugin_data = (Bandpass_a_iir *)instance;

  const float center = *(plugin_data->center);
  const float width = *(plugin_data->width);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  calc_2polebandpass(iirf, gt, center, width, sample_rate);
                  iir_process_buffer_1s_5(iirf, gt, input, output, sample_count);
                
}

static const LV2_Descriptor bandpass_a_iirDescriptor = {
  "http://plugin.org.uk/swh-plugins/bandpass_a_iir",
  instantiateBandpass_a_iir,
  connectPortBandpass_a_iir,
  activateBandpass_a_iir,
  runBandpass_a_iir,
  NULL,
  cleanupBandpass_a_iir,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &bandpass_a_iirDescriptor;
  default:
    return NULL;
  }
}
