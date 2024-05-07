#include "EvManager.hpp"
#include <unistd.h>
#include <errno.h>
#include <string.h>

int EvManager::_numEvents = 0;
const int EvManager::CLIENT_LIMIT;

#ifdef __linux__
        fd_set          EvManager::_rfds;
        fd_set          EvManager::_wfds;
        fd_set          EvManager::_activeRfds;
        fd_set          EvManager::_activeWfds;
        int             EvManager::_nfds;
        std::set<int>         EvManager::_fdRSet;
        std::set<int>         EvManager::_fdWSet;
        std::set<int>         EvManager::_fdActiveSet;
        int EvManager::_curFd;
#else
        int EvManager::_kq = 0;
        std::map<int, std::time_t>   EvManager::_fdActiveSet;
        struct kevent EvManager::_evList[1000];
#endif


#ifdef __linux__

bool EvManager::start() {
    FD_ZERO(&_rfds);
    FD_ZERO(&_wfds);
    _nfds = 0;
    _curFd = 3;
    return (true);
}

bool EvManager::addEvent(int fd, Flag flag, socketType type) {
    (void)type;
    if (flag == read) {
        _fdRSet.insert(fd);
        FD_SET(fd, &_rfds);
        if (_nfds <= fd) {
            _nfds = fd + 1;
        }
    } else if (flag == write) {
        _fdWSet.insert(fd);
        FD_SET(fd, &_wfds);
        if (_nfds <= fd) {
            _nfds = fd + 1;
        }
    }

    return (true);
}

bool EvManager::delEvent(int fd, Flag flag) {
    if (flag == read) {
        FD_CLR(fd, &_rfds);
        _fdRSet.erase(fd);

    } else if (flag == write) {
        FD_CLR(fd, &_wfds);
        _fdWSet.erase(fd);

    }
    if (_fdWSet.empty() == false) {
        int a = *_fdWSet.rbegin() + 1;

        if (a > _nfds) {
            _nfds = a;
        }
    }

    if (_fdRSet.empty() == false) {
        int a = *_fdRSet.rbegin() + 1;

        if (a > _nfds) {
            _nfds = a;
        }
    }


    return (true);
}

std::pair<EvManager::Flag, int> EvManager::listen() {
    while (true) {
        if (_curFd == 3) {
            _activeRfds = _rfds;
            _activeWfds = _wfds;
            // std::cout << "_numEvents = " << _numEvents << std::endl;
            _numEvents = select(_nfds, &_activeRfds, &_activeWfds, NULL, NULL);
            // std::cout << "_numEvents = " << _numEvents << std::endl;
            if (_numEvents == -1) {
                throw std::runtime_error(std::string("kevent: ") + strerror(errno));
            }
        }
        while (_curFd < _nfds) {
            if (FD_ISSET(_curFd, &_activeRfds)) {
                return (std::pair<EvManager::Flag, int>(EvManager::read, _curFd++));
            } else if (FD_ISSET(_curFd, &_activeWfds)) {
                return (std::pair<EvManager::Flag, int>(EvManager::write, _curFd++));
            }
            _curFd++;
        }
        _curFd = 3;
    }
    return (std::pair<EvManager::Flag, int>(EvManager::def, -1));
}

#else


bool EvManager::start() {
    if (_kq == 0) {
        _kq = kqueue();
        if (_kq == -1) {
            throw std::runtime_error(std::string("kqueue: ") + strerror(errno));
        }
    }
    return (true);
}

bool EvManager::addEvent(int fd, Flag flag, socketType type) {
    if (_kq != 0) {
        int evFlag = getFlag(flag);
        struct kevent evSet;

        EV_SET(&evSet, fd, evFlag, EV_ADD, 0, 0, NULL);
        kevent(_kq, &evSet, 1, NULL, 0, NULL);
        if (type == client) {
            _fdActiveSet[fd] = std::time(NULL);
        }
        return (true);
    }
    return (false);
}

bool EvManager::delEvent(int fd, Flag flag) {
    if (_kq != 0) {
        int evFlag = getFlag(flag);
        struct kevent evSet;

        EV_SET(&evSet, fd, evFlag, EV_DELETE, 0, 0, NULL);
        kevent(_kq, &evSet, 1, NULL, 0, NULL);
        _fdActiveSet.erase(fd);
        return (true);
    }
    return (false);
}

#include <limits.h>

std::pair<EvManager::Flag, int> EvManager::listen() {
    struct timespec timeout;
    static std::vector<int>             _fdToRemove;

    timeout.tv_sec = KEEP_ALIVE_TIMEOUT;
    timeout.tv_nsec = 0;
    while (_numEvents == 0) {
        if (_fdToRemove.empty() == false) {
            int tmp = *--_fdToRemove.end();
            _fdToRemove.pop_back();
            return (std::pair<EvManager::Flag, int>(EvManager::eof, tmp));
        }
        _numEvents = kevent(_kq, NULL, 0, _evList, INT_MAX, &timeout);
        
        for (std::pair<std::map<int, std::time_t>::iterator, std::map<int, std::time_t>::iterator> it = std::make_pair(_fdActiveSet.begin(), _fdActiveSet.end());
                it.first != it.second; ++it.first) {
            if (it.first->second < std::time(NULL) - KEEP_ALIVE_TIMEOUT) {
                _fdToRemove.push_back(it.first->first);
            }
        }
    }

    if (_numEvents == -1) {
        throw std::runtime_error(std::string("kevent: ") + strerror(errno));
    }

    if (_evList[_numEvents - 1].flags == EV_ERROR) {
        throw std::runtime_error(std::string("event: ") + strerror(errno));
    }

    EvManager::Flag flag = error;

    if (_evList[_numEvents - 1].flags & EV_EOF) {
        flag = eof;
    } else if (_evList[_numEvents - 1].filter == EVFILT_READ) {
        flag = read;
    } else if (_evList[_numEvents - 1].filter == EVFILT_WRITE) {
        flag = write;
    }

    std::pair<EvManager::Flag, int> result(flag, _evList[_numEvents - 1].ident);
    std::map<int, std::time_t>::iterator it = _fdActiveSet.find(_evList[_numEvents - 1].ident);

    if (it != _fdActiveSet.end()) {
        it->second = time(NULL);
    }
    --_numEvents;
    return (result);
}


int EvManager::getFlag(Flag flag) {
    switch (flag)
    {
    case read :
        return (EVFILT_READ);
    case write :
        return (EVFILT_WRITE);
    case eof :
        return (EV_EOF);
    case error :
        return (EV_ERROR);
    default :
        return (def);
    }
    return (def);
};

#endif