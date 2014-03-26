
      #include "ladspa-util.h"
      #include "util/biquad.h"

      #define BANDS 3

      #define PEAK_BW	  0.3f /* Peak EQ bandwidth (octaves) */
      #define SHELF_SLOPE 1.5f /* Shelf EQ slope (arb. units) */
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Dj_eq_mono {
  float *lo;
  float *mid;
  float *hi;
  float *input;
  float *output;
  float *latency;
float fs;
biquad * filters;
} Dj_eq_mono;

static void cleanupDj_eq_mono(LV2_Handle instance)
{

  free(instance);
}

static void connectPortDj_eq_mono(LV2_Handle instance, uint32_t port, void *data)
{
  Dj_eq_mono *plugin = (Dj_eq_mono *)instance;

  switch (port) {
  case 0:
    plugin->lo = data;
    break;
  case 1:
    plugin->mid = data;
    break;
  case 2:
    plugin->hi = data;
    break;
  case 3:
    plugin->input = data;
    break;
  case 4:
    plugin->output = data;
    break;
  case 5:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiateDj_eq_mono(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Dj_eq_mono *plugin_data = (Dj_eq_mono *)malloc(sizeof(Dj_eq_mono));
  float fs = plugin_data->fs;
  biquad * filters = plugin_data->filters;
  
fs = s_rate;

filters = calloc(BANDS, sizeof(biquad));
    
  plugin_data->fs = fs;
  plugin_data->filters = filters;
  
  return (LV2_Handle)plugin_data;
}


static void activateDj_eq_mono(LV2_Handle instance)
{
  Dj_eq_mono *plugin_data = (Dj_eq_mono *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  biquad * filters __attribute__ ((unused)) = plugin_data->filters;
  
	biquad_init(&filters[0]);
	eq_set_params(&filters[0], 100.0f, 0.0f, PEAK_BW, fs);
	biquad_init(&filters[1]);
	eq_set_params(&filters[1], 1000.0f, 0.0f, PEAK_BW, fs);
	biquad_init(&filters[2]);
	hs_set_params(&filters[2], 10000.0f, 0.0f, SHELF_SLOPE, fs);
    
}


static void runDj_eq_mono(LV2_Handle instance, uint32_t sample_count)
{
  Dj_eq_mono *plugin_data = (Dj_eq_mono *)instance;

  const float lo = *(plugin_data->lo);
  const float mid = *(plugin_data->mid);
  const float hi = *(plugin_data->hi);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float latency;
  float fs = plugin_data->fs;
  biquad * filters = plugin_data->filters;
  
      unsigned long pos;
      float samp;

      eq_set_params(&filters[0], 100.0f, lo, PEAK_BW, fs);
      eq_set_params(&filters[1], 1000.0f, mid, PEAK_BW, fs);
      hs_set_params(&filters[2], 10000.0f, hi, SHELF_SLOPE, fs);

      for (pos = 0; pos < sample_count; pos++) {
	samp = biquad_run(&filters[0], input[pos]);
        samp = biquad_run(&filters[1], samp);
        samp = biquad_run(&filters[2], samp);
        buffer_write(output[pos], samp);
      }

      *(plugin_data->latency) = 3; //XXX is this right?
    
}

static const LV2_Descriptor dj_eq_monoDescriptor = {
  "http://plugin.org.uk/swh-plugins/dj_eq_mono",
  instantiateDj_eq_mono,
  connectPortDj_eq_mono,
  activateDj_eq_mono,
  runDj_eq_mono,
  NULL,
  cleanupDj_eq_mono,
  NULL
};

#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Dj_eq {
  float *lo;
  float *mid;
  float *hi;
  float *left_input;
  float *right_input;
  float *left_output;
  float *right_output;
  float *latency;
float fs;
biquad * filters;
} Dj_eq;

static void cleanupDj_eq(LV2_Handle instance)
{

  free(instance);
}

static void connectPortDj_eq(LV2_Handle instance, uint32_t port, void *data)
{
  Dj_eq *plugin = (Dj_eq *)instance;

  switch (port) {
  case 0:
    plugin->lo = data;
    break;
  case 1:
    plugin->mid = data;
    break;
  case 2:
    plugin->hi = data;
    break;
  case 3:
    plugin->left_input = data;
    break;
  case 4:
    plugin->right_input = data;
    break;
  case 5:
    plugin->left_output = data;
    break;
  case 6:
    plugin->right_output = data;
    break;
  case 7:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiateDj_eq(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Dj_eq *plugin_data = (Dj_eq *)malloc(sizeof(Dj_eq));
  float fs = plugin_data->fs;
  biquad * filters = plugin_data->filters;
  
fs = s_rate;

filters = calloc(BANDS * 2, sizeof(biquad));
    
  plugin_data->fs = fs;
  plugin_data->filters = filters;
  
  return (LV2_Handle)plugin_data;
}


static void activateDj_eq(LV2_Handle instance)
{
  Dj_eq *plugin_data = (Dj_eq *)instance;
  float fs __attribute__ ((unused)) = plugin_data->fs;
  biquad * filters __attribute__ ((unused)) = plugin_data->filters;
  
      int i;

      for (i=0; i<2; i++) {
	biquad_init(&filters[i*BANDS + 0]);
	eq_set_params(&filters[i*BANDS + 0], 100.0f, 0.0f, PEAK_BW, fs);
	biquad_init(&filters[i*BANDS + 1]);
	eq_set_params(&filters[i*BANDS + 1], 1000.0f, 0.0f, PEAK_BW, fs);
	biquad_init(&filters[i*BANDS + 2]);
	hs_set_params(&filters[i*BANDS + 2], 10000.0f, 0.0f, SHELF_SLOPE, fs);
      }
    
}


static void runDj_eq(LV2_Handle instance, uint32_t sample_count)
{
  Dj_eq *plugin_data = (Dj_eq *)instance;

  const float lo = *(plugin_data->lo);
  const float mid = *(plugin_data->mid);
  const float hi = *(plugin_data->hi);
  const float * const left_input = plugin_data->left_input;
  const float * const right_input = plugin_data->right_input;
  float * const left_output = plugin_data->left_output;
  float * const right_output = plugin_data->right_output;
  float latency;
  float fs = plugin_data->fs;
  biquad * filters = plugin_data->filters;
  
      unsigned long pos;
      unsigned int i;
      float samp;

      for (i=0; i<2; i++) {
        eq_set_params(&filters[i*BANDS + 0], 100.0f, lo, PEAK_BW, fs);
        eq_set_params(&filters[i*BANDS + 1], 1000.0f, mid, PEAK_BW, fs);
        hs_set_params(&filters[i*BANDS + 2], 10000.0f, hi, SHELF_SLOPE, fs);
      }

      for (pos = 0; pos < sample_count; pos++) {
	samp = biquad_run(&filters[0], left_input[pos]);
        samp = biquad_run(&filters[1], samp);
        samp = biquad_run(&filters[2], samp);
        buffer_write(left_output[pos], samp);

	samp = biquad_run(&filters[3], right_input[pos]);
        samp = biquad_run(&filters[4], samp);
        samp = biquad_run(&filters[5], samp);
        buffer_write(right_output[pos], samp);
      }

      *(plugin_data->latency) = 3; //XXX is this right?
    
}

static const LV2_Descriptor dj_eqDescriptor = {
  "http://plugin.org.uk/swh-plugins/dj_eq",
  instantiateDj_eq,
  connectPortDj_eq,
  activateDj_eq,
  runDj_eq,
  NULL,
  cleanupDj_eq,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &dj_eq_monoDescriptor;
  case 1:
    return &dj_eqDescriptor;
  default:
    return NULL;
  }
}
