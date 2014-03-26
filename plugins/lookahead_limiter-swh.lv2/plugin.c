
      #include "string.h"
      #include "ladspa-util.h"
      #include "util/db.h"

      /* Minimum buffer size in seconds */
      #define BUFFER_TIME 2
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _LookaheadLimiter {
  float *limit;
  float *delay_s;
  float *attenuation;
  float *in_1;
  float *in_2;
  float *out_1;
  float *out_2;
  float *latency;
float * buffer;
unsigned int buffer_len;
unsigned int buffer_pos;
unsigned int fs;
float atten;
float peak;
unsigned int peak_dist;
} LookaheadLimiter;

static void cleanupLookaheadLimiter(LV2_Handle instance)
{
LookaheadLimiter *plugin_data = (LookaheadLimiter *)instance;

       free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortLookaheadLimiter(LV2_Handle instance, uint32_t port, void *data)
{
  LookaheadLimiter *plugin = (LookaheadLimiter *)instance;

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

static LV2_Handle instantiateLookaheadLimiter(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  LookaheadLimiter *plugin_data = (LookaheadLimiter *)malloc(sizeof(LookaheadLimiter));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_len = plugin_data->buffer_len;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int fs = plugin_data->fs;
  float atten = plugin_data->atten;
  float peak = plugin_data->peak;
  unsigned int peak_dist = plugin_data->peak_dist;
  
      buffer_len = 16384;
      buffer_pos = 0;
      fs = s_rate;

      db_init();

      /* Find size for power-of-two interleaved delay buffer */
      while(buffer_len < s_rate * BUFFER_TIME * 2) {
	buffer_len *= 2;
      }
      buffer = calloc(buffer_len, sizeof(float));

      peak = 0.0f;
      peak_dist = 1;
      atten = 0.0f;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_len = buffer_len;
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->fs = fs;
  plugin_data->atten = atten;
  plugin_data->peak = peak;
  plugin_data->peak_dist = peak_dist;
  
  return (LV2_Handle)plugin_data;
}


static void activateLookaheadLimiter(LV2_Handle instance)
{
  LookaheadLimiter *plugin_data = (LookaheadLimiter *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_len __attribute__ ((unused)) = plugin_data->buffer_len;
  unsigned int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  unsigned int fs __attribute__ ((unused)) = plugin_data->fs;
  float atten __attribute__ ((unused)) = plugin_data->atten;
  float peak __attribute__ ((unused)) = plugin_data->peak;
  unsigned int peak_dist __attribute__ ((unused)) = plugin_data->peak_dist;
  
      memset(buffer, 0, buffer_len * sizeof(float));

      peak = 0.0f;
      peak_dist = 1;
      atten = 0.0f;
    
}


static void runLookaheadLimiter(LV2_Handle instance, uint32_t sample_count)
{
  LookaheadLimiter *plugin_data = (LookaheadLimiter *)instance;

  const float limit = *(plugin_data->limit);
  const float delay_s = *(plugin_data->delay_s);
  float attenuation;
  const float * const in_1 = plugin_data->in_1;
  const float * const in_2 = plugin_data->in_2;
  float * const out_1 = plugin_data->out_1;
  float * const out_2 = plugin_data->out_2;
  float latency;
  float * buffer = plugin_data->buffer;
  unsigned int buffer_len = plugin_data->buffer_len;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int fs = plugin_data->fs;
  float atten = plugin_data->atten;
  float peak = plugin_data->peak;
  unsigned int peak_dist = plugin_data->peak_dist;
  
      unsigned long pos;
      const float max = DB_CO(limit);
      float sig, gain;
      const unsigned int delay = delay_s * fs;

      for (pos = 0; pos < sample_count; pos++) {
	buffer[(buffer_pos * 2) & (buffer_len - 1)] = in_1[pos];
	buffer[(buffer_pos * 2 + 1) & (buffer_len - 1)] = in_2[pos];

	/* sig contains the amplitude of the current frame, in dB's realtive
         * to the limit */
	sig = fabs(in_1[pos]) > fabs(in_2[pos]) ? fabs(in_1[pos]) :
		fabs(in_2[pos]);
        //sig = lin2db(sig) - limit;
        sig = CO_DB(sig) - limit;

	if (sig > 0.0f && sig / (float)delay > peak / (float)peak_dist) {
	  peak_dist = delay;
	  peak = sig;
	}

	/* Incremenatlly approach the correct attenuation for the next peak */
	atten -= (atten - peak) / (float)(peak_dist + 1);

	if (peak_dist-- == 0) {
		peak_dist = delay;
		peak = 0.0f;
	}

	gain = 1.0f / db2lin(atten);
	buffer_write(out_1[pos], buffer[(buffer_pos * 2 - delay * 2) &
					(buffer_len - 1)] * gain);
	buffer_write(out_2[pos], buffer[(buffer_pos * 2 - delay * 2 + 1) &
					(buffer_len - 1)] * gain);

	/* Ensure that the signal really can't be over the limit, potentially
         * changes in the lookahead time could cause us to miss peaks */

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

      *(plugin_data->attenuation) = atten;
      *(plugin_data->latency) = delay;
    
}

static const LV2_Descriptor lookaheadLimiterDescriptor = {
  "http://plugin.org.uk/swh-plugins/lookaheadLimiter",
  instantiateLookaheadLimiter,
  connectPortLookaheadLimiter,
  activateLookaheadLimiter,
  runLookaheadLimiter,
  NULL,
  cleanupLookaheadLimiter,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &lookaheadLimiterDescriptor;
  default:
    return NULL;
  }
}
