Compressor based on flac
======================================

This is the simple implementation of flac compressor. You can find information about it [here](https://xiph.org/flac/).
Also you can find code of flac [here](https://github.com/xiph/flac).

# File Compression

## Compile program
        cmake -S . -B "path to folder to build files"
        cmake --build "path to folder to build files"

## Compress file
        ./demo-flac input_file

## Decompress file
        ./demo-flac -d input_file

### Flags

##### Change bps of input file (by default it is 16 bit)
        --bps=x

##### Custom output file (by default it is input_file.dflac)
        -o output_file

##### Change number of channels (by default it is 1)
        --channels=x

##### Change sample rate (by default it is 44100)
        --sample-rate=x

##### Change block size (by default it is 4096)
        --blocksize=x

##### Make int signed or unsigned (by default it is signed)
        --sign=true
        --sign=false

##### Decompress flag
        -d

##### Make exhaustive search for fixed subframes
        -e

### What differences does flac have

+ Type of input files. In flac you can use different types of input files but here you can use only input files that have raw data.
+ In flac there is 4 types of subframes (constant, verbatium, fixed and lpc). Here you can find only 3 types (constant, verbatium and fixed).
