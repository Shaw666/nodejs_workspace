lcd_clear
terminated="false"
while [ ! "$terminated" = "true" ]; do
    . $JRAM/line.stat 
    for ((i=1;i<=8;i++)); do
        enabled=$(eval "echo \$LINE_"${i}_ENABLED"");
        prefix=$(eval "echo \$LINE_"${i}_PREFIX"");
        port=$(eval "echo \$LINE_"${i}_PORT"");
        protocol=$(eval "echo \$LINE_"${i}_PROTOCOL"");
        online=$(eval "echo \$LINE_"${i}_ONLINE"");
        if [ "$enabled" = "true" ]; then
            enabled="Enabled|"
            else
            enabled="Disabled"
        fi
        l=$(expr $i - 1)
        while [ ${#online} -lt 4 ]; do
            online="|$online"
        done
        
        line="$i $enabled[$online] $port $protocol $prefix"
        lcd_print $l 0 f "$line"
    done

    idle_last=$(date '+%s')
    while [ "$key_pressed" = "" ]; do
        n=$(date '+%s')
        idle=$(expr $n - $idle_last)
        if [[ $idle -gt 30 ]]; then
            terminated="true"
            break;
        fi
        sleep 0.3
    done
    case $key_last in
        "up")
            # if [[ $event_current -gt 1 ]]; then
            #     event_current=$(expr $event_current - 1)
            # fi
            ;;
        "down")
            # if [[ $event_current -lt $event_total ]]; then
            #     event_current=$(expr $event_current + 1)
            # fi
            ;;
        "ok")
            ;;
        "back")
            terminated="true"
            ;;
    esac
    key_last=""
    key_pressed=""
done
