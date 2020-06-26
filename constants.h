#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <cstdint>

const uint32_t MAX_FIXED_ORDER = 4;
const uint32_t MAX_RICE_PARTITION_ORDER = 15;
const uint32_t MAX_CHANNELS = 32;
const uint32_t MAX_BPS = 32;
const uint32_t NUMBER_OF_SUBFRAME_TYPES = 4;
const uint32_t SIZE_OF_SUBFRAME_TYPE = std::ceil(std::log2(NUMBER_OF_SUBFRAME_TYPES));
const uint32_t MAX_RICE_PARAMETR = 15;
const uint32_t MAX_WIDE_RICE_PARAMETR = 31;

#endif