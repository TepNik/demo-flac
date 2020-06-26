#ifndef _BITWRITER_H_
#define _BITWRITER_H_

#include <cstdint>

struct bitwriter
{
    uint8_t *buffer = nullptr;
    mutable int ind1 = 0, ind2 = 0;

    void write_n_th_bit(bool val, int ind1 = -1, int ind2 = -1);
    bool get_n_th_bit(int ind1 = -1, int ind2 = -1) const;
    void write_next_bit(bool val);
    bool get_next_bit() const;
};

#endif