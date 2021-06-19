#!/usr/bin/env bash
set -ex
cd "$(dirname -- "${BASH_SOURCE[0]}")/../../.."


QT_URL="https://github.com/alkatrazstudio/qt5-build/releases/download/v5.14.2-17/mingw-w64-x86_64-qt5alst-5.14.2-1-any.pkg.tar.zst"
ICU_URL="https://github.com/alkatrazstudio/icu-build/releases/download/v69.1-0/icu-msys.tar.xz"
MEGATOOLS_URL="https://megatools.megous.com/builds/experimental/megatools-1.11.0-git-20200503-win64.zip"


#
# Check that all vars are set
#
set +x
[[ -z $EXTERNAL_DATA_URL ]] && (echo 'env not set: $EXTERNAL_DATA_URL'; exit -1)
[[ -z $EXTERNAL_DATA_PASSWORD ]] && (echo 'env not set: $EXTERNAL_DATA_URL'; exit -1)
set -x


#
# Install the required packages
#
pacman --noconfirm -Su
pacman --noconfirm --needed -S \
    mingw-w64-x86_64-qbs \
    unzip \
    p7zip \
    mingw-w64-x86_64-quazip \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-pkgconf \
    > /dev/null


#
# Install Qt
#
mkdir qtpkg
pushd qtpkg
    wget -nv "$QT_URL"
    pacman --noconfirm -U *.pkg.tar.zst
popd
rm -rf qtpkg


#
# Install ICU
#
mkdir -p /opt/icu
pushd /opt/icu
    curl -SsL "$ICU_URL" | unxz | tar -xf-
    cp -a icu/. /mingw64/opt/qt5alst/.
popd
rm -rf /opt/icu


#
# Install megatools
#
mkdir megatools
pushd megatools
    wget -nv "$MEGATOOLS_URL"
    unzip *.zip
    mv */megatools.exe /usr/bin/
popd
rm -rf megatools


#
# Install external data
#
set +x
megatools dl --path=./external.7z --no-progress --no-ask-password "$EXTERNAL_DATA_URL"
7za x -bso0 external.7z -p"$EXTERNAL_DATA_PASSWORD"
set -x

mv external/bass/windows/bass.dll "$MINGW_PREFIX/bin/"
mv external/bass/windows/bassmix.dll "$MINGW_PREFIX/bin/"
cp -r external/bass/windows/plugins ./

if [[ -d external/services ]]
then
    cp -r external/services ./
fi

rm -rf external.7z external


#
# Build
#
setup/build.sh -v
mv /tmp/build/meson-player/installer/*.exe ./
