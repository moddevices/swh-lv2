
#include "ladspa-util.h"

// Constants to match filter types
#define F_LP 1
#define F_HP 2
#define F_BP 3
#define F_BR 4
#define F_AP 5

// Number of filter oversamples
#define F_R 3

/* Structure to hold parameters for SV filter */

typedef struct {
	float f;     // 2.0*sin(PI*fs/(fc*r));
	float q;     // 2.0*cos(pow(q, 0.1)*PI*0.5);
	float qnrm;  // sqrt(m/2.0f+0.01f);
	float h;     // high pass output
	float b;     // band pass output
	float l;     // low pass output
	float p;     // peaking output (allpass with resonance)
	float n;     // notch output
	float *op;   // pointer to output value
} sv_filter;

/* Store data in SVF struct, takes the sampling frequency, cutoff frequency
   and Q, and fills in the structure passed */

static inline void setup_svf(sv_filter *sv, float fs, float fc, float q, int t) {
	sv->f = 2.0f * sin(M_PI * fc / (float)(fs * F_R));
	sv->q = 2.0f * cos(pow(q, 0.1f) * M_PI * 0.5f);
	sv->qnrm = sqrt(sv->q/2.0+0.01);
	switch(t) {
	case F_LP:
		sv->op = &(sv->l);
		break;
	case F_HP:
		sv->op = &(sv->h);
		break;
	case F_BP:
		sv->op = &(sv->b);
		break;
	case F_BR:
		sv->op = &(sv->n);
		break;
	default:
		sv->op = &(sv->p);
	}
}

/* Run one sample through the SV filter. Filter is by andy@vellocet */

static inline float run_svf(sv_filter *sv, float in) {
	float out;
	int i;

	in = sv->qnrm * in ;
	for (i=0; i < F_R; i++) {
		// only needed for pentium chips
		in  = flush_to_zero(in);
		sv->l = flush_to_zero(sv->l);
		// very slight waveshape for extra stability
		sv->b = sv->b - sv->b * sv->b * sv->b * 0.001f;

		// regular state variable code here
		// the notch and peaking outputs are optional
		sv->h = in - sv->l - sv->q * sv->b;
		sv->b = sv->b + sv->f * sv->h;
		sv->l = sv->l + sv->f * sv->b;
		sv->n = sv->l + sv->h;
		sv->p = sv->l - sv->h;

		out = *(sv->op);
		in = out;
	}

	return out;
}

		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Svf {
  float *input;
  float *output;
  float *filt_type;
  float *filt_freq;
  float *filt_q;
  float *filt_res;
int sample_rate;
sv_filter * svf;
} Svf;

static void cleanupSvf(LV2_Handle instance)
{
Svf *plugin_data = (Svf *)instance;

free(plugin_data->svf);
		
  free(instance);
}

static void connectPortSvf(LV2_Handle instance, uint32_t port, void *data)
{
  Svf *plugin = (Svf *)instance;

  switch (port) {
  case 0:
    plugin->input = data;
    break;
  case 1:
    plugin->output = data;
    break;
  case 2:
    plugin->filt_type = data;
    break;
  case 3:
    plugin->filt_freq = data;
    break;
  case 4:
    plugin->filt_q = data;
    break;
  case 5:
    plugin->filt_res = data;
    break;
  }
}

static LV2_Handle instantiateSvf(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Svf *plugin_data = (Svf *)malloc(sizeof(Svf));
  int sample_rate = plugin_data->sample_rate;
  sv_filter * svf = plugin_data->svf;
  
sample_rate = s_rate;

svf = calloc(1, sizeof(sv_filter));
		
  plugin_data->sample_rate = sample_rate;
  plugin_data->svf = svf;
  
  return (LV2_Handle)plugin_data;
}


static void activateSvf(LV2_Handle instance)
{
  Svf *plugin_data = (Svf *)instance;
  int sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  sv_filter * svf __attribute__ ((unused)) = plugin_data->svf;
  
setup_svf(svf, 0, 0, 0, 0);
		
}


static void runSvf(LV2_Handle instance, uint32_t sample_count)
{
  Svf *plugin_data = (Svf *)instance;

  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  const float filt_type = *(plugin_data->filt_type);
  const float filt_freq = *(plugin_data->filt_freq);
  const float filt_q = *(plugin_data->filt_q);
  const float filt_res = *(plugin_data->filt_res);
  int sample_rate = plugin_data->sample_rate;
  sv_filter * svf = plugin_data->svf;
  
long int pos;

setup_svf(svf, sample_rate, filt_freq, filt_q, f_round(filt_type));

for (pos = 0; pos < sample_count; pos++) {
	buffer_write(output[pos], run_svf(svf, input[pos] + (svf->b * filt_res)));
}
		
}

static const LV2_Descriptor svfDescriptor = {
  "http://plugin.org.uk/swh-plugins/svf",
  instantiateSvf,
  connectPortSvf,
  activateSvf,
  runSvf,
  NULL,
  cleanupSvf,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &svfDescriptor;
  default:
    return NULL;
  }
}
