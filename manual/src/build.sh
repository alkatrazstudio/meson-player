#!/usr/bin/env bash
set -e
cd "$(dirname -- "${BASH_SOURCE[0]}")"

DISTDIR=../dist
rm -rf "$DISTDIR"
mkdir "$DISTDIR"

for FILE in *.php
do
    BASE="$(basename -s.php $FILE)"
    if [[ $BASE == header || $BASE == footer ]]
    then
        continue
    fi
    php "$FILE" print > "$DISTDIR/$BASE.html"
done

cp *.svg *.css *.png "$DISTDIR/"
cp ../../setup/meson-player.png "$DISTDIR/app.png"

echo "DONE!"
