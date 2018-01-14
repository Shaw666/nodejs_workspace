function lcd_print {
    row=$1
    col=$2
    attr=$3
    text=${@:4}
    text=$(rpad $text)
    cmd="text,$row/$col/$attr,$text"
    cmd=${cmd//|/ }
    # echo "[$cmd]"
    echo "$cmd" > $PANEL_DEV
    
}
function lcd_clear {
    echo "[clear]"
    echo "clear" > $PANEL_DEV
}

