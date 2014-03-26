
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _SmoothDecimate {
  float *rate;
  float *smooth;
  float *input;
  float *output;
float fs;
float accum;
float * buffer;
int buffer_pos;
} SmoothDecimate;

static void cleanupSmoothDecimate(LV2_Handle instance)
{
SmoothDecimate *plugin_data = (SmoothDecimate *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortSmoothDecimate(LV2_Handle instance, uint32_t port, void *data)
{
  SmoothDecimate *plugin = (SmoothDecimate *)instance;

  switch (port) {
  case 0:
    plugin->rate = data;
    break;
  case 1:
    plugin->smooth = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateSmoothDecimate(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  SmoothDecimate *plugin_data = (SmoothDecimate *)malloc(sizeof(SmoothDecimate));
  float fs = plugin_data->fs;
  float accum = plugin_data->accum;
  float * buffer = plugin_data->buffer;
  int buffer_pos = plugin_data->buffer_pos;
  
      buffer = calloc(8, sizeof(float));
      buffer_pos = 0;
      accum = 0.0f;
      fs = (float)s_rate;
    
  plugin_data->fs = fs;
  plugin_data->accum = accum;
  plugin_data->buffer = buffer;
  plugin_data->buffer_pos = buffer_pos;
  
  return (LV2_Handle)plugin_data;
}


static void activateSmoothDecimate(LV2_Handle instance)
{
  SmoothDecimate *plugin_data = (SmoothDecimate *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  float accum __attribute__ ((unused)) = plugin_data->accum;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  
      buffer_pos = 0;
      accum = 0.0f;
    
}


static void runSmoothDecimate(LV2_Handle instance, uint32_t sample_count)
{
  SmoothDecimate *plugin_data = (SmoothDecimate *)instance;

  const float rate = *(plugin_data->rate);
  const float smooth = *(plugin_data->smooth);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float fs = plugin_data->fs;
  float accum = plugin_data->accum;
  float * buffer = plugin_data->buffer;
  int buffer_pos = plugin_data->buffer_pos;
  
      unsigned long pos;
      float smoothed;
      float inc = (rate / fs);
      inc = f_clamp(inc, 0.0f, 1.0f);

      for (pos = 0; pos < sample_count; pos++) {
	accum += inc;
	if (accum >= 1.0f) {
	  accum -= 1.0f;
	  buffer_pos = (buffer_pos + 1) & 7;
	  buffer[buffer_pos] = input[pos];
	}
	smoothed = cube_interp(accum, buffer[(buffer_pos - 3) & 7],
                                      buffer[(buffer_pos - 2) & 7],
                                      buffer[(buffer_pos - 1) & 7],
                                      buffer[buffer_pos]);
	buffer_write(output[pos], LIN_INTERP(smooth, buffer[(buffer_pos - 3) & 7], smoothed));
      }

      plugin_data->accum = accum;
      plugin_data->buffer_pos = buffer_pos;
    
}

static const LV2_Descriptor smoothDecimateDescriptor = {
  "http://plugin.org.uk/swh-plugins/smoothDecimate",
  instantiateSmoothDecimate,
  connectPortSmoothDecimate,
  activateSmoothDecimate,
  runSmoothDecimate,
  NULL,
  cleanupSmoothDecimate,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &smoothDecimateDescriptor;
  default:
    return NULL;
  }
}
