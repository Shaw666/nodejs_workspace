function load_menu {
    echo "[load-menu] current menu is : $menu_current"
    menu_items=()
    menu_cmds=()
    ss=$(cat $menu_current | awk -F ";" '{printf $1 "|"}')
    OLD_IFS=$IFS
    IFS="|"
    for s in $ss; do
        menu_items[${#menu_items[*]}]=$s
    done
    ss=$(cat $menu_current | awk -F ";" '{printf $2 "|"}')
    for s in $ss; do
        menu_cmds[${#menu_cmds[*]}]=$s
    done
    menu_item_cnt=${#menu_items[*]}
    menu_item_cur=0
    menu_window_top=0
    IFS=$OLD_IFS
    return 0
}
