
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _FreqTracker {
  float *speed;
  float *input;
  float *freq;
float fs;
int cross_time;
float last_amp;
float f;
float fo;
} FreqTracker;

static void cleanupFreqTracker(LV2_Handle instance)
{

  free(instance);
}

static void connectPortFreqTracker(LV2_Handle instance, uint32_t port, void *data)
{
  FreqTracker *plugin = (FreqTracker *)instance;

  switch (port) {
  case 0:
    plugin->speed = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->freq = data;
    break;
  }
}

static LV2_Handle instantiateFreqTracker(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  FreqTracker *plugin_data = (FreqTracker *)malloc(sizeof(FreqTracker));
  float fs = plugin_data->fs;
  int cross_time = plugin_data->cross_time;
  float last_amp = plugin_data->last_amp;
  float f = plugin_data->f;
  float fo = plugin_data->fo;
  
      fs = s_rate;
      f = 0.0f;
      fo = 0.0f;
      cross_time = 0;
      last_amp = 0.0f;
    
  plugin_data->fs = fs;
  plugin_data->cross_time = cross_time;
  plugin_data->last_amp = last_amp;
  plugin_data->f = f;
  plugin_data->fo = fo;
  
  return (LV2_Handle)plugin_data;
}


static void activateFreqTracker(LV2_Handle instance)
{
  FreqTracker *plugin_data = (FreqTracker *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  int cross_time __attribute__ ((unused)) = plugin_data->cross_time;
  float last_amp __attribute__ ((unused)) = plugin_data->last_amp;
  float f __attribute__ ((unused)) = plugin_data->f;
  float fo __attribute__ ((unused)) = plugin_data->fo;
  
      cross_time = 0;
      f = 0.0f;
      fo = 0.0f;
      last_amp = 0.0f;
    
}


static void runFreqTracker(LV2_Handle instance, uint32_t sample_count)
{
  FreqTracker *plugin_data = (FreqTracker *)instance;

  const float speed = *(plugin_data->speed);
  const float * const input = plugin_data->input;
  float * const freq = plugin_data->freq;
  float fs = plugin_data->fs;
  int cross_time = plugin_data->cross_time;
  float last_amp = plugin_data->last_amp;
  float f = plugin_data->f;
  float fo = plugin_data->fo;
  
      unsigned long pos;
      float xm1 = last_amp;
      const float damp_lp = (1.0f - speed) * 0.9f;
      const float damp_lpi = 1.0f - damp_lp;

      for (pos = 0; pos < sample_count; pos++) {
	if (input[pos] < 0.0f && xm1 > 0.0f) {
	  if (cross_time > 3.0f) {
	    f = fs / ((float)cross_time * 2.0f);
	  }
	  cross_time = 0;
	}
	xm1 = input[pos];
	cross_time++;
	fo = fo * damp_lp + f * damp_lpi;
	fo = flush_to_zero(fo);
	buffer_write(freq[pos], fo);
      }

      plugin_data->last_amp = xm1;
      plugin_data->fo = fo;
      plugin_data->f = f;
      plugin_data->cross_time = cross_time;
    
}

static const LV2_Descriptor freqTrackerDescriptor = {
  "http://plugin.org.uk/swh-plugins/freqTracker",
  instantiateFreqTracker,
  connectPortFreqTracker,
  activateFreqTracker,
  runFreqTracker,
  NULL,
  cleanupFreqTracker,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &freqTrackerDescriptor;
  default:
    return NULL;
  }
}
