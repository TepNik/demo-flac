#ifndef _FRAME_H_
#define _FRAME_H_

#include <cstdint>

#include "constants.h"
#include "bitwriter.h"

struct Main_header
{
    uint8_t channels,
            bps;
    uint32_t sample_rate;
};

struct Frame_header
{
    uint32_t blocksize;
};


enum Subframe_type : uint8_t
{
    CONSTANT = 1,
    VERBATIUM = 2,
    FIXED = 3
};

struct Subframe_Constant
{
	int32_t value;
};
struct Subframe_Verbatium
{
    const int32_t *data = nullptr;
};

struct Subframe_Fixed
{
    uint8_t order, rice_parameter;
    int32_t warmup[MAX_FIXED_ORDER];
    bitwriter bw_data;
};


struct Subframe_header
{
    Subframe_type type;
    union _data
    {
        Subframe_Constant constant;
        Subframe_Verbatium verbatium;
        Subframe_Fixed fixed;
        _data() {}
        ~_data() {}
    } data;
    uint8_t wasted_bits;
    Subframe_header() {}
    ~Subframe_header() {}
};

#endif