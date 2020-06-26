cmake_minimum_required (VERSION 3.0)
project (Demo_flac VERSION 1.0 LANGUAGES CXX)

set(HEADER_FILES compress.h constants.h decompress.h frame.h operations.h param.h rice.h bitwriter.h)

add_library(CompressLib compress.cpp ${HEADER_FILES})
add_library(DecompressLib decompress.cpp ${HEADER_FILES})
add_library(OperationsLib operations.cpp ${HEADER_FILES})
add_library(ParamLib param.cpp ${HEADER_FILES})
add_library(RiceLib rice.cpp ${HEADER_FILES})
add_library(BitWriterLib bitwriter.cpp ${HEADER_FILES})
add_executable(demo_flac demo_flac.cpp ${HEADER_FILES})

target_link_libraries(demo_flac PRIVATE CompressLib)
target_link_libraries(demo_flac PRIVATE DecompressLib)
target_link_libraries(demo_flac PRIVATE OperationsLib)
target_link_libraries(demo_flac PRIVATE RiceLib)
target_link_libraries(demo_flac PRIVATE BitWriterLib)
target_link_libraries(demo_flac PRIVATE ParamLib)