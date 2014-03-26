
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _RateShifter {
  float *rate;
  float *input;
  float *output;
float * buffer;
unsigned int buffer_mask;
fixp32 read_ptr;
unsigned int write_ptr;
} RateShifter;

static void cleanupRateShifter(LV2_Handle instance)
{
RateShifter *plugin_data = (RateShifter *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortRateShifter(LV2_Handle instance, uint32_t port, void *data)
{
  RateShifter *plugin = (RateShifter *)instance;

  switch (port) {
  case 0:
    plugin->rate = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateRateShifter(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  RateShifter *plugin_data = (RateShifter *)malloc(sizeof(RateShifter));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  fixp32 read_ptr = plugin_data->read_ptr;
  unsigned int write_ptr = plugin_data->write_ptr;
  
      unsigned int size = 32768;
      const float fs = s_rate;

      while (size < 2.7f * fs) {
	size *= 2;
      }
      buffer = calloc(size, sizeof(float));
      buffer_mask = size - 1;
      read_ptr.all = 0;
      write_ptr = size / 2;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->read_ptr = read_ptr;
  plugin_data->write_ptr = write_ptr;
  
  return (LV2_Handle)plugin_data;
}


static void activateRateShifter(LV2_Handle instance)
{
  RateShifter *plugin_data = (RateShifter *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  fixp32 read_ptr __attribute__ ((unused)) = plugin_data->read_ptr;
  unsigned int write_ptr __attribute__ ((unused)) = plugin_data->write_ptr;
  
      memset(buffer, 0, buffer_mask + 1);
      read_ptr.all = 0;
      write_ptr = (buffer_mask + 1) / 2;
      write_ptr = 0;
    
}


static void runRateShifter(LV2_Handle instance, uint32_t sample_count)
{
  RateShifter *plugin_data = (RateShifter *)instance;

  const float rate = *(plugin_data->rate);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  fixp32 read_ptr = plugin_data->read_ptr;
  unsigned int write_ptr = plugin_data->write_ptr;
  
      unsigned long pos;
      fixp32 read_inc;

      read_inc.all = (long long)(rate * 4294967296.0f);

      for (pos = 0; pos < sample_count; pos++) {
	const unsigned int rp = read_ptr.part.in;

	/* Do write pointer stuff */
        buffer[write_ptr] = input[pos];
	write_ptr = (write_ptr + 1) & buffer_mask;

	/* And now read pointer */
        buffer_write(output[pos], cube_interp((float)read_ptr.part.fr / 4294967296.0f, buffer[(rp - 1) & buffer_mask], buffer[rp],  buffer[(rp + 1) & buffer_mask], buffer[(rp + 2) & buffer_mask]));
	read_ptr.all += read_inc.all;
	read_ptr.part.in &= buffer_mask;
      }

      plugin_data->read_ptr.all = read_ptr.all;
      plugin_data->write_ptr = write_ptr;
    
}

static const LV2_Descriptor rateShifterDescriptor = {
  "http://plugin.org.uk/swh-plugins/rateShifter",
  instantiateRateShifter,
  connectPortRateShifter,
  activateRateShifter,
  runRateShifter,
  NULL,
  cleanupRateShifter,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &rateShifterDescriptor;
  default:
    return NULL;
  }
}
