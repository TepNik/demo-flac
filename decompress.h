#ifndef _DECOMPRESS_H_
#define _DECOMPRESS_H_

#include <iostream>
#include <fstream>

#include "param.h"
#include "bitwriter.h"
#include "frame.h"

void decompress_flac(pr_param &param);

bool get_next_bit_from_file(bitwriter &bw, std::ifstream &fin);
void get_main_header(bitwriter &bw, std::ifstream &fin, Main_header &main_header);
void get_frame_header(bitwriter &bw, std::ifstream &fin, Frame_header &frame);
void get_sub_frame_samples(bitwriter &bw,
                           std::ifstream &fin,
                           const Main_header &frame,
                           int32_t *samples,
                           uint32_t blocksize);

void from_fixed_to_normal(int32_t *samples, int32_t blocksize, int32_t order);

void get_sub_frame_header(bitwriter &bw, std::ifstream &fin, Subframe_header &sub_header);

#endif