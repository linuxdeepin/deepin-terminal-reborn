/*
    This file is part of Konsole, KDE's terminal.

    Copyright 2007-2008 by Robert Knight <robertknight@gmail.com>
    Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "SessionManager.h"

// Qt
#include <QStringList>
#include <QTextCodec>
#include <QDebug>

// Konsole
#include "Session.h"

using namespace Konsole;

Q_GLOBAL_STATIC(SessionManager, theSessionManager)
SessionManager *SessionManager::instance()
{
    return theSessionManager;
}

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
}

//void SessionManager::closeAllSessions()
//{
//    // close remaining sessions
//    for (int i = 0; i < _sessions.size(); i++) {
//        Session *session = _sessions.at(i);
//        session->close();
//    }
//    _sessions.clear();
//}

QList<Session *> SessionManager::sessions() const
{
    return _sessions;
}

void SessionManager::sessionTerminated(Session *session)
{
    Q_ASSERT(session);

    _sessions.removeAll(session);

    session->deleteLater();
}

void SessionManager::saveSession(Session *session)
{
    _sessions << session;
}

bool SessionManager::removeSession(int id)
{
    if (_sessions.count() == 0) {
        return false;
    }

    int removeIndex = -1;
    for (int i = 0; i < _sessions.size(); i++) {
        Session *session = _sessions.at(i);
        if (session->sessionId() == id) {
            removeIndex = i;
            break;
        }
    }

    if (removeIndex >= 0) {
        _sessions.removeAt(removeIndex);
        return true;
    }

    return false;
}

int SessionManager::getSessionId(Session *session)
{
    return session->sessionId();
}

Session *SessionManager::idToSession(int id)
{
    for (int i = 0; i < _sessions.size(); i++) {
        Session *session = _sessions.at(i);
        if (session->sessionId() == id) {
            return session;
        }
    }
    // this should not happen
    qDebug() << "Failed to find session for ID" << id;
    return nullptr;
}

void SessionManager::saveCurrShellPrompt(int sessionId, QString strPrompt)
{
    strPrompt = strPrompt.trimmed();
    _shellPromptSessionMap.insert(sessionId, strPrompt);
}

QString SessionManager::getCurrShellPrompt(int sessionId)
{
    return _shellPromptSessionMap.value(sessionId);
}

void SessionManager::saveCurrShellCommand(int sessionId, QString strCommand)
{
    strCommand = strCommand.trimmed();
    _shellCommandSessionMap.insert(sessionId, strCommand);
}

QString SessionManager::getCurrShellCommand(int sessionId)
{
    return _shellCommandSessionMap.value(sessionId);
}

void SessionManager::setTerminalResizing(int sessionId, bool bTerminalResizing)
{
    _terminalResizeStateMap.insert(sessionId, bTerminalResizing);
}

bool SessionManager::isTerminalResizing(int sessionId)
{
    return _terminalResizeStateMap.value(sessionId);
}
