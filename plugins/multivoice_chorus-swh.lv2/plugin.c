
#include "ladspa-util.h"
#define MAX_LAWS 7
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _MultivoiceChorus {
  float *voices;
  float *delay_base;
  float *voice_spread;
  float *detune;
  float *law_freq;
  float *attendb;
  float *input;
  float *output;
long sample_rate;
long count;
int law_pos;
int law_roll;
int max_law_p;
int last_law_p;
float * delay_tbl;
unsigned int delay_pos;
unsigned int delay_size;
unsigned int delay_mask;
unsigned int * prev_peak_pos;
unsigned int * next_peak_pos;
float * prev_peak_amp;
float * next_peak_amp;
float * dp_targ;
float * dp_curr;
} MultivoiceChorus;

static void cleanupMultivoiceChorus(LV2_Handle instance)
{
MultivoiceChorus *plugin_data = (MultivoiceChorus *)instance;

free(plugin_data->delay_tbl);
free(plugin_data->prev_peak_pos);
free(plugin_data->next_peak_pos);
free(plugin_data->prev_peak_amp);
free(plugin_data->next_peak_amp);
free(plugin_data->dp_targ);
free(plugin_data->dp_curr);
    
  free(instance);
}

static void connectPortMultivoiceChorus(LV2_Handle instance, uint32_t port, void *data)
{
  MultivoiceChorus *plugin = (MultivoiceChorus *)instance;

  switch (port) {
  case 0:
    plugin->voices = data;
    break;
  case 1:
    plugin->delay_base = data;
    break;
  case 2:
    plugin->voice_spread = data;
    break;
  case 3:
    plugin->detune = data;
    break;
  case 4:
    plugin->law_freq = data;
    break;
  case 5:
    plugin->attendb = data;
    break;
  case 6:
    plugin->input = data;
    break;
  case 7:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateMultivoiceChorus(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  MultivoiceChorus *plugin_data = (MultivoiceChorus *)malloc(sizeof(MultivoiceChorus));
  long sample_rate = plugin_data->sample_rate;
  long count = plugin_data->count;
  int law_pos = plugin_data->law_pos;
  int law_roll = plugin_data->law_roll;
  int max_law_p = plugin_data->max_law_p;
  int last_law_p = plugin_data->last_law_p;
  float * delay_tbl = plugin_data->delay_tbl;
  unsigned int delay_pos = plugin_data->delay_pos;
  unsigned int delay_size = plugin_data->delay_size;
  unsigned int delay_mask = plugin_data->delay_mask;
  unsigned int * prev_peak_pos = plugin_data->prev_peak_pos;
  unsigned int * next_peak_pos = plugin_data->next_peak_pos;
  float * prev_peak_amp = plugin_data->prev_peak_amp;
  float * next_peak_amp = plugin_data->next_peak_amp;
  float * dp_targ = plugin_data->dp_targ;
  float * dp_curr = plugin_data->dp_curr;
  
int min_size;

sample_rate = s_rate;

max_law_p = s_rate/2;
last_law_p = -1;
law_pos = 0;
law_roll = 0;

min_size = sample_rate / 10;
for (delay_size = 1024; delay_size < min_size; delay_size *= 2);
delay_mask = delay_size - 1;
delay_tbl = calloc(sizeof(float), delay_size);
delay_pos = 0;

prev_peak_pos = malloc(sizeof(unsigned int) * MAX_LAWS);
next_peak_pos = malloc(sizeof(unsigned int) * MAX_LAWS);
prev_peak_amp = malloc(sizeof(float) * MAX_LAWS);
next_peak_amp = malloc(sizeof(float) * MAX_LAWS);
dp_targ = malloc(sizeof(float) * MAX_LAWS);
dp_curr = malloc(sizeof(float) * MAX_LAWS);

count = 0;
    
  plugin_data->sample_rate = sample_rate;
  plugin_data->count = count;
  plugin_data->law_pos = law_pos;
  plugin_data->law_roll = law_roll;
  plugin_data->max_law_p = max_law_p;
  plugin_data->last_law_p = last_law_p;
  plugin_data->delay_tbl = delay_tbl;
  plugin_data->delay_pos = delay_pos;
  plugin_data->delay_size = delay_size;
  plugin_data->delay_mask = delay_mask;
  plugin_data->prev_peak_pos = prev_peak_pos;
  plugin_data->next_peak_pos = next_peak_pos;
  plugin_data->prev_peak_amp = prev_peak_amp;
  plugin_data->next_peak_amp = next_peak_amp;
  plugin_data->dp_targ = dp_targ;
  plugin_data->dp_curr = dp_curr;
  
  return (LV2_Handle)plugin_data;
}


static void activateMultivoiceChorus(LV2_Handle instance)
{
  MultivoiceChorus *plugin_data = (MultivoiceChorus *)instance;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  long count __attribute__ ((unused)) = plugin_data->count;
  int law_pos __attribute__ ((unused)) = plugin_data->law_pos;
  int law_roll __attribute__ ((unused)) = plugin_data->law_roll;
  int max_law_p __attribute__ ((unused)) = plugin_data->max_law_p;
  int last_law_p __attribute__ ((unused)) = plugin_data->last_law_p;
  float * delay_tbl __attribute__ ((unused)) = plugin_data->delay_tbl;
  unsigned int delay_pos __attribute__ ((unused)) = plugin_data->delay_pos;
  unsigned int delay_size __attribute__ ((unused)) = plugin_data->delay_size;
  unsigned int delay_mask __attribute__ ((unused)) = plugin_data->delay_mask;
  unsigned int * prev_peak_pos __attribute__ ((unused)) = plugin_data->prev_peak_pos;
  unsigned int * next_peak_pos __attribute__ ((unused)) = plugin_data->next_peak_pos;
  float * prev_peak_amp __attribute__ ((unused)) = plugin_data->prev_peak_amp;
  float * next_peak_amp __attribute__ ((unused)) = plugin_data->next_peak_amp;
  float * dp_targ __attribute__ ((unused)) = plugin_data->dp_targ;
  float * dp_curr __attribute__ ((unused)) = plugin_data->dp_curr;
  
memset(delay_tbl, 0, sizeof(float) * delay_size);
memset(prev_peak_pos, 0, sizeof(unsigned int) * MAX_LAWS);
memset(next_peak_pos, 0, sizeof(unsigned int) * MAX_LAWS);
memset(prev_peak_amp, 0, sizeof(float) * MAX_LAWS);
memset(next_peak_amp, 0, sizeof(float) * MAX_LAWS);
memset(dp_targ, 0, sizeof(float) * MAX_LAWS);
memset(dp_curr, 0, sizeof(float) * MAX_LAWS);
    
}


static void runMultivoiceChorus(LV2_Handle instance, uint32_t sample_count)
{
  MultivoiceChorus *plugin_data = (MultivoiceChorus *)instance;

  const float voices = *(plugin_data->voices);
  const float delay_base = *(plugin_data->delay_base);
  const float voice_spread = *(plugin_data->voice_spread);
  const float detune = *(plugin_data->detune);
  const float law_freq = *(plugin_data->law_freq);
  const float attendb = *(plugin_data->attendb);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  long sample_rate = plugin_data->sample_rate;
  long count = plugin_data->count;
  int law_pos = plugin_data->law_pos;
  int law_roll = plugin_data->law_roll;
  int max_law_p = plugin_data->max_law_p;
  int last_law_p = plugin_data->last_law_p;
  float * delay_tbl = plugin_data->delay_tbl;
  unsigned int delay_pos = plugin_data->delay_pos;
  unsigned int delay_size = plugin_data->delay_size;
  unsigned int delay_mask = plugin_data->delay_mask;
  unsigned int * prev_peak_pos = plugin_data->prev_peak_pos;
  unsigned int * next_peak_pos = plugin_data->next_peak_pos;
  float * prev_peak_amp = plugin_data->prev_peak_amp;
  float * next_peak_amp = plugin_data->next_peak_amp;
  float * dp_targ = plugin_data->dp_targ;
  float * dp_curr = plugin_data->dp_curr;
  
unsigned long pos;
int d_base, t;
float out;
float delay_depth;
float dp; // float delay position
float dp_frac; // fractional part
int dp_idx; // Integer delay index
int laws, law_separation, base_offset;
int law_p; // Period of law
float atten; // Attenuation

// Set law params
laws = LIMIT(f_round(voices) - 1, 0, 7);
law_p = LIMIT(f_round(sample_rate/f_clamp(law_freq, 0.0001f, 1000.0f)), 1, max_law_p);
if (laws > 0) {
	law_separation = law_p / laws;
} else {
	law_separation = 0;
}

// Calculate voice spread in samples
base_offset = (f_clamp(voice_spread, 0.0f, 2.0f) * sample_rate) / 1000;
// Calculate base delay size in samples
d_base = (f_clamp(delay_base, 5.0f, 40.0f) * sample_rate) / 1000;
// Calculate delay depth in samples
delay_depth = f_clamp((law_p * f_clamp(detune, 0.0f, 10.0f)) / (100.0f * M_PI), 0.0f, delay_size - d_base - 1 - (base_offset * laws));

// Calculate output attenuation
atten = DB_CO(f_clamp(attendb, -100.0, 24.0));

for (pos = 0; pos < sample_count; pos++) {
	// N times per law 'frequency' splurge a new set of windowed data
	// into one of the N law buffers. Keeps the laws out of phase.
	if (laws > 0 && (count % law_separation) == 0) {
		next_peak_amp[law_roll] = (float)rand() / (float)RAND_MAX;
		next_peak_pos[law_roll] = count + law_p;
	}
	if (laws > 0 && (count % law_separation) == law_separation/2) {
		prev_peak_amp[law_roll] = (float)rand() / (float)RAND_MAX;
		prev_peak_pos[law_roll] = count + law_p;
		// Pick the next law to be changed
		law_roll = (law_roll + 1) % laws;
	}

	out = input[pos];
	if (count % 16 < laws) {
		unsigned int t = count % 16;
		// Calculate sinus phases
		float n_ph = (float)(law_p - abs(next_peak_pos[t] - count))/law_p;
		float p_ph = n_ph + 0.5f;
		if (p_ph > 1.0f) {
			p_ph -= 1.0f;
		}

		dp_targ[t] = f_sin_sq(3.1415926f*p_ph)*prev_peak_amp[t] + f_sin_sq(3.1415926f*n_ph)*next_peak_amp[t];
	}
	for (t=0; t<laws; t++) {
		dp_curr[t] = 0.9f*dp_curr[t] + 0.1f*dp_targ[t];
		//dp_curr[t] = dp_targ[t];
		dp = (float)(delay_pos + d_base - (t*base_offset)) - delay_depth * dp_curr[t];
		// Get the integer part
		dp_idx = f_round(dp-0.5f);
		// Get the fractional part
		dp_frac = dp - dp_idx;
		// Calculate the modulo'd table index
		dp_idx = dp_idx & delay_mask;

		// Accumulate into output buffer
		out += cube_interp(dp_frac, delay_tbl[(dp_idx-1) & delay_mask], delay_tbl[dp_idx], delay_tbl[(dp_idx+1) & delay_mask], delay_tbl[(dp_idx+2) & delay_mask]);
	}
	law_pos = (law_pos + 1) % (max_law_p * 2);

	// Store new delay value
	delay_tbl[delay_pos] = input[pos];
	delay_pos = (delay_pos + 1) & delay_mask;

	buffer_write(output[pos], out * atten);
	count++;
}

plugin_data->count = count;
plugin_data->law_pos = law_pos;
plugin_data->last_law_p = last_law_p;
plugin_data->law_roll = law_roll;
plugin_data->delay_pos = delay_pos;
    
}

static const LV2_Descriptor multivoiceChorusDescriptor = {
  "http://plugin.org.uk/swh-plugins/multivoiceChorus",
  instantiateMultivoiceChorus,
  connectPortMultivoiceChorus,
  activateMultivoiceChorus,
  runMultivoiceChorus,
  NULL,
  cleanupMultivoiceChorus,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &multivoiceChorusDescriptor;
  default:
    return NULL;
  }
}
