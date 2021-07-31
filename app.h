/****************************************************************************}
{ app.h - application core                                                   }
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

#pragma once

#include "coreapp.h"
#include "mse/types.h"
#include "mse/engine.h"
#include "mse/utils/lastfm.h"
#include "mse/utils/mixer.h"

#ifdef MSE_MODULE_MPRIS
    #include "mse/utils/mpris.h"
#endif

#include "notificationpopup.h"

#include <QActionGroup>
#include <QTextBrowser>

#ifdef Q_OS_WIN
    #include <windows.h>

    #ifndef VK_MEDIA_NEXT_TRACK
        #define VK_MEDIA_NEXT_TRACK 0xB0
    #endif
    #ifndef VK_MEDIA_PREV_TRACK
        #define VK_MEDIA_PREV_TRACK 0xB1
    #endif
    #ifndef VK_MEDIA_PLAY_PAUSE
        #define VK_MEDIA_PLAY_PAUSE 0xB3
    #endif
#endif

#ifdef Q_OS_LINUX
    #include <xcb/xcb.h>
    #include <xcb/xcb_keysyms.h>
    #include <X11/keysym.h>
    #include <X11/XF86keysym.h>
    #include "eventqueue.h"
#endif

#ifdef Q_OS_OSX
    #include <Carbon/Carbon.h>
#endif

enum class ChangeTrackNotifications {
    always,
    interaction,
    never
};

struct Settings {
    bool autoNumlock = true;
    bool subdirs = true;
    bool autoResume = false;
    quint8 volumeStep = 5;
    bool unlimitedPlaylists = true;
    bool balloons = true; // deprecated
    bool notifications = true;
    ChangeTrackNotifications notificationsTrackChanged = ChangeTrackNotifications::always;
    bool notificationsTrackVolume = true;
    bool notificationsSystemVolume = true;
    bool notificationsSeek = true;
    quint16 typeTimeout = 1000;
    bool shortCaptions = false;
    quint32 frequency = 44100;
    bool use8Bits = false;
    MSE_SoundSampleType sampleType = mse_sstFloat32;
    int nChannels = 2;
    int device = -1;
    bool useDefaultDevice = true;
    bool useSoftware = false;
    bool playlistsAutoload = false;
    MSE_SoundSampleInterpolation sampleInterpolation = mse_ssiSinc;
    MSE_SoundSampleRamping sampleRamping = mse_ssrSensitive;
    MSE_SoundSurroundMode surroundMode = mse_ssmNone;
    MSE_SoundTrackerEmulation trackerEmulation = mse_steNone;
    MSE_PlaylistFormatType playlistsType = mse_pftM3U;
    bool shoutcastPlaylists = true;
    bool logErrors = false;
    int minFolderLength = 5;
    int volume = -1;
    int index = -1;
    MSE_PlaylistPlaybackMode playbackMode = mse_ppmAllLoop;
    bool setPlaybackMode = false;
    bool closeOnStop = true;
    bool trayIcon = true;
    Qt::KeyboardModifiers modsCmd = Qt::NoModifier;
    Qt::KeyboardModifiers modsLoad = Qt::ControlModifier;
    Qt::KeyboardModifiers modsSave = Qt::AltModifier;
    Qt::KeyboardModifiers modsMM = Qt::NoModifier;
    Qt::KeyboardModifiers modsMM2 = Qt::ControlModifier;
    bool systemPlaylists = true;
    float mvolStep = 5;
    QString proxy = "system";
    bool loadOnly = false;
    int popupDuration = 5;
    int updatePeriod = 100;
    int bufferLength = 500;
    double position = -1;
    bool mpris = true;
    bool useICU = false;
    int icuMinConfidence = 0;
    bool icuUseForRemoteSources = false;
    bool showPlaylistTagsForRemoteSources = true;
};

struct PlaylistSource {
    MSE_SourceLoadFlags loadFlags;
    QString source;
};

struct HotKeyInfo {
    int id;
    int nKey;
    int nMod;
#ifdef Q_OS_OSX
    EventHotKeyRef ref;
#endif
    bool isRegistered;
    bool isDisabled;
};

typedef QList<HotKeyInfo> HotKeysInfo;

class App : public CoreApp
{
    Q_OBJECT

public:
    enum class Err {
        registerHotKey,
        unregisterHotKey,
        iconFileNotFound,
        keyModifiersOverlap,
        shiftModNotSupported,
        systemProxyDisabled,
        invalidProxyUrl,
        invalidProxyScheme,
        invalidProxyPort,
        trayInaccessible,
        createDir,
        createFile
    };
    Q_ENUM(Err)

protected:
    virtual int main();
    virtual void onQuit();

    void showBalloonsDeprecationWarning();
    ChangeTrackNotifications stringToTrackChangedNotificationsEnum(const QString &s);
    bool loadSettings();
    void loadConfig(const QString& filename);
    virtual QStringList getArgsForPassingToLocalServer() override;
    void parseCommandLine();
    void outputCmdResult(const QString &cmd, const QString &prefix = QString());
    virtual void onNewInstanceArgs();
    bool createSoundObject();
    bool createLfmObject();
    void tryCreateTray();
    bool createTray();
    void updateModeGroup(MSE_PlaylistPlaybackMode playbackMode);
    void updateLfmMenu();

    bool registerHotKey(HotKeyInfo& info);
    bool unregisterHotKey(HotKeyInfo& info);
    bool registerHotKeys(HotKeysInfo& infos);
    bool unregisterHotKeys(HotKeysInfo& infos);
    bool registerControlHotKeys();
    bool unregisterControlHotKeys();
    bool registerAppCommandHotKeys();
    bool unregisterAppCommandHotKeys();

    QString hotkeyToString(const HotKeyInfo &info);
    void updateTray(bool showNotification = true, bool showTime = false);
    void showTrayMessage(const QString& msg);
    void showMasterVolumeInfo();
    void updateVolumeLabel();
    void loadState();
    void setPlaybackMode(MSE_PlaylistPlaybackMode mode);
    void initSound();
    int getNativeKey(Qt::Key key);
    int getNativeKeyMod(Qt::KeyboardModifiers mod);
    void getHotKeyInfo(HotKeyInfo& info, int id, Qt::Key key, Qt::KeyboardModifiers mod = Qt::NoModifier);
    void addHotKeyInfo(HotKeysInfo& infos, int id, Qt::Key key, Qt::KeyboardModifiers mod = Qt::NoModifier);
    void addHotKeysInfo();
    void setNumLock();
    Qt::KeyboardModifiers stringToModifiers(const QString& str) const;
#ifdef Q_OS_LINUX
    int searchHotKey(const HotKeysInfo& infos, int nKey, uint16_t nMod);
#endif
    void correctPath(QString& s);

    QString getPlaylistFilename();
    void loadPlaylist();
    void savePlaylist();
    void startPrefixTimer();
    void killPrefixTimer();
    void startTrayTimer();
    void killTrayTimer();
    void startSaveTimer();
    void killSaveTimer();
    void execAppCommand(const QString& cmd);
    void timerEvent(QTimerEvent* event);
    void showNotificationOnUserTrackChange();

    bool createEmptyUserSettingsFileIfNeeded();
    void viewFile(const QString& filename);
    void showDir(const QString& dirname);
    void showFile(const QString& filename);

    static QString errorCodeToString(Err code);
    static QString errorDataToString(Err errorCode, const void *data);

    QString getCmd(const QString& arg);
    void cmdTogglePlayStop();
    void cmdTogglePlayPause();
    void cmdPlay();
    void cmdStop(bool forceClose = false);
    void cmdPause();
    void cmdNextTrack(bool autoPlay);
    void cmdPrevTrack(bool autoPlay);
    void cmdSeekForward();
    void cmdSeekBack();
    void cmdNextDir();
    void cmdPrevDir();
    void cmdVolUp();
    void cmdVolDown();
    void cmdMVolUp();
    void cmdMVolDown();

    QString secsToTimeFormat(int secs);
    void setSoundPosition(double secs);
    void setSoundVolume(float val, bool snapToGrid);

#ifdef MSE_MODULE_MPRIS
    MSE_Mpris* mpris;
    void mprisInit();
#endif

    HotKeysInfo hotKeysNum, hotKeysMedia, hotKeysAppMain, hotKeysApp;

    QIcon* curTrayIco;

    bool exitNow;
    QString settingsDir;
    Settings settings;
    QList<PlaylistSource> cmdPlaylist;
    QString appDir;
    QString globalSettingsFilename;
    QString userSettingsFilename;
    QSystemTrayIcon* tray;
    NotificationPopup* trayPopup;
    QMenu* trayMenu;
    QActionGroup* modeGroup;
    MSE_Engine* player;
    MSE_Sound* sound;
    MSE_Mixer* mixer;
    MSE_Playlist* playlist;
    MSE_Lastfm* lfm;
    bool showLfmLoginNotification;
    bool doPlay;
    QString lastFilename;
    QList<MSE_PlaylistEntry> statePlaylist;
    bool hasInputFiles;
    bool hotkeysOn;
    bool appCommandHotkeysOn;
    bool appCommandInput;
    float mvolStepReal;
    float volStepReal;
    bool ignoreContState;

    QIcon* icoApp;
    QIcon* icoPlay;
    QIcon* icoPause;
    QIcon* icoStop;

    QAction* actionShuffle;
    QAction* actionPlaylists;
    QAction* actionHotkeys;
    QAction* actionCurrentFile;
    QAction* actionCloseFile;
    QMenu* menuServiceLfm;
    QAction* actionLfmLoginWeb;
    QAction* actionLfmLoginForm;
    QAction* actionLfmUser;
    QAction* actionLfmLogout;
    QAction* actionExit;

    QList<QAction*> *modeActions;
    QAction *addModeAction(
            QMenu *menuModes,
            QActionGroup *group,
            const QString& title,
            MSE_PlaylistPlaybackMode mode);

    QString playlistPrefix;
    bool doSaveList;
    int prefixTimer;
    int trayTimer;
    int trayTimerTriesLeft;
    int saveTimer;
    QString playlistsExt;

    struct {
        QString formattedTitle;
        QString title;
        QString artist;
        QString filename;
        QString fullFilename;
        double duration;
        double fullDuration;
    } lastTrackData;

    void updateLastTrackData();
    void clearLastTrackData();

    bool hotkeysDisabled;
    QString curDir;
    QString curVol;

#ifdef Q_OS_LINUX
    xcb_connection_t* xconn;
    xcb_screen_t* xscreen;
    xcb_window_t xwin;
    xcb_key_symbols_t* xkeysyms;
    int keyHookTimer;
    EventQueue* eventQueue;
#endif
#ifdef Q_OS_OSX
    EventHandlerRef keyHooks;
    int keyRepeatTimer;
    int keyRepeatCounter;
    int keyRepeatKeyId;
#endif
#ifdef Q_OS_WIN
    HWND messageWin;
#endif

    QDialog* aboutDialog;
    QTextBrowser* aboutText;
public:
    explicit App(int &argc, char **argv);
    ~App();

    void migrateSettings();
    void copyDir(const QDir& src, const QDir& dst);
    void onHotKey(int id, bool isPressed);
#ifdef Q_OS_LINUX
    bool handleXKeyPress(const xcb_key_press_event_t *event, bool isPressed);
    bool xcb_ok(xcb_void_cookie_t cookie);
#endif
#ifdef Q_OS_OSX
    void startKeyRepeatTimer(int keyId);
    void stopKeyRepeatTimer();
#endif

    void loadCmdPlaylist();
    void moveCurFileToTrash();

public slots:
    void saveState();
    void saveStateList();
    void onLfmChangeState();

protected slots:
    void onInfoChange();
    void updateTrayIcon();

    void onActionPlaylists();
    void onActionHotkeys(bool checked);
    void onActionCurrentFile();
    void onActionCloseFile();
    void onActionExit();
    void onActionMode(bool checked);

    void onSaveData();

    virtual void onNativeEvent(void* event, bool& stopPropagation);
};
