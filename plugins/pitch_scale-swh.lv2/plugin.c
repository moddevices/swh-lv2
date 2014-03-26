
			#include "util/pitchscale.h"

			#define FRAME_LENGTH 4096
			#define OVER_SAMP    16
		
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _PitchScaleHQ {
  float *mult;
  float *input;
  float *output;
  float *latency;
sbuffers * buffers;
long sample_rate;
} PitchScaleHQ;

static void cleanupPitchScaleHQ(LV2_Handle instance)
{
PitchScaleHQ *plugin_data = (PitchScaleHQ *)instance;

                        free (plugin_data->buffers->gInFIFO);
                        free (plugin_data->buffers->gOutFIFO);
                        free (plugin_data->buffers->gLastPhase);
                        free (plugin_data->buffers->gSumPhase);
                        free (plugin_data->buffers->gOutputAccum);
                        free (plugin_data->buffers->gAnaFreq);
                        free (plugin_data->buffers->gAnaMagn);
                        free (plugin_data->buffers->gSynFreq);
                        free (plugin_data->buffers->gSynMagn);
                        free (plugin_data->buffers->gWindow);
                        free (plugin_data->buffers);
		
  free(instance);
}

static void connectPortPitchScaleHQ(LV2_Handle instance, uint32_t port, void *data)
{
  PitchScaleHQ *plugin = (PitchScaleHQ *)instance;

  switch (port) {
  case 0:
    plugin->mult = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  case 3:
    plugin->latency = data;
    break;
  }
}

static LV2_Handle instantiatePitchScaleHQ(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  PitchScaleHQ *plugin_data = (PitchScaleHQ *)malloc(sizeof(PitchScaleHQ));
  sbuffers * buffers = plugin_data->buffers;
  long sample_rate = plugin_data->sample_rate;
  
			int i;
			float arg;

			buffers = malloc(sizeof(sbuffers));
			sample_rate = s_rate;
			buffers->gInFIFO = malloc(FRAME_LENGTH * sizeof(float));
			buffers->gOutFIFO = malloc(FRAME_LENGTH * sizeof(float));
			buffers->gLastPhase = malloc(FRAME_LENGTH * sizeof(float));
			buffers->gSumPhase = malloc(FRAME_LENGTH * sizeof(float));
			buffers->gOutputAccum = malloc(2*FRAME_LENGTH * sizeof(float));
			buffers->gAnaFreq = malloc(FRAME_LENGTH * sizeof(float));
			buffers->gAnaMagn = malloc(FRAME_LENGTH * sizeof(float));
			buffers->gSynFreq = malloc(FRAME_LENGTH * sizeof(float));
			buffers->gSynMagn = malloc(FRAME_LENGTH * sizeof(float));
			buffers->gWindow = malloc(FRAME_LENGTH * sizeof(float));

			arg = 2.0f * M_PI / (float)(FRAME_LENGTH-1);
			for (i=0; i < FRAME_LENGTH; i++) {
				// Blackman-Harris
				buffers->gWindow[i] =  0.35875f - 0.48829f * cos(arg * (float)i) + 0.14128f * cos(2.0f * arg * (float)i) - 0.01168f * cos(3.0f * arg * (float)i);
				// Gain correction
				buffers->gWindow[i] *= 0.761f;

			}

		
  plugin_data->buffers = buffers;
  plugin_data->sample_rate = sample_rate;
  
  return (LV2_Handle)plugin_data;
}


static void activatePitchScaleHQ(LV2_Handle instance)
{
  PitchScaleHQ *plugin_data = (PitchScaleHQ *)instance;
  sbuffers * buffers __attribute__ ((unused)) = plugin_data->buffers;
  long sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  
			memset(buffers->gInFIFO, 0, FRAME_LENGTH*sizeof(float));
			memset(buffers->gOutFIFO, 0, FRAME_LENGTH*sizeof(float));
			memset(buffers->gLastPhase, 0, FRAME_LENGTH*sizeof(float)/2);
			memset(buffers->gSumPhase, 0, FRAME_LENGTH*sizeof(float)/2);
			memset(buffers->gOutputAccum, 0, 2*FRAME_LENGTH*sizeof(float));
			memset(buffers->gAnaFreq, 0, FRAME_LENGTH*sizeof(float));
			memset(buffers->gAnaMagn, 0, FRAME_LENGTH*sizeof(float));
			buffers->gRover = 0;
			pitch_scale(buffers, 1.0, FRAME_LENGTH, 16, FRAME_LENGTH, sample_rate, buffers->gInFIFO, buffers->gOutFIFO, 0, 0.0f);
		
}


static void runPitchScaleHQ(LV2_Handle instance, uint32_t sample_count)
{
  PitchScaleHQ *plugin_data = (PitchScaleHQ *)instance;

  const float mult = *(plugin_data->mult);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float latency;
  sbuffers * buffers = plugin_data->buffers;
  long sample_rate = plugin_data->sample_rate;
  
			pitch_scale(buffers, mult, FRAME_LENGTH, OVER_SAMP, sample_count, sample_rate, input, output, 0, 0.0);
			*(plugin_data->latency) = FRAME_LENGTH - (FRAME_LENGTH
							/ OVER_SAMP);
		
}

static const LV2_Descriptor pitchScaleHQDescriptor = {
  "http://plugin.org.uk/swh-plugins/pitchScaleHQ",
  instantiatePitchScaleHQ,
  connectPortPitchScaleHQ,
  activatePitchScaleHQ,
  runPitchScaleHQ,
  NULL,
  cleanupPitchScaleHQ,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &pitchScaleHQDescriptor;
  default:
    return NULL;
  }
}
