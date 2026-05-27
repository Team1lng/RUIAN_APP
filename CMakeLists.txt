cmake_minimum_required(VERSION 2.8)# cmake最低使用版本
# set(CMAKE_C_COMPILER "/opt/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-gcc")
# set(CMAKE_C_COMPILER "/home/cyy/share/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-gcc")
# set(CMAKE_CXX_COMPILER "/home/cyy/share/arm-anykav500-linux-uclibcgnueabi/bin/arm-anykav500-linux-uclibcgnueabi-g++")
set(CMAKE_C_COMPILER "arm-anykav500-linux-uclibcgnueabi-gcc")
set(CMAKE_CXX_COMPILER "arm-anykav500-linux-uclibcgnueabi-g++")
# 设置编译器，此处为ARM平台，如果你安装了编译链就用名称，没安装请用绝对路径
SET(leo_proj ../ANYKA37E.BIN)
add_definitions(-DLV_LVGL_H_INCLUDE_SIMPLE)
add_definitions(
	-D__LITTLE_ENDIAN__
	-D_GNU_SOURCE
	#7寸屏注释_PLATFORM_800_1280和_PLATFORM_8inch，10寸屏注释_PLATFORM_8inch，8寸屏不需要注释

	 
	#新wifi模块ws73v100
	# -DWS73V100_WIFI
	-DTUYA_AUDIO_SAMPLE=16000
)

project(${leo_proj})

# 工程名
SET(CMAKE_BUILD_TYPE "Release")
SET(CMAKE_C_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -s -Wall -std=gnu11  -w")#-Werror
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3  -s -Wall -std=c++1y -w")#-Werror

# 工程名

# 添加src目录
add_subdirectory(src)
