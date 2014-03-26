
#include "ladspa-util.h"
#include "util/blo.h"

// Return the value of the LDO's for given coeffs
#define LFO(a,b) (a*lfo1 + b*lfo2)

// Ampmod / ringmod two signals together with given depth
#define RINGMOD(c,m,d) (c * ((d * 0.5f) * m + (2.0f - d)))

// Stuff needed for the soft clipping code
#define MAX_AMP 1.0f
#define CLIP 0.8f
#define CLIP_A ((MAX_AMP - CLIP) * (MAX_AMP - CLIP))
#define CLIP_B (MAX_AMP - 2.0f * CLIP)

// Constants to match filter types
#define F_LP 1
#define F_HP 2
#define F_BP 3
#define F_BR 4
#define F_AP 5

// Number of filter oversamples
#define F_R 3

// Magic number
#define NOISE 23

float *sin_tbl, *tri_tbl, *saw_tbl, *squ_tbl;
int tbl_ref_count = 0;
long sample_rate;

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

inline float soft_clip(float sc_in) {
	if ((sc_in < CLIP) && (sc_in > -CLIP)) {
		return sc_in;
	} else if (sc_in > 0.0f) {
		return MAX_AMP - (CLIP_A / (CLIP_B + sc_in));
	} else {
		return -(MAX_AMP - (CLIP_A / (CLIP_B - sc_in)));
	}
}

/* Store data in SVF struct, takes the sampling frequency, cutoff frequency
   and Q, and fills in the structure passed */

inline void setup_svf(sv_filter *sv, float fs, float fc, float q, int t) {
	sv->f = 2.0f * sinf(M_PI * fc / (float)(fs * F_R));
	sv->q = 2.0f * cosf(powf(q, 0.1f) * M_PI * 0.5f);
	sv->qnrm = sqrtf(sv->q*0.5f + 0.01f);

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

/* Change the frequency of a running SVF */

inline void setup_f_svf(sv_filter *sv, const float fs, const float fc) {
	sv->f = 2.0f * sin(M_PI * fc / ((float)(fs * F_R)));
}

/* Run one sample through the SV filter. Filter is by andy@vellocet */

inline float run_svf(sv_filter *sv, float in) {
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

inline int wave_tbl(const float wave) {
	switch (f_round(wave)) {
		case 0:
		return BLO_SINE;
		break;

		case 1:
		return BLO_TRI;
		break;

		case 2:
		return BLO_SAW;
		break;

		case 3:
		return BLO_SQUARE;
		break;
	}
	return NOISE;
}
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _HermesFilter {
  float *lfo1_freq;
  float *lfo1_wave;
  float *lfo2_freq;
  float *lfo2_wave;
  float *osc1_freq;
  float *osc1_wave;
  float *osc2_freq;
  float *osc2_wave;
  float *rm1_depth;
  float *rm2_depth;
  float *rm3_depth;
  float *osc1_gain_db;
  float *rm1_gain_db;
  float *osc2_gain_db;
  float *rm2_gain_db;
  float *in_gain_db;
  float *rm3_gain_db;
  float *xover_lfreqp;
  float *xover_ufreqp;
  float *drive1;
  float *drive2;
  float *drive3;
  float *filt1_type;
  float *filt1_freq;
  float *filt1_q;
  float *filt1_res;
  float *filt1_lfo1;
  float *filt1_lfo2;
  float *filt2_type;
  float *filt2_freq;
  float *filt2_q;
  float *filt2_res;
  float *filt2_lfo1;
  float *filt2_lfo2;
  float *filt3_type;
  float *filt3_freq;
  float *filt3_q;
  float *filt3_res;
  float *filt3_lfo1;
  float *filt3_lfo2;
  float *dela1_length;
  float *dela1_fb;
  float *dela1_wet;
  float *dela2_length;
  float *dela2_fb;
  float *dela2_wet;
  float *dela3_length;
  float *dela3_fb;
  float *dela3_wet;
  float *band1_gain_db;
  float *band2_gain_db;
  float *band3_gain_db;
  float *input;
  float *output;
blo_h_tables * tables;
blo_h_osc * osc1_d;
blo_h_osc * osc2_d;
blo_h_osc * lfo1_d;
blo_h_osc * lfo2_d;
float lfo1;
float lfo2;
float lfo1_phase;
float lfo2_phase;
sv_filter ** filt_data;
sv_filter * xover_b1_data;
sv_filter * xover_b2_data;
float ** dela_data;
int * dela_pos;
long count;
} HermesFilter;

static void cleanupHermesFilter(LV2_Handle instance)
{
HermesFilter *plugin_data = (HermesFilter *)instance;

free(plugin_data->filt_data[0]);
free(plugin_data->filt_data[1]);
free(plugin_data->filt_data[2]);
free(plugin_data->dela_data[0]);
free(plugin_data->dela_data[1]);
free(plugin_data->dela_data[2]);
free(plugin_data->filt_data);
free(plugin_data->dela_data);
free(plugin_data->dela_pos);
free(plugin_data->xover_b1_data);
free(plugin_data->xover_b2_data);

blo_h_free(plugin_data->osc1_d);
blo_h_free(plugin_data->osc2_d);
blo_h_free(plugin_data->lfo1_d);
blo_h_free(plugin_data->lfo2_d);
blo_h_tables_free(plugin_data->tables);
		
  free(instance);
}

static void connectPortHermesFilter(LV2_Handle instance, uint32_t port, void *data)
{
  HermesFilter *plugin = (HermesFilter *)instance;

  switch (port) {
  case 0:
    plugin->lfo1_freq = data;
    break;
  case 1:
    plugin->lfo1_wave = data;
    break;
  case 2:
    plugin->lfo2_freq = data;
    break;
  case 3:
    plugin->lfo2_wave = data;
    break;
  case 4:
    plugin->osc1_freq = data;
    break;
  case 5:
    plugin->osc1_wave = data;
    break;
  case 6:
    plugin->osc2_freq = data;
    break;
  case 7:
    plugin->osc2_wave = data;
    break;
  case 8:
    plugin->rm1_depth = data;
    break;
  case 9:
    plugin->rm2_depth = data;
    break;
  case 10:
    plugin->rm3_depth = data;
    break;
  case 11:
    plugin->osc1_gain_db = data;
    break;
  case 12:
    plugin->rm1_gain_db = data;
    break;
  case 13:
    plugin->osc2_gain_db = data;
    break;
  case 14:
    plugin->rm2_gain_db = data;
    break;
  case 15:
    plugin->in_gain_db = data;
    break;
  case 16:
    plugin->rm3_gain_db = data;
    break;
  case 17:
    plugin->xover_lfreqp = data;
    break;
  case 18:
    plugin->xover_ufreqp = data;
    break;
  case 19:
    plugin->drive1 = data;
    break;
  case 20:
    plugin->drive2 = data;
    break;
  case 21:
    plugin->drive3 = data;
    break;
  case 22:
    plugin->filt1_type = data;
    break;
  case 23:
    plugin->filt1_freq = data;
    break;
  case 24:
    plugin->filt1_q = data;
    break;
  case 25:
    plugin->filt1_res = data;
    break;
  case 26:
    plugin->filt1_lfo1 = data;
    break;
  case 27:
    plugin->filt1_lfo2 = data;
    break;
  case 28:
    plugin->filt2_type = data;
    break;
  case 29:
    plugin->filt2_freq = data;
    break;
  case 30:
    plugin->filt2_q = data;
    break;
  case 31:
    plugin->filt2_res = data;
    break;
  case 32:
    plugin->filt2_lfo1 = data;
    break;
  case 33:
    plugin->filt2_lfo2 = data;
    break;
  case 34:
    plugin->filt3_type = data;
    break;
  case 35:
    plugin->filt3_freq = data;
    break;
  case 36:
    plugin->filt3_q = data;
    break;
  case 37:
    plugin->filt3_res = data;
    break;
  case 38:
    plugin->filt3_lfo1 = data;
    break;
  case 39:
    plugin->filt3_lfo2 = data;
    break;
  case 40:
    plugin->dela1_length = data;
    break;
  case 41:
    plugin->dela1_fb = data;
    break;
  case 42:
    plugin->dela1_wet = data;
    break;
  case 43:
    plugin->dela2_length = data;
    break;
  case 44:
    plugin->dela2_fb = data;
    break;
  case 45:
    plugin->dela2_wet = data;
    break;
  case 46:
    plugin->dela3_length = data;
    break;
  case 47:
    plugin->dela3_fb = data;
    break;
  case 48:
    plugin->dela3_wet = data;
    break;
  case 49:
    plugin->band1_gain_db = data;
    break;
  case 50:
    plugin->band2_gain_db = data;
    break;
  case 51:
    plugin->band3_gain_db = data;
    break;
  case 52:
    plugin->input = data;
    break;
  case 53:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateHermesFilter(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  HermesFilter *plugin_data = (HermesFilter *)malloc(sizeof(HermesFilter));
  blo_h_tables * tables = plugin_data->tables;
  blo_h_osc * osc1_d = plugin_data->osc1_d;
  blo_h_osc * osc2_d = plugin_data->osc2_d;
  blo_h_osc * lfo1_d = plugin_data->lfo1_d;
  blo_h_osc * lfo2_d = plugin_data->lfo2_d;
  float lfo1 = plugin_data->lfo1;
  float lfo2 = plugin_data->lfo2;
  float lfo1_phase = plugin_data->lfo1_phase;
  float lfo2_phase = plugin_data->lfo2_phase;
  sv_filter ** filt_data = plugin_data->filt_data;
  sv_filter * xover_b1_data = plugin_data->xover_b1_data;
  sv_filter * xover_b2_data = plugin_data->xover_b2_data;
  float ** dela_data = plugin_data->dela_data;
  int * dela_pos = plugin_data->dela_pos;
  long count = plugin_data->count;
  
long i;

sample_rate = s_rate;
count = 0;

tables = blo_h_tables_new(1024);
osc1_d = blo_h_new(tables, BLO_SINE, (float)s_rate);
osc2_d = blo_h_new(tables, BLO_SINE, (float)s_rate);
lfo1_d = blo_h_new(tables, BLO_SINE, (float)s_rate);
lfo2_d = blo_h_new(tables, BLO_SINE, (float)s_rate);

xover_b1_data = calloc(1, sizeof(sv_filter));
xover_b2_data = calloc(1, sizeof(sv_filter));

dela_data = malloc(3 * sizeof(float));
dela_pos = malloc(3 * sizeof(int));
filt_data = malloc(3 * sizeof(sv_filter *));
for (i = 0; i < 3; i++) {
	dela_data[i] = malloc(sample_rate * 2 * sizeof(float));
	dela_pos[i] = 0;
	filt_data[i] = calloc(1, sizeof(sv_filter));
}
lfo1 = 0.0f;
lfo2 = 0.0f;
lfo1_phase = 0.0f;
lfo2_phase = 0.0f;
		
  plugin_data->tables = tables;
  plugin_data->osc1_d = osc1_d;
  plugin_data->osc2_d = osc2_d;
  plugin_data->lfo1_d = lfo1_d;
  plugin_data->lfo2_d = lfo2_d;
  plugin_data->lfo1 = lfo1;
  plugin_data->lfo2 = lfo2;
  plugin_data->lfo1_phase = lfo1_phase;
  plugin_data->lfo2_phase = lfo2_phase;
  plugin_data->filt_data = filt_data;
  plugin_data->xover_b1_data = xover_b1_data;
  plugin_data->xover_b2_data = xover_b2_data;
  plugin_data->dela_data = dela_data;
  plugin_data->dela_pos = dela_pos;
  plugin_data->count = count;
  
  return (LV2_Handle)plugin_data;
}


static void activateHermesFilter(LV2_Handle instance)
{
  HermesFilter *plugin_data = (HermesFilter *)instance;
  blo_h_tables * tables __attribute__ ((unused)) = plugin_data->tables;
  blo_h_osc * osc1_d __attribute__ ((unused)) = plugin_data->osc1_d;
  blo_h_osc * osc2_d __attribute__ ((unused)) = plugin_data->osc2_d;
  blo_h_osc * lfo1_d __attribute__ ((unused)) = plugin_data->lfo1_d;
  blo_h_osc * lfo2_d __attribute__ ((unused)) = plugin_data->lfo2_d;
  float lfo1 __attribute__ ((unused)) = plugin_data->lfo1;
  float lfo2 __attribute__ ((unused)) = plugin_data->lfo2;
  float lfo1_phase __attribute__ ((unused)) = plugin_data->lfo1_phase;
  float lfo2_phase __attribute__ ((unused)) = plugin_data->lfo2_phase;
  sv_filter ** filt_data __attribute__ ((unused)) = plugin_data->filt_data;
  sv_filter * xover_b1_data __attribute__ ((unused)) = plugin_data->xover_b1_data;
  sv_filter * xover_b2_data __attribute__ ((unused)) = plugin_data->xover_b2_data;
  float ** dela_data __attribute__ ((unused)) = plugin_data->dela_data;
  int * dela_pos __attribute__ ((unused)) = plugin_data->dela_pos;
  long count __attribute__ ((unused)) = plugin_data->count;
  
setup_svf(filt_data[0], 0, 0, 0, 0);
setup_svf(filt_data[1], 0, 0, 0, 0);
setup_svf(filt_data[2], 0, 0, 0, 0);
setup_svf(xover_b1_data, sample_rate, 1000.0, 0.0, F_HP);
setup_svf(xover_b2_data, sample_rate, 100.0, 0.0, F_LP);
memset(dela_data[0], 0, sample_rate * 2 * sizeof(float));
memset(dela_data[1], 0, sample_rate * 2 * sizeof(float));
memset(dela_data[2], 0, sample_rate * 2 * sizeof(float));
dela_pos[0] = 0;
dela_pos[1] = 0;
dela_pos[2] = 0;
/*
osc1_d->ph.all = 0;
osc2_d->ph.all = 0;
lfo1_d->ph.all = 0;
lfo2_d->ph.all = 0;
*/
count = 0;
lfo1 = 0.0f;
lfo2 = 0.0f;
lfo1_phase = 0.0f;
lfo2_phase = 0.0f;
		
}


static void runHermesFilter(LV2_Handle instance, uint32_t sample_count)
{
  HermesFilter *plugin_data = (HermesFilter *)instance;

  const float lfo1_freq = *(plugin_data->lfo1_freq);
  const float lfo1_wave = *(plugin_data->lfo1_wave);
  const float lfo2_freq = *(plugin_data->lfo2_freq);
  const float lfo2_wave = *(plugin_data->lfo2_wave);
  const float osc1_freq = *(plugin_data->osc1_freq);
  const float osc1_wave = *(plugin_data->osc1_wave);
  const float osc2_freq = *(plugin_data->osc2_freq);
  const float osc2_wave = *(plugin_data->osc2_wave);
  const float rm1_depth = *(plugin_data->rm1_depth);
  const float rm2_depth = *(plugin_data->rm2_depth);
  const float rm3_depth = *(plugin_data->rm3_depth);
  const float osc1_gain_db = *(plugin_data->osc1_gain_db);
  const float rm1_gain_db = *(plugin_data->rm1_gain_db);
  const float osc2_gain_db = *(plugin_data->osc2_gain_db);
  const float rm2_gain_db = *(plugin_data->rm2_gain_db);
  const float in_gain_db = *(plugin_data->in_gain_db);
  const float rm3_gain_db = *(plugin_data->rm3_gain_db);
  const float xover_lfreqp = *(plugin_data->xover_lfreqp);
  const float xover_ufreqp = *(plugin_data->xover_ufreqp);
  const float drive1 = *(plugin_data->drive1);
  const float drive2 = *(plugin_data->drive2);
  const float drive3 = *(plugin_data->drive3);
  const float filt1_type = *(plugin_data->filt1_type);
  const float filt1_freq = *(plugin_data->filt1_freq);
  const float filt1_q = *(plugin_data->filt1_q);
  const float filt1_res = *(plugin_data->filt1_res);
  const float filt1_lfo1 = *(plugin_data->filt1_lfo1);
  const float filt1_lfo2 = *(plugin_data->filt1_lfo2);
  const float filt2_type = *(plugin_data->filt2_type);
  const float filt2_freq = *(plugin_data->filt2_freq);
  const float filt2_q = *(plugin_data->filt2_q);
  const float filt2_res = *(plugin_data->filt2_res);
  const float filt2_lfo1 = *(plugin_data->filt2_lfo1);
  const float filt2_lfo2 = *(plugin_data->filt2_lfo2);
  const float filt3_type = *(plugin_data->filt3_type);
  const float filt3_freq = *(plugin_data->filt3_freq);
  const float filt3_q = *(plugin_data->filt3_q);
  const float filt3_res = *(plugin_data->filt3_res);
  const float filt3_lfo1 = *(plugin_data->filt3_lfo1);
  const float filt3_lfo2 = *(plugin_data->filt3_lfo2);
  const float dela1_length = *(plugin_data->dela1_length);
  const float dela1_fb = *(plugin_data->dela1_fb);
  const float dela1_wet = *(plugin_data->dela1_wet);
  const float dela2_length = *(plugin_data->dela2_length);
  const float dela2_fb = *(plugin_data->dela2_fb);
  const float dela2_wet = *(plugin_data->dela2_wet);
  const float dela3_length = *(plugin_data->dela3_length);
  const float dela3_fb = *(plugin_data->dela3_fb);
  const float dela3_wet = *(plugin_data->dela3_wet);
  const float band1_gain_db = *(plugin_data->band1_gain_db);
  const float band2_gain_db = *(plugin_data->band2_gain_db);
  const float band3_gain_db = *(plugin_data->band3_gain_db);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  blo_h_tables * tables = plugin_data->tables;
  blo_h_osc * osc1_d = plugin_data->osc1_d;
  blo_h_osc * osc2_d = plugin_data->osc2_d;
  blo_h_osc * lfo1_d = plugin_data->lfo1_d;
  blo_h_osc * lfo2_d = plugin_data->lfo2_d;
  float lfo1 = plugin_data->lfo1;
  float lfo2 = plugin_data->lfo2;
  float lfo1_phase = plugin_data->lfo1_phase;
  float lfo2_phase = plugin_data->lfo2_phase;
  sv_filter ** filt_data = plugin_data->filt_data;
  sv_filter * xover_b1_data = plugin_data->xover_b1_data;
  sv_filter * xover_b2_data = plugin_data->xover_b2_data;
  float ** dela_data = plugin_data->dela_data;
  int * dela_pos = plugin_data->dela_pos;
  long count = plugin_data->count;
  
unsigned long pos;
int i;

// dB gains converted to coefficients
float osc1_gain, rm1_gain, osc2_gain, rm2_gain, in_gain, rm3_gain;

// Output values for the oscilators etc.
float osc1, osc2, in, rm1, rm2, rm3, mixer1;

// Outputs from xover
float xover[3], band_gain[3];

// Output values for disortions
float dist[3];

// Stuff for distortions
float drive[3];

// Stuff for filters
float filt[3];
float filt_freq[3];
float filt_res[3];
float filt_lfo1[3];
float filt_lfo2[3];
int filt_t[3];

// Values for delays
float dela[3], dela_wet[3], dela_fb[3];
int dela_offset[3];

// Output of mixer2
float mixer2;

// X overs
const float xover_ufreq = f_clamp(xover_ufreqp, 200.0f, (float)(sample_rate / 6));
const float xover_lfreq = f_clamp(xover_lfreqp, 0.0f, xover_ufreq);
setup_f_svf(xover_b1_data, sample_rate, xover_ufreq);
setup_f_svf(xover_b2_data, sample_rate, xover_lfreq);

// Calculate delay offsets
dela_offset[0] = dela1_length * sample_rate;
dela_offset[1] = dela2_length * sample_rate;
dela_offset[2] = dela3_length * sample_rate;
for (i = 0; i < 3; i++) {
	if (dela_offset[i] > sample_rate * 2 || dela_offset[i] < 0) {
		dela_offset[i] = 0;
	}
	dela[i] = 0.0f;
	filt_t[i] = 0;
}

// Convert dB gains to coefficients
osc1_gain = DB_CO(osc1_gain_db);
osc2_gain = DB_CO(osc2_gain_db);
in_gain   = DB_CO(in_gain_db);
rm1_gain  = DB_CO(rm1_gain_db);
rm2_gain  = DB_CO(rm2_gain_db);
rm3_gain  = DB_CO(rm3_gain_db);
band_gain[0] = DB_CO(band1_gain_db);
band_gain[1] = DB_CO(band2_gain_db);
band_gain[2] = DB_CO(band3_gain_db);

osc1_d->wave = wave_tbl(osc1_wave);
osc2_d->wave = wave_tbl(osc2_wave);
lfo1_d->wave = wave_tbl(lfo1_wave);
lfo2_d->wave = wave_tbl(lfo2_wave);

blo_hd_set_freq(osc1_d, osc1_freq);
blo_hd_set_freq(osc2_d, osc2_freq);
blo_hd_set_freq(lfo1_d, lfo1_freq * 16);
blo_hd_set_freq(lfo2_d, lfo2_freq * 16);

#define SETUP_F(n,f,q,t) setup_svf(filt_data[n], sample_rate, f, q, (int)t)

// Set filter stuff
SETUP_F(0, filt1_freq, filt1_q, filt1_type);
SETUP_F(1, filt2_freq, filt2_q, filt2_type);
SETUP_F(2, filt3_freq, filt3_q, filt3_type);

filt_freq[0] = filt1_freq;
filt_freq[1] = filt2_freq;
filt_freq[2] = filt3_freq;
filt_res[0] = filt1_res;
filt_res[1] = filt2_res;
filt_res[2] = filt3_res;
filt_lfo1[0] = filt1_lfo1;
filt_lfo1[1] = filt2_lfo1;
filt_lfo1[2] = filt3_lfo1;
filt_lfo2[0] = filt1_lfo2;
filt_lfo2[1] = filt2_lfo2;
filt_lfo2[2] = filt3_lfo2;

// Setup distortions
drive[0] = drive1;
drive[1] = drive2;
drive[2] = drive3;

// Setup delays
dela_wet[0] = dela1_wet;
dela_wet[1] = dela2_wet;
dela_wet[2] = dela3_wet;
dela_fb[0] = dela1_fb;
dela_fb[1] = dela2_fb;
dela_fb[2] = dela3_fb;

tables = tables; // To shut up gcc

for (pos = 0; pos < sample_count; pos++) {
	count++; // Count of number of samples processed

	// Calculate oscilator values for this sample

	if (osc1_d->wave == NOISE) {
		osc1 = rand() * (0.5f/(float)RAND_MAX) - 1.0f;
	} else {
		osc1 = blo_hd_run_lin(osc1_d);
	}
	if (osc2_d->wave == NOISE) {
		osc2 = rand() * (0.5f/(float)RAND_MAX) - 1.0f;
	} else {
		osc2 = blo_hd_run_lin(osc2_d);
	}

	// Calculate LFO values every 16 samples
	if ((count & 15) == 1) {
		// Calculate lfo values
		if (lfo1_d->wave == NOISE) {
			lfo1_phase += lfo1_freq;
			if (lfo1_phase >= sample_rate) {
				lfo1_phase -= sample_rate;
				lfo1 = rand() * (0.5f/(float)RAND_MAX) - 1.0f;
			}
		} else {
			lfo1 = blo_hd_run_lin(lfo1_d);
		}
		if (lfo2_d->wave == NOISE) {
			lfo2_phase += lfo1_freq;
			if (lfo2_phase >= sample_rate) {
				lfo2_phase -= sample_rate;
				lfo2 = rand() * (0.5f/(float)RAND_MAX) - 1.0f;
			}
		} else {
			lfo2 = blo_hd_run_lin(lfo2_d);
		}
	}

	in = input[pos];
	rm1 = RINGMOD(osc2, osc1, rm1_depth);
	rm2 = RINGMOD(in, osc2, rm2_depth);
	rm3 = RINGMOD(osc1, in, rm3_depth);

	mixer1 = (osc1 * osc1_gain) + (osc2 * osc2_gain) + (in * in_gain) +
	         (rm1 * rm1_gain) + (rm2 * rm2_gain) + (rm3 * rm3_gain);

	mixer1 = soft_clip(mixer1);

	// Higpass off the top band
	xover[0] = run_svf(xover_b1_data, mixer1);
	// Lowpass off the bottom band
	xover[2] = run_svf(xover_b2_data, mixer1);
	// The middle band is whats left
	xover[1] = mixer1 - xover[0] - xover[2];

	mixer2 = 0.0f;
	for (i = 0; i < 3; i++) {
		dist[i] = xover[i]*(fabs(xover[i]) + drive1)/(xover[i]*xover[i] + (drive[i]-1)*fabs(xover[i]) + 1.0f);

		if (filt_t[i] == 0) {
			filt[i] = dist[i];
		} else {
			if (count % 16 == 1) {
				setup_f_svf(filt_data[i], sample_rate, filt_freq[i]+LFO(filt_lfo1[i], filt_lfo2[i]));
			}
			filt[i] = run_svf(filt_data[i], dist[i] + (filt_res[i] * (filt_data[i])->b));
		}

		dela[i] = (dela_data[i][dela_pos[i]] * dela_wet[i]) + filt[i];
		dela_data[i][(dela_pos[i] + dela_offset[i]) %
		 (2 * sample_rate)] = filt[i] + (dela[i] * dela_fb[i]);
		dela_pos[i] = (dela_pos[i] + 1) % (2 * sample_rate);

		mixer2 += band_gain[i] * dela[i];
	}

	buffer_write(output[pos], soft_clip(mixer2));
}

plugin_data->count = count;
plugin_data->lfo1 = lfo1;
plugin_data->lfo2 = lfo2;
plugin_data->lfo1_phase = lfo1_phase;
plugin_data->lfo2_phase = lfo2_phase;
		
}

static const LV2_Descriptor hermesFilterDescriptor = {
  "http://plugin.org.uk/swh-plugins/hermesFilter",
  instantiateHermesFilter,
  connectPortHermesFilter,
  activateHermesFilter,
  runHermesFilter,
  NULL,
  cleanupHermesFilter,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &hermesFilterDescriptor;
  default:
    return NULL;
  }
}
