
			#include "util/iir.h"
			#include "ladspa-util.h"
                        
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Lowpass_iir {
  float *cutoff;
  float *stages;
  float *input;
  float *output;
iirf_t* iirf;
iir_stage_t* gt;
long sample_rate;
} Lowpass_iir;

static void cleanupLowpass_iir(LV2_Handle instance)
{
Lowpass_iir *plugin_data = (Lowpass_iir *)instance;

                  free_iirf_t(plugin_data->iirf, plugin_data->gt);
                  free_iir_stage(plugin_data->gt);
                
  free(instance);
}

static void connectPortLowpass_iir(LV2_Handle instance, uint32_t port, void *data)
{
  Lowpass_iir *plugin = (Lowpass_iir *)instance;

  switch (port) {
  case 0:
    plugin->cutoff = data;
    break;
  case 1:
    plugin->stages = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateLowpass_iir(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Lowpass_iir *plugin_data = (Lowpass_iir *)malloc(sizeof(Lowpass_iir));
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  sample_rate = s_rate;
                
  plugin_data->iirf = iirf;
  plugin_data->gt = gt;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activateLowpass_iir(LV2_Handle instance)
{
  Lowpass_iir *plugin_data = (Lowpass_iir *)instance;
  iirf_t* iirf __attribute__ ((unused)) = plugin_data->iirf;
  iir_stage_t* gt __attribute__ ((unused)) = plugin_data->gt;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
                    
                  plugin_data->gt = init_iir_stage(IIR_STAGE_LOWPASS,10,3,2);
                  plugin_data->iirf = init_iirf_t(plugin_data->gt);
                  chebyshev(plugin_data->iirf, plugin_data->gt, 2*CLAMP(f_round(*(plugin_data->stages)),1,10), IIR_STAGE_LOWPASS, 
                            *(plugin_data->cutoff)/(float)sample_rate, 0.5f);
                
}


static void runLowpass_iir(LV2_Handle instance, uint32_t sample_count)
{
  Lowpass_iir *plugin_data = (Lowpass_iir *)instance;

  const float cutoff = *(plugin_data->cutoff);
  const float stages = *(plugin_data->stages);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  chebyshev(iirf, gt, 2*CLAMP((int)stages,1,10), IIR_STAGE_LOWPASS, cutoff/(float)sample_rate, 0.5f);
                  iir_process_buffer_ns_5(iirf, gt, input, output, sample_count);
                
}

static const LV2_Descriptor lowpass_iirDescriptor = {
  "http://plugin.org.uk/swh-plugins/lowpass_iir",
  instantiateLowpass_iir,
  connectPortLowpass_iir,
  activateLowpass_iir,
  runLowpass_iir,
  NULL,
  cleanupLowpass_iir,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &lowpass_iirDescriptor;
  default:
    return NULL;
  }
}
