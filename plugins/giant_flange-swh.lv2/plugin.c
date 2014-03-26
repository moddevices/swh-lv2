
      #include <sys/types.h>
      #include "ladspa-util.h"

      #define INT_SCALE   16384.0f
      /* INT_SCALE reciprocal includes factor of two scaling */
      #define INT_SCALE_R 0.000030517578125f

      #define MAX_AMP 1.0f
      #define CLIP 0.8f
      #define CLIP_A ((MAX_AMP - CLIP) * (MAX_AMP - CLIP))
      #define CLIP_B (MAX_AMP - 2.0f * CLIP)
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _GiantFlange {
  float *deldouble;
  float *freq1;
  float *delay1;
  float *freq2;
  float *delay2;
  float *feedback;
  float *wet;
  float *input;
  float *output;
int16_t * buffer;
unsigned int buffer_pos;
unsigned int buffer_mask;
float fs;
float x1;
float y1;
float x2;
float y2;
} GiantFlange;

static void cleanupGiantFlange(LV2_Handle instance)
{
GiantFlange *plugin_data = (GiantFlange *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortGiantFlange(LV2_Handle instance, uint32_t port, void *data)
{
  GiantFlange *plugin = (GiantFlange *)instance;

  switch (port) {
  case 0:
    plugin->deldouble = data;
    break;
  case 1:
    plugin->freq1 = data;
    break;
  case 2:
    plugin->delay1 = data;
    break;
  case 3:
    plugin->freq2 = data;
    break;
  case 4:
    plugin->delay2 = data;
    break;
  case 5:
    plugin->feedback = data;
    break;
  case 6:
    plugin->wet = data;
    break;
  case 7:
    plugin->input = data;
    break;
  case 8:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateGiantFlange(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  GiantFlange *plugin_data = (GiantFlange *)malloc(sizeof(GiantFlange));
  int16_t * buffer = plugin_data->buffer;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  float fs = plugin_data->fs;
  float x1 = plugin_data->x1;
  float y1 = plugin_data->y1;
  float x2 = plugin_data->x2;
  float y2 = plugin_data->y2;
  
      int buffer_size = 32768;

      fs = s_rate;
      while (buffer_size < fs * 10.5f) {
	buffer_size *= 2;
      }
      buffer = calloc(buffer_size, sizeof(int16_t));
      buffer_mask = buffer_size - 1;
      buffer_pos = 0;
      x1 = 0.5f;
      y1 = 0.0f;
      x2 = 0.5f;
      y2 = 0.0f;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->fs = fs;
  plugin_data->x1 = x1;
  plugin_data->y1 = y1;
  plugin_data->x2 = x2;
  plugin_data->y2 = y2;
  
  return (LV2_Handle)plugin_data;
}


static void activateGiantFlange(LV2_Handle instance)
{
  GiantFlange *plugin_data = (GiantFlange *)instance;
  int16_t * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  float x1 __attribute__ ((unused)) = plugin_data->x1;
  float y1 __attribute__ ((unused)) = plugin_data->y1;
  float x2 __attribute__ ((unused)) = plugin_data->x2;
  float y2 __attribute__ ((unused)) = plugin_data->y2;
  
      memset(buffer, 0, (buffer_mask + 1) * sizeof(int16_t));
    
}


static void runGiantFlange(LV2_Handle instance, uint32_t sample_count)
{
  GiantFlange *plugin_data = (GiantFlange *)instance;

  const float deldouble = *(plugin_data->deldouble);
  const float freq1 = *(plugin_data->freq1);
  const float delay1 = *(plugin_data->delay1);
  const float freq2 = *(plugin_data->freq2);
  const float delay2 = *(plugin_data->delay2);
  const float feedback = *(plugin_data->feedback);
  const float wet = *(plugin_data->wet);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  int16_t * buffer = plugin_data->buffer;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  float fs = plugin_data->fs;
  float x1 = plugin_data->x1;
  float y1 = plugin_data->y1;
  float x2 = plugin_data->x2;
  float y2 = plugin_data->y2;
  
      unsigned long pos;
      const float omega1 = 6.2831852f * (freq1 / fs);
      const float omega2 = 6.2831852f * (freq2 / fs);
      float fb;
      float d1, d2;
      float d1out, d2out;
      float fbs;

      if (feedback > 99.0f) {
	fb = 0.99f;
      } else if (feedback < -99.0f) {
	fb = -0.99f;
      } else {
	fb = feedback * 0.01f;
      }

      if (f_round(deldouble)) {
        const float dr1 = delay1 * fs * 0.25f;
        const float dr2 = delay2 * fs * 0.25f;

      for (pos = 0; pos < sample_count; pos++) {
	/* Write input into delay line */
	buffer[buffer_pos] = f_round(input[pos] * INT_SCALE);

	/* Calcuate delays */
	d1 = (x1 + 1.0f) * dr1;
	d2 = (y2 + 1.0f) * dr2;

	d1out = buffer[(buffer_pos - f_round(d1)) & buffer_mask] * INT_SCALE_R;
	d2out = buffer[(buffer_pos - f_round(d2)) & buffer_mask] * INT_SCALE_R;

	/* Add feedback, must be done afterwards for case where delay = 0 */
	fbs = input[pos] + (d1out + d2out) * fb;
	if(fbs < CLIP && fbs > -CLIP) {
	  buffer[buffer_pos] = fbs * INT_SCALE;
	} else if (fbs > 0.0f) {
	  buffer[buffer_pos] = (MAX_AMP - (CLIP_A / (CLIP_B + fbs))) *
					INT_SCALE;
	} else {
	  buffer[buffer_pos] =  (MAX_AMP - (CLIP_A / (CLIP_B - fbs))) *
					-INT_SCALE;
	}

	/* Write output */
	buffer_write(output[pos], LIN_INTERP(wet, input[pos], d1out + d2out));

	if (pos % 2) {
	  buffer_pos = (buffer_pos + 1) & buffer_mask;
	}

	/* Run LFOs */
	x1 -= omega1 * y1;
	y1 += omega1 * x1;
	x2 -= omega2 * y2;
	y2 += omega2 * x2;
      }
      } else {
        const float dr1 = delay1 * fs * 0.5f;
        const float dr2 = delay2 * fs * 0.5f;

      for (pos = 0; pos < sample_count; pos++) {
	/* Write input into delay line */
	buffer[buffer_pos] = f_round(input[pos] * INT_SCALE);

	/* Calcuate delays */
	d1 = (x1 + 1.0f) * dr1;
	d2 = (y2 + 1.0f) * dr2;

	d1out = buffer[(buffer_pos - f_round(d1)) & buffer_mask] * INT_SCALE_R;
	d2out = buffer[(buffer_pos - f_round(d2)) & buffer_mask] * INT_SCALE_R;

	/* Add feedback, must be done afterwards for case where delay = 0 */
	fbs = input[pos] + (d1out + d2out) * fb;
	if(fbs < CLIP && fbs > -CLIP) {
		buffer[buffer_pos] = fbs * INT_SCALE;
	} else if (fbs > 0.0f) {
		buffer[buffer_pos] = (MAX_AMP - (CLIP_A / (CLIP_B + fbs))) *
					INT_SCALE;
	} else {
		buffer[buffer_pos] =  (MAX_AMP - (CLIP_A / (CLIP_B - fbs))) *
					-INT_SCALE;
	}

	/* Write output */
	buffer_write(output[pos], LIN_INTERP(wet, input[pos], d1out + d2out));

	buffer_pos = (buffer_pos + 1) & buffer_mask;

	/* Run LFOs */
	x1 -= omega1 * y1;
	y1 += omega1 * x1;
	x2 -= omega2 * y2;
	y2 += omega2 * x2;
      }
      }

      plugin_data->x1 = x1;
      plugin_data->y1 = y1;
      plugin_data->x2 = x2;
      plugin_data->y2 = y2;
      plugin_data->buffer_pos = buffer_pos;
    
}

static const LV2_Descriptor giantFlangeDescriptor = {
  "http://plugin.org.uk/swh-plugins/giantFlange",
  instantiateGiantFlange,
  connectPortGiantFlange,
  activateGiantFlange,
  runGiantFlange,
  NULL,
  cleanupGiantFlange,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &giantFlangeDescriptor;
  default:
    return NULL;
  }
}
