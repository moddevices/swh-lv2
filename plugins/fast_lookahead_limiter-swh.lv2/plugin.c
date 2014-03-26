
      #include "ladspa-util.h"

      //#define DEBUG

      #define NUM_CHUNKS 16
      #define BUFFER_TIME 0.0053

      #ifdef DEBUG
	#include "stdio.h"
      #endif
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _FastLookaheadLimiter {
  float *ingain;
  float *limit;
  float *release;
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
float atten_lp;
float peak;
float delta;
unsigned int delay;
unsigned int chunk_num;
unsigned int chunk_pos;
unsigned int chunk_size;
float * chunks;
} FastLookaheadLimiter;

static void cleanupFastLookaheadLimiter(LV2_Handle instance)
{
FastLookaheadLimiter *plugin_data = (FastLookaheadLimiter *)instance;

       free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortFastLookaheadLimiter(LV2_Handle instance, uint32_t port, void *data)
{
  FastLookaheadLimiter *plugin = (FastLookaheadLimiter *)instance;

  switch (port) {
  case 0:
    plugin->ingain = data;
    break;
  case 1:
    plugin->limit = data;
    break;
  case 2:
    plugin->release = data;
    break;
  case 3:
    plugin->attenuation = data;
    break;
  case 4:
    plugin->in_1 = data;
    break;
  case 5:
    plugin->in_2 = data;
    break;
  case 6:
    plugin->out_1 = data;
    break;
  case 7:
    plugin->out_2 = data;
    break;
  case 8:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiateFastLookaheadLimiter(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  FastLookaheadLimiter *plugin_data = (FastLookaheadLimiter *)malloc(sizeof(FastLookaheadLimiter));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_len = plugin_data->buffer_len;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int fs = plugin_data->fs;
  float atten = plugin_data->atten;
  float atten_lp = plugin_data->atten_lp;
  float peak = plugin_data->peak;
  float delta = plugin_data->delta;
  unsigned int delay = plugin_data->delay;
  unsigned int chunk_num = plugin_data->chunk_num;
  unsigned int chunk_pos = plugin_data->chunk_pos;
  unsigned int chunk_size = plugin_data->chunk_size;
  float * chunks = plugin_data->chunks;
  
      fs = s_rate;
      buffer_len = 128;
      buffer_pos = 0;

      /* Find size for power-of-two interleaved delay buffer */
      while(buffer_len < fs * BUFFER_TIME * 2) {
        buffer_len *= 2;
      }
      buffer = calloc(buffer_len, sizeof(float));
      delay = (int)(0.005 * fs);

      chunk_pos = 0;
      chunk_num = 0;

      /* find a chunk size (in smaples) thats roughly 0.5ms */
      chunk_size = s_rate / 2000; 
      chunks = calloc(NUM_CHUNKS, sizeof(float));

      peak = 0.0f;
      atten = 1.0f;
      atten_lp = 1.0f;
      delta = 0.0f;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_len = buffer_len;
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->fs = fs;
  plugin_data->atten = atten;
  plugin_data->atten_lp = atten_lp;
  plugin_data->peak = peak;
  plugin_data->delta = delta;
  plugin_data->delay = delay;
  plugin_data->chunk_num = chunk_num;
  plugin_data->chunk_pos = chunk_pos;
  plugin_data->chunk_size = chunk_size;
  plugin_data->chunks = chunks;
  
  return (LV2_Handle)plugin_data;
}


static void activateFastLookaheadLimiter(LV2_Handle instance)
{
  FastLookaheadLimiter *plugin_data = (FastLookaheadLimiter *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_len __attribute__ ((unused)) = plugin_data->buffer_len;
  unsigned int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  unsigned int fs __attribute__ ((unused)) = plugin_data->fs;
  float atten __attribute__ ((unused)) = plugin_data->atten;
  float atten_lp __attribute__ ((unused)) = plugin_data->atten_lp;
  float peak __attribute__ ((unused)) = plugin_data->peak;
  float delta __attribute__ ((unused)) = plugin_data->delta;
  unsigned int delay __attribute__ ((unused)) = plugin_data->delay;
  unsigned int chunk_num __attribute__ ((unused)) = plugin_data->chunk_num;
  unsigned int chunk_pos __attribute__ ((unused)) = plugin_data->chunk_pos;
  unsigned int chunk_size __attribute__ ((unused)) = plugin_data->chunk_size;
  float * chunks __attribute__ ((unused)) = plugin_data->chunks;
  
      memset(buffer, 0, NUM_CHUNKS * sizeof(float));

      chunk_pos = 0;
      chunk_num = 0;
      peak = 0.0f;
      atten = 1.0f;
      atten_lp = 1.0f;
      delta = 0.0f;
    
}


static void runFastLookaheadLimiter(LV2_Handle instance, uint32_t sample_count)
{
  FastLookaheadLimiter *plugin_data = (FastLookaheadLimiter *)instance;

  const float ingain = *(plugin_data->ingain);
  const float limit = *(plugin_data->limit);
  const float release = *(plugin_data->release);
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
  float atten_lp = plugin_data->atten_lp;
  float peak = plugin_data->peak;
  float delta = plugin_data->delta;
  unsigned int delay = plugin_data->delay;
  unsigned int chunk_num = plugin_data->chunk_num;
  unsigned int chunk_pos = plugin_data->chunk_pos;
  unsigned int chunk_size = plugin_data->chunk_size;
  float * chunks = plugin_data->chunks;
  
      unsigned long pos;
      const float max = DB_CO(limit);
      const float trim = DB_CO(ingain);
      float sig;
      unsigned int i;

      #ifdef DEBUG
      float clip = 0.0, clipp = 0.0;
      int clipc = 0;
      #endif

      for (pos = 0; pos < sample_count; pos++) {
	if (chunk_pos++ == chunk_size) {
	  /* we've got a full chunk */
         
	  delta = (1.0f - atten) / (fs * release);
	  round_to_zero(&delta);
	  for (i=0; i<10; i++) {
	    const int p = (chunk_num - 9 + i) & (NUM_CHUNKS - 1);
            const float this_delta = (max / chunks[p] - atten) /
				      ((float)(i) * fs * 0.0005f + 1.0f);

	    if (this_delta < delta) {
	      delta = this_delta;
	    }
          }

          chunks[chunk_num++ & (NUM_CHUNKS - 1)] = peak;
	  peak = 0.0f;
	  chunk_pos = 0;
        }

	buffer[(buffer_pos * 2) & (buffer_len - 1)] =     in_1[pos] * trim
							+ 1.0e-30;
	buffer[(buffer_pos * 2 + 1) & (buffer_len - 1)] = in_2[pos] * trim
							+ 1.0e-30;

	sig = fabs(in_1[pos]) > fabs(in_2[pos]) ? fabs(in_1[pos]) :
		fabs(in_2[pos]);
	sig += 1.0e-30;
	if (sig * trim > peak) {
	  peak = sig * trim;
	}
	//round_to_zero(&peak);
	//round_to_zero(&sig);

	atten += delta;
	atten_lp = atten * 0.1f + atten_lp * 0.9f;
	//round_to_zero(&atten_lp);
	if (delta > 0.0f && atten > 1.0f) {
	  atten = 1.0f;
	  delta = 0.0f;
	}

	buffer_write(out_1[pos], buffer[(buffer_pos * 2 - delay * 2) &
					(buffer_len - 1)] * atten_lp);
	buffer_write(out_2[pos], buffer[(buffer_pos * 2 - delay * 2 + 1) &
					(buffer_len - 1)] * atten_lp);
	round_to_zero(&out_1[pos]);
	round_to_zero(&out_2[pos]);

	if (out_1[pos] < -max) {
          #ifdef DEBUG
	  clip += 20.0*log10(out_1[pos] / -max);
	  clipc++;
          if (fabs(out_1[pos] - max) > clipp) {
            clipp = fabs(out_1[pos] / -max);
          }
          #endif
	  buffer_write(out_1[pos], -max);
	} else if (out_1[pos] > max) {
          #ifdef DEBUG
	  clip += 20.0*log10(out_1[pos] / max);
	  clipc++;
          if (fabs(out_1[pos] - max) > clipp) {
            clipp = fabs(out_1[pos] / max);
          }
          #endif
	  buffer_write(out_1[pos], max);
	}
	if (out_2[pos] < -max) {
          #ifdef DEBUG
	  clip += 20.0*log10(out_2[pos] / -max);
	  clipc++;
          if (fabs(out_2[pos] - max) > clipp) {
            clipp = fabs(out_2[pos] / -max);
          }
          #endif
	  buffer_write(out_2[pos], -max);
	} else if (out_2[pos] > max) {
          #ifdef DEBUG
	  clip += 20.0*log10(out_2[pos] / max);
	  clipc++;
          if (fabs(out_2[pos] - max) > clipp) {
            clipp = fabs(out_2[pos] / max);
          }
          #endif
	  buffer_write(out_2[pos], max);
	}

	buffer_pos++;
      }

      #ifdef DEBUG
      if (clipc > 0) {
        printf("%d overs: %fdB avg, %fdB peak\n", clipc, clip/(float)clipc, 20.0*log10(clipp));
      }
      #endif

      plugin_data->buffer_pos = buffer_pos;
      plugin_data->peak = peak;
      plugin_data->atten = atten;
      plugin_data->atten_lp = atten_lp;
      plugin_data->chunk_pos = chunk_pos;
      plugin_data->chunk_num = chunk_num;

      *(plugin_data->attenuation) = -CO_DB(atten);
      *(plugin_data->latency) = delay;
    
}

static const LV2_Descriptor fastLookaheadLimiterDescriptor = {
  "http://plugin.org.uk/swh-plugins/fastLookaheadLimiter",
  instantiateFastLookaheadLimiter,
  connectPortFastLookaheadLimiter,
  activateFastLookaheadLimiter,
  runFastLookaheadLimiter,
  NULL,
  cleanupFastLookaheadLimiter,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &fastLookaheadLimiterDescriptor;
  default:
    return NULL;
  }
}
