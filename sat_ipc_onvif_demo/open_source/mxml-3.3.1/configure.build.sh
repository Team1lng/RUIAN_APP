./configure \
    --host="arm-anykav500-linux-uclibcgnueabi"  \
    --prefix="$PWD/output"



make -j12
make install
make clean
# --host: 指定目标程序运行环境，只写到第三个-前即可。
# --prefix: 指定交叉编译完成后生成的文件的存放路径，即安装位置。如果不指定则默认安装在/usr/local/目录下。



