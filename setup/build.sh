#!/usr/bin/env bash
set -e
if [[ $1 == -v ]]
then
    set -x
fi

SETUP_DIR="$(dirname -- "${BASH_SOURCE[0]}")"
cd "$SETUP_DIR"
SETUP_DIR="$(pwd)"
cd "$SETUP_DIR/.."
THISDIR="$(pwd)"

if [[ -f "$SETUP_DIR/env" ]]
then
    . "$SETUP_DIR/env"
fi

case $OSTYPE in
    darwin*)
        OS=osx
        ;;

    msys)
        OS=windows
        ;;

    *)
        OS=linux
        ;;
esac


#
# SETUP
#

PROJECT_TITLE="Meson Player"
PROJECT_NAME=meson-player
PROJECT_VERSION="$($THISDIR/common/modules/Version/version.py)"
PROJECT_VERSION="${PROJECT_VERSION:0:$((${#PROJECT_VERSION}-2))}"

PROJECT_NAME_NODASH="${PROJECT_NAME//-/}"
PROJECT_BUNDLE_ID="net.alkatrazstudio.$PROJECT_NAME_NODASH"

case $OS in
    linux)
        QT_DIR="/opt/qt/qt5"
        export LD_LIBRARY_PATH="$QT_DIR/lib:$LD_LIBRARY_PATH"
        NPROC="$(nproc)"
        ;;

    windows)
        QT_DIR="/mingw64/opt/qt5alst"
        NPROC="$(nproc)"
        export LD_LIBRARY_PATH="$QT_DIR/lib:$LD_LIBRARY_PATH"
        export PATH="/c/dev/php:/c/Program Files (x86)/Inno Setup 6:$PATH"
        ;;

    osx)
        QT_DIR="/opt/qt/qt5"
        export DYLD_FALLBACK_LIBRARY_PATH="$QT_DIR/lib:/usr/local/lib:$DYLD_FALLBACK_LIBRARY_PATH"
        NPROC="$(sysctl -n hw.activecpu)"
        ;;
esac

export PATH="$QT_DIR/bin:$PATH"

export PKG_CONFIG_PATH="$QT_DIR/lib/pkgconfig:$PKG_CONFIG_PATH"
TMP_DIR="/tmp/build/$PROJECT_NAME"
BUILD_DIR="$TMP_DIR/build"
DIST_DIR="$TMP_DIR/dist"
QT_PLUGINS_DIR="qt.plugins"
QBS_PROFILE="qtopt"


#
# CLEAN
#

if [[ -z $NO_REBUILD ]]
then
    rm -rf "$BUILD_DIR" "$DIST_DIR"
fi


#
# DOCS
#

if [[ -z $NO_REBUILD ]]
then
    echo "Building manual..."
    "$THISDIR/manual/src/build.sh"
fi


#
# PREPARE QBS PROFILES
#

if [[ -z $NO_REBUILD ]]
then
    qbs-setup-toolchains --detect
    case "$OS" in
        linux)
            qbs-setup-qt "$QT_DIR/bin/qmake" "$QBS_PROFILE"
            QBS_BASE_PROFILE="$(qbs config --list profiles | grep -Po "^profiles.\Kx86_64-linux-gnu-gcc-[^.]+" | head -1)"
            ;;

        windows)
            qbs-setup-qt "$QT_DIR/bin/qmake.exe" "$QBS_PROFILE"
            QBS_BASE_PROFILE="$(qbs config --list profiles | grep -Po "^profiles.\K$MINGW_CHOST-gcc-[^.]+" | head -1)"
            ;;

        osx)
            qbs-setup-qt "$QT_DIR/bin/qmake" "$QBS_PROFILE"
            QBS_BASE_PROFILE=xcode-macosx-x86_64
            ;;
    esac

    qbs-config "profiles.$QBS_PROFILE.baseProfile" "$QBS_BASE_PROFILE"
    qbs-config defaultProfile "$QBS_PROFILE"
fi


#
# BUILD
#

if [[ -z $NO_REBUILD ]]
then
    if [[ $OS == windows ]]
    then
        QBS_INSTALL_ROOT="$(cygpath -w "$DIST_DIR")"
    else
        QBS_INSTALL_ROOT="$DIST_DIR"
    fi
    qbs build \
        -d "$BUILD_DIR/$PROJECT_NAME" \
        -f "$THISDIR/$PROJECT_NAME.qbs" \
        --jobs "$NPROC" \
        qbs.defaultBuildVariant:release \
        qbs.installRoot:"$QBS_INSTALL_ROOT" \
        modules.xcodeModule.sdk:macosx10.15 \
        profile:"$QBS_PROFILE"
    if [[ $OS == windows ]]
    then
        qbs resolve \
            -d "$BUILD_DIR/$PROJECT_NAME" \
            qbs.defaultBuildVariant:release \
            qbs.installRoot:"$QBS_INSTALL_ROOT" \
            modules.xcodeModule.sdk:macosx10.15 \
            profile:"$QBS_PROFILE" \
            project.ctrlProfile:true
        qbs build \
            -d "$BUILD_DIR/$PROJECT_NAME" \
            -f "$THISDIR/$PROJECT_NAME.qbs" \
            --jobs "$NPROC"
    fi
fi


#
# QT CONF
#

if [[ $OS != osx ]]
then
    cat > "$DIST_DIR/qt.conf" << EOF
[Paths]
Plugins = $QT_PLUGINS_DIR
EOF
fi


function build_linux
{
    # copy libs
    mkdir -p "$DIST_DIR/lib"
    cp -a \
        "$QT_DIR"/lib/libQt5Core.so* \
        "$QT_DIR"/lib/libQt5DBus.so* \
        "$QT_DIR"/lib/libQt5Gui.so* \
        "$QT_DIR"/lib/libQt5Network.so* \
        "$QT_DIR"/lib/libQt5Widgets.so* \
        "$QT_DIR"/lib/libQt5XcbQpa.so* \
        "$QT_DIR"/lib/libQt5Qml.so* \
        "$QT_DIR"/lib/libmpris-qt5.so* \
        "$QT_DIR"/lib/libdbusextended-qt5.so* \
        "$QT_DIR"/lib/libquazip.so* \
        "$QT_DIR"/lib/libicudata.so* \
        "$QT_DIR"/lib/libicui18n.so* \
        "$QT_DIR"/lib/libicuuc.so* \
            "$DIST_DIR/lib/"

    mkdir -p "$DIST_DIR/$QT_PLUGINS_DIR/platforms"
    cp -a "$QT_DIR"/plugins/platforms/libqxcb.so "$DIST_DIR/$QT_PLUGINS_DIR/platforms/"

    mkdir -p "$DIST_DIR/$QT_PLUGINS_DIR/imageformats"
    cp -a "$QT_DIR"/plugins/imageformats/libqico.so "$DIST_DIR/$QT_PLUGINS_DIR/imageformats/"

    cp -a \
        /usr/local/lib/libbass.so \
        /usr/local/lib/libbassmix.so \
        "$DIST_DIR/lib/"

    # copy fallback libs
    mkdir -p "$DIST_DIR/lib-fallback"
    cp -a \
        /usr/lib/x86_64-linux-gnu/libxcb-keysyms.so* \
            "$DIST_DIR/lib-fallback/"

    # patch RUNPATH
    SYS_RUNPATH=/usr/local/lib/x86_64-linux-gnu:/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu:/usr/lib/x86_64-linux-gnu64:/usr/local/lib64:/lib64:/usr/lib64:/usr/local/lib:/lib:/usr/lib:/usr/x86_64-linux-gnu/lib64:/usr/x86_64-linux-gnu/lib
    patchelf --set-rpath '$ORIGIN/lib:'"$SYS_RUNPATH"':$ORIGIN/lib-fallback' "$DIST_DIR/mesonplayer"
    patchelf --set-rpath '$ORIGIN' "$DIST_DIR"/lib/libicui18n.so.*.*
    patchelf --set-rpath '$ORIGIN' "$DIST_DIR"/lib/libicuuc.so.*.*

    # prepare AppImage links
    ln -sf "mesonplayer" "$DIST_DIR/AppRun"
    cp "$SETUP_DIR/AppImage/net.alkatrazstudio.mesonplayer.desktop" "$DIST_DIR/"
    cp "$SETUP_DIR/meson-player.png" "$DIST_DIR/"
    mkdir -p "$DIST_DIR/usr/share/metainfo/"
    cp "$SETUP_DIR/AppImage/net.alkatrazstudio.mesonplayer.appdata.xml" "$DIST_DIR/usr/share/metainfo/"
    mkdir -p "$DIST_DIR/usr/share/applications/"
    ln -sf "../../../net.alkatrazstudio.mesonplayer.desktop" "$DIST_DIR/usr/share/applications/"

    # build AppImage
    APPIMAGE_DIR="$TMP_DIR/image"
    rm -rf "$APPIMAGE_DIR"
    mkdir "$APPIMAGE_DIR"
    APPIMAGE_FILE="$APPIMAGE_DIR/$PROJECT_NAME_NODASH-v$PROJECT_VERSION-linux-x86_64.AppImage"

    if [[ -z $SIGN_KEY ]]
    then
        appimagetool --appimage-extract-and-run "$DIST_DIR" "$APPIMAGE_FILE"
    else
        appimagetool --appimage-extract-and-run --sign --sign-key "$SIGN_KEY" "$DIST_DIR" "$APPIMAGE_FILE"
    fi
}


function build_windows
{
    # prepare setup contents
    cp "$THISDIR/EnableAutorun.bat" "$DIST_DIR/"
    cp "$THISDIR/DisableAutorun.bat" "$DIST_DIR/"

    cp "$THISDIR/README.md" "$DIST_DIR/README.txt"
    cp "$THISDIR/CHANGELOG.md" "$DIST_DIR/CHANGELOG.txt"
    cp "$THISDIR/LICENSE.txt" "$DIST_DIR/"

    cp "$QT_DIR/bin/Qt5Core.dll" "$DIST_DIR/"
    cp "$QT_DIR/bin/Qt5Gui.dll" "$DIST_DIR/"
    cp "$QT_DIR/bin/Qt5Network.dll" "$DIST_DIR/"
    cp "$QT_DIR/bin/Qt5Widgets.dll" "$DIST_DIR/"
    cp "$QT_DIR/bin/Qt5Xml.dll" "$DIST_DIR/"

    cp "$QT_DIR"/bin/icudt*.dll "$DIST_DIR/"
    cp "$QT_DIR"/bin/icuin*.dll "$DIST_DIR/"
    cp "$QT_DIR"/bin/icuuc*.dll "$DIST_DIR/"

    mkdir -p "$DIST_DIR/imageformats"
    cp "$QT_DIR/share/qt5/plugins/imageformats/qico.dll" "$DIST_DIR/imageformats/"
    mkdir -p "$DIST_DIR/platforms"
    cp "$QT_DIR/share/qt5/plugins/platforms/qwindows.dll" "$DIST_DIR/platforms/"

    cp "$MINGW_PREFIX/bin/libquazip5.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libstdc++-6.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libgcc_s_seh-1.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/bassmix.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/bass.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libwinpthread-1.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libdouble-conversion.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/zlib1.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libpcre2-16-0.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libzstd.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libharfbuzz-0.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libpng16-16.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libfreetype-6.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libssl-1_1-x64.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libcrypto-1_1-x64.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libbrotlidec.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libbrotlicommon.dll" "$DIST_DIR/"

    cp "$MINGW_PREFIX/bin/libglib-2.0-0.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libgraphite2.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libbz2-1.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libpcre-1.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libintl-8.dll" "$DIST_DIR/"
    cp "$MINGW_PREFIX/bin/libiconv-2.dll" "$DIST_DIR/"

    # make the installer
    INSTALLER_DIR="$TMP_DIR/installer"
    rm -rf "$INSTALLER_DIR"
    ISCC \
        //DVersion="$PROJECT_VERSION" \
        //DOutDir="$(cygpath -w "$INSTALLER_DIR")" \
        //DBinDir="$(cygpath -w "$DIST_DIR")" \
        "$SETUP_DIR/InnoSetup/installer.iss"
}


function build_osx
{
    PKG_DIR="$TMP_DIR/pkg"
    rm -rf "$PKG_DIR"

    # copy app
    CONTENTS_DIR="$PKG_DIR/components/app/$PROJECT_TITLE.app/Contents"
    mkdir -p "$CONTENTS_DIR/MacOS"
    cp "$DIST_DIR/Info.plist" "$DIST_DIR/PkgInfo" "$CONTENTS_DIR/"
    mkdir -p "$CONTENTS_DIR/Resources"
    cp "$DIST_DIR"/*.icns "$CONTENTS_DIR/Resources/"
    find "$DIST_DIR" \
        ! -name 'Info.plist' \
        ! -name 'PkgInfo' \
        ! -name '*.icns' \
        -depth 1 \
        -exec cp -r {} "$CONTENTS_DIR/MacOS" \;

    # copy context menu handler
    mkdir "$PKG_DIR/components/ctx"
    cp -r "$SETUP_DIR/pkg/Open in $PROJECT_TITLE.workflow" "${PKG_DIR}/components/ctx/"

    # bundle libs
    pushd "$CONTENTS_DIR"
        macdeployqt .. -libpath="$QT_DIR/lib" -libpath=/usr/local/lib
        find PlugIns -depth 2 ! \( \
            -name "libqico.*" -o \
            -name "libqcocoa.*" \
        \) -exec rm -rf {} +
        cp /usr/local/lib/libbass.dylib Frameworks/
        cp /usr/local/lib/libbassmix.dylib Frameworks/
        find . -type d -empty -delete

        pushd MacOS
            OLD_BASS_PATH="@loader_path/libbass.dylib"
            OLD_BASSMIX_PATH="@loader_path/libbassmix.dylib"
            NEW_BASS_PATH="@executable_path/../Frameworks/libbass.dylib"
            NEW_BASSMIX_PATH="@executable_path/../Frameworks/libbassmix.dylib"

            install_name_tool -change "$OLD_BASS_PATH" "$NEW_BASS_PATH" ../Frameworks/libbassmix.dylib
            install_name_tool -change "$OLD_BASS_PATH" "$NEW_BASS_PATH" "./$PROJECT_NAME_NODASH"
            install_name_tool -change "$OLD_BASSMIX_PATH" "$NEW_BASSMIX_PATH" "./$PROJECT_NAME_NODASH"
            install_name_tool -add_rpath "@executable_path/../Frameworks" "./$PROJECT_NAME_NODASH"
            pushd plugins
                find . -name "*.dylib" -exec install_name_tool -change "$OLD_BASS_PATH" "$NEW_BASS_PATH" {} \;
            pushd
        popd
    popd

    # setup pre/post-install scripts
    APP_SCRIPTS_ROOT="$PKG_DIR/scripts/app"
    CTX_SCRIPTS_ROOT="$PKG_DIR/scripts/ctx"
    mkdir -p "${APP_SCRIPTS_ROOT}"
    cat > "${APP_SCRIPTS_ROOT}/preinstall" <<EOF
#!/bin/bash
killall $PROJECT_NAME
sudo rm -rf "/Applications/$PROJECT_TITLE.app"
exit 0
EOF
    chmod a+x "${APP_SCRIPTS_ROOT}/preinstall"

    mkdir -p "${CTX_SCRIPTS_ROOT}"
    cat > "${CTX_SCRIPTS_ROOT}/postinstall" <<EOF
#!/bin/bash
sudo rm -rf "/System/Library/Services/Open in $PROJECT_TITLE.workflow"
exit 0
EOF
    chmod a+x "${CTX_SCRIPTS_ROOT}/postinstall"

    # prepare extra resources
    pushd "$SETUP_DIR/pkg"
        sed -e 's/%VERSION%/'"$PROJECT_VERSION"'/' Distribution.xml > "$PKG_DIR/distribution.dist"
    popd
    mkdir -p "$PKG_DIR/resources"
    cp -r "$SETUP_DIR/pkg/html" "$PKG_DIR/resources/"

    # normalize attributes and access rights
    xattr -rc "${PKG_DIR}"
    chmod -R u=rwX,go=rX .

    # make package
    PKG_FILENAME="$PKG_DIR/${PROJECT_NAME_NODASH}-v${PROJECT_VERSION}-osx-setup.pkg"
    SUBPKG_DIR="$PKG_DIR/packages"
    rm -rf "$PKG_FILENAME" "$SUBPKG_DIR"
    mkdir -p "$SUBPKG_DIR"

    pkgbuild \
        --identifier "$PROJECT_BUNDLE_ID.app" \
        --install-location "/Applications/${PROJECT_TITLE}.app" \
        --root "${PKG_DIR}/components/app/${PROJECT_TITLE}.app" \
        --version ${PROJECT_VERSION} \
        --scripts "${APP_SCRIPTS_ROOT}" \
        "$SUBPKG_DIR/app.pkg"

    pkgbuild \
        --identifier "$PROJECT_BUNDLE_ID.ctx" \
        --install-location "/Library/Services/" \
        --root "${PKG_DIR}/components/ctx" \
        --version ${PROJECT_VERSION} \
        --scripts "${CTX_SCRIPTS_ROOT}" \
        "$SUBPKG_DIR/ctx.pkg"

    productbuild \
        --identifier "$PROJECT_BUNDLE_ID" \
        --distribution "$PKG_DIR/distribution.dist" \
        --package-path "$SUBPKG_DIR" \
        --version "$PROJECT_VERSION" \
        --resources "$PKG_DIR/resources" \
        "$PKG_FILENAME"
}


case $OS in
    linux)
        build_linux
        ;;

    windows)
        build_windows
        ;;

    osx)
        build_osx
        ;;
esac

echo "DONE!"
