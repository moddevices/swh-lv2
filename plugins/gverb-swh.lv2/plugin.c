
/*

GVerb algorithm designed and implemented by Juhana Sadeharju.
LADSPA implementation and GVerb speeds ups by Steve Harris.

Comments and suggestions should be mailed to Juhana Sadeharju
(kouhia at nic funet fi).

*/

#include "ladspa-util.h"
#include "gverb/gverbdsp.h"
#include "gverb/gverb.h"
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Gverb {
  float *roomsize;
  float *revtime;
  float *damping;
  float *inputbandwidth;
  float *drylevel;
  float *earlylevel;
  float *taillevel;
  float *input;
  float *outl;
  float *outr;
ty_gverb * verb;
} Gverb;

static void cleanupGverb(LV2_Handle instance)
{
Gverb *plugin_data = (Gverb *)instance;

      gverb_free(plugin_data->verb);
    
  free(instance);
}

static void connectPortGverb(LV2_Handle instance, uint32_t port, void *data)
{
  Gverb *plugin = (Gverb *)instance;

  switch (port) {
  case 0:
    plugin->roomsize = data;
    break;
  case 1:
    plugin->revtime = data;
    break;
  case 2:
    plugin->damping = data;
    break;
  case 3:
    plugin->inputbandwidth = data;
    break;
  case 4:
    plugin->drylevel = data;
    break;
  case 5:
    plugin->earlylevel = data;
    break;
  case 6:
    plugin->taillevel = data;
    break;
  case 7:
    plugin->input = data;
    break;
  case 8:
    plugin->outl = data;
    break;
  case 9:
    plugin->outr = data;
    break;
  }
}

static LV2_Handle instantiateGverb(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Gverb *plugin_data = (Gverb *)malloc(sizeof(Gverb));
  ty_gverb * verb = plugin_data->verb;
  
      verb = gverb_new(s_rate, 300.0f, 50.0f, 7.0f, 0.5f, 15.0f, 0.5f, 0.5f, 0.5f);
    
  plugin_data->verb = verb;
  
  return (LV2_Handle)plugin_data;
}


static void activateGverb(LV2_Handle instance)
{
  Gverb *plugin_data = (Gverb *)instance;
  ty_gverb * verb __attribute__ ((unused)) = plugin_data->verb;
  
      gverb_flush(plugin_data->verb);
    
}


static void runGverb(LV2_Handle instance, uint32_t sample_count)
{
  Gverb *plugin_data = (Gverb *)instance;

  const float roomsize = *(plugin_data->roomsize);
  const float revtime = *(plugin_data->revtime);
  const float damping = *(plugin_data->damping);
  const float inputbandwidth = *(plugin_data->inputbandwidth);
  const float drylevel = *(plugin_data->drylevel);
  const float earlylevel = *(plugin_data->earlylevel);
  const float taillevel = *(plugin_data->taillevel);
  const float * const input = plugin_data->input;
  float * const outl = plugin_data->outl;
  float * const outr = plugin_data->outr;
  ty_gverb * verb = plugin_data->verb;
  
      unsigned long pos;
      float l, r;
      float dryc = DB_CO(drylevel);

      gverb_set_roomsize(verb, roomsize);
      gverb_set_revtime(verb, revtime);
      gverb_set_damping(verb, damping);
      gverb_set_inputbandwidth(verb, inputbandwidth);
      gverb_set_earlylevel(verb, DB_CO(earlylevel));
      gverb_set_taillevel(verb, DB_CO(taillevel));

      for (pos = 0; pos < sample_count; pos++) {
        gverb_do(verb, input[pos], &l, &r);
        buffer_write(outl[pos], l + input[pos] * dryc);
        buffer_write(outr[pos], r + input[pos] * dryc);
      }
    
}

static const LV2_Descriptor gverbDescriptor = {
  "http://plugin.org.uk/swh-plugins/gverb",
  instantiateGverb,
  connectPortGverb,
  activateGverb,
  runGverb,
  NULL,
  cleanupGverb,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &gverbDescriptor;
  default:
    return NULL;
  }
}
