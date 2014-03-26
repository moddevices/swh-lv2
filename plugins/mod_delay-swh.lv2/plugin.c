
      #include "ladspa-util.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _ModDelay {
  float *base;
  float *delay;
  float *input;
  float *output;
float fs;
float * buffer;
unsigned int buffer_mask;
unsigned int write_ptr;
} ModDelay;

static void cleanupModDelay(LV2_Handle instance)
{
ModDelay *plugin_data = (ModDelay *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortModDelay(LV2_Handle instance, uint32_t port, void *data)
{
  ModDelay *plugin = (ModDelay *)instance;

  switch (port) {
  case 0:
    plugin->base = data;
    break;
  case 1:
    plugin->delay = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateModDelay(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  ModDelay *plugin_data = (ModDelay *)malloc(sizeof(ModDelay));
  float fs = plugin_data->fs;
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int write_ptr = plugin_data->write_ptr;
  
      unsigned int size = 32768;

      fs = s_rate;
      while (size < 2.7f * fs) {
	size *= 2;
      }
      buffer = calloc(size, sizeof(float));
      buffer_mask = size - 1;
      write_ptr = 0;
    
  plugin_data->fs = fs;
  plugin_data->buffer = buffer;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->write_ptr = write_ptr;
  
  return (LV2_Handle)plugin_data;
}


static void activateModDelay(LV2_Handle instance)
{
  ModDelay *plugin_data = (ModDelay *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  unsigned int write_ptr __attribute__ ((unused)) = plugin_data->write_ptr;
  
      memset(buffer, 0, buffer_mask + 1);
      write_ptr = 0;
    
}


static void runModDelay(LV2_Handle instance, uint32_t sample_count)
{
  ModDelay *plugin_data = (ModDelay *)instance;

  const float base = *(plugin_data->base);
  const float * const delay = plugin_data->delay;
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float fs = plugin_data->fs;
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int write_ptr = plugin_data->write_ptr;
  
      unsigned long pos;

      for (pos = 0; pos < sample_count; pos++) {
	float tmp;
	const float rpf = modff((base + delay[pos]) * fs, &tmp);
	const int rp = write_ptr - 4 - f_round(tmp);

        buffer[write_ptr++] = input[pos];
	write_ptr &= buffer_mask;

        buffer_write(output[pos], cube_interp(rpf, buffer[(rp - 1) & buffer_mask], buffer[rp & buffer_mask],  buffer[(rp + 1) & buffer_mask], buffer[(rp + 2) & buffer_mask]));
      }
      plugin_data->write_ptr = write_ptr;
    
}

static const LV2_Descriptor modDelayDescriptor = {
  "http://plugin.org.uk/swh-plugins/modDelay",
  instantiateModDelay,
  connectPortModDelay,
  activateModDelay,
  runModDelay,
  NULL,
  cleanupModDelay,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &modDelayDescriptor;
  default:
    return NULL;
  }
}
