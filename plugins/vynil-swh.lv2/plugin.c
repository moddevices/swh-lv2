
      #include <stdlib.h>
      #include <limits.h>

      #include "ladspa-util.h"
      #include "util/biquad.h"

      #define BUF_LEN 0.1
      #define CLICK_BUF_SIZE 4096

      #define df(x) ((sinf(x) + 1.0f) * 0.5f)

      inline static float noise();
      inline static float noise()
      {
         static unsigned int randSeed = 23;
         randSeed = (randSeed * 196314165) + 907633515;
         return randSeed / (float)INT_MAX - 1.0f;
      }

    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Vynil {
  float *year;
  float *rpm;
  float *warp;
  float *click;
  float *wear;
  float *in_l;
  float *in_r;
  float *out_l;
  float *out_r;
float fs;
float * buffer_m;
float * buffer_s;
unsigned int buffer_mask;
unsigned int buffer_pos;
float * click_buffer;
fixp16 click_buffer_pos;
fixp16 click_buffer_omega;
float click_gain;
float phi;
unsigned int sample_cnt;
float def;
float def_target;
biquad * lowp_m;
biquad * lowp_s;
biquad * noise_filt;
biquad * highp;
} Vynil;

static void cleanupVynil(LV2_Handle instance)
{
Vynil *plugin_data = (Vynil *)instance;

      free(plugin_data->buffer_m);
      free(plugin_data->buffer_s);
      free(plugin_data->click_buffer);
      free(plugin_data->lowp_m);
      free(plugin_data->lowp_s);
      free(plugin_data->noise_filt);
    
  free(instance);
}

static void connectPortVynil(LV2_Handle instance, uint32_t port, void *data)
{
  Vynil *plugin = (Vynil *)instance;

  switch (port) {
  case 0:
    plugin->year = data;
    break;
  case 1:
    plugin->rpm = data;
    break;
  case 2:
    plugin->warp = data;
    break;
  case 3:
    plugin->click = data;
    break;
  case 4:
    plugin->wear = data;
    break;
  case 5:
    plugin->in_l = data;
    break;
  case 6:
    plugin->in_r = data;
    break;
  case 7:
    plugin->out_l = data;
    break;
  case 8:
    plugin->out_r = data;
    break;
  }
}

static LV2_Handle instantiateVynil(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Vynil *plugin_data = (Vynil *)malloc(sizeof(Vynil));
  float fs = plugin_data->fs;
  float * buffer_m = plugin_data->buffer_m;
  float * buffer_s = plugin_data->buffer_s;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  float * click_buffer = plugin_data->click_buffer;
  fixp16 click_buffer_pos = plugin_data->click_buffer_pos;
  fixp16 click_buffer_omega = plugin_data->click_buffer_omega;
  float click_gain = plugin_data->click_gain;
  float phi = plugin_data->phi;
  unsigned int sample_cnt = plugin_data->sample_cnt;
  float def = plugin_data->def;
  float def_target = plugin_data->def_target;
  biquad * lowp_m = plugin_data->lowp_m;
  biquad * lowp_s = plugin_data->lowp_s;
  biquad * noise_filt = plugin_data->noise_filt;
  biquad * highp = plugin_data->highp;
  
      unsigned int i;
      unsigned int buffer_size;

      fs = (float)s_rate;
      buffer_size = 4096;
      while (buffer_size < s_rate * BUF_LEN) {
	buffer_size *= 2;
      }
      buffer_m = malloc(sizeof(float) * buffer_size);
      buffer_s = malloc(sizeof(float) * buffer_size);
      buffer_mask = buffer_size - 1;
      buffer_pos = 0;
      click_gain = 0;
      phi = 0.0f; /* Angular phase */

      click_buffer = malloc(sizeof(float) * CLICK_BUF_SIZE);
      for (i=0; i<CLICK_BUF_SIZE; i++) {
	if (i<CLICK_BUF_SIZE / 2) {
	  click_buffer[i] = (double)i / (double)(CLICK_BUF_SIZE / 2);
	  click_buffer[i] *= click_buffer[i];
	  click_buffer[i] *= click_buffer[i];
	  click_buffer[i] *= click_buffer[i];
	} else {
	  click_buffer[i] = click_buffer[CLICK_BUF_SIZE - i];
	}
      }

      sample_cnt = 0;
      def = 0.0f;
      def_target = 0.0f;

      lowp_m = calloc(sizeof(biquad), 1);
      lowp_s = calloc(sizeof(biquad), 1);
      highp = calloc(sizeof(biquad), 1);
      noise_filt = calloc(sizeof(biquad), 1);
    
  plugin_data->fs = fs;
  plugin_data->buffer_m = buffer_m;
  plugin_data->buffer_s = buffer_s;
  plugin_data->buffer_mask = buffer_mask;
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->click_buffer = click_buffer;
  plugin_data->click_buffer_pos = click_buffer_pos;
  plugin_data->click_buffer_omega = click_buffer_omega;
  plugin_data->click_gain = click_gain;
  plugin_data->phi = phi;
  plugin_data->sample_cnt = sample_cnt;
  plugin_data->def = def;
  plugin_data->def_target = def_target;
  plugin_data->lowp_m = lowp_m;
  plugin_data->lowp_s = lowp_s;
  plugin_data->noise_filt = noise_filt;
  plugin_data->highp = highp;
  
  return (LV2_Handle)plugin_data;
}


static void activateVynil(LV2_Handle instance)
{
  Vynil *plugin_data = (Vynil *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  float * buffer_m __attribute__ ((unused)) = plugin_data->buffer_m;
  float * buffer_s __attribute__ ((unused)) = plugin_data->buffer_s;
  unsigned int buffer_mask __attribute__ ((unused)) = plugin_data->buffer_mask;
  unsigned int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  float * click_buffer __attribute__ ((unused)) = plugin_data->click_buffer;
  fixp16 click_buffer_pos __attribute__ ((unused)) = plugin_data->click_buffer_pos;
  fixp16 click_buffer_omega __attribute__ ((unused)) = plugin_data->click_buffer_omega;
  float click_gain __attribute__ ((unused)) = plugin_data->click_gain;
  float phi __attribute__ ((unused)) = plugin_data->phi;
  unsigned int sample_cnt __attribute__ ((unused)) = plugin_data->sample_cnt;
  float def __attribute__ ((unused)) = plugin_data->def;
  float def_target __attribute__ ((unused)) = plugin_data->def_target;
  biquad * lowp_m __attribute__ ((unused)) = plugin_data->lowp_m;
  biquad * lowp_s __attribute__ ((unused)) = plugin_data->lowp_s;
  biquad * noise_filt __attribute__ ((unused)) = plugin_data->noise_filt;
  biquad * highp __attribute__ ((unused)) = plugin_data->highp;
  
      memset(buffer_m, 0, sizeof(float) * (buffer_mask + 1));
      memset(buffer_s, 0, sizeof(float) * (buffer_mask + 1));
      plugin_data->buffer_pos = 0;
      plugin_data->click_buffer_pos.all = 0;
      plugin_data->click_buffer_omega.all = 0;
      plugin_data->click_gain = 0;
      plugin_data->phi = 0.0f;

      lp_set_params(lowp_m, 16000.0, 0.5, fs);
      lp_set_params(lowp_s, 16000.0, 0.5, fs);
      lp_set_params(highp, 10.0, 0.5, fs);
      lp_set_params(noise_filt, 1000.0, 0.5, fs);
    
}


static void runVynil(LV2_Handle instance, uint32_t sample_count)
{
  Vynil *plugin_data = (Vynil *)instance;

  const float year = *(plugin_data->year);
  const float rpm = *(plugin_data->rpm);
  const float warp = *(plugin_data->warp);
  const float click = *(plugin_data->click);
  const float wear = *(plugin_data->wear);
  const float * const in_l = plugin_data->in_l;
  const float * const in_r = plugin_data->in_r;
  float * const out_l = plugin_data->out_l;
  float * const out_r = plugin_data->out_r;
  float fs = plugin_data->fs;
  float * buffer_m = plugin_data->buffer_m;
  float * buffer_s = plugin_data->buffer_s;
  unsigned int buffer_mask = plugin_data->buffer_mask;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  float * click_buffer = plugin_data->click_buffer;
  fixp16 click_buffer_pos = plugin_data->click_buffer_pos;
  fixp16 click_buffer_omega = plugin_data->click_buffer_omega;
  float click_gain = plugin_data->click_gain;
  float phi = plugin_data->phi;
  unsigned int sample_cnt = plugin_data->sample_cnt;
  float def = plugin_data->def;
  float def_target = plugin_data->def_target;
  biquad * lowp_m = plugin_data->lowp_m;
  biquad * lowp_s = plugin_data->lowp_s;
  biquad * noise_filt = plugin_data->noise_filt;
  biquad * highp = plugin_data->highp;
  
      unsigned long pos;
      float deflec = def;
      float deflec_target = def_target;
      float src_m, src_s;

      /* angular velocity of platter * 16 */
      const float omega = 960.0f / (rpm * fs);
      const float age = (2000 - year) * 0.01f;
      const unsigned int click_prob = (age*age*(float)RAND_MAX)/10 + click * 0.02 * RAND_MAX;
      const float noise_amp = (click + wear * 0.3f) * 0.12f + (1993.0f - year) * 0.0031f;
      const float bandwidth = (year - 1880.0f) * (rpm * 1.9f);
      const float noise_bandwidth = bandwidth * (0.25 - wear * 0.02) + click * 200.0 + 300.0;
      const float stereo = f_clamp((year - 1940.0f) * 0.02f, 0.0f, 1.0f);
      const float wrap_gain = age * 3.1f + 0.05f;
      const float wrap_bias = age * 0.1f;

      lp_set_params(lowp_m, bandwidth * (1.0 - wear * 0.86), 2.0, fs);
      lp_set_params(lowp_s, bandwidth * (1.0 - wear * 0.89), 2.0, fs);
      hp_set_params(highp, (2000-year) * 8.0, 1.5, fs);
      lp_set_params(noise_filt, noise_bandwidth, 4.0 + wear * 2.0, fs);

      for (pos = 0; pos < sample_count; pos++) {
	unsigned int o1, o2;
	float ofs;

	if ((sample_cnt & 15) == 0) {
	  const float ang = phi * 2.0f * M_PI;
	  const float w = warp * (2000.0f - year) * 0.01f;
	  deflec_target = w*df(ang)*0.5f + w*w*df(2.0f*ang)*0.31f +
                             w*w*w*df(3.0f*ang)*0.129f;
	  phi += omega;
	  while (phi > 1.0f) {
	    phi -= 1.0f;
	  }
	  if ((unsigned int)rand() < click_prob) {
	    click_buffer_omega.all = ((rand() >> 6) + 1000) * rpm;
	    click_gain = noise_amp * 5.0f * noise();
	  }
	}
	deflec = deflec * 0.1f + deflec_target * 0.9f;

	/* matrix into mid_side representation (this is roughly what stereo
         * LPs do) */
	buffer_m[buffer_pos] = in_l[pos] + in_r[pos];
	buffer_s[buffer_pos] = in_l[pos] - in_r[pos];

	/* cacluate the effects of the surface warping */
	ofs = fs * 0.009f * deflec;
	o1 = f_round(floorf(ofs));
	o2 = f_round(ceilf(ofs));
	ofs -= o1;
	src_m = LIN_INTERP(ofs, buffer_m[(buffer_pos - o1 - 1) & buffer_mask], buffer_m[(buffer_pos - o2 - 1) & buffer_mask]);
	src_s = LIN_INTERP(ofs, buffer_s[(buffer_pos - o1 - 1) & buffer_mask], buffer_s[(buffer_pos - o2 - 1) & buffer_mask]);

	src_m = biquad_run(lowp_m, src_m + click_buffer[click_buffer_pos.part.in & (CLICK_BUF_SIZE - 1)] * click_gain);

	/* waveshaper */
	src_m = LIN_INTERP(age, src_m, sinf(src_m * wrap_gain + wrap_bias));

	/* output highpass */
	src_m = biquad_run(highp, src_m) + biquad_run(noise_filt, noise()) * noise_amp + click_buffer[click_buffer_pos.part.in & (CLICK_BUF_SIZE - 1)] * click_gain * 0.5f;

	/* stereo seperation filter */
	src_s = biquad_run(lowp_s, src_s) * stereo;

        buffer_write(out_l[pos], (src_s + src_m) * 0.5f);
        buffer_write(out_r[pos], (src_m - src_s) * 0.5f);

	/* roll buffer indexes */
	buffer_pos = (buffer_pos + 1) & buffer_mask;
	click_buffer_pos.all += click_buffer_omega.all;
	if (click_buffer_pos.part.in >= CLICK_BUF_SIZE) {
	  click_buffer_pos.all = 0;
	  click_buffer_omega.all = 0;
	}
	sample_cnt++;
      }

      plugin_data->buffer_pos = buffer_pos;
      plugin_data->click_buffer_pos = click_buffer_pos;
      plugin_data->click_buffer_omega = click_buffer_omega;
      plugin_data->click_gain = click_gain;
      plugin_data->sample_cnt = sample_cnt;
      plugin_data->def_target = deflec_target;
      plugin_data->def = deflec;
      plugin_data->phi = phi;
    
}

static const LV2_Descriptor vynilDescriptor = {
  "http://plugin.org.uk/swh-plugins/vynil",
  instantiateVynil,
  connectPortVynil,
  activateVynil,
  runVynil,
  NULL,
  cleanupVynil,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &vynilDescriptor;
  default:
    return NULL;
  }
}
