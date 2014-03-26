
      #include <sys/types.h>
      #include "ladspa-util.h"

      #define DELAY_TIME 0.005f
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _DjFlanger {
  float *sync;
  float *period;
  float *depth;
  float *feedback;
  float *input;
  float *output;
float * buffer;
unsigned int buffer_pos;
unsigned int buffer_mask;
float fs;
float x;
float y;
unsigned int last_sync;
} DjFlanger;

static void cleanupDjFlanger(LV2_Handle instance)
{
DjFlanger *plugin_data = (DjFlanger *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortDjFlanger(LV2_Handle instance, uint32_t port, void *data)
{
  DjFlanger *plugin = (DjFlanger *)instance;

  switch (port) {
  case 0:
    plugin->sync = data;
    break;
  case 1:
    plugin->period = data;
    break;
  case 2:
    plugin->depth = data;
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

static LV2_Handle instantiateDjFlanger(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  DjFlanger *plugin_data = (DjFlanger *)malloc(sizeof(DjFlanger));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  float fs = plugin_data->fs;
  float x = plugin_data->x;
  float y = plugin_data->y;
  unsigned int last_sync = plugin_data->last_sync;
  
      int buffer_size = 2048;

      fs = s_rate;
      while (buffer_size < fs * DELAY_TIME + 3.0f) {
	buffer_size *= 2;
      }
      buffer = calloc(buffer_size, sizeof(float));
      buffer_mask = buffer_size - 1;
      buffer_pos = 0;
      x = 0.5f;
      y = 0.0f;
      last_sync = 0;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->fs = fs;
  plugin_data->x = x;
  plugin_data->y = y;
  plugin_data->last_sync = last_sync;
  
  return (LV2_Handle)plugin_data;
}


static void activateDjFlanger(LV2_Handle instance)
{
  DjFlanger *plugin_data = (DjFlanger *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  float x __attribute__ ((unused)) = plugin_data->x;
  float y __attribute__ ((unused)) = plugin_data->y;
  unsigned int last_sync __attribute__ ((unused)) = plugin_data->last_sync;
  
      memset(buffer, 0, (buffer_mask + 1) * sizeof(float));
      last_sync = 0;
    
}


static void runDjFlanger(LV2_Handle instance, uint32_t sample_count)
{
  DjFlanger *plugin_data = (DjFlanger *)instance;

  const float sync = *(plugin_data->sync);
  const float period = *(plugin_data->period);
  const float depth = *(plugin_data->depth);
  const float feedback = *(plugin_data->feedback);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float * buffer = plugin_data->buffer;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  float fs = plugin_data->fs;
  float x = plugin_data->x;
  float y = plugin_data->y;
  unsigned int last_sync = plugin_data->last_sync;
  
      unsigned long pos;
      const float omega = 6.2831852f / (period * fs);
      const float dr = 0.001f * fs * depth;
      float fb;
      float d;
      float dout, out;
      unsigned int dof;

      if (feedback > 99.0f) {
	fb = 0.99f;
      } else if (feedback < -99.0f) {
	fb = -0.99f;
      } else {
	fb = feedback * 0.01f;
      }

      if (sync > 0) {
	if (!last_sync) {
          x = 0.5f;
          y = 0.0f;
        }
	plugin_data->last_sync = 1;
      } else {
	plugin_data->last_sync = 0;
      }

      for (pos = 0; pos < sample_count; pos++) {
	/* Write input into delay line */
	buffer[buffer_pos] = input[pos];

	/* Calcuate delay */
	d = (x + 0.5f) * dr;

	dof = f_round(d);
	//dout = buffer[(buffer_pos - f_round(d)) & buffer_mask];
	dout = cube_interp(d - floor(d),
			   buffer[(buffer_pos - dof - 3) & buffer_mask],
                           buffer[(buffer_pos - dof - 2) & buffer_mask],
                           buffer[(buffer_pos - dof - 1) & buffer_mask],
                           buffer[(buffer_pos - dof) & buffer_mask]);

	/* Write output */
	out = (buffer[buffer_pos] + dout) * 0.5f;
	buffer[buffer_pos] = input[pos] + out * fb;
	buffer_write(output[pos], out);

	/* Roll ringbuffer */
	buffer_pos = (buffer_pos + 1) & buffer_mask;

	/* Run LFO */
	x -= omega * y;
	y += omega * x;
      }

      plugin_data->x = x;
      plugin_data->y = y;
      plugin_data->buffer_pos = buffer_pos;
    
}

static const LV2_Descriptor djFlangerDescriptor = {
  "http://plugin.org.uk/swh-plugins/djFlanger",
  instantiateDjFlanger,
  connectPortDjFlanger,
  activateDjFlanger,
  runDjFlanger,
  NULL,
  cleanupDjFlanger,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &djFlangerDescriptor;
  default:
    return NULL;
  }
}
