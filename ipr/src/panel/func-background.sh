function background {
    parent=$1
    echo "parent pid = $parent  PANEL_DEV = $PANEL_DEV"
    while true; do
        while read line; do
            if [ ! "$line" = "" ]; then
                IFS=',' arr=($line)
                cmd=${arr[0]}
                key=${arr[1]}
                if [ "$cmd" = "btn-p" ]; then
                    case "$key" in
                        "up")
                            kill -s 41 $parent
                            ;;
                        "down")
                            kill -s 42 $parent
                            ;;
                        "left")
                            kill -s 43 $parent
                            ;;
                        "right")
                            kill -s 44 $parent
                            ;;
                        "ok")
                            kill -s 45 $parent
                            ;;
                        "back")
                            kill -s 46 $parent
                            ;;
                    esac
                    sleep 0.3
                fi
            fi
        done < $PANEL_DEV
    done
}
