
      #include "ladspa-util.h"
      #include "util/db.h"

      /* Minimum buffer size in seconds */
      #define BUFFER_TIME 0.15f
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _LookaheadLimiterConst {
  float *limit;
  float *delay_s;
  float *attenuation;
  float *in_1;
  float *in_2;
  float *out_1;
  float *out_2;
  float *latency;
float * buffer;
float * amp_buffer;
unsigned int buffer_len;
unsigned int buffer_mask;
unsigned int buffer_pos;
unsigned int fs;
float atten;
float peak;
unsigned int peak_dist;
float last_delay;
} LookaheadLimiterConst;

static void cleanupLookaheadLimiterConst(LV2_Handle instance)
{
LookaheadLimiterConst *plugin_data = (LookaheadLimiterConst *)instance;

       free(plugin_data->buffer);
       free(plugin_data->amp_buffer);
    
  free(instance);
}

static void connectPortLookaheadLimiterConst(LV2_Handle instance, uint32_t port, void *data)
{
  LookaheadLimiterConst *plugin = (LookaheadLimiterConst *)instance;

  switch (port) {
  case 0:
    plugin->limit = data;
    break;
  case 1:
    plugin->delay_s = data;
    break;
  case 2:
    plugin->attenuation = data;
    break;
  case 3:
    plugin->in_1 = data;
    break;
  case 4:
    plugin->in_2 = data;
    break;
  case 5:
    plugin->out_1 = data;
    break;
  case 6:
    plugin->out_2 = data;
    break;
  case 7:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiateLookaheadLimiterConst(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  LookaheadLimiterConst *plugin_data = (LookaheadLimiterConst *)malloc(sizeof(LookaheadLimiterConst));
  float * buffer = plugin_data->buffer;
  float * amp_buffer = plugin_data->amp_buffer;
  unsigned int buffer_len = plugin_data->buffer_len;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int fs = plugin_data->fs;
  float atten = plugin_data->atten;
  float peak = plugin_data->peak;
  unsigned int peak_dist = plugin_data->peak_dist;
  float last_delay = plugin_data->last_delay;
  
      buffer_len = 4096;
      buffer_pos = 0;
      fs = s_rate;

      db_init();

      /* Find size for power-of-two interleaved delay buffer */
      while(buffer_len < s_rate * BUFFER_TIME) {
	buffer_len *= 2;
      }
      buffer_mask = buffer_len * 2 - 1;
      buffer = calloc(buffer_len * 2, sizeof(float));
      amp_buffer = calloc(buffer_len, sizeof(float));

      peak = 0.0f;
      peak_dist = 1;
      atten = 0.0f;
      last_delay = -1.0f;
    
  plugin_data->buffer = buffer;
  plugin_data->amp_buffer = amp_buffer;
  plugin_data->buffer_len = buffer_len;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->fs = fs;
  plugin_data->atten = atten;
  plugin_data->peak = peak;
  plugin_data->peak_dist = peak_dist;
  plugin_data->last_delay = last_delay;
  
  return (LV2_Handle)plugin_data;
}


static void activateLookaheadLimiterConst(LV2_Handle instance)
{
  LookaheadLimiterConst *plugin_data = (LookaheadLimiterConst *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  float * amp_buffer __attribute__ ((unused)) = plugin_data->amp_buffer;
  unsigned int buffer_len __attribute__ ((unused)) = plugin_data->buffer_len;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  unsigned int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  unsigned int fs __attribute__ ((unused)) = plugin_data->fs;
  float atten __attribute__ ((unused)) = plugin_data->atten;
  float peak __attribute__ ((unused)) = plugin_data->peak;
  unsigned int peak_dist __attribute__ ((unused)) = plugin_data->peak_dist;
  float last_delay __attribute__ ((unused)) = plugin_data->last_delay;
  
      int i;

      memset(buffer, 0, buffer_len * 2 * sizeof(float));
      for (i=0; i<buffer_len; i++) amp_buffer[i] = 1.0f;

      buffer_pos = 0;
      peak = 0.0f;
      peak_dist = 1;
      atten = 0.0f;
      last_delay = -1.0f;
    
}


static void runLookaheadLimiterConst(LV2_Handle instance, uint32_t sample_count)
{
  LookaheadLimiterConst *plugin_data = (LookaheadLimiterConst *)instance;

  const float limit = *(plugin_data->limit);
  const float delay_s = *(plugin_data->delay_s);
  float attenuation;
  const float * const in_1 = plugin_data->in_1;
  const float * const in_2 = plugin_data->in_2;
  float * const out_1 = plugin_data->out_1;
  float * const out_2 = plugin_data->out_2;
  float latency;
  float * buffer = plugin_data->buffer;
  float * amp_buffer = plugin_data->amp_buffer;
  unsigned int buffer_len = plugin_data->buffer_len;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int fs = plugin_data->fs;
  float atten = plugin_data->atten;
  float peak = plugin_data->peak;
  unsigned int peak_dist = plugin_data->peak_dist;
  float last_delay = plugin_data->last_delay;
  
      unsigned long pos;
      const float max = DB_CO(limit);
      float sig, gain;
      float delay = last_delay;
      float delay_delta;
      float a, b;

      if (delay < 0.0f) {
	delay = delay_s * fs;
	delay_delta = 0.0f;
      } else {
        delay_delta = (delay_s * fs - last_delay) / (sample_count - 1);
      }

      for (pos = 0; pos < sample_count; pos++) {
	delay += delay_delta;
	buffer[(buffer_pos * 2) & buffer_mask] = in_1[pos];
	buffer[(buffer_pos * 2 + 1) & buffer_mask] = in_2[pos];

        a = fabs(buffer[((buffer_pos + f_round(delay)) * 2) & buffer_mask]);
        b = fabs(buffer[((buffer_pos + f_round(delay)) * 2 + 1) & buffer_mask]);
        sig = a > b ? a : b;
/* XXX
        sig = fabs(in_1[pos]) > fabs(in_2[pos]) ? fabs(in_1[pos]) :
                fabs(in_2[pos]);
*/

        if (sig > max) {
          const float rel = lin2db(sig) - limit;

          if (rel / delay > peak / (float)peak_dist) {
            peak_dist = delay;
            peak = rel;
          }
        }

	/* Incremenatlly approach the correct attenuation for the next peak */
	atten -= (atten - peak) / (float)(peak_dist + 1);

	if (peak_dist-- == 0) {
		peak_dist = f_round(delay);
		peak = 0.0f;
	}

	/* Cacluate the apropriate gain reduction and write it back into the
	 * buffer */
	gain = amp_buffer[(buffer_pos - f_round(delay)) & (buffer_len - 1)];
	amp_buffer[(buffer_pos - f_round(delay)) & (buffer_len - 1)] =
					 1.0f / db2lin(atten);

gain=1.0f / db2lin(atten);

        buffer_write(out_1[pos], buffer[(2 * (buffer_pos + 1)) &
                                        buffer_mask] * gain);
        buffer_write(out_2[pos], buffer[(2 * (buffer_pos + 1)+1) &
                                        buffer_mask] * gain);

	/* Ensure that the signal really can't be over the limit */

#if 0
XXX FIXME XXX
	if (out_1[pos] < -max) {
	  buffer_write(out_1[pos], -max);
	} else if (out_1[pos] > max) {
	  buffer_write(out_1[pos], max);
	}
	if (out_2[pos] < -max) {
	  buffer_write(out_2[pos], -max);
	} else if (out_2[pos] > max) {
	  buffer_write(out_2[pos], max);
	}
#endif

	buffer_pos++;
      }

      plugin_data->buffer_pos = buffer_pos;
      plugin_data->peak = peak;
      plugin_data->peak_dist = peak_dist;
      plugin_data->atten = atten;
      plugin_data->last_delay = delay;

      *(plugin_data->attenuation) = atten;
      *(plugin_data->latency) = buffer_len - 1;
    
}

static const LV2_Descriptor lookaheadLimiterConstDescriptor = {
  "http://plugin.org.uk/swh-plugins/lookaheadLimiterConst",
  instantiateLookaheadLimiterConst,
  connectPortLookaheadLimiterConst,
  activateLookaheadLimiterConst,
  runLookaheadLimiterConst,
  NULL,
  cleanupLookaheadLimiterConst,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &lookaheadLimiterConstDescriptor;
  default:
    return NULL;
  }
}
