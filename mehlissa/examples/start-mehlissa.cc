/* -*-  Mode: C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Universität zu Lübeck [GEYER]
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
 */

#include "ns3/Bloodcircuit.h"

using namespace ns3;
using namespace std;

int
main (int argc, char *argv[])
{
  //Add default values:
  int numOfNanobots = 100;
  int simulationDuration = 10;
  int injectionVessel = 1;
  int numOfCollectors = 0;
  int numOfLocators = 0;
  //Particle Mode - Set mode to one if you want to simulate ldl particles, 
  //and to two if you want so simulate liquid biopsy. 
  //See Bloodcircuit.cc Line 197ff for the setup of both scenarios
  int particleMode = 0;
  bool isDeterministic = true;

  CommandLine cmd;
  cmd.AddValue ("simulationDuration", "simulationDuration", simulationDuration);
  cmd.AddValue ("numOfNanobots", "numOfNanobots", numOfNanobots);
  cmd.AddValue ("injectionVessel", "injectionVessel", injectionVessel);
  cmd.AddValue ("numOfCollectors", "numOfCollectors", numOfCollectors);
  cmd.AddValue ("numOfLocators", "numOfLocators", numOfLocators);
  cmd.AddValue ("particleMode", "particleMode", particleMode);
  cmd.AddValue ("isDeterministic", "isDeterministic", isDeterministic);

  cmd.Parse (argc, argv);
  return Bloodcircuit::BeginSimulation (simulationDuration, 
                                        numOfNanobots, 
                                        injectionVessel, 
                                        numOfCollectors, 
                                        numOfLocators, 
                                        particleMode, 
                                        isDeterministic);
}
