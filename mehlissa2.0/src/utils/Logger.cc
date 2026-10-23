/*
 * Copyright (c) 2026 Universität zu Lübeck [WENDT] and Technische Universität Berlin [DEBUS]
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
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

#include "Logger.h"

namespace utils {

Logger::Logger(int level) {
    initialized = true;
    logLevel = level;
}

Logger::~Logger() {}

void Logger::SetLogLevel(int level) {
    switch (level) {
    case 0:
        boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                            boost::log::trivial::error);
        break;
    case 1:
        boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                            boost::log::trivial::warning);
        break;
    case 2:
        boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                            boost::log::trivial::info);
        break;
    case 3:
        boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                            boost::log::trivial::debug);
        break;
    case 4:
        boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                            boost::log::trivial::trace);
        break;
    default:
        boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                            boost::log::trivial::info);
        break;
    }
}

void Logger::LogTrace(std::string message) { BOOST_LOG_TRIVIAL(trace) << message; }

void Logger::LogDebug(std::string message) { BOOST_LOG_TRIVIAL(debug) << message; }

void Logger::LogInfo(std::string message) { BOOST_LOG_TRIVIAL(info) << message; }

void Logger::LogWarning(std::string message) { BOOST_LOG_TRIVIAL(warning) << message; }

void Logger::LogError(std::string message) { BOOST_LOG_TRIVIAL(error) << message; }

} // namespace utils
