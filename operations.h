#ifndef _OPERATIONS_H_
#define _OPERATIONS_H_

#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <vector>

off_t get_size_of_file(const std::string &file_name);
off_t get_size_of_file(const char *file_name);

void get_channels(int32_t *buffer[],
                  int32_t num_channels,
                  int32_t bps,
                  int32_t is_signed,
                  int32_t number_of_samples,
                  std::ifstream &fin);
/* template <typename type>
void write_channels(const std::string &file_name, const Channels<int32_t> &out); */

/* template <typename type>
void write_n_th_bit(type &ch, int ind, bool val);
template <typename type>
bool get_n_th_bit(type ch, int ind);
void write_next_bit(std::vector<int8_t> &out, int &ind_1, int &ind_2, bool val);
bool get_next_bit(const std::vector<int8_t> &in, int &ind_1, int &ind_2); */

uint32_t get_wasted_bits(int32_t *channel, uint32_t blocksize);

/* template <typename type>
void write_channels(const std::string &file_name, const Channels<int32_t> &out)
{
    std::ofstream fout;
    fout.open(file_name, std::ios::binary);

    union
    {
        type x;
        char ch[sizeof(type)];
    };

    for (int i = 0; out.size() > 0 && i < out[0].size(); ++i)
        for(int j = 0; j < out.size(); ++j)
        {
            x = out[j][i];
            fout.write(ch, sizeof(type));
        }
} */

#endif