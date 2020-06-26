#ifndef _BITWRITER_CPP_
#define _BITWRITER_CPP_

#include "bitwriter.h"

void bitwriter::write_n_th_bit(bool val, int _ind1, int _ind2)
{
    if (_ind1 == -1)
        _ind1 = ind1;
    if (_ind2 == -1)
        _ind2 = ind2;
    uint8_t &ch = buffer[_ind1];
    uint8_t mask = (1 << _ind2);
    if (val)
        ch |= mask;
    else if (_ind2 < sizeof(uint8_t)*8)
        ch &= ~mask;
}

bool bitwriter::get_n_th_bit(int _ind1, int _ind2) const
{
    if (_ind1 == -1)
        _ind1 = ind1;
    if (_ind2 == -1)
        _ind2 = ind2;
    uint8_t ch = buffer[_ind1];
    uint8_t mask = (1 << _ind2);
    return (ch & mask) != 0;
}

void bitwriter::write_next_bit(bool val)
{
    write_n_th_bit(val);
    ++ind2;
    if (ind2 == 8)
    {
        buffer[++ind1] = 0;
        ind2 = 0;
    }
}

bool bitwriter::get_next_bit() const
{
    bool result = get_n_th_bit();
    ++ind2;
    if (ind2 == 8)
    {
        ++ind1;
        ind2 = 0;
    }
    return result;
}

#endif