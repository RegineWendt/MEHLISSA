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

#ifndef CLASS_NANOLOCATOR_
#define CLASS_NANOLOCATOR_

#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "Nanobot.h"
#include <iostream>
namespace ns3 {
class Nanolocator;
/**
* \brief Nanolocator is a mobile Object.
*
* Each Nanolocator is a Nanobot with a specific tileset loaded, that is able to form a message when in the
* right organ and a disease marker is present. 
* To see in the csv output file if its a Nanolocator or not, check the column before last. (0) is a 
* Nanobot or locator and (1) is a Nanocollector. And in the last column a number higher than one indicates the organ the locator is able to detect. 
*/

class Nanolocator : public Nanobot
{
private:
  bool m_hasFingerprint; //has fingerprint loaded in the beginning, after release false
	int m_targetOrgan; //target organ of loaded fingerprint
  
public:
  //static TypeId GetTypeId (void);

  /// Constructor to initialize values of all variables.
  //Nanolocator (int target);
  /// Constructor to initialize values of all variables.
  Nanolocator ();

  /// Destructor [does nothing].
  ~Nanolocator ();

  ///Getter and setter methods

  /**
   * This function returns the target organ.
   *
   * @return The target organ.
   */
  int GetTargetOrgan();

  /**
   * This function sets the target organ.
   *
   * @param[in] target The target organ to be set.
   */
  void SetTargetOrgan(int target);

  /**
   * This function checks if a fingerprint is loaded.
   *
   * @return True if a fingerprint is loaded, false otherwise.
   */
  bool HasFingerprintLoaded();

  /**
   * This function releases fingerprint tiles.
   */
  void releaseFingerprintTiles();



};
}; // namespace ns3
#endif
