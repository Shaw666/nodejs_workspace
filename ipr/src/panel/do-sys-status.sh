terminated="false"
lcd_clear
while [ ! "$terminated" = "true" ]; do
    . $JRAM/system.stat 
    if [ -f $EVENT_LOG_FILE ]; then
        event=$(tail -n 1 $LATEST_EVENT_LOG_FILE)
        event_time=$(echo $event | awk -F ";" '{print $1}')
        event_cid=$(echo $event | awk -F ";" '{print $3}')
        event_from=$(echo $event | awk -F ";" '{print $7}')
        event_from_line=$(echo $event | awk -F ";" '{print $2}')
        event_from_port=$(echo $event | awk -F ";" '{print $6}')
    else
        event=""
        event_from=""
        event_time=""
        event_cid=""
        event_from_line=""
        event_from_port=""
    fi
    

    serial_fail=$SYS_SERIAL_FAIL
    model=$SYS_MODEL
    net_fail=$SYS_NETWORK_FAIL
    if [ "$serial_fail" = "true" ]; then
        serial_fail="FAIL"
    else
        serial_fail="OK"
    fi
    if [ "$net_fail" = "true" ]; then
        net_fail="FAIL"
    else
        net_fail="OK"
    fi
    
    lcd_print 0 0 f "CONWIN $model : $(date '+%Y-%m-%d %H:%M:%S')"

    msg="[$event_from_port/$event_from_line] $event_from"
    lcd_print 2 0 f "FROM : $msg"
    lcd_print 3 0 f "|CID : $event_cid"
    lcd_print 4 0 f "TIME : $event_time"

    lcd_print 7 0 f "COM:[$serial_fail][$PENDING_EVENT/$PENDING_STATUS/$EVENT_LOG_LENGTH] NET:[$net_fail][$TOTAL_ONLINE]"

    sleep 0.5
    if [ ! "$key_pressed" = "" ]; then
        if [ "$key_last" = "back" ]; then
            terminated="true"
        fi
        key_last=""
        key_pressed=""
    fi
done
