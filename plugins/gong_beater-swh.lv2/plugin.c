
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _GongBeater {
  float *imp_gain;
  float *strike_gain;
  float *strike_duration;
  float *input;
  float *output;
float x;
float y;
float xm;
float ym;
unsigned int running;
float fs;
float imp_level;
} GongBeater;

static void cleanupGongBeater(LV2_Handle instance)
{

  free(instance);
}

static void connectPortGongBeater(LV2_Handle instance, uint32_t port, void *data)
{
  GongBeater *plugin = (GongBeater *)instance;

  switch (port) {
  case 0:
    plugin->imp_gain = data;
    break;
  case 1:
    plugin->strike_gain = data;
    break;
  case 2:
    plugin->strike_duration = data;
    break;
  case 3:
    plugin->input = data;
    break;
  case 4:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateGongBeater(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  GongBeater *plugin_data = (GongBeater *)malloc(sizeof(GongBeater));
  float x = plugin_data->x;
  float y = plugin_data->y;
  float xm = plugin_data->xm;
  float ym = plugin_data->ym;
  unsigned int running = plugin_data->running;
  float fs = plugin_data->fs;
  float imp_level = plugin_data->imp_level;
  
      running = 0;
      x = 0.5f;
      y = 0.0f;
      xm = 0.5f;
      ym = 0.0f;
      fs = (float)s_rate;
      imp_level = 0.0f;
    
  plugin_data->x = x;
  plugin_data->y = y;
  plugin_data->xm = xm;
  plugin_data->ym = ym;
  plugin_data->running = running;
  plugin_data->fs = fs;
  plugin_data->imp_level = imp_level;
  
  return (LV2_Handle)plugin_data;
}


static void activateGongBeater(LV2_Handle instance)
{
  GongBeater *plugin_data = (GongBeater *)instance;
  float x __attribute__ ((unused)) = plugin_data->x;
  float y __attribute__ ((unused)) = plugin_data->y;
  float xm __attribute__ ((unused)) = plugin_data->xm;
  float ym __attribute__ ((unused)) = plugin_data->ym;
  unsigned int running __attribute__ ((unused)) = plugin_data->running;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  float imp_level __attribute__ ((unused)) = plugin_data->imp_level;
  
      running = 0;
      x = 0.5f;
      y = 0.0f;
      xm = 0.5f;
      ym = 0.0f;
    
}


static void runGongBeater(LV2_Handle instance, uint32_t sample_count)
{
  GongBeater *plugin_data = (GongBeater *)instance;

  const float imp_gain = *(plugin_data->imp_gain);
  const float strike_gain = *(plugin_data->strike_gain);
  const float strike_duration = *(plugin_data->strike_duration);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float x = plugin_data->x;
  float y = plugin_data->y;
  float xm = plugin_data->xm;
  float ym = plugin_data->ym;
  unsigned int running = plugin_data->running;
  float fs = plugin_data->fs;
  float imp_level = plugin_data->imp_level;
  
      unsigned long pos;
      const float imp_amp = DB_CO(imp_gain);
      const float strike_amp = DB_CO(strike_gain);
      const float omega = 6.2831852f / (strike_duration * fs);

      pos = 0;
      while (pos < sample_count) {
        for (; !running && pos < sample_count; pos++) {
	  if (fabs(input[pos]) > 0.05f) {
	    running = strike_duration * fs;
	    imp_level = fabs(input[pos]);
	  }
          buffer_write(output[pos], input[pos] * imp_amp);
        }
        for (; running && pos < sample_count; pos++, running--) {
	  if (fabs(input[pos]) > imp_level) {
	    imp_level = fabs(input[pos]);
	  }
	  x -= omega * y;
	  y += omega * x;
	  xm -= omega * 0.5f * ym;
	  ym += omega * 0.5f * xm;

	  buffer_write(output[pos], input[pos] * imp_amp + y * strike_amp *
			    imp_level * 4.0f * ym);
	}
      }

      plugin_data->x = x;
      plugin_data->y = y;
      plugin_data->xm = xm;
      plugin_data->ym = ym;
      plugin_data->running = running;
      plugin_data->imp_level = imp_level;
    
}

static const LV2_Descriptor gongBeaterDescriptor = {
  "http://plugin.org.uk/swh-plugins/gongBeater",
  instantiateGongBeater,
  connectPortGongBeater,
  activateGongBeater,
  runGongBeater,
  NULL,
  cleanupGongBeater,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &gongBeaterDescriptor;
  default:
    return NULL;
  }
}
