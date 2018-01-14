function menu_update {
    start=$menu_window_top
    end=$(expr $menu_window_top + 8)
    for ((i=$start;i<$end;i++)); do
        n=$(expr $i + 1)
        if [[ $i -lt $menu_item_cnt ]]; then
            item=${menu_items[i]}
            item="$n $item"
        else
            item=""
        fi
        text=$(rpad $item)
        if [[ $menu_item_cur -eq i ]]; then
            inverse='t'
        else
            inverse='f'
        fi
        n=$(expr $i - $menu_window_top)
        lcd_print $n 0 $inverse $text
    done
}
