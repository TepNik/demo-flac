#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <fstream>
#include <cmath>
#include <chrono>

#include "param.h"
#include "compress.h"
#include "decompress.h"
#include "operations.h"

int main(int argc, char *argv[])
{
    std::cout << "Implementation of flac\n";

    auto param = get_start_paramentrs(argc, argv);
    std::cout << "Input file: " << param.input_file << '\n'
              << "Output file: " << param.output_file << '\n';
    if (param.compress)
    std::cout << "Bps: " << param.bps << '\n'
              << "Number of channels: " << param.num_channels << '\n'
              << "Sample rate: " << param.sample_rate << '\n';
    std::cout << '\n';

    set_block_size(param);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    if (param.compress)
    {
        start = std::chrono::system_clock::now();
        compress_flac(param);
        end = std::chrono::system_clock::now();
        uint32_t in_size = get_size_of_file(param.input_file),
                 out_size = get_size_of_file(param.output_file);
        std::cout << "Compress ratio in/out = " <<
                  in_size << '/' << out_size << " = "
                  << in_size*1.0/out_size << '\n';
    }
    else
    {
        start = std::chrono::system_clock::now();
        decompress_flac(param);
        end = std::chrono::system_clock::now();
    }
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << " seconds.\n";

    return 0;
}