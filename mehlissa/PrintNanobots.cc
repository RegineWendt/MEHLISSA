/* -*-  Mode: C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universität zu Lübeck [WENDT]
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
 * Author: Regine WENDT <regine.wendt@uni-luebeck.de>
 */

#include "PrintNanobots.h"

using namespace std;
namespace ns3 {

PrintNanobots::PrintNanobots (int particleMode)
{
  particlePrintMode = particleMode;
  output.open("csvNano.csv", ios::out | ios::trunc);
}

PrintNanobots::~PrintNanobots ()
{
  output.flush();
  output.close ();
}

void
PrintNanobots::PrintNanobot (Ptr<Nanobot> n, int vesselID)
{
  int id = n->GetNanobotID ();
  double x = n->GetPosition ().x;
  double y = n->GetPosition ().y;
  double z = n->GetPosition ().z;
  double m_start = Simulator::Now ().GetSeconds();
  int BvID = vesselID;
  int st = n->GetStream ();
  bool is_nc = n->GetDelay(); //is the nanobot a nanocollector?
  bool is_nl = n->HasFingerprintLoaded(); //is the nanobot a nanolocator?
  int target = n->GetTargetOrgan();
  bool detected = n->HasTissueDetected(); //is the nanobot an active nanocollector carrying a message? 
  bool is_np = n->GetDelay() > 1;
  int np_detected;
  if (is_np){
    np_detected = n->GotDetected();
  }
  else
    np_detected = -1;
  //Output if Particles are simulated and we only want to know if they are detected 
  if (particlePrintMode == 1){
    if (is_np){
    //output << id << "," << m_start << "," << BvID << "," << is_np << "," << np_detected << "\n";
      output << m_start << "," << np_detected << "\n";
    }
  }
  else if (particlePrintMode == 2){
    if (is_np){
    //output << id << "," << m_start << "," << BvID << "," << is_np << "," << np_detected << "\n";
      output  << id << "," << m_start << "," << np_detected << "\n";
    }
  }
  else{
  output << id << "," << x << "," << y << "," << z << "," << m_start << "," << BvID << "," << st << "," << is_nc << "," << is_nl << "," << target << "," << detected << "\n";
  }
  //Without coordinates
  //output << id << "," << m_start << "," << BvID << "," << is_nc << "\n";
}

void
PrintNanobots::PrintSomeNanobots (list<Ptr<Nanobot>> nbl, int vesselID)
{
  for (const Ptr<Nanobot> &bot : nbl)
    {
      PrintNanobot (bot, vesselID);
    }
}

void
PrintNanobots::PrintInTerminal (vector<Ptr<Bloodstream>> streamsOfVessel, int vesselIDl)
{
  cout.precision (3);
  cout << "VESSEL  ----------------" << vesselIDl << "--------" << endl;
  cout << "Time  ----------------" << Simulator::Now () << "--------" << endl;
  for (uint j = 0; j < streamsOfVessel.size (); j++)
    {
      cout << "Stream " << j + 1 << " ------------------------" << endl;
      for (uint i = 0; i < streamsOfVessel[j]->CountNanobots (); i++)
        {
          Ptr<Nanobot> n = streamsOfVessel[j]->GetNanobot (i);
          cout << n->GetNanobotID () << ":" << n->GetPosition ().x << ":" << n->GetPosition ().y
               << ":" << n->GetPosition ().z << ":" << streamsOfVessel[j]->GetVelocity () << endl;
        }
      cout << "----------------------" << endl;
    }
}

} // namespace ns3