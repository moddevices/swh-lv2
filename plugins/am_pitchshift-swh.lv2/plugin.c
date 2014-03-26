
      #include <stdlib.h>
      #include <math.h>

      #include "ladspa-util.h"

      /* Beware of dependcies if you change this */
      #define DELAY_SIZE 8192
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _AmPitchshift {
  float *pitch;
  float *size;
  float *input;
  float *output;
  float *latency;
float * delay;
fixp16 rptr;
unsigned int wptr;
int last_size;
unsigned int delay_mask;
unsigned int delay_ofs;
float last_gain;
float last_inc;
unsigned int count;
} AmPitchshift;

static void cleanupAmPitchshift(LV2_Handle instance)
{
AmPitchshift *plugin_data = (AmPitchshift *)instance;

      free(plugin_data->delay);
    
  free(instance);
}

static void connectPortAmPitchshift(LV2_Handle instance, uint32_t port, void *data)
{
  AmPitchshift *plugin = (AmPitchshift *)instance;

  switch (port) {
  case 0:
    plugin->pitch = data;
    break;
  case 1:
    plugin->size = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  case 4:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiateAmPitchshift(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  AmPitchshift *plugin_data = (AmPitchshift *)malloc(sizeof(AmPitchshift));
  float * delay = plugin_data->delay;
  fixp16 rptr = plugin_data->rptr;
  unsigned int wptr = plugin_data->wptr;
  int last_size = plugin_data->last_size;
  unsigned int delay_mask = plugin_data->delay_mask;
  unsigned int delay_ofs = plugin_data->delay_ofs;
  float last_gain = plugin_data->last_gain;
  float last_inc = plugin_data->last_inc;
  unsigned int count = plugin_data->count;
  
      delay = calloc(DELAY_SIZE, sizeof(float));
      rptr.all = 0;
      wptr = 0;
      last_size = -1;
      delay_mask = 0xFF;
      delay_ofs = 0x80;
      last_gain = 0.5f;
      count = 0;
      last_inc = 0.0f;
    
  plugin_data->delay = delay;
  plugin_data->rptr = rptr;
  plugin_data->wptr = wptr;
  plugin_data->last_size = last_size;
  plugin_data->delay_mask = delay_mask;
  plugin_data->delay_ofs = delay_ofs;
  plugin_data->last_gain = last_gain;
  plugin_data->last_inc = last_inc;
  plugin_data->count = count;
  
  return (LV2_Handle)plugin_data;
}



static void runAmPitchshift(LV2_Handle instance, uint32_t sample_count)
{
  AmPitchshift *plugin_data = (AmPitchshift *)instance;

  const float pitch = *(plugin_data->pitch);
  const float size = *(plugin_data->size);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float latency;
  float * delay = plugin_data->delay;
  fixp16 rptr = plugin_data->rptr;
  unsigned int wptr = plugin_data->wptr;
  int last_size = plugin_data->last_size;
  unsigned int delay_mask = plugin_data->delay_mask;
  unsigned int delay_ofs = plugin_data->delay_ofs;
  float last_gain = plugin_data->last_gain;
  float last_inc = plugin_data->last_inc;
  unsigned int count = plugin_data->count;
  
      unsigned long pos;
      fixp16 om;
      float gain = last_gain, gain_inc = last_inc;
      unsigned int i;

      om.all = f_round(pitch * 65536.0f);

      if (size != last_size) {
	int size_tmp = f_round(size);

	if (size_tmp > 7) {
	  size_tmp = 5;
	} else if (size_tmp < 1) {
	  size_tmp = 1;
	}
	plugin_data->last_size = size;

	/* Calculate the ringbuf parameters, the magick constants will need
	 * to be changed if you change DELAY_SIZE */
	delay_mask = (1 << (size_tmp + 6)) - 1;
	delay_ofs = 1 << (size_tmp + 5);
      }

      for (pos = 0; pos < sample_count; pos++) {
	float out = 0.0f;

	if (count++ > 14) {
	  float tmp;
	  count = 0;
	  tmp = 0.5f * (float)((rptr.part.in - wptr + delay_ofs/2) &
                delay_mask) / (float)delay_ofs;
	  tmp = sinf(M_PI * 2.0f * tmp) * 0.5f + 0.5f;
	  gain_inc = (tmp - gain) / 15.0f;
	}
	gain += gain_inc;

	delay[wptr] = input[pos];

	/* Add contributions from the two readpointers, scaled by thier
         * distance from the write pointer */
	i = rptr.part.in;
	out += cube_interp((float)rptr.part.fr * 0.0000152587f,
                           delay[(i - 1) & delay_mask], delay[i],
                           delay[(i + 1) & delay_mask],
                           delay[(i + 2) & delay_mask]) * (1.0f - gain);
	i += delay_ofs;
	out += cube_interp((float)rptr.part.fr * 0.0000152587f,
                           delay[(i - 1) & delay_mask], delay[i & delay_mask],
                           delay[(i + 1) & delay_mask],
                           delay[(i + 2) & delay_mask]) * gain;
	
	buffer_write(output[pos], out);

	/* Increment ringbuffer pointers */
	wptr = (wptr + 1) & delay_mask;
	rptr.all += om.all;
	rptr.part.in &= delay_mask;
      }

    plugin_data->rptr.all = rptr.all;
    plugin_data->wptr = wptr;
    plugin_data->delay_mask = delay_mask;
    plugin_data->delay_ofs = delay_ofs;
    plugin_data->last_gain = gain;
    plugin_data->count = count;
    plugin_data->last_inc = gain_inc;

    *(plugin_data->latency) = delay_ofs/2;
    
}

static const LV2_Descriptor amPitchshiftDescriptor = {
  "http://plugin.org.uk/swh-plugins/amPitchshift",
  instantiateAmPitchshift,
  connectPortAmPitchshift,
  NULL,
  runAmPitchshift,
  NULL,
  cleanupAmPitchshift,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &amPitchshiftDescriptor;
  default:
    return NULL;
  }
}
