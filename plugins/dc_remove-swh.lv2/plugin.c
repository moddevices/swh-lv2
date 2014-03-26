
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _DcRemove {
  float *input;
  float *output;
float itm1;
float otm1;
} DcRemove;

static void cleanupDcRemove(LV2_Handle instance)
{

  free(instance);
}

static void connectPortDcRemove(LV2_Handle instance, uint32_t port, void *data)
{
  DcRemove *plugin = (DcRemove *)instance;

  switch (port) {
  case 0:
    plugin->input = data;
    break;
  case 1:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateDcRemove(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  DcRemove *plugin_data = (DcRemove *)malloc(sizeof(DcRemove));
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  
  plugin_data->itm1 = itm1;
  plugin_data->otm1 = otm1;
  
  return (LV2_Handle)plugin_data;
}


static void activateDcRemove(LV2_Handle instance)
{
  DcRemove *plugin_data = (DcRemove *)instance;
  float itm1 __attribute__ ((unused)) = plugin_data->itm1;
  float otm1 __attribute__ ((unused)) = plugin_data->otm1;
  
      itm1 = 0.0f;
      otm1 = 0.0f;
    
}


static void runDcRemove(LV2_Handle instance, uint32_t sample_count)
{
  DcRemove *plugin_data = (DcRemove *)instance;

  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float itm1 = plugin_data->itm1;
  float otm1 = plugin_data->otm1;
  
      unsigned long pos;

      for (pos = 0; pos < sample_count; pos++) {
        otm1 = 0.999f * otm1 + input[pos] - itm1;
	itm1 = input[pos];
        output[pos] = otm1;
      }

      plugin_data->itm1 = itm1;
      plugin_data->otm1 = otm1;
    
}

static const LV2_Descriptor dcRemoveDescriptor = {
  "http://plugin.org.uk/swh-plugins/dcRemove",
  instantiateDcRemove,
  connectPortDcRemove,
  activateDcRemove,
  runDcRemove,
  NULL,
  cleanupDcRemove,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &dcRemoveDescriptor;
  default:
    return NULL;
  }
}
