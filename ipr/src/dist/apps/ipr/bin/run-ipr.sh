#!/bin/sh
pid=
function clean_up () {
    [ ! "$pid" = "" ] && kill $pid
    exit
}
trap "clean_up" SIGTERM SIGINT

echo "conwin init of ipr"

echo "APP_NAME = $APP_NAME"
echo "APP_HOME = $APP_HOME"

cd $APP_HOME/lib
if [ -f "ipr-main.js" ]; then
    while true ; do
        echo "====$(date)========  start $APP_NAME  =============="
        node ipr-main.js &
        pid=$!
        wait $!
        sleep 1
    done
fi
