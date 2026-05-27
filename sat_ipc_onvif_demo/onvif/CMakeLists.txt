add_definitions(
        -Wno-format
)
include_directories(
    .
    $(CMAKE_CURRENT_SOURCE_DIR)/../install/include
)


add_library(onvif STATIC
        onvif.c
    #    onvif_io.c
        onvif_user_token.c
        sat_user_common.c
	    sat_ipcamera.c
 )
