
      #include "ladspa-util.h"
      #include "util/biquad.h"

      #define ENV_TR 0.0001f

      #define CLOSED  1
      #define OPENING 2
      #define OPEN    3
      #define HOLD    4
      #define CLOSING 5
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Gate {
  float *lf_fc;
  float *hf_fc;
  float *threshold;
  float *attack;
  float *hold;
  float *decay;
  float *range;
  float *select;
  float *level;
  float *gate_state;
  float *input;
  float *output;
float fs;
float env;
float gate;
int state;
int hold_count;
biquad * lf;
biquad * hf;
} Gate;

static void cleanupGate(LV2_Handle instance)
{
Gate *plugin_data = (Gate *)instance;

      free(plugin_data->lf);
      free(plugin_data->hf);
    
  free(instance);
}

static void connectPortGate(LV2_Handle instance, uint32_t port, void *data)
{
  Gate *plugin = (Gate *)instance;

  switch (port) {
  case 0:
    plugin->lf_fc = data;
    break;
  case 1:
    plugin->hf_fc = data;
    break;
  case 2:
    plugin->threshold = data;
    break;
  case 3:
    plugin->attack = data;
    break;
  case 4:
    plugin->hold = data;
    break;
  case 5:
    plugin->decay = data;
    break;
  case 6:
    plugin->range = data;
    break;
  case 7:
    plugin->select = data;
    break;
  case 8:
    plugin->level = data;
    break;
  case 9:
    plugin->gate_state = data;
    break;
  case 10:
    plugin->input = data;
    break;
  case 11:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateGate(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Gate *plugin_data = (Gate *)malloc(sizeof(Gate));
  float fs = plugin_data->fs;
  float env = plugin_data->env;
  float gate = plugin_data->gate;
  int state = plugin_data->state;
  int hold_count = plugin_data->hold_count;
  biquad * lf = plugin_data->lf;
  biquad * hf = plugin_data->hf;
  
      fs = s_rate;
      env = 0.0f;
      gate = 0.0f;
      state = CLOSED;
      hold_count = 0;

      lf = malloc(sizeof(biquad));
      hf = malloc(sizeof(biquad));
      biquad_init(lf);
      biquad_init(hf);
    
  plugin_data->fs = fs;
  plugin_data->env = env;
  plugin_data->gate = gate;
  plugin_data->state = state;
  plugin_data->hold_count = hold_count;
  plugin_data->lf = lf;
  plugin_data->hf = hf;
  
  return (LV2_Handle)plugin_data;
}


static void activateGate(LV2_Handle instance)
{
  Gate *plugin_data = (Gate *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  float env __attribute__ ((unused)) = plugin_data->env;
  float gate __attribute__ ((unused)) = plugin_data->gate;
  int state __attribute__ ((unused)) = plugin_data->state;
  int hold_count __attribute__ ((unused)) = plugin_data->hold_count;
  biquad * lf __attribute__ ((unused)) = plugin_data->lf;
  biquad * hf __attribute__ ((unused)) = plugin_data->hf;
  
      env = 0.0f;
      gate = 0.0f;
      state = CLOSED;
      biquad_init(lf);
      biquad_init(hf);
    
}


static void runGate(LV2_Handle instance, uint32_t sample_count)
{
  Gate *plugin_data = (Gate *)instance;

  const float lf_fc = *(plugin_data->lf_fc);
  const float hf_fc = *(plugin_data->hf_fc);
  const float threshold = *(plugin_data->threshold);
  const float attack = *(plugin_data->attack);
  const float hold = *(plugin_data->hold);
  const float decay = *(plugin_data->decay);
  const float range = *(plugin_data->range);
  const float select = *(plugin_data->select);
  float level;
  float gate_state;
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float fs = plugin_data->fs;
  float env = plugin_data->env;
  float gate = plugin_data->gate;
  int state = plugin_data->state;
  int hold_count = plugin_data->hold_count;
  biquad * lf = plugin_data->lf;
  biquad * hf = plugin_data->hf;
  
      unsigned long pos;
      float cut = DB_CO(range);
      float t_level = DB_CO(threshold);
      float a_rate = 1000.0f / (attack * fs);
      float d_rate = 1000.0f / (decay * fs);
      float post_filter, apost_filter;
      int op = f_round(select);

      ls_set_params(lf, lf_fc, -40.0f, 0.6f, fs);
      hs_set_params(hf, hf_fc, -50.0f, 0.6f, fs);

      for (pos = 0; pos < sample_count; pos++) {
	post_filter = biquad_run(lf, input[pos]);
	post_filter = biquad_run(hf, post_filter);
	apost_filter = fabs(post_filter);

        if (apost_filter > env) {
          env = apost_filter;
        } else {
          env = apost_filter * ENV_TR + env * (1.0f - ENV_TR);
        }

	if (state == CLOSED) {
	  if (env >= t_level) {
	    state = OPENING;
	  }
        } else if (state == OPENING) {
	  gate += a_rate;
	  if (gate >= 1.0f) {
	    gate = 1.0f;
	    state = OPEN;
	  }
        } else if (state == OPEN) {
          if (env < t_level) {
            state = HOLD;
            hold_count = f_round(hold * fs * 0.001f);
          }
        } else if (state == HOLD) {
	  if (env >= t_level) {
	    state = OPEN;
          } else if (hold_count <= 0) {
            state = CLOSING;
          } else {
            hold_count--;
          }
	} else if (state == CLOSING) {
	  gate -= d_rate;
	  if (env >= t_level) {
	    state = OPENING;
	  } else if (gate <= 0.0f) {
	    gate = 0.0f;
	    state = CLOSED;
	  }
	}

	if (op == 0) {
          buffer_write(output[pos], input[pos] * (cut * (1.0f - gate) + gate));
	} else if (op == -1) {
          buffer_write(output[pos], post_filter);
	} else {
	  buffer_write(output[pos], input[pos]);
	}
      }

      *(plugin_data->level) = CO_DB(env);
      switch (state) {
      case OPEN:
        *(plugin_data->gate_state) = 1.0;
        break;
      case HOLD:
        *(plugin_data->gate_state) = 0.5;
        break;
      default:
        *(plugin_data->gate_state) = 0.0;
      }

      plugin_data->env = env;
      plugin_data->gate = gate;
      plugin_data->state = state;
      plugin_data->hold_count = hold_count;
    
}

static const LV2_Descriptor gateDescriptor = {
  "http://plugin.org.uk/swh-plugins/gate",
  instantiateGate,
  connectPortGate,
  activateGate,
  runGate,
  NULL,
  cleanupGate,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &gateDescriptor;
  default:
    return NULL;
  }
}
