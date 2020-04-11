/****************************************************************************}
{ lastfmpassform.h - login form for Last.fm authenctication                  }
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

#pragma once

#include <QtCore>
#include <QtWidgets>
#include <functional>

class LastfmPassForm : public QDialog
{
    Q_OBJECT

public:
    LastfmPassForm();
    static void showForm(const std::function<void (const QString&, const QString&)>& callback);

protected:
    QLineEdit* inputUsername;
    QLineEdit* inputPassword;

signals:
    void onLogin(const QString& username, const QString& password);
};
