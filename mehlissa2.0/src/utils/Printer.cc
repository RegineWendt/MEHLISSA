/*
 * Copyright (c) 2025 Universität zu Lübeck [WENDT] and Technische Universität
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

#include "Printer.h"

using namespace std;
namespace utils {

Printer::Printer(int particleMode, int printInterval) {
    particlePrintMode = particleMode;
    output.open("csvNano.csv", ios::out | ios::trunc);
    interval = printInterval;
}

Printer::Printer(string simFile, string gwFile, int particleMode,
                 int printInterval) {
    particlePrintMode = particleMode;
    if (!simFile.empty())
        output.open(simFile, ios::out | ios::trunc);
    if (!gwFile.empty())
        gwOutput.open(gwFile, ios::out | ios::trunc);
    interval = printInterval;
}

Printer::~Printer() {
    if (output.is_open()) {
        output.flush();
        output.close();
    }
    if (gwOutput.is_open()) {
        gwOutput.flush();
        gwOutput.close();
    }
}

void Printer::CloseFile() {
    if (output.is_open()) {
        output.flush();
        output.close();
    }
    if (gwOutput.is_open()) {
        gwOutput.flush();
        gwOutput.close();
    }
}

void Printer::PrintGateway(int vesselID, string outputString) {
    if (gwOutput.is_open()) {
        double m_start = GlobalTimer::NowInSeconds();
        if (int(m_start) % interval != 0)
            return;
        gwOutput << vesselID << "," << m_start << "," << outputString << "\n";
    }
}

void Printer::PrintParticle(shared_ptr<Particle> n, int vesselID) {
    if (output.is_open()) {
        int id = n->GetParticleID();
        double x = n->GetPosition().x;
        double y = n->GetPosition().y;
        double z = n->GetPosition().z;
        double m_start = GlobalTimer::NowInSeconds();
        if (int(m_start) % interval != 0)
            return;
        int BvID = vesselID;
        int st = n->GetStream();
        bool is_nc = n->GetDelay(); // is the nanobot a nanocollector?
        bool is_nl = n->HasFingerprintLoaded(); // is the nanobot a nanolocator?
        int target = n->GetTargetOrgan();
        bool detected =
            n->HasTissueDetected(); // is the nanobot an active
                                    // nanocollector carrying a message?
        bool is_np = n->GetDelay() > 1;
        int type = n->particleType;
        // extend type desciptor for different particle types
        if (type == ParticleType::HormoneType)
            type = type * 100 + n->GetSubType();

        int np_detected;
        if (is_np)
            np_detected = n->GotDetected();
        else
            np_detected = -1;

        switch (particlePrintMode) {
        case 1: // print only detected
            if (is_np)
                output << m_start << "," << np_detected << "\n";
            break;
        case 2: // rint only id and detected
            if (is_np)
                output << id << "," << m_start << "," << np_detected << "\n";
            break;
        default:
            output << id << "," << x << "," << y << "," << z << "," << m_start
                   << "," << BvID << "," << st << "," << is_nc << "," << is_nl
                   << "," << target << "," << detected << "," << type << "\n";
            break;
        }
    }
}

void Printer::PrintVessel(list<shared_ptr<Particle>> nbl, int vesselID) {
    if (output.is_open()) {
        double m_start = GlobalTimer::NowInSeconds();
        if (int(m_start) % interval != 0)
            return;

        // count for each existing partciel type
        list<shared_ptr<Particle>> ps(nbl);
        std::map<int, int> counting;
        while (ps.size() > 0) {
            shared_ptr<Particle> p = ps.front();
            int type = p->particleType;
            int count = 1;
            // extend type desciptor for different particle types
            if (type == ParticleType::HormoneType)
                type = type * 100 + p->GetSubType();
            int pre = ps.size();
            auto it_ps = std::remove_if(
                ps.begin(), ps.end(), [type](shared_ptr<Particle> value) {
                    return (type == value->particleType ||
                            type == value->particleType * 100 +
                                        value->GetSubType());
                });
            ps.erase(it_ps, ps.end());
            counting[type] = pre - ps.size();

            /**
            shared_ptr<Particle> p = ps.front();
            int type = p->particleType;
            int count = 1;
            // extend type desciptor for different particle types
            if (type == ParticleType::HormoneType)
                type = type * 100 + p->GetSubType();
            else if (type == ParticleType::SwitchableParticleType)
                type = type * 100 + p->GetState();
            p->pop_front();
            list<shared_ptr<Particle>> intermediate(ps);
            for (const shared_ptr<Particle> n : ps) {
                if (type == n->particleType || type == n->particleType * 100 +
            n->GetSubType()){ count++; intermediate.remove(n);
                }
            }
            ps.erase();
            ps.merge(intermediate);
            dounting[type] = count;
            **/
        }
        map<int, int>::iterator it;
        for (it = counting.begin(); it != counting.end(); ++it)
            output << vesselID << "," << m_start << "," << it->first << ","
                   << it->second << '\n';
    }
}

void Printer::PrintParticles(list<shared_ptr<Particle>> nbl, int vesselID) {
    if (particlePrintMode == 3) {
        PrintVessel(nbl, vesselID);
    } else {
        for (const shared_ptr<Particle> bot : nbl) {
            PrintParticle(bot, vesselID);
        }
    }
}

void Printer::PrintInTerminal(vector<shared_ptr<Bloodstream>> streamsOfVessel,
                              int vesselIDl) {
    // cout.precision(3);
    Logger::LogInfo("VESSEL  ----------------" + std::to_string(vesselIDl) +
                    "--------");
    Logger::LogInfo("Time  ----------------" +
                    std::to_string(GlobalTimer::NowInSeconds()) +
                    " s --------");
    for (uint j = 0; j < streamsOfVessel.size(); j++) {
        Logger::LogInfo("Stream " + std::to_string(j + 1) +
                        " ------------------------");
        for (uint i = 0; i < streamsOfVessel[j]->CountParticles(); i++) {
            shared_ptr<Particle> n = streamsOfVessel[j]->GetParticle(i);
            Logger::LogInfo(std::to_string(n->GetParticleID()) + ":" +
                            std::to_string(n->GetPosition().x) + ":" +
                            std::to_string(n->GetPosition().y) + ":" +
                            std::to_string(n->GetPosition().z) + ":" +
                            std::to_string(streamsOfVessel[j]->GetVelocity()) +
                            "");
        }
        Logger::LogInfo("----------------------");
    }
}
} // namespace utils
