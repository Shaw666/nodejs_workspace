#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$JROOT/DSS7016_SDK_Lib_64bit/DPSDK_64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$JROOT/DSS7016_SDK_Lib_64bit/libxml2_64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$JROOT/DSS7016_SDK_Lib_64bit/things_64
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$JROOT/DSS7016_SDK_Lib_64bit/log4c_64

config_file=$JSYS/etc/gateway.json
config_default=etc/gateway.default.json
tid=""

for file in `ls $JFLAGS`; do
    result=$(echo $file | grep "0802")
    if [[ $result != "" ]]; then
        tid=$(echo $result | awk -F '_' '{print $2}')
        break
    fi
done

if [[ $tid == ""  ]]; then
    echo "dss7016 gateway module missing tid"
    exit -1
fi

if [ -f $config_file ]; then
    result=$(cat $config_file | grep "dss7016-server-ip")
    if [ "$result" != "" ]; then
        rm $config_file
    fi
fi

if [ ! -f $config_file ]; then
    cat $config_default | json .tid=$tid | json -I 4 . >> $config_file
fi

if [ ! -d "$JVAR/gateway" ]; then
    mkdir -p $JVAR/gateway
fi

monitor_pid=""

function clean_up {
    runtime_file=$JRAM/gateway-runtime.stat
    things_connected=$JRAM/flags/things-server-connected
    dss_connected=$JRAM/flags/dss7016-server-connected

    if [ -f $runtime_file ]; then
        rm $runtime_file
    fi

    if [ -f $dss_connected ]; then
        rm $dss_connected
    fi

    if [ -f $things_connected ]; then
        rm $things_connected
    fi

    if [ "$monitor_pid" != "" ]; then
	    kill -SIGTERM $monitor_pid
    fi

    exit
}

trap "clean_up" SIGTERM SIGINT

function app_disabled_monitor {
    gateway_pid=$1
    while true ; do
        if [ ! -f "$JFLAGS/gateway-enabled" ]; then
            if [ ! $gateway_pid == "" ]; then
                kill -SIGINT $gateway_pid
		        exit
            fi
        fi
        sleep 1
    done
}

cd bin
if [ -x "dss7016_gateway" ]; then
    while true ; do
        if [ -f "$JFLAGS/gateway-enabled" ]; then 
            echo "====$(date)========  start $APP_NAME  =============="
            ./dss7016_gateway &
            pid=$!
            app_disabled_monitor $pid &
	        monitor_pid=$!
            wait $monitor_pid
        fi
        sleep 1
    done

fi


