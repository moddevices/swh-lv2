
			#include "ladspa-util.h"
			#define COMB_SIZE 0x4000
			#define COMB_MASK 0x3FFF
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _CombSplitter {
  float *freq;
  float *input;
  float *out1;
  float *out2;
float * comb_tbl;
long comb_pos;
long sample_rate;
float last_offset;
} CombSplitter;

static void cleanupCombSplitter(LV2_Handle instance)
{
CombSplitter *plugin_data = (CombSplitter *)instance;

                        free(plugin_data->comb_tbl);
                
  free(instance);
}

static void connectPortCombSplitter(LV2_Handle instance, uint32_t port, void *data)
{
  CombSplitter *plugin = (CombSplitter *)instance;

  switch (port) {
  case 0:
    plugin->freq = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->out1 = data;
    break;
  case 3:
    plugin->out2 = data;
    break;
  }
}

static LV2_Handle instantiateCombSplitter(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  CombSplitter *plugin_data = (CombSplitter *)malloc(sizeof(CombSplitter));
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


static void activateCombSplitter(LV2_Handle instance)
{
  CombSplitter *plugin_data = (CombSplitter *)instance;
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


static void runCombSplitter(LV2_Handle instance, uint32_t sample_count)
{
  CombSplitter *plugin_data = (CombSplitter *)instance;

  const float freq = *(plugin_data->freq);
  const float * const input = plugin_data->input;
  float * const out1 = plugin_data->out1;
  float * const out2 = plugin_data->out2;
  float * comb_tbl = plugin_data->comb_tbl;
  long comb_pos = plugin_data->comb_pos;
  long sample_rate = plugin_data->sample_rate;
  float last_offset = plugin_data->last_offset;
  
			float offset;
			int data_pos;
			unsigned long pos;
			float xf, xf_step, d_pos, fr, interp, in;

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
				in = input[pos];
				comb_tbl[comb_pos] = in;
				out1[pos] = (in + interp) * 0.5f;
				out2[pos] = (in - interp) * 0.5f;
				comb_pos = (comb_pos + 1) & COMB_MASK;
			}

			plugin_data->comb_pos = comb_pos;
			plugin_data->last_offset = offset;
		
}

static const LV2_Descriptor combSplitterDescriptor = {
  "http://plugin.org.uk/swh-plugins/combSplitter",
  instantiateCombSplitter,
  connectPortCombSplitter,
  activateCombSplitter,
  runCombSplitter,
  NULL,
  cleanupCombSplitter,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &combSplitterDescriptor;
  default:
    return NULL;
  }
}
