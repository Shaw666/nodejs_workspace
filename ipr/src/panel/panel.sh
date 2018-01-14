#!/bin/sh
[ "$BOARD_DEV_LCD" = "" ] && echo "missing env BOARD_DEV_LCD" && exit

PANEL_DEV=$BOARD_DEV_LCD
EVENT_LOG_FILE=$JVAR/event.log
TTYS0_Q_FILE=$JVAR/ttys0.q
LATEST_EVENT_LOG_FILE=$JRAM/latest_event.log

function clean_up () {
    [ ! "$pid_background" = "" ] && kill $pid_background
    [ ! "$pid_beep" = "" ] && kill $pid_beep
    exit
}
trap "clean_up" SIGTERM SIGINT

stty -F $PANEL_DEV raw -echo -echoe -echok 115200

[ ! -f "$EVENT_LOG_FILE" ] && touch $EVENT_LOG_FILE
[ ! -f "$TTYS0_Q_FILE" ] && touch $TTYS0_Q_FILE
[ ! -f "$LATEST_EVENT_LOG_FILE" ] && touch $LATEST_EVENT_LOG_FILE

stty -F $PANEL_DEV 115200 -echo raw

key_pressed=""
key_last=""

. ./func-key-trap.sh
. ./func-background.sh
. ./func-beep.sh
. ./func-pad.sh
. ./func-lcd.sh
. ./func-load-menu.sh
. ./func-menu-update.sh

lcd_clear

echo "pid=$$"
background $$ &
pid_background=$!
do_beep &
pid_beep=$!


menu_items=()
menu_cmds=()
menu_item_cnt=0
menu_item_cur=0
menu_window_top=0
menu_selected=0
menu_current="menu-main.menu"
menu_stack=()

load_menu
default_menu="menu-main.menu"
default_menu_select=0

while true; do
    if [ ! "$menu_selected" = "" ]; then
        echo "$menu_selected selected"
        cmd=${menu_cmds[$menu_selected]};
        if [ "$cmd" = "back" ]; then
            key_pressed="true"
            key_last="back"
        elif [ ! "$cmd" = "" ]; then
            echo "exec cmd : $cmd"
            is_menu=$(echo $cmd | grep ".*\.menu$")
            if [ ! "$is_menu" = "" ]; then
                menu_stack[${#menu_stack[*]}]="$menu_current/$menu_item_cur/$menu_window_top"
                menu_current=$cmd
                load_menu
            else
                if [ -x $cmd ]; then
                    . $cmd
                fi
            fi
        fi
        menu_selected=""
    fi
    echo "current menu = $menu_current"
    echo "menu stack   = (${menu_stack[*]})"
    menu_update

    idle_last=$(date '+%s')
    while [ "$key_pressed" = "" ]; do
        n=$(date '+%s')
        idle=$(expr $n - $idle_last)
        if [[ $idle -gt 30 ]]; then
            menu_current=$default_menu
            menu_selected=$default_menu_select
            menu_stack=()
            load_menu
            break;
        fi
        sleep 0.1
    done
    echo "key_pressed : $key_last"
    case $key_last in
        "up")
            if [[ $menu_item_cur -gt 0 ]]; then
                menu_item_cur=$(expr $menu_item_cur - 1)
                if [[ $menu_item_cur -lt $menu_window_top ]]; then
                    menu_window_top=$menu_item_cur
                fi

            fi
            ;;
        "down")
            if [[ $menu_item_cur -lt $(expr $menu_item_cnt - 1) ]]; then
                menu_item_cur=$(expr $menu_item_cur + 1)
                n=$(expr $menu_window_top + 8)
                if [[ $menu_item_cur -ge $n ]]; then
                    menu_window_top=$(expr $menu_window_top + 1)
                fi
            fi
            ;;
        "ok")
            menu_selected=$menu_item_cur
            ;;
        "back")
            n=${#menu_stack[*]}
            if [[ $n -gt 0 ]]; then
                lcd_clear
                n=$(expr $n - 1)
                last=${menu_stack[$n]}
                unset menu_stack[$n]
                
                menu_current=$(echo $last|awk -F "/" '{printf $1}')
                menu_item_cur=$(echo $last|awk -F "/" '{printf $2}')
                menu_window_top=$(echo $last|awk -F "/" '{printf $3}')
                echo "last=$last"
                echo "menu_current=$menu_current"
                echo "menu_item-cur=$menu_item_cur"
                
                load_menu
            else
                echo "top menu"
            fi
            ;;
    esac
    key_pressed=""
    key_last=""
done
