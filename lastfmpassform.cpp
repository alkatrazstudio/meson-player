/****************************************************************************}
{ lastfmpassform.cpp - login form for Last.fm authenctication                }
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

#include "lastfmpassform.h"

static const int inputMaxLen = 100;
static LastfmPassForm* instance = nullptr;

LastfmPassForm::LastfmPassForm() : QDialog()
{
    setWindowTitle(tr("Log in to Last.fm"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::Tool | Qt::WindowStaysOnTopHint);

    new QVBoxLayout(this);

    inputUsername = new QLineEdit();
    inputUsername->setMaxLength(inputMaxLen);
    inputUsername->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    inputUsername->setPlaceholderText(tr("Your username"));
    layout()->addWidget(inputUsername);

    inputPassword = new QLineEdit();
    inputPassword->setMaxLength(inputMaxLen);
    inputPassword->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    inputPassword->setEchoMode(QLineEdit::Password);
    inputPassword->setPlaceholderText(tr("Your password"));
    layout()->addWidget(inputPassword);

    QPushButton* btnOk = new QPushButton();
    btnOk->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnOk->setText(tr("Log in"));
    connect(btnOk, &QPushButton::clicked, [this](){
        emit onLogin(inputUsername->text(), inputPassword->text());
    });
    layout()->addWidget(btnOk);

    setMinimumWidth(500);
    resize(minimumWidth(), layout()->contentsRect().height());
    setMaximumHeight(height());

    inputUsername->setFocus();
}

void LastfmPassForm::showForm(const std::function <void (const QString&, const QString&)>& callback)
{
    if(instance)
    {
        instance->raise();
        instance->activateWindow();
        instance->setFocus();
        return;
    }

    instance = new LastfmPassForm();
    instance->show();
    instance->activateWindow();
    instance->setFocus();
    connect(instance, &LastfmPassForm::onLogin,
        [callback](const QString& username, const QString& password){
            callback(username, password);
            instance->close();
        }
    );
    connect(instance, &LastfmPassForm::destroyed, [](){
        instance = nullptr;
    });
}
