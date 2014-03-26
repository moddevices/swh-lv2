
      #include "util/db.h"
      #include "util/rms.h"

      #define A_TBL 256
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Sc4 {
  float *rms_peak;
  float *attack;
  float *release;
  float *threshold;
  float *ratio;
  float *knee;
  float *makeup_gain;
  float *amplitude;
  float *gain_red;
  float *left_in;
  float *right_in;
  float *left_out;
  float *right_out;
rms_env * rms;
float * as;
float sum;
float amp;
float gain;
float gain_t;
float env;
float env_rms;
float env_peak;
unsigned int count;
} Sc4;

static void cleanupSc4(LV2_Handle instance)
{
Sc4 *plugin_data = (Sc4 *)instance;

      rms_env_free(plugin_data->rms);
      free(plugin_data->as);
    
  free(instance);
}

static void connectPortSc4(LV2_Handle instance, uint32_t port, void *data)
{
  Sc4 *plugin = (Sc4 *)instance;

  switch (port) {
  case 0:
    plugin->rms_peak = data;
    break;
  case 1:
    plugin->attack = data;
    break;
  case 2:
    plugin->release = data;
    break;
  case 3:
    plugin->threshold = data;
    break;
  case 4:
    plugin->ratio = data;
    break;
  case 5:
    plugin->knee = data;
    break;
  case 6:
    plugin->makeup_gain = data;
    break;
  case 7:
    plugin->amplitude = data;
    break;
  case 8:
    plugin->gain_red = data;
    break;
  case 9:
    plugin->left_in = data;
    break;
  case 10:
    plugin->right_in = data;
    break;
  case 11:
    plugin->left_out = data;
    break;
  case 12:
    plugin->right_out = data;
    break;
  }
}

static LV2_Handle instantiateSc4(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Sc4 *plugin_data = (Sc4 *)malloc(sizeof(Sc4));
  rms_env * rms = plugin_data->rms;
  float * as = plugin_data->as;
  float sum = plugin_data->sum;
  float amp = plugin_data->amp;
  float gain = plugin_data->gain;
  float gain_t = plugin_data->gain_t;
  float env = plugin_data->env;
  float env_rms = plugin_data->env_rms;
  float env_peak = plugin_data->env_peak;
  unsigned int count = plugin_data->count;
  
      unsigned int i;
      float sample_rate = (float)s_rate;

      rms = rms_env_new();
      sum = 0.0f;
      amp = 0.0f;
      gain = 0.0f;
      gain_t = 0.0f;
      env = 0.0f;
      env_rms = 0.0f;
      env_peak = 0.0f;
      count = 0;

      as = malloc(A_TBL * sizeof(float));
      as[0] = 1.0f;
      for (i=1; i<A_TBL; i++) {
	as[i] = expf(-1.0f / (sample_rate * (float)i / (float)A_TBL));
      }

      db_init();
    
  plugin_data->rms = rms;
  plugin_data->as = as;
  plugin_data->sum = sum;
  plugin_data->amp = amp;
  plugin_data->gain = gain;
  plugin_data->gain_t = gain_t;
  plugin_data->env = env;
  plugin_data->env_rms = env_rms;
  plugin_data->env_peak = env_peak;
  plugin_data->count = count;
  
  return (LV2_Handle)plugin_data;
}



static void runSc4(LV2_Handle instance, uint32_t sample_count)
{
  Sc4 *plugin_data = (Sc4 *)instance;

  const float rms_peak = *(plugin_data->rms_peak);
  const float attack = *(plugin_data->attack);
  const float release = *(plugin_data->release);
  const float threshold = *(plugin_data->threshold);
  const float ratio = *(plugin_data->ratio);
  const float knee = *(plugin_data->knee);
  const float makeup_gain = *(plugin_data->makeup_gain);
  float amplitude;
  float gain_red;
  const float * const left_in = plugin_data->left_in;
  const float * const right_in = plugin_data->right_in;
  float * const left_out = plugin_data->left_out;
  float * const right_out = plugin_data->right_out;
  rms_env * rms = plugin_data->rms;
  float * as = plugin_data->as;
  float sum = plugin_data->sum;
  float amp = plugin_data->amp;
  float gain = plugin_data->gain;
  float gain_t = plugin_data->gain_t;
  float env = plugin_data->env;
  float env_rms = plugin_data->env_rms;
  float env_peak = plugin_data->env_peak;
  unsigned int count = plugin_data->count;
  
      unsigned long pos;

      const float ga = attack < 2.0f ? 0.0f : as[f_round(attack * 0.001f * (float)(A_TBL-1))];
      const float gr = as[f_round(release * 0.001f * (float)(A_TBL-1))];
      const float rs = (ratio - 1.0f) / ratio;
      const float mug = db2lin(makeup_gain);
      const float knee_min = db2lin(threshold - knee);
      const float knee_max = db2lin(threshold + knee);
      const float ef_a = ga * 0.25f;
      const float ef_ai = 1.0f - ef_a;

      for (pos = 0; pos < sample_count; pos++) {
	const float la = fabs(left_in[pos]);
	const float ra = fabs(right_in[pos]);
	const float lev_in = f_max(la, ra);
        sum += lev_in * lev_in;

        if (amp > env_rms) {
          env_rms = env_rms * ga + amp * (1.0f - ga);
        } else {
          env_rms = env_rms * gr + amp * (1.0f - gr);
        }
	round_to_zero(&env_rms);
        if (lev_in > env_peak) {
          env_peak = env_peak * ga + lev_in * (1.0f - ga);
        } else {
          env_peak = env_peak * gr + lev_in * (1.0f - gr);
        }
	round_to_zero(&env_peak);
        if ((count++ & 3) == 3) {
          amp = rms_env_process(rms, sum * 0.25f);
          sum = 0.0f;
	  if (isnan(env_rms)) {
	    // This can happen sometimes, but I don't know why
	    env_rms = 0.0f;
	  }

          env = LIN_INTERP(rms_peak, env_rms, env_peak);

	  if (env <= knee_min) {
            gain_t = 1.0f;
	  } else if (env < knee_max) {
	    const float x = -(threshold - knee - lin2db(env)) / knee;
	    gain_t = db2lin(-knee * rs * x * x * 0.25f);
          } else {
            gain_t = db2lin((threshold - lin2db(env)) * rs);
          }
        }
        gain = gain * ef_a + gain_t * ef_ai;
        buffer_write(left_out[pos], left_in[pos] * gain * mug);
        buffer_write(right_out[pos], right_in[pos] * gain * mug);
      }
      plugin_data->sum = sum;
      plugin_data->amp = amp;
      plugin_data->gain = gain;
      plugin_data->gain_t = gain_t;
      plugin_data->env = env;
      plugin_data->env_rms = env_rms;
      plugin_data->env_peak = env_peak;
      plugin_data->count = count;

      *(plugin_data->amplitude) = lin2db(env);
      *(plugin_data->gain_red) = lin2db(gain);
    
}

static const LV2_Descriptor sc4Descriptor = {
  "http://plugin.org.uk/swh-plugins/sc4",
  instantiateSc4,
  connectPortSc4,
  NULL,
  runSc4,
  NULL,
  cleanupSc4,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &sc4Descriptor;
  default:
    return NULL;
  }
}
