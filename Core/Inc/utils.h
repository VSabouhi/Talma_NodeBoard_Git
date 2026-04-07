#ifndef INC_UTILS_H_
#define INC_UTILS_H_
/*----------------------------------------------------------*/

#include <stdint.h>

/*----------------------------------------------------------*/

static inline uint8_t clamp_u8(int32_t v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}
/*----------------------------------------------------------*/

#endif /* INC_UTILS_H_ */
