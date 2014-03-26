
			#include "ladspa-util.h"
			#define COMB_SIZE 0x4000
			#define COMB_MASK 0x3FFF
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Comb {
  float *freq;
  float *fb;
  float *input;
  float *output;
float * comb_tbl;
long comb_pos;
long sample_rate;
float last_offset;
} Comb;

static void cleanupComb(LV2_Handle instance)
{
Comb *plugin_data = (Comb *)instance;

			free(plugin_data->comb_tbl);
		
  free(instance);
}

static void connectPortComb(LV2_Handle instance, uint32_t port, void *data)
{
  Comb *plugin = (Comb *)instance;

  switch (port) {
  case 0:
    plugin->freq = data;
    break;
  case 1:
    plugin->fb = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateComb(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Comb *plugin_data = (Comb *)malloc(sizeof(Comb));
  float * comb_tbl = plugin_data->comb_tbl;
  long comb_pos = plugin_data->comb_pos;
  long sample_rate = plugin_data->sample_rate;
  float last_offset = plugin_data->last_offset;
  
			sample_rate = s_rate;
			comb_tbl = malloc(sizeof(float) * COMB_SIZE);
			comb_pos = 0;
			last_offset = 1000;
		
  plugin_data->comb_tbl = comb_tbl;
  plugin_data->comb_pos = comb_pos;
  plugin_data->sample_rate = sample_rate;
  plugin_data->last_offset = last_offset;
  
  return (LV2_Handle)plugin_data;
}


static void activateComb(LV2_Handle instance)
{
  Comb *plugin_data = (Comb *)instance;
  float * comb_tbl __attribute__ ((unused)) = plugin_data->comb_tbl;
  long comb_pos __attribute__ ((unused)) = plugin_data->comb_pos;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float last_offset __attribute__ ((unused)) = plugin_data->last_offset;
  
			int i;

			for (i = 0; i < COMB_SIZE; i++) {
				comb_tbl[i] = 0;
			}
			comb_pos = 0;
			last_offset = 1000;
		
}


static void runComb(LV2_Handle instance, uint32_t sample_count)
{
  Comb *plugin_data = (Comb *)instance;

  const float freq = *(plugin_data->freq);
  const float fb = *(plugin_data->fb);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float * comb_tbl = plugin_data->comb_tbl;
  long comb_pos = plugin_data->comb_pos;
  long sample_rate = plugin_data->sample_rate;
  float last_offset = plugin_data->last_offset;
  
			float offset;
			int data_pos;
			unsigned long pos;
			float xf, xf_step, d_pos, fr, interp;

			offset = sample_rate / freq;
			offset = f_clamp(offset, 0, COMB_SIZE - 1);
			xf_step = 1.0f / (float)sample_count;
			xf = 0.0f;

			for (pos = 0; pos < sample_count; pos++) {
				xf += xf_step;
				d_pos = comb_pos - LIN_INTERP(xf, last_offset, offset);
				data_pos = f_trunc(d_pos);
				fr = d_pos - data_pos;
				interp =  cube_interp(fr, comb_tbl[(data_pos - 1) & COMB_MASK], comb_tbl[data_pos & COMB_MASK], comb_tbl[(data_pos + 1) & COMB_MASK], comb_tbl[(data_pos + 2) & COMB_MASK]);
				comb_tbl[comb_pos] = input[pos] + fb * interp;
				buffer_write(output[pos], (input[pos] + interp) * 0.5f);
				comb_pos = (comb_pos + 1) & COMB_MASK;
			}

			plugin_data->comb_pos = comb_pos;
			plugin_data->last_offset = offset;
		
}

static const LV2_Descriptor combDescriptor = {
  "http://plugin.org.uk/swh-plugins/comb",
  instantiateComb,
  connectPortComb,
  activateComb,
  runComb,
  NULL,
  cleanupComb,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &combDescriptor;
  default:
    return NULL;
  }
}
