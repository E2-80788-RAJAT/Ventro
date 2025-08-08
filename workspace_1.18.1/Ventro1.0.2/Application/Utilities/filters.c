#include "filters.h"

void IIRFilter_Init(IIRFilter *filter, float alpha)
{
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    filter->alpha = alpha;
    filter->y_prev = 0.0f;
}

float IIRFilter_Apply(IIRFilter *filter, float input)
{
    float output = filter->alpha * input + (1.0f - filter->alpha) * filter->y_prev;
    filter->y_prev = output;
    return output;
}
