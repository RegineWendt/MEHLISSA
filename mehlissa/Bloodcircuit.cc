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

#include "Bloodcircuit.h"
#include <fstream>

/**
 * This function sets the Bloodvessel map of the human body of a female: hight 1.72m, weight 69kg, age 29.
 */

namespace ns3 {

Bloodcircuit::Bloodcircuit (unsigned int numberOfNanobots, unsigned int numberOfCollectors, unsigned int numberOfLocators, unsigned int injectionVessel, Ptr<PrintNanobots> printer, unsigned int particleMode)
{
  // initialise map with bloodvesselinformation
  m_bloodvessels = map<int, Ptr<Bloodvessel>> ();
  this->printer = printer;
  // loading vasculature via csv
  std::ifstream infile{"vasculature_transitions95.csv"};
  if (infile.good ())
    {
      std::vector<int> numbers;
      numbers.resize (valuesPerLine);
      std::string buffer;
      buffer.reserve (64);
      int errorflag = 0;
      while (infile.good ())
        {
          for (auto &&elem : numbers)
            {
              if (std::getline (infile, buffer, ','))
                {
                  elem = std::stoi (buffer);
                }
              else
                {
                  elem = 0;
                  errorflag = 1;
                }
            }
          if (errorflag == 0)
            {
              AddVesselData (numbers[0], (BloodvesselType) numbers[1],
                             Vector (numbers[2], numbers[3], numbers[4]),
                             Vector (numbers[5], numbers[6], numbers[7]));
            }
            //male
            // if (errorflag == 0)
            // {
            //   AddVesselData (numbers[0], (BloodvesselType) numbers[1],
            //                  Vector (round(numbers[2]*1.04), round(numbers[3]*1.04), numbers[4]),
            //                  Vector (round(numbers[5]*1.04), round(numbers[6]*1.04), numbers[7]));
            // }
            //female
            // if (errorflag == 0)
            // {
            //   AddVesselData (numbers[0], (BloodvesselType) numbers[1],
            //                  Vector (round(numbers[2]*0.96), round(numbers[3]*0.96), numbers[4]),
            //                  Vector (round(numbers[5]*0.96), round(numbers[6]*0.96), numbers[7]));
            // }
        }
      cout << "load CSV - transitions" << endl;
    }
  else
    { // Old way of loading data
      cout << "NO VALID CSV FILE FOUND! " << endl;
      cout << "please provide a valid 'vasculatures.csv'" << endl;
    }

  //Create Bloodcircuit with all Bloodvessels.
  //
  ConnectBloodvessels ();

  SetTransitionProbabilities();
  SetFingerprintTimes();
  //Inject Nanobots here!
  //Places x Bots, randomly on Streams at specific vessel injected.
  //If you choose other values, the nanobots all get injected at the same coordinates
  if (m_bloodvessels.size() > 1){
    InjectNanobots (numberOfNanobots, numberOfCollectors, numberOfLocators, m_bloodvessels[injectionVessel <  m_bloodvessels.size() ?  injectionVessel : m_bloodvessels.size() -1], particleMode);
  }
}

//Starts the simulation in a bloodvessel
void
Starter (Ptr<Bloodvessel> vessel)
{
  vessel->Start ();
}

int
Bloodcircuit::BeginSimulation (unsigned int simulationDuration, unsigned int numOfNanobots, unsigned int injectionVessel, unsigned int numOfCollectors, unsigned int numOfLocators, unsigned int particleMode)
{
  //execution time
  clock_t start, finish;
  start = clock ();
  Ptr<PrintNanobots> printNano = new PrintNanobots (particleMode);
  //Create the bloodcircuit
  Bloodcircuit circuit (numOfNanobots, numOfCollectors, numOfCollectors, injectionVessel, printNano, particleMode);
  //Get the map of the bloodcircuit
  map<int, ns3::Ptr<ns3::Bloodvessel>> circuitMap = circuit.GetBloodcircuit ();
  if(circuitMap.size() > 1){
    // Schedule and run the Simulation in each bloodvessel
    for (unsigned int i = 1; i < circuitMap.size() + 1; i++)
      {
                Simulator::Schedule (Seconds (0.0), &Starter, circuitMap[i]); 
      }
    //Stop simulation after specific time
    Simulator::Stop (Seconds (simulationDuration + 1));
    Simulator::Run ();
    Simulator::Destroy ();
    //output execution time
    finish = clock ();
    cout << "Dauer: " << simulationDuration << "s " << numOfNanobots << "NB " << numOfCollectors << "NC " << numOfLocators << "NL -> "
        << (finish - start) / 1000000 << "s ------------------------" << endl;
    cout << "Injected Vessel: " << injectionVessel << endl;
    //Flushing last entries in csv 
    printNano->~PrintNanobots();
    return 0;
  } else {
     cout << "Not enough vessels for simulation! Please check 'vasculature.csv'" << endl;
     return 1;
  }
}

Bloodcircuit::~Bloodcircuit ()
{
  m_bloodvessels.clear ();
}

map<int, ns3::Ptr<ns3::Bloodvessel>>
Bloodcircuit::GetBloodcircuit ()
{
  return m_bloodvessels;
}

Vector
Bloodcircuit::CalcDirectionVectorNorm (Ptr<Bloodvessel> m_bloodvessel)
{
  Vector start = m_bloodvessel->GetStartPositionBloodvessel ();
  Vector end = m_bloodvessel->GetStopPositionBloodvessel ();
  double x = end.x - start.x;
  double y = end.y - start.y;
  double z = end.z - start.z;
  double vectorLength = sqrt (pow (x, 2) + pow (y, 2) + pow (z, 2));
  x = x / vectorLength;
  y = y / vectorLength;
  z = z / vectorLength;
  return Vector (x, y, z);
}

void
Bloodcircuit::InjectNanobots (int numberOfNanobots, int numberOfCollectors, int numberOfLocators, Ptr<Bloodvessel> bloodvessel, int particleMode)
{
  int numberOfNanocollectors = numberOfCollectors; //if bigger than numnb fehler
  int numberOfNanolocators = numberOfLocators; //if bigger than numnb fehler
  int nanobotGroupSize = 10;
  Ptr<UniformRandomVariable> distribute_randomly =
      bloodvessel->getRandomObjectBetween (0, bloodvessel->GetNumberOfStreams ());
  Vector m_coordinateStart = bloodvessel->GetStartPositionBloodvessel ();
  int intervall =
      (numberOfNanobots >= nanobotGroupSize) //IF  equal or more than 10 Nanobots are injected,
          ? div (numberOfNanobots, nanobotGroupSize)
                .quot // THEN Divide number of Nanobots into 10 equaly sized groups + remainder
          : numberOfNanobots; // ELSE put them on beginning of the bloodvessel in one point.
  //Calculate the normalized direction vector of the start vessel.
  Vector m_direction = CalcDirectionVectorNorm (bloodvessel);
  //Set direction intervall as 1/10 of the normalized direction vector.
  Vector directionIntervall =
      Vector (m_direction.x / nanobotGroupSize, m_direction.y / nanobotGroupSize,
              m_direction.z / nanobotGroupSize);
  unsigned int group = 0; // Group 0 to 9
  
  // //Particle mode LDL Cholesterol  
  if (particleMode == 1)
  {
    //release particles from the liver Organ 36
    for (int i = 1; i <= 100; ++i)
      {
        Ptr<Bloodvessel> liver = m_bloodvessels[36];
        Vector m_coordinateLiver = liver->GetStartPositionBloodvessel ();
        Ptr<Nanobot> temp_np = CreateObject<Nanoparticle> ();
        temp_np->SetNanobotID (i+numberOfNanocollectors+numberOfNanolocators+numberOfNanobots);
        //Get random stream number.
        int dr = floor (distribute_randomly->GetValue ());
        temp_np->SetShouldChange (false);
        temp_np->SetPosition (Vector (m_coordinateLiver.x,
                                      m_coordinateLiver.y,
                                      m_coordinateLiver.z));
        //Set Speed and Detection Radius of particles
        temp_np->SetDelay (2.32);
        temp_np->SetDetectionRadius (0.2);
        //Set position with random stream dr.
        bloodvessel->AddNanobotToStream (dr, temp_np);
        //for a second release, go to Bloodvessel.cc, Line 90 and change the variable secondParticleRelease to true
      }
  }
    //Particle mode liquid biopsy breast 
  if (particleMode == 2)
  {
    //release particles from the breast Organ 24
    for (int i = 1; i <= 160000; ++i)
      {
        Ptr<Bloodvessel> breast = m_bloodvessels[24];
        Vector m_coordinateBreast = breast->GetStartPositionBloodvessel ();
        Ptr<Nanobot> temp_np = CreateObject<Nanoparticle> ();
        temp_np->SetNanobotID (i+numberOfNanocollectors+numberOfNanolocators+numberOfNanobots);
        //Get random stream number.
        int dr = floor (distribute_randomly->GetValue ());
        temp_np->SetShouldChange (false);
        temp_np->SetPosition (Vector (m_coordinateBreast.x,
                                      m_coordinateBreast.y,
                                      m_coordinateBreast.z));
        //Set Speed and Detection Radius of particles
        temp_np->SetDelay (4.71);
        temp_np->SetDetectionRadius (0.01);
        //Set position with random stream dr.
        bloodvessel->AddNanobotToStream (dr, temp_np);
      }
  }

  //Distribute Nanocollectors at the beginning of the start vessel.
  if (numberOfNanocollectors > 0)
  {
    for (int i = 1; i <= numberOfNanocollectors; ++i)
    {
          Ptr<Nanocollector> temp_nb = CreateObject<Nanocollector> ();
          temp_nb->SetNanobotID (i);
          //Get random stream number.
          int dr = floor (distribute_randomly->GetValue ());
          temp_nb->SetShouldChange (false);
          temp_nb->SetPosition (Vector (m_coordinateStart.x,
                                      m_coordinateStart.y,
                                      m_coordinateStart.z));
          //Divide the Collectors equally among the fingerprint organs
          temp_nb->SetTargetOrgan(m_fingerprint_organs[i%9]);
          //Set position with random stream dr.
          bloodvessel->AddNanobotToStream (dr, temp_nb);
    }
  }
  //Distribute Nanolocators at the beginning of the start vessel.
    if (numberOfNanolocators > 0)
  {
    for (int i = 1; i <= numberOfNanolocators; ++i)
    {
          Ptr<Nanolocator> temp_nb = CreateObject<Nanolocator> ();
          temp_nb->SetNanobotID (i);
          //Get random stream number.
          int dr = floor (distribute_randomly->GetValue ());
          temp_nb->SetShouldChange (false);
          temp_nb->SetPosition (Vector (m_coordinateStart.x,
                                      m_coordinateStart.y,
                                      m_coordinateStart.z));
          //Divide the Locators equally among the fingerprint organs
          temp_nb->SetTargetOrgan(m_fingerprint_organs[i%9]);
          //Set position with random stream dr.
          bloodvessel->AddNanobotToStream (dr, temp_nb);
    }
  }
  //Distribute Nanobots in 10 groups over the beginning of the start vessel.
  for (int i = 1; i <= numberOfNanobots; ++i)
    {
      group = (i - 1) / intervall;
      Ptr<Nanobot> temp_nb = CreateObject<Nanobot> ();
      temp_nb->SetNanobotID (i+numberOfNanocollectors);
      //Get random stream number.
      int dr = floor (distribute_randomly->GetValue ());
      temp_nb->SetShouldChange (false);
      temp_nb->SetPosition (Vector (m_coordinateStart.x + (directionIntervall.x * group),
                                    m_coordinateStart.y + (directionIntervall.y * group),
                                    m_coordinateStart.z + (directionIntervall.z * group)));
      //Set position with random stream dr.
      bloodvessel->AddNanobotToStream (dr, temp_nb);
    }
  //Print Nanobots in csv-file.

  bloodvessel->PrintNanobotsOfVessel ();
}


double
Bloodcircuit::GetSpeedClassOfBloodVesselType (BloodvesselType type)
{
  if (type == ARTERY)
    {
      return 10.0;
    }
  else if (type == VEIN)
    {
      return 3.7;
    }
  else // if (type == ORGAN)
    {
      return 1.0;
    }
}

void
Bloodcircuit::AddVesselData (int id, BloodvesselType type, Vector start, Vector stop)
{
  Ptr<Bloodvessel> vessel = CreateObject<Bloodvessel> ();
  vessel->SetBloodvesselID (id);
  vessel->SetBloodvesselType (type);
  vessel->SetStartPositionBloodvessel (start);
  vessel->SetStopPositionBloodvessel (stop);
  vessel->SetVesselWidth (0.25); // 0.25
  vessel->SetPrinter (printer);
  // Init Bloodvessel: Calculate length and angle & velocity.
  vessel->InitBloodstreamLengthAngleAndVelocity (GetSpeedClassOfBloodVesselType (type));
  m_bloodvessels[id] = vessel;
  cout << "New Vessel(" + to_string (id) + "," + to_string (type) + "," + to_string (start.x) +
              "," + to_string (start.y) + "," + to_string (start.z) + "," + to_string (stop.x) +
              "," + to_string (stop.y) + "," + to_string (stop.z) + ")"
       << endl;
}

void
Bloodcircuit::InitialiseBloodvessels (int vesseldata[][8], unsigned int count)
{
  for (unsigned int i = 0; i < count; i++)
    {
      AddVesselData (vesseldata[i][0], (BloodvesselType) vesseldata[i][1],
                     Vector (vesseldata[i][2], vesseldata[i][3], vesseldata[i][4]),
                     Vector (vesseldata[i][5], vesseldata[i][6], vesseldata[i][7]));
    }
}

void
Bloodcircuit::ConnectBloodvessels ()
{
  unsigned int count = m_bloodvessels.size();
  cout << count << endl;
  //Set Connections between bloodvessels if they have the same start/end coordinates
  for (unsigned int i = 1; i < count + 1; i++)
    {
      unsigned int counter = 0;
      Vector end = m_bloodvessels[i]->GetStopPositionBloodvessel ();
      //Make sure that inititally all Bloodvessels have no Fingerprint Formation Times
      m_bloodvessels[i]->SetFingerprintFormationTime(0);
      for (unsigned int j = 1; j < count + 1; j++)
        {
          Vector start = m_bloodvessels[j]->GetStartPositionBloodvessel ();
          if (end.x == start.x && end.y == start.y && end.z == start.z)
            {
              counter++;
              if (counter == 1)
                {
                  m_bloodvessels[i]->SetNextBloodvessel1 (m_bloodvessels[j]);
                }
              else
                {
                  m_bloodvessels[i]->SetNextBloodvessel2 (m_bloodvessels[j]);
                }
            }
        }
    }
}

void
Bloodcircuit::SetTransitionProbabilities (){
  std::ifstream infile{"transitions95.csv"};
  if (infile.good ())
     {
       std::vector<double> numbers;
       numbers.resize (3);
       std::string buffer;
       buffer.reserve (64);
       int errorflag = 0;
       while (infile.good ())
         {
           for (auto &&elem : numbers)
             {
               if (std::getline (infile, buffer, ','))
                 {
                   elem = std::stod (buffer);
                 }
               else
                 {
                   elem = 0;
                   errorflag = 1;
                 }
             }
           if (errorflag == 0)
             {
               m_bloodvessels[numbers[0]]->SetTransition1 (numbers[1]);
               m_bloodvessels[numbers[0]]->SetTransition2 (numbers[2]);
             }
         }
       cout << "load transitions" << endl;
     }
  else
    { // Old way of loading data
      cout << "NO VALID CSV FILE FOUND! " << endl;
      cout << "Transitions set to 1 and 0" << endl;
    }
}
void 
Bloodcircuit::SetFingerprintTimes (){
  std::ifstream infile{"fingerprint.csv"};
  if (infile.good ())
     {
       std::vector<double> numbers;
       numbers.resize (2);
       std::string buffer;
       buffer.reserve (64);
       int errorflag = 0;
       while (infile.good ())
         {
           for (auto &&elem : numbers)
             {
               if (std::getline (infile, buffer, ','))
                 {
                   elem = std::stod (buffer);
                 }
               else
                 {
                   elem = 0;
                   errorflag = 1;
                 }
             }
           if (errorflag == 0)
             {
               m_bloodvessels[numbers[0]]->SetFingerprintFormationTime (numbers[1]);
               

               //Make a vector list, with the organs that have a fingerprint
               m_fingerprint_organs.push_back(numbers[0]);
             }
         }
       cout << "load fingerprint times" << endl;
     }
  else
    { // Old way of loading data
      cout << "NO VALID CSV FILE FOUND! " << endl;
      cout << "Fingerprint times not added" << endl;
    }
}

} // namespace ns
