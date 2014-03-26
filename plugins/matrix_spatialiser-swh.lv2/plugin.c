
	/*
	   thanks to Steve Harris for walking me through my first plugin !
	*/

	#include "ladspa-util.h"

        /* we use sin/cos panning and start at pi/4. this is the correction factor
	   to bring the signal back to unity gain in neutral position.
	   it should be 1/x : sin(x) = cos(x) (~1.41421...). but since we are using an
	   approximation of sin/cos, we take its equal gain point, which leads to 1.3333...
	*/
	#define EQUALGAINPOINT_OFFSET 128.0f
	#define EQUALGAINPOINT_TO_UNITY 4.0f / 3.0f

	#define BITSPERCYCLE 10                 /* resolution of the width parameter for */
	#define BITSPERQUARTER (BITSPERCYCLE-2) /* one cycle (0-2pi) */

	/* borrowed code: http://www.dspguru.com/comp.dsp/tricks/alg/sincos.htm
	   i'm using a constant of 0.75, which makes the calculations simpler and does
	   not yield discontinuities.
	   author: Olli Niemitalo (oniemita@mail.student.oulu.fi)
	*/
	static inline void sin_cos_approx(int phasein, float *vsin, float *vcos) {
		// Modulo phase into quarter, convert to float 0..1
		float modphase = (phasein & ((1<<BITSPERQUARTER) - 1))
		* 1.0f / (1<<BITSPERQUARTER);
		// Extract quarter bits
		int quarter = phasein & (3<<BITSPERQUARTER);
		// Recognize quarter
		if (!quarter) {
			// First quarter, angle = 0 .. pi/2
			float x = modphase - 0.5f;
			float temp = 0.75f - x * x;
			*vsin = temp + x;
			*vcos = temp - x;
		} else if (quarter == 1<<BITSPERQUARTER) {
			// Second quarter, angle = pi/2 .. pi
			float x = 0.5f - modphase;
			float temp = 0.75f - x*x;
			*vsin = x + temp;
			*vcos = x - temp;
		} else if (quarter == 2<<BITSPERQUARTER) {
			// Third quarter, angle = pi .. 1.5pi
			float x = modphase - 0.5f;
			float temp = x*x - 0.75f;
			*vsin = temp - x;
			*vcos = temp + x;
		} else {
			// Fourth quarter, angle = 1.5pi..2pi
			float x = modphase - 0.5f;
			float temp = 0.75f - x*x;
			*vsin = x - temp;
			*vcos = x + temp;
		}
	}
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _MatrixSpatialiser {
  float *i_left;
  float *i_right;
  float *width;
  float *o_left;
  float *o_right;
float current_m_gain;
float current_s_gain;
} MatrixSpatialiser;

static void cleanupMatrixSpatialiser(LV2_Handle instance)
{

  free(instance);
}

static void connectPortMatrixSpatialiser(LV2_Handle instance, uint32_t port, void *data)
{
  MatrixSpatialiser *plugin = (MatrixSpatialiser *)instance;

  switch (port) {
  case 0:
    plugin->i_left = data;
    break;
  case 1:
    plugin->i_right = data;
    break;
  case 2:
    plugin->width = data;
    break;
  case 3:
    plugin->o_left = data;
    break;
  case 4:
    plugin->o_right = data;
    break;
  }
}

static LV2_Handle instantiateMatrixSpatialiser(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  MatrixSpatialiser *plugin_data = (MatrixSpatialiser *)malloc(sizeof(MatrixSpatialiser));
  float current_m_gain = plugin_data->current_m_gain;
  float current_s_gain = plugin_data->current_s_gain;
  
	current_m_gain = 0.0f;
	current_s_gain = 0.0f;
    
  plugin_data->current_m_gain = current_m_gain;
  plugin_data->current_s_gain = current_s_gain;
  
  return (LV2_Handle)plugin_data;
}


static void activateMatrixSpatialiser(LV2_Handle instance)
{
  MatrixSpatialiser *plugin_data = (MatrixSpatialiser *)instance;
  float current_m_gain __attribute__ ((unused)) = plugin_data->current_m_gain;
  float current_s_gain __attribute__ ((unused)) = plugin_data->current_s_gain;
  
	sin_cos_approx(EQUALGAINPOINT_OFFSET, &current_s_gain, &current_m_gain);
	current_m_gain *= EQUALGAINPOINT_TO_UNITY; /* normalize the neutral  */
	current_s_gain *= EQUALGAINPOINT_TO_UNITY; /* setting to unity gain. */
    
}


static void runMatrixSpatialiser(LV2_Handle instance, uint32_t sample_count)
{
  MatrixSpatialiser *plugin_data = (MatrixSpatialiser *)instance;

  const float * const i_left = plugin_data->i_left;
  const float * const i_right = plugin_data->i_right;
  const float width = *(plugin_data->width);
  float * const o_left = plugin_data->o_left;
  float * const o_right = plugin_data->o_right;
  float current_m_gain = plugin_data->current_m_gain;
  float current_s_gain = plugin_data->current_s_gain;
  
	unsigned long pos;
	float mid, side;
	float m_gain, s_gain;
	int width_ = f_round(width + EQUALGAINPOINT_OFFSET);

	/* smoothen the gain changes. to spread the curve over the entire
	   buffer length (i.e.#sample_count samples), make lp dependent on
	   sample_count.
	*/
	const float lp = 7.0f / (float) sample_count; /* value found by experiment */
	const float lp_i = 1.0f - lp;

	/* do approximately the same as
	   s_gain = sin(width); m_gain = cos(width);
	   but a lot faster:
	*/
	sin_cos_approx(width_, &s_gain, &m_gain);

	m_gain *= EQUALGAINPOINT_TO_UNITY; /* normalize the neutral  */
	s_gain *= EQUALGAINPOINT_TO_UNITY; /* setting to unity gain. */

	#ifdef DEBUG
	/* do a "hardware bypass" if width == 0 */
	/* no smoothing here                    */
	if (width_ == 128) {
		for (pos = 0; pos < sample_count; pos++) {
		o_left[pos] = i_left[pos];
		o_right[pos] =i_right[pos];
		}
	} else
	#endif

	for (pos = 0; pos < sample_count; pos++) {
		current_m_gain = current_m_gain * lp_i + m_gain * lp;
		current_s_gain = current_s_gain * lp_i + s_gain * lp;
		mid = (i_left[pos] + i_right[pos]) * 0.5f * current_m_gain;
		side = (i_left[pos] - i_right[pos]) * 0.5f * current_s_gain;
		o_left[pos] = mid + side;
		o_right[pos] = mid - side;
	}

	plugin_data->current_m_gain = current_m_gain;
	plugin_data->current_s_gain = current_s_gain;
    
}

static const LV2_Descriptor matrixSpatialiserDescriptor = {
  "http://plugin.org.uk/swh-plugins/matrixSpatialiser",
  instantiateMatrixSpatialiser,
  connectPortMatrixSpatialiser,
  activateMatrixSpatialiser,
  runMatrixSpatialiser,
  NULL,
  cleanupMatrixSpatialiser,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &matrixSpatialiserDescriptor;
  default:
    return NULL;
  }
}
