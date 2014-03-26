
      #include "ladspa-util.h"
      #include <stdio.h>

      #define MIN(a,b) ((a) < (b) ? (a) : (b))
      #define CALC_DELAY(delaytime) \
        (f_clamp (delaytime * sample_rate, 1.f, (float)(buffer_size + 1)))

    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Revdelay {
  float *in;
  float *out;
  float *delay_time;
  float *dry_level;
  float *wet_level;
  float *feedback;
  float *xfade_samp;
float * buffer;
unsigned int buffer_size;
unsigned int sample_rate;
float delay_samples;
long write_phase;
float last_delay_time;
} Revdelay;

static void cleanupRevdelay(LV2_Handle instance)
{
Revdelay *plugin_data = (Revdelay *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortRevdelay(LV2_Handle instance, uint32_t port, void *data)
{
  Revdelay *plugin = (Revdelay *)instance;

  switch (port) {
  case 0:
    plugin->in = data;
    break;
  case 1:
    plugin->out = data;
    break;
  case 2:
    plugin->delay_time = data;
    break;
  case 3:
    plugin->dry_level = data;
    break;
  case 4:
    plugin->wet_level = data;
    break;
  case 5:
    plugin->feedback = data;
    break;
  case 6:
    plugin->xfade_samp = data;
    break;
  }
}

static LV2_Handle instantiateRevdelay(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Revdelay *plugin_data = (Revdelay *)malloc(sizeof(Revdelay));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_size = plugin_data->buffer_size;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float last_delay_time = plugin_data->last_delay_time;
  
      sample_rate = s_rate;
      buffer_size = 0;
      delay_samples = 0;
      last_delay_time = 0;
      write_phase = 0;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_size = buffer_size;
  plugin_data->sample_rate = sample_rate;
  plugin_data->delay_samples = delay_samples;
  plugin_data->write_phase = write_phase;
  plugin_data->last_delay_time = last_delay_time;
  
  return (LV2_Handle)plugin_data;
}


static void activateRevdelay(LV2_Handle instance)
{
  Revdelay *plugin_data = (Revdelay *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_size __attribute__ ((unused)) = plugin_data->buffer_size;
  unsigned int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float delay_samples __attribute__ ((unused)) = plugin_data->delay_samples;
  long write_phase __attribute__ ((unused)) = plugin_data->write_phase;
  float last_delay_time __attribute__ ((unused)) = plugin_data->last_delay_time;
  
      unsigned int size;

      size = sample_rate * 5 * 2; /* 5 second maximum */
        
      /* calloc sets the buffer to zero. */
      plugin_data->buffer = calloc(size, sizeof(float));

      plugin_data->buffer_size = size;
      plugin_data->write_phase = 0;
      plugin_data->delay_samples = 0;
    
}


static void runRevdelay(LV2_Handle instance, uint32_t sample_count)
{
  Revdelay *plugin_data = (Revdelay *)instance;

  const float * const in = plugin_data->in;
  float * const out = plugin_data->out;
  const float delay_time = *(plugin_data->delay_time);
  const float dry_level = *(plugin_data->dry_level);
  const float wet_level = *(plugin_data->wet_level);
  const float feedback = *(plugin_data->feedback);
  const float xfade_samp = *(plugin_data->xfade_samp);
  float * buffer = plugin_data->buffer;
  unsigned int buffer_size = plugin_data->buffer_size;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float last_delay_time = plugin_data->last_delay_time;
  
      int i;
      unsigned long delay2;
      float dry = DB_CO(dry_level);
      float wet = DB_CO(wet_level);
      float fadescale;
      unsigned long xfadesamp = xfade_samp;

      if (write_phase == 0) {
        plugin_data->last_delay_time = delay_time;
        plugin_data->delay_samples = delay_samples = CALC_DELAY (delay_time);
      }

      if (delay_time == last_delay_time) {
        long idelay_samples = (long)delay_samples;
        delay2 = idelay_samples * 2;

        if (xfadesamp > idelay_samples) {
            /* force it to half */
            xfadesamp = idelay_samples / 2;
        }

        for (i=0; i<sample_count; i++) {
          long read_phase = delay2 - write_phase;
          float read;
          float insamp;

          insamp = in[i];
          read =  (wet * buffer[read_phase]) + (dry * insamp);

          if ( (write_phase % idelay_samples) < xfadesamp) {
            fadescale = (write_phase % idelay_samples) / (1.0 * xfadesamp);
          }
          else if ((write_phase % idelay_samples) > (idelay_samples - xfadesamp)) {
            fadescale = (idelay_samples - (write_phase % idelay_samples)) / (1.0 * xfadesamp);
          }
          else {
            fadescale = 1.0;
          }

          buffer[write_phase] = fadescale * (insamp + (feedback * read)); 
	  buffer[write_phase] = flush_to_zero(buffer[write_phase]);
                  
	  buffer_write(out[i], read);
          write_phase = (write_phase + 1) % delay2;
        }
      } else {
        float next_delay_samples = CALC_DELAY (delay_time);
        float delay_samples_slope = (next_delay_samples - delay_samples) / sample_count;

        for (i=0; i<sample_count; i++) {
          long read_phase, idelay_samples;
          float frac, read;
          float insamp;
          insamp = in[i];

          delay_samples += delay_samples_slope;
          delay2 = (long) (delay_samples * 2);
          write_phase = (write_phase + 1) % delay2;

          read_phase = delay2 - write_phase;
          idelay_samples = (long)delay_samples;
          frac = delay_samples - idelay_samples;
          read = wet * buffer[read_phase]   + (dry * insamp);

          if ((write_phase % idelay_samples) < xfade_samp) {
            fadescale = (write_phase % idelay_samples) / (1.0 * xfade_samp);
          }
          else if ((write_phase % idelay_samples) > (idelay_samples - xfade_samp)) {
            fadescale = (idelay_samples - (write_phase % idelay_samples)) / (1.0 * xfade_samp);
          }
          else {
            fadescale = 1.0;
          }

          buffer[write_phase] = fadescale * (insamp + (feedback * read)); 
	  buffer[write_phase] = flush_to_zero(buffer[write_phase]);

	  buffer_write(out[i], read);
        }

        plugin_data->last_delay_time = delay_time;
        plugin_data->delay_samples = delay_samples;
      }
      
      plugin_data->write_phase = write_phase;
    
}

static const LV2_Descriptor revdelayDescriptor = {
  "http://plugin.org.uk/swh-plugins/revdelay",
  instantiateRevdelay,
  connectPortRevdelay,
  activateRevdelay,
  runRevdelay,
  NULL,
  cleanupRevdelay,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &revdelayDescriptor;
  default:
    return NULL;
  }
}
