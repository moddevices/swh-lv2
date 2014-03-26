
      #include "ladspa-util.h"

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

typedef struct _SurroundEncoder {
  float *l;
  float *r;
  float *c;
  float *s;
  float *lt;
  float *rt;
unsigned int buffer_size;
unsigned int buffer_pos;
float * buffer;
float * delay;
unsigned int dptr;
} SurroundEncoder;

static void cleanupSurroundEncoder(LV2_Handle instance)
{
SurroundEncoder *plugin_data = (SurroundEncoder *)instance;

      free(plugin_data->buffer);
      
      free(plugin_data->delay);
    
  free(instance);
}

static void connectPortSurroundEncoder(LV2_Handle instance, uint32_t port, void *data)
{
  SurroundEncoder *plugin = (SurroundEncoder *)instance;

  switch (port) {
  case 0:
    plugin->l = data;
    break;
  case 1:
    plugin->r = data;
    break;
  case 2:
    plugin->c = data;
    break;
  case 3:
    plugin->s = data;
    break;
  case 4:
    plugin->lt = data;
    break;
  case 5:
    plugin->rt = data;
    break;
  }
}

static LV2_Handle instantiateSurroundEncoder(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  SurroundEncoder *plugin_data = (SurroundEncoder *)malloc(sizeof(SurroundEncoder));
  unsigned int buffer_size = plugin_data->buffer_size;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  float * buffer = plugin_data->buffer;
  float * delay = plugin_data->delay;
  unsigned int dptr = plugin_data->dptr;
  
      buffer_size = (int)(0.0072f * s_rate);
      buffer_pos = 0;
      buffer = calloc(buffer_size, sizeof(float));
      
      delay = calloc(D_SIZE, sizeof(float));

      dptr = 0;
    
  plugin_data->buffer_size = buffer_size;
  plugin_data->buffer_pos = buffer_pos;
  plugin_data->buffer = buffer;
  plugin_data->delay = delay;
  plugin_data->dptr = dptr;
  
  return (LV2_Handle)plugin_data;
}


static void activateSurroundEncoder(LV2_Handle instance)
{
  SurroundEncoder *plugin_data = (SurroundEncoder *)instance;
  unsigned int buffer_size __attribute__ ((unused)) = plugin_data->buffer_size;
  unsigned int buffer_pos __attribute__ ((unused)) = plugin_data->buffer_pos;
  float * buffer __attribute__ ((unused)) = plugin_data->buffer;
  float * delay __attribute__ ((unused)) = plugin_data->delay;
  unsigned int dptr __attribute__ ((unused)) = plugin_data->dptr;
  
      memset(buffer, 0, buffer_size * sizeof(float));
    
}


static void runSurroundEncoder(LV2_Handle instance, uint32_t sample_count)
{
  SurroundEncoder *plugin_data = (SurroundEncoder *)instance;

  const float * const l = plugin_data->l;
  const float * const r = plugin_data->r;
  const float * const c = plugin_data->c;
  const float * const s = plugin_data->s;
  float * const lt = plugin_data->lt;
  float * const rt = plugin_data->rt;
  unsigned int buffer_size = plugin_data->buffer_size;
  unsigned int buffer_pos = plugin_data->buffer_pos;
  float * buffer = plugin_data->buffer;
  float * delay = plugin_data->delay;
  unsigned int dptr = plugin_data->dptr;
  
      unsigned long pos;
      float s_delayed;
      unsigned int i;
      float hilb;

      for (pos = 0; pos < sample_count; pos++) {
        delay[dptr] = s[pos];
	hilb = 0.0f;
	for (i = 0; i <= NZEROS/2; i++) {
	  hilb += (xcoeffs[i] * delay[(dptr - i*2) & (D_SIZE - 1)]);
	}
	dptr = (dptr + 1) & (D_SIZE - 1);
      
      
        s_delayed = buffer[buffer_pos];
	buffer[buffer_pos++] = hilb;
	buffer_pos %= buffer_size;

        buffer_write(lt[pos], l[pos] + c[pos] * 0.707946f -
                     s_delayed * 0.707946f);
        buffer_write(rt[pos], r[pos] + c[pos] * 0.707946f +
                     s_delayed * 0.707946f);
      }
      
      plugin_data->dptr = dptr;

      plugin_data->buffer_pos = buffer_pos;
    
}

static const LV2_Descriptor surroundEncoderDescriptor = {
  "http://plugin.org.uk/swh-plugins/surroundEncoder",
  instantiateSurroundEncoder,
  connectPortSurroundEncoder,
  activateSurroundEncoder,
  runSurroundEncoder,
  NULL,
  cleanupSurroundEncoder,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &surroundEncoderDescriptor;
  default:
    return NULL;
  }
}
