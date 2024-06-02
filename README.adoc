= MEHLISSA: A Medical Holistic Simulation Architecture for Nanonetworks in Humans
Regine Wendt <regine.wendt@uni-luebeck.de>
v1.1, 2024-06-02

== Introduction

MEHLISSA is a ns-3 module that simulates the global movement of objects in the human body. It’s main purpose is to model the work environment of medical nanobots. 
MEHLISSA is an extension and replacement of the BloodVoyagerS Module for ns-3 (https://github.com/RegineWendt/blood-voyager-s). 

== Features

  - 3D coordinates of major vessels and organs
  - Simulates permanent movement of nanobots in cardiovascular system
  - Injection of nanobots in specific vessels
  - Provides global position data of nanobots (/ns-3.x/csvNano.csv)
  - Simulates Nanolocators, Nanocollectors and Nanoparticles

== Important Parameters

  - Number of nanobots `m_numberOfNanobots`
  - Injection vessel `m_injectionVessel [1-95]`
  - Simulation duration in seconds `m_simulationDuration`
  - Duration between each simulation step `m_deltaT`
  - Velocities: `m_arteryVelocity = 10 cm/s, m_veinVelocity = 3,7 cm/s, m_organVelocity = 1cm/s`

== Tutorial/Get started

=== You’ve already installed ns-3.36

Take the mehlissa version, it is set for ns-3.36. Note that ns3 moved from waf to cmake, so the building and running is different. 

=== You're new to ns-3

You need to download the complete ns-3.36 (or possibly newer) and follow the instructions in this  https://www.nsnam.org/docs/tutorial/html/getting-started.html[ns-3 tutorial]. 
Then you download the mehlissa folder from here and put it in the src folder. 
In addition, you need to put the other three .csv files in your ns-3.x folder. For further instructions see "Run MEHLISSA".

== Run MEHLISSA Version ns-3.36
    SIMDURATION=   # simulation duration in seconds
    NUMOFNANOBOTS= # number of nanobots
    INJECTVESSEL=  # injection vessel [1-95]
    NUMOFCOLLECTORS= # specialised nanodevices that collect messages
    NUMOFLOCATORS= # specialised nanodevices that can locate nine organs
    PARTICLEMODE = Mode 1 simulates LDL particles, Mode 2 simulates Liquid Biopsy

    for the simulation as in bvs with a refined transition model and new 3D features
    ./ns3 run "start-mehlissa --simulationDuration=$SIMDURATION, --numOfNanobots=$NUMOFNANOBOTS, --injectionVessel=$INJECTVESSEL"
    
    or with default values
    ./ns3 run start-mehlissa

    to simulate nanocollectors and nanolocators
    ./ns3 run "start-mehlissa --simulationDuration=$SIMDURATION, --numOfNanobots=$NUMOFNANOBOTS, --injectionVessel=$INJECTVESSEL, --numOfCollectors=$NUMOFCollectors, --numOfLocators=$NUMOFLOCATORS"

    to simulate ldl particle detection
    ./ns3 run "start-mehlissa --simulationDuration=$SIMDURATION, --numOfNanobots=$NUMOFNANOBOTS, --injectionVessel=$INJECTVESSEL, --particleMode=1"

    you can release more particles from the Bloodvessel.cc during the simulation

     to simulate liquid biopsy of ctDNA
    ./ns3 run "start-mehlissa --simulationDuration=$SIMDURATION, --numOfNanobots=$NUMOFNANOBOTS, --injectionVessel=$INJECTVESSEL, --particleMode=2"

    In addition you can use other bodymodels by changing the used vasculature.csv in the Bloodcircuit.cc 

    
The simulation returns a csv-file (/ns-3.36/csvNano.csv) with the position data of the simulated nanobots or other devices and particles and their status in every timestep. 

