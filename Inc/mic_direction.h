#ifndef INC_MIC_DIRECTION_H_
#define INC_MIC_DIRECTION_H_


#include <stdint.h>
#include "settings.h"

float autocorelation(float sample[SAMPLE_NUM][2], int shift);
int findShift (float sample[SAMPLE_NUM][2]);

#endif /* INC_MIC_DIRECTION_H_ */
