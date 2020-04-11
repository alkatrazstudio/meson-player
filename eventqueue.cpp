/****************************************************************************}
{ eventqueue.cpp - queue interface for Linux events                          }
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

#include "eventqueue.h"
#include <cstdlib>

EventQueue::EventQueue(xcb_connection_t* conn) :
    conn(conn),
    curEvent(nullptr),
    nextEvent(nullptr)
{
}

EventQueue::~EventQueue()
{
    std::free(curEvent);
    std::free(nextEvent);
}

const xcb_generic_event_t *EventQueue::moveToNextEvent()
{
    std::free(curEvent);
    curEvent = nextEvent;
    if(!curEvent)
        curEvent = xcb_poll_for_event(conn);
    if(curEvent)
        nextEvent = xcb_poll_for_event(conn);
    else
        nextEvent = nullptr;
    return curEvent;
}
