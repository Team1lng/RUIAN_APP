#!/bin/bash


#COLORS
CDEF="\033[0m"
C_RED="\033[0;31m"
C_GREEN="\033[0;32m"
C_YELLO="\033[0;33m"
C_BLUE="\033[0;34m"
C_PINK="\033[0;35m"
C_CYAN="\033[0;36m"
LOG_WHITE() { echo -e "${CDEF}"    "[${FUNCNAME[1]},${BASH_LINENO}] ${@} ${CDEF}" ;}
LOG_RED()   { echo -e "${C_RED}"   "[${FUNCNAME[1]},${BASH_LINENO}] ${@} ${CDEF}" ;}
LOG_GREEN() { echo -e "${C_GREEN}" "[${FUNCNAME[1]},${BASH_LINENO}] ${@} ${CDEF}" ;}
LOG_YELLO() { echo -e "${C_YELLO}" "[${FUNCNAME[1]},${BASH_LINENO}] ${@} ${CDEF}" ;}
LOG_BLUE()  { echo -e "${C_BLUE}"  "[${FUNCNAME[1]},${BASH_LINENO}] ${@} ${CDEF}" ;}
LOG_PINK()  { echo -e "${C_PINK}"  "[${FUNCNAME[1]},${BASH_LINENO}] ${@} ${CDEF}" ;}
LOG_CYAN()  { echo -e "${C_CYAN}"  "[${FUNCNAME[1]},${BASH_LINENO}] ${@} ${CDEF}" ;}



PROJPATH=$(cd $(dirname $0);pwd -P) # 工程路径
PROJNAME=${projPath##*/}      # 工程文件夹名

BUILDDIR="_build"

#####################################################################

make_all(){

    cd $PROJPATH

    mkdir -p $BUILDDIR
    cd $BUILDDIR
    cmake ..
    make -j12
    
    cd $PROJPATH
}





make_clean(){

    cd $PROJPATH

    # 清除编译产生的中间文件
    find . -name "$BUILDDIR" | xargs rm -rf
    find . -name "Makefile"  | xargs rm -rf
    find . -name "CMakeFiles" | xargs rm -rf
    find . -name "CMakeCache.txt" | xargs rm -rf
    find . -name "cmake_install.cmake" | xargs rm -rf
}


make_install(){
    cd $PROJPATH
    cd $BUILDDIR

    make install
}

make_lib(){ # [1] 路径

# """
#     # 需要先修改CmakeLists.txx
#     add_definitions(
#     )
#     include_directories(
#         .
#     )
#     add_library(md5 STATIC
#             ./md5.c
#     )

#     # 这里指定需要安装的文件
#     install(TARGETS md5 DESTINATION lib)
#     install(FILES md5.h DESTINATION include)

# """
    cd $SHELL_FOLDER
    if [ ! -d $1 ];then
        echo "PWD=$PWD"
        echo "'$1' not exists!!!"
        exit
    fi

    cd $1

    # 清除编译产生的中间文件
    rm -rf $buildDir

    rm -rf CMakeFiles
    rm -rf cmake_install.cmake
    rm -rf CMakeCache.txt
    rm -rf Makefile

    if [ -f configure ];then
        ./configure \
            --host="arm-anykav500-linux-uclibcgnueabi"  \
            --prefix="/home/xiaole/smb/ipc_onvif_demo/share"

        make -j12
        make install
        make clean

    else
        mkdir -p $buildDir
        cd $buildDir
        cmake .. -DCMAKE_C_COMPILER="arm-anykav500-linux-uclibcgnueabi-gcc" \
                 -DCMAKE_C_STANDARD="11"  \
                 -DCMAKE_INSTALL_PREFIX="/home/xiaole/smb/ipc_onvif_demo/share"

        make -j12
        make install


    fi


}


build_lib(){
    make_lib open_source_libraries/base64
    make_lib open_source_libraries/md5
    make_lib open_source_libraries/mxml-3.3.1
    make_lib open_source_libraries/sha1
}





main () {
    case ${1} in
        -ALL|ALL|-all|all)
            LOG_CYAN "MAKE ALL"
            make_all
        ;;

        -clean|clean)
            LOG_CYAN "MAKE CLEAN"
            make_clean
        ;;

        -install|install)
            LOG_CYAN "MAKE install"
            make_install
        ;;
        
        *)
            LOG_CYAN "####################### MENU #######################"
            LOG_CYAN "$0          MENU"
            LOG_CYAN "$0 -ALL...  编译全部"
            LOG_CYAN "$0 -clean   删除编译生成的文件"
            LOG_CYAN "$0 -install 安装所需的文件"
            LOG_CYAN "###################################################"
            
        ;;
    esac
    
    
    
    
}


main $*
