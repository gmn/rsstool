#!/bin/bash
EE() {
    echo
    echo "$@"
    echo
    $@
}
EE rss update
FILE=/var/www/feeds/feed_$(date "+%Y%m%d_%H%M").html
echo
echo "rss show -n -H > $FILE"
echo
rss show -n -H > $FILE
chmod 755 $FILE
