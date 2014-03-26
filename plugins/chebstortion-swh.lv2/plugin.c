
      #include <ladspa-util.h>

      #define HARMONICS 11
      #define STAGES 2

      static float cd_lut[STAGES][HARMONICS];

      /* Calculate Chebychev coefficents from partial magnitudes, adapted from
       * example in Num. Rec. */
      void chebpc(float c[], float d[])
      {
          int k, j;
          float sv, dd[HARMONICS];
      
          for (j = 0; j < HARMONICS; j++) {
              d[j] = dd[j] = 0.0;
          }
      
          d[0] = c[HARMONICS - 1];
      
          for (j = HARMONICS - 2; j >= 1; j--) {
              for (k = HARMONICS - j; k >= 1; k--) {
                  sv = d[k];
                  d[k] = 2.0 * d[k - 1] - dd[k];
                  dd[k] = sv;
              }
              sv = d[0];
              d[0] = -dd[0] + c[j];
              dd[0] = sv;
          }
      
          for (j = HARMONICS - 1; j >= 1; j--) {
              d[j] = d[j - 1] - dd[j];
          }
          d[0] = -dd[0] + 0.5 * c[0];
      }

    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Chebstortion {
  float *dist;
  float *input;
  float *output;
float itm1;
float otm1;
float env;
unsigned int count;
} Chebstortion;

static void cleanupChebstortion(LV2_Handle instance)
{

  free(instance);
}

static void connectPortChebstortion(LV2_Handle instance, uint32_t port, void *data)
{
  Chebstortion *plugin = (Chebstortion *)instance;

  switch (port) {
  case 0:
    plugin->dist = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateChebstortion(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Chebstortion *plugin_data = (Chebstortion *)malloc(sizeof(Chebstortion));
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  float env = plugin_data->env;
  unsigned int count = plugin_data->count;
  
      unsigned int i;

      cd_lut[0][0] = 0.0f;
      cd_lut[0][1] = 1.0f;
      for (i=2; i<HARMONICS; i++) {
        cd_lut[0][i] = 0.0f;
      }
      cd_lut[1][0] = 0.0f;
      cd_lut[1][1] = 1.0f;
      for (i=2; i<HARMONICS; i++) {
        cd_lut[1][i] = 1.0f/(float)i;
      }

      itm1 = 0.0f;
      otm1 = 0.0f;
      env = 0.0f;
      count = 0;
    
  plugin_data->itm1 = itm1;
  plugin_data->otm1 = otm1;
  plugin_data->env = env;
  plugin_data->count = count;
  
  return (LV2_Handle)plugin_data;
}


static void activateChebstortion(LV2_Handle instance)
{
  Chebstortion *plugin_data = (Chebstortion *)instance;
  float itm1 __attribute__ ((unused)) = plugin_data->itm1;
  float otm1 __attribute__ ((unused)) = plugin_data->otm1;
  float env __attribute__ ((unused)) = plugin_data->env;
  unsigned int count __attribute__ ((unused)) = plugin_data->count;
  
      itm1 = 0.0f;
      otm1 = 0.0f;
      env = 0.0f;
      count = 0;
    
}


static void runChebstortion(LV2_Handle instance, uint32_t sample_count)
{
  Chebstortion *plugin_data = (Chebstortion *)instance;

  const float dist = *(plugin_data->dist);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  float env = plugin_data->env;
  unsigned int count = plugin_data->count;
  
      unsigned long pos, i;
      float p[HARMONICS], interp[HARMONICS];

      for (pos = 0; pos < sample_count; pos++) {
        const float x = input[pos];
        const float a = fabs(input[pos]);
	float y;

	if (a > env) {
		env = env * 0.9f + a * 0.1f;
	} else {
		env = env * 0.97f + a * 0.03f;
	}

	if (count-- == 0) {
	  for (i=0; i<HARMONICS; i++) {
	    interp[i] = cd_lut[0][i] * (1.0f - env * dist) +
                        cd_lut[1][i] * env * dist;
	  }
	  chebpc(interp, p);

	  count = 4;
	}

        // Evaluate the polynomial using Horner's Rule
	y = p[0] + (p[1] + (p[2] + (p[3] + (p[4] + (p[5] + (p[6] + (p[7] +
            (p[8] + (p[9] + p[10] * x) * x) * x) * x) * x) * x) * x) * x) *
            x) * x;

	// DC offset remove (odd harmonics cause DC offset)
        otm1 = 0.999f * otm1 + y - itm1;
        itm1 = y;

        buffer_write(output[pos], otm1);
      }

      plugin_data->itm1 = itm1;
      plugin_data->otm1 = otm1;
      plugin_data->env = env;
      plugin_data->count = count;
    
}

static const LV2_Descriptor chebstortionDescriptor = {
  "http://plugin.org.uk/swh-plugins/chebstortion",
  instantiateChebstortion,
  connectPortChebstortion,
  activateChebstortion,
  runChebstortion,
  NULL,
  cleanupChebstortion,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &chebstortionDescriptor;
  default:
    return NULL;
  }
}
