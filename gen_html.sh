#!/bin/bash
rss update
FILE=/var/www/feeds/feed_$(date "+%Y%m%d_%H%M").html
rss show -n -H > $FILE
chmod 755 $FILE
