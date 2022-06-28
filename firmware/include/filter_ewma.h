#ifndef FILTER_EWMA_H
#define FILTER_EWMA_H

// Implements a filter using Exponential Window Moving Average

class FilterEWMA {

    public:
    
    float alpha = 0.2;
    float value = 0;

    float filter(float v) {
        value = (alpha * v) + ((1.0f - alpha) * value);
        return value;
    }

};

#endif
