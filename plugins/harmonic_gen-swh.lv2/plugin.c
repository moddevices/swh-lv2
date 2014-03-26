
      #define HARMONICS 11

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

typedef struct _HarmonicGen {
  float *mag_1;
  float *mag_2;
  float *mag_3;
  float *mag_4;
  float *mag_5;
  float *mag_6;
  float *mag_7;
  float *mag_8;
  float *mag_9;
  float *mag_10;
  float *input;
  float *output;
float itm1;
float otm1;
} HarmonicGen;

static void cleanupHarmonicGen(LV2_Handle instance)
{

  free(instance);
}

static void connectPortHarmonicGen(LV2_Handle instance, uint32_t port, void *data)
{
  HarmonicGen *plugin = (HarmonicGen *)instance;

  switch (port) {
  case 0:
    plugin->mag_1 = data;
    break;
  case 1:
    plugin->mag_2 = data;
    break;
  case 2:
    plugin->mag_3 = data;
    break;
  case 3:
    plugin->mag_4 = data;
    break;
  case 4:
    plugin->mag_5 = data;
    break;
  case 5:
    plugin->mag_6 = data;
    break;
  case 6:
    plugin->mag_7 = data;
    break;
  case 7:
    plugin->mag_8 = data;
    break;
  case 8:
    plugin->mag_9 = data;
    break;
  case 9:
    plugin->mag_10 = data;
    break;
  case 10:
    plugin->input = data;
    break;
  case 11:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateHarmonicGen(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  HarmonicGen *plugin_data = (HarmonicGen *)malloc(sizeof(HarmonicGen));
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  
  plugin_data->itm1 = itm1;
  plugin_data->otm1 = otm1;
  
  return (LV2_Handle)plugin_data;
}


static void activateHarmonicGen(LV2_Handle instance)
{
  HarmonicGen *plugin_data = (HarmonicGen *)instance;
  float itm1 __attribute__ ((unused)) = plugin_data->itm1;
  float otm1 __attribute__ ((unused)) = plugin_data->otm1;
  
      itm1 = 0.0f;
      otm1 = 0.0f;
    
}


static void runHarmonicGen(LV2_Handle instance, uint32_t sample_count)
{
  HarmonicGen *plugin_data = (HarmonicGen *)instance;

  const float mag_1 = *(plugin_data->mag_1);
  const float mag_2 = *(plugin_data->mag_2);
  const float mag_3 = *(plugin_data->mag_3);
  const float mag_4 = *(plugin_data->mag_4);
  const float mag_5 = *(plugin_data->mag_5);
  const float mag_6 = *(plugin_data->mag_6);
  const float mag_7 = *(plugin_data->mag_7);
  const float mag_8 = *(plugin_data->mag_8);
  const float mag_9 = *(plugin_data->mag_9);
  const float mag_10 = *(plugin_data->mag_10);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  
      unsigned long pos, i;
      float mag_fix;
      float mag[HARMONICS] = {0.0f, mag_1, mag_2, mag_3, mag_4, mag_5, mag_6,
                              mag_7, mag_8, mag_9, mag_10};
      float p[HARMONICS];

      // Normalise magnitudes
      mag_fix = (fabs(mag_1) + fabs(mag_2) + fabs(mag_3) + fabs(mag_4) +
                 fabs(mag_5) + fabs(mag_6) + fabs(mag_7) + fabs(mag_8) +
                 fabs(mag_9) + fabs(mag_10));
      if (mag_fix < 1.0f) {
        mag_fix = 1.0f;
      } else {
        mag_fix = 1.0f / mag_fix;
      }
      for (i=0; i<HARMONICS; i++) {
        mag[i] *= mag_fix;
      }

      // Calculate polynomial coefficients, using Chebychev aproximation
      chebpc(mag, p);

      for (pos = 0; pos < sample_count; pos++) {
        float x = input[pos], y;

        // Calculate the polynomial using Horner's Rule
	y = p[0] + (p[1] + (p[2] + (p[3] + (p[4] + (p[5] + (p[6] + (p[7] +
            (p[8] + (p[9] + p[10] * x) * x) * x) * x) * x) * x) * x) * x) *
            x) * x;

	// DC offset remove (odd harmonics cause DC offset)
        otm1 = 0.999f * otm1 + y - itm1;
        itm1 = y;

        output[pos] = otm1;
      }

      plugin_data->itm1 = itm1;
      plugin_data->otm1 = otm1;
    
}

static const LV2_Descriptor harmonicGenDescriptor = {
  "http://plugin.org.uk/swh-plugins/harmonicGen",
  instantiateHarmonicGen,
  connectPortHarmonicGen,
  activateHarmonicGen,
  runHarmonicGen,
  NULL,
  cleanupHarmonicGen,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &harmonicGenDescriptor;
  default:
    return NULL;
  }
}
