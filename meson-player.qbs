/****************************************************************************}
{ meson-player.qbs - main project file                                       }
{                                                                            }
{ This file is a part of the project                                         }
{   Meson Player - Music Player with no GUI                                  }
{                                                                            }
{ Copyright (c) 2012 Alkatraz Studio                                         }
{                                                                            }
{ author: Alexey Parfenov a.k.a. ZXED                                        }
{ author's e-mail: zxed@alkatrazstudio.net                                   }
{                                                                            }
{ Meson Player is free software: you can redistribute it and/or              }
{ modify it under the terms of the GNU General Public License                }
{ as published by the Free Software Foundation, either version 3 of          }
{ the License, or (at your option) any later version.                        }
{                                                                            }
{ Meson Player is distributed in the hope that it will be useful,            }
{ but WITHOUT ANY WARRANTY; without even the implied warranty of             }
{ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU           }
{ General Public License for more details.                                   }
{                                                                            }
{ You may read the GNU General Public License at:                            }
{   https://www.gnu.org/copyleft/gpl.html                                    }
{                                                                            }
{ project started: July 22, 2012                                             }
{****************************************************************************/

import qbs.TextFile

Project {
    minimumQbsVersion: '1.13.0'

    property bool ctrlProfile: false

    qbsSearchPaths: [
        './common',
        './meson-sound-engine'
    ]

    QtGuiApplication {
        name: 'mesonplayer'
        version: Version.autoVersion

        Depends {
            name: 'Qt'
            submodules: ['core', 'gui', 'widgets', 'network']
        }

        Depends {name: 'Version'}
        Depends {name: 'Common'}
        Depends {name: 'CoreApp'}
        Depends {name: 'ErrorManager'}
        Depends {name: 'QIODeviceHelper'}
        Depends {name: 'NotificationPopup'}
        Depends {name: 'SimpleCrypt'}

        Depends {name: 'MesonSoundEngine'}

        Depends {
            condition: Common.isLinux
            name: 'xcb'
        }
        Depends {
            condition: Common.isLinux
            name: 'xcb-keysyms'
        }

        CoreApp.singleInstanceKey: name
        CoreApp.dummyWindow: true
        Common.appTitle: 'Meson Player'
        qbs.installPrefix: ''
        consoleApplication: ctrlProfile
        targetName: name + (ctrlProfile ? "ctrl" : "")

        MesonSoundEngine.sourceUrl: true
        MesonSoundEngine.lastfm: true
        MesonSoundEngine.mpris: true
        MesonSoundEngine.mixer: true
        MesonSoundEngine.icu: true

        Group {
            name: 'App'
            files: {
                var files = [
                    'main.cpp',
                    'app.h',
                    'app.cpp',
                    'lastfmpassform.h',
                    'lastfmpassform.cpp',
                    'movetotrash.h',
                    'movetotrash.cpp'
                ]
                if(Common.isLinux)
                {
                    files.push('eventqueue.h')
                    files.push('eventqueue.cpp')
                }
                return files
            }
        }

        cpp.defines: {
            var defs = []
            if(lfmDecryptKey)
                defs.push('LFM_DECRYPT_KEY=0x' + lfmDecryptKey)
            return defs
        }

        cpp.dynamicLibraries: {
            var libs = []
            if(Common.isWindows)
                libs.push('uuid')
            return libs
        }

        cpp.libraryPaths: Common.libraryPaths

        Properties {
            condition: Common.isOSX

            cpp.frameworks: outer.concat([
                'Carbon'
            ])
        }

        Group {
            name: 'Assets'
            condition: !project.ctrlProfile
            qbs.install: Common.realInstall
            qbs.installSourceBase: ''
            files: [
                'icons/*.png',
                'icons/*.ico'
            ]
        }

        Group {
            name: 'Manual'
            condition: !project.ctrlProfile
            qbs.install: Common.realInstall
            qbs.installDir: 'manual'
            files: [
                'manual/dist/*.html',
                'manual/dist/*.css',
                'manual/dist/*.svg',
                'manual/dist/*.png'
            ]
        }

        Group {
            name: 'Plugins'
            condition: !project.ctrlProfile
            qbs.install: Common.realInstall
            qbs.installSourceBase: ''
            files: [
                'plugins/*.' + Common.libExt
            ]
        }

        readonly property string lfmDecryptKey: lfmDecryptKeyProbe.lfmDecryptKey
        Probe {
            id: lfmDecryptKeyProbe
            property string lfmDecryptKey
            configure: {
                try{
                    var f = TextFile(path+'/services/lastfm/key')
                    lfmDecryptKey = f.readLine()
                    found = true
                }catch(e){
                    console.warn(e)
                    lfmDecryptKey = ''
                    found = false
                }
            }
        }

        Rule {
            multiplex: true
            Artifact {
                filePath: product.sourceDirectory+'/data.qrc'
                fileTags: 'qrc'
            }
            prepare: {
                var cmd = new JavaScriptCommand()
                cmd.description = 'generating ' + output.fileName
                cmd.sourceCode = function() {
                    var f = new TextFile(output.filePath, TextFile.WriteOnly)
                    f.writeLine('<RCC>')
                    f.writeLine('    <qresource prefix="/">')
                    if(product.lfmDecryptKey)
                        f.writeLine('        <file>services/lastfm/data</file>')
                    f.writeLine('    </qresource>')
                    f.writeLine('</RCC>')
                    f.close()
                }
                return cmd
            }
        }
    }
}
