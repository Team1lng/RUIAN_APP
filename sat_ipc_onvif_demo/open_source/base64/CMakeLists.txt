add_definitions(
)
include_directories(
    .
)
add_library(base64 STATIC
        ./lib/lib.c
        ./lib/codec_choose.c
        ./lib/tables/tables.c
        ./lib/arch/avx2/codec.c
        ./lib/arch/neon32/codec.c
        ./lib/arch/neon64/codec.c
        ./lib/arch/ssse3/codec.c
        ./lib/arch/sse41/codec.c
        ./lib/arch/sse42/codec.c
        ./lib/arch/avx/codec.c
        ./lib/arch/generic/codec.c
)


# install(TARGETS base64 DESTINATION lib)
# install(FILES libbase64.h DESTINATION include)
