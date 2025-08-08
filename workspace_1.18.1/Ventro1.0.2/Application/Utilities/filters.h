#ifndef __FILTERS_H
#define __FILTERS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float alpha;    // smoothing factor (0 to 1)
    float y_prev;   // previous output
} IIRFilter;

void IIRFilter_Init(IIRFilter *filter, float alpha);
float IIRFilter_Apply(IIRFilter *filter, float input);

#ifdef __cplusplus
}
#endif

#endif /* __FILTERS_H */
