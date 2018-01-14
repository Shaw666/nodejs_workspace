#/bin/bash
lcd_clear

function list_update {
    start=0
    end=8
    for ((i=$start;i<$end;i++)); do
        n=$(expr $i + 1)
        if [[ $i -lt $list_item_cnt ]]; then
            item=${list_items[i]}
            item="$n $item"
        else
            item=""
        fi
        text=$(rpad $item)
        if [[ $list_item_cur -eq i ]]; then
            inverse='t'
        else
            inverse='f'
        fi
        n=$(expr $i - $list_window_top)
        lcd_print $n 0 $inverse $text
    done
}
list_items=($(ls /etc/network/restore -r));
list_cmds=($(ls /etc/network/restore -r));
list_item_cnt=${#list_items[@]}
list_item_cur=0
list_window_top=0

while true; do
    if [ ! "$list_selected" = "" ]; then
        echo "$list_selected selected"
        cmd=${list_cmds[$list_selected]};
        if [ "$cmd" = "back" ]; then
            key_pressed="true"
            key_last="back"
        elif [ ! "$cmd" = "" ]; then
            echo "exec cmd : $cmd"
            is_menu=$(echo $cmd | grep ".*\.menu$")
            if [ ! "$is_menu" = "" ]; then
                list_stack[${#list_stack[*]}]="$list_current/$list_item_cur/$list_window_top"
                list_current=$cmd
                load_menu
            else
                if [ -x $cmd ]; then
                    . $cmd
                fi
            fi
        fi
        list_selected=""
    fi
    list_update

    idle_last=$(date '+%s')
    while [ "$key_pressed" = "" ]; do
        n=$(date '+%s')
        idle=$(expr $n - $idle_last)
        if [[ $idle -gt 30 ]]; then
            return;
        fi
        sleep 0.1
    done
    echo "key_pressed : $key_last"
    case $key_last in
        "up")
            if [[ $list_item_cur -gt 0 ]]; then
                list_item_cur=$(expr $list_item_cur - 1)
                if [[ $list_item_cur -lt $list_window_top ]]; then
                    list_window_top=$list_item_cur
                fi

            fi
            ;;
        "down")
            if [[ $list_item_cur -lt $(expr $list_item_cnt - 1) ]]; then
                list_item_cur=$(expr $list_item_cur + 1)
                n=$(expr $list_window_top + 8)
                if [[ $list_item_cur -ge $n ]]; then
                    list_window_top=$(expr $list_window_top + 1)
                fi
            fi
            ;;
        "ok")
            i=$list_item_cur
            path="/etc/network/restore/${list_items[$i]}"
            echo "selected: $list_item_cur, $path"
            cp $path/eth0 /etc/network/eth0
            lcd_clear
            lcd_print 2 10 f "Reboot......"
            reboot
            key_pressed=""
            key_last=""
            return;
            ;;
        "back")
            key_pressed=""
            key_last=""
            return;
    esac
    key_pressed=""
    key_last=""
done
