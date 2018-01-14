#!/bin/bash -x
logger "[box] Installing none conwin dependencies ..."

function install_node() {
    folder=$JTMP/download/deps
    [ -d "$folder" ] && rm -rf $folder
    [ ! -d "$folder" ] && mkdir -p $folder
    cd $folder
    file="box2-node.tgz"
    wget "http://cos.conwin.cn/download/box2/$file"
    tar -xf $file
    target=/usr/bin/node
    [ -f "$target" ] && rm $target
    mv box2-node-v4.2.2-linux-x64 $target
}

function install_node_modules() {
    folder=$JTMP/download/deps
    [ -d "$folder" ] && rm -rf $folder
    [ ! -d "$folder" ] && mkdir -p $folder
    cd $folder
    file="box2-node_modules.tgz"
    wget "http://cos.conwin.cn/download/box2/$file"
    tar -xf $file
    [ -d "$JROOT/node_modules" ] && rm -rf $JROOT/node_modules
    mv ./node_modules $JROOT
}

function install_dss7016_sdk_lib_64bit() {
    if [ -d "$JROOT/DSS7016_SDK_Lib_64bit" ]; then
        return
    fi
    folder=$JTMP/download/deps
    [ -d "$folder" ] && rm -rf $folder
    [ ! -d "$folder" ] && mkdir -p $folder
    cd $folder
    file="DSS7016_SDK_Lib_64bit.tar.gz"
    wget "http://cos.conwin.cn/download/box2/$file"
    tar -zxvf $file
    target=$JROOT/DSS7016_SDK_Lib_64bit
    [ -d "$target" ] && rm -rf $target
    mv ./DSS7016_SDK_Lib_64bit $JROOT
}

## install node

need_install=true

[ ! "$(which node)" = "" ] && [ "$(node -v)" = "v4.2.2" ]  && need_install=false

if [ "$need_install" = "true" ]; then
    install_node
    
fi

## install node_modules
install_node_modules
install DPSDK_DSS7016_SDK_Lib_64bit
install_dss7016_sdk_lib_64bit

