
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _MatrixStMS {
  float *left;
  float *right;
  float *mid;
  float *side;
} MatrixStMS;

static void cleanupMatrixStMS(LV2_Handle instance)
{

  free(instance);
}

static void connectPortMatrixStMS(LV2_Handle instance, uint32_t port, void *data)
{
  MatrixStMS *plugin = (MatrixStMS *)instance;

  switch (port) {
  case 0:
    plugin->left = data;
    break;
  case 1:
    plugin->right = data;
    break;
  case 2:
    plugin->mid = data;
    break;
  case 3:
    plugin->side = data;
    break;
  }
}

static LV2_Handle instantiateMatrixStMS(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  MatrixStMS *plugin_data = (MatrixStMS *)malloc(sizeof(MatrixStMS));
  
  
  return (LV2_Handle)plugin_data;
}



static void runMatrixStMS(LV2_Handle instance, uint32_t sample_count)
{
  MatrixStMS *plugin_data = (MatrixStMS *)instance;

  const float * const left = plugin_data->left;
  const float * const right = plugin_data->right;
  float * const mid = plugin_data->mid;
  float * const side = plugin_data->side;
  
      unsigned long pos;

      for (pos = 0; pos < sample_count; pos++) {
        mid[pos] = (left[pos] + right[pos]) * 0.5;
        side[pos] = (left[pos] - right[pos]) * 0.5;
      }
    
}

static const LV2_Descriptor matrixStMSDescriptor = {
  "http://plugin.org.uk/swh-plugins/matrixStMS",
  instantiateMatrixStMS,
  connectPortMatrixStMS,
  NULL,
  runMatrixStMS,
  NULL,
  cleanupMatrixStMS,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &matrixStMSDescriptor;
  default:
    return NULL;
  }
}
