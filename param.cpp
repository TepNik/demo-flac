#ifndef _PARAM_CPP_
#define _PARAM_CPP_

#include <cstring>
#include <iostream>

#include "param.h"

pr_param get_start_paramentrs(int argc, char *argv[])
{
    pr_param result;

    const char *variants[] = {
        "-o",
        "--bps=",
        "--channels=",
        "--sample-rate=",
        "--sign=",
        "--blocksize=",
        "-d",
        "--min-residual-partition-order=",
        "--max-residual-partition-order=",
        "--max_lpc_order=",
        "--qlp_coeff_precision=",
        "--rice_parameter_search_dist=",
        "-e"
    };

    for(int i = 1; i < argc; ++i)
    {
        bool is_found = false;
        for (int j = 0; j < sizeof(variants)/sizeof(variants[0]) && !is_found; ++j)
        {
            if (strstr(argv[i], variants[j]) == argv[i])
            {
                if (j == 0)
                    result.output_file = argv[++i];
                else if (j == 1)
                    result.bps = std::atoi(argv[i] + strlen(variants[j]));
                else if (j == 2)
                    result.num_channels = std::atoi(argv[i] + strlen(variants[j]));
                else if (j == 3)
                    result.sample_rate = std::atoi(argv[i] + strlen(variants[j]));
                else if (j == 4 && strcmp(argv[i] + strlen(variants[j]), "true") == 0)
                    result.sign = true;
                else if (j == 4 && strcmp(argv[i] + strlen(variants[j]), "false") == 0)
                    result.sign = false;
                else if (j == 5)
                    result.blocksize = std::atoi(argv[i] + strlen(variants[j]));
                else if (j == 6)
                    result.compress = false;
                else if (j == 7)
                    result.min_residual_partition_order = std::atoi(argv[i] + strlen(variants[j]));
                else if (j == 8)
                    result.max_residual_partition_order = std::atoi(argv[i] + strlen(variants[j]));
                else if (j == 9)
                    result.max_lpc_order = std::atoi(argv[i] + strlen(variants[j]));
                else if (j == 10)
                    result.qlp_coeff_precision = std::atoi(argv[i] + strlen(variants[j]));
                else if (j == 11)
                    result.rice_parameter_search_dist = std::atoi(argv[i] + strlen(variants[j]));
                else if (j == 12)
                    result.do_exhaustive_model_search = true;
                is_found = true;
            }
        }
        if (!is_found && result.input_file.empty())
            result.input_file = argv[i];
        else if (!is_found && !result.input_file.empty() && argv[i][0] != '-')
        {
            std::cout << "\n\nError!\nThere must be only one input file.\n";
            exit(1);
        }
        else if (!is_found && !result.input_file.empty() && argv[i][0] == '-')
        {
            std::cout << "\nError!\nNo such command \"" << argv[i] << "\"\n";
            exit(1);
        }
    }

    std::string suff = ".dflac";
    if (result.compress && result.output_file.empty())
        result.output_file = result.input_file + suff;
    else if (!result.compress && result.output_file.empty())
    {
        result.output_file = result.input_file;
        result.output_file.erase(result.output_file.find(suff), suff.length());
    }

    return result;
}

#endif