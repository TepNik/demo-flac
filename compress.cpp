#ifndef _COMPRESS_CPP_
#define _COMPRESS_CPP_

#include <cmath>

#include "compress.h"
#include "param.h"
#include "bitwriter.h"

int32_t *input[MAX_CHANNELS];
int32_t _in[MAX_CHANNELS][UINT16_MAX];

void compress_flac(pr_param &param)
{
    for(int channel = 0; channel < MAX_CHANNELS; ++channel)
        input[channel] = _in[channel];

    uint32_t blocksize = param.blocksize,
             compressed_samples = 0,
             input_size_bit = get_size_of_file(param.input_file),
             number_of_samples = input_size_bit / (param.bps / 8);

    Subframe_header workspace[MAX_CHANNELS][2];

    uint32_t subframe_bps[MAX_CHANNELS],
             best_bits[MAX_CHANNELS];
    bool best_subframe[MAX_CHANNELS];
    bitwriter bw_write{new uint8_t[UINT16_MAX], 0, 0},
              bw_fixed_sub_frame{new uint8_t[UINT16_MAX], 0, 0};
    bw_write.buffer[0] = 0;
    bw_fixed_sub_frame.buffer[0] = 0;

    std::ifstream fin;
    fin.open(param.input_file, std::ios::binary | std::ios::in);
    std::ofstream fout;
    fout.open(param.output_file, std::ios::binary | std::ios::out);

    if (!fin && !fout)
    {
        std::cout << "Error with files!\n";
        exit(1);
    }

    Main_header main_header;
    main_header.bps = param.bps;
    main_header.channels = param.num_channels;
    main_header.sample_rate = param.sample_rate;
    output_main_header(main_header, bw_write);
    fout.write((char*)bw_write.buffer, bw_write.ind1 + (bw_write.ind2 == 0 ? 0 : 1));
    bw_write.ind1 = 0;
    bw_write.ind2 = 0;

    while(compressed_samples < number_of_samples)
    {
        Frame_header frame_header;
        uint32_t min_partition_order = param.min_residual_partition_order,
                 best_bits[MAX_CHANNELS],
                 max_partition_order,
                 chunk,
                 bit_now;

        bw_write.ind1 = 0;
        bw_write.ind2 = 0;

        chunk = std::min(number_of_samples - compressed_samples, blocksize * param.num_channels) / param.num_channels;
        get_channels(input,
                     param.num_channels,
                     param.bps,
                     param.sign,
                     chunk,
                     fin);

        max_partition_order = get_max_rice_partition_order_from_blocksize(chunk);
        max_partition_order = std::min(max_partition_order, param.max_residual_partition_order);
        min_partition_order = std::min(min_partition_order, max_partition_order);

        frame_header.blocksize = chunk;

        for(int channel = 0; channel < param.num_channels; ++channel)
        {
            uint32_t w = get_wasted_bits(input[channel], chunk);
            if (w > param.bps)
                w = param.bps;
            workspace[channel][0].wasted_bits = workspace[channel][1].wasted_bits = w;
            subframe_bps[channel] = param.bps - w;
        }
        for(int channel = 0; channel < param.num_channels; ++channel)
        {
            best_subframe[channel] = 0;
            best_bits[channel] = evaluate_verbatium_subframe(input[channel], chunk, subframe_bps[channel], workspace[channel][best_subframe[channel]]);
            bool is_constant = true;
            for(int sample = 1; sample < chunk && is_constant; ++sample)
                if (input[channel][sample] != input[channel][sample - 1])
                    is_constant = false;
            if (is_constant)
            {
                bit_now = evaluate_constant_subframe(input[channel], chunk, subframe_bps[channel], workspace[channel][!best_subframe[channel]]);
                if (bit_now < best_bits[channel])
                {
                    best_bits[channel] = bit_now;
                    best_subframe[channel] = !best_subframe[channel];
                }
            }
            else
            {
                float fixed_residual_bits_per_sample[MAX_FIXED_ORDER + 1];
                uint32_t min_fixed_order,
                         max_fixed_order,
                         guess_fixed_order,
                         max_rice_parametr,
                         rice_parameter;
                guess_fixed_order = compute_best_predictor(input[channel], chunk, fixed_residual_bits_per_sample);
                max_rice_parametr = (subframe_bps[channel] > 16 ? MAX_WIDE_RICE_PARAMETR : MAX_RICE_PARAMETR);
                if (param.do_exhaustive_model_search)
                {
                    min_fixed_order = 0;
                    max_fixed_order = MAX_FIXED_ORDER;
                }
                else
                    min_fixed_order = max_fixed_order = guess_fixed_order;
                //std::cout << "Guess = " << guess_fixed_order << '\n';
                for (int fixed_order = min_fixed_order; fixed_order <= max_fixed_order; ++fixed_order)
                {
                    if (fixed_residual_bits_per_sample[fixed_order] >= (float)subframe_bps[channel])
                        continue;
                    rice_parameter = (fixed_residual_bits_per_sample[fixed_order] > 0.0)? (uint32_t)(fixed_residual_bits_per_sample[fixed_order]+0.5) : 0;
                    ++rice_parameter;
                    if (rice_parameter >= max_rice_parametr)
                        rice_parameter = max_rice_parametr - 1;
                    bw_fixed_sub_frame.ind1 = 0;
                    bw_fixed_sub_frame.ind2 = 0;
                    workspace[channel][!best_subframe[channel]].data.fixed.bw_data = bw_fixed_sub_frame;
                    bit_now = evaluate_fixed_subframe(input[channel],
                                                      fixed_order,
                                                      chunk,
                                                      rice_parameter,
                                                      subframe_bps[channel],
                                                      workspace[channel][!best_subframe[channel]]);
                    if (bit_now < best_bits[channel])
                    {
                        best_bits[channel] = bit_now;
                        best_subframe[channel] = !best_subframe[channel];
                    }
                }
            }
        }

        output_frame_header(frame_header, bw_write);
        for(int channel = 0; channel < param.num_channels; ++channel)
        {
            output_subframe_header(workspace[channel][best_subframe[channel]],
                                   bw_write,
                                   chunk,
                                   subframe_bps[channel]);
        }
        fout.write((char*)bw_write.buffer, bw_write.ind1 + (bw_write.ind2 == 0 ? 0 : 1));
        compressed_samples += chunk * param.num_channels;
    }
    delete bw_write.buffer;
    bw_write.buffer = nullptr;
}

void set_block_size(pr_param &param)
{
    if (param.blocksize == 0)
    {
        /* if (param.max_lpc_order == 0)
            param.blocksize = 1152;
        else */
            param.blocksize = 4096;
    }
}

uint32_t evaluate_verbatium_subframe(const int32_t signal[],
                                     uint32_t blocksize,
                                     uint32_t bps,
                                     Subframe_header &subframe)
{
    subframe.type = VERBATIUM;
    subframe.data.verbatium.data = signal;

    return get_verbatium_subframe_size(blocksize, bps);
}

void output_main_header(const Main_header &main_header, bitwriter &bw)
{
    uint32_t bits_for_value = std::ceil(std::log2(MAX_CHANNELS));
    bitwriter bw_read{(uint8_t*)&main_header.channels, 0, 0};
    for(int i = 0; i < bits_for_value; ++i)
        bw.write_next_bit(bw_read.get_next_bit());

    bits_for_value = std::ceil(std::log2(MAX_BPS));
    bw_read.buffer = (uint8_t*)&main_header.bps;
    bw_read.ind1 = 0;
    bw_read.ind2 = 0;
    for(int i = 0; i < bits_for_value; ++i)
        bw.write_next_bit(bw_read.get_next_bit());

    bits_for_value = sizeof(main_header.sample_rate) * 8;
    bw_read.buffer = (uint8_t*)&main_header.sample_rate;
    bw_read.ind1 = 0;
    bw_read.ind2 = 0;
    for(int i = 0; i < bits_for_value; ++i)
        bw.write_next_bit(bw_read.get_next_bit());
}

void output_frame_header(const Frame_header &frame, bitwriter &bw)
{
    int32_t bits_for_value = sizeof(frame.blocksize) * 8;
    bitwriter bw_read{(uint8_t*)&frame.blocksize, 0, 0};
    for(int i = 0; i < bits_for_value; ++i)
        bw.write_next_bit(bw_read.get_next_bit());
}

void output_subframe_header(const Subframe_header &sub_frame, bitwriter &bw, uint32_t blocksize, uint32_t subframe_bps)
{
    uint32_t bits_for_value = SIZE_OF_SUBFRAME_TYPE;
    bitwriter bw_read{(uint8_t*)&sub_frame.type, 0, 0};
    for(int i = 0; i < bits_for_value; ++i)
        bw.write_next_bit(bw_read.get_next_bit());

    bits_for_value = std::ceil(std::log2(MAX_BPS));
    bw_read.buffer = (uint8_t*)&sub_frame.wasted_bits;
    bw_read.ind1 = 0;
    bw_read.ind2 = 0;
    for(int i = 0; i < bits_for_value; ++i)
        bw.write_next_bit(bw_read.get_next_bit());

    if (sub_frame.type == VERBATIUM)
    {
        //std::cout << "VERBATIUM\n";
        for(int sample = 0; sample < blocksize; ++sample)
        {
            bits_for_value = subframe_bps - 1;
            int32_t value = sub_frame.data.verbatium.data[sample];
            bw.write_next_bit(value < 0);
            value = std::abs(value);
            bw_read.buffer = (uint8_t*)&value;
            bw_read.ind1 = 0;
            bw_read.ind2 = 0;
            for(int i = 0; i < bits_for_value; ++i)
                bw.write_next_bit(bw_read.get_next_bit());
        }
    }
    else if (sub_frame.type == CONSTANT)
    {
        //std::cout << "CONSTANT; Value = " << sub_frame.data.constant.value << '\n';
        bits_for_value = subframe_bps - 1;
        int32_t value = sub_frame.data.constant.value;
        bw.write_next_bit(value < 0);
        value = std::abs(value);
        bw_read.buffer = (uint8_t*)&value;
        bw_read.ind1 = 0;
        bw_read.ind2 = 0;
        for(int i = 0; i < bits_for_value; ++i)
            bw.write_next_bit(bw_read.get_next_bit());
    }
    else if (sub_frame.type == FIXED)
    {
        //std::cout << "FIXED; order = " << (int)sub_frame.data.fixed.order << '\n';
        bits_for_value = std::ceil(std::log2(MAX_FIXED_ORDER));
        bw_read.buffer = (uint8_t*)&sub_frame.data.fixed.order;
        bw_read.ind1 = 0;
        bw_read.ind2 = 0;
        for(int i = 0; i < bits_for_value; ++i)
            bw.write_next_bit(bw_read.get_next_bit());

        bits_for_value = std::ceil(std::log2(MAX_BPS));
        bw_read.buffer = (uint8_t*)&sub_frame.data.fixed.rice_parameter;
        bw_read.ind1 = 0;
        bw_read.ind2 = 0;
        for(int i = 0; i < bits_for_value; ++i)
            bw.write_next_bit(bw_read.get_next_bit());

        for(int i = 0; i < MAX_FIXED_ORDER; ++i)
        {
            bits_for_value = subframe_bps - 1;
            int32_t value = sub_frame.data.fixed.warmup[i];
            bw.write_next_bit(value < 0);
            value = std::abs(value);
            bw_read.buffer = (uint8_t*)&value;
            bw_read.ind1 = 0;
            bw_read.ind2 = 0;
            for(int i = 0; i < bits_for_value; ++i)
                bw.write_next_bit(bw_read.get_next_bit());
        }

        bw_read.buffer = sub_frame.data.fixed.bw_data.buffer;
        bw_read.ind1 = 0;
        bw_read.ind2 = 0;
        while(bw_read.ind1 < sub_frame.data.fixed.bw_data.ind1 ||
              bw_read.ind2 < sub_frame.data.fixed.bw_data.ind2)
        {
            bw.write_next_bit(bw_read.get_next_bit());
        }
    }
}

uint32_t compute_best_predictor(const int32_t data[], uint32_t data_len, float residual_bits_per_sample[])
{
	int32_t last_error_0 = data[-1];
	int32_t last_error_1 = data[-1] - data[-2];
	int32_t last_error_2 = last_error_1 - (data[-2] - data[-3]);
	int32_t last_error_3 = last_error_2 - (data[-2] - 2*data[-3] + data[-4]);
	int32_t error, save;
	uint64_t total_error_0 = 0, total_error_1 = 0, total_error_2 = 0, total_error_3 = 0, total_error_4 = 0;
	uint32_t i, order;

	for(i = 0; i < data_len; i++) {
		error  = data[i]     ; total_error_0 += std::abs(error);                      save = error; //std::cout << std::abs(error) << '\n';
		error -= last_error_0; total_error_1 += std::abs(error); last_error_0 = save; save = error; //std::cout << std::abs(error) << '\n';
		error -= last_error_1; total_error_2 += std::abs(error); last_error_1 = save; save = error; //std::cout << std::abs(error) << '\n';
		error -= last_error_2; total_error_3 += std::abs(error); last_error_2 = save; save = error; //std::cout << std::abs(error) << '\n';
		error -= last_error_3; total_error_4 += std::abs(error); last_error_3 = save; //std::cout << std::abs(error) << '\n';
	}
    //std::cout << "Sums = " << total_error_0 << ' ' << total_error_1 << ' ' << total_error_2 << ' ' << total_error_3 << ' ' << total_error_4 << '\n';

	if(total_error_0 < std::min(std::min(std::min(total_error_1, total_error_2), total_error_3), total_error_4))
		order = 0;
	else if(total_error_1 < std::min(std::min(total_error_2, total_error_3), total_error_4))
		order = 1;
	else if(total_error_2 < std::min(total_error_3, total_error_4))
		order = 2;
	else if(total_error_3 < total_error_4)
		order = 3;
	else
		order = 4;

	assert(data_len > 0 || total_error_0 == 0);
	assert(data_len > 0 || total_error_1 == 0);
	assert(data_len > 0 || total_error_2 == 0);
	assert(data_len > 0 || total_error_3 == 0);
	assert(data_len > 0 || total_error_4 == 0);

	residual_bits_per_sample[0] = (float)((total_error_0 > 0) ? std::log(M_LN2 * (double)total_error_0 / (double)data_len) / M_LN2 : 0.0);
	residual_bits_per_sample[1] = (float)((total_error_1 > 0) ? std::log(M_LN2 * (double)total_error_1 / (double)data_len) / M_LN2 : 0.0);
	residual_bits_per_sample[2] = (float)((total_error_2 > 0) ? std::log(M_LN2 * (double)total_error_2 / (double)data_len) / M_LN2 : 0.0);
	residual_bits_per_sample[3] = (float)((total_error_3 > 0) ? std::log(M_LN2 * (double)total_error_3 / (double)data_len) / M_LN2 : 0.0);
	residual_bits_per_sample[4] = (float)((total_error_4 > 0) ? std::log(M_LN2 * (double)total_error_4 / (double)data_len) / M_LN2 : 0.0);

	return order;
}

uint32_t evaluate_constant_subframe(const int32_t signal[],
                                    uint32_t blocksize,
                                    uint32_t bps,
                                    Subframe_header &subframe)
{
    subframe.type = CONSTANT;
    subframe.data.constant.value = signal[0];

    return get_constant_subframe_size(bps);
}

uint32_t evaluate_fixed_subframe(const int32_t signal[],
                                 int32_t order,
                                 int32_t blocksize,
                                 int32_t rice_parameter,
                                 uint32_t bps,
                                 Subframe_header &subframe)
{
    subframe.type = FIXED;
    subframe.data.fixed.order = order;
    subframe.data.fixed.rice_parameter = rice_parameter;
    for(int i = 0; i < MAX_FIXED_ORDER; ++i)
        subframe.data.fixed.warmup[i] = signal[i];

    int32_t residual[20000];
    uint32_t ind_start = MAX_FIXED_ORDER;
    if (order == 0)
        memcpy(residual, signal + MAX_FIXED_ORDER, sizeof(residual[0])*(blocksize - MAX_FIXED_ORDER));
    else
        for(int sample = 0; sample < blocksize - MAX_FIXED_ORDER; ++sample)
        {
            if (order == 1)
                residual[sample] = signal[ind_start + sample] - signal[ind_start + sample - 1];
            else if (order == 2)
                residual[sample] = signal[ind_start + sample] - 2 * signal[ind_start + sample - 1] + signal[ind_start + sample - 2];
            else if (order == 3)
                residual[sample] = signal[ind_start + sample] - 3 * signal[ind_start + sample - 1] + 3 * signal[ind_start + sample - 2] - signal[ind_start + sample - 3];
            else if (order == 4)
                residual[sample] = signal[ind_start + sample] - 4 * signal[ind_start + sample - 1] + 6 * signal[ind_start + sample - 2] - 4 * signal[ind_start + sample - 3] + signal[ind_start + sample - 4];
        }

    rice_c(residual, blocksize - MAX_FIXED_ORDER, rice_parameter, subframe.data.fixed.bw_data);

    return get_fixed_subframe_size(subframe.data.fixed.bw_data, bps);
}

uint32_t get_constant_subframe_size(uint32_t bps)
{
    return SIZE_OF_SUBFRAME_TYPE /* for Subframe_type */ +
           bps /* for value of const */ +
           std::ceil(std::log2(MAX_BPS)) /* for value of wasted_bits (maximum value is MAX_BPS) */;
}

uint32_t get_verbatium_subframe_size(uint32_t blocksize, uint32_t bps)
{
    return SIZE_OF_SUBFRAME_TYPE /* for Subframe_type */ +
           bps * blocksize /* for values */ +
           std::ceil(std::log2(MAX_BPS)) /* for value of wasted_bits (maximum value is MAX_BPS) */;
}

uint32_t get_fixed_subframe_size(bitwriter &bw, uint32_t bps)
{
    return SIZE_OF_SUBFRAME_TYPE /* for Subframe_type */ +
           std::ceil(std::log2(MAX_BPS)) /* for value of wasted_bits (maximum value is MAX_BPS) */ +
           std::ceil(std::log2(MAX_FIXED_ORDER)) /* for order */ +
           bps * MAX_FIXED_ORDER /* for warmup */ +
           bw.ind1 * 8 + bw.ind2 /* for values */ +
           std::ceil(std::log2(MAX_BPS)) /* for rice_parameter */;
}

#endif