#/bin/bash
file=$JSYS/etc/config.json
cat $file | json -I 4 .web-port=8000 .web-pass=admin | sponge $file
