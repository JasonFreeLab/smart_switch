#!/bin/bash
echo "******************** Build ali-smartliving-device-sdk-c lib START ********************"

echo IDF_TARGET = $2

cd ../$1/ali-smartliving-device-sdk-c

if [ -f ".config" ]; then
    result=$(grep "# MODEL  :" .config)
else
    result="0"
fi

result_1=$(echo ${result} | grep "${IDF_TARGET}")
if [[ "$result_1" == "" ]]; then
    echo Rebuild ali-smartliving-device-sdk-c lib!

    make distclean

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
else
    echo Do nothing!
fi

echo "******************** Build ali-smartliving-device-sdk-c lib END **********************"

result=$(grep "CONFIG_IDF_TARGET=" $3/sdkconfig)
result_1=$(echo ${result} | grep "${IDF_TARGET}")
if [[ "$result_1" == "" ]]; then
    rm $3/sdkconfig
    echo RM sdkconfig!
fi
