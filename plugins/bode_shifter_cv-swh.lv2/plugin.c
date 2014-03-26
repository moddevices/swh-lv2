

#include <math.h>

#include "ladspa-util.h"

#define SIN_T_SIZE 1024
#define D_SIZE 256
#define NZEROS 200

/* The non-zero taps of the Hilbert transformer */
static float xcoeffs[] = {
     +0.0008103736f, +0.0008457886f, +0.0009017196f, +0.0009793364f,
     +0.0010798341f, +0.0012044365f, +0.0013544008f, +0.0015310235f,
     +0.0017356466f, +0.0019696659f, +0.0022345404f, +0.0025318040f,
     +0.0028630784f, +0.0032300896f, +0.0036346867f, +0.0040788644f,
     +0.0045647903f, +0.0050948365f, +0.0056716186f, +0.0062980419f,
     +0.0069773575f, +0.0077132300f, +0.0085098208f, +0.0093718901f,
     +0.0103049226f, +0.0113152847f, +0.0124104218f, +0.0135991079f,
     +0.0148917649f, +0.0163008758f, +0.0178415242f, +0.0195321089f,
     +0.0213953037f, +0.0234593652f, +0.0257599469f, +0.0283426636f,
     +0.0312667947f, +0.0346107648f, +0.0384804823f, +0.0430224431f,
     +0.0484451086f, +0.0550553725f, +0.0633242001f, +0.0740128560f,
     +0.0884368322f, +0.1090816773f, +0.1412745301f, +0.1988673273f,
     +0.3326528346f, +0.9997730178f, -0.9997730178f, -0.3326528346f,
     -0.1988673273f, -0.1412745301f, -0.1090816773f, -0.0884368322f,
     -0.0740128560f, -0.0633242001f, -0.0550553725f, -0.0484451086f,
     -0.0430224431f, -0.0384804823f, -0.0346107648f, -0.0312667947f,
     -0.0283426636f, -0.0257599469f, -0.0234593652f, -0.0213953037f,
     -0.0195321089f, -0.0178415242f, -0.0163008758f, -0.0148917649f,
     -0.0135991079f, -0.0124104218f, -0.0113152847f, -0.0103049226f,
     -0.0093718901f, -0.0085098208f, -0.0077132300f, -0.0069773575f,
     -0.0062980419f, -0.0056716186f, -0.0050948365f, -0.0045647903f,
     -0.0040788644f, -0.0036346867f, -0.0032300896f, -0.0028630784f,
     -0.0025318040f, -0.0022345404f, -0.0019696659f, -0.0017356466f,
     -0.0015310235f, -0.0013544008f, -0.0012044365f, -0.0010798341f,
     -0.0009793364f, -0.0009017196f, -0.0008457886f, -0.0008103736f,
};
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _BodeShifterCV {
  float *shift_b;
  float *mix;
  float *input;
  float *atten;
  float *shift;
  float *dout;
  float *uout;
  float *mixout;
  float *latency;
float * delay;
unsigned int dptr;
float phi;
float fs;
float * sint;
} BodeShifterCV;

static void cleanupBodeShifterCV(LV2_Handle instance)
{
BodeShifterCV *plugin_data = (BodeShifterCV *)instance;

      free(plugin_data->delay);
      free(plugin_data->sint);
    
  free(instance);
}

static void connectPortBodeShifterCV(LV2_Handle instance, uint32_t port, void *data)
{
  BodeShifterCV *plugin = (BodeShifterCV *)instance;

  switch (port) {
  case 0:
    plugin->shift_b = data;
    break;
  case 1:
    plugin->mix = data;
    break;
  case 2:
    plugin->input = data;
    break;
  case 3:
    plugin->atten = data;
    break;
  case 4:
    plugin->shift = data;
    break;
  case 5:
    plugin->dout = data;
    break;
  case 6:
    plugin->uout = data;
    break;
  case 7:
    plugin->mixout = data;
    break;
  case 8:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiateBodeShifterCV(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  BodeShifterCV *plugin_data = (BodeShifterCV *)malloc(sizeof(BodeShifterCV));
  float * delay = plugin_data->delay;
  unsigned int dptr = plugin_data->dptr;
  float phi = plugin_data->phi;
  float fs = plugin_data->fs;
  float * sint = plugin_data->sint;
  
      unsigned int i;

      fs = (float)s_rate;

      delay = calloc(D_SIZE, sizeof(float));
      sint  = calloc(SIN_T_SIZE + 4, sizeof(float));

      dptr = 0;
      phi = 0.0f;

      for (i = 0; i < SIN_T_SIZE + 4; i++) {
	sint[i] = sinf(2.0f * M_PI * (float)i / (float)SIN_T_SIZE);
      }
    
  plugin_data->delay = delay;
  plugin_data->dptr = dptr;
  plugin_data->phi = phi;
  plugin_data->fs = fs;
  plugin_data->sint = sint;
  
  return (LV2_Handle)plugin_data;
}



static void runBodeShifterCV(LV2_Handle instance, uint32_t sample_count)
{
  BodeShifterCV *plugin_data = (BodeShifterCV *)instance;

  const float shift_b = *(plugin_data->shift_b);
  const float mix = *(plugin_data->mix);
  const float * const input = plugin_data->input;
  const float atten = *(plugin_data->atten);
  const float * const shift = plugin_data->shift;
  float * const dout = plugin_data->dout;
  float * const uout = plugin_data->uout;
  float * const mixout = plugin_data->mixout;
  float latency;
  float * delay = plugin_data->delay;
  unsigned int dptr = plugin_data->dptr;
  float phi = plugin_data->phi;
  float fs = plugin_data->fs;
  float * sint = plugin_data->sint;
  
      unsigned long pos;
      unsigned int i;
      float hilb, rm1, rm2;
      int int_p;
      float frac_p;
      const float freq_fix = (float)SIN_T_SIZE * 1000.0f * f_clamp(atten, 0.0f, 10.0f) / fs;
      const float base_ofs = (float)SIN_T_SIZE * f_clamp(shift_b, 0.0f, 10000.0f) / fs;
      const float mixc = mix * 0.5f + 0.5f;

      for (pos = 0; pos < sample_count; pos++) {
	delay[dptr] = input[pos];

	/* Perform the Hilbert FIR convolution
         * (probably FFT would be faster) */
        hilb = 0.0f;
        for (i = 0; i <= NZEROS/2; i++) {
          hilb += (xcoeffs[i] * delay[(dptr - i*2) & (D_SIZE - 1)]);
        }

	/* Calcuate the table positions for the sine modulator */
	int_p = f_round(floor(phi));

	/* Calculate ringmod1, the transformed input modulated with a shift Hz
	 * sinewave. This creates a +180 degree sideband at source-shift Hz and
	 * a 0 degree sindeband at source+shift Hz */
	frac_p = phi - int_p;
	rm1 = hilb * 0.63661978f * cube_interp(frac_p, sint[int_p],
			sint[int_p+1], sint[int_p+2], sint[int_p+3]);

	/* Calcuate the table positions for the cosine modulator */
	int_p = (int_p + SIN_T_SIZE / 4) & (SIN_T_SIZE - 1);

	/* Calculate ringmod2, the delayed input modulated with a shift Hz
	 * cosinewave. This creates a 0 degree sideband at source+shift Hz
	 * and a -180 degree sindeband at source-shift Hz */
	rm2 = delay[(dptr - 99) & (D_SIZE - 1)] * cube_interp(frac_p,
	      sint[int_p], sint[int_p+1], sint[int_p+2], sint[int_p+3]);

	/* Output the sum and differences of the ringmods. The +/-180 degree
	 * sidebands cancel (more of less) and just leave the shifted
         * components */
        buffer_write(dout[pos], (rm2 - rm1) * 0.5f);
        buffer_write(uout[pos], (rm2 + rm1) * 0.5f);
        buffer_write(mixout[pos], (dout[pos] - uout[pos]) * mixc + uout[pos]);

	dptr = (dptr + 1) & (D_SIZE - 1);
	phi += f_clamp(shift[pos], 0.0f, 10.0f) * freq_fix + base_ofs;
	while (phi > SIN_T_SIZE) {
	  phi -= SIN_T_SIZE;
	}
      }

      plugin_data->dptr = dptr;
      plugin_data->phi = phi;

      *(plugin_data->latency) = 99;
    
}

static const LV2_Descriptor bodeShifterCVDescriptor = {
  "http://plugin.org.uk/swh-plugins/bodeShifterCV",
  instantiateBodeShifterCV,
  connectPortBodeShifterCV,
  NULL,
  runBodeShifterCV,
  NULL,
  cleanupBodeShifterCV,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &bodeShifterCVDescriptor;
  default:
    return NULL;
  }
}
