function do_beep() {
    while true; do
        . $JRAM/system.stat
	if [ -f "$JFLAGS/monitor-beep-serial" ]; then
            if [ "$SYS_SERIAL_FAIL" = "true" ]; then
		beep -f 1000 -l 500
            fi
	fi

	if [ -f "$JFLAGS/monitor-beep-net" ]; then
            if [ "$SYS_NETWORK_FAIL" = "true" ]; then
		beep -f 3000 -l 500
            fi
	fi
        sleep 1;
    done
}
