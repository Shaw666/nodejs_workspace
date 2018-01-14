lcd_clear
terminated="false"
while [ ! "$terminated" = "true" ]; do
    ip=$(ifconfig  | grep -v "127.0.0.1" | awk -F '[: ]+' '/inet addr/{print $4;}')
    ip=$(rpad $ip)
    . $JRAM/system.stat
    WEB_PORT=$(cat $JSYS/etc/config.json | json .web-port);
    gw=$(route -n | awk '/^0.0.0.0/{print $2}')
    mask=$(ifconfig eth0 | grep "Mask" | cut -d ":" -f 4)
    modules_model=$(find $JFLAGS/module* | sed 's/.*module_.*_\(.*\)/\1/')
    modules_tid=$(find $JFLAGS/module* | sed 's/.*module_\(.*\)_.*/\1/')
    line_model="MODEL: $SYS_MODEL"
    line_tid="||TID: $SYS_SN[$modules_tid]"
    [ ! "$modules_model" = "" ] && line_model="$line_model [$modules_model]"
    [ ! "$modules_tid" = "" ] && line_tid="$line_tid [$modules_tid]"
    lcd_print 0 0 f $line_model
    lcd_print 1 0 f $line_tid
    lcd_print 2 0 f "||VER: $SYS_VERSION"
    lcd_print 3 0 f "|||OS: $SYS_OS"
    lcd_print 4 0 f "|||IP: $ip"
    lcd_print 5 0 f "|||GW: $gw"
    lcd_print 6 0 f "|MASK: $mask"
    lcd_print 7 0 f "||WEB: $WEB_PORT"

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
