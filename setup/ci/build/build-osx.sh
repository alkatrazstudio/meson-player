#!/usr/bin/env bash
set -ex
cd "$(dirname -- "${BASH_SOURCE[0]}")/../../.."


QT_URL="https://github.com/alkatrazstudio/qt5-build/releases/download/v5.14.2-17/qt5-v5.14.2-osx-x86_64.tar.xz"
ICU_URL="https://github.com/alkatrazstudio/icu-build/releases/download/v69.1-0/icu-osx.tar.xz"


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
brew install p7zip megatools pkg-config qbs


#
# Install Qt
#
sudo mkdir -p /opt/qt
pushd /opt/qt
    curl -SsL "$QT_URL" | sudo tar -xJ
    rm -f /opt/qt/qt5/bin/qbs*
popd


#
# Install ICU
#
sudo mkdir -p /opt/icu
pushd /opt/icu
    curl -SsL "$ICU_URL" | sudo tar -xJ
    sudo cp -a icu/. /opt/qt/qt5/.
popd
sudo rm -rf /opt/icu


#
# Install external data
#
set +x
megadl --path=./external.7z --no-progress --no-ask-password "$EXTERNAL_DATA_URL"
7za x -bso0 external.7z -p"$EXTERNAL_DATA_PASSWORD"
set -x

sudo cp external/bass/osx/libbass.dylib /usr/local/lib/
sudo cp external/bass/osx/libbassmix.dylib /usr/local/lib/
cp -a external/bass/osx/plugins ./

if [[ -d external/services ]]
then
    cp -a external/services ./
fi

rm -rf external.7z external


#
# Build
#
setup/build.sh -v
mv /tmp/build/meson-player/pkg/*.pkg ./
