#!/bin/sh

pid=
function clean_up () {
    [ ! "$pid" = "" ] && kill $pid
    exit
}
trap "clean_up" SIGTERM SIGINT

echo "APP_NAME = $APP_NAME"
echo "APP_HOME = $APP_HOME"
cd bin
if [ -f "panel.sh" ]; then
    while true ; do
        echo "====$(date)========  start $APP_NAME  =============="
        bash panel.sh &
        pid=$!
        wait $pid
        sleep 1
    done
fi
