
#include <string.h>
#include <fftw3.h>

typedef fftwf_plan fft_plan;
typedef float fftw_real;

#include "ladspa-util.h"

#define FFT_LENGTH 1024
#define OVER_SAMP  4
#define BANDS      15


float bands[BANDS] =
  { 50.00f, 100.00f, 155.56f, 220.00f, 311.13f,
    440.00f, 622.25f, 880.00f, 1244.51f, 1760.00f, 2489.02f,
    3519.95, 4978.04f, 9956.08f, 19912.16f };
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Mbeq {
  float *band_1;
  float *band_2;
  float *band_3;
  float *band_4;
  float *band_5;
  float *band_6;
  float *band_7;
  float *band_8;
  float *band_9;
  float *band_10;
  float *band_11;
  float *band_12;
  float *band_13;
  float *band_14;
  float *band_15;
  float *input;
  float *output;
  float *latency;
float * in_fifo;
float * out_fifo;
float * out_accum;
fftw_real * real;
fftw_real * comp;
float * window;
long fifo_pos;
float * db_table;
int * bin_base;
float * bin_delta;
fft_plan plan_rc;
fft_plan plan_cr;
} Mbeq;

static void cleanupMbeq(LV2_Handle instance)
{
Mbeq *plugin_data = (Mbeq *)instance;

fftwf_destroy_plan(plugin_data->plan_rc);
fftwf_destroy_plan(plugin_data->plan_cr);
free(plugin_data->in_fifo);
free(plugin_data->out_fifo);
free(plugin_data->out_accum);
free(plugin_data->real);
free(plugin_data->comp);
free(plugin_data->window);
free(plugin_data->bin_base);
free(plugin_data->bin_delta);
free(plugin_data->db_table);
		
  free(instance);
}

static void connectPortMbeq(LV2_Handle instance, uint32_t port, void *data)
{
  Mbeq *plugin = (Mbeq *)instance;

  switch (port) {
  case 0:
    plugin->band_1 = data;
    break;
  case 1:
    plugin->band_2 = data;
    break;
  case 2:
    plugin->band_3 = data;
    break;
  case 3:
    plugin->band_4 = data;
    break;
  case 4:
    plugin->band_5 = data;
    break;
  case 5:
    plugin->band_6 = data;
    break;
  case 6:
    plugin->band_7 = data;
    break;
  case 7:
    plugin->band_8 = data;
    break;
  case 8:
    plugin->band_9 = data;
    break;
  case 9:
    plugin->band_10 = data;
    break;
  case 10:
    plugin->band_11 = data;
    break;
  case 11:
    plugin->band_12 = data;
    break;
  case 12:
    plugin->band_13 = data;
    break;
  case 13:
    plugin->band_14 = data;
    break;
  case 14:
    plugin->band_15 = data;
    break;
  case 15:
    plugin->input = data;
    break;
  case 16:
    plugin->output = data;
    break;
  case 17:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiateMbeq(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Mbeq *plugin_data = (Mbeq *)malloc(sizeof(Mbeq));
  float * in_fifo = plugin_data->in_fifo;
  float * out_fifo = plugin_data->out_fifo;
  float * out_accum = plugin_data->out_accum;
  fftw_real * real = plugin_data->real;
  fftw_real * comp = plugin_data->comp;
  float * window = plugin_data->window;
  long fifo_pos = plugin_data->fifo_pos;
  float * db_table = plugin_data->db_table;
  int * bin_base = plugin_data->bin_base;
  float * bin_delta = plugin_data->bin_delta;
  fft_plan plan_rc = plugin_data->plan_rc;
  fft_plan plan_cr = plugin_data->plan_cr;
  
int i, bin;
float last_bin, next_bin;
float db;
float hz_per_bin = (float)s_rate / (float)FFT_LENGTH;

in_fifo = calloc(FFT_LENGTH, sizeof(float));
out_fifo = calloc(FFT_LENGTH, sizeof(float));
out_accum = calloc(FFT_LENGTH * 2, sizeof(float));
real = calloc(FFT_LENGTH, sizeof(fftw_real));
comp = calloc(FFT_LENGTH, sizeof(fftw_real));
window = calloc(FFT_LENGTH, sizeof(float));
bin_base = calloc(FFT_LENGTH/2, sizeof(int));
bin_delta = calloc(FFT_LENGTH/2, sizeof(float));
fifo_pos = 0;

plan_rc = fftwf_plan_r2r_1d(FFT_LENGTH, real, comp, FFTW_R2HC, FFTW_MEASURE);
plan_cr = fftwf_plan_r2r_1d(FFT_LENGTH, comp, real, FFTW_HC2R, FFTW_MEASURE);

// Create raised cosine window table
for (i=0; i < FFT_LENGTH; i++) {
	window[i] = -0.5f*cos(2.0f*M_PI*(double)i/(double)FFT_LENGTH)+0.5f;
	window[i] *= 2.0f;
}

// Create db->coeffiecnt lookup table
db_table = malloc(1000 * sizeof(float));
for (i=0; i < 1000; i++) {
	db = ((float)i/10) - 70;
	db_table[i] = pow(10.0f, db/20.0f);
}

// Create FFT bin -> band + delta tables
bin = 0;
while (bin <= bands[0]/hz_per_bin) {
	bin_base[bin] = 0;
	bin_delta[bin++] = 0.0f;
}
for (i = 1; i < BANDS-1 && bin < (FFT_LENGTH/2)-1 && bands[i+1] < s_rate/2; i++) {
	last_bin = bin;
	next_bin = (bands[i+1])/hz_per_bin;
	while (bin <= next_bin) {
		bin_base[bin] = i;
		bin_delta[bin] = (float)(bin - last_bin) / (float)(next_bin - last_bin);
		bin++;
	}
}
for (; bin < (FFT_LENGTH/2); bin++) {
	bin_base[bin] = BANDS-1;
	bin_delta[bin] = 0.0f;
}
		
  plugin_data->in_fifo = in_fifo;
  plugin_data->out_fifo = out_fifo;
  plugin_data->out_accum = out_accum;
  plugin_data->real = real;
  plugin_data->comp = comp;
  plugin_data->window = window;
  plugin_data->fifo_pos = fifo_pos;
  plugin_data->db_table = db_table;
  plugin_data->bin_base = bin_base;
  plugin_data->bin_delta = bin_delta;
  plugin_data->plan_rc = plan_rc;
  plugin_data->plan_cr = plan_cr;
  
  return (LV2_Handle)plugin_data;
}


static void activateMbeq(LV2_Handle instance)
{
  Mbeq *plugin_data = (Mbeq *)instance;
  float * in_fifo __attribute__ ((unused)) = plugin_data->in_fifo;
  float * out_fifo __attribute__ ((unused)) = plugin_data->out_fifo;
  float * out_accum __attribute__ ((unused)) = plugin_data->out_accum;
  fftw_real * real __attribute__ ((unused)) = plugin_data->real;
  fftw_real * comp __attribute__ ((unused)) = plugin_data->comp;
  float * window __attribute__ ((unused)) = plugin_data->window;
  long fifo_pos __attribute__ ((unused)) = plugin_data->fifo_pos;
  float * db_table __attribute__ ((unused)) = plugin_data->db_table;
  int * bin_base __attribute__ ((unused)) = plugin_data->bin_base;
  float * bin_delta __attribute__ ((unused)) = plugin_data->bin_delta;
  fft_plan plan_rc __attribute__ ((unused)) = plugin_data->plan_rc;
  fft_plan plan_cr __attribute__ ((unused)) = plugin_data->plan_cr;
  
fifo_pos = 0;
		
}


static void runMbeq(LV2_Handle instance, uint32_t sample_count)
{
  Mbeq *plugin_data = (Mbeq *)instance;

  const float band_1 = *(plugin_data->band_1);
  const float band_2 = *(plugin_data->band_2);
  const float band_3 = *(plugin_data->band_3);
  const float band_4 = *(plugin_data->band_4);
  const float band_5 = *(plugin_data->band_5);
  const float band_6 = *(plugin_data->band_6);
  const float band_7 = *(plugin_data->band_7);
  const float band_8 = *(plugin_data->band_8);
  const float band_9 = *(plugin_data->band_9);
  const float band_10 = *(plugin_data->band_10);
  const float band_11 = *(plugin_data->band_11);
  const float band_12 = *(plugin_data->band_12);
  const float band_13 = *(plugin_data->band_13);
  const float band_14 = *(plugin_data->band_14);
  const float band_15 = *(plugin_data->band_15);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float latency;
  float * in_fifo = plugin_data->in_fifo;
  float * out_fifo = plugin_data->out_fifo;
  float * out_accum = plugin_data->out_accum;
  fftw_real * real = plugin_data->real;
  fftw_real * comp = plugin_data->comp;
  float * window = plugin_data->window;
  long fifo_pos = plugin_data->fifo_pos;
  float * db_table = plugin_data->db_table;
  int * bin_base = plugin_data->bin_base;
  float * bin_delta = plugin_data->bin_delta;
  fft_plan plan_rc = plugin_data->plan_rc;
  fft_plan plan_cr = plugin_data->plan_cr;
  
int i, bin, gain_idx;
float gains[BANDS + 1] =
  { band_1, band_2, band_3, band_4, band_5, band_6, band_7, band_8, band_9,
    band_10, band_11, band_12, band_13, band_14, band_15, 0.0f };
float coefs[FFT_LENGTH / 2];
unsigned long pos;

int step_size = FFT_LENGTH / OVER_SAMP;
int fft_latency = FFT_LENGTH - step_size;

// Convert gains from dB to co-efficents
for (i = 0; i < BANDS; i++) {
	gain_idx = (int)((gains[i] * 10) + 700);
	gains[i] = db_table[LIMIT(gain_idx, 0, 999)];
}

// Calculate coefficients for each bin of FFT
coefs[0] = 0.0f;
coefs[FFT_LENGTH/2-1] = 0.0f;
for (bin=1; bin < (FFT_LENGTH/2-1); bin++) {
	coefs[bin] = ((1.0f-bin_delta[bin]) * gains[bin_base[bin]])
		      + (bin_delta[bin] * gains[bin_base[bin]+1]);
}

if (fifo_pos == 0) {
	fifo_pos = fft_latency;
}

for (pos = 0; pos < sample_count; pos++) {
	in_fifo[fifo_pos] = input[pos];
	buffer_write(output[pos], out_fifo[fifo_pos-fft_latency]);
	fifo_pos++;

	// If the FIFO is full
	if (fifo_pos >= FFT_LENGTH) {
		fifo_pos = fft_latency;

		// Window input FIFO
		for (i=0; i < FFT_LENGTH; i++) {
			real[i] = in_fifo[i] * window[i];
		}

		// Run the real->complex transform
		fftwf_execute(plan_rc);

		// Multiply the bins magnitudes by the coeficients
		comp[0] *= coefs[0];
		for (i = 1; i < FFT_LENGTH/2; i++) {
			comp[i] *= coefs[i];
			comp[FFT_LENGTH-i] *= coefs[i];
		}

		// Run the complex->real transform
                fftwf_execute(plan_cr);

		// Window into the output accumulator
		for (i = 0; i < FFT_LENGTH; i++) {
			out_accum[i] += 0.9186162f * window[i] * real[i]/(FFT_LENGTH * OVER_SAMP);
		}
		for (i = 0; i < step_size; i++) {
			out_fifo[i] = out_accum[i];
		}

		// Shift output accumulator
		memmove(out_accum, out_accum + step_size, FFT_LENGTH*sizeof(float));

		// Shift input fifo
		for (i = 0; i < fft_latency; i++) {
			in_fifo[i] = in_fifo[i+step_size];
		}
	}
}

// Store the fifo_position
plugin_data->fifo_pos = fifo_pos;

*(plugin_data->latency) = fft_latency;
		
}

static const LV2_Descriptor mbeqDescriptor = {
  "http://plugin.org.uk/swh-plugins/mbeq",
  instantiateMbeq,
  connectPortMbeq,
  activateMbeq,
  runMbeq,
  NULL,
  cleanupMbeq,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &mbeqDescriptor;
  default:
    return NULL;
  }
}
