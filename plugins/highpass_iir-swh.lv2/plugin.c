
			#include "util/iir.h"
                        
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Highpass_iir {
  float *cutoff;
  float *stages;
  float *input;
  float *output;
iirf_t* iirf;
iir_stage_t* gt;
long sample_rate;
} Highpass_iir;

static void cleanupHighpass_iir(LV2_Handle instance)
{
Highpass_iir *plugin_data = (Highpass_iir *)instance;

                  free_iirf_t(plugin_data->iirf, plugin_data->gt);
                  free_iir_stage(plugin_data->gt);
                
  free(instance);
}

static void connectPortHighpass_iir(LV2_Handle instance, uint32_t port, void *data)
{
  Highpass_iir *plugin = (Highpass_iir *)instance;

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

static LV2_Handle instantiateHighpass_iir(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Highpass_iir *plugin_data = (Highpass_iir *)malloc(sizeof(Highpass_iir));
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  sample_rate = s_rate;
                
  plugin_data->iirf = iirf;
  plugin_data->gt = gt;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activateHighpass_iir(LV2_Handle instance)
{
  Highpass_iir *plugin_data = (Highpass_iir *)instance;
  iirf_t* iirf __attribute__ ((unused)) = plugin_data->iirf;
  iir_stage_t* gt __attribute__ ((unused)) = plugin_data->gt;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
                    
                  plugin_data->gt = init_iir_stage(IIR_STAGE_HIGHPASS,10,3,2);
                  plugin_data->iirf = init_iirf_t(plugin_data->gt);
                  chebyshev(plugin_data->iirf, plugin_data->gt, 2*CLAMP((int)(*(plugin_data->stages)),1,10), IIR_STAGE_HIGHPASS, *(plugin_data->cutoff)/(float)sample_rate, 0.5f);
                
}


static void runHighpass_iir(LV2_Handle instance, uint32_t sample_count)
{
  Highpass_iir *plugin_data = (Highpass_iir *)instance;

  const float cutoff = *(plugin_data->cutoff);
  const float stages = *(plugin_data->stages);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  chebyshev(iirf, gt, 2*CLAMP((int)stages,1,10), IIR_STAGE_HIGHPASS, cutoff/(float)sample_rate, 0.5f);
                  iir_process_buffer_ns_5(iirf, gt, input, output, sample_count);
                
}

static const LV2_Descriptor highpass_iirDescriptor = {
  "http://plugin.org.uk/swh-plugins/highpass_iir",
  instantiateHighpass_iir,
  connectPortHighpass_iir,
  activateHighpass_iir,
  runHighpass_iir,
  NULL,
  cleanupHighpass_iir,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &highpass_iirDescriptor;
  default:
    return NULL;
  }
}
