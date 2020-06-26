#ifndef _COMPRESS_H_
#define _COMPRESS_H_

#include "param.h"
#include "operations.h"
#include "frame.h"
#include "rice.h"
#include "constants.h"
#include "bitwriter.h"

void compress_flac(pr_param &param);
uint32_t evaluate_verbatium_subframe(const int32_t signal[],
                                     uint32_t blocksize,
                                     uint32_t bps,
                                     Subframe_header &subframe);
uint32_t evaluate_constant_subframe(const int32_t signal[],
                                    uint32_t blocksize,
                                    uint32_t bps,
                                    Subframe_header &subframe);

uint32_t evaluate_fixed_subframe(const int32_t signal[],
                                 int32_t order,
                                 int32_t blocksize,
                                 int32_t rice_parameter,
                                 uint32_t bps,
                                 Subframe_header &subframe);

uint32_t get_constant_subframe_size(uint32_t bps);
uint32_t get_verbatium_subframe_size(uint32_t blocksize, uint32_t bps);
uint32_t get_fixed_subframe_size(bitwriter &bw, uint32_t bps);

void output_main_header(const Main_header &frame, bitwriter &bw);
void output_frame_header(const Frame_header &frame, bitwriter &bw);
void output_subframe_header(const Subframe_header &sub_frame, bitwriter &bw, uint32_t blocksize, uint32_t subframe_bps);

uint32_t compute_best_predictor(const int32_t data[], uint32_t data_len, float residual_bits_per_sample[]);

void set_block_size(pr_param &param);

#endif