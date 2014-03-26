
#include "ladspa-util.h"

#define BASE_BUFFER 0.001 // Base buffer length (s)

inline float sat(float x, float q,  float dist) {
	if (x == q) {
		return 1.0f / dist + q / (1.0f - f_exp(dist * q));
	}
	return ((x - q) / (1.0f - f_exp(-dist * (x - q))) + q /
	 (1.0f - f_exp(dist * q)));
}

		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _RetroFlange {
  float *delay_depth_avg;
  float *law_freq;
  float *input;
  float *output;
float * buffer;
float phase;
int last_phase;
float last_in;
long buffer_size;
long sample_rate;
long count;
int max_law_p;
int last_law_p;
int delay_pos;
int delay_line_length;
float * delay_line;
float z0;
float z1;
float z2;
float prev_law_peak;
float next_law_peak;
int prev_law_pos;
int next_law_pos;
} RetroFlange;

static void cleanupRetroFlange(LV2_Handle instance)
{
RetroFlange *plugin_data = (RetroFlange *)instance;

free(plugin_data->delay_line);
free(plugin_data->buffer);
		
  free(instance);
}

static void connectPortRetroFlange(LV2_Handle instance, uint32_t port, void *data)
{
  RetroFlange *plugin = (RetroFlange *)instance;

  switch (port) {
  case 0:
    plugin->delay_depth_avg = data;
    break;
  case 1:
    plugin->law_freq = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateRetroFlange(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  RetroFlange *plugin_data = (RetroFlange *)malloc(sizeof(RetroFlange));
  float * buffer = plugin_data->buffer;
  float phase = plugin_data->phase;
  int last_phase = plugin_data->last_phase;
  float last_in = plugin_data->last_in;
  long buffer_size = plugin_data->buffer_size;
  long sample_rate = plugin_data->sample_rate;
  long count = plugin_data->count;
  int max_law_p = plugin_data->max_law_p;
  int last_law_p = plugin_data->last_law_p;
  int delay_pos = plugin_data->delay_pos;
  int delay_line_length = plugin_data->delay_line_length;
  float * delay_line = plugin_data->delay_line;
  float z0 = plugin_data->z0;
  float z1 = plugin_data->z1;
  float z2 = plugin_data->z2;
  float prev_law_peak = plugin_data->prev_law_peak;
  float next_law_peak = plugin_data->next_law_peak;
  int prev_law_pos = plugin_data->prev_law_pos;
  int next_law_pos = plugin_data->next_law_pos;
  
sample_rate = s_rate;
buffer_size = BASE_BUFFER * s_rate;
buffer = calloc(buffer_size, sizeof(float));
phase = 0;
last_phase = 0;
last_in = 0.0f;
max_law_p = s_rate*2;
last_law_p = -1;
delay_line_length = sample_rate * 0.01f;
delay_line = calloc(sizeof(float), delay_line_length);

delay_pos = 0;
count = 0;

prev_law_peak = 0.0f;
next_law_peak = 1.0f;
prev_law_pos = 0;
next_law_pos = 10;

z0 = 0.0f;
z1 = 0.0f;
z2 = 0.0f;
		
  plugin_data->buffer = buffer;
  plugin_data->phase = phase;
  plugin_data->last_phase = last_phase;
  plugin_data->last_in = last_in;
  plugin_data->buffer_size = buffer_size;
  plugin_data->sample_rate = sample_rate;
  plugin_data->count = count;
  plugin_data->max_law_p = max_law_p;
  plugin_data->last_law_p = last_law_p;
  plugin_data->delay_pos = delay_pos;
  plugin_data->delay_line_length = delay_line_length;
  plugin_data->delay_line = delay_line;
  plugin_data->z0 = z0;
  plugin_data->z1 = z1;
  plugin_data->z2 = z2;
  plugin_data->prev_law_peak = prev_law_peak;
  plugin_data->next_law_peak = next_law_peak;
  plugin_data->prev_law_pos = prev_law_pos;
  plugin_data->next_law_pos = next_law_pos;
  
  return (LV2_Handle)plugin_data;
}


static void activateRetroFlange(LV2_Handle instance)
{
  RetroFlange *plugin_data = (RetroFlange *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  float phase __attribute__ ((unused)) = plugin_data->phase;
  int last_phase __attribute__ ((unused)) = plugin_data->last_phase;
  float last_in __attribute__ ((unused)) = plugin_data->last_in;
  long buffer_size __attribute__ ((unused)) = plugin_data->buffer_size;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  long count __attribute__ ((unused)) = plugin_data->count;
  int max_law_p __attribute__ ((unused)) = plugin_data->max_law_p;
  int last_law_p __attribute__ ((unused)) = plugin_data->last_law_p;
  int delay_pos __attribute__ ((unused)) = plugin_data->delay_pos;
  int delay_line_length __attribute__ ((unused)) = plugin_data->delay_line_length;
  float * delay_line __attribute__ ((unused)) = plugin_data->delay_line;
  float z0 __attribute__ ((unused)) = plugin_data->z0;
  float z1 __attribute__ ((unused)) = plugin_data->z1;
  float z2 __attribute__ ((unused)) = plugin_data->z2;
  float prev_law_peak __attribute__ ((unused)) = plugin_data->prev_law_peak;
  float next_law_peak __attribute__ ((unused)) = plugin_data->next_law_peak;
  int prev_law_pos __attribute__ ((unused)) = plugin_data->prev_law_pos;
  int next_law_pos __attribute__ ((unused)) = plugin_data->next_law_pos;
  
memset(delay_line, 0, sizeof(float) * delay_line_length);
memset(buffer, 0, sizeof(float) * buffer_size);
z0 = 0.0f;
z1 = 0.0f;
z2 = 0.0f;

prev_law_peak = 0.0f;
next_law_peak = 1.0f;
prev_law_pos = 0;
next_law_pos = 10;
		
}


static void runRetroFlange(LV2_Handle instance, uint32_t sample_count)
{
  RetroFlange *plugin_data = (RetroFlange *)instance;

  const float delay_depth_avg = *(plugin_data->delay_depth_avg);
  const float law_freq = *(plugin_data->law_freq);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float * buffer = plugin_data->buffer;
  float phase = plugin_data->phase;
  int last_phase = plugin_data->last_phase;
  float last_in = plugin_data->last_in;
  long buffer_size = plugin_data->buffer_size;
  long sample_rate = plugin_data->sample_rate;
  long count = plugin_data->count;
  int max_law_p = plugin_data->max_law_p;
  int last_law_p = plugin_data->last_law_p;
  int delay_pos = plugin_data->delay_pos;
  int delay_line_length = plugin_data->delay_line_length;
  float * delay_line = plugin_data->delay_line;
  float z0 = plugin_data->z0;
  float z1 = plugin_data->z1;
  float z2 = plugin_data->z2;
  float prev_law_peak = plugin_data->prev_law_peak;
  float next_law_peak = plugin_data->next_law_peak;
  int prev_law_pos = plugin_data->prev_law_pos;
  int next_law_pos = plugin_data->next_law_pos;
  
long int pos;
int law_p = f_trunc(LIMIT(sample_rate / f_clamp(law_freq, 0.0001f, 100.0f), 1, max_law_p));
float increment;
float lin_int, lin_inc;
int track;
int fph;
float out = 0.0f;
const float dda_c = f_clamp(delay_depth_avg, 0.0f, 10.0f);
int dl_used = (dda_c * sample_rate) / 1000;
float inc_base = 1000.0f * (float)BASE_BUFFER;
const float delay_depth = 2.0f * dda_c;
float n_ph, p_ph, law;

for (pos = 0; pos < sample_count; pos++) {
	// Write into the delay line
	delay_line[delay_pos] = input[pos];
	z0 = delay_line[MOD(delay_pos - dl_used, delay_line_length)] + 0.12919609397f*z1 - 0.31050847f*z2;
	out = sat(z0*0.20466966f + z1*0.40933933f + z2*0.40933933f,
	                -0.23f, 3.3f);
	z2 = z1; z1 = z0;
	delay_pos = (delay_pos + 1) % delay_line_length;

        if ((count++ % law_p) == 0) {
		// Value for amplitude of law peak
		next_law_peak = (float)rand() / (float)RAND_MAX;
		next_law_pos = count + law_p;
	} else if (count % law_p == law_p / 2) {
		// Value for amplitude of law peak
		prev_law_peak = (float)rand() / (float)RAND_MAX;
		prev_law_pos = count + law_p;
        }

        n_ph = (float)(law_p - abs(next_law_pos - count))/(float)law_p;
        p_ph = n_ph + 0.5f;
        if (p_ph > 1.0f) {
                p_ph -= 1.0f;
        }
        law = f_sin_sq(3.1415926f*p_ph)*prev_law_peak +
                f_sin_sq(3.1415926f*n_ph)*next_law_peak;

	increment = inc_base / (delay_depth * law + 0.2);
	fph = f_trunc(phase);
	last_phase = fph;
	lin_int = phase - (float)fph;
	out += LIN_INTERP(lin_int, buffer[(fph+1) % buffer_size],
	 buffer[(fph+2) % buffer_size]);
	phase += increment;
	lin_inc = 1.0f / (floor(phase) - last_phase + 1);
	lin_inc = lin_inc > 1.0f ? 1.0f : lin_inc;
	lin_int = 0.0f;
	for (track = last_phase; track < phase; track++) {
		lin_int += lin_inc;
		buffer[track % buffer_size] =
		 LIN_INTERP(lin_int, last_in, input[pos]);
	}
	last_in = input[pos];
	buffer_write(output[pos], out * 0.707f);
	if (phase >= buffer_size) {
		phase -= buffer_size;
	}
}

// Store current phase in instance
plugin_data->phase = phase;
plugin_data->prev_law_peak = prev_law_peak;
plugin_data->next_law_peak = next_law_peak;
plugin_data->prev_law_pos = prev_law_pos;
plugin_data->next_law_pos = next_law_pos;
plugin_data->last_phase = last_phase;
plugin_data->last_in = last_in;
plugin_data->count = count;
plugin_data->last_law_p = last_law_p;
plugin_data->delay_pos = delay_pos;
plugin_data->z0 = z0;
plugin_data->z1 = z1;
plugin_data->z2 = z2;
		
}

static const LV2_Descriptor retroFlangeDescriptor = {
  "http://plugin.org.uk/swh-plugins/retroFlange",
  instantiateRetroFlange,
  connectPortRetroFlange,
  activateRetroFlange,
  runRetroFlange,
  NULL,
  cleanupRetroFlange,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &retroFlangeDescriptor;
  default:
    return NULL;
  }
}
