add_definitions(
        -Wno-char-subscripts
)
include_directories(
    .
)
add_library(sha1 STATIC
        ./sha1.c
)

install(TARGETS sha1 DESTINATION lib)
install(FILES sha1.h DESTINATION include)

