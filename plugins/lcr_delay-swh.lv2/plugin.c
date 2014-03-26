
      #include "ladspa-util.h"
      #include "util/biquad.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _LcrDelay {
  float *ldel;
  float *llev;
  float *cdel;
  float *clev;
  float *rdel;
  float *rlev;
  float *feedback;
  float *high_d;
  float *low_d;
  float *spread;
  float *wet;
  float *in_l;
  float *in_r;
  float *out_l;
  float *out_r;
float * buffer;
unsigned int buffer_pos;
unsigned int buffer_mask;
float fs;
float last_ll;
float last_cl;
float last_rl;
float last_ld;
float last_cd;
float last_rd;
biquad * filters;
} LcrDelay;

static void cleanupLcrDelay(LV2_Handle instance)
{
LcrDelay *plugin_data = (LcrDelay *)instance;

      free(plugin_data->filters);
      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortLcrDelay(LV2_Handle instance, uint32_t port, void *data)
{
  LcrDelay *plugin = (LcrDelay *)instance;

  switch (port) {
  case 0:
    plugin->ldel = data;
    break;
  case 1:
    plugin->llev = data;
    break;
  case 2:
    plugin->cdel = data;
    break;
  case 3:
    plugin->clev = data;
    break;
  case 4:
    plugin->rdel = data;
    break;
  case 5:
    plugin->rlev = data;
    break;
  case 6:
    plugin->feedback = data;
    break;
  case 7:
    plugin->high_d = data;
    break;
  case 8:
    plugin->low_d = data;
    break;
  case 9:
    plugin->spread = data;
    break;
  case 10:
    plugin->wet = data;
    break;
  case 11:
    plugin->in_l = data;
    break;
  case 12:
    plugin->in_r = data;
    break;
  case 13:
    plugin->out_l = data;
    break;
  case 14:
    plugin->out_r = data;
    break;
  }
}

static LV2_Handle instantiateLcrDelay(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  LcrDelay *plugin_data = (LcrDelay *)malloc(sizeof(LcrDelay));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  float fs = plugin_data->fs;
  float last_ll = plugin_data->last_ll;
  float last_cl = plugin_data->last_cl;
  float last_rl = plugin_data->last_rl;
  float last_ld = plugin_data->last_ld;
  float last_cd = plugin_data->last_cd;
  float last_rd = plugin_data->last_rd;
  biquad * filters = plugin_data->filters;
  
      int buffer_size = 32768;

      fs = s_rate;
      while (buffer_size < fs * 2.7f) {
	buffer_size *= 2;
      }
      buffer = calloc(buffer_size, sizeof(float));
      buffer_mask = buffer_size - 1;
      buffer_pos = 0;
      last_ll = 0.0f;
      last_cl = 0.0f;
      last_rl = 0.0f;
      last_ld = 0.0f;
      last_cd = 0.0f;
      last_rd = 0.0f;

      filters = malloc(2 * sizeof(biquad));
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->fs = fs;
  plugin_data->last_ll = last_ll;
  plugin_data->last_cl = last_cl;
  plugin_data->last_rl = last_rl;
  plugin_data->last_ld = last_ld;
  plugin_data->last_cd = last_cd;
  plugin_data->last_rd = last_rd;
  plugin_data->filters = filters;
  
  return (LV2_Handle)plugin_data;
}


static void activateLcrDelay(LV2_Handle instance)
{
  LcrDelay *plugin_data = (LcrDelay *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  float last_ll __attribute__ ((unused)) = plugin_data->last_ll;
  float last_cl __attribute__ ((unused)) = plugin_data->last_cl;
  float last_rl __attribute__ ((unused)) = plugin_data->last_rl;
  float last_ld __attribute__ ((unused)) = plugin_data->last_ld;
  float last_cd __attribute__ ((unused)) = plugin_data->last_cd;
  float last_rd __attribute__ ((unused)) = plugin_data->last_rd;
  biquad * filters __attribute__ ((unused)) = plugin_data->filters;
  
      memset(buffer, 0, (buffer_mask + 1) * sizeof(float));
      last_ll = 0.0f;
      last_cl = 0.0f;
      last_rl = 0.0f;
      last_ld = 0.0f;
      last_cd = 0.0f;
      last_rd = 0.0f;
      biquad_init(filters);
      biquad_init(filters + 1);
    
}


static void runLcrDelay(LV2_Handle instance, uint32_t sample_count)
{
  LcrDelay *plugin_data = (LcrDelay *)instance;

  const float ldel = *(plugin_data->ldel);
  const float llev = *(plugin_data->llev);
  const float cdel = *(plugin_data->cdel);
  const float clev = *(plugin_data->clev);
  const float rdel = *(plugin_data->rdel);
  const float rlev = *(plugin_data->rlev);
  const float feedback = *(plugin_data->feedback);
  const float high_d = *(plugin_data->high_d);
  const float low_d = *(plugin_data->low_d);
  const float spread = *(plugin_data->spread);
  const float wet = *(plugin_data->wet);
  const float * const in_l = plugin_data->in_l;
  const float * const in_r = plugin_data->in_r;
  float * const out_l = plugin_data->out_l;
  float * const out_r = plugin_data->out_r;
  float * buffer = plugin_data->buffer;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  float fs = plugin_data->fs;
  float last_ll = plugin_data->last_ll;
  float last_cl = plugin_data->last_cl;
  float last_rl = plugin_data->last_rl;
  float last_ld = plugin_data->last_ld;
  float last_cd = plugin_data->last_cd;
  float last_rd = plugin_data->last_rd;
  biquad * filters = plugin_data->filters;
  
      unsigned long pos;
      const float sc_r = 1.0f / (float)sample_count;
      const float spr_t = 0.5f + spread * 0.01f;
      const float spr_o = 0.5f - spread * 0.01f;
      float fb = feedback * 0.01f;
      float ll, cl, rl, ld, cd, rd;
      float ll_d, cl_d, rl_d, ld_d, cd_d, rd_d;
      float left, right;
      float fbs; /* Feedback signal */

      if (fb < -0.99f) {
	fb = -0.99f;
      } else if (fb > 0.99f) {
	fb = 0.99f;
      }

      ls_set_params(filters, fs * 0.0001f * powf(2.0f, low_d * 0.12f),
		    -0.5f * low_d, 0.5f, fs);
      hs_set_params(filters + 1, fs * (0.41f - 0.0001f *
		    powf(2.0f, high_d * 0.12f)), -70.0f, 0.9f, fs);

      ll = last_ll;				/* Start value of Left Level */
      ll_d = (llev * 0.01f - last_ll) * sc_r; 	/* Delta for Left Level */
      cl = last_cl;
      cl_d = (clev * 0.01f - last_cl) * sc_r;
      rl = last_rl;
      rl_d = (rlev * 0.01f - last_rl) * sc_r;

      ld = last_ld;
      ld_d = (ldel * fs * 0.001f - last_ld) * sc_r;
      cd = last_cd;
      cd_d = (cdel * fs * 0.001f - last_cd) * sc_r;
      rd = last_rd;
      rd_d = (rdel * fs * 0.001f - last_rd) * sc_r;

      for (pos = 0; pos < sample_count; pos++) {
        /* Increment linear interpolators */
	ll += ll_d;
	rl += rl_d;
	cl += cl_d;
	ld += ld_d;
	rd += rd_d;
	cd += cd_d;

	/* Write input into delay line */
	buffer[buffer_pos] = in_l[pos] + in_r[pos];
	/* Add feedback, must be done afterwards for case where C delay = 0 */
	fbs = buffer[(buffer_pos - f_round(cd)) & buffer_mask] * fb;
	fbs = flush_to_zero(fbs);
	fbs = biquad_run(filters, fbs);
	fbs = biquad_run(filters + 1, fbs);
	buffer[buffer_pos] += fbs;

	/* Outputs from left and right delay beffers + centre mix */
        left  = buffer[(buffer_pos - f_round(ld)) & buffer_mask] * ll +
                buffer[(buffer_pos - f_round(cd)) & buffer_mask] * cl;
        right = buffer[(buffer_pos - f_round(rd)) & buffer_mask] * rl +
                buffer[(buffer_pos - f_round(cd)) & buffer_mask] * cl;

	/* Left and right channel outs */
	buffer_write(out_l[pos], in_l[pos] * (1.0f - wet) +
			(left * spr_t + right * spr_o) * wet);
        buffer_write(out_r[pos], in_r[pos] * (1.0f - wet) +
			(left * spr_o + right * spr_t) * wet);

	buffer_pos = (buffer_pos + 1) & buffer_mask;
      }

      plugin_data->last_ll = ll;
      plugin_data->last_cl = cl;
      plugin_data->last_rl = rl;
      plugin_data->last_ld = ld;
      plugin_data->last_cd = cd;
      plugin_data->last_rd = rd;
      plugin_data->buffer_pos = buffer_pos;
    
}

static const LV2_Descriptor lcrDelayDescriptor = {
  "http://plugin.org.uk/swh-plugins/lcrDelay",
  instantiateLcrDelay,
  connectPortLcrDelay,
  activateLcrDelay,
  runLcrDelay,
  NULL,
  cleanupLcrDelay,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &lcrDelayDescriptor;
  default:
    return NULL;
  }
}
