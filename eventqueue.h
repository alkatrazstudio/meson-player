/****************************************************************************}
{ eventqueue.h - queue interface for Linux events                            }
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

#include <xcb/xcb.h>

class EventQueue {
public:
    EventQueue(xcb_connection_t* conn);
    virtual ~EventQueue();

    const xcb_generic_event_t* moveToNextEvent();
    inline const xcb_generic_event_t* peekNextEvent() const {return nextEvent;}

protected:
    xcb_connection_t* conn;
    xcb_generic_event_t* curEvent;
    xcb_generic_event_t* nextEvent;
};
