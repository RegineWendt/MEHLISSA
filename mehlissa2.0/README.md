# MEHLISSA 2.0: A Medical Holistic Simulation Architecture for Nanonetworks in Humans
Regine Wendt <regine.wendt@uni-luebeck.de>
v1.1, 2024-06-02

Lisa Y. Debus <debus@ccs-labs.org>
v2.0, 2025-07-10

## Introduction

MEHLISSA is a program that simulates the global movement of objects in the human body. Its main purpose is to model the work environment of medical nanodevices. 
MEHLISSA is an extension and replacement of the BloodVoyagerS Module for ns-3 (https://github.com/RegineWendt/blood-voyager-s). 
MEHLISSA 2.0 removes the ns3 dependencies and is a stand-alone simulator.

## References 

Please check out the literature folder for more information on the model and scenario. If you use MEHLISSA, cite at least:

```
Regine Wendt und Stefan Fischer: „MEHLISSA: a medical holistic simulation architecture for nanonetworks in humans“,
Proceedings of the 7th ACM International Conference on Nanoscale Computing and Communication,
Virtual Event USA: ACM, Sep. 2020, S. 1–6,
isbn: 978-1-4503-8083-6,
doi: 10.1145/ 3411295.3411305,
url: https://dl.acm.org/doi/10.1145/3411295.3411305
```

You can find detailed descriptions of the complete model, UML structures, scenarios, calculations, etc. in Regine's dissertation (in German). 

If you use information from Regine's dissertation, i.e., the scenarios, cite in addition: 

```
Regine Wendt: „Einsatz von Nanotechnologien in der Präzisionsmedizin“,
Dissertation,
Universität zu Lübeck, 2024
```

If you use fingerprinting, cite Regine's dissertation and: 

```
Regine Wendt, Florian-Lennert Lau, Lena Unger und Stefan Fischer: „Proteome Fingerprinting as a Localization Scheme for Nanobots“,
Proceedings of the 10th ACM International Conference on Nanoscale Computing and Communication,
NANOCOM ’23, Coventry, United Kingdom: Association for Computing Machinery, 2023, S. 27–32,
isbn: 9798400700347,
doi: 10.1145/3576781.3608728,
url: https://doi.org/10.1145/3576781.3608728
```

## Features

  - 3D coordinates of major vessels and organs
  - Simulates permanent movement of nanodevices in the cardiovascular system
  - Injection of nanodevices in specific vessels
  - Detection of nanodevices in a specific vessel
  - Provides global position data of nano devices
  - Simulates Nanolocators, Nanocollectors, Nanoparticles, Cancer Cells, CAR-T Cells, and T Cells
  - Provides interaction functionality for the simulated particles, e.g., mitosis based on static growth rate, mitosis triggered by particle interaction, cell fratricide, localization, detection

## Important Parameters

  - Vessel IDs between `[1-95]`
  - Simulation duration in seconds `m_simulationDuration`
  - Duration between each simulation step `m_deltaT`
  - Velocities: `m_arteryVelocity = 10 cm/s, m_veinVelocity = 3,7 cm/s, m_organVelocity = 1cm/s`

## Getting started

### Compile MEHLISSA 2.0

To compile MEHLISSA 2.0 you will need to have CMake installed on your computer.
For the compilation, navigate into the mehlissa2.0/src folder and execute the following commands:

```
cmake .
make clean all
```

### Running MEHLISSA 2.0

You can pass different command-line arguments to MEHLISSA for the simulation.


|Parameter|Type|Defaultvalue|
|---------|--------|--------|
|"numCancerCells" | int | 100 |
|"numCarTCells" | int | 100 |
|"numTCells" | int | 100 |
|"simulationStep" | double | 1.0 |
|"simulationDuration" | int | 100 |
|"injectionTime" | double | 20.0 |
|"injectionVessel" | int | 29 |
|"detectionVessel" | int | 23 |
|"isDeterministic" | bool | false |
|"parallel" | int | 1 |
|"simFile" | string | "../output/csvnano.csv" |
|"gwFile" | string | "../output/gwDetect.csv" |
|"networkFile" | string | "../data/vasculature_transitions95.csv" |
|"transitionsFile" | string | "../data/transitions95.csv" |
|"fingerprintFile" | string | "../data/fingerprint.csv" |

You can run MEHLISSA 2.0 from the src folder as:

```
NUMOFNANOBOTS=0 INJECTVESSEL=68 DETECTVESSEL=29 NUMCANCERCELLS=300000 NUMCARTCELLS=2000 NUMTCELLS=300000 SIMDURATION=100 SIMSTEP=1 INJECTTIME=20 ISDETERMINISTIC=false PARALLELITY=1 SIMFILE="../output/csvNano_cancer001.csv" GWFILE="../output/gwNano_cancer001.csv" VASCFILE="../data/95_vasculature.csv" TRANSFILE="../data/95_transitions.csv" FINGERPRINTS="../data/95_fingerprints.csv"
../bin/MehlissaCancer --simulationDuration=$SIMDURATION --simulationStep=$SIMSTEP --numCancerCells=$NUMCANCERCELLS --numCarTCells=$NUMCARTCELLS --numTCells=$NUMTCELLS --injectionTime=$INJECTTIME --injectionVessel=$INJECTVESSEL --detectionVessel=$DETECTVESSEL --isDeterministic=$ISDETERMINISTIC --simFile=$SIMFILE  --gwFile=$GWFILE --parallel=$PARALLELITY --networkFile=$VASCFILE --transitionsFile=$TRANSFILE --fingerprintFile=$FINGERPRINTS
```

