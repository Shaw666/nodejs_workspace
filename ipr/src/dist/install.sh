#!/bin/bash -x


# install conwin application
# 1 conwin tools like json
# 2 conwin apps

if [ -x "apps/install.sh" ]; then
    echo "apps/install.sh"
    pushd .
    apps/install.sh
    popd
fi

# install dependencies
# 3rd part component like node

if [ -x "dependencies/install.sh" ]; then
    echo "dependencies/install.sh"
    pushd .
    dependencies/install.sh
    popd
fi
exit 0
