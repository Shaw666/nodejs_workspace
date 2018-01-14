#!/bin/bash
[ "$(which cleancss)"     = "" ] && sudo npm install clean-css -g
[ "$(which uglifyjs)"      = "" ] && sudo npm install uglifyjs -g
[ "$(which html-minifier)" = "" ] && sudo npm install html-minifier -g
[ "$(which gzip)"          = "" ] && sudo apt-get install gzip

base=$1
[ "$base" = '' ] && echo "missing base dir" && exit

[ ! -d tmp ] && mkdir tmp
[ -d "build/$base" ] && rm -rf build/$base
mkdir -p build/$base

while read file
do
	ext=$(echo $file | sed "s/.*\.\(.*\)$/\1/")
	type=$(cat tools/mime.xref | grep "\.$ext " | awk '{print $2}')
	[ "$type" = "" ] && type="application/octet-stream"
	[ ! -f $file ] && continue
	[ "$file" = "" ] && continue
	fs_total=$(expr $fs_total + 1)
	working_file="tmp/working.tmp~"
    echo $type
	case $type in
	    text/html)
	        cat $file | html-minifier \
	                        --collapse-whitespace \
	                        --minify-js --minify-css \
                            --remove-comments \
                            --collapse-boolean-attributes \
                            --remove-attribute-quotes \
                            --remove-redundant-attributes \
                            --use-short-doctype \
                            --remove-empty-attributes \
                            --remove-script-type-attributes \
                            --remove-style-link-type-attributes \
                            --remove-optional-tags \
                            --minify-urls  > $working_file
            ;;
        application/x-javascript)
            cat $file | uglifyjs > $working_file
            ;;
        text/css)
            cat $file | cleancss --s0 > $working_file
            ;;
        *)
            cp -f $file $working_file
    esac
    size=$(stat -c %s $working_file)
    printf "processing [%8d] %s ...\n"  $size $file
    size_total=$(expr $size_total + $size)

	path=${file#src/$base}
    path=$(dirname $path)
    filename=$(basename $file)
    [ -d build/$base/$path ] || mkdir -p build/$base/$path
    mv $working_file build/$base/$path/$filename
done <<< "$(find src/$base | grep -v ".*~")"
