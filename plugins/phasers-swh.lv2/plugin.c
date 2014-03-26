
      #include "ladspa-util.h"

      #define LFO_SIZE 4096

      typedef struct {
        float a1;
	float zm1;
      } allpass;

      inline static float ap_run(allpass *a, float x)
      {
	float y = x * -(a->a1) + a->zm1;
        a->zm1 = y * a->a1 + x;

        return y;
      }

      inline static void ap_set_delay(allpass *a, float d)
      {
        a->a1 = (1.0f - d) / (1.0f + d);
      }

      inline static void ap_clear(allpass *a)
      {
        a->a1  = 0.0f;
        a->zm1 = 0.0f;
      }

      typedef struct {
        float ga;
        float gr;
        float env;
      } envelope;

      inline static float env_run(envelope *e, float in)
      {
        float env_lvl = e->env;

        in = fabs(in);

        if (env_lvl < in) {
          env_lvl = e->ga * (env_lvl - in) + in;
        } else {
          env_lvl = e->gr * (env_lvl - in) + in;
        }

	e->env = env_lvl;
	return env_lvl;
      }

      // Set attack time in samples
      inline static void env_set_attack(envelope *e, float a)
      {
        e->ga = f_exp(-1.0f/a);
      }

      // Set release time in samples
      inline static void env_set_release(envelope *e, float r)
      {
        e->gr = f_exp(-1.0f/r);
      }

    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _LfoPhaser {
  float *lfo_rate;
  float *lfo_depth;
  float *fb;
  float *spread;
  float *input;
  float *output;
allpass * ap;
int count;
float * lfo_tbl;
int lfo_pos;
float f_per_lv;
float ym1;
} LfoPhaser;

static void cleanupLfoPhaser(LV2_Handle instance)
{
LfoPhaser *plugin_data = (LfoPhaser *)instance;

      free(plugin_data->ap);
      free(plugin_data->lfo_tbl);
    
  free(instance);
}

static void connectPortLfoPhaser(LV2_Handle instance, uint32_t port, void *data)
{
  LfoPhaser *plugin = (LfoPhaser *)instance;

  switch (port) {
  case 0:
    plugin->lfo_rate = data;
    break;
  case 1:
    plugin->lfo_depth = data;
    break;
  case 2:
    plugin->fb = data;
    break;
  case 3:
    plugin->spread = data;
    break;
  case 4:
    plugin->input = data;
    break;
  case 5:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateLfoPhaser(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  LfoPhaser *plugin_data = (LfoPhaser *)malloc(sizeof(LfoPhaser));
  allpass * ap = plugin_data->ap;
  int count = plugin_data->count;
  float * lfo_tbl = plugin_data->lfo_tbl;
  int lfo_pos = plugin_data->lfo_pos;
  float f_per_lv = plugin_data->f_per_lv;
  float ym1 = plugin_data->ym1;
  
      unsigned int i;
      float p;

      ap = calloc(6, sizeof(allpass));
      ym1 = 0.0f;
      lfo_tbl = malloc(sizeof(float) * LFO_SIZE);
      p = 0.0f;
      for (i=0; i<LFO_SIZE; i++) {
        p += M_PI * 0.0004882812f;
        lfo_tbl[i] = (sin(p) + 1.1f) * 0.25f;
      }
      lfo_pos = 0;

      // Frames per lfo value
      f_per_lv = (float)s_rate * 0.0002441406f;

      count = 0;
    
  plugin_data->ap = ap;
  plugin_data->count = count;
  plugin_data->lfo_tbl = lfo_tbl;
  plugin_data->lfo_pos = lfo_pos;
  plugin_data->f_per_lv = f_per_lv;
  plugin_data->ym1 = ym1;
  
  return (LV2_Handle)plugin_data;
}


static void activateLfoPhaser(LV2_Handle instance)
{
  LfoPhaser *plugin_data = (LfoPhaser *)instance;
  allpass * ap __attribute__ ((unused)) = plugin_data->ap;
  int count __attribute__ ((unused)) = plugin_data->count;
  float * lfo_tbl __attribute__ ((unused)) = plugin_data->lfo_tbl;
  int lfo_pos __attribute__ ((unused)) = plugin_data->lfo_pos;
  float f_per_lv __attribute__ ((unused)) = plugin_data->f_per_lv;
  float ym1 __attribute__ ((unused)) = plugin_data->ym1;
  
      ap_clear(ap);
      ap_clear(ap+1);
      ap_clear(ap+2);
      ap_clear(ap+3);
      ap_clear(ap+4);
      ap_clear(ap+5);
    
}


static void runLfoPhaser(LV2_Handle instance, uint32_t sample_count)
{
  LfoPhaser *plugin_data = (LfoPhaser *)instance;

  const float lfo_rate = *(plugin_data->lfo_rate);
  const float lfo_depth = *(plugin_data->lfo_depth);
  const float fb = *(plugin_data->fb);
  const float spread = *(plugin_data->spread);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  allpass * ap = plugin_data->ap;
  int count = plugin_data->count;
  float * lfo_tbl = plugin_data->lfo_tbl;
  int lfo_pos = plugin_data->lfo_pos;
  float f_per_lv = plugin_data->f_per_lv;
  float ym1 = plugin_data->ym1;
  
      unsigned long pos;
      unsigned int mod;
      float y, d, ofs;

      mod = f_round(f_per_lv / lfo_rate);
      if (mod < 1) {
        mod=1;
      }

      d = lfo_tbl[lfo_pos];

      for (pos = 0; pos < sample_count; pos++) {
        // Get new value for LFO if needed
	if (++count % mod == 0) {
	  lfo_pos++;
	  lfo_pos &= 0x7FF;
	  count = 0;
          d = lfo_tbl[lfo_pos] * lfo_depth;

          ap_set_delay(ap, d);
	  ofs = spread * 0.01562f;
          ap_set_delay(ap+1, d+ofs);
	  ofs *= 2.0f;
          ap_set_delay(ap+2, d+ofs);
	  ofs *= 2.0f;
          ap_set_delay(ap+3, d+ofs);
	  ofs *= 2.0f;
          ap_set_delay(ap+4, d+ofs);
	  ofs *= 2.0f;
          ap_set_delay(ap+5, d+ofs);

        }
	//Run in series, doesn't quite sound as nice
	y = ap_run(ap, input[pos] + ym1 * fb);
	y = ap_run(ap+1, y);
	y = ap_run(ap+2, y);
	y = ap_run(ap+3, y);
	y = ap_run(ap+4, y);
	y = ap_run(ap+5, y);

        buffer_write(output[pos], y);
	ym1 = y;
      }

      plugin_data->ym1 = ym1;
      plugin_data->count = count;
      plugin_data->lfo_pos = lfo_pos;
    
}

static const LV2_Descriptor lfoPhaserDescriptor = {
  "http://plugin.org.uk/swh-plugins/lfoPhaser",
  instantiateLfoPhaser,
  connectPortLfoPhaser,
  activateLfoPhaser,
  runLfoPhaser,
  NULL,
  cleanupLfoPhaser,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _FourByFourPole {
  float *f0;
  float *fb0;
  float *f1;
  float *fb1;
  float *f2;
  float *fb2;
  float *f3;
  float *fb3;
  float *input;
  float *output;
allpass * ap;
float y0;
float y1;
float y2;
float y3;
float sr_r_2;
} FourByFourPole;

static void cleanupFourByFourPole(LV2_Handle instance)
{
FourByFourPole *plugin_data = (FourByFourPole *)instance;

      free(plugin_data->ap);
    
  free(instance);
}

static void connectPortFourByFourPole(LV2_Handle instance, uint32_t port, void *data)
{
  FourByFourPole *plugin = (FourByFourPole *)instance;

  switch (port) {
  case 0:
    plugin->f0 = data;
    break;
  case 1:
    plugin->fb0 = data;
    break;
  case 2:
    plugin->f1 = data;
    break;
  case 3:
    plugin->fb1 = data;
    break;
  case 4:
    plugin->f2 = data;
    break;
  case 5:
    plugin->fb2 = data;
    break;
  case 6:
    plugin->f3 = data;
    break;
  case 7:
    plugin->fb3 = data;
    break;
  case 8:
    plugin->input = data;
    break;
  case 9:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateFourByFourPole(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  FourByFourPole *plugin_data = (FourByFourPole *)malloc(sizeof(FourByFourPole));
  allpass * ap = plugin_data->ap;
  float y0 = plugin_data->y0;
  float y1 = plugin_data->y1;
  float y2 = plugin_data->y2;
  float y3 = plugin_data->y3;
  float sr_r_2 = plugin_data->sr_r_2;
  
      ap = calloc(16, sizeof(allpass));
      y0 = 0.0f;
      y1 = 0.0f;
      y2 = 0.0f;
      y3 = 0.0f;
      sr_r_2 = 1.0f / s_rate;
    
  plugin_data->ap = ap;
  plugin_data->y0 = y0;
  plugin_data->y1 = y1;
  plugin_data->y2 = y2;
  plugin_data->y3 = y3;
  plugin_data->sr_r_2 = sr_r_2;
  
  return (LV2_Handle)plugin_data;
}


static void activateFourByFourPole(LV2_Handle instance)
{
  FourByFourPole *plugin_data = (FourByFourPole *)instance;
  allpass * ap __attribute__ ((unused)) = plugin_data->ap;
  float y0 __attribute__ ((unused)) = plugin_data->y0;
  float y1 __attribute__ ((unused)) = plugin_data->y1;
  float y2 __attribute__ ((unused)) = plugin_data->y2;
  float y3 __attribute__ ((unused)) = plugin_data->y3;
  float sr_r_2 __attribute__ ((unused)) = plugin_data->sr_r_2;
  
      ap_clear(ap);
      ap_clear(ap+1);
      ap_clear(ap+2);
      ap_clear(ap+3);
      ap_clear(ap+4);
      ap_clear(ap+5);
      ap_clear(ap+6);
      ap_clear(ap+7);
      ap_clear(ap+8);
      ap_clear(ap+9);
      ap_clear(ap+10);
      ap_clear(ap+11);
      ap_clear(ap+12);
      ap_clear(ap+13);
      ap_clear(ap+14);
      ap_clear(ap+15);
    
}


static void runFourByFourPole(LV2_Handle instance, uint32_t sample_count)
{
  FourByFourPole *plugin_data = (FourByFourPole *)instance;

  const float f0 = *(plugin_data->f0);
  const float fb0 = *(plugin_data->fb0);
  const float f1 = *(plugin_data->f1);
  const float fb1 = *(plugin_data->fb1);
  const float f2 = *(plugin_data->f2);
  const float fb2 = *(plugin_data->fb2);
  const float f3 = *(plugin_data->f3);
  const float fb3 = *(plugin_data->fb3);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  allpass * ap = plugin_data->ap;
  float y0 = plugin_data->y0;
  float y1 = plugin_data->y1;
  float y2 = plugin_data->y2;
  float y3 = plugin_data->y3;
  float sr_r_2 = plugin_data->sr_r_2;
  
      unsigned long pos;

      ap_set_delay(ap,   f0 * sr_r_2);
      ap_set_delay(ap+1, f0 * sr_r_2);
      ap_set_delay(ap+2, f0 * sr_r_2);
      ap_set_delay(ap+3, f0 * sr_r_2);
      ap_set_delay(ap+4, f1 * sr_r_2);
      ap_set_delay(ap+5, f1 * sr_r_2);
      ap_set_delay(ap+6, f1 * sr_r_2);
      ap_set_delay(ap+7, f1 * sr_r_2);
      ap_set_delay(ap+8, f2 * sr_r_2);
      ap_set_delay(ap+9, f2 * sr_r_2);
      ap_set_delay(ap+10, f2 * sr_r_2);
      ap_set_delay(ap+11, f2 * sr_r_2);
      ap_set_delay(ap+12, f3 * sr_r_2);
      ap_set_delay(ap+13, f3 * sr_r_2);
      ap_set_delay(ap+14, f3 * sr_r_2);
      ap_set_delay(ap+15, f3 * sr_r_2);

      for (pos = 0; pos < sample_count; pos++) {
	y0 = ap_run(ap,   input[pos] + y0 * fb0);
	y0 = ap_run(ap+1,   y0);
	y0 = ap_run(ap+2,   y0);
	y0 = ap_run(ap+3,   y0);

	y1 = ap_run(ap+4,   y0 + y1 * fb1);
	y1 = ap_run(ap+5,   y1);
	y1 = ap_run(ap+6,   y1);
	y1 = ap_run(ap+7,   y1);

	y2 = ap_run(ap+8,  y1 + y2 * fb2);
	y2 = ap_run(ap+9,  y2);
	y2 = ap_run(ap+10, y2);
	y2 = ap_run(ap+11, y2);

	y3 = ap_run(ap+12, y2 + y3 * fb3);
	y3 = ap_run(ap+13, y3);
	y3 = ap_run(ap+14, y3);
	y3 = ap_run(ap+15, y3);

        buffer_write(output[pos], y3);
      }

      plugin_data->y0 = y0;
      plugin_data->y1 = y1;
      plugin_data->y2 = y2;
      plugin_data->y3 = y3;
    
}

static const LV2_Descriptor fourByFourPoleDescriptor = {
  "http://plugin.org.uk/swh-plugins/fourByFourPole",
  instantiateFourByFourPole,
  connectPortFourByFourPole,
  activateFourByFourPole,
  runFourByFourPole,
  NULL,
  cleanupFourByFourPole,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _AutoPhaser {
  float *attack_p;
  float *decay_p;
  float *depth_p;
  float *fb;
  float *spread;
  float *input;
  float *output;
allpass * ap;
float ym1;
envelope * env;
float sample_rate;
} AutoPhaser;

static void cleanupAutoPhaser(LV2_Handle instance)
{
AutoPhaser *plugin_data = (AutoPhaser *)instance;

      free(plugin_data->ap);
      free(plugin_data->env);
    
  free(instance);
}

static void connectPortAutoPhaser(LV2_Handle instance, uint32_t port, void *data)
{
  AutoPhaser *plugin = (AutoPhaser *)instance;

  switch (port) {
  case 0:
    plugin->attack_p = data;
    break;
  case 1:
    plugin->decay_p = data;
    break;
  case 2:
    plugin->depth_p = data;
    break;
  case 3:
    plugin->fb = data;
    break;
  case 4:
    plugin->spread = data;
    break;
  case 5:
    plugin->input = data;
    break;
  case 6:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateAutoPhaser(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  AutoPhaser *plugin_data = (AutoPhaser *)malloc(sizeof(AutoPhaser));
  allpass * ap = plugin_data->ap;
  float ym1 = plugin_data->ym1;
  envelope * env = plugin_data->env;
  float sample_rate = plugin_data->sample_rate;
  
      ap = calloc(6, sizeof(allpass));
      env = calloc(1, sizeof(envelope));
      ym1 = 0.0f;
      sample_rate = (float)s_rate;
    
  plugin_data->ap = ap;
  plugin_data->ym1 = ym1;
  plugin_data->env = env;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activateAutoPhaser(LV2_Handle instance)
{
  AutoPhaser *plugin_data = (AutoPhaser *)instance;
  allpass * ap __attribute__ ((unused)) = plugin_data->ap;
  float ym1 __attribute__ ((unused)) = plugin_data->ym1;
  envelope * env __attribute__ ((unused)) = plugin_data->env;
  float sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  
      ap_clear(ap);
      ap_clear(ap+1);
      ap_clear(ap+2);
      ap_clear(ap+3);
      ap_clear(ap+4);
      ap_clear(ap+5);
    
}


static void runAutoPhaser(LV2_Handle instance, uint32_t sample_count)
{
  AutoPhaser *plugin_data = (AutoPhaser *)instance;

  const float attack_p = *(plugin_data->attack_p);
  const float decay_p = *(plugin_data->decay_p);
  const float depth_p = *(plugin_data->depth_p);
  const float fb = *(plugin_data->fb);
  const float spread = *(plugin_data->spread);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  allpass * ap = plugin_data->ap;
  float ym1 = plugin_data->ym1;
  envelope * env = plugin_data->env;
  float sample_rate = plugin_data->sample_rate;
  
      unsigned long pos;
      float y, d, ofs;
      float attack = attack_p;
      float decay = decay_p;
      const float depth = depth_p * 0.5f;

      if (attack < 0.01f) {
        attack = 0.01f;
      }
      if (decay < 0.01f) {
        decay = 0.01f;
      }
      env_set_attack(env, attack * sample_rate * 0.25f);
      env_set_release(env, decay * sample_rate * 0.25f);


      for (pos = 0; pos < sample_count; pos++) {
        if (pos % 4 == 0) {
          d = env_run(env, input[pos]) * depth;
          ap_set_delay(ap, d);
          ofs = spread * 0.01562f;
          ap_set_delay(ap+1, d+ofs);
          ofs *= 2.0f;
          ap_set_delay(ap+2, d+ofs);
          ofs *= 2.0f;
          ap_set_delay(ap+3, d+ofs);
          ofs *= 2.0f;
          ap_set_delay(ap+4, d+ofs);
          ofs *= 2.0f;
          ap_set_delay(ap+5, d+ofs);
        }

	//Run allpass filters in series
	y = ap_run(ap, input[pos] + ym1 * fb);
	y = ap_run(ap+1, y);
	y = ap_run(ap+2, y);
	y = ap_run(ap+3, y);
	y = ap_run(ap+4, y);
	y = ap_run(ap+5, y);

        buffer_write(output[pos], y);
	ym1 = y;
      }

      plugin_data->ym1 = ym1;
    
}

static const LV2_Descriptor autoPhaserDescriptor = {
  "http://plugin.org.uk/swh-plugins/autoPhaser",
  instantiateAutoPhaser,
  connectPortAutoPhaser,
  activateAutoPhaser,
  runAutoPhaser,
  NULL,
  cleanupAutoPhaser,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &lfoPhaserDescriptor;
  case 1:
    return &fourByFourPoleDescriptor;
  case 2:
    return &autoPhaserDescriptor;
  default:
    return NULL;
  }
}
