#!/bin/bash
EE() {
    echo
    echo "$@"
    echo
    $@
}

WEBD=/var/www/feeds/
FILE=feed_$(date "+%Y%m%d_%H%M").html
if [ -e $WEBD ]; then
    FILE=${WEBD}${FILE}
fi

EE rss update

echo
echo "rss show -n -H > $FILE"
echo
rss show -n -H > $FILE
chmod 755 $FILE

if [ -e './url' ]; then
    EE rsync -e ssh -r -v --progress --partial $FILE $(cat url)
else
    echo consider filling out destination file named: url
fi
