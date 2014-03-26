
/*
 * Copyright (c) 1996, John S. Dyson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * This code (easily) runs realtime on a P5-166 w/EDO, Triton-II on FreeBSD.
 *
 * More info/comments: dyson@freebsd.org
 *
 * This program provides compression of a stereo 16bit audio stream,
 * such as that contained by a 16Bit wav file.  Extreme measures have
 * been taken to make the compression as subtile as possible.  One
 * possible purpose for this code would be to master cassette tapes from
 * CD's for playback in automobiles where dynamic range needs to be
 * restricted.
 *
 * Suitably recoded for an embedded DSP, this would make a killer audio
 * compressor for broadcast or recording.  When writing this code, I
 * ignored the issues of roundoff error or trucation -- Pentiums have
 * really nice FP processors :-).
 */

      #include <ladspa-util.h>

      #define MAXLEVEL 0.9f
      #define NFILT 12
      #define NEFILT 17

      /* These filters should filter at least the lowest audio freq */
      #define RLEVELSQ0FILTER .001
      #define RLEVELSQ1FILTER .010
      /* These are the attack time for the rms measurement */
      #define RLEVELSQ0FFILTER .001
      #define RLEVELSQEFILTER .001
    
      #define RMASTERGAIN0FILTER .000003
    
      #define RPEAKGAINFILTER .001

      #define MAXFASTGAIN 3
      #define MAXSLOWGAIN 9

      #define FLOORLEVEL 0.06

      float hardlimit(float value, float knee, float limit) {
        float ab = fabs(value);
        if (ab >= limit) {
          value = value > 0 ? limit : -limit;
        }

        return value;
      }
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _DysonCompress {
  float *peak_limit;
  float *release_time;
  float *cfrate;
  float *crate;
  float *input;
  float *output;
float sample_rate;
float mingain;
float maxgain;
float rpeaklimitdelay;
float rgain;
float rlevelsq0;
float rlevelsq1;
float ndelay;
float * delay;
float * rlevelsqn;
float * rlevelsqe;
float rmastergain0;
float rpeakgain0;
float rpeakgain1;
int peaklimitdelay;
unsigned int ndelayptr;
float lastrgain;
float extra_maxlevel;
} DysonCompress;

static void cleanupDysonCompress(LV2_Handle instance)
{
DysonCompress *plugin_data = (DysonCompress *)instance;

      free(plugin_data->delay);
      free(plugin_data->rlevelsqn);
      free(plugin_data->rlevelsqe);
    
  free(instance);
}

static void connectPortDysonCompress(LV2_Handle instance, uint32_t port, void *data)
{
  DysonCompress *plugin = (DysonCompress *)instance;

  switch (port) {
  case 0:
    plugin->peak_limit = data;
    break;
  case 1:
    plugin->release_time = data;
    break;
  case 2:
    plugin->cfrate = data;
    break;
  case 3:
    plugin->crate = data;
    break;
  case 4:
    plugin->input = data;
    break;
  case 5:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateDysonCompress(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  DysonCompress *plugin_data = (DysonCompress *)malloc(sizeof(DysonCompress));
  float sample_rate = plugin_data->sample_rate;
  float mingain = plugin_data->mingain;
  float maxgain = plugin_data->maxgain;
  float rpeaklimitdelay = plugin_data->rpeaklimitdelay;
  float rgain = plugin_data->rgain;
  float rlevelsq0 = plugin_data->rlevelsq0;
  float rlevelsq1 = plugin_data->rlevelsq1;
  float ndelay = plugin_data->ndelay;
  float * delay = plugin_data->delay;
  float * rlevelsqn = plugin_data->rlevelsqn;
  float * rlevelsqe = plugin_data->rlevelsqe;
  float rmastergain0 = plugin_data->rmastergain0;
  float rpeakgain0 = plugin_data->rpeakgain0;
  float rpeakgain1 = plugin_data->rpeakgain1;
  int peaklimitdelay = plugin_data->peaklimitdelay;
  unsigned int ndelayptr = plugin_data->ndelayptr;
  float lastrgain = plugin_data->lastrgain;
  float extra_maxlevel = plugin_data->extra_maxlevel;
  
      sample_rate = (float)s_rate;

      mingain = 10000;
      maxgain = 0;

      rpeaklimitdelay = 2500;
    
      rgain = rmastergain0 = 1.0;
      rlevelsq0 = 0;
      rlevelsq1 = 0;
      ndelay = (int)(1.0 / RLEVELSQ0FFILTER);

      delay = calloc(ndelay, sizeof(float));
      rlevelsqn = calloc(NFILT + 1, sizeof(float));
      rlevelsqe = calloc(NEFILT + 1, sizeof(float));
    
      rpeakgain0 = 1.0;
      rpeakgain1 = 1.0;
      rpeaklimitdelay = 0;
      ndelayptr = 0;
      lastrgain = 1.0;

      extra_maxlevel = 0.0f;
      peaklimitdelay = 0;
    
  plugin_data->sample_rate = sample_rate;
  plugin_data->mingain = mingain;
  plugin_data->maxgain = maxgain;
  plugin_data->rpeaklimitdelay = rpeaklimitdelay;
  plugin_data->rgain = rgain;
  plugin_data->rlevelsq0 = rlevelsq0;
  plugin_data->rlevelsq1 = rlevelsq1;
  plugin_data->ndelay = ndelay;
  plugin_data->delay = delay;
  plugin_data->rlevelsqn = rlevelsqn;
  plugin_data->rlevelsqe = rlevelsqe;
  plugin_data->rmastergain0 = rmastergain0;
  plugin_data->rpeakgain0 = rpeakgain0;
  plugin_data->rpeakgain1 = rpeakgain1;
  plugin_data->peaklimitdelay = peaklimitdelay;
  plugin_data->ndelayptr = ndelayptr;
  plugin_data->lastrgain = lastrgain;
  plugin_data->extra_maxlevel = extra_maxlevel;
  
  return (LV2_Handle)plugin_data;
}


static void activateDysonCompress(LV2_Handle instance)
{
  DysonCompress *plugin_data = (DysonCompress *)instance;
  float sample_rate __attribute__ ((unused)) = plugin_data->sample_rate;
  float mingain __attribute__ ((unused)) = plugin_data->mingain;
  float maxgain __attribute__ ((unused)) = plugin_data->maxgain;
  float rpeaklimitdelay __attribute__ ((unused)) = plugin_data->rpeaklimitdelay;
  float rgain __attribute__ ((unused)) = plugin_data->rgain;
  float rlevelsq0 __attribute__ ((unused)) = plugin_data->rlevelsq0;
  float rlevelsq1 __attribute__ ((unused)) = plugin_data->rlevelsq1;
  float ndelay __attribute__ ((unused)) = plugin_data->ndelay;
  float * delay __attribute__ ((unused)) = plugin_data->delay;
  float * rlevelsqn __attribute__ ((unused)) = plugin_data->rlevelsqn;
  float * rlevelsqe __attribute__ ((unused)) = plugin_data->rlevelsqe;
  float rmastergain0 __attribute__ ((unused)) = plugin_data->rmastergain0;
  float rpeakgain0 __attribute__ ((unused)) = plugin_data->rpeakgain0;
  float rpeakgain1 __attribute__ ((unused)) = plugin_data->rpeakgain1;
  int peaklimitdelay __attribute__ ((unused)) = plugin_data->peaklimitdelay;
  unsigned int ndelayptr __attribute__ ((unused)) = plugin_data->ndelayptr;
  float lastrgain __attribute__ ((unused)) = plugin_data->lastrgain;
  float extra_maxlevel __attribute__ ((unused)) = plugin_data->extra_maxlevel;
  
      unsigned int i;

      for (i=0; i<ndelay; i++) {
        delay[i] = 0;
      }
      for (i=0; i<NFILT + 1; i++) {
        rlevelsqn[i] = 0;
      }
      for (i=0; i<NEFILT + 1; i++) {
        rlevelsqe[i] = 0;
      }

      mingain = 10000;
      maxgain = 0;

      rpeaklimitdelay = 2500;
    
      rgain = rmastergain0 = 1.0;
      rlevelsq0 = 0;
      rlevelsq1 = 0;
    
      rpeakgain0 = 1.0;
      rpeakgain1 = 1.0;
      rpeaklimitdelay = 0;
      ndelayptr = 0;
      lastrgain = 1.0;

      extra_maxlevel = 0.0f;
      peaklimitdelay = 0;
    
}


static void runDysonCompress(LV2_Handle instance, uint32_t sample_count)
{
  DysonCompress *plugin_data = (DysonCompress *)instance;

  const float peak_limit = *(plugin_data->peak_limit);
  const float release_time = *(plugin_data->release_time);
  const float cfrate = *(plugin_data->cfrate);
  const float crate = *(plugin_data->crate);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  float sample_rate = plugin_data->sample_rate;
  float mingain = plugin_data->mingain;
  float maxgain = plugin_data->maxgain;
  float rpeaklimitdelay = plugin_data->rpeaklimitdelay;
  float rgain = plugin_data->rgain;
  float rlevelsq0 = plugin_data->rlevelsq0;
  float rlevelsq1 = plugin_data->rlevelsq1;
  float ndelay = plugin_data->ndelay;
  float * delay = plugin_data->delay;
  float * rlevelsqn = plugin_data->rlevelsqn;
  float * rlevelsqe = plugin_data->rlevelsqe;
  float rmastergain0 = plugin_data->rmastergain0;
  float rpeakgain0 = plugin_data->rpeakgain0;
  float rpeakgain1 = plugin_data->rpeakgain1;
  int peaklimitdelay = plugin_data->peaklimitdelay;
  unsigned int ndelayptr = plugin_data->ndelayptr;
  float lastrgain = plugin_data->lastrgain;
  float extra_maxlevel = plugin_data->extra_maxlevel;
  
      unsigned long pos;
      float targetlevel = MAXLEVEL * DB_CO(peak_limit);
      float rgainfilter = 1.0f / (release_time * sample_rate);
      float fastgaincompressionratio = cfrate;
      float compressionratio = crate;
      float efilt;
      float levelsqe;
      float gain;
      float tgain;
      float d;
      float fastgain;
      float qgain;
      float tslowgain;
      float slowgain;
      float npeakgain;
      float new;
      float nrgain;
      float ngain;
      float ngsq;
      float tnrgain;
      float sqrtrpeakgain;
      float totalgain;
      unsigned int i;

      for (pos = 0; pos < sample_count; pos++) {
        // Ergh! this was originally meant to track a stereo signal
        float levelsq0 = 2.0f * (input[pos] * input[pos]);

        delay[ndelayptr] = input[pos];
        ndelayptr++;

        if (ndelayptr >= ndelay) {
          ndelayptr = 0;
        }

        if (levelsq0 > rlevelsq0) {
          rlevelsq0 = (levelsq0 * RLEVELSQ0FFILTER) +
           rlevelsq0 * (1 - RLEVELSQ0FFILTER);
        } else {
          rlevelsq0 = (levelsq0 * RLEVELSQ0FILTER) +
           rlevelsq0 * (1 - RLEVELSQ0FILTER);
        }

        if (rlevelsq0 <= FLOORLEVEL * FLOORLEVEL) {
          goto skipagc;
        }

        if (rlevelsq0 > rlevelsq1) {
          rlevelsq1 = rlevelsq0;
        } else {
          rlevelsq1 = rlevelsq0 * RLEVELSQ1FILTER +
            rlevelsq1 * (1 - RLEVELSQ1FILTER);
        }

        rlevelsqn[0] = rlevelsq1;
        for(i = 0; i < NFILT-1; i++) {
          if (rlevelsqn[i] > rlevelsqn[i+1])
            rlevelsqn[i+1] = rlevelsqn[i];
          else
            rlevelsqn[i+1] = rlevelsqn[i] * RLEVELSQ1FILTER +
              rlevelsqn[i+1] * (1 - RLEVELSQ1FILTER);
        }

        efilt = RLEVELSQEFILTER;
        levelsqe = rlevelsqe[0] = rlevelsqn[NFILT-1];
        for(i = 0; i < NEFILT-1; i++) {
          rlevelsqe[i+1] = rlevelsqe[i] * efilt +
            rlevelsqe[i+1] * (1.0 - efilt);
          if (rlevelsqe[i+1] > levelsqe)
            levelsqe = rlevelsqe[i+1];
          efilt *= 1.0f / 1.5f;
        }

        gain = targetlevel / sqrt(levelsqe);
        if (compressionratio < 0.99f) {
          if (compressionratio == 0.50f)
            gain = sqrt(gain);
          else
            gain = f_exp(log(gain) * compressionratio);
        }

        if (gain < rgain)
          rgain = gain * RLEVELSQEFILTER/2 +
            rgain * (1 - RLEVELSQEFILTER/2);
        else
          rgain = gain * rgainfilter +
            rgain * (1 - rgainfilter);

        lastrgain = rgain;
        if ( gain < lastrgain)
          lastrgain = gain;

      skipagc:;

        tgain = lastrgain;
    
        d = delay[ndelayptr];
    
        fastgain = tgain;
        if (fastgain > MAXFASTGAIN)
          fastgain = MAXFASTGAIN;
    
        if (fastgain < 0.0001)
          fastgain = 0.0001;

        qgain = f_exp(log(fastgain) * fastgaincompressionratio);

        tslowgain = tgain / qgain;
        if (tslowgain > MAXSLOWGAIN)
          tslowgain = MAXSLOWGAIN;
        if (tslowgain < rmastergain0)
          rmastergain0 = tslowgain;
        else
          rmastergain0 = tslowgain * RMASTERGAIN0FILTER +
            (1 - RMASTERGAIN0FILTER) * rmastergain0;
    
        slowgain = rmastergain0;
        npeakgain = slowgain * qgain;
    
        new = d * npeakgain;
        if (fabs(new) >= MAXLEVEL)
          nrgain = MAXLEVEL / fabs(new);
        else
          nrgain = 1.0;
    
        ngain = nrgain;
    
        ngsq = ngain * ngain;
        if (ngsq <= rpeakgain0) {
          rpeakgain0 = ngsq /* * 0.50 + rpeakgain0 * 0.50 */;
          rpeaklimitdelay = peaklimitdelay;
        } else if (rpeaklimitdelay == 0) {
          if (nrgain > 1.0)
            tnrgain = 1.0;
          else
            tnrgain = nrgain;
          rpeakgain0 = tnrgain * RPEAKGAINFILTER +
            (1.0 - RPEAKGAINFILTER) * rpeakgain0;
        }

        if (rpeakgain0 <= rpeakgain1) {
          rpeakgain1 = rpeakgain0;
          rpeaklimitdelay = peaklimitdelay;
        } else if (rpeaklimitdelay == 0) {
          rpeakgain1 = RPEAKGAINFILTER * rpeakgain0 +
            (1.0 - RPEAKGAINFILTER) * rpeakgain1;
        } else {
          --rpeaklimitdelay;
        }

        sqrtrpeakgain = sqrt(rpeakgain1);
        totalgain = npeakgain * sqrtrpeakgain;
    
        buffer_write(output[pos], new * sqrtrpeakgain);
    
        if (totalgain > maxgain)
          maxgain = totalgain;
        if (totalgain < mingain)
          mingain = totalgain;
        if (output[pos] > extra_maxlevel)
          extra_maxlevel = output[pos];
      }

      plugin_data->ndelayptr = ndelayptr;
      plugin_data->rlevelsq0 = rlevelsq0;
      plugin_data->rlevelsq1 = rlevelsq1;
      plugin_data->mingain = mingain;
      plugin_data->maxgain = maxgain;
      plugin_data->rpeaklimitdelay = rpeaklimitdelay;
      plugin_data->rgain = rgain;
      plugin_data->rmastergain0 = rmastergain0;
      plugin_data->rpeakgain0 = rpeakgain0;
      plugin_data->rpeakgain1 = rpeakgain1;
      plugin_data->lastrgain = lastrgain;
      plugin_data->extra_maxlevel = extra_maxlevel;
    
}

static const LV2_Descriptor dysonCompressDescriptor = {
  "http://plugin.org.uk/swh-plugins/dysonCompress",
  instantiateDysonCompress,
  connectPortDysonCompress,
  activateDysonCompress,
  runDysonCompress,
  NULL,
  cleanupDysonCompress,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &dysonCompressDescriptor;
  default:
    return NULL;
  }
}
