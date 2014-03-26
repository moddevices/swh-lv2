
#include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Flanger {
  float *delay_base;
  float *detune;
  float *law_freq;
  float *feedback;
  float *input;
  float *output;
long sample_rate;
long count;
float prev_law_peak;
float next_law_peak;
int prev_law_pos;
int next_law_pos;
float * delay_tbl;
long delay_pos;
long delay_size;
long old_d_base;
} Flanger;

static void cleanupFlanger(LV2_Handle instance)
{
Flanger *plugin_data = (Flanger *)instance;

free(plugin_data->delay_tbl);
    
  free(instance);
}

static void connectPortFlanger(LV2_Handle instance, uint32_t port, void *data)
{
  Flanger *plugin = (Flanger *)instance;

  switch (port) {
  case 0:
    plugin->delay_base = data;
    break;
  case 1:
    plugin->detune = data;
    break;
  case 2:
    plugin->law_freq = data;
    break;
  case 3:
    plugin->feedback = data;
    break;
  case 4:
    plugin->input = data;
    break;
  case 5:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateFlanger(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Flanger *plugin_data = (Flanger *)malloc(sizeof(Flanger));
  long sample_rate = plugin_data->sample_rate;
  long count = plugin_data->count;
  float prev_law_peak = plugin_data->prev_law_peak;
  float next_law_peak = plugin_data->next_law_peak;
  int prev_law_pos = plugin_data->prev_law_pos;
  int next_law_pos = plugin_data->next_law_pos;
  float * delay_tbl = plugin_data->delay_tbl;
  long delay_pos = plugin_data->delay_pos;
  long delay_size = plugin_data->delay_size;
  long old_d_base = plugin_data->old_d_base;
  
int min_size;

sample_rate = s_rate;

prev_law_peak = 0.0f;
next_law_peak = 1.0f;
prev_law_pos = 0;
next_law_pos = 10;

min_size = sample_rate * 0.04f;
for (delay_size = 1024; delay_size < min_size; delay_size *= 2);
delay_tbl = malloc(sizeof(float) * delay_size);
delay_pos = 0;
count = 0;
old_d_base = 0;
    
  plugin_data->sample_rate = sample_rate;
  plugin_data->count = count;
  plugin_data->prev_law_peak = prev_law_peak;
  plugin_data->next_law_peak = next_law_peak;
  plugin_data->prev_law_pos = prev_law_pos;
  plugin_data->next_law_pos = next_law_pos;
  plugin_data->delay_tbl = delay_tbl;
  plugin_data->delay_pos = delay_pos;
  plugin_data->delay_size = delay_size;
  plugin_data->old_d_base = old_d_base;
  
  return (LV2_Handle)plugin_data;
}


static void activateFlanger(LV2_Handle instance)
{
  Flanger *plugin_data = (Flanger *)instance;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  long count __attribute__ ((unused)) = plugin_data->count;
  float prev_law_peak __attribute__ ((unused)) = plugin_data->prev_law_peak;
  float next_law_peak __attribute__ ((unused)) = plugin_data->next_law_peak;
  int prev_law_pos __attribute__ ((unused)) = plugin_data->prev_law_pos;
  int next_law_pos __attribute__ ((unused)) = plugin_data->next_law_pos;
  float * delay_tbl __attribute__ ((unused)) = plugin_data->delay_tbl;
  long delay_pos __attribute__ ((unused)) = plugin_data->delay_pos;
  long delay_size __attribute__ ((unused)) = plugin_data->delay_size;
  long old_d_base __attribute__ ((unused)) = plugin_data->old_d_base;
  
memset(delay_tbl, 0, sizeof(float) * delay_size);
delay_pos = 0;
count = 0;
old_d_base = 0;
    
}


static void runFlanger(LV2_Handle instance, uint32_t sample_count)
{
  Flanger *plugin_data = (Flanger *)instance;

  const float delay_base = *(plugin_data->delay_base);
  const float detune = *(plugin_data->detune);
  const float law_freq = *(plugin_data->law_freq);
  const float feedback = *(plugin_data->feedback);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  long sample_rate = plugin_data->sample_rate;
  long count = plugin_data->count;
  float prev_law_peak = plugin_data->prev_law_peak;
  float next_law_peak = plugin_data->next_law_peak;
  int prev_law_pos = plugin_data->prev_law_pos;
  int next_law_pos = plugin_data->next_law_pos;
  float * delay_tbl = plugin_data->delay_tbl;
  long delay_pos = plugin_data->delay_pos;
  long delay_size = plugin_data->delay_size;
  long old_d_base = plugin_data->old_d_base;
  
unsigned long pos;
long d_base, new_d_base;
float out;
float delay_depth;
float dp; // float delay position
float dp_frac; // fractional part
long dp_idx; // integer delay index
long law_p; // period of law
float frac = 0.0f, step; // Portion the way through the block
float law; /* law amplitude */
float n_ph, p_ph;
const float fb = f_clamp(feedback, -0.999f, 0.999f);

// Set law params
law_p = (float)sample_rate / law_freq;
if (law_p < 1) {
	law_p = 1;
}

// Calculate base delay size in samples
new_d_base = (LIMIT(f_round(delay_base), 0, 25) * sample_rate) / 1000;

// Calculate delay depth in samples
delay_depth = f_clamp(detune * (float)sample_rate * 0.001f, 0.0f, delay_size - new_d_base - 1.0f);

step = 1.0f/sample_count;
for (pos = 0; pos < sample_count; pos++) {
	if (count % law_p == 0) {
		// Value for amplitude of law peak
		next_law_peak = (float)rand() / (float)RAND_MAX;
		next_law_pos = count + law_p;
	} else if (count % law_p == law_p / 2) {
		// Value for amplitude of law peak
		prev_law_peak = (float)rand() / (float)RAND_MAX;
		prev_law_pos = count + law_p;
	}

	// Calculate position in delay table
	d_base = LIN_INTERP(frac, old_d_base, new_d_base);
	n_ph = (float)(law_p - abs(next_law_pos - count))/(float)law_p;
	p_ph = n_ph + 0.5f;
	while (p_ph > 1.0f) {
		p_ph -= 1.0f;
	}
	law = f_sin_sq(3.1415926f*p_ph)*prev_law_peak +
		f_sin_sq(3.1415926f*n_ph)*next_law_peak;

	dp = (float)(delay_pos - d_base) - (delay_depth * law);
	// Get the integer part
	dp_idx = f_round(dp - 0.5f);
	// Get the fractional part
	dp_frac = dp - dp_idx;

	// Accumulate into output buffer
	out = cube_interp(dp_frac, delay_tbl[(dp_idx-1) & (delay_size-1)], delay_tbl[dp_idx & (delay_size-1)], delay_tbl[(dp_idx+1) & (delay_size-1)], delay_tbl[(dp_idx+2) & (delay_size-1)]);

	// Store new delayed value
	delay_tbl[delay_pos] = flush_to_zero(input[pos] + (fb * out));
	// Sometimes the delay can pick up NaN values, I'm not sure why
	// and this is easier than fixing it
	if (isnan(delay_tbl[delay_pos])) {
		delay_tbl[delay_pos] = 0.0f;
	}

	out = f_clamp(delay_tbl[delay_pos] * 0.707f, -1.0, 1.0);
	buffer_write(output[pos], out);

	frac += step;
	delay_pos = (delay_pos + 1) & (delay_size-1);

	count++;
}

plugin_data->count = count;
plugin_data->prev_law_peak = prev_law_peak;
plugin_data->next_law_peak = next_law_peak;
plugin_data->prev_law_pos = prev_law_pos;
plugin_data->next_law_pos = next_law_pos;
plugin_data->delay_pos = delay_pos;
plugin_data->old_d_base = new_d_base;
    
}

static const LV2_Descriptor flangerDescriptor = {
  "http://plugin.org.uk/swh-plugins/flanger",
  instantiateFlanger,
  connectPortFlanger,
  activateFlanger,
  runFlanger,
  NULL,
  cleanupFlanger,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &flangerDescriptor;
  default:
    return NULL;
  }
}
