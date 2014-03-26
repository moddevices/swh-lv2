
      #include <string.h>
      #include "ladspa-util.h"

      #define N_TAPS 128

      typedef struct {
        unsigned int delay;
        float gain;
      } tap;
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Delayorama {
  float *seed;
  float *gain;
  float *feedback_pc;
  float *tap_count;
  float *first_delay;
  float *delay_range;
  float *delay_scale;
  float *delay_rand_pc;
  float *gain_scale;
  float *gain_rand_pc;
  float *wet;
  float *input;
  float *output;
unsigned long buffer_pos;
unsigned int buffer_size;
unsigned int sample_rate;
float last_start;
float last_range;
float last_delaysc;
float last_ampsc;
unsigned int last_ntaps;
float last_seed;
float last_a_rand;
float last_d_rand;
float last_out;
unsigned int active_set;
unsigned int next_set;
tap ** taps;
float * buffer;
} Delayorama;

static void cleanupDelayorama(LV2_Handle instance)
{
Delayorama *plugin_data = (Delayorama *)instance;

      free(plugin_data->taps[0]);
      free(plugin_data->taps[1]);
      free(plugin_data->taps);
      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortDelayorama(LV2_Handle instance, uint32_t port, void *data)
{
  Delayorama *plugin = (Delayorama *)instance;

  switch (port) {
  case 0:
    plugin->seed = data;
    break;
  case 1:
    plugin->gain = data;
    break;
  case 2:
    plugin->feedback_pc = data;
    break;
  case 3:
    plugin->tap_count = data;
    break;
  case 4:
    plugin->first_delay = data;
    break;
  case 5:
    plugin->delay_range = data;
    break;
  case 6:
    plugin->delay_scale = data;
    break;
  case 7:
    plugin->delay_rand_pc = data;
    break;
  case 8:
    plugin->gain_scale = data;
    break;
  case 9:
    plugin->gain_rand_pc = data;
    break;
  case 10:
    plugin->wet = data;
    break;
  case 11:
    plugin->input = data;
    break;
  case 12:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateDelayorama(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Delayorama *plugin_data = (Delayorama *)malloc(sizeof(Delayorama));
  unsigned long buffer_pos = plugin_data->buffer_pos;
  unsigned int buffer_size = plugin_data->buffer_size;
  unsigned int sample_rate = plugin_data->sample_rate;
  float last_start = plugin_data->last_start;
  float last_range = plugin_data->last_range;
  float last_delaysc = plugin_data->last_delaysc;
  float last_ampsc = plugin_data->last_ampsc;
  unsigned int last_ntaps = plugin_data->last_ntaps;
  float last_seed = plugin_data->last_seed;
  float last_a_rand = plugin_data->last_a_rand;
  float last_d_rand = plugin_data->last_d_rand;
  float last_out = plugin_data->last_out;
  unsigned int active_set = plugin_data->active_set;
  unsigned int next_set = plugin_data->next_set;
  tap ** taps = plugin_data->taps;
  float * buffer = plugin_data->buffer;
  
      sample_rate = s_rate;

      buffer_pos = 0;

      buffer_size = 6.0f * sample_rate;

      taps = malloc(2 * sizeof(tap *));
      taps[0] = calloc(N_TAPS, sizeof(tap));
      taps[1] = calloc(N_TAPS, sizeof(tap));
      active_set = 0;
      next_set = 1;

      buffer = calloc(buffer_size, sizeof(float));

      last_out = 0.0f;

      last_ampsc = 0.0f;
      last_delaysc = 0.0f;
      last_start = 0;
      last_range = 0;
      last_ntaps = 0;
      last_seed = 0;
      last_a_rand = 0;
      last_d_rand = 0;
    
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->buffer_size = buffer_size;
  plugin_data->sample_rate = sample_rate;
  plugin_data->last_start = last_start;
  plugin_data->last_range = last_range;
  plugin_data->last_delaysc = last_delaysc;
  plugin_data->last_ampsc = last_ampsc;
  plugin_data->last_ntaps = last_ntaps;
  plugin_data->last_seed = last_seed;
  plugin_data->last_a_rand = last_a_rand;
  plugin_data->last_d_rand = last_d_rand;
  plugin_data->last_out = last_out;
  plugin_data->active_set = active_set;
  plugin_data->next_set = next_set;
  plugin_data->taps = taps;
  plugin_data->buffer = buffer;
  
  return (LV2_Handle)plugin_data;
}


static void activateDelayorama(LV2_Handle instance)
{
  Delayorama *plugin_data = (Delayorama *)instance;
  unsigned long buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  unsigned int buffer_size __attribute__ ((unused)) = plugin_data->buffer_size;
  unsigned int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float last_start __attribute__ ((unused)) = plugin_data->last_start;
  float last_range __attribute__ ((unused)) = plugin_data->last_range;
  float last_delaysc __attribute__ ((unused)) = plugin_data->last_delaysc;
  float last_ampsc __attribute__ ((unused)) = plugin_data->last_ampsc;
  unsigned int last_ntaps __attribute__ ((unused)) = plugin_data->last_ntaps;
  float last_seed __attribute__ ((unused)) = plugin_data->last_seed;
  float last_a_rand __attribute__ ((unused)) = plugin_data->last_a_rand;
  float last_d_rand __attribute__ ((unused)) = plugin_data->last_d_rand;
  float last_out __attribute__ ((unused)) = plugin_data->last_out;
  unsigned int active_set __attribute__ ((unused)) = plugin_data->active_set;
  unsigned int next_set __attribute__ ((unused)) = plugin_data->next_set;
  tap ** taps __attribute__ ((unused)) = plugin_data->taps;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  
      memset(buffer, 0, buffer_size * sizeof(float));

      last_out = 0.0f;
      last_ampsc = 0.0f;
      last_delaysc = 0.0f;
      last_start = 0;
      last_range = 0;
      last_ntaps = 0;
      last_seed = 0;
      last_a_rand = 0;
      last_d_rand = 0;
    
}


static void runDelayorama(LV2_Handle instance, uint32_t sample_count)
{
  Delayorama *plugin_data = (Delayorama *)instance;

  const float seed = *(plugin_data->seed);
  const float gain = *(plugin_data->gain);
  const float feedback_pc = *(plugin_data->feedback_pc);
  const float tap_count = *(plugin_data->tap_count);
  const float first_delay = *(plugin_data->first_delay);
  const float delay_range = *(plugin_data->delay_range);
  const float delay_scale = *(plugin_data->delay_scale);
  const float delay_rand_pc = *(plugin_data->delay_rand_pc);
  const float gain_scale = *(plugin_data->gain_scale);
  const float gain_rand_pc = *(plugin_data->gain_rand_pc);
  const float wet = *(plugin_data->wet);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  unsigned long buffer_pos = plugin_data->buffer_pos;
  unsigned int buffer_size = plugin_data->buffer_size;
  unsigned int sample_rate = plugin_data->sample_rate;
  float last_start = plugin_data->last_start;
  float last_range = plugin_data->last_range;
  float last_delaysc = plugin_data->last_delaysc;
  float last_ampsc = plugin_data->last_ampsc;
  unsigned int last_ntaps = plugin_data->last_ntaps;
  float last_seed = plugin_data->last_seed;
  float last_a_rand = plugin_data->last_a_rand;
  float last_d_rand = plugin_data->last_d_rand;
  float last_out = plugin_data->last_out;
  unsigned int active_set = plugin_data->active_set;
  unsigned int next_set = plugin_data->next_set;
  tap ** taps = plugin_data->taps;
  float * buffer = plugin_data->buffer;
  
      unsigned long pos;
      float coef = DB_CO(gain);
      unsigned int i;
      unsigned int recalc = 0;
      unsigned int ntaps = LIMIT(f_round(tap_count), 2, N_TAPS);
      float range = f_clamp(delay_range * sample_rate, 0.0f,
				(float)(buffer_size-1));
      float out;
      float xfade = 0.0f;

      const float feedback = feedback_pc * 0.01f;
      const float gain_rand = gain_rand_pc * 0.01f;
      const float delay_rand = delay_rand_pc * 0.01f;


      if (ntaps != last_ntaps) {
        recalc = 1;
        plugin_data->last_ntaps = ntaps;
      }
      if (first_delay != last_start) {
        recalc = 1;
        plugin_data->last_start = first_delay;
      }
      if (range != last_range) {
        recalc = 1;
        plugin_data->last_range = range;
      }
      if (delay_scale != last_delaysc) {
        recalc = 1;
        plugin_data->last_delaysc = delay_scale;
      }
      if (gain_scale != last_ampsc) {
        recalc = 1;
        plugin_data->last_ampsc = gain_scale;
      }
      if (seed != last_seed) {
        recalc = 1;
        plugin_data->last_seed = seed;
      }
      if (gain_rand != last_a_rand) {
        recalc = 1;
        plugin_data->last_a_rand = gain_rand;
      }
      if (delay_rand != last_d_rand) {
        recalc = 1;
        plugin_data->last_d_rand = delay_rand;
      }

      if (recalc) {
        float delay_base = first_delay * sample_rate;
        float delay_fix;
        float gain, delay, delay_sum;
	float d_rand, g_rand;

	srand(f_round(seed));
        if (delay_base + range > buffer_size-1) {
          delay_base = buffer_size - 1 - range;
        }

	if (gain_scale <= 1.0f) {
          gain = 1.0f;
	} else {
          gain = 1.0f / pow(gain_scale, ntaps-1);
        }

	if (delay_scale == 1.0f) {
		delay_fix = range / (ntaps - 1);
	} else {
		delay_fix = range * (delay_scale - 1.0f) / (pow(delay_scale, ntaps - 1) - 1.0f);
	}
        delay = 1.0f;
	delay_sum = 0.0f;

        for (i=0; i<ntaps; i++) {
	  g_rand = (1.0f-gain_rand) + (float)rand() / (float)RAND_MAX * 2.0f * gain_rand;
	  d_rand = (1.0f-delay_rand) + (float)rand() / (float)RAND_MAX * 2.0f * delay_rand;
          taps[next_set][i].delay = LIMIT((unsigned int)(delay_base + delay_sum * delay_fix * d_rand), 0, buffer_size-1);
          taps[next_set][i].gain = gain * g_rand;

          delay_sum += delay;
          delay *= delay_scale;
	  gain *= gain_scale;
        }
        for (; i<N_TAPS; i++) {
	  taps[next_set][i].delay = 0.0f;
	  taps[next_set][i].gain = 0.0f;
        }
      }

      out = last_out;
      for (pos = 0; pos < sample_count; pos++) {
        buffer[buffer_pos] = input[pos] * coef + (out * feedback);

        out = 0.0f;
        for (i=0; i<ntaps; i++) {
          int p = buffer_pos - taps[active_set][i].delay;
          if (p<0) p += buffer_size;
          out += buffer[p] * taps[active_set][i].gain;
        }

        if (recalc) {
	  xfade += 1.0f / (float)sample_count;
          out *= (1-xfade);
          for (i=0; i<ntaps; i++) {
            int p = buffer_pos - taps[next_set][i].delay;
            if (p<0) p += buffer_size;
            out += buffer[p] * taps[next_set][i].gain * xfade;
          }
        }

        buffer_write(output[pos], LIN_INTERP(wet, input[pos], out));

	if (++buffer_pos >= buffer_size) {
          buffer_pos = 0;
        }
      }

      if (recalc) {
	plugin_data->active_set = next_set;
	plugin_data->next_set = active_set;
      }

      plugin_data->buffer_pos = buffer_pos;
      plugin_data->last_out = out;
    
}

static const LV2_Descriptor delayoramaDescriptor = {
  "http://plugin.org.uk/swh-plugins/delayorama",
  instantiateDelayorama,
  connectPortDelayorama,
  activateDelayorama,
  runDelayorama,
  NULL,
  cleanupDelayorama,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &delayoramaDescriptor;
  default:
    return NULL;
  }
}
