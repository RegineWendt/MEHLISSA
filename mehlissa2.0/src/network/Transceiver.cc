/*
 * Copyright (c) 2025 Universität zu Lübeck [WENDT] and Technische Universität Berlin [DEBUS]
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

#include "Transceiver.h"

namespace network {
Transceiver::Transceiver(TransceiverType t_type, int t_id, Position t_position,
                         shared_ptr<BloodVessel> t_vessel,
                         std::vector<ParticleType> t_detectedParticles, double t_p_detection,
                         double t_p_absorption) {
    type = t_type;
    m_id = t_id;                               // transceiver's id
    m_position = Position(t_position);         // transceiver's position
    m_vessel = t_vessel;                       // transceiver's vessel
    m_detectedParticles = t_detectedParticles; // transceiver's detected particle types
    m_p_detection = t_p_detection / 100;       // transceiver's probability of detection
    m_p_absorption = t_p_absorption / 100;     // transceiver's probability of absorption
}

Transceiver::~Transceiver() {}

/**
 * \returns the transceiver Id.
 */
int Transceiver::GetTransceiverID() { return m_id; }

/**
 * \returns the position of the transceiver.
 */
Position Transceiver::GetPosition() { return m_position; }

/**
 * \returns the vessel of the transceiver.
 */
void Transceiver::SetVessel(shared_ptr<BloodVessel> vessel) { m_vessel = vessel; }

/**
 * \returns the vessel of the transceiver.
 */
shared_ptr<BloodVessel> Transceiver::GetVessel() { return m_vessel; }

/**
 * Runs the detection algorithm of the transceiver on the given nanobots.
 * \returns.An updated list of nanobots.
 */
vector<shared_ptr<Particle>> Transceiver::DetectParticles(vector<shared_ptr<Particle>> nanobots) {
    return {};
}

string Transceiver::GetGatewayOutput() { return ""; }
} // namespace network
