#!/bin/bash -x
echo "install IPR..."
echo "install PANEL..."
echo "install DSS7016_Gateway..."

[ -d "$JAPPS/ipr" ] && rm -rf $JAPPS/ipr
[ -d "$JAPPS/panel" ] && rm -rf $JAPPS/panel
[ -d "$JAPPS/dss7016_gateway" ] && rm -rf $JAPPS/dss7016_gateway

mv apps/ipr $JAPPS
mv apps/panel $JAPPS
mv apps/dss7016_gateway $JAPPS
