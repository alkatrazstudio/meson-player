#!/usr/bin/env bash
set -ex
cd "$(dirname -- "${BASH_SOURCE[0]}")/../../.."


QT_URL="https://github.com/alkatrazstudio/qt5-build/releases/download/v5.14.2-17/qt5-v5.14.2-linux-x86_64.tar.xz"
APPIMAGETOOL_URL="https://github.com/AppImage/AppImageKit/releases/download/12/appimagetool-x86_64.AppImage"


#
# Check that all vars are set
#
set +x
[[ -z $EXTERNAL_DATA_URL ]] && (echo 'env not set: $EXTERNAL_DATA_URL'; exit -1)
[[ -z $EXTERNAL_DATA_PASSWORD ]] && (echo 'env not set: $EXTERNAL_DATA_URL'; exit -1)
set -x


#
# Simulate sudo if it's not needed
#
if [[ $UID == 0 && ! $(which sudo) ]]
then
    function sudo {
        "$@"
    }
fi


#
# Install the required packages
#
export DEBIAN_FRONTEND=noninteractive
sudo apt-get update -yq
sudo apt-get install -yq \
    python3 \
    php-cli \
    curl \
    megatools \
    p7zip-full \
    g++ \
    pkg-config \
    libasound2-dev \
    zlib1g-dev \
    libxcb1-dev \
    libxcb-keysyms1-dev \
    libgl-dev


#
# Install Qt
#
sudo mkdir -p /opt/qt
pushd /opt/qt
    curl -SsL "$QT_URL" | sudo tar -xJ
popd


#
# Install external data
#
set +x
megadl --path=./external.7z --no-progress --no-ask-password "$EXTERNAL_DATA_URL"
7za x -bso0 external.7z -p"$EXTERNAL_DATA_PASSWORD"
set -x

sudo cp external/bass/linux/libbass.so /usr/local/lib/
sudo cp external/bass/linux/libbassmix.so /usr/local/lib/
cp -a external/bass/linux/plugins ./

if [[ -d external/services ]]
then
    cp -a external/services ./
fi

rm -rf external.7z external


#
# Install appimagetool
#
curl -SsL -o ./appimagetool "$APPIMAGETOOL_URL"
chmod u+x ./appimagetool
sudo mv ./appimagetool /usr/local/bin/


#
# Build
#
setup/build.sh -v
mv /tmp/build/meson-player/image/*.AppImage ./
