
			#include "ladspa-util.h"

			int refcount;
			float *sin_tbl, *tri_tbl, *saw_tbl, *squ_tbl;
			long sample_rate;
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Ringmod_2i1o {
  float *depth;
  float *input;
  float *modulator;
  float *output;
} Ringmod_2i1o;

static void cleanupRingmod_2i1o(LV2_Handle instance)
{

  free(instance);
}

static void connectPortRingmod_2i1o(LV2_Handle instance, uint32_t port, void *data)
{
  Ringmod_2i1o *plugin = (Ringmod_2i1o *)instance;

  switch (port) {
  case 0:
    plugin->depth = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->modulator = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateRingmod_2i1o(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Ringmod_2i1o *plugin_data = (Ringmod_2i1o *)malloc(sizeof(Ringmod_2i1o));
  
  
  return (LV2_Handle)plugin_data;
}



static void runRingmod_2i1o(LV2_Handle instance, uint32_t sample_count)
{
  Ringmod_2i1o *plugin_data = (Ringmod_2i1o *)instance;

  const float depth = *(plugin_data->depth);
  const float * const input = plugin_data->input;
  const float * const modulator = plugin_data->modulator;
  float * const output = plugin_data->output;
  
			unsigned long pos;
			float tmpa = depth * 0.5f;
			float tmpb = 2.0f - depth;

			for (pos = 0; pos < sample_count; pos++) {
				buffer_write(output[pos], input[pos] * (tmpa * modulator[pos] + tmpb));
			}
		
}

static const LV2_Descriptor ringmod_2i1oDescriptor = {
  "http://plugin.org.uk/swh-plugins/ringmod_2i1o",
  instantiateRingmod_2i1o,
  connectPortRingmod_2i1o,
  NULL,
  runRingmod_2i1o,
  NULL,
  cleanupRingmod_2i1o,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Ringmod_1i1o1l {
  float *depthp;
  float *freq;
  float *sin;
  float *tri;
  float *saw;
  float *squ;
  float *input;
  float *output;
float offset;
} Ringmod_1i1o1l;

static void cleanupRingmod_1i1o1l(LV2_Handle instance)
{
Ringmod_1i1o1l *plugin_data = (Ringmod_1i1o1l *)instance;

			plugin_data = plugin_data;
			if (--refcount == 0) {
				free(sin_tbl);
				free(tri_tbl);
				free(squ_tbl);
				free(saw_tbl);
			}
		
  free(instance);
}

static void connectPortRingmod_1i1o1l(LV2_Handle instance, uint32_t port, void *data)
{
  Ringmod_1i1o1l *plugin = (Ringmod_1i1o1l *)instance;

  switch (port) {
  case 0:
    plugin->depthp = data;
    break;
  case 1:
    plugin->freq = data;
    break;
  case 2:
    plugin->sin = data;
    break;
  case 3:
    plugin->tri = data;
    break;
  case 4:
    plugin->saw = data;
    break;
  case 5:
    plugin->squ = data;
    break;
  case 6:
    plugin->input = data;
    break;
  case 7:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateRingmod_1i1o1l(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Ringmod_1i1o1l *plugin_data = (Ringmod_1i1o1l *)malloc(sizeof(Ringmod_1i1o1l));
  float offset = plugin_data->offset;
  
			long i;

			sample_rate = s_rate;

			if (refcount++ == 0) {
				sin_tbl = malloc(sizeof(float) * sample_rate);
				for (i = 0; i < sample_rate; i++) {
					sin_tbl[i] = sin(i * 2 * M_PI / sample_rate);
				}
				
				tri_tbl = malloc(sizeof(float) * sample_rate);
				for (i = 0; i < sample_rate; i++) {
					tri_tbl[i] = acos(cos(i * 2 * M_PI / sample_rate)) / M_PI * 2 - 1;
				}

				squ_tbl = malloc(sizeof(float) * sample_rate);
				for (i = 0; i < sample_rate; i++) {
					squ_tbl[i] = (i < sample_rate/2) ? 1 : -1;
				}

				saw_tbl = malloc(sizeof(float) * sample_rate);
				for (i = 0; i < sample_rate; i++) {
					saw_tbl[i] = ((2.0 * i) - (float)sample_rate) / (float)sample_rate;
				}
			}

			offset = 0;
		
  plugin_data->offset = offset;
  
  return (LV2_Handle)plugin_data;
}


static void activateRingmod_1i1o1l(LV2_Handle instance)
{
  Ringmod_1i1o1l *plugin_data = (Ringmod_1i1o1l *)instance;
  float offset __attribute__ ((unused)) = plugin_data->offset;
  
			offset = 0;
		
}


static void runRingmod_1i1o1l(LV2_Handle instance, uint32_t sample_count)
{
  Ringmod_1i1o1l *plugin_data = (Ringmod_1i1o1l *)instance;

  const float depthp = *(plugin_data->depthp);
  const float freq = *(plugin_data->freq);
  const float sin = *(plugin_data->sin);
  const float tri = *(plugin_data->tri);
  const float saw = *(plugin_data->saw);
  const float squ = *(plugin_data->squ);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float offset = plugin_data->offset;
  
			float scale = fabs(sin) + fabs(tri) +
			 fabs(saw) + fabs(squ);
			int o;
			unsigned long pos;

			// Rescale to more useful value
			const float depth = depthp * 0.5f;

			if (scale == 0.0) {
				scale = 1.0;
			}

			for (pos = 0; pos < sample_count; pos++) {
				o = f_round(offset);
				buffer_write(output[pos], input[pos] *
				 (depth * (((sin / scale) * sin_tbl[o]) +
				   ((tri / scale) * tri_tbl[o]) +
				   ((saw / scale) * saw_tbl[o]) +
				   ((squ / scale) * squ_tbl[o])) +
				   (1.0f - depth)));
				offset += freq;
				if (offset > sample_rate) {
					offset -= sample_rate;
				}
			}

			plugin_data->offset = offset;
		
}

static const LV2_Descriptor ringmod_1i1o1lDescriptor = {
  "http://plugin.org.uk/swh-plugins/ringmod_1i1o1l",
  instantiateRingmod_1i1o1l,
  connectPortRingmod_1i1o1l,
  activateRingmod_1i1o1l,
  runRingmod_1i1o1l,
  NULL,
  cleanupRingmod_1i1o1l,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &ringmod_2i1oDescriptor;
  case 1:
    return &ringmod_1i1o1lDescriptor;
  default:
    return NULL;
  }
}
