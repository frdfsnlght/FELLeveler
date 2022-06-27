#ifndef FILTER_EWMA_H
#define FILTER_EWMA_H

class FilterEWMA {

    public:
    
    float alpha = 0.5;
    float value = 0;

    float filter(float v) {
        value = (alpha * v) + ((1.0f - alpha) * value);
        return value;
    }

};

#endif
