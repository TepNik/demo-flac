#ifndef _PARAM_H_
#define _PARAM_H_

#include <string>
#include <cstdint>

struct pr_param
{
    std::string input_file, output_file;

    bool sign                        = true,
         compress                    = true,
         do_exhaustive_model_search  = false,
         do_qlp_coeff_prec_search    = false;

    uint32_t bps                           = 16,
             num_channels                  = 1,
             sample_rate                   = 44100,
             blocksize                     = 0,
             max_lpc_order                 = 8,
             qlp_coeff_precision           = 1,
             min_residual_partition_order  = 0,
             max_residual_partition_order  = 5,
             rice_parameter_search_dist    = 0;
};
pr_param get_start_paramentrs(int argc, char *argv[]);

#endif