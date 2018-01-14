#!/bin/bash
logger "[box] Installing conwin box ..."
logger "[box] PATH=$PATH"
logger "[box] env = $(export)"
export PATH=$PATH:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
apt-get -y install  htop tmux wget  imagemagick graphicsmagick autossh socat build-essential moreutils xinetd
sed -i 's/\(disable\s*=\s*\)yes/\1no/' /etc/xinetd.d/time


if [ -d "/usr/lib/box" ]; then
    logger "[box] Save current box to box_last"
    [ -d "/usr/lib/box_last" ] && rm -rf /usr/lib/box_last
    mv /usr/lib/box /usr/lib/box_last
fi
mkdir /usr/lib/box
cp -fRLH * /usr/lib/box
cp -f etc/rc.local /etc/rc.local
# cp etc/box.json /etc/box.json
[ ! -f "/etc/box.json" ] && cp etc/box.json /etc/box.json
if [ "$(cat /etc/network/interfaces | grep conwin)" = "" ]; then
    logger "[box] Init network settings"
    cp etc/network/interfaces /etc/network
    cp etc/network/eth /etc/network/eth0
    cp etc/network/eth /etc/network/eth1
    sed -i 's/ethx/eth0/' /etc/network/eth0
    sed -i 's/ethx/eth1/' /etc/network/eth1
fi

userdel conwin
useradd conwin -s /usr/bin/conwin-rescue
echo conwin:cn8000 | chpasswd
chmod +w /etc/sudoers
user=$(cat /etc/sudoers | grep "conwin")
if [ "$user" = "" ]; then
    echo "conwin  ALL=(ALL:ALL) ALL" >> /etc/sudoers
fi
chmod -w /etc/sudoers


ln -f /usr/local/bin/node /usr/bin/node
ln -fs /usr/lib/box/dashboard/bin/dashboard /usr/bin/dashboard
ln -fs /usr/lib/box/box/box.js /usr/bin/box
ln -fs /usr/lib/box/fs/fs.js /usr/bin/fs

ln -fs /usr/lib/box/bin/box-net-get /usr/bin/box-net-get
ln -fs /usr/lib/box/bin/box-net-set /usr/bin/box-net-set
ln -fs /usr/lib/box/bin/box-net-stat /usr/bin/box-net-stat
ln -fs /usr/lib/box/bin/box-net-restart /usr/bin/box-net-restart

ln -fs /usr/lib/box/bin/jnode-get /usr/bin/jnode-get
ln -fs /usr/lib/box/bin/udp_boxinfo /usr/bin/udp_boxinfo
ln -fs /usr/lib/box/bin/boxinfo /usr/bin/boxinfo
ln -fs /usr/lib/box/bin/box-clean /usr/bin/box-clean
ln -fs /usr/lib/box/bin/boxset /usr/bin/boxset
ln -fs /usr/lib/box/bin/box-start /usr/bin/box-start
ln -fs /usr/lib/box/bin/box-stop /usr/bin/box-stop
ln -fs /usr/lib/box/bin/dashboard-start /usr/bin/dashboard-start
ln -fs /usr/lib/box/bin/fs-start /usr/bin/fs-start
ln -fs /usr/lib/box/bin/box-restart /usr/bin/box-restart
ln -fs /usr/lib/box/bin/netinfo /usr/bin/netinfo
ln -fs /usr/lib/box/bin/start.sh /usr/bin/start.sh

cp -f /usr/lib/box/bin/json /usr/bin/json
cp -f /usr/lib/box/bin/conwin-rescue /usr/bin/conwin-rescue

logger "[box] install node modules"
logger "[box] python = $(which python)"
logger "[box] npm    = $(which npm)"
logger "[box] g++    = $(which g++)"
cd /usr/lib/box 
[ -f "node_modules.v2.tgz" ] && rm node_modules.v2.tgz
wget http://cos.conwin.cn/download/box/node_modules.v2.tgz
tar -xf node_modules.v2.tgz
# npm install > /var/log/npm.log
# mv misc/bcrypt node_modules/
# cd node_modules/bcrypt
# npm install
cd /usr/lib/box 


sed -i "s/\(start *= *\).*/\1start.sh/"  /etc/cwcdn.conf
logger "[box] Install finished"

port=$(cat /etc/cwcdn.conf | grep broadcast | awk -F = '{print $2}')
if [ "$port" = "" ]; then
    echo "broadcast=8002" >> /etc/cwcdn.conf
fi

eval $(json -f /etc/box.json .server_fs_root/ftproot)
echo ftproot=$ftproot
[ ! -d $ftproot ] && mkdir -p $ftproot
exit 0
