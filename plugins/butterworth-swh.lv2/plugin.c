
			#include "util/iir.h"
                        #include "util/buffer.h"
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Bwxover_iir {
  float *cutoff;
  float *resonance;
  float *input;
  float *lpoutput;
  float *hpoutput;
iirf_t* iirf;
iir_stage_t* gt;
long sample_rate;
} Bwxover_iir;

static void cleanupBwxover_iir(LV2_Handle instance)
{
Bwxover_iir *plugin_data = (Bwxover_iir *)instance;

                  free_iirf_t(plugin_data->iirf, plugin_data->gt);
                  free_iir_stage(plugin_data->gt);
                
  free(instance);
}

static void connectPortBwxover_iir(LV2_Handle instance, uint32_t port, void *data)
{
  Bwxover_iir *plugin = (Bwxover_iir *)instance;

  switch (port) {
  case 0:
    plugin->cutoff = data;
    break;
  case 1:
    plugin->resonance = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->lpoutput = data;
    break;
  case 4:
    plugin->hpoutput = data;
    break;
  }
}

static LV2_Handle instantiateBwxover_iir(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Bwxover_iir *plugin_data = (Bwxover_iir *)malloc(sizeof(Bwxover_iir));
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  sample_rate = s_rate;
                
  plugin_data->iirf = iirf;
  plugin_data->gt = gt;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activateBwxover_iir(LV2_Handle instance)
{
  Bwxover_iir *plugin_data = (Bwxover_iir *)instance;
  iirf_t* iirf __attribute__ ((unused)) = plugin_data->iirf;
  iir_stage_t* gt __attribute__ ((unused)) = plugin_data->gt;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
                    
                  plugin_data->gt = init_iir_stage(IIR_STAGE_LOWPASS,1,3,2);
                  plugin_data->iirf = init_iirf_t(plugin_data->gt);
                  butterworth_stage(plugin_data->gt, 0, *(plugin_data->cutoff), 
		  			   *(plugin_data->resonance), 
					   sample_rate);
                
}


static void runBwxover_iir(LV2_Handle instance, uint32_t sample_count)
{
  Bwxover_iir *plugin_data = (Bwxover_iir *)instance;

  const float cutoff = *(plugin_data->cutoff);
  const float resonance = *(plugin_data->resonance);
  const float * const input = plugin_data->input;
  float * const lpoutput = plugin_data->lpoutput;
  float * const hpoutput = plugin_data->hpoutput;
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  butterworth_stage(gt, 0, cutoff, resonance, sample_rate);
                  iir_process_buffer_1s_5(iirf, gt, input, lpoutput, sample_count);
		  buffer_sub(input, lpoutput, hpoutput, sample_count);
                
}

static const LV2_Descriptor bwxover_iirDescriptor = {
  "http://plugin.org.uk/swh-plugins/bwxover_iir",
  instantiateBwxover_iir,
  connectPortBwxover_iir,
  activateBwxover_iir,
  runBwxover_iir,
  NULL,
  cleanupBwxover_iir,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Buttlow_iir {
  float *cutoff;
  float *resonance;
  float *input;
  float *output;
iirf_t* iirf;
iir_stage_t* gt;
long sample_rate;
} Buttlow_iir;

static void cleanupButtlow_iir(LV2_Handle instance)
{
Buttlow_iir *plugin_data = (Buttlow_iir *)instance;

                  free_iirf_t(plugin_data->iirf, plugin_data->gt);
                  free_iir_stage(plugin_data->gt);
                
  free(instance);
}

static void connectPortButtlow_iir(LV2_Handle instance, uint32_t port, void *data)
{
  Buttlow_iir *plugin = (Buttlow_iir *)instance;

  switch (port) {
  case 0:
    plugin->cutoff = data;
    break;
  case 1:
    plugin->resonance = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateButtlow_iir(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Buttlow_iir *plugin_data = (Buttlow_iir *)malloc(sizeof(Buttlow_iir));
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  sample_rate = s_rate;
                
  plugin_data->iirf = iirf;
  plugin_data->gt = gt;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activateButtlow_iir(LV2_Handle instance)
{
  Buttlow_iir *plugin_data = (Buttlow_iir *)instance;
  iirf_t* iirf __attribute__ ((unused)) = plugin_data->iirf;
  iir_stage_t* gt __attribute__ ((unused)) = plugin_data->gt;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
                    
                  plugin_data->gt = init_iir_stage(IIR_STAGE_LOWPASS,1,3,2);
                  plugin_data->iirf = init_iirf_t(plugin_data->gt);
                  butterworth_stage(plugin_data->gt, 0, *(plugin_data->cutoff), 
		  			   *(plugin_data->resonance), 
					   sample_rate);
                
}


static void runButtlow_iir(LV2_Handle instance, uint32_t sample_count)
{
  Buttlow_iir *plugin_data = (Buttlow_iir *)instance;

  const float cutoff = *(plugin_data->cutoff);
  const float resonance = *(plugin_data->resonance);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  butterworth_stage(gt, 0, cutoff, resonance, sample_rate);
                  iir_process_buffer_1s_5(iirf, gt, input, output, sample_count);
                
}

static const LV2_Descriptor buttlow_iirDescriptor = {
  "http://plugin.org.uk/swh-plugins/buttlow_iir",
  instantiateButtlow_iir,
  connectPortButtlow_iir,
  activateButtlow_iir,
  runButtlow_iir,
  NULL,
  cleanupButtlow_iir,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Butthigh_iir {
  float *cutoff;
  float *resonance;
  float *input;
  float *output;
iirf_t* iirf;
iir_stage_t* gt;
long sample_rate;
} Butthigh_iir;

static void cleanupButthigh_iir(LV2_Handle instance)
{
Butthigh_iir *plugin_data = (Butthigh_iir *)instance;

                  free_iirf_t(plugin_data->iirf, plugin_data->gt);
                  free_iir_stage(plugin_data->gt);
                
  free(instance);
}

static void connectPortButthigh_iir(LV2_Handle instance, uint32_t port, void *data)
{
  Butthigh_iir *plugin = (Butthigh_iir *)instance;

  switch (port) {
  case 0:
    plugin->cutoff = data;
    break;
  case 1:
    plugin->resonance = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateButthigh_iir(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Butthigh_iir *plugin_data = (Butthigh_iir *)malloc(sizeof(Butthigh_iir));
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  sample_rate = s_rate;
                
  plugin_data->iirf = iirf;
  plugin_data->gt = gt;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activateButthigh_iir(LV2_Handle instance)
{
  Butthigh_iir *plugin_data = (Butthigh_iir *)instance;
  iirf_t* iirf __attribute__ ((unused)) = plugin_data->iirf;
  iir_stage_t* gt __attribute__ ((unused)) = plugin_data->gt;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
                    
                  plugin_data->gt = init_iir_stage(IIR_STAGE_LOWPASS,1,3,2);
                  plugin_data->iirf = init_iirf_t(plugin_data->gt);
                  butterworth_stage(plugin_data->gt, 1, *(plugin_data->cutoff), 
		  			   *(plugin_data->resonance), 
					   sample_rate);
                
}


static void runButthigh_iir(LV2_Handle instance, uint32_t sample_count)
{
  Butthigh_iir *plugin_data = (Butthigh_iir *)instance;

  const float cutoff = *(plugin_data->cutoff);
  const float resonance = *(plugin_data->resonance);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  iirf_t* iirf = plugin_data->iirf;
  iir_stage_t* gt = plugin_data->gt;
  long sample_rate = plugin_data->sample_rate;
  
                  butterworth_stage(gt, 1, cutoff, resonance, sample_rate);
                  iir_process_buffer_1s_5(iirf, gt, input, output, sample_count);
                
}

static const LV2_Descriptor butthigh_iirDescriptor = {
  "http://plugin.org.uk/swh-plugins/butthigh_iir",
  instantiateButthigh_iir,
  connectPortButthigh_iir,
  activateButthigh_iir,
  runButthigh_iir,
  NULL,
  cleanupButthigh_iir,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &bwxover_iirDescriptor;
  case 1:
    return &buttlow_iirDescriptor;
  case 2:
    return &butthigh_iirDescriptor;
  default:
    return NULL;
  }
}
