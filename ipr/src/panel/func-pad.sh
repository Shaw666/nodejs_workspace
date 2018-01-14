function rpad {
    s=$*
    while [ ${#s} -lt 40 ]; do
        s="$s "
    done
    echo "$s"
}

function lpad {
    s=$*
    while [ ${#s} -lt 40 ]; do
        s=" $s"
    done
    echo "$s"
    sleep 0.01
}

