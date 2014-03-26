
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _MatrixMSSt {
  float *width;
  float *mid;
  float *side;
  float *left;
  float *right;
} MatrixMSSt;

static void cleanupMatrixMSSt(LV2_Handle instance)
{

  free(instance);
}

static void connectPortMatrixMSSt(LV2_Handle instance, uint32_t port, void *data)
{
  MatrixMSSt *plugin = (MatrixMSSt *)instance;

  switch (port) {
  case 0:
    plugin->width = data;
    break;
  case 1:
    plugin->mid = data;
    break;
  case 2:
    plugin->side = data;
    break;
  case 3:
    plugin->left = data;
    break;
  case 4:
    plugin->right = data;
    break;
  }
}

static LV2_Handle instantiateMatrixMSSt(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  MatrixMSSt *plugin_data = (MatrixMSSt *)malloc(sizeof(MatrixMSSt));
  
  
  return (LV2_Handle)plugin_data;
}



static void runMatrixMSSt(LV2_Handle instance, uint32_t sample_count)
{
  MatrixMSSt *plugin_data = (MatrixMSSt *)instance;

  const float width = *(plugin_data->width);
  const float * const mid = plugin_data->mid;
  const float * const side = plugin_data->side;
  float * const left = plugin_data->left;
  float * const right = plugin_data->right;
  
      unsigned long pos;

      for (pos = 0; pos < sample_count; pos++) {
        left[pos] = mid[pos] + side[pos] * width;
        right[pos] = mid[pos] - side[pos] * width;
      }
    
}

static const LV2_Descriptor matrixMSStDescriptor = {
  "http://plugin.org.uk/swh-plugins/matrixMSSt",
  instantiateMatrixMSSt,
  connectPortMatrixMSSt,
  NULL,
  runMatrixMSSt,
  NULL,
  cleanupMatrixMSSt,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &matrixMSStDescriptor;
  default:
    return NULL;
  }
}
