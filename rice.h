#ifndef _RICE_H_
#define _RICE_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <vector>

#include "operations.h"
#include "bitwriter.h"

int get_overflow(uint32_t x, int32_t k);

uint32_t get_max_rice_partition_order_from_blocksize(uint32_t blocksize);

void rice_c(int32_t in[], int32_t blocksize, int32_t k, bitwriter &bw);
uint32_t rice_d(bitwriter &bw,
                std::ifstream &fin,
                int k,
                int32_t *output,
                uint32_t blocksize);







/* template <typename type>
int get_overflow(type x, int k)
{
    return x / (type(1) << k);
}

template <typename type>
int get_first_k_bit(type x, int k)
{
    type mask = 0;
    for (int i = 0; i < k; ++i)
        mask |= (type(1) << i);
    return x & mask;
}

template <typename type>
std::vector<int8_t> rice_c(const std::vector<type> &in, bool is_signed, int k)
{
    std::vector<int8_t> result = {0};
    int ind_1 = 0, ind_2 = 0;
    for(int i = 0; i < in.size(); ++i)
    {
        if(is_signed)
            write_next_bit(result, ind_1, ind_2, in[i] < 0);
        type x = std::abs(in[i]);

        int overflow = get_overflow(x, k);
        for (int j = 0; j < overflow; ++j)
            write_next_bit(result, ind_1, ind_2, 0);
        write_next_bit(result, ind_1, ind_2, 1);

        for (int j = k - 1; j >= 0; --j)
            write_next_bit(result, ind_1, ind_2, get_n_th_bit(x, j));
    }
    return result;
}

template <typename type>
std::vector<type> rice_d(const std::vector<int8_t> &in, bool is_signed, int k)
{
    std::vector<type> result;
    int ind_1 = 0, ind_2 = 0;
    while(ind_1 < in.size())
    {
        bool sign = get_next_bit(in, ind_1, ind_2);
        type x = 0;
        while(get_next_bit(in, ind_1, ind_2) == 0 && ind_1 < in.size())
            ++x;
        if (ind_1 >= in.size())
            return result;
        x <<= k;
        for (int i = k - 1; i >= 0; --i)
        {
            bool bit = get_next_bit(in, ind_1, ind_2);
            write_n_th_bit(x, i, bit);
        }
        if (sign)
            x *= -1;
        result.push_back(x);
    }
    return result;
} */

#endif