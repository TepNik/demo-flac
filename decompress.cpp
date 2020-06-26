#ifndef _DECOMPRESS_CPP_
#define _DECOMPRESS_CPP_

#include <fstream>
#include <cmath>

#include "decompress.h"
#include "bitwriter.h"
#include "constants.h"
#include "frame.h"
#include "rice.h"

void decompress_flac(pr_param &param)
{
    int32_t _out[MAX_CHANNELS][20000];
    int32_t *output[MAX_CHANNELS];
    int32_t wasted_bits[MAX_CHANNELS] {};
    for(int channel = 0; channel < MAX_CHANNELS; ++channel)
        output[channel] = _out[channel];


    std::ifstream fin(param.input_file, std::ios::binary);
    std::ofstream fout(param.output_file, std::ios::binary);

    bitwriter bw_read{new uint8_t, 0, 0};
    fin.read((char*)bw_read.buffer, sizeof(uint8_t));

    Main_header main_header;
    get_main_header(bw_read, fin, main_header);
    while(bw_read.ind2 != 0 && fin)
            get_next_bit_from_file(bw_read, fin);

    int ind = 0;
    while(fin)
    {
        Frame_header frame_header;
        get_frame_header(bw_read, fin, frame_header);
        if (!fin)
            break;

        for(int channel = 0; channel < main_header.channels; ++channel)
            get_sub_frame_samples(bw_read, fin, main_header, output[channel], frame_header.blocksize);

        for(int sample = 0; sample < frame_header.blocksize; ++sample)
            for(int channel = 0; channel < main_header.channels; ++channel)
            {
                if (main_header.bps == 8 && param.sign)
                {
                    int8_t x = output[channel][sample];
                    fout.write((char*)&x, sizeof(x));
                }
                else if (main_header.bps == 8 && !param.sign)
                {
                    uint8_t x = output[channel][sample];
                    x += 0x80;
                    fout.write((char*)&x, sizeof(x));
                }
                else if (main_header.bps == 16 && param.sign)
                {
                    int16_t x = output[channel][sample];
                    fout.write((char*)&x, sizeof(x));
                }
                else if (main_header.bps == 16 && !param.sign)
                {
                    uint16_t x = output[channel][sample];
                    x += 0x8000;
                    fout.write((char*)&x, sizeof(x));
                }
                else if (main_header.bps == 32 && param.sign)
                {
                    int32_t x = output[channel][sample];
                    fout.write((char*)&x, sizeof(x));
                }
                else if (main_header.bps == 32 && !param.sign)
                {
                    uint32_t x = output[channel][sample];
                    x += 0x800000;
                    fout.write((char*)&x, sizeof(x));
                }
            }
        while(bw_read.ind2 != 0 && fin)
            get_next_bit_from_file(bw_read, fin);
    }

    delete bw_read.buffer;
}

bool get_next_bit_from_file(bitwriter &bw, std::ifstream &fin)
{
    if (bw.buffer == nullptr)
    {
        bw.buffer = new uint8_t{};
        fin.read((char*)bw.buffer, sizeof(uint8_t));
        if (!fin)
            exit(1);
    }
    if (bw.ind2 == 7)
    {
        bool result = bw.get_next_bit();
        fin.read((char*)bw.buffer, sizeof(uint8_t));
        if (!fin)
            return false;
        bw.ind1 = 0;
        bw.ind2 = 0;
        return result;
    }
    else
        return bw.get_next_bit();
}

void get_main_header(bitwriter &bw, std::ifstream &fin, Main_header &main_header)
{
    uint32_t bits_for_value = std::ceil(std::log2(MAX_CHANNELS));
    uint8_t channels = 0;
    bitwriter bw_write{(uint8_t*)&channels, 0, 0};
    for(int i = 0; i < bits_for_value && fin; ++i)
        bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
    if (!fin)
        return;
    main_header.channels = channels;

    bits_for_value = std::ceil(std::log2(MAX_BPS));
    uint8_t bps = 0;
    bw_write.buffer = (uint8_t*)&bps;
    bw_write.ind1 = 0;
    bw_write.ind2 = 0;
    for(int i = 0; i < bits_for_value && fin; ++i)
        bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
    main_header.bps = bps;

    bits_for_value = sizeof(main_header.sample_rate) * 8;
    uint32_t sample_rate = 0;
    bw_write.buffer = (uint8_t*)&sample_rate;
    bw_write.ind1 = 0;
    bw_write.ind2 = 0;
    for(int i = 0; i < bits_for_value && fin; ++i)
        bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
    main_header.sample_rate = sample_rate;

    /* std::cout << "Channels = " << (int)main_header.channels << '\n'
              << "Bps = " << (int)main_header.bps << '\n'
              << "Sample rate = " << (int)main_header.sample_rate << '\n'; */

    if (!fin)
        return;
}

void get_frame_header(bitwriter &bw, std::ifstream &fin, Frame_header &frame)
{
    uint32_t bits_for_value = sizeof(frame.blocksize) * 8 - 1;
    int blocksize = 0;
    bitwriter bw_write{(uint8_t*)&blocksize, 0, 0};
    for(int i = 0; i < bits_for_value && fin; ++i)
        bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
    get_next_bit_from_file(bw, fin);
    frame.blocksize = blocksize;

    if (!fin)
        return;
}

void get_sub_frame_samples(bitwriter &bw,
                           std::ifstream &fin,
                           const Main_header &frame,
                           int32_t *samples,
                           uint32_t blocksize)
{
    Subframe_header sub_header;
    get_sub_frame_header(bw, fin, sub_header);

    uint32_t bps = frame.bps - sub_header.wasted_bits;
    if (sub_header.type == VERBATIUM)
    {
        for(int sample = 0; sample < blocksize; ++sample)
        {
            uint32_t bits_for_value = bps - 1;
            bool sign = get_next_bit_from_file(bw, fin);
            samples[sample] = 0;
            bitwriter bw_write{(uint8_t*)&samples[sample], 0, 0};
            for(int i = 0; i < bits_for_value && fin; ++i)
                bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
            if (sign)
                samples[sample] *= -1;

            samples[sample] <<= sub_header.wasted_bits;
        }
    }
    else if (sub_header.type == CONSTANT)
    {
        uint32_t bits_for_value = bps - 1;
        int32_t value = 0;
        bool sign = get_next_bit_from_file(bw, fin);
        bitwriter bw_write{(uint8_t*)&value, 0, 0};
        for(int i = 0; i < bits_for_value && fin; ++i)
            bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
        if (sign)
            value *= -1;
        sub_header.data.constant.value = value;

        sub_header.data.constant.value <<= sub_header.wasted_bits;
        for(int sample = 0; sample < blocksize; ++sample)
            samples[sample] = sub_header.data.constant.value;
    }
    else if (sub_header.type == FIXED)
    {
        uint32_t bits_for_value = std::ceil(std::log2(MAX_FIXED_ORDER));
        int32_t value = 0;
        bitwriter bw_write{(uint8_t*)&value, 0, 0};
        for(int i = 0; i < bits_for_value && fin; ++i)
            bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
        sub_header.data.fixed.order = value;

        bits_for_value = std::ceil(std::log2(MAX_BPS));
        value = 0;
        bw_write.buffer = (uint8_t*)&value;
        bw_write.ind1 = 0;
        bw_write.ind2 = 0;
        for(int i = 0; i < bits_for_value && fin; ++i)
            bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
        sub_header.data.fixed.rice_parameter = value;

        for(int i = 0; i < MAX_FIXED_ORDER; ++i)
        {
            bits_for_value = bps - 1;
            bool sign = get_next_bit_from_file(bw, fin);
            value = 0;
            bw_write.buffer =(uint8_t*)&value;
            bw_write.ind1 = 0;
            bw_write.ind2 = 0;
            for(int i = 0; i < bits_for_value && fin; ++i)
                bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
            if (sign)
                value *= -1;

            sub_header.data.fixed.warmup[i] = value;
            samples[i] = value;
        }

        rice_d(bw, fin,
               sub_header.data.fixed.rice_parameter,
               samples + MAX_FIXED_ORDER,
               blocksize - MAX_FIXED_ORDER);

        from_fixed_to_normal(samples + MAX_FIXED_ORDER,
                             blocksize - MAX_FIXED_ORDER,
                             sub_header.data.fixed.order);
    }
    else
    {
        std::cout << "Error!\n";
        exit(1);
    }
}

void get_sub_frame_header(bitwriter &bw, std::ifstream &fin, Subframe_header &sub_header)
{
    uint32_t bits_for_value = SIZE_OF_SUBFRAME_TYPE;
    Subframe_type type = CONSTANT;
    bitwriter bw_write{(uint8_t*)&type, 0, 0};
    for(int i = 0; i < bits_for_value && fin; ++i)
        bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
    sub_header.type = type;

    bits_for_value = std::ceil(std::log2(MAX_BPS));
    uint8_t wasted_bits = 0;
    bw_write.buffer = (uint8_t*)&wasted_bits;
    bw_write.ind1 = 0;
    bw_write.ind2 = 0;
    for(int i = 0; i < bits_for_value && fin; ++i)
        bw_write.write_next_bit(get_next_bit_from_file(bw, fin));
    sub_header.wasted_bits = wasted_bits;
}

void from_fixed_to_normal(int32_t *samples, int32_t blocksize, int32_t order)
{
    if (order == 0)
        return;
    for(int sample = 0; sample < blocksize; ++sample)
    {
        if (order == 1)
            samples[sample] = samples[sample] + samples[sample - 1];
        else if (order == 2)
            samples[sample] = samples[sample] + 2 * samples[sample - 1] - samples[sample - 2];
        else if (order == 3)
            samples[sample] = samples[sample] + 3 * samples[sample - 1] - 3 * samples[sample - 2] + samples[sample - 3];
        else if (order == 4)
            samples[sample] = samples[sample] + 4 * samples[sample - 1] - 6 * samples[sample - 2] + 4 * samples[sample - 3] - samples[sample - 4];
    }
}

#endif