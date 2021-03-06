
      #include "ladspa-util.h"

      #define MIN(a,b) ((a) < (b) ? (a) : (b))
      #define CALC_DELAY(delaytime) \
        (f_clamp (delaytime * sample_rate, 1.f, (float)(buffer_mask + 1)))

      #define LOG001 -6.9077552789f

      static inline float
      calc_feedback (float delaytime, float decaytime)
      {
        if (delaytime == 0.f)
          return 0.f;
        else if (decaytime > 0.f)
          return exp(LOG001 * delaytime / decaytime);
	else if (decaytime < 0.f)
          return -exp(LOG001 * delaytime / -decaytime);
        else
          return 0.f;
      }

      void ignore(float some_var)
      { }
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Allpass_n {
  float *in;
  float *out;
  float *max_delay;
  float *delay_time;
  float *decay_time;
float * buffer;
unsigned int buffer_mask;
unsigned int sample_rate;
float delay_samples;
long write_phase;
float feedback;
float last_delay_time;
float last_decay_time;
} Allpass_n;

static void cleanupAllpass_n(LV2_Handle instance)
{
Allpass_n *plugin_data = (Allpass_n *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortAllpass_n(LV2_Handle instance, uint32_t port, void *data)
{
  Allpass_n *plugin = (Allpass_n *)instance;

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
  case 4:
    plugin->decay_time = data;
    break;
  }
}

static LV2_Handle instantiateAllpass_n(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Allpass_n *plugin_data = (Allpass_n *)malloc(sizeof(Allpass_n));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float feedback = plugin_data->feedback;
  float last_delay_time = plugin_data->last_delay_time;
  float last_decay_time = plugin_data->last_decay_time;
  
      sample_rate = s_rate;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->sample_rate = sample_rate;
  plugin_data->delay_samples = delay_samples;
  plugin_data->write_phase = write_phase;
  plugin_data->feedback = feedback;
  plugin_data->last_delay_time = last_delay_time;
  plugin_data->last_decay_time = last_decay_time;
  
  return (LV2_Handle)plugin_data;
}


static void activateAllpass_n(LV2_Handle instance)
{
  Allpass_n *plugin_data = (Allpass_n *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  unsigned int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float delay_samples __attribute__ ((unused)) = plugin_data->delay_samples;
  long write_phase __attribute__ ((unused)) = plugin_data->write_phase;
  float feedback __attribute__ ((unused)) = plugin_data->feedback;
  float last_delay_time __attribute__ ((unused)) = plugin_data->last_delay_time;
  float last_decay_time __attribute__ ((unused)) = plugin_data->last_decay_time;
  
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


static void runAllpass_n(LV2_Handle instance, uint32_t sample_count)
{
  Allpass_n *plugin_data = (Allpass_n *)instance;

  const float * const in = plugin_data->in;
  float * const out = plugin_data->out;
  const float max_delay = *(plugin_data->max_delay);
  const float delay_time = *(plugin_data->delay_time);
  const float decay_time = *(plugin_data->decay_time);
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float feedback = plugin_data->feedback;
  float last_delay_time = plugin_data->last_delay_time;
  float last_decay_time = plugin_data->last_decay_time;
  
      int i;

      ignore(max_delay);

      if (write_phase == 0) {
        plugin_data->last_delay_time = delay_time;
        plugin_data->last_decay_time = decay_time;
        plugin_data->delay_samples = delay_samples = CALC_DELAY (delay_time);
        plugin_data->feedback = feedback = calc_feedback (delay_time, decay_time);
      }
      
      if (delay_time == plugin_data->last_delay_time) {
        long read_phase = write_phase - (long)delay_samples;
        float *readptr = buffer + (read_phase & buffer_mask);
        float *writeptr = buffer + (write_phase & buffer_mask);
        float *lastptr = buffer + buffer_mask + 1;

        if (decay_time == plugin_data->last_decay_time) {
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
              float written = read * feedback + in[i];
              *(writeptr++) = written;
              buffer_write(out[i], read - feedback * written);
            }

            if (readptr == lastptr) readptr = buffer;
            if (writeptr == lastptr) writeptr = buffer;
          }
        } else {
          float next_feedback = calc_feedback (delay_time, decay_time);
          float feedback_slope = (next_feedback - feedback) / sample_count;
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
              float written = read * feedback + in[i];
              *(writeptr++) = written;
              buffer_write(out[i], read - feedback * written);
              feedback += feedback_slope;
            }

            if (readptr == lastptr) readptr = buffer;
            if (writeptr == lastptr) writeptr = buffer;
          }

          plugin_data->last_decay_time = decay_time;
          plugin_data->feedback = feedback;
        }

        write_phase += sample_count;
      } else {
        float next_delay_samples = CALC_DELAY (delay_time);
        float delay_samples_slope = (next_delay_samples - delay_samples) / sample_count;
        float next_feedback = calc_feedback (delay_time, decay_time);
        float feedback_slope = (next_feedback - feedback) / sample_count;

        for (i=0; i<sample_count; i++) {
          long read_phase;
          float read, written;

          delay_samples += delay_samples_slope;
          write_phase++;
          read_phase = write_phase - (long)delay_samples;
          read = buffer[read_phase & buffer_mask];

          written = read * feedback + in[i];
          buffer[write_phase & buffer_mask] = written;
          buffer_write(out[i], read - feedback * written);

          feedback += feedback_slope;
        }

        plugin_data->last_delay_time = delay_time;
        plugin_data->last_decay_time = decay_time;
        plugin_data->feedback = feedback;
        plugin_data->delay_samples = delay_samples;
      }
      
      plugin_data->write_phase = write_phase;
    
}

static const LV2_Descriptor allpass_nDescriptor = {
  "http://plugin.org.uk/swh-plugins/allpass_n",
  instantiateAllpass_n,
  connectPortAllpass_n,
  activateAllpass_n,
  runAllpass_n,
  NULL,
  cleanupAllpass_n,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Allpass_l {
  float *in;
  float *out;
  float *max_delay;
  float *delay_time;
  float *decay_time;
float * buffer;
unsigned int buffer_mask;
unsigned int sample_rate;
float delay_samples;
long write_phase;
float feedback;
float last_delay_time;
float last_decay_time;
} Allpass_l;

static void cleanupAllpass_l(LV2_Handle instance)
{
Allpass_l *plugin_data = (Allpass_l *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortAllpass_l(LV2_Handle instance, uint32_t port, void *data)
{
  Allpass_l *plugin = (Allpass_l *)instance;

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
  case 4:
    plugin->decay_time = data;
    break;
  }
}

static LV2_Handle instantiateAllpass_l(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Allpass_l *plugin_data = (Allpass_l *)malloc(sizeof(Allpass_l));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float feedback = plugin_data->feedback;
  float last_delay_time = plugin_data->last_delay_time;
  float last_decay_time = plugin_data->last_decay_time;
  
      sample_rate = s_rate;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->sample_rate = sample_rate;
  plugin_data->delay_samples = delay_samples;
  plugin_data->write_phase = write_phase;
  plugin_data->feedback = feedback;
  plugin_data->last_delay_time = last_delay_time;
  plugin_data->last_decay_time = last_decay_time;
  
  return (LV2_Handle)plugin_data;
}


static void activateAllpass_l(LV2_Handle instance)
{
  Allpass_l *plugin_data = (Allpass_l *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  unsigned int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float delay_samples __attribute__ ((unused)) = plugin_data->delay_samples;
  long write_phase __attribute__ ((unused)) = plugin_data->write_phase;
  float feedback __attribute__ ((unused)) = plugin_data->feedback;
  float last_delay_time __attribute__ ((unused)) = plugin_data->last_delay_time;
  float last_decay_time __attribute__ ((unused)) = plugin_data->last_decay_time;
  
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


static void runAllpass_l(LV2_Handle instance, uint32_t sample_count)
{
  Allpass_l *plugin_data = (Allpass_l *)instance;

  const float * const in = plugin_data->in;
  float * const out = plugin_data->out;
  const float max_delay = *(plugin_data->max_delay);
  const float delay_time = *(plugin_data->delay_time);
  const float decay_time = *(plugin_data->decay_time);
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float feedback = plugin_data->feedback;
  float last_delay_time = plugin_data->last_delay_time;
  float last_decay_time = plugin_data->last_decay_time;
  
      int i;

      if (write_phase == 0) {
        plugin_data->last_delay_time = delay_time;
        plugin_data->last_decay_time = decay_time;
        plugin_data->delay_samples = delay_samples = CALC_DELAY (delay_time);
        plugin_data->feedback = feedback = calc_feedback (delay_time, decay_time);
      }
      
      if (delay_time == plugin_data->last_delay_time && decay_time == plugin_data->last_decay_time) {
        long idelay_samples = (long)delay_samples;
        float frac = delay_samples - idelay_samples;

        for (i=0; i<sample_count; i++) {
          long read_phase = write_phase - (long)delay_samples;
          float r1 = buffer[read_phase & buffer_mask];
          float r2 = buffer[(read_phase-1) & buffer_mask];
          float read = LIN_INTERP (frac, r1, r2);
          float written = read * feedback + in[i];

          buffer[write_phase++ & buffer_mask] = written;
          buffer_write(out[i], read - feedback * written);
          write_phase++;
        }
      } else {
        float next_delay_samples = CALC_DELAY (delay_time);
        float delay_samples_slope = (next_delay_samples - delay_samples) / sample_count;
        float next_feedback = calc_feedback (delay_time, decay_time);
        float feedback_slope = (next_feedback - feedback) / sample_count;

        for (i=0; i<sample_count; i++) {
          long read_phase, idelay_samples;
          float read, written, frac;

          delay_samples += delay_samples_slope;
          write_phase++;
          read_phase = write_phase - (long)delay_samples;
          idelay_samples = (long)delay_samples;
          frac = delay_samples - idelay_samples;
          read = LIN_INTERP (frac,
                             buffer[read_phase & buffer_mask], 
                             buffer[(read_phase-1) & buffer_mask]);
          written = read * feedback + in[i];
          buffer[write_phase & buffer_mask] = written;
          buffer_write(out[i], read - feedback * written);

          feedback += feedback_slope;
        }

        plugin_data->last_delay_time = delay_time;
        plugin_data->last_decay_time = decay_time;
        plugin_data->feedback = feedback;
        plugin_data->delay_samples = delay_samples;
      }
      
      plugin_data->write_phase = write_phase;
    
}

static const LV2_Descriptor allpass_lDescriptor = {
  "http://plugin.org.uk/swh-plugins/allpass_l",
  instantiateAllpass_l,
  connectPortAllpass_l,
  activateAllpass_l,
  runAllpass_l,
  NULL,
  cleanupAllpass_l,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Allpass_c {
  float *in;
  float *out;
  float *max_delay;
  float *delay_time;
  float *decay_time;
float * buffer;
unsigned int buffer_mask;
unsigned int sample_rate;
float delay_samples;
long write_phase;
float feedback;
float last_delay_time;
float last_decay_time;
} Allpass_c;

static void cleanupAllpass_c(LV2_Handle instance)
{
Allpass_c *plugin_data = (Allpass_c *)instance;

      free(plugin_data->buffer);
    
  free(instance);
}

static void connectPortAllpass_c(LV2_Handle instance, uint32_t port, void *data)
{
  Allpass_c *plugin = (Allpass_c *)instance;

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
  case 4:
    plugin->decay_time = data;
    break;
  }
}

static LV2_Handle instantiateAllpass_c(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Allpass_c *plugin_data = (Allpass_c *)malloc(sizeof(Allpass_c));
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float feedback = plugin_data->feedback;
  float last_delay_time = plugin_data->last_delay_time;
  float last_decay_time = plugin_data->last_decay_time;
  
      sample_rate = s_rate;
    
  plugin_data->buffer = buffer;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->sample_rate = sample_rate;
  plugin_data->delay_samples = delay_samples;
  plugin_data->write_phase = write_phase;
  plugin_data->feedback = feedback;
  plugin_data->last_delay_time = last_delay_time;
  plugin_data->last_decay_time = last_decay_time;
  
  return (LV2_Handle)plugin_data;
}


static void activateAllpass_c(LV2_Handle instance)
{
  Allpass_c *plugin_data = (Allpass_c *)instance;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  unsigned int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float delay_samples __attribute__ ((unused)) = plugin_data->delay_samples;
  long write_phase __attribute__ ((unused)) = plugin_data->write_phase;
  float feedback __attribute__ ((unused)) = plugin_data->feedback;
  float last_delay_time __attribute__ ((unused)) = plugin_data->last_delay_time;
  float last_decay_time __attribute__ ((unused)) = plugin_data->last_decay_time;
  
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


static void runAllpass_c(LV2_Handle instance, uint32_t sample_count)
{
  Allpass_c *plugin_data = (Allpass_c *)instance;

  const float * const in = plugin_data->in;
  float * const out = plugin_data->out;
  const float max_delay = *(plugin_data->max_delay);
  const float delay_time = *(plugin_data->delay_time);
  const float decay_time = *(plugin_data->decay_time);
  float * buffer = plugin_data->buffer;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int sample_rate = plugin_data->sample_rate;
  float delay_samples = plugin_data->delay_samples;
  long write_phase = plugin_data->write_phase;
  float feedback = plugin_data->feedback;
  float last_delay_time = plugin_data->last_delay_time;
  float last_decay_time = plugin_data->last_decay_time;
  
      int i;

      if (write_phase == 0) {
        plugin_data->last_delay_time = delay_time;
        plugin_data->last_decay_time = decay_time;
        plugin_data->delay_samples = delay_samples = CALC_DELAY (delay_time);
        plugin_data->feedback = feedback = calc_feedback (delay_time, decay_time);
      }
      
      if (delay_time == plugin_data->last_delay_time && decay_time == plugin_data->last_decay_time) {
        long idelay_samples = (long)delay_samples;
        float frac = delay_samples - idelay_samples;

        for (i=0; i<sample_count; i++) {
          long read_phase = write_phase - (long)delay_samples;
          float read = cube_interp (frac,
                                          buffer[(read_phase-1) & buffer_mask], 
                                          buffer[read_phase & buffer_mask], 
                                          buffer[(read_phase+1) & buffer_mask], 
                                          buffer[(read_phase+2) & buffer_mask]);
          float written = read * feedback + in[i];

          buffer[write_phase++ & buffer_mask] = written;
          buffer_write(out[i], read - feedback * written);
        }
      } else {
        float next_delay_samples = CALC_DELAY (delay_time);
        float delay_samples_slope = (next_delay_samples - delay_samples) / sample_count;
        float next_feedback = calc_feedback (delay_time, decay_time);
        float feedback_slope = (next_feedback - feedback) / sample_count;

        for (i=0; i<sample_count; i++) {
          long read_phase, idelay_samples;
          float read, written, frac;

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
          written = read * feedback + in[i];
          buffer[write_phase & buffer_mask] = written;
          buffer_write(out[i], read - feedback * written);

          feedback += feedback_slope;
        }

        plugin_data->last_delay_time = delay_time;
        plugin_data->last_decay_time = decay_time;
        plugin_data->feedback = feedback;
        plugin_data->delay_samples = delay_samples;
      }
      
      plugin_data->write_phase = write_phase;
    
}

static const LV2_Descriptor allpass_cDescriptor = {
  "http://plugin.org.uk/swh-plugins/allpass_c",
  instantiateAllpass_c,
  connectPortAllpass_c,
  activateAllpass_c,
  runAllpass_c,
  NULL,
  cleanupAllpass_c,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &allpass_nDescriptor;
  case 1:
    return &allpass_lDescriptor;
  case 2:
    return &allpass_cDescriptor;
  default:
    return NULL;
  }
}
