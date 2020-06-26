#ifndef _OPERATIONS_CPP_
#define _OPERATIONS_CPP_

#include <sys/stat.h>

#include "operations.h"

off_t get_size_of_file(const std::string &file_name)
{
    return get_size_of_file(file_name.c_str());
}

off_t get_size_of_file(const char *file_name)
{
    struct stat s;
    stat(file_name, &s);
    return s.st_size;
}

void get_channels(int32_t *buffer[],
                  int32_t num_channels,
                  int32_t bps,
                  int32_t is_signed,
                  int32_t number_of_samples,
                  std::ifstream &fin)
{
    for(int ind = 0; ind < number_of_samples; ++ind)
    {
        int i = 0;
        for(i = 0; i < num_channels; ++i)
        {
            if (bps == 8 && !is_signed)
            {
                uint8_t value;
                fin.read((char*)&value, sizeof(value));
                buffer[i][ind] = (int32_t)value - 0x80;
            }
            else if (bps == 8 && is_signed)
            {
                int8_t value;
                fin.read((char*)&value, sizeof(value));
                buffer[i][ind] = value;
            }
            else if (bps == 16 && !is_signed)
            {
                uint16_t value;
                fin.read((char*)&value, sizeof(value));
                buffer[i][ind] = (int32_t)value - 0x8000;
            }
            else if (bps == 16 && is_signed)
            {
                int16_t value;
                fin.read((char*)&value, sizeof(value));
                buffer[i][ind] = value;
            }
            else if (bps == 32 && !is_signed)
            {
                uint32_t value;
                fin.read((char*)&value, sizeof(value));
                buffer[i][ind] = (int32_t)value - 0x800000;
            }
            else if (bps == 32 && is_signed)
                fin.read((char*)&buffer[i][ind], sizeof(buffer[i][ind]));
        }
        if (i != num_channels)
        {
            std::cout << "\nError!\nWrong number of samples!\n";
            exit(1);
        }
    }
}

uint32_t get_wasted_bits(int32_t *channel, uint32_t blocksize)
{
    uint32_t i, shift, x = 0;

	for(i = 0; i < blocksize && !(x&1); i++)
    {
		x |= channel[i];
    }

	if(x == 0) {
		shift = 0;
	}
	else {
		for(shift = 0; !(x&1); shift++)
			x >>= 1;
	}

    /* for(i = 1; i < blocksize; ++i)
        std::cout << channel[i] << ' ';
    std::cout << "\n\n"; */

	if(shift > 0) {
		for(i = 0; i < blocksize; i++)
			 channel[i] >>= shift;
	}
    /* for(i = 1; i < blocksize; ++i)
        std::cout << channel[i] << ' ';
    std::cout << '\n'; */
        /* if (channel[i] != channel[i - 1])
            std::cout << "i = " << i << '\n'; */

	return shift;
}

#endif