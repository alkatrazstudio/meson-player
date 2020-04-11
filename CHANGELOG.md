# Meson Player: Changelog

## v0.8

* added: the Windows installer no longer requires admin privileges
* added: Linux distribution via AppImage
* fixed: losing current track information after the application is closed due to an OS restart
* fixed: mono tracks play sound from only one channel while the other channel transmits silence
* fixed: changing system volume on Linux ignores the current volume set outside of the application
* fixed: English translation is ignored if other languages with lower priority are also supported
* removed: Windows 32-bit support


## v0.7

* added: long press on "NumPad 4" to rewind, long press on "NumPad 6" to fast-forward
* added: ability to remove current file to Trash (might not be available on some Linux distributions)
* added: in-app login form for Last.fm authentication
* added: ability to show a current file in file manager on Linux
* added: MPRIS protocol support on Linux (i.e. you can now control the app via KDE Connect)
* added: OptimFROG support for Windows x86_64
* added: album title is now scrobbled to Last.fm too
* added: command line param --position
* added: settings options --mpris, --popup-duration, --buffer-length, --update-period
* added: commands --cmd-seek-forward, --cmd-seek-back
* changed: playback is now gapless
* changed: balloon notification messages were completely revamped
* changed: tray icons are now high resolution
* changed: Last.fm calls now use HTTPS
* changed: using --index parameter without filenames now always starts a playback (use --load-only to prevent)
* fixed: crash on Linux when passing params to running app
* fixed: changing sound volume on Linux mutes all sound
* fixed: crash when using system proxy on Windows x86_64
* fixed: tags from the last track in CUE-sheet may not be correctly parsed
* fixed: on Linux, playlists folder is opened via smb:// protocol instead of local filesystem
* fixed: command line options --volume and --index do not work


## v0.6

* added: application manual (can be accessed via manual/index.html or via tray context menu Help > Manual)
* added: support for various ZX Spectrum AY/YM modules, Amiga/Atari/PC tracker modules, SID modules and some other chiptune/tracker formats
* added: OptimFROG support (Windows i686 only)
* added: options "playback-mode", "load-only" (see "Setup parameters" in the manual)
* added: --cmd-get-* command line commands (see "Control commands" in the manual)
* changed: "Shuffle" tray menu option is replaced with "Mode" submenu which lets you choose playback mode
* changed: tray icon now represents application state (playing, paused, stopped)
* changed: the app doesn't need Growl anymore to show baloon tooltips on OSX
* changed: "auto-resume" option is now false by default
* changed: system playlists 8 and 88 are replaced with 81, 82, 83, 84, 85 (see "System playlists" in the manual)
* changed: the sound volume level is now tied to a grid based on volume step
* changed: option "app-commands" was renamed to "system-playlists"
* changed: non-boolean options with wrong values are now ignored
* fixed: when "auto-resume" option is set to true, music will autoplay after system restart even if the player was paused
* fixed: can't open a current file location if the player is stopped
* fixed: can't parse *.cue file if it doesn't end with newline character
* fixed: app may crash if a file read/write error occurs
* fixed: the tray hint sometimes breaks fullscreen apps when a cursor is hovered above tray icon
* fixed: multiple issues with Internet-radio playback
* fixed: "minFolderLength" option affects "frequency" option
* removed: Windows XP, Windows Vista, OSX 10.8 support
* removed: support for multimedia keys for OSX
* removed: options "index", "volume", "shuffle", "keys-mm", "keys-num"
* removed: settings.ini.template (see "Setup parameters" in the manual)


## v0.5

* added: Last.fm scrobbling
* added: SOCKS5 and HTTP proxy support (settings option name: proxy)
* added: playlists can now be opened via file context menu just as folders
* added: new commands: --cmd-pause, --cmd-toggle-pause
* added: new option "OFF" for mods-* parameters (to completely disable a certain set of hotkeys)
* changed: the app now uses system proxy settings by default (use settings option "proxy" to configure a proxy)
* changed: a balloon tooltip is now not displayed after the app starts unless the track is played
* changed: when the app starts and the current track is not playing, this track will be closed
* changed: hotkeys initialization errors no longer cause the app to fail to start
* fixed: M3U playlists in local 8-bit encoding are not properly parsed
* fixed: new playlist is not saved as a current playlist right after opening
* fixed: track title in tooltips is lost after changing a volume when the player is stopped
* fixed: autoload-playlists option ignored, playlists are always loaded when opening a folder
* fixed: tracker-emulation & tray-icon command line options are broken
* removed: Icecast radio streams support
* note for devs: building the app is now only possible with C++11 compliant compiler


## v0.4

* added: use "NumPad 1" and "NumPad 3" to control the system volume (there's also a corresponding parameter mvol-step)
* added: app commands (see "App commands" section in README)
* added: new option tray-icon (to hide a tray icon)
* added: you now can customize key modifiers (see "Customizing keyboard modifiers" section in README)
* added: new popup menu item "Close the File" to stop and close a current file (only visible if close-on-stop=false)
* added: russian localization
* added: x64 version for Windows
* removed: shift-mm-keys and shift-num-keys
* removed: [*] from all tips (random playback indicator)
* removed: auto-numlock option for Linux
* changed: the file is now closed if you stop a playback (also, there's a new option: close-on-stop)
* fixed: the app crashes when an initialization fails
* fixed: tooltip text is truncated on Windows
* fixed: "&" symbols are truncated from a tooltip text on Windows
* note: the app has moved to Qt5
* note for devs: building the app with MinGW32 is not possible now, use MinGW64 to build Meson Player on Windows


## v0.3

* added: press "NumPad 0" to pause/resume current track
* added: installer for OSX
* changed: if a current track is not the first one in its folder, then "NumPad 7" will select the first playable file in a current folder
* changed: shift-num-keys is disabled on Windows
* changed: windows installer won't create applications icons
* changed: "playlists" folder is created when context menu item "Playlists" clicked
* fixed: unable to open a whole local disk on Windows
* fixed: invalid urls cause track skipping
* fixed: shift-mm-keys param doesn't affect multimedia keys


## v0.2

* added: support of CUE sheets
* added: sound samples are now 32-bit float by default (parameter: sample-type)
* added: if folder name is shorter than X symbols, then prepend subfolder name to it (parameter: min-folder-length)
* added: in command line parameters, "true" and "false" are now translated into "1" and "0" respectively
* added: new parameters: index, shuffle, volume
* added: new commands: --cmd-stop, --cmd-play, --cmd-toggle-play, --cmd-next, --cmd-prev, --cmd-next-dir, --cmd-prev-dir, --cmd-quit, --cmd-vol-up, --cmd-vol-down
* changed: new shuffling algorithm
* changed: playback will not be resumed, if a user exits the app manually
* removed: command --quit
* fixed: cannot change a volume if music isn't playing
* fixed: random track order is always the same
* fixed: "--quit" parameter is not working
* fixed: folder name is truncated by last dot
* fixed: on Linux, the output device frequency is not changing when specifying "frequency" parameter
* fixed: natural sorting algorithm is not working properly when filenames start from digit
* fixed: wrong command line parameters are interpreted as input filenames
* fixed: sometimes, after the app starts, playback is resumed not from a last played file
* fixed: sometimes, after the app starts, playback resumes when it shouldn't
* fixed: the entry in "Programs and Features" on Windows has no application icon


## v0.1

* initial release
