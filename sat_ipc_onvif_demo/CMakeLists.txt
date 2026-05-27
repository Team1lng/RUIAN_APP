cmake_minimum_required(VERSION 3.5.4)

# SET(CMAKE_C_COMPILER "gcc")
# SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_C_COMPILER   "arm-anykav500-linux-uclibcgnueabi-gcc")
SET(CMAKE_CXX_COMPILER "arm-anykav500-linux-uclibcgnueabi-g++")


SET(CMAKE_BUILD_TYPE "Release")		# Release/Debug
IF(CMAKE_BUILD_TYPE STREQUAL Release)
	message("================================================ BUILD TYPE : Release")
	SET(CMAKE_C_FLAGS_RELEASE   "$ENV{CXXFLAGS} -std=gnu11 -Wall -s  -Os") # -Werror
	SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -std=c++1y -Wall -s  -Os") # -Werror

	# -Os   尽量减少目标的大小,这个经常用于存储量比较小的设备
	# -Wall 显示所有警告


ELSEIF(CMAKE_BUILD_TYPE STREQUAL Debug)
	message("================================================ BUILD TYPE : Debug")
	SET(CMAKE_C_FLAGS_DEBUG    "$ENV{CXXFLAGS} -std=gnu11 -Wall  -g3 ") 
	SET(CMAKE_CXX_FLAGS_DEBUG  "$ENV{CXXFLAGS} -std=c++1y -Wall  -g3 ") 
	
	# add_compile_options(-g3)	# 加上编译选项
	# -g0表示不生成调试信息
	# -g3表示生成最多的调试信息
ENDIF()




SET(Proj demo)
project(${Proj})


include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/onvif/
	${CMAKE_CURRENT_SOURCE_DIR}/open_source/base64/include
	${CMAKE_CURRENT_SOURCE_DIR}/open_source/md5
	${CMAKE_CURRENT_SOURCE_DIR}/open_source/mxml-3.3.1/output/include
	${CMAKE_CURRENT_SOURCE_DIR}/open_source/sha1
)

add_definitions(
	-DONVIF_XML_PATH="/etc/config/onvif/"
	# -DONVIF_XML_PATH="${CMAKE_CURRENT_SOURCE_DIR}/xml_res/"
	
)

aux_source_directory(. DIR_SRCS)



set(PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(${PROJECT_PATH}/open_source/base64)
add_subdirectory(${PROJECT_PATH}/open_source/md5)
add_subdirectory(${PROJECT_PATH}/open_source/sha1)
add_subdirectory(${PROJECT_PATH}/open_source/mxml-3.3.1)
add_subdirectory(${PROJECT_PATH}/onvif)
set(LIBS ${LIBS} base64)
set(LIBS ${LIBS} md5)
set(LIBS ${LIBS} sha1)
set(LIBS ${LIBS} mxml)
set(LIBS ${LIBS} onvif)





add_executable(${Proj} ${DIR_SRCS})





set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)
set(INSTALL_FILES ${INSTALL_FILES} open_source/base64/include/libbase64.h)
set(INSTALL_FILES ${INSTALL_FILES} open_source/md5/md5.h)
set(INSTALL_FILES ${INSTALL_FILES} open_source/mxml-3.3.1/mxml-private.h)
set(INSTALL_FILES ${INSTALL_FILES} open_source/mxml-3.3.1/mxml.h)
set(INSTALL_FILES ${INSTALL_FILES} open_source/sha1/sha1.h)
set(INSTALL_FILES ${INSTALL_FILES} onvif/sat_ipcamera.h)
set(INSTALL_TARGETS ${INSTALL_TARGETS} ${LIBS})
# install(TARGETS ${INSTALL_TARGETS} DESTINATION lib)
# install(FILES ${INSTALL_FILES} DESTINATION include)





target_link_libraries(${Proj} -Wl,--start-group

	# share lib
	${LIBS}

	# stand lib
	m
	pthread
	dl

	
	-Wl,--end-group)