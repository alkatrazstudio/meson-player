/****************************************************************************}
{ movetotrash.cpp - Trash/Recycle Bin partial interface                      }
{                                                                            }
{ Copyright (c) 2017 Alexey Parfenov <zxed@alkatrazstudio.net>               }
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

#include "movetotrash.h"
#include <QStandardPaths>
#include <QProcess>
#include <QFile>
#include <QDir>

#ifdef Q_OS_WIN
    #include <windows.h>
    #include <shobjidl.h>
    #include <shellapi.h>
#endif

#ifdef Q_OS_OSX
    #import <CoreFoundation/CoreFoundation.h>
    #include <QUrl>
#endif

namespace MoveToTrash {
    static bool checkIfCanDoIt()
    {
#ifdef Q_OS_WIN
        return true;
#endif

#ifdef Q_OS_OSX
        return true;
#endif

#ifdef Q_OS_LINUX
        QStringList apps = {
            "kioclient5",
            "kioclient",
            "gvfs-trash",
            "trash"
        };
        for(QString app : apps)
            if(!QStandardPaths::findExecutable(app).isEmpty())
                return true;
#endif
        return false;
    }

    bool canDoIt()
    {
        static enum {
            unknown,
            canDo,
            cannotDo
        } status = unknown;

        if(status == unknown)
            status = checkIfCanDoIt() ? canDo : cannotDo;

        return status == canDo;
    }

    bool doIt(const QString& path)
    {
        if(!canDoIt() || path.isEmpty())
            return false;

        QFileInfo info(path);
        if(!info.exists())
            return false;

        QFileInfo dirInfo = QFileInfo(info.dir().path());
        if(!dirInfo.isWritable())
            return false;

        QString filename = QDir::toNativeSeparators(info.absoluteFilePath());

#ifdef Q_OS_WIN
        // Windows is the only OS that has a reliable API to move files to the Trash.
        // However, look at how many steps it takes.
        // It also forces you to use single-threaded COM for the whole app.
        if(SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
        {
            bool ok = false;
            IFileOperation *pfo;
            if(SUCCEEDED(CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo))))
            {
                if(SUCCEEDED(pfo->SetOperationFlags(FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_WANTNUKEWARNING)))
                {
                    IShellItem *item = NULL;
                    std::wstring ws = filename.toStdWString();
                    if(SUCCEEDED(SHCreateItemFromParsingName(ws.c_str(), NULL, IID_PPV_ARGS(&item))))
                    {
                        if(SUCCEEDED(pfo->DeleteItem(item, NULL)))
                        {
                            if(SUCCEEDED(pfo->PerformOperations()))
                            {
                                if(!QFile::exists(path))
                                {
                                    ok = true;
                                }
                            }
                        }
                    }
                    item->Release();
                }
                pfo->Release();
            }
            CoUninitialize();
            return ok;
        }
#endif

#ifdef Q_OS_OSX
        // OSX has a lot of APIs to trash files.
        // Sadly, none of them support "Put Back" functionality.
        // So, use Finder to do this correctly.
        filename.replace("\\", "\\\\").replace("\"", "\\\"");
        QString cmd = QString("tell app \"Finder\" to move POSIX file \"")+filename+"\" to trash";
        QProcess::execute("osascript", {"-e", cmd});
        if(!QFile::exists(path))
            return true;
#endif

#ifdef Q_OS_LINUX
        // Linux doesn't have any Trash API at all.
        // Everything needs to be done manually using X.org specs.
        // But since no Linux distribution follows those specs to the letter,
        // there's no proper cross-Linux way to trash files.
        // So, let's pass this burden to external tools and hope for the best.
        QString app;

        app = QStandardPaths::findExecutable("kioclient5");
        if(!app.isEmpty())
        {
            QStringList args = {
                "--platform", "offscreen",
                "move", filename, "trash://"
            };
            QProcess::execute(app, args);
            if(!QFile::exists(path))
                return true;
        }

        app = QStandardPaths::findExecutable("kioclient");
        if(!app.isEmpty())
        {
            QStringList args = {
                "--noninteractive",
                "move", filename, "trash://"
            };
            QProcess::execute(app, args);
            if(!QFile::exists(path))
                return true;
        }

        app = QStandardPaths::findExecutable("gvfs-trash");
        if(!app.isEmpty())
        {
            QStringList args = {path};
            QProcess::execute(app, args);
            if(!QFile::exists(path))
                return true;
        }

        app = QStandardPaths::findExecutable("trash");
        if(!app.isEmpty())
        {
            QStringList args = {path};
            QProcess::execute(app, args);
            if(!QFile::exists(path))
                return true;
        }
#endif
        return false;
    }
}
