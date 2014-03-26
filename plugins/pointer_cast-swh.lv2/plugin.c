
      #include <limits.h>

      #include "ladspa-util.h"
      #include "util/biquad.h"

      typedef union {
	float fp;
	int         in;
      } pcast;
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _PointerCastDistortion {
  float *cutoff;
  float *wet;
  float *input;
  float *output;
float fs;
biquad * filt;
} PointerCastDistortion;

static void cleanupPointerCastDistortion(LV2_Handle instance)
{
PointerCastDistortion *plugin_data = (PointerCastDistortion *)instance;

      free(plugin_data->filt);
    
  free(instance);
}

static void connectPortPointerCastDistortion(LV2_Handle instance, uint32_t port, void *data)
{
  PointerCastDistortion *plugin = (PointerCastDistortion *)instance;

  switch (port) {
  case 0:
    plugin->cutoff = data;
    break;
  case 1:
    plugin->wet = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiatePointerCastDistortion(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  PointerCastDistortion *plugin_data = (PointerCastDistortion *)malloc(sizeof(PointerCastDistortion));
  float fs = plugin_data->fs;
  biquad * filt = plugin_data->filt;
  
      filt = malloc(sizeof(biquad));
      fs = s_rate;
    
  plugin_data->fs = fs;
  plugin_data->filt = filt;
  
  return (LV2_Handle)plugin_data;
}


static void activatePointerCastDistortion(LV2_Handle instance)
{
  PointerCastDistortion *plugin_data = (PointerCastDistortion *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  biquad * filt __attribute__ ((unused)) = plugin_data->filt;
  
      biquad_init(filt);
    
}


static void runPointerCastDistortion(LV2_Handle instance, uint32_t sample_count)
{
  PointerCastDistortion *plugin_data = (PointerCastDistortion *)instance;

  const float cutoff = *(plugin_data->cutoff);
  const float wet = *(plugin_data->wet);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float fs = plugin_data->fs;
  biquad * filt = plugin_data->filt;
  
      unsigned long pos;
      const float filt_scale = cutoff < 50.0f ? cutoff / 50.0f : 1.0f;

      lp_set_params(filt, cutoff, 1.0f, fs);

      for (pos = 0; pos < sample_count; pos++) {
	pcast val;
	float sign, filt_val, dist_val;

	filt_val = biquad_run(filt, input[pos]) * filt_scale;
	sign = filt_val < 0.0f ? -1.0f : 1.0f;
	val.fp = fabs(filt_val);
	dist_val = sign * (float)val.in / (float)INT_MAX +
		   (input[pos] - filt_val);
        buffer_write(output[pos], LIN_INTERP(wet, input[pos], dist_val));
      }
    
}

static const LV2_Descriptor pointerCastDistortionDescriptor = {
  "http://plugin.org.uk/swh-plugins/pointerCastDistortion",
  instantiatePointerCastDistortion,
  connectPortPointerCastDistortion,
  activatePointerCastDistortion,
  runPointerCastDistortion,
  NULL,
  cleanupPointerCastDistortion,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &pointerCastDistortionDescriptor;
  default:
    return NULL;
  }
}
