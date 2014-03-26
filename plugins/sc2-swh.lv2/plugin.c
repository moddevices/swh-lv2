
      #include "util/db.h"
      #include "util/rms.h"

      #define A_TBL 256
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Sc2 {
  float *attack;
  float *release;
  float *threshold;
  float *ratio;
  float *knee;
  float *makeup_gain;
  float *sidechain;
  float *input;
  float *output;
rms_env * rms;
float * as;
float sum;
float amp;
float gain;
float gain_t;
float env;
unsigned int count;
} Sc2;

static void cleanupSc2(LV2_Handle instance)
{
Sc2 *plugin_data = (Sc2 *)instance;

      rms_env_free(plugin_data->rms);
      free(plugin_data->as);
    
  free(instance);
}

static void connectPortSc2(LV2_Handle instance, uint32_t port, void *data)
{
  Sc2 *plugin = (Sc2 *)instance;

  switch (port) {
  case 0:
    plugin->attack = data;
    break;
  case 1:
    plugin->release = data;
    break;
  case 2:
    plugin->threshold = data;
    break;
  case 3:
    plugin->ratio = data;
    break;
  case 4:
    plugin->knee = data;
    break;
  case 5:
    plugin->makeup_gain = data;
    break;
  case 6:
    plugin->sidechain = data;
    break;
  case 7:
    plugin->input = data;
    break;
  case 8:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateSc2(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Sc2 *plugin_data = (Sc2 *)malloc(sizeof(Sc2));
  rms_env * rms = plugin_data->rms;
  float * as = plugin_data->as;
  float sum = plugin_data->sum;
  float amp = plugin_data->amp;
  float gain = plugin_data->gain;
  float gain_t = plugin_data->gain_t;
  float env = plugin_data->env;
  unsigned int count = plugin_data->count;
  
      unsigned int i;
      float sample_rate = (float)s_rate;

      rms = rms_env_new();
      sum = 0.0f;
      amp = 0.0f;
      gain = 0.0f;
      gain_t = 0.0f;
      env = 0.0f;
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
  plugin_data->count = count;
  
  return (LV2_Handle)plugin_data;
}



static void runSc2(LV2_Handle instance, uint32_t sample_count)
{
  Sc2 *plugin_data = (Sc2 *)instance;

  const float attack = *(plugin_data->attack);
  const float release = *(plugin_data->release);
  const float threshold = *(plugin_data->threshold);
  const float ratio = *(plugin_data->ratio);
  const float knee = *(plugin_data->knee);
  const float makeup_gain = *(plugin_data->makeup_gain);
  const float * const sidechain = plugin_data->sidechain;
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  rms_env * rms = plugin_data->rms;
  float * as = plugin_data->as;
  float sum = plugin_data->sum;
  float amp = plugin_data->amp;
  float gain = plugin_data->gain;
  float gain_t = plugin_data->gain_t;
  float env = plugin_data->env;
  unsigned int count = plugin_data->count;
  
      unsigned long pos;

      const float ga = as[f_round(attack * 0.001f * (float)(A_TBL-1))];
      const float gr = as[f_round(release * 0.001f * (float)(A_TBL-1))];
      const float rs = (ratio - 1.0f) / ratio;
      const float mug = db2lin(makeup_gain);
      const float knee_min = db2lin(threshold - knee);
      const float knee_max = db2lin(threshold + knee);
      const float ef_a = ga * 0.25f;
      const float ef_ai = 1.0f - ef_a;

      for (pos = 0; pos < sample_count; pos++) {
        sum += sidechain[pos] * sidechain[pos];

        if (amp > env) {
          env = env * ga + amp * (1.0f - ga);
        } else {
          env = env * gr + amp * (1.0f - gr);
        }
        if (count++ % 4 == 3) {
          amp = rms_env_process(rms, sum * 0.25f);
          sum = 0.0f;
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
        buffer_write(output[pos], input[pos] * gain * mug);
      }
      plugin_data->sum = sum;
      plugin_data->amp = amp;
      plugin_data->gain = gain;
      plugin_data->gain_t = gain_t;
      plugin_data->env = env;
      plugin_data->count = count;
    
}

static const LV2_Descriptor sc2Descriptor = {
  "http://plugin.org.uk/swh-plugins/sc2",
  instantiateSc2,
  connectPortSc2,
  NULL,
  runSc2,
  NULL,
  cleanupSc2,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &sc2Descriptor;
  default:
    return NULL;
  }
}
