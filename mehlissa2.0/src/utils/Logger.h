/*
 * Copyright (c) 2026 Universität zu Lübeck [WENDT] and Technische Universität
 * Berlin [DEBUS] This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Author: Regine Wendt <regine.wendt@uni-luebeck.de>
 * Author: Lisa Y. Debus <debus@ccs-labs.org>
 */

#ifndef CLASS_LOGGER_
#define CLASS_LOGGER_

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <string>

namespace utils {

class Logger {
private:
    bool initialized;
    int logLevel;

    Logger(int level);
    ~Logger();

public:
    static void SetLogLevel(int level = 2);

    static void LogInfo(std::string message);
    static void LogWarning(std::string message);
    static void LogError(std::string message);
    static void LogDebug(std::string message);
    static void LogTrace(std::string message);
};
}; // namespace utils
#endif
