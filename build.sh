# 将其放置工程文件夹目录下，运行即可编译  # 
# 作者:葛小乐
########################################


#!/bin/bash

# 工程文件夹路径
project_path=$(cd $(dirname $0);pwd -P)
project_name="${project_path##*/}"

# 打印第二行,匹配第六项，得到当前CMake使用的路径
cmake_folder_path=`awk 'NR==2{print $6}' CMakeCache.txt`

# 最终编译目标
build_target=ANYKA37EOS

# 选择屏幕尺寸
select_screen_size() {
    echo "====================="
    echo "  请选择屏幕尺寸"
    echo "====================="
    echo "1) 7寸屏 (1024x600) "
    echo "2) 8寸屏"
    echo "3) 10寸屏 (800x1280) "
    echo "====================="
    read -p "请输入数字[1-3]:" choice

    # 删除旧屏参宏
    sed -i '/-D_PLATFORM_800_1280/d' CMakeLists.txt
    sed -i '/-D_PLATFORM_8inch/d' CMakeLists.txt

    if [ "$choice" = "1" ]; then
        echo "已选择:7寸屏"
        cp -v "$project_path/dtb/7/EVB_CBDM_AK3760E_V1.0.1.dtb" "$project_path/platform/upgrade/EVB_CBDM_AK3760E_V1.0.1.dtb"
    elif [ "$choice" = "2" ]; then
        echo "已选择:8寸屏"
        sed -i '/^add_definitions($/a\	-D_PLATFORM_8inch' CMakeLists.txt
        cp -v "$project_path/dtb/7/EVB_CBDM_AK3760E_V1.0.1.dtb" "$project_path/platform/upgrade/EVB_CBDM_AK3760E_V1.0.1.dtb"
    elif [ "$choice" = "3" ]; then
        echo "已选择:10寸屏"
        sed -i '/^add_definitions($/a\	-D_PLATFORM_800_1280' CMakeLists.txt
        cp -v "$project_path/dtb/10.1/EVB_CBDM_AK3760E_V1.0.1.dtb" "$project_path/platform/upgrade/EVB_CBDM_AK3760E_V1.0.1.dtb"
    else
        echo "输入错误,默认使用7寸屏"
        sed -i '/^add_definitions($/a\	-D_PLATFORM_800_1280' CMakeLists.txt
        cp -v "$project_path/dtb/7/EVB_CBDM_AK3760E_V1.0.1.dtb" "$project_path/platform/upgrade/EVB_CBDM_AK3760E_V1.0.1.dtb"
    fi
}

# 编译主程序
function auto_build()
{
    cp src/layout/resource/rom.bin .
    cp -v src/layout/resource/font/sat_leo.ttf ./

    # 检查当前路径与CMake缓存路径是否一致
    # 不一致时需要重新生成CMake配置
    if [ $project_path != $cmake_folder_path ];then
        echo "folder path is not same!"
        rm -v CMakeCache.txt 
        cmake .
        make
    else
        echo "folder path same"
        make
    fi
    
     # 对二进制文件进行strip操作，去除调试信息
    arm-linux-strip ANYKA37E.BIN 
   
    #cp -v src/layout/resource/font/sat_leo.ttf ./
    # cp -v src/layout/resource/rom.bin ./
    

    # 开启粗体文本显示
    echo -e "\033[1m"

    # echo "tar -zcvf ANYKA37EOS ANYKA37E.BIN rom.bin sat_leo.ttf"
    # # cp -r ./src/api/xls/language.xls .
    # tar -zcvf ANYKA37EOS ANYKA37E.BIN  sat_leo.ttf 
    tar -zcvf ANYKA37EOS ANYKA37E.BIN rom.bin sat_leo.ttf language.xls
    # rm -rf language.xsl

    # echo "tar -zcvf firmware ./platform"
    # tar -zcvf firmware ./platform
    # rm -v rom.bin sat_leo.ttf

   # 关闭文本格式设置
    echo -e "\033[0m"

}

# cyy：资源文件打包函数
function resource_file_pack() {

    # 同样检查路径并执行CMake构建
    if [ $project_path != $cmake_folder_path ];then
        echo "folder path is not same!"
        rm -v CMakeCache.txt 
        cmake .
        make
    else
        echo "folder path same"
        make
    fi
    # cp -v src/layout/resource/rom.bin ./

	# #cp -r src/layout/resource/rings/ ./
	# cp -v src/layout/resource/font/sat_leo.ttf ./

	# tar -zcvf ANYKA37EOS ANYKA37E.BIN rom.bin sat_leo.ttf  #rings
	# rm -r rom.bin sat_leo.ttf #rings
}

function main() {

    if [ "$option" == "-all" ]; then
        select_screen_size
        auto_build
        date

        # exit 0
    elif [ "$option" == "-ui" ]; then

        resource_file_pack

        exit 0
    else
        echo "***************** <menu> ***************** "
        echo "  pack all file        ./build.sh -all"
        echo ""
        echo "  only pack resource file       ./build.sh -ui"
        echo "****************************************** "

        exit 0
    fi

}

option=$1
main


# 以下部分为独立的打包流程，在main函数执行后执行
# 将编译好的二进制文件复制到平台应用目录
cp -r ANYKA37E.BIN ./platform/app
# 生成应用文件系统压缩包
./platform/mksquashfs ./platform/app app.sqsh4 -noappend -comp xz
# 移动压缩包到升级目录
mv app.sqsh4 platform/upgrade
# 进入升级目录
cd platform/upgrade
# 删除上次残留的升级包，避免被打包进去
rm -f ANYKA37EOS
# 将所有文件打包为最终的升级包（排除自身）
tar -zcvf ANYKA37EOS --exclude=ANYKA37EOS ./*
# 移动升级包到上层目录
mv ANYKA37EOS ../../
# 返回原目录
cd -

echo "Build application layer OK!!"
# 注释掉的备用打包命令
# tar -zcvf ANYKA37EOS platform/upgrade/*