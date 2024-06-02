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
 * Author: Regine Wendt <regine.wendt@uni-luebeck.de>
 */

#ifndef CLASS_NANOPARTICLE_
#define CLASS_NANOPARTICLE_

#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "Nanobot.h"
#include <iostream>
namespace ns3 {
class Nanoparticle;
/**
* \brief Nanoparticle is a mobile Object.
*
* Each Nanoparticle is a small molecule or objekt with more speed than a nanobot, due to its smaller size. 
* To see in the csv output file if its a Nanoparticle or not, check the last column. False (0) is a 
* Nanobot and true (1) a Nanoparticle. 
*/

class Nanoparticle : public Nanobot
{
private:
  double m_delay; // factor for faster traveling speed. 
  int m_got_detected; //counts up when particle was detected by nanobot 
  double m_detectionRadius;

public:
  //static TypeId GetTypeId (void);

  /// Constructor to initialize values of all variables.
  Nanoparticle ();

  /// Destructor [does nothing].
  ~Nanoparticle ();

  /**
   * Getter method for the delay.
   * If the delay is greater than zero, it indicates that it is a Nanoparticle.
   *
   * @return The delay value.
   */
  double GetDelay ();

  /**
   * Setter method for the delay.
   * @param value The delay value to set.
   */
  void SetDelay (double value);

  /**
   * Getter method for detection status.
   * @return counts up if it was detected, false otherwise.
   */
  int GotDetected();

  /**
   * Setter method for detection status.
   * This method is used to signal a detection.
   */
  void GetsDetected();

  double GetDetectionRadius ();

  void SetDetectionRadius (double value);


};
}; // namespace ns3
#endif
