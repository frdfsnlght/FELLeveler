#ifndef FILTER_EWMA_H
#define FILTER_EWMA_H

// Implements a filter using Exponential Window Moving Average with a quadratic prefilter

class FilterEWMA {

    public:
    
    //float a = 1.0f;
    //float b = 0.06373194f;
    //float c = 0.001299763f;
    float alpha = 0.15f;
    float value = 0.0f;

    float filter(float v) {
        // prefilter - this scales the new value toward the old value, more the farther away it is
        // the coefficients are based on the difference and the desired scales:
        // y = 1 - 0.06373194*x + 0.001299763*x^2
        // 0 -> 1
        // 9.8067 -> 0.5    (1g difference will be halved)
        // 19.6134 -> 0.25  (2g difference will be quarted)
        float diff = v - value;
        if (abs(diff) > 2.0f)
            v = v - (0.1f * diff);
        //diff = diff * (a - b * abs(diff) + c * abs(diff) * abs(diff));
        //v = v - diff;

        value = (alpha * v) + ((1.0f - alpha) * value);
        return value;
    }

};

#endif
