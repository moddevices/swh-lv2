
      #include "ladspa-util.h"

      #define MIN(a,b) ((a) < (b) ? (a) : (b))
      #define CALC_DELAY(delaytime) \
        (f_clamp (delaytime * sample_rate, 1.f, (float)(buffer_mask + 1)))

    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Delay_n {
  float *in;
  float *out;
  float *max_delay;
  float *delay_time;
float * buffer;
unsigned int buffer_mask;
unsigned int sample_rate;
float delay_samples;
long write_phase;
float last_delay_time;
} Delay_n;

static void cleanupDelay_n(LV2_Handle instance)
{
Delay_n *plugin_data = (Delay_n *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortDelay_n(LV2_Handle instance, uint32_t port, void *data)
{
  Delay_n *plugin = (Delay_n *)instance;

  switch (port) {
  case 0:
    plugin->in = data;
    break;
  case 1:
    plugin->out = data;
    break;
  case 2:
    plugin->max_delay = data;
    break;
  case 3:
    plugin->delay_time = data;
    break;
  }
}

static LV2_Handle instantiateDelay_n(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Delay_n *plugin_data = (Delay_n *)malloc(sizeof(Delay_n));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float last_delay_time = plugin_data->last_delay_time;
  
      sample_rate = s_rate;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->sample_rate = sample_rate;
  plugin_data->delay_samples = delay_samples;
  plugin_data->write_phase = write_phase;
  plugin_data->last_delay_time = last_delay_time;
  
  return (LV2_Handle)plugin_data;
}


static void activateDelay_n(LV2_Handle instance)
{
  Delay_n *plugin_data = (Delay_n *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  unsigned int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float delay_samples __attribute__ ((unused)) = plugin_data->delay_samples;
  long write_phase __attribute__ ((unused)) = plugin_data->write_phase;
  float last_delay_time __attribute__ ((unused)) = plugin_data->last_delay_time;
  
      unsigned int minsize, size;
    
      if (plugin_data->max_delay && *plugin_data->max_delay > 0)
        minsize = sample_rate * *plugin_data->max_delay;
      else if (plugin_data->delay_time)
        minsize = sample_rate * *plugin_data->delay_time;
      else
        minsize = sample_rate; /* 1 second default */
    
      size = 1;
      while (size < minsize) size <<= 1;
    
      /* calloc sets the buffer to zero. */
      plugin_data->buffer = calloc(size, sizeof(float));
      if (plugin_data->buffer)
        plugin_data->buffer_mask = size - 1;
      else
        plugin_data->buffer_mask = 0;
      plugin_data->write_phase = 0;
    
}


static void runDelay_n(LV2_Handle instance, uint32_t sample_count)
{
  Delay_n *plugin_data = (Delay_n *)instance;

  const float * const in = plugin_data->in;
  float * const out = plugin_data->out;
  const float max_delay = *(plugin_data->max_delay);
  const float delay_time = *(plugin_data->delay_time);
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float last_delay_time = plugin_data->last_delay_time;
  
      int i;

      if (write_phase == 0) {
        plugin_data->last_delay_time = delay_time;
        plugin_data->delay_samples = delay_samples = CALC_DELAY (delay_time);
      }
      
      if (delay_time == plugin_data->last_delay_time) {
        long read_phase = write_phase - (long)delay_samples;
        float *readptr = buffer + (read_phase & buffer_mask);
        float *writeptr = buffer + (write_phase & buffer_mask);
        float *lastptr = buffer + buffer_mask + 1;

        long remain = sample_count;

        while (remain) {
          long read_space = lastptr - readptr;
          long write_space = lastptr - writeptr;
          long to_process = MIN (MIN (read_space, remain), write_space);

          if (to_process == 0)
            return; // buffer not allocated.

          remain -= to_process;

          for (i=0; i<to_process; i++) {
            float read = *(readptr++);
            *(writeptr++) = in[i];
	    buffer_write(out[i], read);
          }

          if (readptr == lastptr) readptr = buffer;
          if (writeptr == lastptr) writeptr = buffer;
        }

        write_phase += sample_count;
      } else {
        float next_delay_samples = CALC_DELAY (delay_time);
        float delay_samples_slope = (next_delay_samples - delay_samples) / sample_count;

        for (i=0; i<sample_count; i++) {
          long read_phase;
          float read;

          delay_samples += delay_samples_slope;
          write_phase++;
          read_phase = write_phase - (long)delay_samples;

          read = buffer[read_phase & buffer_mask];
          buffer[write_phase & buffer_mask] = in[i];
	  buffer_write(out[i], read);
        }

        plugin_data->last_delay_time = delay_time;
        plugin_data->delay_samples = delay_samples;
      }
      
      plugin_data->write_phase = write_phase;
    
}

static const LV2_Descriptor delay_nDescriptor = {
  "http://plugin.org.uk/swh-plugins/delay_n",
  instantiateDelay_n,
  connectPortDelay_n,
  activateDelay_n,
  runDelay_n,
  NULL,
  cleanupDelay_n,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Delay_l {
  float *in;
  float *out;
  float *max_delay;
  float *delay_time;
float * buffer;
unsigned int buffer_mask;
unsigned int sample_rate;
float delay_samples;
long write_phase;
float last_delay_time;
} Delay_l;

static void cleanupDelay_l(LV2_Handle instance)
{
Delay_l *plugin_data = (Delay_l *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortDelay_l(LV2_Handle instance, uint32_t port, void *data)
{
  Delay_l *plugin = (Delay_l *)instance;

  switch (port) {
  case 0:
    plugin->in = data;
    break;
  case 1:
    plugin->out = data;
    break;
  case 2:
    plugin->max_delay = data;
    break;
  case 3:
    plugin->delay_time = data;
    break;
  }
}

static LV2_Handle instantiateDelay_l(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Delay_l *plugin_data = (Delay_l *)malloc(sizeof(Delay_l));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float last_delay_time = plugin_data->last_delay_time;
  
      sample_rate = s_rate;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->sample_rate = sample_rate;
  plugin_data->delay_samples = delay_samples;
  plugin_data->write_phase = write_phase;
  plugin_data->last_delay_time = last_delay_time;
  
  return (LV2_Handle)plugin_data;
}


static void activateDelay_l(LV2_Handle instance)
{
  Delay_l *plugin_data = (Delay_l *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  unsigned int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float delay_samples __attribute__ ((unused)) = plugin_data->delay_samples;
  long write_phase __attribute__ ((unused)) = plugin_data->write_phase;
  float last_delay_time __attribute__ ((unused)) = plugin_data->last_delay_time;
  
      unsigned int minsize, size;
    
      if (plugin_data->max_delay && *plugin_data->max_delay > 0)
        minsize = sample_rate * *plugin_data->max_delay;
      else if (plugin_data->delay_time)
        minsize = sample_rate * *plugin_data->delay_time;
      else
        minsize = sample_rate; /* 1 second default */
    
      size = 1;
      while (size < minsize) size <<= 1;
    
      /* calloc sets the buffer to zero. */
      plugin_data->buffer = calloc(size, sizeof(float));
      if (plugin_data->buffer)
        plugin_data->buffer_mask = size - 1;
      else
        plugin_data->buffer_mask = 0;
      plugin_data->write_phase = 0;
    
}


static void runDelay_l(LV2_Handle instance, uint32_t sample_count)
{
  Delay_l *plugin_data = (Delay_l *)instance;

  const float * const in = plugin_data->in;
  float * const out = plugin_data->out;
  const float max_delay = *(plugin_data->max_delay);
  const float delay_time = *(plugin_data->delay_time);
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float last_delay_time = plugin_data->last_delay_time;
  
      int i;

      if (write_phase == 0) {
        plugin_data->last_delay_time = delay_time;
        plugin_data->delay_samples = delay_samples = CALC_DELAY (delay_time);
      }
      
      if (delay_time == plugin_data->last_delay_time) {
        long idelay_samples = (long)delay_samples;
        float frac = delay_samples - idelay_samples;

        for (i=0; i<sample_count; i++) {
          long read_phase = write_phase - (long)delay_samples;
          float read;
          read = LIN_INTERP (frac,
                                 buffer[(read_phase-1) & buffer_mask],
                                 buffer[read_phase & buffer_mask]);
          buffer[write_phase & buffer_mask] = in[i];
	  buffer_write(out[i], read);
          write_phase++;
        }
      } else {
        float next_delay_samples = CALC_DELAY (delay_time);
        float delay_samples_slope = (next_delay_samples - delay_samples) / sample_count;

        for (i=0; i<sample_count; i++) {
          long read_phase, idelay_samples;
          float frac, read;

          delay_samples += delay_samples_slope;
          write_phase++;
          read_phase = write_phase - (long)delay_samples;
          idelay_samples = (long)delay_samples;
          frac = delay_samples - idelay_samples;
          read = LIN_INTERP (frac,
                             buffer[(read_phase-1) & buffer_mask],
                             buffer[read_phase & buffer_mask]); 
          buffer[write_phase & buffer_mask] = in[i];
	  buffer_write(out[i], read);
        }

        plugin_data->last_delay_time = delay_time;
        plugin_data->delay_samples = delay_samples;
      }
      
      plugin_data->write_phase = write_phase;
    
}

static const LV2_Descriptor delay_lDescriptor = {
  "http://plugin.org.uk/swh-plugins/delay_l",
  instantiateDelay_l,
  connectPortDelay_l,
  activateDelay_l,
  runDelay_l,
  NULL,
  cleanupDelay_l,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Delay_c {
  float *in;
  float *out;
  float *max_delay;
  float *delay_time;
float * buffer;
unsigned int buffer_mask;
unsigned int sample_rate;
float delay_samples;
long write_phase;
float last_delay_time;
} Delay_c;

static void cleanupDelay_c(LV2_Handle instance)
{
Delay_c *plugin_data = (Delay_c *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortDelay_c(LV2_Handle instance, uint32_t port, void *data)
{
  Delay_c *plugin = (Delay_c *)instance;

  switch (port) {
  case 0:
    plugin->in = data;
    break;
  case 1:
    plugin->out = data;
    break;
  case 2:
    plugin->max_delay = data;
    break;
  case 3:
    plugin->delay_time = data;
    break;
  }
}

static LV2_Handle instantiateDelay_c(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Delay_c *plugin_data = (Delay_c *)malloc(sizeof(Delay_c));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float last_delay_time = plugin_data->last_delay_time;
  
      sample_rate = s_rate;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->sample_rate = sample_rate;
  plugin_data->delay_samples = delay_samples;
  plugin_data->write_phase = write_phase;
  plugin_data->last_delay_time = last_delay_time;
  
  return (LV2_Handle)plugin_data;
}


static void activateDelay_c(LV2_Handle instance)
{
  Delay_c *plugin_data = (Delay_c *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  unsigned int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float delay_samples __attribute__ ((unused)) = plugin_data->delay_samples;
  long write_phase __attribute__ ((unused)) = plugin_data->write_phase;
  float last_delay_time __attribute__ ((unused)) = plugin_data->last_delay_time;
  
      unsigned int minsize, size;
    
      if (plugin_data->max_delay && *plugin_data->max_delay > 0)
        minsize = sample_rate * *plugin_data->max_delay;
      else if (plugin_data->delay_time)
        minsize = sample_rate * *plugin_data->delay_time;
      else
        minsize = sample_rate; /* 1 second default */
    
      size = 1;
      while (size < minsize) size <<= 1;
    
      /* calloc sets the buffer to zero. */
      plugin_data->buffer = calloc(size, sizeof(float));
      if (plugin_data->buffer)
        plugin_data->buffer_mask = size - 1;
      else
        plugin_data->buffer_mask = 0;
      plugin_data->write_phase = 0;
    
}


static void runDelay_c(LV2_Handle instance, uint32_t sample_count)
{
  Delay_c *plugin_data = (Delay_c *)instance;

  const float * const in = plugin_data->in;
  float * const out = plugin_data->out;
  const float max_delay = *(plugin_data->max_delay);
  const float delay_time = *(plugin_data->delay_time);
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float last_delay_time = plugin_data->last_delay_time;
  
      int i;

      if (write_phase == 0) {
        plugin_data->last_delay_time = delay_time;
        plugin_data->delay_samples = delay_samples = CALC_DELAY (delay_time);
      }
      
      if (delay_time == plugin_data->last_delay_time) {
        long idelay_samples = (long)delay_samples;
        float frac = delay_samples - idelay_samples;

        for (i=0; i<sample_count; i++) {
          long read_phase = write_phase - (long)delay_samples;
          float read = cube_interp (frac,
                                          buffer[(read_phase-1) & buffer_mask], 
                                          buffer[read_phase & buffer_mask], 
                                          buffer[(read_phase+1) & buffer_mask], 
                                          buffer[(read_phase+2) & buffer_mask]);
          buffer[write_phase++ & buffer_mask] = in[i];
	  buffer_write(out[i], read);
        }
      } else {
        float next_delay_samples = CALC_DELAY (delay_time);
        float delay_samples_slope = (next_delay_samples - delay_samples) / sample_count;

        for (i=0; i<sample_count; i++) {
          long read_phase, idelay_samples;
          float written, frac, read;

          delay_samples += delay_samples_slope;
          write_phase++;
          read_phase = write_phase - (long)delay_samples;
          idelay_samples = (long)delay_samples;
          frac = delay_samples - idelay_samples;
          read = cube_interp (frac,
                              buffer[(read_phase-1) & buffer_mask], 
                              buffer[read_phase & buffer_mask], 
                              buffer[(read_phase+1) & buffer_mask], 
                              buffer[(read_phase+2) & buffer_mask]);
          buffer[write_phase & buffer_mask] = in[i];
	  buffer_write(out[i], read);
        }

        plugin_data->last_delay_time = delay_time;
        plugin_data->delay_samples = delay_samples;
      }
      
      plugin_data->write_phase = write_phase;
    
}

static const LV2_Descriptor delay_cDescriptor = {
  "http://plugin.org.uk/swh-plugins/delay_c",
  instantiateDelay_c,
  connectPortDelay_c,
  activateDelay_c,
  runDelay_c,
  NULL,
  cleanupDelay_c,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &delay_nDescriptor;
  case 1:
    return &delay_lDescriptor;
  case 2:
    return &delay_cDescriptor;
  default:
    return NULL;
  }
}
