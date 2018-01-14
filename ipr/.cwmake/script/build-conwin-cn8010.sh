#!/bin/sh
# cwmake build script for conwin/cn8010
# copy all file to <git repo>/.tmp
# current path is root of <git repo>

echo "do the build action...."

cp -rfL src/dist/* .tmp/
find .tmp | grep ".*~" | xargs rm

