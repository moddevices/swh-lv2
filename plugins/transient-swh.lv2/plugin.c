
#include "ladspa-util.h"

#define BUFFER_SIZE 10240
#define SSTAB 0.00001f
#define ASTAB 0.02f
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Transient {
  float *attack;
  float *sustain;
  float *input;
  float *output;
long count;
float fast_track;
float medi_track;
float slow_track;
float * buffer;
int buffer_pos;
float fast_buffer_sum;
float medi_buffer_sum;
float slow_buffer_sum;
int sample_rate;
} Transient;

static void cleanupTransient(LV2_Handle instance)
{
Transient *plugin_data = (Transient *)instance;

free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortTransient(LV2_Handle instance, uint32_t port, void *data)
{
  Transient *plugin = (Transient *)instance;

  switch (port) {
  case 0:
    plugin->attack = data;
    break;
  case 1:
    plugin->sustain = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateTransient(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Transient *plugin_data = (Transient *)malloc(sizeof(Transient));
  long count = plugin_data->count;
  float fast_track = plugin_data->fast_track;
  float medi_track = plugin_data->medi_track;
  float slow_track = plugin_data->slow_track;
  float * buffer = plugin_data->buffer;
  int buffer_pos = plugin_data->buffer_pos;
  float fast_buffer_sum = plugin_data->fast_buffer_sum;
  float medi_buffer_sum = plugin_data->medi_buffer_sum;
  float slow_buffer_sum = plugin_data->slow_buffer_sum;
  int sample_rate = plugin_data->sample_rate;
  
buffer = calloc(BUFFER_SIZE, sizeof(float));
fast_buffer_sum = 0.1;
medi_buffer_sum = 0.1;
slow_buffer_sum = 0.1;
buffer_pos = 0;
fast_track = 0.0;
medi_track = 0.0;
slow_track = 0.0;
count = 0;
sample_rate = s_rate;
    
  plugin_data->count = count;
  plugin_data->fast_track = fast_track;
  plugin_data->medi_track = medi_track;
  plugin_data->slow_track = slow_track;
  plugin_data->buffer = buffer;
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->fast_buffer_sum = fast_buffer_sum;
  plugin_data->medi_buffer_sum = medi_buffer_sum;
  plugin_data->slow_buffer_sum = slow_buffer_sum;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activateTransient(LV2_Handle instance)
{
  Transient *plugin_data = (Transient *)instance;
  long count __attribute__ ((unused)) = plugin_data->count;
  float fast_track __attribute__ ((unused)) = plugin_data->fast_track;
  float medi_track __attribute__ ((unused)) = plugin_data->medi_track;
  float slow_track __attribute__ ((unused)) = plugin_data->slow_track;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  float fast_buffer_sum __attribute__ ((unused)) = plugin_data->fast_buffer_sum;
  float medi_buffer_sum __attribute__ ((unused)) = plugin_data->medi_buffer_sum;
  float slow_buffer_sum __attribute__ ((unused)) = plugin_data->slow_buffer_sum;
  int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  
memset(buffer, 0, BUFFER_SIZE * sizeof(float));
fast_buffer_sum = 0.1;
medi_buffer_sum = 0.1;
slow_buffer_sum = 0.1;
buffer_pos = 0;
fast_track = 0.1;
medi_track = 0.1;
slow_track = 0.1;
count = 0;
sample_rate = sample_rate;
    
}


static void runTransient(LV2_Handle instance, uint32_t sample_count)
{
  Transient *plugin_data = (Transient *)instance;

  const float attack = *(plugin_data->attack);
  const float sustain = *(plugin_data->sustain);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  long count = plugin_data->count;
  float fast_track = plugin_data->fast_track;
  float medi_track = plugin_data->medi_track;
  float slow_track = plugin_data->slow_track;
  float * buffer = plugin_data->buffer;
  int buffer_pos = plugin_data->buffer_pos;
  float fast_buffer_sum = plugin_data->fast_buffer_sum;
  float medi_buffer_sum = plugin_data->medi_buffer_sum;
  float slow_buffer_sum = plugin_data->slow_buffer_sum;
  int sample_rate = plugin_data->sample_rate;
  
unsigned long pos;
const int fast_sum_size = (2 * sample_rate) / 1000;
const int medi_sum_size = (25 * sample_rate) / 1000;
const int slow_sum_size = (100 * sample_rate) / 1000;
const float fast_track_lag = 1.5f / fast_sum_size;
const float medi_track_lag = 1.0f / medi_sum_size;
const float slow_track_lag = 1.3f / slow_sum_size;
float ratio;
float in;

for (pos = 0; pos < sample_count; pos++) {
	in = input[pos];
	buffer[buffer_pos] = fabs(in);
	fast_buffer_sum += buffer[buffer_pos];
	medi_buffer_sum += buffer[buffer_pos];
	slow_buffer_sum += buffer[buffer_pos];
	fast_buffer_sum -= buffer[MOD(buffer_pos - fast_sum_size, BUFFER_SIZE)];
	medi_buffer_sum -= buffer[MOD(buffer_pos - medi_sum_size, BUFFER_SIZE)];
	slow_buffer_sum -= buffer[MOD(buffer_pos - slow_sum_size, BUFFER_SIZE)];
	if (count++ > slow_sum_size) {
		fast_track += (fast_buffer_sum/fast_sum_size - fast_track)
		 * fast_track_lag;
		medi_track += (medi_buffer_sum/medi_sum_size - medi_track)
		 * medi_track_lag;
		slow_track += (slow_buffer_sum/slow_sum_size - slow_track)
		 * slow_track_lag;
	}

	// Attack
	ratio = (fast_track + ASTAB) / (medi_track + ASTAB);
	if (ratio * attack > 1.0f) {
		in *= ratio * attack;
	} else if (ratio * attack < -1.0f) {
		in /= ratio * -attack;
	}

	// Sustain
	ratio = (slow_track + SSTAB) / (medi_track + SSTAB);
	if (ratio * sustain > 1.0f) {
		in *= ratio * sustain;
	} else if (ratio * sustain < -1.0f) {
		in /= ratio * -sustain;
	}

	buffer_write(output[pos], in);
	buffer_pos = (buffer_pos + 1) % BUFFER_SIZE;
}

plugin_data->count = count;
plugin_data->fast_track = fast_track;
plugin_data->medi_track = medi_track;
plugin_data->slow_track = slow_track;
plugin_data->buffer_pos = buffer_pos;
plugin_data->fast_buffer_sum = fast_buffer_sum;
plugin_data->medi_buffer_sum = medi_buffer_sum;
plugin_data->slow_buffer_sum = slow_buffer_sum;
    
}

static const LV2_Descriptor transientDescriptor = {
  "http://plugin.org.uk/swh-plugins/transient",
  instantiateTransient,
  connectPortTransient,
  activateTransient,
  runTransient,
  NULL,
  cleanupTransient,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &transientDescriptor;
  default:
    return NULL;
  }
}
