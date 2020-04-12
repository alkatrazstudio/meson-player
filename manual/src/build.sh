#!/usr/bin/env bash
set -e
cd "$(dirname -- "${BASH_SOURCE[0]}")"

MODE="$1"

if [[ $MODE == site ]]
then
    DISTDIR=../site
else
    DISTDIR=../dist
fi
rm -rf "$DISTDIR"
mkdir "$DISTDIR"

for FILE in *.php
do
    BASE="$(basename -s.php $FILE)"
    if [[ $BASE == header || $BASE == footer ]]
    then
        continue
    fi
    php "$FILE" "$MODE" > "$DISTDIR/$BASE.html"
done

cp *.svg *.css *.png "$DISTDIR/"
cp ../../setup/meson-player.png "$DISTDIR/app.png"

if [[ $MODE == site ]]
then
    cp -r screenshots "$DISTDIR/"
fi

echo "DONE!"
