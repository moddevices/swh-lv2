
#include "ladspa-util.h"

#define BASE_BUFFER 8 // Tape length (inches)
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _TapeDelay {
  float *speed;
  float *da_db;
  float *t1d;
  float *t1a_db;
  float *t2d;
  float *t2a_db;
  float *t3d;
  float *t3a_db;
  float *t4d;
  float *t4a_db;
  float *input;
  float *output;
float * buffer;
unsigned int buffer_size;
unsigned int buffer_mask;
float phase;
unsigned int last_phase;
float last_in;
float last2_in;
float last3_in;
int sample_rate;
float z0;
float z1;
float z2;
} TapeDelay;

static void cleanupTapeDelay(LV2_Handle instance)
{
TapeDelay *plugin_data = (TapeDelay *)instance;

			free(plugin_data->buffer);
		
  free(instance);
}

static void connectPortTapeDelay(LV2_Handle instance, uint32_t port, void *data)
{
  TapeDelay *plugin = (TapeDelay *)instance;

  switch (port) {
  case 0:
    plugin->speed = data;
    break;
  case 1:
    plugin->da_db = data;
    break;
  case 2:
    plugin->t1d = data;
    break;
  case 3:
    plugin->t1a_db = data;
    break;
  case 4:
    plugin->t2d = data;
    break;
  case 5:
    plugin->t2a_db = data;
    break;
  case 6:
    plugin->t3d = data;
    break;
  case 7:
    plugin->t3a_db = data;
    break;
  case 8:
    plugin->t4d = data;
    break;
  case 9:
    plugin->t4a_db = data;
    break;
  case 10:
    plugin->input = data;
    break;
  case 11:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateTapeDelay(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  TapeDelay *plugin_data = (TapeDelay *)malloc(sizeof(TapeDelay));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_size = plugin_data->buffer_size;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  float phase = plugin_data->phase;
  unsigned int last_phase = plugin_data->last_phase;
  float last_in = plugin_data->last_in;
  float last2_in = plugin_data->last2_in;
  float last3_in = plugin_data->last3_in;
  int sample_rate = plugin_data->sample_rate;
  float z0 = plugin_data->z0;
  float z1 = plugin_data->z1;
  float z2 = plugin_data->z2;
  
			unsigned int mbs = BASE_BUFFER * s_rate;
			sample_rate = s_rate;
			for (buffer_size = 4096; buffer_size < mbs;
			     buffer_size *= 2);
			buffer = malloc(buffer_size * sizeof(float));
			buffer_mask = buffer_size - 1;
			phase = 0;
			last_phase = 0;
			last_in = 0.0f;
			last2_in = 0.0f;
			last3_in = 0.0f;
			z0 = 0.0f;
			z1 = 0.0f;
			z2 = 0.0f;
		
  plugin_data->buffer = buffer;
  plugin_data->buffer_size = buffer_size;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->phase = phase;
  plugin_data->last_phase = last_phase;
  plugin_data->last_in = last_in;
  plugin_data->last2_in = last2_in;
  plugin_data->last3_in = last3_in;
  plugin_data->sample_rate = sample_rate;
  plugin_data->z0 = z0;
  plugin_data->z1 = z1;
  plugin_data->z2 = z2;
  
  return (LV2_Handle)plugin_data;
}


static void activateTapeDelay(LV2_Handle instance)
{
  TapeDelay *plugin_data = (TapeDelay *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_size __attribute__ ((unused)) = plugin_data->buffer_size;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  float phase __attribute__ ((unused)) = plugin_data->phase;
  unsigned int last_phase __attribute__ ((unused)) = plugin_data->last_phase;
  float last_in __attribute__ ((unused)) = plugin_data->last_in;
  float last2_in __attribute__ ((unused)) = plugin_data->last2_in;
  float last3_in __attribute__ ((unused)) = plugin_data->last3_in;
  int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float z0 __attribute__ ((unused)) = plugin_data->z0;
  float z1 __attribute__ ((unused)) = plugin_data->z1;
  float z2 __attribute__ ((unused)) = plugin_data->z2;
  
			int i;

			for (i = 0; i < buffer_size; i++) {
				buffer[i] = 0;
			}
			phase = 0;
			last_phase = 0;
			last_in = 0.0f;
			last2_in = 0.0f;
			last3_in = 0.0f;
			sample_rate = sample_rate;
			z0 = 0.0f;
			z1 = 0.0f;
			z2 = 0.0f;
		
}


static void runTapeDelay(LV2_Handle instance, uint32_t sample_count)
{
  TapeDelay *plugin_data = (TapeDelay *)instance;

  const float speed = *(plugin_data->speed);
  const float da_db = *(plugin_data->da_db);
  const float t1d = *(plugin_data->t1d);
  const float t1a_db = *(plugin_data->t1a_db);
  const float t2d = *(plugin_data->t2d);
  const float t2a_db = *(plugin_data->t2a_db);
  const float t3d = *(plugin_data->t3d);
  const float t3a_db = *(plugin_data->t3a_db);
  const float t4d = *(plugin_data->t4d);
  const float t4a_db = *(plugin_data->t4a_db);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float * buffer = plugin_data->buffer;
  unsigned int buffer_size = plugin_data->buffer_size;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  float phase = plugin_data->phase;
  unsigned int last_phase = plugin_data->last_phase;
  float last_in = plugin_data->last_in;
  float last2_in = plugin_data->last2_in;
  float last3_in = plugin_data->last3_in;
  int sample_rate = plugin_data->sample_rate;
  float z0 = plugin_data->z0;
  float z1 = plugin_data->z1;
  float z2 = plugin_data->z2;
  
unsigned int pos;
float increment = f_clamp(speed, 0.0f, 40.0f);
float lin_int, lin_inc;
unsigned int track;
unsigned int fph;
float out;

const float da = DB_CO(da_db);
const float t1a = DB_CO(t1a_db);
const float t2a = DB_CO(t2a_db);
const float t3a = DB_CO(t3a_db);
const float t4a = DB_CO(t4a_db);
const unsigned int t1d_s = f_round(t1d * sample_rate);
const unsigned int t2d_s = f_round(t2d * sample_rate);
const unsigned int t3d_s = f_round(t3d * sample_rate);
const unsigned int t4d_s = f_round(t4d * sample_rate);

for (pos = 0; pos < sample_count; pos++) {
	fph = f_trunc(phase);
	last_phase = fph;
	lin_int = phase - (float)fph;

	out = buffer[(unsigned int)(fph - t1d_s) & buffer_mask] * t1a;
	out += buffer[(unsigned int)(fph - t2d_s) & buffer_mask] * t2a;
	out += buffer[(unsigned int)(fph - t3d_s) & buffer_mask] * t3a;
	out += buffer[(unsigned int)(fph - t4d_s) & buffer_mask] * t4a;

	phase += increment;
	lin_inc = 1.0f / (floor(phase) - last_phase + 1);
	lin_inc = lin_inc > 1.0f ? 1.0f : lin_inc;
	lin_int = 0.0f;
	for (track = last_phase; track < phase; track++) {
		lin_int += lin_inc;
		buffer[track & buffer_mask] =
		 cube_interp(lin_int, last3_in, last2_in, last_in, input[pos]);
	}
	last3_in = last2_in;
	last2_in = last_in;
	last_in = input[pos];
	out += input[pos] * da;
	buffer_write(output[pos], out);
	if (phase >= buffer_size) {
		phase -= buffer_size;
	}
}

// Store current phase in instance
plugin_data->phase = phase;
plugin_data->last_phase = last_phase;
plugin_data->last_in = last_in;
plugin_data->last2_in = last2_in;
plugin_data->last3_in = last3_in;
plugin_data->z0 = z0;
plugin_data->z1 = z1;
plugin_data->z2 = z2;
		
}

static const LV2_Descriptor tapeDelayDescriptor = {
  "http://plugin.org.uk/swh-plugins/tapeDelay",
  instantiateTapeDelay,
  connectPortTapeDelay,
  activateTapeDelay,
  runTapeDelay,
  NULL,
  cleanupTapeDelay,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &tapeDelayDescriptor;
  default:
    return NULL;
  }
}
