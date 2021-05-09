/****************************************************************************}
{ app.cpp - application core                                                 }
{                                                                            }
{ Copyright (c) 2012 Alexey Parfenov <zxed@alkatrazstudio.net>               }
{                                                                            }
{ This file is part of Meson Player.                                         }
{                                                                            }
{ Meson Player is free software: you can redistribute it and/or modify it    }
{ under the terms of the GNU General Public License as published by          }
{ the Free Software Foundation, either version 3 of the License,             }
{ or (at your option) any later version.                                     }
{                                                                            }
{ Meson Player is distributed in the hope that it will be useful,            }
{ but WITHOUT ANY WARRANTY; without even the implied warranty of             }
{ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU           }
{ General Public License for more details: https://gnu.org/licenses/gpl.html }
{****************************************************************************/


#include "app.h"
#include "lastfmpassform.h"
#include "movetotrash.h"
#include "qiodevicehelper.h"

#include <thread>
#include <chrono>

static const int APPKEY_PAUSE = 0;
static const int APPKEY_PREV_DIR = 7;
static const int APPKEY_NEXT_DIR = 9;
static const int APPKEY_VOL_UP = 8;
static const int APPKEY_VOL_DOWN = 2;
static const int APPKEY_PREV_TRACK = 4;
static const int APPKEY_NEXT_TRACK = 6;
static const int APPKEY_PLAY = 5;
static const int APPKEY_MVOL_DOWN = 1;
static const int APPKEY_MVOL_UP = 3;
static const int APPKEYBASE_MM = 10;
static const int APPKEYBASE_LOAD_PLAYLIST = 20;
static const int APPKEYBASE_SAVE_PLAYLIST = 30;

static const int trayTimerInterval = 1000; // 1 sec
static const int trayTimerMaxTries = 100;
static const int saveTimerInterval = 3000; // 3 secs

#ifdef Q_OS_OSX
static const int KEY_REPEAT_TICK_INTERVAL = 50;
static const int KEY_REPEAT_WAIT_TICKS = 20;
#endif

#define VOLUME_OWNER mixer

#ifdef Q_OS_OSX
OSStatus hotKeyHandler(EventHandlerCallRef eventHandlerCallRef, EventRef eventRef, void *data)
{
    App* app = (App*)data;
    if(!app->isQuitting())
    {
        EventHotKeyID keyEvent;
        GetEventParameter(eventRef, kEventParamDirectObject, typeEventHotKeyID, nullptr, sizeof(keyEvent), nullptr, &keyEvent);

        UInt32 kind = GetEventKind(eventRef);
        switch(kind)
        {
            case kEventHotKeyPressed:
                app->onHotKey(keyEvent.id, true);
                app->startKeyRepeatTimer(keyEvent.id);
                break;

            case kEventHotKeyReleased:
                app->stopKeyRepeatTimer();
                app->onHotKey(keyEvent.id, false);
                break;
        }
    }
    return CallNextEventHandler(eventHandlerCallRef, eventRef);
}
#endif

void App::onHotKey(int id, bool isPressed)
{
    enum class KeyState {
        notPressed,
        pressedOnce,
        held
    };

    static int lastKeyId = -1;
    static KeyState lastKeyState = KeyState::notPressed;
    if(lastKeyId != id)
    {
        lastKeyState = KeyState::notPressed;
        lastKeyId = id;
    }

    //qDebug() << "KEY" << id << isPressed << lastKeyState;

    bool keyHold = false;
    if(isPressed)
    {
        switch(lastKeyState)
        {
            case KeyState::notPressed:
                lastKeyState = KeyState::pressedOnce;
                break;

            case KeyState::pressedOnce:
                lastKeyState = KeyState::held;
                keyHold = true;
                break;

            case KeyState::held:
                keyHold = true;
                break;
        }
    }
    else
    {
        switch(lastKeyState)
        {
            case KeyState::notPressed:
                // should not happen
                return;

            case KeyState::pressedOnce:
                lastKeyState = KeyState::notPressed;
                break;

            case KeyState::held:
                lastKeyState = KeyState::notPressed;
                return; // only capture key press on hold, not key release
        }
    }

    int pKey;
    pKey = id - APPKEYBASE_LOAD_PLAYLIST;
    if((pKey >= 0) && (pKey <= 9) && !keyHold && !isPressed)
    {
        doSaveList = false;
        if(settings.unlimitedPlaylists)
        {
            QString s;
            s.setNum(pKey);
            playlistPrefix.append(s);
            startPrefixTimer();
            if(settings.systemPlaylists && !appCommandInput)
            {
                if(playlistPrefix == "00")
                {
                    if(!appCommandHotkeysOn)
                        registerAppCommandHotKeys();
                    appCommandInput = true;
                }
            }
        }
        else
        {
            playlistPrefix.setNum(pKey);
            loadPlaylist();
        }
        return;
    }

    pKey = id - APPKEYBASE_SAVE_PLAYLIST;
    if((pKey >= 0) && (pKey <= 9) && !keyHold && !isPressed)
    {
        doSaveList = true;
        if(settings.unlimitedPlaylists)
        {
            QString s;
            s.setNum(pKey);
            playlistPrefix.append(s);
            startPrefixTimer();
        }
        else
        {
            playlistPrefix.setNum(pKey);
            savePlaylist();
        }
        return;
    }

    int mmKey = id - APPKEYBASE_MM;
    if((mmKey >= 0) && (mmKey <= 9))
        id = mmKey;

    switch(id)
    {
        case APPKEY_VOL_DOWN:
            if(isPressed)
                cmdVolDown();
            break;

        case APPKEY_VOL_UP:
            if(isPressed)
                cmdVolUp();
            break;

        case APPKEY_PREV_TRACK:
            if(!isPressed)
            {
                cmdPrevTrack(true);
            }
            else
            {
                if(keyHold)
                    cmdSeekBack();
            }
            break;

        case APPKEY_NEXT_TRACK:
            if(!isPressed)
            {
                cmdNextTrack(true);
            }
            else
            {
                if(keyHold)
                    cmdSeekForward();
            }
            break;

        case APPKEY_PREV_DIR:
            if(isPressed)
                cmdPrevDir();
            break;

        case APPKEY_NEXT_DIR:
            if(isPressed)
                cmdNextDir();
            break;

        case APPKEY_PLAY:
            if(isPressed)
                cmdTogglePlayStop();
            break;

        case APPKEY_PAUSE:
            if(isPressed)
                cmdTogglePlayPause();
            break;

        case APPKEY_MVOL_DOWN:
            if(isPressed)
                cmdMVolDown();
            break;

        case APPKEY_MVOL_UP:
            if(isPressed)
                cmdMVolUp();
            break;

        default:
            break;
    }
}

#ifdef Q_OS_OSX
void App::startKeyRepeatTimer(int keyId)
{
    stopKeyRepeatTimer();
    keyRepeatCounter = 0;
    keyRepeatKeyId = keyId;
    keyRepeatTimer = startTimer(KEY_REPEAT_TICK_INTERVAL);
}

void App::stopKeyRepeatTimer()
{
    if(keyRepeatTimer)
    {
        killTimer(keyRepeatTimer);
        keyRepeatTimer = 0;
    }
}
#endif

int App::main()
{
    qDebug()
            << applicationDisplayName()
            << version().toString()
            << "(c)" << organizationName()
            << "[" << organizationDomain() << "]";

    exitAfterMain = false;
    QDir d(settingsDir);
    if(!d.mkpath(settingsDir))
        SETERROR(Err::createDir, settingsDir);

#ifdef Q_OS_LINUX
    int scrNum;
    xconn = xcb_connect(nullptr, &scrNum);
    eventQueue = new EventQueue(xconn);
    xcb_screen_iterator_t scrIter = xcb_setup_roots_iterator(xcb_get_setup(xconn));
    xscreen = nullptr;
    xwin = 0;
    for(; scrIter.rem; --scrNum, xcb_screen_next(&scrIter))
    {
        if(scrNum == 0)
        {
            xscreen = scrIter.data;
            xwin = xscreen->root;
            break;
        }
    }
    xkeysyms = xcb_key_symbols_alloc(xconn);
#endif

#ifdef Q_OS_WIN
    CreateMutexA(nullptr, false, applicationName().toUtf8().constData()); // for Inno Installer
#endif
    if(!loadSettings())
        return -1;
    if(exitNow)
    {
        quit();
        return 0;
    }
    if(!startLocalServer())
        return -1;
    loadTranslations();
    int c = 10; // try 10 times before failing
    QString s;

    while(!createSoundObject())
    {
        c--;
        if(!c)
            return -1;
        s.setNum(c);
        if(lfm)
        {
            delete lfm;
            lfm = nullptr;
        }
        if(mixer)
        {
            delete mixer;
            mixer = nullptr;
        }
        if(sound)
        {
            delete sound;
            sound = nullptr;
        }
        if(player)
        {
            auto err = ErrorManager::lastError();
            delete player;
            player = nullptr;
            if(err.codeIs(MSE_Object::Err::invalidVersion))
                return -1;
        }
        qDebug() << "tries left:" << c;
        // sleep for one second then try again
        // because it may be a temporary failure
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    if(settings.trayIcon)
        tryCreateTray();
    loadState();
    initSound();
    updateTray(false);
    addHotKeysInfo();
    if(hotkeysOn)
        if(!registerControlHotKeys())
        {
            //return -1;
        }
    if(!registerHotKeys(hotKeysAppMain))
    {
        //return -1;
    }
    updateVolumeLabel();
    if(tray)
        tray->show();
    if(settings.autoNumlock)
        setNumLock();

#ifdef Q_OS_OSX
    EventTypeSpec eventTypeSpec[2];
    eventTypeSpec[0].eventClass = kEventClassKeyboard;
    eventTypeSpec[0].eventKind = kEventHotKeyPressed;
    eventTypeSpec[1].eventClass = kEventClassKeyboard;
    eventTypeSpec[1].eventKind = kEventHotKeyReleased;
    InstallApplicationEventHandler(&hotKeyHandler, 2, eventTypeSpec, this, &keyHooks);
#endif
#ifdef Q_OS_WIN
    messageWin = CreateWindowExA(
        0,
        "Message",
        "MesonPlayerMessageWindow",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        HWND_MESSAGE,
        NULL,
        GetModuleHandleA(NULL),
        NULL
    );

    RAWINPUTDEVICE rid;
    memset(&rid, 0, sizeof(rid));
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = messageWin;
    if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE) {
        return -1;
    }
#endif
#ifdef Q_OS_LINUX
    keyHookTimer = startTimer(10);
#endif

#ifdef MSE_MODULE_MPRIS
    if(settings.mpris)
        mprisInit();
#endif

    qDebug() << "initialization done";
    return 0;
}

void App::onQuit()
{
    if(exitNow)
        return;

    killSaveTimer();
    saveState();
    saveStateList();

    killPrefixTimer();
    killTrayTimer();
#ifdef Q_OS_LINUX
    if(keyHookTimer)
        killTimer(keyHookTimer);
#endif
#ifdef Q_OS_OSX
    stopKeyRepeatTimer();
    RemoveEventHandler(keyHooks);
#endif
#ifdef Q_OS_WIN
    if(messageWin != NULL)
        DestroyWindow(messageWin);
#endif
    if(hotkeysOn)
        unregisterControlHotKeys();

    unregisterHotKeys(hotKeysAppMain);

    if(tray)
    {
        delete actionShuffle;
        delete actionPlaylists;
        delete actionHotkeys;
        delete actionCurrentFile;
        delete actionExit;
        delete tray;
        delete icoApp;
        delete icoPause;
        delete icoPlay;
        delete icoStop;
    }
    delete mixer;
    if(sound)
    {
        sound->disconnect(this);
        delete sound;
    }
    if(player)
    {
        player->disconnect(this);
        delete player;
    }
}

App::App(int& argc, char** argv) : CoreApp(argc, argv)
#ifdef MSE_MODULE_MPRIS
  ,mpris(nullptr)
#endif
  ,curTrayIco(nullptr)
  ,exitNow(false)
  ,settingsDir(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()+"/."+qCoreApp->applicationName()+"/")
  ,appDir(applicationDirPath()+"/")
  ,globalSettingsFilename(appDir+"settings.ini")
  ,userSettingsFilename(settingsDir+"settings.ini")
  ,tray(nullptr)
  ,trayPopup(nullptr)
  ,player(nullptr)
  ,sound(nullptr)
  ,mixer(nullptr)
  ,playlist(nullptr)
  ,lfm(nullptr)
  ,showLfmLoginNotification(false)
  ,doPlay(false)
  ,hasInputFiles(false)
  ,hotkeysOn(false)
  ,appCommandHotkeysOn(false)
  ,appCommandInput(false)
  ,mvolStepReal(0)
  ,volStepReal(0)
  ,ignoreContState(false)
  ,icoApp(nullptr)
  ,icoPlay(nullptr)
  ,icoPause(nullptr)
  ,icoStop(nullptr)
  ,actionShuffle(nullptr)
  ,actionPlaylists(nullptr)
  ,actionHotkeys(nullptr)
  ,actionCurrentFile(nullptr)
  ,actionCloseFile(nullptr)
  ,actionExit(nullptr)
  ,modeActions(nullptr)
  ,doSaveList(false)
  ,prefixTimer(0)
  ,trayTimer(0)
  ,trayTimerTriesLeft(trayTimerMaxTries)
  ,saveTimer(0)
  ,hotkeysDisabled(true)
#ifdef Q_OS_LINUX
  ,xconn(nullptr)
  ,xscreen(nullptr)
  ,xkeysyms(nullptr)
  ,keyHookTimer(0)
  ,eventQueue(nullptr)
#endif
#ifdef Q_OS_OSX
  ,keyRepeatTimer(0)
  ,keyRepeatCounter(0)
  ,keyRepeatKeyId(-1)
#endif
#ifdef Q_OS_WIN
  ,messageWin(NULL)
#endif
  ,aboutDialog(nullptr)
  ,aboutText(nullptr)
{
    singleInstance = true;
    //createDummyWindow = true;
}

App::~App()
{
#ifdef Q_OS_LINUX
    delete eventQueue;
    xcb_key_symbols_free(xkeysyms);
#endif
    delete modeActions;
}

#ifdef MSE_MODULE_MPRIS
void App::mprisInit()
{
    class AppMpris : public MSE_Mpris
    {
        public:
            AppMpris(MSE_Sound* sound, MSE_Mixer* mixer) : MSE_Mpris(sound)
                ,mixer(mixer)
                {}
            virtual ~AppMpris(){}

        protected:
            MSE_Mixer* mixer;

            virtual float getVolume()
            {
                return VOLUME_OWNER->getVolume();
            }
    };

    mpris = new AppMpris(sound, mixer);

    connect(mpris, &AppMpris::playRequested, [this]{
        cmdPlay();
    });

    connect(mpris, &AppMpris::pauseRequested, [this]{
        cmdPause();
    });

    connect(mpris, &AppMpris::playPauseRequested, [this]{
        cmdTogglePlayPause();
    });

    connect(mpris, &AppMpris::stopRequested, [this]{
        cmdStop();
    });

    connect(mpris, &AppMpris::nextRequested, [this]{
        cmdNextTrack(false);
    });

    connect(mpris, &AppMpris::previousRequested, [this]{
        cmdPrevTrack(false);
    });

    connect(mpris, &AppMpris::seekRequested, [this](double secs){
        setSoundPosition(secs);
    });

    connect(mpris, &AppMpris::playbackModeRequested, [this](MSE_PlaylistPlaybackMode playbackMode){
        updateModeGroup(playbackMode);
    });

    connect(mpris, &AppMpris::volumeRequested, [this](double volume){
        setSoundVolume(volume, false);
    });

    connect(mpris, &AppMpris::openUriRequested, [this](const QUrl &url){
        cmdStop();
        playlist->clear();
        clearLastTrackData();
        playlist->addAnything(url.toString());
        saveStateList();
        sound->playNextValid();
        mixer->play();
        startSaveTimer();
        showNotificationOnUserTrackChange();
    });

    connect(mpris, &AppMpris::quitRequested, []{
        quit();
    });

    mpris->updateVolume();
}
#endif

void App::onInfoChange()
{
    if(sound->getType() == mse_sctRemote)
    {
        curDir = sound->getTrackFilename();
    }
    else
    {
        QFileInfo info(sound->getTrackFilename());
        QFileInfo dirinfo(info.absolutePath());
        curDir = dirinfo.fileName();
        if(curDir.size() < settings.minFolderLength)
        {
            QFileInfo dirinfo2(dirinfo.absolutePath());
            QString subName = dirinfo2.fileName();
            if(!subName.isEmpty())
                if(subName != "/")
                    curDir.prepend(subName+"/");
        }
    }

    updateLastTrackData();
    updateTray(settings.notificationsTrackChanged == ChangeTrackNotifications::always);
}

void App::onActionPlaylists()
{
    QString dirname;
    QDir dir;
    QString path;
    dirname = settingsDir+"playlists";
    dir.setPath(dirname);
    if(!dir.exists())
    {
        CHECKV(dir.mkdir(dirname), Err::createDir, dirname);
    }
    showDir(dirname);
}

void App::onActionHotkeys(bool checked)
{
    if(checked)
        registerControlHotKeys();
    else
        unregisterControlHotKeys();
    updateTray(false);
}

bool App::createEmptyUserSettingsFileIfNeeded()
{
    QFile f(userSettingsFilename);
    if(f.exists())
        return true;

    if(!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        showTrayMessage(tr("Cannot create %1").arg(userSettingsFilename));
        return false;
    }

    if(!f.write("[config]\n"))
        showTrayMessage(tr("Cannot write to %1").arg(userSettingsFilename));

    return true;
}

void App::viewFile(const QString& filename)
{
    if(!QFile::exists(filename))
    {
        showTrayMessage(tr("File not found: %1").arg(filename));
        return;
    }

    QString path = QDir::toNativeSeparators(filename);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void App::showDir(const QString& dirname)
{
    QDir dir(dirname);
    if(!dir.exists())
    {
        showTrayMessage(tr("Directory not found: %1").arg(dirname));
        return;
    }

    QString path = QDir::toNativeSeparators(dirname);
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void App::showFile(const QString& filename)
{
    if(!QFile::exists(filename))
    {
        showTrayMessage(tr("File not found: %1").arg(filename));
        return;
    }

#ifdef Q_OS_LINUX
    // based on qBittorrent function
    // https://github.com/qbittorrent/qBittorrent/blob/5b35981c8575dac0b67e35e77b2bcc66d7d8fb4e/src/base/utils/misc.cpp#L581
    QProcess proc;
    proc.start("xdg-mime", QStringList() << "query" << "default" << "inode/directory");
    proc.waitForFinished();
    QString output = QString::fromUtf8(proc.readLine()).simplified().toLower();
    if((output == "dolphin.desktop") || (output == "org.kde.dolphin.desktop"))
        QProcess::startDetached("dolphin", QStringList() << "--select" << filename);
    else if((output == "nautilus.desktop") || (output == "org.gnome.nautilus.desktop")
             || (output == "nautilus-folder-handler.desktop"))
        QProcess::startDetached("nautilus", QStringList() << "--no-desktop" << filename);
    else if(output == "nemo.desktop")
        QProcess::startDetached("nemo", QStringList() << "--no-desktop" << filename);
    else if((output == "konqueror.desktop") || (output == "kfmclient_dir.desktop"))
        QProcess::startDetached("konqueror", QStringList() << "--select" << filename);
    else
        QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(filename).dir().absolutePath()));
#endif

#ifdef Q_OS_OSX
    QStringList args;
    args << "--reveal" << filename;
    QProcess::startDetached("open", args);
#endif

#ifdef Q_OS_WIN
    QStringList args;
    args << "/select," << filename;
    QProcess::startDetached("explorer", args);
#endif
}

void App::onActionCurrentFile()
{
    if(playlist->getIndex() < 0)
        return;
    if(lastTrackData.filename.isEmpty())
        return;
    MSE_SoundChannelType channelType = player->typeByFilename(lastTrackData.filename);
    if((channelType == mse_sctUnknown) || (channelType == mse_sctRemote))
        return;
    if(!QFile::exists(lastTrackData.filename))
        return;

    QString filename = QDir::toNativeSeparators(lastTrackData.filename);
    showFile(filename);
}

void App::onActionCloseFile()
{
    cmdStop();
    sound->close();
}

void App::onActionExit()
{
    cmdStop();
    quit();
}

void App::onActionMode(bool checked)
{
    Q_UNUSED(checked);

    QAction* action = qobject_cast<QAction*>(sender());

    QString modeStr = action->property("mode").toString();
    MSE_PlaylistPlaybackMode mode = playlist->playbackModeFromString(modeStr);
    playlist->setPlaybackMode(mode);
}

void App::onSaveData()
{
    saveStateList();
    saveState();
}

void App::onNativeEvent(void *event, bool &stopPropagation)
{
    if(!isQuitting())
    {
#ifdef Q_OS_WIN
        static int lastVKey = -1;
        static int lastKeyId = -1;

        MSG* msg = (MSG*)event;
        UINT sz;
        RAWINPUT raw;

        switch(msg->message)
        {
            case WM_INPUT:
                if(lastVKey == -1)
                    break;
                sz = sizeof(RAWINPUT);
                GetRawInputData((HRAWINPUT)msg->lParam, RID_INPUT, (PUINT)&raw, &sz, sizeof(RAWINPUTHEADER));
                if(raw.header.dwType == RIM_TYPEKEYBOARD)
                {
                    RAWKEYBOARD& kbd = raw.data.keyboard;
                    if(kbd.VKey == lastVKey && (kbd.Flags & RI_KEY_BREAK))
                    {
                        lastVKey = -1;
                        stopPropagation = true;
                        onHotKey(lastKeyId, false);
                    }
                }
                break;

            case WM_HOTKEY:
                stopPropagation = true;
                lastVKey = HIWORD(msg->lParam);
                lastKeyId = msg->wParam;
                stopPropagation = true;
                onHotKey(lastKeyId, true);
                break;
        }
#endif
    }
    CoreApp::onNativeEvent(event, stopPropagation);
}

bool App::loadSettings()
{
    loadConfig(globalSettingsFilename);
    loadConfig(userSettingsFilename);
    parseCommandLine();

#ifdef DEBUG_MODE
    settings.logErrors = true;
#endif
    logToFile = settings.logErrors;

    hasInputFiles = !cmdPlaylist.isEmpty();

    playlistsExt = MSE_Playlist::extByType(settings.playlistsType);

#ifdef Q_OS_OSX
    settings.modsMM = Qt::GroupSwitchModifier;
    settings.modsMM2 = Qt::GroupSwitchModifier;
#endif

#ifdef Q_OS_WIN
    if(settings.modsCmd.testFlag(Qt::ShiftModifier))
    {
        settings.modsCmd &~ Qt::ShiftModifier;
        SETERROR(Err::shiftModNotSupported);
    }
    if(settings.modsLoad.testFlag(Qt::ShiftModifier))
    {
        settings.modsLoad &~ Qt::ShiftModifier;
        SETERROR(Err::shiftModNotSupported);
    }
    if(settings.modsSave.testFlag(Qt::ShiftModifier))
    {
        settings.modsSave &~ Qt::ShiftModifier;
        SETERROR(Err::shiftModNotSupported);
    }
    if(settings.modsMM.testFlag(Qt::ShiftModifier))
    {
        settings.modsMM &~ Qt::ShiftModifier;
        SETERROR(Err::shiftModNotSupported);
    }
    if(settings.modsMM2.testFlag(Qt::ShiftModifier))
    {
        settings.modsMM2 &~ Qt::ShiftModifier;
        SETERROR(Err::shiftModNotSupported);
    }
#endif

    if((settings.modsCmd == settings.modsLoad) && (settings.modsCmd != Qt::GroupSwitchModifier))
    {
        settings.modsLoad = Qt::GroupSwitchModifier;
        SETERROR(Err::keyModifiersOverlap);
    }
    if((settings.modsCmd == settings.modsSave) && (settings.modsCmd != Qt::GroupSwitchModifier))
    {
        settings.modsSave = Qt::GroupSwitchModifier;
        SETERROR(Err::keyModifiersOverlap);
    }
    if((settings.modsLoad == settings.modsSave) && (settings.modsLoad != Qt::GroupSwitchModifier))
    {
        settings.modsSave = Qt::GroupSwitchModifier;
        SETERROR(Err::keyModifiersOverlap);
    }
    if((settings.modsMM == settings.modsMM2) && (settings.modsMM != Qt::GroupSwitchModifier))
    {
        settings.modsMM2 = Qt::GroupSwitchModifier;
        SETERROR(Err::keyModifiersOverlap);
    }

    mvolStepReal = settings.mvolStep / 100.0;
    volStepReal = settings.volumeStep / 100.0;

    if(settings.proxy == "0")
        settings.proxy.clear();

    if(!settings.proxy.isEmpty())
    {
        if(settings.proxy == "system")
        {
#ifndef NO_SYSTEM_PROXY
            QNetworkProxyFactory::setUseSystemConfiguration(true);
#else
            SETERROR(app_err_SystemProxyDisabled);
#endif
        }
        else
        {
            QUrl proxyUrl(settings.proxy);
            if(proxyUrl.isValid())
            {
                QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;
                if(proxyUrl.scheme() == "socks5")
                    proxyType = QNetworkProxy::Socks5Proxy;
                else
                    if(proxyUrl.scheme() == "http")
                        proxyType = QNetworkProxy::HttpProxy;
                if(proxyType != QNetworkProxy::NoProxy)
                {
                    int proxyPort = proxyUrl.port(0);
                    if(proxyPort)
                    {
                        QNetworkProxy proxyObj;
                        proxyObj.setType(proxyType);
                        proxyObj.setHostName(proxyUrl.host(QUrl::FullyDecoded));
                        proxyObj.setPort(proxyPort);
                        QString proxyUsername = proxyUrl.userName(QUrl::FullyDecoded);
                        if(!proxyUsername.isEmpty())
                        {
                            proxyObj.setUser("username");
                            QString proxyPassword = proxyUrl.password(QUrl::FullyDecoded);
                            if(!proxyPassword.isEmpty())
                                proxyObj.setPassword("password");
                        }
                        QNetworkProxy::setApplicationProxy(proxyObj);
                    }
                    else
                    {
                        SETERROR(Err::invalidProxyPort, settings.proxy);
                    }
                }
                else
                {
                    SETERROR(Err::invalidProxyScheme, settings.proxy);
                }
            }
            else
            {
                SETERROR(Err::invalidProxyUrl, settings.proxy);
            }
        }
    }

    return true;
}

void App::showBalloonsDeprecationWarning()
{
    qWarning() << "\"balloons\" setting is deprecated, use \"notifications\" setting instead.";
}

ChangeTrackNotifications App::stringToTrackChangedNotificationsEnum(const QString &s)
{
    if(s == "interaction")
        return ChangeTrackNotifications::interaction;
    if(s == "never")
        return ChangeTrackNotifications::never;
    return ChangeTrackNotifications::always;
}

void App::loadConfig(const QString &filename)
{
    if(!QFile::exists(filename))
        return;
    QSettings* ini = new QSettings(filename, QSettings::IniFormat);
    ini->beginGroup("config");
    bool ok;

    if(ini->contains("auto-numlock"))
        settings.autoNumlock = ini->value("auto-numlock", true).toBool();

    if(ini->contains("auto-resume"))
        settings.autoResume = ini->value("auto-resume", false).toBool();

    if(ini->contains("balloons"))
    {
        settings.balloons = ini->value("balloons", true).toBool();
        settings.notifications = settings.balloons;
        showBalloonsDeprecationWarning();
    }

    if(ini->contains("notifications"))
    {
        settings.notifications = ini->value("notifications", true).toBool();
        settings.balloons = settings.notifications;
    }

    if(ini->contains("notifications-track-changed"))
    {
        settings.notificationsTrackChanged = stringToTrackChangedNotificationsEnum(
            ini->value("notifications-track-changed", true).toString()
        );
    }

    if(ini->contains("notifications-track-volume"))
        settings.notificationsTrackVolume = ini->value("notifications-track-volume", true).toBool();

    if(ini->contains("notifications-system-volume"))
        settings.notificationsSystemVolume = ini->value("notifications-system-volume", true).toBool();

    if(ini->contains("notifications-seek"))
        settings.notificationsSeek = ini->value("notifications-seek", true).toBool();

    if(ini->contains("buffer-length"))
    {
        uint _val = ini->value("buffer-length", 500).toUInt(&ok);
        if(ok)
            settings.bufferLength = _val;
    }

    if(ini->contains("channels"))
    {
        uint _val = settings.nChannels = ini->value("channels", 2).toUInt(&ok);
        if(ok)
            settings.nChannels = _val;
    }

    if(ini->contains("close-on-stop"))
        settings.closeOnStop = ini->value("close-on-stop", true).toBool();

    if(ini->contains("device"))
    {
        int _val = ini->value("device", -1).toInt(&ok);
        if(ok)
            settings.device = _val;
    }

    if(ini->contains("frequency"))
    {
        uint _val = ini->value("frequency", 44100).toUInt(&ok);
        if(ok)
            settings.frequency = _val;
    }

    if(ini->contains("icu"))
        settings.useICU = ini->value("icu", false).toBool();

    if(ini->contains("icu-min-confidence"))
    {
        int confidence = ini->value("icu-min-confidence", 0).toInt(&ok);
        if(ok)
            settings.icuMinConfidence = confidence;
    }

    if(ini->contains("log"))
        settings.logErrors = ini->value("log", false).toBool();

    if(ini->contains("min-folder-length"))
    {
        uint _val = ini->value("min-folder-length", 5).toUInt(&ok);
        if(ok)
            settings.minFolderLength = _val;
    }

    if(ini->contains("mods-cmd"))
        settings.modsCmd = stringToModifiers(ini->value("mods-cmd", "").toString());

    if(ini->contains("mods-mm"))
        settings.modsMM = stringToModifiers(ini->value("mods-mm", "").toString());

    if(ini->contains("mods-mm2"))
        settings.modsMM2= stringToModifiers(ini->value("mods-mm2", "CTRL").toString());

    if(ini->contains("mods-load"))
        settings.modsLoad = stringToModifiers(ini->value("mods-load", "CTRL").toString());

    if(ini->contains("mods-save"))
        settings.modsSave = stringToModifiers(ini->value("mods-save", "ALT").toString());

    if(ini->contains("mpris"))
        settings.mpris = ini->value("mpris", false).toBool();

    if(ini->contains("mvol-step"))
    {
        float _val = ini->value("mvol-step", 5).toFloat(&ok);
        if(ok)
            settings.mvolStep = _val;
    }

    if(ini->contains("autoload-playlists"))
        settings.playlistsAutoload = ini->value("autoload-playlists", false).toBool();

    if(ini->contains("playlists-type"))
    {
        MSE_PlaylistFormatType _val = MSE_Playlist::typeByName(ini->value("playlists-type", "M3U").toString());
        if(_val != mse_pftUnknown)
            settings.playlistsType = _val;
    }

    if(ini->contains("popup-duration"))
    {
        uint _val = ini->value("popup-duration", 5).toUInt(&ok);
        if(ok)
            settings.popupDuration = _val;
    }

    if(ini->contains("proxy"))
        settings.proxy = ini->value("proxy").toString();

    if(ini->contains("sample-interpolation"))
    {
        MSE_SoundSampleInterpolation _val = MSE_Sound::sampleInterpolationFromString(ini->value("sample-interpolation").toString(), &ok);
        if(ok)
            settings.sampleInterpolation = _val;
    }

    if(ini->contains("sample-ramping"))
    {
        MSE_SoundSampleRamping _val = MSE_Sound::sampleRampingFromString(ini->value("sample-ramping").toString(), &ok);
        if(ok)
            settings.sampleRamping = _val;
    }

    if(ini->contains("sample-ramping"))
    {
        MSE_SoundSampleType _val = MSE_Sound::sampleTypeFromString(ini->value("sample-type").toString(), &ok);
        if(ok)
            settings.sampleType = _val;
    }

    if(ini->contains("short-captions"))
        settings.shortCaptions = ini->value("short-captions", false).toBool();

    if(ini->contains("shoutcast-playlists"))
        settings.shoutcastPlaylists = ini->value("shoutcast-playlists", true).toBool();

    if(ini->contains("subdirs"))
        settings.subdirs = ini->value("subdirs", true).toBool();

    if(ini->contains("surround-mode"))
    {
        MSE_SoundSurroundMode _val = MSE_Sound::surroundModeFromString(ini->value("surround-mode").toString(), &ok);
        if(ok)
            settings.surroundMode = _val;
    }

    if(ini->contains("system-playlists"))
        settings.systemPlaylists = ini->value("system-commands", true).toBool();

    if(ini->contains("tracker-emulation"))
    {
        MSE_SoundTrackerEmulation _val = MSE_Sound::trackerEmulationFromString(ini->value("tracker-emulation").toString(), &ok);
        if(ok)
            settings.trackerEmulation = _val;
    }

    if(ini->contains("tray-icon"))
        settings.trayIcon = ini->value("tray-icon", true).toBool();

    if(ini->contains("type-timeout"))
    {
        uint _val = ini->value("type-timeout", 1000).toUInt(&ok);
        if(ok)
            settings.typeTimeout = _val;
    }

    if(ini->contains("unlimited-playlists"))
        settings.unlimitedPlaylists = ini->value("unlimited-playlists", true).toBool();

    if(ini->contains("update-period"))
    {
        uint _val = ini->value("update-period", 100).toUInt(&ok);
        if(ok)
            settings.updatePeriod = _val;
    }

    if(ini->contains("use-8bits"))
        settings.use8Bits = ini->value("use-8bits", false).toBool();

    if(ini->contains("use-default-device"))
        settings.useDefaultDevice = ini->value("use-default-device", true).toBool();

    if(ini->contains("use-software"))
        settings.useSoftware = ini->value("use-software", false).toBool();

    if(ini->contains("volume-step"))
    {
        uint _val = ini->value("volume-step", 5).toUInt(&ok);
        if(ok)
            settings.volumeStep = _val;
    }

    ini->endGroup();
    delete ini;
}

void App::parseCommandLine()
{
    bool ok;
    QString paramName, paramValue;
    PlaylistSource iniSrc;
    int p;
    QStringList args = arguments();
    if(args.size())
        args.removeFirst();

    if(args.size() == 1)
    {
        QString cmd = getCmd(args.at(0));
        if(cmd != "")
        {
            if(cmd == "quit")
                exitNow = true;
            return;
        }
    }

    foreach(QString arg, args)
    {
        if(arg.startsWith("--"))
        {
            p = arg.indexOf('=');
            if(p > 0)
            {
                paramName = arg.mid(2, p - 2);
                paramValue = arg.mid(p + 1);
                if(!QString::compare(paramValue, "true", Qt::CaseInsensitive))
                {
                    paramValue = "1";
                }
                else
                {
                    if(!QString::compare(paramValue, "false", Qt::CaseInsensitive))
                        paramValue = "0";
                }

                if(paramName == "auto-numlock")
                {
                    settings.autoNumlock = (paramValue != "0");
                    continue;
                }

                if(paramName == "auto-resume")
                {
                    settings.autoResume = (paramValue == "1");
                    continue;
                }

                if(paramName == "balloons")
                {
                    settings.balloons = (paramValue != "0");
                    settings.notifications = settings.balloons;
                    showBalloonsDeprecationWarning();
                    continue;
                }

                if(paramName == "notifications")
                {
                    settings.notifications = (paramValue != "0");
                    settings.balloons = settings.notifications;
                    continue;
                }

                if(paramName == "notifications-track-changed")
                {
                    settings.notificationsTrackChanged = stringToTrackChangedNotificationsEnum(paramValue);
                    continue;
                }

                if(paramName == "notifications-system-volume")
                {
                    settings.notificationsSystemVolume = (paramValue != "0");
                    continue;
                }

                if(paramName == "notifications-track-volume")
                {
                    settings.notificationsTrackVolume = (paramValue != "0");
                    continue;
                }

                if(paramName == "notifications-seek")
                {
                    settings.notificationsSeek = (paramValue != "0");
                    continue;
                }

                if(paramName == "buffer-length")
                {
                    uint _val = paramValue.toUInt(&ok);
                    if(ok)
                        settings.bufferLength = _val;
                    continue;
                }

                if(paramName == "channels")
                {
                    uint _val = paramValue.toUInt(&ok);
                    if(!ok)
                        settings.nChannels = _val;
                    continue;
                }

                if(paramName == "close-on-stop")
                {
                    settings.closeOnStop = (paramValue != "0");
                    continue;
                }

                if(paramName == "device")
                {
                    int _val = paramValue.toInt(&ok);
                    if(ok)
                        settings.device = _val;
                    continue;
                }

                if(paramName == "frequency")
                {
                    uint _val = paramValue.toUInt(&ok);
                    if(ok)
                        settings.frequency = _val;
                    continue;
                }

                if(paramName == "icu")
                {
                    settings.useICU = (paramValue == "1");
                    continue;
                }

                if(paramName == "icu-min-confidence")
                {
                    int _confidence = paramValue.toInt(&ok);
                    if(ok)
                        settings.icuMinConfidence = _confidence;
                    continue;
                }

                if(paramName == "index")
                {
                    uint _index = paramValue.toUInt(&ok);
                    if(!ok)
                        settings.index = _index;
                    continue;
                }

                if(paramName == "load-only")
                {
                    settings.loadOnly = (paramValue != "0");
                    continue;
                }

                if(paramName == "log")
                {
                    settings.logErrors = (paramValue == "1");
                    continue;
                }

                if(paramName == "min-folder-length")
                {
                    uint _val = paramValue.toUInt(&ok);
                    if(ok)
                        settings.minFolderLength = _val;
                    continue;
                }

                if(paramName == "mods-cmd")
                {
                    settings.modsCmd = stringToModifiers(paramValue);
                    continue;
                }

                if(paramName == "mods-mm")
                {
                    settings.modsMM = stringToModifiers(paramValue);
                    continue;
                }

                if(paramName == "mods-mm2")
                {
                    settings.modsMM2 = stringToModifiers(paramValue);
                    continue;
                }

                if(paramName == "mods-load")
                {
                    settings.modsLoad = stringToModifiers(paramValue);
                    continue;
                }

                if(paramName == "mods-save")
                {
                    settings.modsSave = stringToModifiers(paramValue);
                    continue;
                }

                if(paramName == "mpris")
                {
                    settings.mpris = (paramValue != "0");
                    continue;
                }

                if(paramName == "mvol-step")
                {
                    float _val = paramValue.toFloat(&ok);
                    if(ok)
                        settings.mvolStep = _val;
                    continue;
                }

                if(paramName == "autoload-playlists")
                {
                    settings.playlistsAutoload = (paramValue == "1");
                    continue;
                }

                if(paramName == "playback-mode")
                {
                    MSE_PlaylistPlaybackMode _mode = playlist->playbackModeFromString(paramValue, &ok);
                    if(ok)
                    {
                        settings.playbackMode = _mode;
                        settings.setPlaybackMode = true;
                    }
                    continue;
                }

                if(paramName == "playlists-type")
                {
                    MSE_PlaylistFormatType _val = MSE_Playlist::typeByName(paramValue);
                    if(_val != mse_pftUnknown)
                        settings.playlistsType = _val;
                    continue;
                }

                if(paramName == "popup-duration")
                {
                    uint _val = paramValue.toUInt(&ok);
                    if(ok)
                        settings.popupDuration = _val;
                    continue;
                }

                if(paramName == "position")
                {
                    double _val = paramValue.toDouble(&ok);
                    if(ok)
                        settings.position = _val;
                    continue;
                }

                if(paramName == "proxy")
                {
                    settings.proxy = paramValue;
                }

                if(paramName == "sample-interpolation")
                {
                    MSE_SoundSampleInterpolation _val = MSE_Sound::sampleInterpolationFromString(paramValue, &ok);
                    if(ok)
                        settings.sampleInterpolation = _val;
                    continue;
                }

                if(paramName == "sample-ramping")
                {
                    MSE_SoundSampleRamping _val = MSE_Sound::sampleRampingFromString(paramValue, &ok);
                    if(ok)
                        settings.sampleRamping = _val;
                    continue;
                }

                if(paramName == "sample-type")
                {
                    MSE_SoundSampleType _val = MSE_Sound::sampleTypeFromString(paramValue, &ok);
                    if(ok)
                        settings.sampleType = _val;
                    continue;
                }

                if(paramName == "short-captions")
                {
                    settings.shortCaptions = (paramValue == "1");
                    continue;
                }

                if(paramName == "shoutcast-playlists")
                {
                    settings.shoutcastPlaylists = (paramValue != "0");
                    continue;
                }

                if(paramName == "subdirs")
                {
                    settings.subdirs = (paramValue != "0");
                    continue;
                }

                if(paramName == "surround-mode")
                {
                    MSE_SoundSurroundMode _val = MSE_Sound::surroundModeFromString(paramValue, &ok);
                    if(ok)
                        settings.surroundMode = _val;
                    continue;
                }

                if(paramName == "system-playlists")
                {
                    settings.systemPlaylists = (paramValue != "0");
                    continue;
                }

                if(paramName == "tracker-emulation")
                {
                    MSE_SoundTrackerEmulation _val = MSE_Sound::trackerEmulationFromString(paramValue, &ok);
                    if(ok)
                        settings.trackerEmulation = _val;
                    continue;
                }

                if(paramName == "tray-icon")
                {
                    settings.trayIcon = (paramValue != "0");
                    continue;
                }

                if(paramName == "type-timeout")
                {
                    uint _val = paramValue.toUInt(&ok);
                    if(ok)
                        settings.typeTimeout = _val;
                    continue;
                }

                if(paramName == "unlimited-playlists")
                {
                    settings.unlimitedPlaylists = (paramValue == "1");
                    continue;
                }

                if(paramName == "update-period")
                {
                    uint _val = paramValue.toUInt(&ok);
                    if(ok)
                        settings.updatePeriod = _val;
                    continue;
                }

                if(paramName == "use-8bits")
                {
                    settings.use8Bits = (paramValue == "1");
                    continue;
                }

                if(paramName == "use-default-device")
                {
                    settings.useDefaultDevice = (paramValue != "0");
                    continue;
                }

                if(paramName == "use-software")
                {
                    settings.useSoftware = (paramValue == "1");
                    continue;
                }

                if(paramName == "volume")
                {
                    uint _val = paramValue.toUInt(&ok);
                    if(!ok)
                        settings.volume = _val;
                    continue;
                }

                if(paramName == "volume-step")
                {
                    uint _val = paramValue.toUInt(&ok);
                    if(ok)
                        settings.volumeStep = _val;
                    continue;
                }
            }
            continue;
        }

        iniSrc.loadFlags = mse_slfDefault;
        if(settings.playlistsAutoload)
            iniSrc.loadFlags |= mse_slfLoadPlaylists;
        if(settings.subdirs)
            iniSrc.loadFlags |= mse_slfRecurseSubdirs;
        correctPath(arg);
        iniSrc.source = arg;
        cmdPlaylist.append(iniSrc);
    }
}

void App::outputCmdResult(const QString& cmd, const QString& prefix)
{
    QIODeviceExDec dev(localSock);

    if(cmd == "get-position")
    {
        double pos = sound->getPosition();
        if(pos < 0)
            pos = 0;
        dev.writeLnUTF8(prefix+QString::number(pos, 'f', 2));
        return;
    }
    if(cmd == "get-real-position")
    {
        double pos = sound->getRealPosition();
        if(pos < 0)
            pos = 0;
        dev.writeLnUTF8(prefix+QString::number(pos, 'f', 2));
        return;
    }
    if(cmd == "get-duration")
    {
        double len = lastTrackData.duration;
        if(len < 0)
            len = 0;
        dev.writeLnUTF8(prefix+QString::number(len, 'f', 2));
        return;
    }
    if(cmd == "get-full-duration")
    {
        double len = lastTrackData.fullDuration;
        if(len < 0)
            len = 0;
        dev.writeLnUTF8(prefix+QString::number(len, 'f', 2));
        return;
    }
    if(cmd == "get-index")
    {
        dev.writeLnUTF8(prefix+QString::number(playlist->getIndex()));
        return;
    }
    if(cmd == "get-count")
    {
        dev.writeLnUTF8(prefix+QString::number(playlist->getList()->size()));
        return;
    }
    if(cmd == "get-state")
    {
        dev.writeLnUTF8(prefix+sound->channelStateToString(sound->getState()));
        return;
    }
    if(cmd == "get-playback-mode")
    {
        dev.writeLnUTF8(prefix+playlist->playbackModeToString(playlist->getPlaybackMode()));
        return;
    }
    if(cmd == "get-formatted-title")
    {
        dev.writeLnUTF8(prefix+lastTrackData.formattedTitle);
        return;
    }
    if(cmd == "get-title")
    {
        dev.writeLnUTF8(prefix+lastTrackData.title);
        return;
    }
    if(cmd == "get-artist")
    {
        dev.writeLnUTF8(prefix+lastTrackData.artist);
        return;
    }
    if(cmd == "get-filename")
    {
        dev.writeLnUTF8(prefix+lastTrackData.filename);
        return;
    }
    if(cmd == "get-full-filename")
    {
        dev.writeLnUTF8(prefix+lastTrackData.fullFilename);
        return;
    }
    if(cmd == "get-volume")
    {
        dev.writeLnUTF8(prefix+QString::number(qRound(VOLUME_OWNER->getVolume()*100)));
        return;
    }
    if(cmd == "get-master-volume")
    {
        dev.writeLnUTF8(prefix+QString::number(player->getMasterVolume()*100, 'f', 2));
        return;
    }
    if(cmd == "get-all")
    {
        outputCmdResult("get-position", "position: ");
        outputCmdResult("get-real-position", "real-position: ");
        outputCmdResult("get-duration", "duration: ");
        outputCmdResult("get-full-duration", "full-duration: ");
        outputCmdResult("get-index", "index: ");
        outputCmdResult("get-count", "count: ");
        outputCmdResult("get-state", "state: ");
        outputCmdResult("get-playback-mode", "playback-mode: ");
        outputCmdResult("get-formatted-title", "formatted-title: ");
        outputCmdResult("get-title", "title: ");
        outputCmdResult("get-artist", "artist: ");
        outputCmdResult("get-filename", "filename: ");
        outputCmdResult("get-full-filename", "full-filename: ");
        outputCmdResult("get-volume", "volume: ");
        outputCmdResult("get-master-volume", "master-volume: ");
        return;
    }
}

void App::onNewInstanceArgs()
{
    if(localSockArgs.size() == 1)
    {
        QString cmd = getCmd(localSockArgs.first());

        if(!cmd.isEmpty())
        {
            if(cmd == "quit")
            {
                quit();
                return;
            }
            if(cmd == "stop")
            {
                cmdStop();
                return;
            }
            if(cmd == "play")
            {
                cmdPlay();
                return;
            }
            if(cmd == "toggle-play")
            {
                cmdTogglePlayStop();
                outputCmdResult("get-state");
                return;
            }
            if(cmd == "pause")
            {
                cmdPause();
                return;
            }
            if(cmd == "toggle-pause")
            {
                cmdTogglePlayPause();
                outputCmdResult("get-state");
                return;
            }
            if(cmd == "volume-up")
            {
                cmdVolUp();
                outputCmdResult("get-volume");
                return;
            }
            if(cmd == "volume-down")
            {
                cmdVolDown();
                outputCmdResult("get-volume");
                return;
            }
            if(cmd == "next")
            {
                cmdNextTrack(true);
                outputCmdResult("get-all");
                return;
            }
            if(cmd == "prev")
            {
                cmdPrevTrack(true);
                outputCmdResult("get-all");
                return;
            }
            if(cmd == "open-next")
            {
                sound->openNextValid();
                outputCmdResult("get-all");
                return;
            }
            if(cmd == "open-prev")
            {
                sound->openPrevValid();
                outputCmdResult("get-all");
                return;
            }
            if(cmd == "seek-forward")
            {
                cmdSeekForward();
                outputCmdResult("get-position");
                return;
            }
            if(cmd == "seek-back")
            {
                cmdSeekBack();
                outputCmdResult("get-position");
                return;
            }

            outputCmdResult(cmd);
            return;
        }
    }

    PlaylistSource iniSrc;
    int p;
    bool ok;
    QString paramName, paramValue;
    int _volume = -1;
    int _index = -1;
    bool _mode_ok = false;
    bool _loadonly = false;
    double _position = -1;
    MSE_PlaylistPlaybackMode _mode = mse_ppmAllLoop;

    foreach(QString arg, localSockArgs)
    {
        if(arg.startsWith("--"))
        {
            p = arg.indexOf('=');
            if(p > 0)
            {
                paramName = arg.mid(2, p - 2);
                paramValue = arg.mid(p + 1);
                if(!QString::compare(paramValue, "true", Qt::CaseInsensitive))
                {
                    paramValue = "1";
                }
                else
                {
                    if(!QString::compare(paramValue, "false", Qt::CaseInsensitive))
                        paramValue = "0";
                }

                if(paramName == "autoload-playlists"){
                    settings.playlistsAutoload = (paramValue == "1");continue;
                }else
                if(paramName == "subdirs"){
                    settings.subdirs = (paramValue != "0");continue;
                }else
                if(paramName == "volume"){
                    uint _val = paramValue.toUInt(&ok);
                    if(ok)
                        _volume = _val;
                }else
                if(paramName == "index"){
                    uint _val = paramValue.toUInt(&ok);
                    if(ok)
                        _index = _val;
                }else
                if(paramName == "playback-mode"){
                    _mode = playlist->playbackModeFromString(paramValue, &_mode_ok);
                    continue;
                }else
                if(paramName == "load-only"){
                    _loadonly = (paramValue != "0");continue;
                }else
                if(paramName == "position"){
                    double _val = paramValue.toDouble(&ok);
                    if(ok && _val >= 0)
                        _position = _val;
                }
            }
            continue;
        }

        iniSrc.loadFlags = mse_slfDefault;
        if(settings.playlistsAutoload)
            iniSrc.loadFlags |= mse_slfLoadPlaylists;
        if(settings.subdirs)
            iniSrc.loadFlags |= mse_slfRecurseSubdirs;
        correctPath(arg);
        iniSrc.source = arg;
        cmdPlaylist.append(iniSrc);
    }

    if(_mode_ok)
        setPlaybackMode(_mode);

    if(_volume >= 0)
        setSoundVolume(_volume/100.0, false);

    if(!cmdPlaylist.isEmpty())
    {
        playlist->clear();
        clearLastTrackData();
        foreach(PlaylistSource src, cmdPlaylist)
            playlist->addAnything(src.source, src.loadFlags);
        cmdPlaylist.clear();
        saveStateList();
    }

    if(playlist->getList()->isEmpty())
    {
        updateTray();
    }
    else
    {
        if(_loadonly)
        {
            if(_index >= 0)
                sound->openValid(_index);
            else
                sound->openNextValid();
        }
        else
        {
            if(_index >= 0)
                sound->playValid(_index);
            else
                sound->playNextValid();
            mixer->play();
            if(_position >= 0)
                setSoundPosition(_position);
        }
    }

    showNotificationOnUserTrackChange();
}

bool App::createSoundObject()
{
    qDebug() << "initializing sound engine";
    player = MSE_Engine::getInstance();
    MSE_EngineInitParams engineParams;
    engineParams.device = settings.device;
    engineParams.outputFrequency = settings.frequency;
    engineParams.use8Bits = settings.use8Bits;
    engineParams.useDefaultDevice = settings.useDefaultDevice;
    engineParams.nChannels = settings.nChannels;
    engineParams.use3D = false;
    engineParams.bufferLength = settings.bufferLength;
    engineParams.updatePeriod = settings.updatePeriod;
    if(!player->init(engineParams))
        return false;
    qDebug() << "BASS library version:" << player->getLibVersion().asString();

//#ifndef Q_OS_OSX
    QDir tmpDir;
    QString pluginsDirname = appDir+"plugins";
    if(tmpDir.exists(pluginsDirname))
        player->loadPluginsFromDirectory(pluginsDirname);
    else
        qDebug() << "Application plugins directory not found:" << pluginsDirname;
//#else
//    player->loadPluginsFromDirectory(appDir);
//#endif

    //*
    int n = player->getPluginsCount();
    if(n)
    {
        qDebug() << "Loaded plugins:";
        for(int a=0; a<n; a++)
        {
            const MSE_EnginePluginInfo& info = player->getPluginInfo(a);
            qDebug() << " " << info.filename << info.version.asString();
            foreach(const MSE_EnginePluginFormat& format, info.formats)
            {
                qDebug() << "   " << format.description;
                foreach(QString ext, format.extensions)
                    qDebug() << "     " << QStringLiteral(".")+ext;
            }
        }
    }
    //*/

    sound = new MSE_Sound();
    MSE_SoundInitParams soundParams;
    soundParams.sampleInterpolation = settings.sampleInterpolation;
    soundParams.sampleRamping = settings.sampleRamping;
    soundParams.sampleType = settings.sampleType;
    soundParams.surroundMode = settings.surroundMode;
    soundParams.trackerEmulation = settings.trackerEmulation;
    soundParams.useSoftware = settings.useSoftware;
    soundParams.decodeOnly = true;
    soundParams.useICU = settings.useICU;
    soundParams.icuMinConfidence = settings.icuMinConfidence;
    if(!sound->init(soundParams))
        return false;
    connect(sound, SIGNAL(onInfoChange()), SLOT(onInfoChange()));
    connect(sound, SIGNAL(onContinuousStateChange()), SLOT(updateTrayIcon()));
    playlist = sound->getPlaylist();

    mixer = new MSE_Mixer();
    MSE_MixerInitParams mixerParams;
    mixerParams.useSoftware = settings.useSoftware;
    mixerParams.sampleType = settings.sampleType;
    mixer->init(mixerParams);
    mixer->addInput(sound);
    mixer->play();
    sound->setVolume(1);

    foreach(PlaylistSource src, cmdPlaylist)
        playlist->addAnything(src.source, src.loadFlags);

    cmdPlaylist.clear();

    playlist->setPlaybackMode(mse_ppmAllLoop);

    lfm = new MSE_Lastfm(sound);
    MSE_LastfmInitParams lfmParams;
    lfm->getDefaultInitParams(lfmParams);

#ifdef LFM_DECRYPT_KEY
    QFileEx f(":/services/lastfm/data");
    if(f.open(QIODevice::ReadOnly))
    {
        try{
            SimpleCrypt crypt(LFM_DECRYPT_KEY);
            QStringList lines = QString::fromUtf8(crypt.decryptToByteArray(f.readAll())).split(QStringLiteral("\n"));
            if(lines.size() < 4)
                throw;
            bool ok;
            quint64 cacheKey = lines[1].left(16).toULongLong(&ok, 16);
            if(!ok)
                throw;
            lfmParams.cacheKey = cacheKey;
            lfmParams.apiKey = lines[2].left(32);
            lfmParams.sharedSecret = lines[3].left(32);
            lfmParams.cacheFile = settingsDir+"lastfm.cache";
            lfm->init(&lfmParams);
            connect(lfm, SIGNAL(onStateChange()), SLOT(onLfmChangeState()));
        }catch(...){
            // no Last.fm support
        }
    }
#endif

    return true;
}

QAction* App::addModeAction(
        QMenu* menuModes,
        QActionGroup* group,
        const QString& title,
        MSE_PlaylistPlaybackMode mode)
{
    QAction* action = menuModes->addAction(title);
    connect(action, SIGNAL(triggered(bool)), SLOT(onActionMode(bool)));
    action->setActionGroup(group);
    QString modeStr = playlist->playbackModeToString(mode);
    action->setProperty("mode", modeStr);
    action->setCheckable(true);
    modeActions->append(action);
    return action;
}

void App::updateLastTrackData()
{
    lastTrackData.formattedTitle = sound->getTrackFormattedTitle();
    if(sound->isTrackArtistFromTags())
        lastTrackData.artist = sound->getTrackArtist();
    else
        lastTrackData.artist.clear();
    if(sound->isTrackTitleFromTags())
        lastTrackData.title = sound->getTrackTitle();
    else
        lastTrackData.title.clear();
    lastTrackData.filename = sound->getTrackFilename();
    lastTrackData.fullFilename = playlist->getCurrentSource()->getFullFilename();
    lastTrackData.duration = sound->getTrackDuration();
    lastTrackData.fullDuration = sound->getFullTrackDuration();
}

void App::clearLastTrackData()
{
    lastTrackData.formattedTitle.clear();
    lastTrackData.artist.clear();
    lastTrackData.title.clear();
    lastTrackData.filename.clear();
    lastTrackData.fullFilename.clear();
    lastTrackData.duration = 0;
    lastTrackData.fullDuration = 0;
}

void App::tryCreateTray()
{
    if(!QSystemTrayIcon::isSystemTrayAvailable())
    {
        if(trayTimer)
        {
            trayTimerTriesLeft--;
            if(!trayTimerTriesLeft)
            {
                killTrayTimer();
                SETERROR(Err::trayInaccessible);
            }
            else
            {
                qDebug() << "System tray is still inaccessible," << trayTimerTriesLeft << "tries left";
            }
        }
        else
        {
            qDebug() << "System tray is inaccessible, starting countdown";
            startTrayTimer();
        }
        return;
    }
    killTrayTimer();

    if(!createTray())
        quit();
}

void App::moveCurFileToTrash()
{
    if(sound->getType() != mse_sctRemote)
    {
        cmdStop(true);
        if(MoveToTrash::doIt(lastTrackData.filename))
            cmdNextTrack(true);
        else
            showTrayMessage(tr("Cannot move the current file to the Trash"));
    }
}

bool App::createTray()
{
    QString icoDir = appDir+"icons/";

    QString icoFilename = icoDir + "play.png";
    CHECK(QFile::exists(icoFilename), Err::iconFileNotFound, icoFilename);
    icoPlay = new QIcon(icoFilename);
    icoFilename = icoDir + "pause.png";
    CHECK(QFile::exists(icoFilename), Err::iconFileNotFound, icoFilename);
    icoPause = new QIcon(icoFilename);
    icoFilename = icoDir + "stop.png";
    CHECK(QFile::exists(icoFilename), Err::iconFileNotFound, icoFilename);
    icoStop = new QIcon(icoFilename);

    tray = new QSystemTrayIcon(this);
    if(settings.notifications)
    {
        icoFilename = icoDir + "app.ico";
        CHECK(QFile::exists(icoFilename), Err::iconFileNotFound, icoFilename);
        icoApp = new QIcon(icoFilename);
        trayPopup = new NotificationPopup(tray, dummyWindow);
        trayPopup->setIcon(*icoApp);
    }

    trayMenu = new QMenu(dummyWindow);
    tray->setContextMenu(trayMenu);
    updateTrayIcon();

    QMenu* trayMenu =
            tray->contextMenu();

    if(MoveToTrash::canDoIt())
    {
        QAction* actionTrash = trayMenu->addAction(tr("Move to Trash"));
        connect(actionTrash, &QAction::triggered, this, [this]{
            moveCurFileToTrash();
        });
    }

    modeActions = new QList<QAction*>;
    QMenu* menuModes = trayMenu->addMenu(tr("Mode"));
    modeGroup = new QActionGroup(this);
    modeGroup->setExclusive(true);
    addModeAction(menuModes, modeGroup, tr("Track once"), mse_ppmTrackOnce);
    addModeAction(menuModes, modeGroup, tr("Playlist once"), mse_ppmAllOnce);
    addModeAction(menuModes, modeGroup, tr("Loop track"), mse_ppmTrackLoop);
    addModeAction(menuModes, modeGroup, tr("Loop playlist"), mse_ppmAllLoop);
    addModeAction(menuModes, modeGroup, tr("Random"), mse_ppmRandom);
    updateModeGroup(playlist->getPlaybackMode());

    actionPlaylists = trayMenu->addAction(tr("Playlists"));
    connect(actionPlaylists, SIGNAL(triggered()), SLOT(onActionPlaylists()));
    actionHotkeys = trayMenu->addAction(tr("Hot keys"));
    actionHotkeys->setCheckable(true);
    actionHotkeys->setChecked(hotkeysOn);
    connect(actionHotkeys, SIGNAL(triggered(bool)), SLOT(onActionHotkeys(bool)));

    auto menuSettings = trayMenu->addMenu(tr("Settings"));
    connect(menuSettings->addAction(tr("Open settings file")), &QAction::triggered, [=]{
        if(createEmptyUserSettingsFileIfNeeded())
            viewFile(userSettingsFilename);
    });
    connect(menuSettings->addAction(tr("Show settings location")), &QAction::triggered, [=]{
        if(createEmptyUserSettingsFileIfNeeded())
            showFile(userSettingsFilename);
    });

    actionCurrentFile = trayMenu->addAction(tr("Current File"));
    connect(actionCurrentFile, SIGNAL(triggered()), SLOT(onActionCurrentFile()));

    actionCloseFile = trayMenu->addAction(tr("Close the File"));
    connect(actionCloseFile, SIGNAL(triggered()), SLOT(onActionCloseFile()));
    actionCloseFile->setVisible(!settings.closeOnStop);

    QMenu* menuLfm = trayMenu->addMenu(tr("Last.fm"));
    if(!lfm->getInitParams().sharedSecret.isEmpty())
    {
        actionLfmLoginWeb = menuLfm->addAction(tr("Web login"));
        connect(actionLfmLoginWeb, &QAction::triggered, [this](){
            showLfmLoginNotification = true;
            lfm->startWebAuth();
        });
        actionLfmLoginForm = menuLfm->addAction(tr("In-App login"));
        connect(actionLfmLoginForm, &QAction::triggered, [this](){
            LastfmPassForm::showForm([this](const QString& username, const QString& password){
                showLfmLoginNotification = true;
                lfm->startMobileAuth(username, password);
            });
        });
        actionLfmUser = menuLfm->addAction("");
        connect(actionLfmUser, &QAction::triggered, [this](){
            if(lfm->getState() == mse_lfmLoggedIn)
            {
                QString url(QStringLiteral("https://www.last.fm/user/"));
                url.append(lfm->getUserName());
                QDesktopServices::openUrl(url);
            }
            else
            {
                QDesktopServices::openUrl(QUrl("https://www.last.fm/"));
            }
        });
        actionLfmLogout = menuLfm->addAction(tr("Log out"));
        connect(actionLfmLogout, &QAction::triggered, [this](){
            lfm->logout();
            showTrayMessage(QStringLiteral("Last.fm: ")+tr("logged out"));
            updateLfmMenu();
        });

        updateLfmMenu();
    }
    if(menuLfm->isEmpty())
    {
        delete menuLfm;
    }

    QMenu* menuHelp = trayMenu->addMenu(tr("Help"));
    connect(menuHelp->addAction(tr("Manual")), &QAction::triggered, [](){
        QString fileName = applicationDirPath()+"/manual/index.html";
        QUrl url;
        url.setPath(fileName);
        url.setScheme("file");
        QDesktopServices::openUrl(url);
    });
    connect(menuHelp->addAction(tr("Website")), &QAction::triggered, [](){
        QDesktopServices::openUrl(QUrl("https://mesonplayer.alkatrazstudio.net"));
    });
    connect(menuHelp->addAction(tr("About...")), &QAction::triggered, [this](){
        if(aboutDialog)
            return;

        aboutDialog = new QDialog();
        aboutDialog->setWindowFlags(aboutDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
        aboutDialog->setFixedSize(320, 240);
        connect(aboutDialog, &QDialog::finished, [this](){
            aboutDialog->deleteLater();
            aboutDialog = nullptr;
        });

        QString html =
            "<h3>"+qApp->applicationDisplayName().toHtmlEscaped()+"</h3>"
            "<p>"+tr("Version:").toHtmlEscaped()+" "+CoreApp::majMinVersion().toString().toHtmlEscaped()+"</p>"
            "<p>"+tr("Built:").toHtmlEscaped()+" "+CoreApp::buildDate().toString(Qt::DefaultLocaleLongDate)+"</p>"
            "<p>&copy; "+qApp->organizationName().toHtmlEscaped()+", "+QString::number(CoreApp::buildDate().date().year())+"</p>";

        aboutText = new QTextBrowser(aboutDialog);
        aboutText->move(0, 0);
        aboutText->resize(aboutDialog->size());
        QTextOption textOption = aboutText->document()->defaultTextOption();
        textOption.setAlignment(Qt::AlignCenter);
        aboutText->document()->setDefaultTextOption(textOption);
        aboutText->setHtml(html);

        aboutDialog->show();
        aboutDialog->raise();
        aboutDialog->activateWindow();
    });

    QString s(tr("Quit"));
#ifdef Q_OS_OSX
    // OSX (or Qt5 on OSX) hides the last entry in a tray popup menu
    // that contains "Quit" or "Exit" text in it,
    // so the following is a hack to prevent such disappearance
    s.insert(1, "\u00ad");
#endif
    actionExit = trayMenu->addAction(s);
    connect(actionExit, SIGNAL(triggered()), SLOT(onActionExit()));

    return true;
}

void App::updateModeGroup(MSE_PlaylistPlaybackMode playbackMode)
{
    QString ppmStr(playlist->playbackModeToString(playbackMode));
    foreach(QAction* action, modeGroup->actions())
    {
        if(ppmStr == action->property("mode"))
        {
            action->setChecked(true);
            break;
        }
    }
}

void App::updateLfmMenu()
{
    if(!tray || !actionLfmUser)
        return;

    QString s;
    switch(lfm->getState())
    {
        case mse_lfmIdle:
            s = QStringLiteral("<")+tr("not logged in")+">";
            showTrayMessage(QStringLiteral("Last.fm: ")+tr("logged out"));
            break;

        case mse_lfmGetToken:
            s = QStringLiteral("<")+tr("connecting...")+">";
            actionLfmUser->setText("Last.fm: "+s);
            showTrayMessage(QStringLiteral("Last.fm: ")+tr("connecting..."));
            break;

        case mse_lfmGetSession:
            s = QStringLiteral("<")+tr("waiting for authorization...")+">";
            actionLfmUser->setText("Last.fm: "+s);
            showTrayMessage(QStringLiteral("Last.fm: ")+tr("waiting for authorization..."));
            break;

        case mse_lfmLoggedIn:
            s = lfm->getUserName();
            if(showLfmLoginNotification)
                showTrayMessage(QStringLiteral("Last.fm: \"")+s+"\" - "+tr("logged in"));
            actionLfmUser->setText("Last.fm: "+s);
            break;
    }

    bool isIdle = lfm->getState() == mse_lfmIdle;
    actionLfmLoginWeb->setVisible(isIdle);
    actionLfmLoginForm->setVisible(isIdle);
    actionLfmUser->setVisible(!isIdle);
    actionLfmLogout->setVisible(!isIdle);
}

QString App::hotkeyToString(const HotKeyInfo& info)
{
    return QStringLiteral("id=")
            +QString::number(info.id)
            +", key="
            +QString::number(info.nKey)
            +", mod="
            +QString::number(info.nMod);
}

bool App::registerHotKey(HotKeyInfo& info)
{
    if(info.isRegistered)
        return true;
    if(info.isDisabled)
        return true;
    bool result;
    try{
#ifdef Q_OS_WIN
        result = RegisterHotKey(0, info.id, info.nMod, info.nKey);
#endif
#ifdef Q_OS_LINUX
        result = xcb_ok(xcb_grab_key_checked(xconn, 0, xwin, info.nMod, info.nKey,
                                             XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC));
#endif
#ifdef Q_OS_OSX
        EventHotKeyID hotkeyId;
        hotkeyId.id = info.id;
        int errCode = RegisterEventHotKey(info.nKey, info.nMod, hotkeyId,
                                          GetApplicationEventTarget(), 0, &info.ref);
        result = (errCode == noErr);
#endif
    }catch(...){
        result = false;
    }
    if(!result)
        SETERROR(Err::registerHotKey, hotkeyToString(info));
    else
        info.isRegistered = true;
    return result;
}

bool App::unregisterHotKey(HotKeyInfo &info)
{
    if(!info.isRegistered)
        return true;
    if(info.isDisabled)
        return true;
    bool result;
    try{
#ifdef Q_OS_WIN
        result = UnregisterHotKey(0, info.id);
#endif
#ifdef Q_OS_LINUX
        result = xcb_ok(xcb_ungrab_key_checked(xconn, info.nKey, xwin, info.nMod));
#endif
#ifdef Q_OS_OSX
        result = (UnregisterEventHotKey(info.ref) == noErr);
#endif
    }catch(...){
        result = false;
    }
    if(!result)
        SETERROR(Err::unregisterHotKey, hotkeyToString(info));
    else
        info.isRegistered = false;
    return result;
}

bool App::registerHotKeys(HotKeysInfo& infos)
{
    bool result = true;
    HotKeysInfo::iterator i = infos.begin();
    HotKeysInfo::iterator iEnd = infos.end();
    while(i != iEnd)
    {
        if(!registerHotKey(*i))
            result = false;
        i++;
    }
    return result;
}

bool App::unregisterHotKeys(HotKeysInfo &infos)
{
    bool result = true;
    HotKeysInfo::iterator i = infos.begin();
    HotKeysInfo::iterator iEnd = infos.end();
    while(i != iEnd)
    {
        if(!unregisterHotKey(*i))
            result = false;
        i++;
    }
    return result;
}

bool App::registerControlHotKeys()
{
    bool result = true;
    if(!registerHotKeys(hotKeysNum))
        result = false;
    if(!registerHotKeys(hotKeysMedia))
        result = false;
    if(!registerAppCommandHotKeys())
        result = false;

    hotkeysOn = true;
    if(actionHotkeys)
        actionHotkeys->setChecked(hotkeysOn);

    return result;
}

bool App::unregisterControlHotKeys()
{
    bool result = true;
    if(!unregisterHotKeys(hotKeysNum))
        result = false;
    if(!unregisterHotKeys(hotKeysMedia))
        result = false;
    if(!unregisterAppCommandHotKeys())
        result = false;

    hotkeysOn = false;
    if(actionHotkeys)
        actionHotkeys->setChecked(hotkeysOn);
    return result;
}

bool App::registerAppCommandHotKeys()
{
    appCommandHotkeysOn = true;
    if(appCommandInput)
        return true;
    return registerHotKeys(hotKeysApp);
}

bool App::unregisterAppCommandHotKeys()
{
    appCommandHotkeysOn = false;
    if(appCommandInput)
        return true;
    return unregisterHotKeys(hotKeysApp);
}

void App::updateTray(bool showNotification, bool showTime)
{
    startSaveTimer();

#ifdef Q_OS_WIN
    QString endl("\r\n");
#else
    QString endl("\n");
#endif

    if(!tray)
        return;

    QString iTip, bTip;
    QString n;
    n.setNum(playlist->getIndex() + 1);
    if(lastTrackData.formattedTitle.isEmpty())
    {
        QString _nm = QStringLiteral(" - ")+tr("file not loaded")+" - ";
        if(!settings.shortCaptions)
        {
            iTip = QStringLiteral("[???] - ")+curVol;
            iTip.append(endl+_nm);
            bTip = QStringLiteral("[???]")+endl+_nm;
        }
        else
        {
            iTip = _nm;
            bTip = _nm;
        }
    }
    else
    {
        if(!settings.shortCaptions)
        {
            iTip = QStringLiteral("[")+curDir+"] - "+curVol;
            iTip.append(endl+n+". "+lastTrackData.formattedTitle);
            bTip = QStringLiteral("[")+curDir+"]"+endl+n+". "+lastTrackData.formattedTitle;
        }
        else
        {
            iTip = lastTrackData.formattedTitle;
            bTip = lastTrackData.formattedTitle;
        }
    }

    tray->setToolTip(iTip);
    if(showNotification)
    {
        if(showTime)
        {
            double secs = sound->getPosition();
            double dur = sound->getTrackDuration();
            bTip += endl + "[" + secsToTimeFormat(secs) + " / " + secsToTimeFormat(dur) + "]";
        }
        showTrayMessage(bTip);
    }
}

void App::showTrayMessage(const QString &msg)
{
    if(!tray)
        return;
    if(!trayPopup)
        return;
    if(!tray->isVisible())
        return;

    QString s = qCoreApp->applicationDisplayName() + " ["+curVol+"]";
    trayPopup->show(s, msg, settings.popupDuration * 1000, tray);
}

void App::showMasterVolumeInfo()
{
    QString s = tr("System audio volume")+": "+QString::number(player->getMasterVolume()*100, 'f', 1)+"%";
    showTrayMessage(s);
}

void App::updateVolumeLabel()
{
    curVol.setNum(qRound(VOLUME_OWNER->getVolume()*100));
    curVol.append("%");
}

void App::saveState()
{
    if(!player || !sound)
        return;

    QSettings* ini = new QSettings(settingsDir+"state.ini", QSettings::IniFormat);
    ini->beginGroup("state");
    ini->setValue("volume", qRound(VOLUME_OWNER->getVolume()*100));
    ini->setValue("play", sound->getState() == mse_scsPlaying);
    if((playlist->getIndex() >= 0) && (playlist->getList()->size() > playlist->getIndex()))
        ini->setValue("last", playlist->getList()->at(playlist->getIndex())->getFullFilename());
    else
        ini->setValue("last", "");
    ini->setValue("mode", playlist->playbackModeToString(playlist->getPlaybackMode()));
    ini->setValue("hotkeys", hotkeysOn);
    ini->endGroup();
    delete ini;
}

void App::saveStateList()
{
    if(!player || !sound)
        return;

    QString fName = settingsDir+"playlist";
    playlist->write(fName);
}

void App::onLfmChangeState()
{
    updateLfmMenu();
}

void App::loadState()
{
    QSettings* ini = new QSettings(settingsDir+"state.ini", QSettings::IniFormat);
    ini->beginGroup("state");
    if(settings.volume < 0)
    {
        bool ok;
        int i = ini->value("volume", 100).toUInt(&ok);
        if(!ok)
            i = 100;
        VOLUME_OWNER->setVolume(i/100.0);
    }
    else
    {
        VOLUME_OWNER->setVolume(settings.volume);
    }

    MSE_PlaylistPlaybackMode playbackMode;
    if(settings.setPlaybackMode)
        playbackMode = settings.playbackMode;
    else
        playbackMode = playlist->playbackModeFromString(ini->value("mode", false).toString());
    setPlaybackMode(playbackMode);

    if(!hasInputFiles)
    {
        doPlay = ini->value("play", false).toBool() && settings.autoResume;
        QString fName = settingsDir+"playlist";
        playlist->parse(fName, statePlaylist);
        lastFilename = ini->value("last", "").toString();
    }
    else
    {
        lastFilename = "";
        doPlay = false;
    }
    hotkeysOn = ini->value("hotkeys", true).toBool();
    if(actionHotkeys)
        actionHotkeys->setChecked(hotkeysOn);
    ini->endGroup();
    delete ini;
}

void App::setPlaybackMode(MSE_PlaylistPlaybackMode mode)
{
    if(tray)
    {
        QString modeStr = playlist->playbackModeToString(mode);
        foreach(QAction* action, *modeActions)
        {
            if(action->property("mode") == modeStr)
            {
                action->setChecked(true);
                break;
            }
        }
    }

    // setChecked emits toggled(), but we're only listening for triggered(),
    // so we need to manually set playback mode here
    playlist->setPlaybackMode(mode);
}

void App::initSound()
{
    if(settings.volume >= 0)
        VOLUME_OWNER->setVolume(settings.volume/100.0);
    updateVolumeLabel();

    if(playlist->getList()->isEmpty() && statePlaylist.size())
    {
        playlist->addAnything(statePlaylist, mse_slfDefault);
        bool hasLastIndex = lastFilename.size() || (settings.index >= 0);
        if(hasLastIndex)
        {
            int fileIndex;
            if(settings.index >= 0)
                fileIndex = settings.index;
            else
                fileIndex = playlist->indexOfFullName(lastFilename);
            ignoreContState = true;
            sound->openFromList(fileIndex);
            ignoreContState = false;
        }
        if(tray)
            tray->show();
        if(doPlay)
        {
            if(hasLastIndex)
            {
                updateTray();
                if(!settings.loadOnly)
                    sound->play();
                showNotificationOnUserTrackChange();
            }
            else
            {
                if(settings.loadOnly)
                {
                    sound->openNextValid();
                    cmdStop();
                }
                else
                {
                    sound->playNextValid();
                }
                if(settings.position >= 0)
                    setSoundPosition(settings.position);
                showNotificationOnUserTrackChange();
                return;
            }
        }
    }
    if((sound->getType() == mse_sctUnknown) && (doPlay || !playlist->getList()->isEmpty()) && !settings.loadOnly)
    {
        if(!tray->isVisible())
            tray->show();
        if((settings.index >= 0) && (!playlist->getList()->isEmpty()))
        {
            sound->playValid(settings.index);
            showNotificationOnUserTrackChange();
        }
        else
        {
            sound->playNextValid();
            if(settings.position >= 0)
                setSoundPosition(settings.position);
            showNotificationOnUserTrackChange();
            return;
        }
    }

    if(settings.position >= 0)
        setSoundPosition(settings.position);

    if(sound->getState() != mse_scsPlaying)
        sound->close();
}

int App::getNativeKey(Qt::Key key)
{
#ifdef Q_OS_WIN
    switch(key)
    {
        case Qt::Key_0:
            return VK_NUMPAD0;

        case Qt::Key_1:
            return VK_NUMPAD1;

        case Qt::Key_2:
            return VK_NUMPAD2;

        case Qt::Key_3:
            return VK_NUMPAD3;

        case Qt::Key_4:
            return VK_NUMPAD4;

        case Qt::Key_5:
            return VK_NUMPAD5;

        case Qt::Key_6:
            return VK_NUMPAD6;

        case Qt::Key_7:
            return VK_NUMPAD7;

        case Qt::Key_8:
            return VK_NUMPAD8;

        case Qt::Key_9:
            return VK_NUMPAD9;

        case Qt::Key_MediaTogglePlayPause:
            return VK_MEDIA_PLAY_PAUSE;

        case Qt::Key_MediaPrevious:
            return VK_MEDIA_PREV_TRACK;

        case Qt::Key_MediaNext:
            return VK_MEDIA_NEXT_TRACK;

        default:
            return 0;
    }
#endif

#ifdef Q_OS_LINUX
    int nKey;
    switch(key)
    {
        case Qt::Key_0:
            nKey = XK_KP_0;
            break;

        case Qt::Key_1:
            nKey = XK_KP_1;
            break;

        case Qt::Key_2:
            nKey = XK_KP_2;
            break;

        case Qt::Key_3:
            nKey = XK_KP_3;
            break;

        case Qt::Key_4:
            nKey = XK_KP_4;
            break;

        case Qt::Key_5:
            nKey = XK_KP_5;
            break;

        case Qt::Key_6:
            nKey = XK_KP_6;
            break;

        case Qt::Key_7:
            nKey = XK_KP_7;
            break;

        case Qt::Key_8:
            nKey = XK_KP_8;
            break;

        case Qt::Key_9:
            nKey = XK_KP_9;
            break;

        case Qt::Key_MediaTogglePlayPause:
            nKey = XF86XK_AudioPlay;
            break;

        case Qt::Key_MediaPrevious:
            nKey = XF86XK_AudioPrev;
            break;

        case Qt::Key_MediaNext:
            nKey = XF86XK_AudioNext;
            break;

        default:
            return 0;
    }
    xcb_keycode_t* keycode = xcb_key_symbols_get_keycode(xkeysyms, nKey);
    if(!keycode)
        return 0;
    xcb_keycode_t result = keycode[0];
    free(keycode);
    if(result == XCB_NO_SYMBOL)
        return 0;
    return result;
#endif

#ifdef Q_OS_OSX
    switch(key)
    {
        case Qt::Key_0:
            return kVK_ANSI_Keypad0;

        case Qt::Key_1:
            return kVK_ANSI_Keypad1;

        case Qt::Key_2:
            return kVK_ANSI_Keypad2;

        case Qt::Key_3:
            return kVK_ANSI_Keypad3;

        case Qt::Key_4:
            return kVK_ANSI_Keypad4;

        case Qt::Key_5:
            return kVK_ANSI_Keypad5;

        case Qt::Key_6:
            return kVK_ANSI_Keypad6;

        case Qt::Key_7:
            return kVK_ANSI_Keypad7;

        case Qt::Key_8:
            return kVK_ANSI_Keypad8;

        case Qt::Key_9:
            return kVK_ANSI_Keypad9;

        case Qt::Key_MediaTogglePlayPause:
        case Qt::Key_MediaPrevious:
        case Qt::Key_MediaNext:
        default:
            return 0;
    }
#endif
}

int App::getNativeKeyMod(Qt::KeyboardModifiers mod)
{
    int nMod = 0;
#ifdef Q_OS_WIN
    if(mod & Qt::ShiftModifier)
        nMod |= MOD_SHIFT;
    if(mod & Qt::ControlModifier)
        nMod |= MOD_CONTROL;
    if(mod & Qt::AltModifier)
        nMod |= MOD_ALT;
#endif
#ifdef Q_OS_LINUX
    if(mod & Qt::ShiftModifier)
        nMod |= XCB_MOD_MASK_SHIFT;
    if(mod & Qt::ControlModifier)
        nMod |= XCB_MOD_MASK_CONTROL;
    if(mod & Qt::AltModifier)
        nMod |= XCB_MOD_MASK_1;
#endif
#ifdef Q_OS_OSX
    if(mod & Qt::ShiftModifier)
        nMod |= shiftKey;
    if(mod & Qt::ControlModifier)
        nMod |= cmdKey;
    if(mod & Qt::AltModifier)
        nMod |= optionKey;
#endif
    return nMod;
}

void App::getHotKeyInfo(HotKeyInfo &info, int id, Qt::Key key, Qt::KeyboardModifiers mod)
{
    info.id = id;
    info.nKey = getNativeKey(key);
    info.nMod = getNativeKeyMod(mod);
    info.isDisabled = (mod == Qt::GroupSwitchModifier);
}

void App::addHotKeyInfo(HotKeysInfo &infos, int id, Qt::Key key, Qt::KeyboardModifiers mod)
{
    HotKeyInfo info;
    getHotKeyInfo(info, id, key, mod);
    info.isRegistered = false;
    infos.append(info);
#ifdef Q_OS_LINUX
    info.nMod |= XCB_MOD_MASK_2;
    infos.append(info);
    info.nMod |= XCB_MOD_MASK_LOCK;
    infos.append(info);
#endif

}

void App::addHotKeysInfo()
{
    if(settings.systemPlaylists)
    {
        addHotKeyInfo(hotKeysAppMain, APPKEYBASE_LOAD_PLAYLIST+0, Qt::Key_0, settings.modsLoad);
        addHotKeyInfo(hotKeysApp, APPKEYBASE_LOAD_PLAYLIST+1, Qt::Key_1, settings.modsLoad);
        addHotKeyInfo(hotKeysApp, APPKEYBASE_LOAD_PLAYLIST+2, Qt::Key_2, settings.modsLoad);
        addHotKeyInfo(hotKeysApp, APPKEYBASE_LOAD_PLAYLIST+3, Qt::Key_3, settings.modsLoad);
        addHotKeyInfo(hotKeysApp, APPKEYBASE_LOAD_PLAYLIST+4, Qt::Key_4, settings.modsLoad);
        addHotKeyInfo(hotKeysApp, APPKEYBASE_LOAD_PLAYLIST+5, Qt::Key_5, settings.modsLoad);
        addHotKeyInfo(hotKeysApp, APPKEYBASE_LOAD_PLAYLIST+6, Qt::Key_6, settings.modsLoad);
        addHotKeyInfo(hotKeysApp, APPKEYBASE_LOAD_PLAYLIST+7, Qt::Key_7, settings.modsLoad);
        addHotKeyInfo(hotKeysApp, APPKEYBASE_LOAD_PLAYLIST+8, Qt::Key_8, settings.modsLoad);
        addHotKeyInfo(hotKeysApp, APPKEYBASE_LOAD_PLAYLIST+9, Qt::Key_9, settings.modsLoad);
    }

    addHotKeyInfo(hotKeysNum, APPKEY_PAUSE, Qt::Key_0, settings.modsCmd);
    addHotKeyInfo(hotKeysNum, APPKEY_MVOL_DOWN, Qt::Key_1, settings.modsCmd);
    addHotKeyInfo(hotKeysNum, APPKEY_VOL_DOWN, Qt::Key_2, settings.modsCmd);
    addHotKeyInfo(hotKeysNum, APPKEY_MVOL_UP, Qt::Key_3, settings.modsCmd);
    addHotKeyInfo(hotKeysNum, APPKEY_PREV_TRACK, Qt::Key_4, settings.modsCmd);
    addHotKeyInfo(hotKeysNum, APPKEY_PLAY, Qt::Key_5, settings.modsCmd);
    addHotKeyInfo(hotKeysNum, APPKEY_NEXT_TRACK, Qt::Key_6, settings.modsCmd);
    addHotKeyInfo(hotKeysNum, APPKEY_PREV_DIR, Qt::Key_7, settings.modsCmd);
    addHotKeyInfo(hotKeysNum, APPKEY_VOL_UP, Qt::Key_8, settings.modsCmd);
    addHotKeyInfo(hotKeysNum, APPKEY_NEXT_DIR, Qt::Key_9, settings.modsCmd);

    if(!settings.systemPlaylists)
    {
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+0, Qt::Key_0, settings.modsLoad);
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+1, Qt::Key_1, settings.modsLoad);
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+2, Qt::Key_2, settings.modsLoad);
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+3, Qt::Key_3, settings.modsLoad);
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+4, Qt::Key_4, settings.modsLoad);
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+5, Qt::Key_5, settings.modsLoad);
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+6, Qt::Key_6, settings.modsLoad);
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+7, Qt::Key_7, settings.modsLoad);
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+8, Qt::Key_8, settings.modsLoad);
        addHotKeyInfo(hotKeysNum, APPKEYBASE_LOAD_PLAYLIST+9, Qt::Key_9, settings.modsLoad);
    }

    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+0, Qt::Key_0, settings.modsSave);
    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+1, Qt::Key_1, settings.modsSave);
    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+2, Qt::Key_2, settings.modsSave);
    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+3, Qt::Key_3, settings.modsSave);
    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+4, Qt::Key_4, settings.modsSave);
    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+5, Qt::Key_5, settings.modsSave);
    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+6, Qt::Key_6, settings.modsSave);
    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+7, Qt::Key_7, settings.modsSave);
    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+8, Qt::Key_8, settings.modsSave);
    addHotKeyInfo(hotKeysNum, APPKEYBASE_SAVE_PLAYLIST+9, Qt::Key_9, settings.modsSave);

    addHotKeyInfo(hotKeysMedia, APPKEYBASE_MM + APPKEY_PREV_TRACK, Qt::Key_MediaPrevious, settings.modsMM);
    addHotKeyInfo(hotKeysMedia, APPKEYBASE_MM + APPKEY_PLAY, Qt::Key_MediaTogglePlayPause, settings.modsMM);
    addHotKeyInfo(hotKeysMedia, APPKEYBASE_MM + APPKEY_NEXT_TRACK, Qt::Key_MediaNext, settings.modsMM);
    addHotKeyInfo(hotKeysMedia, APPKEYBASE_MM + APPKEY_PREV_DIR, Qt::Key_MediaPrevious, settings.modsMM2);
    addHotKeyInfo(hotKeysMedia, APPKEYBASE_MM + APPKEY_NEXT_DIR, Qt::Key_MediaNext, settings.modsMM2);
    addHotKeyInfo(hotKeysMedia, APPKEYBASE_MM + APPKEY_PAUSE, Qt::Key_MediaTogglePlayPause, settings.modsMM2);
}

void App::setNumLock()
{
#ifdef Q_OS_WIN
    if(!(GetKeyState(VK_NUMLOCK) & 0x1))
    {
        keybd_event(VK_NUMLOCK, VK_NUMLOCK, 0, 0);
        keybd_event(VK_NUMLOCK, VK_NUMLOCK, KEYEVENTF_KEYUP, 0);
    }
#endif
#ifdef Q_OS_LINUX
    // everything is easier on Linux...
/*
 * TODO: xcb solution
 *
    XkbDescRec* xkb = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
    if(!xkb)
        return;
    if(!xkb->names)
        return;

    int i;
    for(i=0; i<XkbNumVirtualMods; i++)
    {
        char* modStr = XGetAtomName(xkb->dpy, xkb->names->vmods[i]);
        if(!modStr)
            continue;

        if(strcmp("NumLock", modStr))
            continue;
        unsigned int modMask;
        XkbVirtualModsToReal(xkb, 1<<i, &modMask);
        XkbFreeKeyboard(xkb, 0, True);

        XkbStateRec rec;
        XkbGetState(dpy, XkbUseCoreKbd, &rec);
        if(rec.locked_mods & modMask)
            return;
        XkbLockModifiers(dpy, XkbUseCoreKbd, modMask, modMask);
        return;
    }
    */
#endif
}

Qt::KeyboardModifiers App::stringToModifiers(const QString &str) const
{
    Qt::KeyboardModifiers mods = Qt::NoModifier;
    QStringList list = str.split("+");
    foreach(QString item, list)
    {
        item = item.trimmed().toUpper();
        if(item == "CTRL")
            mods |= Qt::ControlModifier;
        else
            if(item == "ALT")
                mods |= Qt::AltModifier;
            else
                if(item == "SHIFT")
                    mods |= Qt::ShiftModifier;
                else
                    if(item == "OFF")
                    {
                        mods = Qt::GroupSwitchModifier; // stands for "keys are disabled"
                        break;
                    }
    }
    return mods;
}

void App::correctPath(QString &s)
{
#ifdef Q_OS_WIN
    if(s.endsWith("\""))
        s.chop(1);
#else
    Q_UNUSED(s);
#endif
}

#ifdef Q_OS_LINUX
int App::searchHotKey(const HotKeysInfo &infos, int nKey, uint16_t nMod)
{
    foreach(const HotKeyInfo info, infos)
        if((info.nKey == nKey) && (info.nMod == nMod))
            return info.id;
    return -1;
}

bool App::handleXKeyPress(const xcb_key_press_event_t *event, bool isPressed)
{
    int id;
    uint16_t nMod = event->state & (XCB_MOD_MASK_SHIFT | XCB_MOD_MASK_CONTROL | XCB_MOD_MASK_1);
    id = searchHotKey(hotKeysNum, event->detail, nMod);
    if(id < 0)
    {
        id = searchHotKey(hotKeysNum, event->detail, nMod);
        if(id < 0)
        {
            id = searchHotKey(hotKeysApp, event->detail, nMod);
            if(id < 0)
            {
                id = searchHotKey(hotKeysAppMain, event->detail, nMod);
                if(id < 0)
                    return false;
            }
        }
    }

    onHotKey(id, isPressed);
    return true;
}

bool App::xcb_ok(xcb_void_cookie_t cookie)
{
    xcb_generic_error_t* err = xcb_request_check(xconn, cookie);
    if(!err)
        return true;
    return err->error_code == 0;
}
#endif

QString App::getPlaylistFilename()
{
    int n = playlistPrefix.size();
    if(!n)
        return "";
    QString dirName = settingsDir+"playlists/";
    QDir dir(dirName);
    QStringList nameFilters;
    nameFilters << (playlistPrefix+"*");
    QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable);
    QChar c;
    foreach(QString filename, files)
    {
        if(filename.size() == n)
            return dirName + filename;
        c = filename.at(n);
        if((c <= '0') || (c >= '9'))
            return dirName + filename;
    }
    return dirName + playlistPrefix + playlistsExt;
}

void App::loadPlaylist()
{
    QString filename = getPlaylistFilename();
    if(QFile::exists(filename))
    {
        playlist->clear();
        clearLastTrackData();
        playlist->addFromPlaylist(filename);
        saveStateList();
        mixer->stop();
        sound->playNextValid();
        mixer->play();
    }
    else
    {
        if(settings.shoutcastPlaylists && playlistPrefix.startsWith("0") && (playlistPrefix.size() > 1))
        {
            QString url = "https://yp.shoutcast.com/sbin/tunein-station.pls?id="+playlistPrefix.mid(1);
            playlist->clear();
            clearLastTrackData();
            playlist->addUrl(url);
            saveStateList();
            mixer->stop();
            sound->playNextValid();
            mixer->play();
        }
    }
    updateTray(false);
}

void App::savePlaylist()
{
    QString filename = getPlaylistFilename();
    QFileInfo i(filename);
    QString dirname = i.absolutePath();
    QDir dir(dirname);
    if(!dir.exists())
        CHECKV(dir.mkdir(dirname), Err::createDir, dirname);
    playlist->write(filename, settings.playlistsType);
}

void App::startPrefixTimer()
{
    killPrefixTimer();
    prefixTimer = startTimer(settings.typeTimeout);
}

void App::killPrefixTimer()
{
    if(prefixTimer)
        killTimer(prefixTimer);
    prefixTimer = 0;
}

void App::startTrayTimer()
{
    killTrayTimer();
    trayTimer = startTimer(trayTimerInterval);
}

void App::killTrayTimer()
{
    if(trayTimer)
        killTimer(trayTimer);
    trayTimer = 0;
}

void App::startSaveTimer()
{
    killSaveTimer();
    saveTimer = startTimer(saveTimerInterval);
}

void App::killSaveTimer()
{
    if(saveTimer)
        killTimer(saveTimer);
    saveTimer = 0;
}

void App::execAppCommand(const QString &cmd)
{
    if(cmd == "11")
    {
        onActionExit();
        return;
    }

    if(cmd == "22")
    {
        if(!hotkeysOn)
            registerControlHotKeys();
        return;
    }

    if(cmd == "2")
    {
        if(hotkeysOn)
            unregisterControlHotKeys();
        return;
    }

    if(cmd == "5")
    {
        onActionCloseFile();
        return;
    }

    if(cmd == "555")
    {
        moveCurFileToTrash();
        return;
    }

    if(cmd == "81")
    {
        setPlaybackMode(mse_ppmTrackOnce);
        return;
    }
    if(cmd == "82")
    {
        setPlaybackMode(mse_ppmAllOnce);
        return;
    }
    if(cmd == "83")
    {
        setPlaybackMode(mse_ppmTrackLoop);
        return;
    }
    if(cmd == "84")
    {
        setPlaybackMode(mse_ppmAllLoop);
        return;
    }
    if(cmd == "85")
    {
        setPlaybackMode(mse_ppmRandom);
        return;
    }

    if(cmd == "7")
    {
        onActionCurrentFile();
        return;
    }

    if(cmd == "77")
    {
        onActionPlaylists();
        return;
    }
}

void App::timerEvent(QTimerEvent *event)
{
    int timerId = event->timerId();

    if(timerId == prefixTimer)
    {
        killPrefixTimer();
        if(doSaveList)
            savePlaylist();
        else
        {
            if(appCommandInput)
            {
                execAppCommand(playlistPrefix.mid(2));
                appCommandInput = false;
                if(!hotkeysOn)
                    unregisterAppCommandHotKeys();
            }
            else
            {
                loadPlaylist();
            }
        }
        playlistPrefix.clear();
    }

#ifdef Q_OS_LINUX
    else if(timerId == keyHookTimer)
    {
        while(const xcb_generic_event_t* e = eventQueue->moveToNextEvent())
        {
            const xcb_key_press_event_t *curEvent = reinterpret_cast<const xcb_key_press_event_t*>(e);
            const xcb_key_press_event_t *nextEvent = reinterpret_cast<const xcb_key_press_event_t*>(eventQueue->peekNextEvent());

            bool isPressed;
            switch(curEvent->response_type & ~0x80)
            {
                case XCB_KEY_PRESS:
                    isPressed = true;
                    break;

                case XCB_KEY_RELEASE:
                    isPressed = false;
                    break;

                default:
                    return; // some other mysterious key event, should not happen
            }

            if(nextEvent)
            {
                if(
                   // check if this is not a real release event
                   !isPressed &&
                   ((nextEvent->response_type & ~0x80) == XCB_KEY_PRESS) &&
                   (curEvent->detail == nextEvent->detail) &&
                   (curEvent->state == nextEvent->state) &&
                   (curEvent->time == nextEvent->time)
                ){
                    // the key has not been actually released
                    // so skip this event
                    return;
                }
            }

            handleXKeyPress(curEvent, isPressed);
        }
    }
#endif

#ifdef Q_OS_OSX
    else if(timerId == keyRepeatTimer)
    {
        keyRepeatCounter++;
        if(keyRepeatCounter > KEY_REPEAT_WAIT_TICKS)
            onHotKey(keyRepeatKeyId, true);
    }
#endif

    else if(timerId == trayTimer)
    {
        tryCreateTray();
    }

    else if(timerId == saveTimer)
    {
        killSaveTimer();
        saveState();
    }
}

QString App::errorCodeToString(Err code)
{
    switch(code)
    {
        case Err::registerHotKey: return QStringLiteral("Cannot register hot key");
        case Err::unregisterHotKey: return QStringLiteral("Failed to unregister hot key");
        case Err::iconFileNotFound: return QStringLiteral("Application icon file not found");
        case Err::keyModifiersOverlap: return QStringLiteral("Key modifiers overlap");
        case Err::shiftModNotSupported: return QStringLiteral("SHIFT modifier is not supported");
        case Err::systemProxyDisabled: return QStringLiteral("System proxy is not supported in this version");
        case Err::invalidProxyUrl: return QStringLiteral("Invalid proxy URL");
        case Err::invalidProxyScheme: return QStringLiteral("Invalid proxy URL scheme");
        case Err::invalidProxyPort: return QStringLiteral("Invalid proxy port");
        case Err::trayInaccessible: return QStringLiteral("System tray is inaccessible");
        case Err::createDir: return QStringLiteral("Cannot create dir");
        default: return QString();
    }
}

void App::showNotificationOnUserTrackChange()
{
    if(tray && trayPopup && settings.notifications && settings.notificationsTrackChanged == ChangeTrackNotifications::interaction)
        updateTray();
}

QString App::getCmd(const QString &arg)
{
    if(arg.startsWith("--cmd-"))
        return arg.mid(6);
    else
        return QString();
}

void App::cmdTogglePlayStop()
{
    if(sound->getState() == mse_scsPlaying)
        cmdStop();
    else
        cmdPlay();
}

void App::cmdTogglePlayPause()
{
    if(sound->getState() == mse_scsPlaying)
        cmdPause();
    else
        cmdPlay();
}

void App::cmdPlay()
{
    if(sound->getState() == mse_scsPaused)
    {
        sound->unpause();
        mixer->unpause();
        startSaveTimer();
    }
    else
    {
        mixer->stop();
        if(!sound->isOpen())
        {
            sound->open();
            sound->play();
        }
        else
        {
            sound->play();
        }
        mixer->play();
        showNotificationOnUserTrackChange();
    }
}

void App::cmdStop(bool forceClose)
{
    if(settings.closeOnStop || forceClose)
        sound->close();
    else
        sound->stop();
    mixer->stop();
    startSaveTimer();
}

void App::cmdPause()
{
    sound->pause();
    mixer->pause();
    startSaveTimer();
}

void App::cmdNextTrack(bool autoPlay)
{
    bool doPlay = sound->getState() == mse_scsPlaying || autoPlay;
    if(doPlay)
    {
        sound->playNextValid();
        mixer->play();
    }
    else
    {
        mixer->stop();
        sound->openNextValid();
    }
    showNotificationOnUserTrackChange();
}

void App::cmdPrevTrack(bool autoPlay)
{
    bool doPlay = sound->getState() == mse_scsPlaying || autoPlay;
    if(doPlay)
    {
        sound->playPrevValid();
        mixer->play();
    }
    else
    {
        mixer->stop();
        sound->openPrevValid();
    }
    showNotificationOnUserTrackChange();
}

void App::cmdSeekForward()
{
    double secs = sound->getTrackDuration();
    if(secs >= 1)
        setSoundPosition(sound->getPosition() + qMax(secs / 100, 1.0));
}

void App::cmdSeekBack()
{
    double secs = sound->getTrackDuration();
    if(secs >= 1)
        setSoundPosition(sound->getPosition() - qMax(secs / 100, 1.0));
}

void App::cmdNextDir()
{
    sound->playFirstValidInNextDir();
    mixer->play();
    showNotificationOnUserTrackChange();
}

void App::cmdPrevDir()
{
    if(sound->getPlaylist()->isFirstInDir())
    {
        sound->playFirstValidInPrevDir();
    }
    else
    {
        int index = sound->getPlaylist()->getIndex();
        sound->openFirstValidInDir();
        if(sound->getPlaylist()->getIndex() == index)
            sound->playFirstValidInPrevDir();
        else
            sound->play();
    }
    mixer->play();
    showNotificationOnUserTrackChange();
}

void App::cmdVolUp()
{
    setSoundVolume(VOLUME_OWNER->getVolume() + volStepReal, true);
}

void App::cmdVolDown()
{
    setSoundVolume(VOLUME_OWNER->getVolume() - volStepReal, true);
}

void App::cmdMVolUp()
{
    player->changeMasterVolume(+mvolStepReal, true);
    if(settings.notificationsSystemVolume)
        showMasterVolumeInfo();
}

void App::cmdMVolDown()
{
    player->changeMasterVolume(-mvolStepReal, true);
    if(settings.notificationsSystemVolume)
        showMasterVolumeInfo();
}

QString App::secsToTimeFormat(int secs)
{
    if(secs < 0)
        return "-";

    int m = secs / 60;
    int s = secs - m * 60;
    int h = m / 60;
    m = m - h * 60;

    if(h)
    {
        return QString("%1:%2:%3")
                .arg(h)
                .arg(m, 2, 10, QChar('0'))
                .arg(s, 2, 10, QChar('0'));
    }
    else
    {
        return QString("%1:%2")
                .arg(m)
                .arg(s, 2, 10, QChar('0'));
    }
}

void App::setSoundPosition(double secs)
{
    if((sound->getType() == mse_sctUnknown) || (sound->getType() == mse_sctRemote))
        return;
    mixer->stop();
    sound->setPosition(secs);
    updateTray(settings.notificationsSeek, true);
    mixer->play();
}

void App::setSoundVolume(float val, bool snapToGrid)
{
    if(snapToGrid)
        val = player->snapVolumeToGrid(val, static_cast<float>(settings.volumeStep)/100.0);
    VOLUME_OWNER->setVolume(val);
#ifdef MSE_MODULE_MPRIS
    mpris->updateVolume();
#endif
    updateVolumeLabel();
    updateTray(settings.notificationsTrackVolume);
}

void App::updateTrayIcon()
{
    if(!tray)
        return;

    if(ignoreContState)
        return;

    QIcon* ico;

    switch(sound->getContinuousState())
    {
        case mse_scsPaused:
            ico = icoPause;
            break;

        case mse_scsPlaying:
            ico = icoPlay;
            break;

        default:
            ico = icoStop;
    }

    if(ico == curTrayIco)
        return;

    tray->setIcon(*ico);
}
