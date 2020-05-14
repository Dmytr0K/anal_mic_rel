#include "mic_direction.h"

float autocorelation(float sample[SAMPLE_NUM][2], int shift) {
    float sum = 0;
    int el = 0;
    for (int i = 0; i < SAMPLE_NUM; i++) {
        el = i + shift;
        if (el >= SAMPLE_NUM) {
            el -= SAMPLE_NUM;
        } else if (el < 0) {
            el += SAMPLE_NUM;
        }
        sum += sample[i][0] * sample[el][1];
    }
    return sum / SAMPLE_NUM;
}

int findShift (float sample[SAMPLE_NUM][2]) {
    int max_shift = 0;
    float max_correlation = 0;
    float corr = 0;
    for (int shift = 0; shift < SAMPLE_NUM / 4; shift++) {
        corr = autocorelation(sample, shift);
        if (corr > max_correlation) {
            max_correlation = corr;
            max_shift = shift;
        }
        corr = autocorelation(sample, -shift);
        if (corr > max_correlation) {
            max_correlation = corr;
            max_shift = shift;
        }
    }
    return max_shift;
}
