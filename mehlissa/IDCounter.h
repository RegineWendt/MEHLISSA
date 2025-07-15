/*
 * Copyright (c) 2024 Technische Universität Berlin [DEBUS]
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
 * Author: Lisa Y. Debus <debus@ccs-labs.org>
 */

#ifndef H_IDCOUNTER_
#define H_IDCOUNTER_

using namespace std;

namespace ns3 {
class IDCounter {
private:
    static unsigned int m_nextNanobotID;

public:
    static void InitIDCounter();

    // will return the next unique nanobot ID
    static unsigned int GetNextNanobotID();
};
}; // namespace ns3
#endif

