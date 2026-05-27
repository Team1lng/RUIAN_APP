add_definitions(
)
include_directories(
    .
)
add_library(md5 STATIC
        ./md5.c
)


install(TARGETS md5 DESTINATION lib)
install(FILES md5.h DESTINATION include)
