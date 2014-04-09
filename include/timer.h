/*
 * Copyright (C) 2013 Emmanuel Durand
 *
 * This file is part of Log.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * blobserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with blobserver.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @timer.h
 * The Timer class
 */

#ifndef SPLASH_TIMER_H
#define SPLASH_TIMER_H

#include <chrono>
#include <ctime>
#include <iostream>
#include <map>
#include <mutex>
#include <string>

#include "config.h"

namespace Splash
{

class Timer
{
    public:
        Timer() {}
        ~Timer() {}

        /**
         * Start / end a timer
         */
        void start(std::string name)
        {
            if (!_enabled)
                return;
            _mutex.lock();
            _timeMap[name] = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            _mutex.unlock();
        }

        void stop(std::string name)
        {
            if (!_enabled)
                return;
            _mutex.lock();
            if (_timeMap.find(name) != _timeMap.end())
            {
                auto now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                _durationMap[name] = now - _timeMap[name];
            }
            _mutex.unlock();
        }

        /**
         * Wait for the specified timer to reach a certain value, in us
         */
         void waitUntilDuration(std::string name, unsigned long long duration)
         {
            if (!_enabled)
                return;

            if (_timeMap.find(name) == _timeMap.end())
                return;

            unsigned long long now = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            unsigned long long elapsed;
            {
                _mutex.lock();
                elapsed = now - _timeMap[name];
                _timeMap.erase(name);
                _mutex.unlock();
            }

            timespec nap;
            nap.tv_sec = 0;
            if (elapsed < duration)
                nap.tv_nsec = (duration - elapsed) * 1e3;
            else
                nap.tv_nsec = 0;

            {
                _mutex.lock();
                _durationMap[name] = std::max(duration, elapsed);
                _mutex.unlock();
            }
            nanosleep(&nap, NULL);
         }

         /**
          * Get the last occurence of the specified duration
          */
         unsigned long long getDuration(std::string name)
         {
            if (_durationMap.find(name) == _durationMap.end())
                return 0;
            unsigned long long duration;
            {
                _mutex.lock();
                duration = _durationMap[name];
                _mutex.unlock();
            }
            return duration;
         }

         /**
          * Get the whole time map
          */
         std::map<std::string, unsigned long long> getDurationMap()
         {
            _mutex.lock();
            auto durationMap = _durationMap;
            _mutex.unlock();
            return durationMap;
         }

         /**
          * Set an element in the duration map. Used for transmitting timings between pairs
          */
         void setDuration(std::string name, unsigned long long value)
         {
            _mutex.lock();
            _durationMap[name] = value;
            _mutex.unlock();
         }

         /**
          * Some facilities
          */
         Timer& operator<<(std::string name)
         {
            start(name);
            _currentDuration = 0;
            return *this;
         }

         Timer& operator>>(unsigned long long duration)
         {
            _currentDuration = duration;
            return *this;
         }

         Timer& operator>>(std::string name)
         {
            if (_currentDuration > 0)
                waitUntilDuration(name, _currentDuration);
            else
                stop(name);
            return *this;
         }

         unsigned long long operator[](std::string name) {return getDuration(name);}

         /**
          * Enable / disable the timers
          */
         void setStatus(bool enabled) {_enabled = enabled;}

    private:
        std::map<std::string, unsigned long long> _timeMap; 
        std::map<std::string, unsigned long long> _durationMap;
        unsigned long long _currentDuration;
        std::mutex _mutex;
        bool _enabled {true};
};

struct STimer
{
    public:
        static Timer timer;
};

} // end of namespace

#endif // SPLASH_TIMER_H
