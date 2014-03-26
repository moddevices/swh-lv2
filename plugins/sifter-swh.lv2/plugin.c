
#include "ladspa-util.h"

#define MAX_BSIZE 1000

inline int partition(float array[], int left, int right);

/* required for clang compilation */
void q_sort(float array[], int left, int right);

inline void q_sort(float array[], int left, int right) {
	float pivot = partition(array, left, right);

	if (left < pivot) {
		q_sort(array, left, pivot-1);
	}
	if (right > pivot) {
		q_sort(array, pivot+1, right);
	}
}

inline int partition(float array[], int left, int right) {
	float pivot = array[left];

	while (left < right) {
		while (array[right] >= pivot && left < right) {
			right--;
		}
		if (left != right) {
			array[left] = array[right];
			left++;
		}
		while (array[left] <= pivot && left < right) {
			left++;
		}
		if (left != right) {
			array[right] = array[left];
			right--;
		}
	}
	array[left] = pivot;

	return left;
}
    
#include <math.h>
#include <stdlib.h>
#include "lv2.h"

typedef struct _Sifter {
  float *size;
  float *input;
  float *output;
long b1ptr;
long b2ptr;
float * b1;
float * b2;
float * ob;
float * rc;
} Sifter;

static void cleanupSifter(LV2_Handle instance)
{
Sifter *plugin_data = (Sifter *)instance;

      free(plugin_data->b1);
      free(plugin_data->b2);
      free(plugin_data->ob);
      free(plugin_data->rc);
    
  free(instance);
}

static void connectPortSifter(LV2_Handle instance, uint32_t port, void *data)
{
  Sifter *plugin = (Sifter *)instance;

  switch (port) {
  case 0:
    plugin->size = data;
    break;
  case 1:
    plugin->input = data;
    break;
  case 2:
    plugin->output = data;
    break;
  }
}

static LV2_Handle instantiateSifter(const LV2_Descriptor *descriptor,
            double s_rate, const char *path,
            const LV2_Feature *const *features)
{
  Sifter *plugin_data = (Sifter *)malloc(sizeof(Sifter));
  long b1ptr = plugin_data->b1ptr;
  long b2ptr = plugin_data->b2ptr;
  float * b1 = plugin_data->b1;
  float * b2 = plugin_data->b2;
  float * ob = plugin_data->ob;
  float * rc = plugin_data->rc;
  
      long i;
      float scla = (float)MAX_BSIZE * 0.5f;
      float sclb = (float)MAX_BSIZE;

      b1 = (float *)calloc(MAX_BSIZE, sizeof(float));
      b2 = (float *)calloc(MAX_BSIZE, sizeof(float));
      ob = (float *)calloc(MAX_BSIZE, sizeof(float));
      rc = (float *)calloc(MAX_BSIZE, sizeof(float));

      // Calculate raised cosine table, to build windowing function from
      rc[0] = cos(((0.0f - scla) / sclb) * M_PI);
      rc[0] *= rc[0];
      for (i=1; i<MAX_BSIZE / 2; i++) {
        rc[i] = cos((((float)i - scla) / sclb) * M_PI);
        rc[i] *= rc[i];
        rc[MAX_BSIZE - i] = rc[i];
      }
      rc[MAX_BSIZE / 2] = 1.0f;

      b1ptr = 0;
      b2ptr = 0;
    
  plugin_data->b1ptr = b1ptr;
  plugin_data->b2ptr = b2ptr;
  plugin_data->b1 = b1;
  plugin_data->b2 = b2;
  plugin_data->ob = ob;
  plugin_data->rc = rc;
  
  return (LV2_Handle)plugin_data;
}


static void activateSifter(LV2_Handle instance)
{
  Sifter *plugin_data = (Sifter *)instance;
  long b1ptr __attribute__ ((unused)) = plugin_data->b1ptr;
  long b2ptr __attribute__ ((unused)) = plugin_data->b2ptr;
  float * b1 __attribute__ ((unused)) = plugin_data->b1;
  float * b2 __attribute__ ((unused)) = plugin_data->b2;
  float * ob __attribute__ ((unused)) = plugin_data->ob;
  float * rc __attribute__ ((unused)) = plugin_data->rc;
  
      b1ptr = 0;
      b2ptr = 0;
      memset(b1, 0, MAX_BSIZE * sizeof(float));
      memset(b2, 0, MAX_BSIZE * sizeof(float));
      memset(ob, 0, MAX_BSIZE * sizeof(float));
    
}


static void runSifter(LV2_Handle instance, uint32_t sample_count)
{
  Sifter *plugin_data = (Sifter *)instance;

  const float size = *(plugin_data->size);
  const float * const input = plugin_data->input;
  float * const output = plugin_data->output;
  long b1ptr = plugin_data->b1ptr;
  long b2ptr = plugin_data->b2ptr;
  float * b1 = plugin_data->b1;
  float * b2 = plugin_data->b2;
  float * ob = plugin_data->ob;
  float * rc = plugin_data->rc;
  
unsigned long pos, i;
long bsize = f_round(LIMIT(size, 1, MAX_BSIZE));

for (pos = 0; pos < sample_count; pos++) {
	if (b1ptr >= bsize) {
		float wstep = (float)MAX_BSIZE / (float)b1ptr, wpos = 0.0f;

		q_sort(b1, 0, b1ptr);
		for (i=0; i<b1ptr; i++) {
			ob[i] += b1[i] * rc[f_round(wpos)];
			wpos += wstep;
		}
		b1ptr = 0;
		b2ptr = (bsize+1) / 2;
	}

	if (b2ptr >= bsize) {
		float wstep = (float)MAX_BSIZE / (float)b2ptr, wpos = 0.0f;
		int offset = (b2ptr+1)/2;

		q_sort(b2, 0, b2ptr);
		for (i=0; i<offset; i++) {
			ob[i + offset] += b2[i] * rc[f_round(wpos)];
			wpos += wstep;
		}
		for (; i<b2ptr; i++) {
			ob[i - offset] += b2[i] * rc[f_round(wpos)];
			wpos += wstep;
		}
		b2ptr = 0;
	}

	if (bsize < 2) {
		ob[b1ptr] = input[pos];
	}

	b1[b1ptr] = input[pos];
	b2[b2ptr] = input[pos];
	buffer_write(output[pos], ob[b1ptr]);
	ob[b1ptr] = 0.0f;
	b1ptr++;
	b2ptr++;
}

plugin_data->b1ptr = b1ptr;
plugin_data->b2ptr = b2ptr;
    
}

static const LV2_Descriptor sifterDescriptor = {
  "http://plugin.org.uk/swh-plugins/sifter",
  instantiateSifter,
  connectPortSifter,
  activateSifter,
  runSifter,
  NULL,
  cleanupSifter,
  NULL
};


LV2_SYMBOL_EXPORT
const LV2_Descriptor *lv2_descriptor(uint32_t index)
{
  switch (index) {
  case 0:
    return &sifterDescriptor;
  default:
    return NULL;
  }
}
