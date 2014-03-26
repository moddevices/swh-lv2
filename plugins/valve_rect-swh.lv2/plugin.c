
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _ValveRect {
  float *sag;
  float *dist_p;
  float *input;
  float *output;
float lp1tm1;
float lp2tm1;
float * avg;
int avg_size;
float avg_sizer;
float avgs;
unsigned int apos;
} ValveRect;

static void cleanupValveRect(LV2_Handle instance)
{
ValveRect *plugin_data = (ValveRect *)instance;

      free(plugin_data->avg);
    
  free(instance);
}

static void connectPortValveRect(LV2_Handle instance, uint32_t port, void *data)
{
  ValveRect *plugin = (ValveRect *)instance;

  switch (port) {
  case 0:
    plugin->sag = data;
    break;
  case 1:
    plugin->dist_p = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateValveRect(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  ValveRect *plugin_data = (ValveRect *)malloc(sizeof(ValveRect));
  float lp1tm1 = plugin_data->lp1tm1;
  float lp2tm1 = plugin_data->lp2tm1;
  float * avg = plugin_data->avg;
  int avg_size = plugin_data->avg_size;
  float avg_sizer = plugin_data->avg_sizer;
  float avgs = plugin_data->avgs;
  unsigned int apos = plugin_data->apos;
  
      // Number of samples in averaging buffer
      avg_size = s_rate / 9;
      // Reciprocal of obove
      avg_sizer = 9.0f / (float)s_rate;
      // Averaging buffer
      avg = calloc(avg_size, sizeof(float));
      // Sum of samples in averaging buffer
      avgs = 0.0f;
      // Position in averaging buffer
      apos = 0;
      // Last value in lowpass 1
      lp1tm1 = 0.0f;
      // Last value in lowpass 2
      lp2tm1 = 0.0f;
    
  plugin_data->lp1tm1 = lp1tm1;
  plugin_data->lp2tm1 = lp2tm1;
  plugin_data->avg = avg;
  plugin_data->avg_size = avg_size;
  plugin_data->avg_sizer = avg_sizer;
  plugin_data->avgs = avgs;
  plugin_data->apos = apos;
  
  return (LV2_Handle)plugin_data;
}


static void activateValveRect(LV2_Handle instance)
{
  ValveRect *plugin_data = (ValveRect *)instance;
  float lp1tm1 __attribute__ ((unused)) = plugin_data->lp1tm1;
  float lp2tm1 __attribute__ ((unused)) = plugin_data->lp2tm1;
  float * avg __attribute__ ((unused)) = plugin_data->avg;
  int avg_size __attribute__ ((unused)) = plugin_data->avg_size;
  float avg_sizer __attribute__ ((unused)) = plugin_data->avg_sizer;
  float avgs __attribute__ ((unused)) = plugin_data->avgs;
  unsigned int apos __attribute__ ((unused)) = plugin_data->apos;
  
      memset(avg, 0, avg_size * sizeof(float));
      avgs = 0.0f;
      apos = 0;
      lp1tm1 = 0.0f;
      lp2tm1 = 0.0f;
    
}


static void runValveRect(LV2_Handle instance, uint32_t sample_count)
{
  ValveRect *plugin_data = (ValveRect *)instance;

  const float sag = *(plugin_data->sag);
  const float dist_p = *(plugin_data->dist_p);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float lp1tm1 = plugin_data->lp1tm1;
  float lp2tm1 = plugin_data->lp2tm1;
  float * avg = plugin_data->avg;
  int avg_size = plugin_data->avg_size;
  float avg_sizer = plugin_data->avg_sizer;
  float avgs = plugin_data->avgs;
  unsigned int apos = plugin_data->apos;
  
      unsigned long pos;
      float q, x, fx;
      const float dist = dist_p * 40.0f + 0.1f;

      for (pos = 0; pos < sample_count; pos++) {
        x = fabs(input[pos]);
        if (x > lp1tm1) {
          lp1tm1 = x;
        } else {
          lp1tm1 = 0.9999f * lp1tm1 + 0.0001f * x;
        }

        avgs -= avg[apos];
        avgs += lp1tm1;
        avg[apos++] = lp1tm1;
        apos %= avg_size;

        lp2tm1 = 0.999f * lp2tm1 + avgs*avg_sizer * 0.001f;
	q = lp1tm1 * sag - lp2tm1 * 1.02f - 1.0f;
	if (q > -0.01f) {
          q = -0.01f;
        } else if (q < -1.0f) {
          q = -1.0f;
        }

        if (input[pos] == q) {
          fx = 1.0f / dist + q / (1.0f - f_exp(dist * q));
        } else {
          fx = (input[pos] - q) /
           (1.0f - f_exp(-dist * (input[pos] - q))) +
           q / (1.0f - f_exp(dist * q));
        }

        buffer_write(output[pos], fx);
      }

      plugin_data->lp1tm1 = lp1tm1;
      plugin_data->lp2tm1 = lp2tm1;
      plugin_data->avgs = avgs;
      plugin_data->apos = apos;
    
}

static const LV2_Descriptor valveRectDescriptor = {
  "http://plugin.org.uk/swh-plugins/valveRect",
  instantiateValveRect,
  connectPortValveRect,
  activateValveRect,
  runValveRect,
  NULL,
  cleanupValveRect,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &valveRectDescriptor;
  default:
    return NULL;
  }
}
