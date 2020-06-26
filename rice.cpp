#ifndef _RICE_CPP_
#define _RICE_CPP_

#include <cmath>

#include "rice.h"
#include "constants.h"
#include "decompress.h"

uint32_t get_max_rice_partition_order_from_blocksize(uint32_t blocksize)
{
    uint32_t max_rice_partition_order = 0;
	while(!(blocksize & 1)) {
		max_rice_partition_order++;
		blocksize >>= 1;
	}
	return std::min(MAX_RICE_PARTITION_ORDER, max_rice_partition_order);
}

void rice_c(int32_t in[], int32_t blocksize, int32_t k, bitwriter &bw)
{
    for(int sample = 0; sample < blocksize; ++sample)
    {
        bw.write_next_bit(in[sample] < 0);
        int32_t x = std::abs(in[sample]);

        int overflow = get_overflow(x, k);
        for (int j = 0; j < overflow; ++j)
            bw.write_next_bit(0);
        bw.write_next_bit(1);

        for (int j = k - 1; j >= 0; --j)
        {
            int32_t mask = (1 << j);
            int32_t value = ((x & mask) != 0);
            bw.write_next_bit(value);
        }
    }
}

uint32_t rice_d(bitwriter &bw,
                std::ifstream &fin,
                int k,
                int32_t *output,
                uint32_t blocksize)
{
    uint32_t count = 0;
    while(fin && count < blocksize)
    {
        //std::cout << "Count = " << count << '\n';
        bool sign = get_next_bit_from_file(bw, fin);
        int32_t x = 0;
        while(get_next_bit_from_file(bw, fin) == 0 && fin)
            ++x;
        if (!fin)
            return count;
        x <<= k;
        for (int i = k - 1; i >= 0; --i)
        {
            bool bit = get_next_bit_from_file(bw, fin);
            int32_t mask = (1 << i);
            if (bit)
                x |= mask;
            else
                x &= ~mask;
        }
        if (sign)
            x *= -1;
        output[count] = x;
        ++count;
    }
    return count;
}

int get_overflow(uint32_t x, int32_t k)
{
    return x / (uint32_t(1) << k);
}

#endif