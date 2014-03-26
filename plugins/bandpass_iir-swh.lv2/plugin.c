
			#include "util/iir.h"
                        
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Bandpass_iir {
  float *center;
  float *width;
  float *stages;
  float *input;
  float *output;
iirf_t* iirf;
iir_stage_t* gt;
iir_stage_t* first;
iir_stage_t* second;
long sample_rate;
float ufc;
float lfc;
} Bandpass_iir;

static void cleanupBandpass_iir(LV2_Handle instance)
{
Bandpass_iir *plugin_data = (Bandpass_iir *)instance;

                  free_iirf_t(plugin_data->iirf, plugin_data->gt);                  
                  free_iir_stage(plugin_data->first);
                  free_iir_stage(plugin_data->second);
                  free_iir_stage(plugin_data->gt);
                
  free(instance);
}

static void connectPortBandpass_iir(LV2_Handle instance, uint32_t port, void *data)
{
  Bandpass_iir *plugin = (Bandpass_iir *)instance;

  switch (port) {
  case 0:
    plugin->center = data;
    break;
  case 1:
    plugin->width = data;
    break;
  case 2:
    plugin->stages = data;
    break;
  case 3:
    plugin->input = data;
    break;
  case 4:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateBandpass_iir(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Bandpass_iir *plugin_data = (Bandpass_iir *)malloc(sizeof(Bandpass_iir));
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  iir_stage_t* first = plugin_data->first;
  iir_stage_t* second = plugin_data->second;
  long sample_rate = plugin_data->sample_rate;
  float ufc = plugin_data->ufc;
  float lfc = plugin_data->lfc;
  
                  sample_rate = s_rate;
                
  plugin_data->iirf = iirf;
  plugin_data->gt = gt;
  plugin_data->first = first;
  plugin_data->second = second;
  plugin_data->sample_rate = sample_rate;
  plugin_data->ufc = ufc;
  plugin_data->lfc = lfc;
  
  return (LV2_Handle)plugin_data;
}


static void activateBandpass_iir(LV2_Handle instance)
{
  Bandpass_iir *plugin_data = (Bandpass_iir *)instance;
  iirf_t* iirf __attribute__ ((unused)) = plugin_data->iirf;
  iir_stage_t* gt __attribute__ ((unused)) = plugin_data->gt;
  iir_stage_t* first __attribute__ ((unused)) = plugin_data->first;
  iir_stage_t* second __attribute__ ((unused)) = plugin_data->second;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float ufc __attribute__ ((unused)) = plugin_data->ufc;
  float lfc __attribute__ ((unused)) = plugin_data->lfc;
                    
                  plugin_data->ufc = (*(plugin_data->center) + *(plugin_data->width)*0.5f)/(float)sample_rate;
                  plugin_data->lfc = (*(plugin_data->center) - *(plugin_data->width)*0.5f)/(float)sample_rate;
                  plugin_data->first = init_iir_stage(IIR_STAGE_LOWPASS,10,3,2);
                  plugin_data->second = init_iir_stage(IIR_STAGE_HIGHPASS,10,3,2);                  
                  plugin_data->gt = init_iir_stage(IIR_STAGE_BANDPASS,20,3,2); 
                  plugin_data->iirf = init_iirf_t(plugin_data->gt);
                  chebyshev(plugin_data->iirf, plugin_data->first, 2*CLAMP((int)(*(plugin_data->stages)),1,10), IIR_STAGE_LOWPASS, plugin_data->ufc, 0.5f);
                  chebyshev(plugin_data->iirf, plugin_data->second, 2*CLAMP((int)(*(plugin_data->stages)),1,10), IIR_STAGE_HIGHPASS, plugin_data->lfc, 0.5f);
                  combine_iir_stages(IIR_STAGE_BANDPASS, plugin_data->gt, plugin_data->first, plugin_data->second,0,0);
                
}


static void runBandpass_iir(LV2_Handle instance, uint32_t sample_count)
{
  Bandpass_iir *plugin_data = (Bandpass_iir *)instance;

  const float center = *(plugin_data->center);
  const float width = *(plugin_data->width);
  const float stages = *(plugin_data->stages);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  iir_stage_t* first = plugin_data->first;
  iir_stage_t* second = plugin_data->second;
  long sample_rate = plugin_data->sample_rate;
  float ufc = plugin_data->ufc;
  float lfc = plugin_data->lfc;
  
                  ufc = (center + width*0.5f)/(float)sample_rate;
                  lfc = (center - width*0.5f)/(float)sample_rate;
                  combine_iir_stages(IIR_STAGE_BANDPASS, gt, first, second,
                                     chebyshev(iirf, first,  2*CLAMP((int)stages,1,10), IIR_STAGE_LOWPASS,  ufc, 0.5f),
                                     chebyshev(iirf, second, 2*CLAMP((int)stages,1,10), IIR_STAGE_HIGHPASS, lfc, 0.5f));
                  iir_process_buffer_ns_5(iirf, gt, input, output, sample_count);
                
}

static const LV2_Descriptor bandpass_iirDescriptor = {
  "http://plugin.org.uk/swh-plugins/bandpass_iir",
  instantiateBandpass_iir,
  connectPortBandpass_iir,
  activateBandpass_iir,
  runBandpass_iir,
  NULL,
  cleanupBandpass_iir,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &bandpass_iirDescriptor;
  default:
    return NULL;
  }
}
