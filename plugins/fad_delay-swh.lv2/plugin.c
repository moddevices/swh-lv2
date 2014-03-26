
#include "ladspa-util.h"

#define BASE_BUFFER 8 // Base buffer length (s)
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _FadDelay {
  float *delay;
  float *fb_db;
  float *input;
  float *output;
float * buffer;
float phase;
int last_phase;
float last_in;
unsigned long buffer_size;
unsigned long buffer_mask;
long sample_rate;
} FadDelay;

static void cleanupFadDelay(LV2_Handle instance)
{
FadDelay *plugin_data = (FadDelay *)instance;

			free(plugin_data->buffer);
		
  free(instance);
}

static void connectPortFadDelay(LV2_Handle instance, uint32_t port, void *data)
{
  FadDelay *plugin = (FadDelay *)instance;

  switch (port) {
  case 0:
    plugin->delay = data;
    break;
  case 1:
    plugin->fb_db = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateFadDelay(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  FadDelay *plugin_data = (FadDelay *)malloc(sizeof(FadDelay));
  float * buffer = plugin_data->buffer;
  float phase = plugin_data->phase;
  int last_phase = plugin_data->last_phase;
  float last_in = plugin_data->last_in;
  unsigned long buffer_size = plugin_data->buffer_size;
  unsigned long buffer_mask = plugin_data->buffer_mask;
  long sample_rate = plugin_data->sample_rate;
  
			unsigned int min_bs;

			sample_rate = s_rate;
			min_bs = BASE_BUFFER * s_rate;
			for (buffer_size = 4096; buffer_size < min_bs;
			     buffer_size *= 2);
			buffer = calloc(buffer_size, sizeof(float));
			buffer_mask = buffer_size - 1;
			phase = 0;
			last_phase = 0;
			last_in = 0.0f;
		
  plugin_data->buffer = buffer;
  plugin_data->phase = phase;
  plugin_data->last_phase = last_phase;
  plugin_data->last_in = last_in;
  plugin_data->buffer_size = buffer_size;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activateFadDelay(LV2_Handle instance)
{
  FadDelay *plugin_data = (FadDelay *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  float phase __attribute__ ((unused)) = plugin_data->phase;
  int last_phase __attribute__ ((unused)) = plugin_data->last_phase;
  float last_in __attribute__ ((unused)) = plugin_data->last_in;
  unsigned long buffer_size __attribute__ ((unused)) = plugin_data->buffer_size;
  unsigned long buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  
			int i;

			for (i = 0; i < buffer_size; i++) {
				buffer[i] = 0;
			}
			phase = 0;
			last_phase = 0;
			last_in = 0.0f;
			sample_rate = sample_rate;
		
}


static void runFadDelay(LV2_Handle instance, uint32_t sample_count)
{
  FadDelay *plugin_data = (FadDelay *)instance;

  const float delay = *(plugin_data->delay);
  const float fb_db = *(plugin_data->fb_db);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float * buffer = plugin_data->buffer;
  float phase = plugin_data->phase;
  int last_phase = plugin_data->last_phase;
  float last_in = plugin_data->last_in;
  unsigned long buffer_size = plugin_data->buffer_size;
  unsigned long buffer_mask = plugin_data->buffer_mask;
  long sample_rate = plugin_data->sample_rate;
  
long int pos;
float increment = (float)buffer_size / ((float)sample_rate *
					f_max(fabs(delay), 0.01));
float lin_int, lin_inc;
int track;
int fph;
float out;
const float fb = DB_CO(fb_db);

for (pos = 0; pos < sample_count; pos++) {
	fph = f_round(floor(phase));
	last_phase = fph;
	lin_int = phase - (float)fph;
	out = LIN_INTERP(lin_int, buffer[(fph+1) & buffer_mask],
	 buffer[(fph+2) & buffer_mask]);
	phase += increment;
	lin_inc = 1.0f / (floor(phase) - last_phase + 1);
	lin_inc = lin_inc > 1.0f ? 1.0f : lin_inc;
	lin_int = 0.0f;
	for (track = last_phase; track < phase; track++) {
		lin_int += lin_inc;
		buffer[track % buffer_size] = out * fb +
		 LIN_INTERP(lin_int, last_in, input[pos]);
	}
	last_in = input[pos];
	buffer_write(output[pos], out);
	if (phase >= buffer_size) {
		phase -= buffer_size;
	}
}

// Store current phase in instance
plugin_data->phase = phase;
plugin_data->last_phase = last_phase;
plugin_data->last_in = last_in;
		
}

static const LV2_Descriptor fadDelayDescriptor = {
  "http://plugin.org.uk/swh-plugins/fadDelay",
  instantiateFadDelay,
  connectPortFadDelay,
  activateFadDelay,
  runFadDelay,
  NULL,
  cleanupFadDelay,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &fadDelayDescriptor;
  default:
    return NULL;
  }
}
