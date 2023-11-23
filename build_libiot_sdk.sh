#!/bin/bash
echo "******************** Build ali-smartliving-device-sdk-c lib START ********************"
cd ../$1/ali-smartliving-device-sdk-c
make distclean

echo IDF_TARGET = $2

if [ $2 == esp32 ]; then
    number=2
elif [ $2 == esp32c3 ]; then
    number=3
elif [ $2 == esp32c6 ]; then
    number=4
elif [ $2 == esp32s2 ]; then
    number=5
elif [ $2 == esp32s3 ]; then
    number=6
elif [ $2 == esp8266 ]; then
    number=7
fi

make reconfig<<<${number}
make
echo "******************** Build ali-smartliving-device-sdk-c lib END **********************"

result_0=$(grep "CONFIG_IDF_TARGET=" $3/sdkconfig)
result_1=$(echo ${result_0} | grep "${IDF_TARGET}")
if [[ "$result_1" == "" ]]; then
    rm $3/sdkconfig
    echo RM sdkconfig!
fi
