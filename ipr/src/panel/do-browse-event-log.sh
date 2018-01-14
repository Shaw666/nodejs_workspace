echo "this is do-browser-event-log.sh"
lcd_clear
terminated="false"
event_current=1
event_total=$(cat $EVENT_LOG_FILE | wc -l)
while [ ! "$terminated" = "true" ]; do
    now=$(date '+%Y-%m-%d %H:%M:%S')
    now=$(rpad $now)
    lcd_print 0 0 f "TIME: $now"
    event=$(tail -n $event_current $LATEST_EVENT_LOG_FILE | head -n 1)
    event_time=$(echo $event | awk -F ";" '{print $1}')
    event_line=$(echo $event | awk -F ";" '{print $2}')
    event_cid=$(echo $event | awk -F ";" '{print $3}')
    event_protocol=$(echo $event | awk -F ";" '{print $5}')
    event_port=$(echo $event | awk -F ";" '{print $6}')
    event_ip=$(echo $event | awk -F ";" '{print $7}')
    event_cid=$(rpad $event_cid)
    lcd_print 2 0 f "LINE: $event_line PORT: $event_port"
    lcd_print 3 0 f "|CID: $event_cid"
    lcd_print 4 0 f "TIME: $event_time"
    event_time=$(rpad $event_time)
    lcd_print 5 0 f "||IP: $event_ip"
    s="$event_current/$event_total"
    s=$(lpad $s)
    lcd_print 7 0 f "$s"
    idle_last=$(date '+%s')
    while [ "$key_pressed" = "" ]; do
        n=$(date '+%s')
        idle=$(expr $n - $idle_last)
        if [[ $idle -gt 30 ]]; then
            terminated="true"
            break;
        fi
        sleep 0.1
    done
    case $key_last in
        "down")
            if [[ $event_current -gt 1 ]]; then
                event_current=$(expr $event_current - 1)
            fi
            ;;
        "up")
            if [[ $event_current -lt $event_total ]]; then
                event_current=$(expr $event_current + 1)
            fi
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
