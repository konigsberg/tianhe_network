export PATH='/home/yaokang/gnuplot/bin:/root/.composer/vendor/bin:/usr/bin/anaconda2/bin:/usr/local/cuda/bin:/usr/lib64/qt-3.3/bin:/usr/local/bin:/bin:/usr/bin:/usr/local/protobuf/bin/:/usr/local/sbin:/usr/sbin:/sbin:/usr/local/MATLAB/R2014b/bin:/usr/local/node-v4.2.3-linux-x86//bin:/home/zhangyi/omnetpp-5.1.1/bin:/home/zhangyi/bin'
export LD_LIBRARY_PATH='/usr/local/cuda/lib64:/usr/local/cuda/extras/CUPTI/lib64:/usr/lib64/atlas/:/usr/local/cuda-8.0/lib64'

rm globalParam.h
rm omnetpp.ini
python config.py

make clean
make MODE=debug CONFIGNAME=gcc-debug all
./th2net -r 0 -u Cmdenv -c THFatTree --debug-on-errors=true omnetpp.ini
