# MEHLISSA 2.0: A Medical Holistic Simulation Architecture for Nanonetworks in Humans
Regine Wendt <regine.wendt@uni-luebeck.de>
v1.1, 2024-06-02

Lisa Y. Debus <debus@ccs-labs.org>
v2.0, 2025-07-10

## Introduction

MEHLISSA is a program that simulates the global movement of objects in the human body. 
Its main purpose is to model the work environment of medical nanodevices. 
The original MEHLISSA application relies on the general-purpose event-based network simulator ns-3 to model the movement of molecular communication (MC) particles through the human circulatory system (HCS).
While ns-3 is very versatile and offers many options and extensions, these are not all necessary in MEHLISSA and unnecessarily increase its required computational resources.
We therefore decided to remove MEHLISSA's dependency on ns-3 and instead implemented a much simpler simulation core.

At the start of the simulation process, we create a `BloodCircuit` instance.
It contains all `BloodVessel` and `Particle` objects.
We pass the prepared `BloodCircuit` to an instance of the `Simulator` class on creation, which then starts the simulation process.
The simulation loop moves forward in time according to the time step size set via the command line. 
In each step, the `Simulator` prompts the movement of the particles in every `BloodVessel` and their transition between connected vessels.
The `Simulator` also coordinates several utility classes.
Existing extensions of MEHLISSA or its predecessor BVS can be adapted to work in MEHLISSA 2.0 by simply adapting the necessary calls to the new timer and position structures.

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

### Compiling MEHLISSA 2.0

To compile MEHLISSA 2.0 you will need to have CMake installed on your computer.
For the compilation, navigate into the mehlissa2.0/src folder and execute the following commands:

```
cmake .
make clean all
```

### Running MEHLISSA 2.0


You can run MEHLISSA 2.0 from the command line via:

```
./bin/MehlissaCancer [command line arguments]
```

#### Example: CAR-T Cell Treatment

As an example, we implement the movement of particles in a chimeric antigen receptor (CAR)-T cell treatment for leukemia in MEHLISSA 2.0.
CAR-T cells are a relatively new immunotherapy against cancer.
The approach relies on modifying T cells into functionalized CAR-T cells that are specialized in attacking the tumorous T cells.
At the moment, they are mainly used for the treatment of cancers based in the hematopoietic and lymphoid tissues [1].
Here, we aim to simulate the treatment of leukemia with CAR-T cells and implement the movement and interaction of CAR-T cells with leukemic cells and healthy T cells in the HCS.

<img width="1561" height="429" alt="cart" src="https://github.com/user-attachments/assets/6bf6f08e-a9ce-4e4a-831d-88c302bcbf05" />

Fig. 1: Particle interactions in the CAR-T cell treatment for leukemia based on Pérez-García et al. [2].

We implemented classes to represent the CAR-T cells, leukemic T cells, and healthy T cells.
Their interaction was implemented according to the mathematical model by Perez-Garcia et al. [2].
As shown in Figure 1, cancer cells replicate at a constant rate and are killed by CAR-T cells with a certain probability.
At the same time, their interaction stimulates the proliferation of the CAR-T cells.
With a lower probability, CAR-T cells can also be toxic towards the host immune system and kill off healthy T-cells.
Similarly, healthy T-cells can also stimulate the proliferation of CAR-T cells.
Additionally, CAR-T cells are killed by fratricide and die after a finite lifetime.
The interaction probabilities and radii, as well as the replication rate, are set statically for each class.

At the start of the simulation, cancer cells and healthy T cells are positioned in a uniform distribution over the full HCS.
MEHLISSA starts the simulation and allows a short period for the system to reach a steady state, before it injects CAR-T cells into the HCS via a Bloodvessel object.
In the following simulation, all particles move through the HCS according to the blood flow in the vessels and interact with each other as described above.

#### Command Line Arguments
You can pass different command-line arguments to MEHLISSA for the simulation.


|Parameter|Type|Defaultvalue|Description|
|---------|--------|--------|-----------|
|"numCancerCells" | int | 100 | number of simulated cancer cells |
|"numCarTCells" | int | 100 | number of simulated CAR-T cells |
|"numTCells" | int | 100 | number of simulated T cells |
|"simulationStep" | double | 1.0 | simulation time step size in seconds |
|"simulationDuration" | int | 100 | simulation time in seconds |
|"injectionTime" | double | 20.0 | injection time for the CAR-T cells in seconds |
|"injectionVessel" | int | 29 | injection vessel for the CAR-T cells |
|"detectionVessel" | int | 23 | gateway vessel, registering all passing cells |
|"isDeterministic" | bool | false | use a random seed or not |
|"parallel" | int | 1 | parallel execution of the simulation (currently only = 1) |
|"simFile" | string | "../output/csvnano.csv" | output file of all particle positions |
|"gwFile" | string | "../output/gwDetect.csv" | output file of particles detected at the gateway |
|"networkFile" | string | "../data/95_vasculature.csv" | network file of the simulation |
|"transitionsFile" | string | "../data/95_transitions.csv" | transitions file of the simulation |
|"fingerprintFile" | string | "../data/95_fingerprint.csv" | fingerprints file of the simulation |

#### Running the Simulation

You can run the CAR-T cell treatment simulation in MEHLISSA 2.0 from the src folder as:

```
NUMOFNANOBOTS=0 INJECTVESSEL=68 DETECTVESSEL=29 NUMCANCERCELLS=300000 NUMCARTCELLS=2000 NUMTCELLS=300000 SIMDURATION=100 SIMSTEP=1 INJECTTIME=20 ISDETERMINISTIC=false PARALLELITY=1 SIMFILE="../output/csvNano_cancer001.csv" GWFILE="../output/gwNano_cancer001.csv" VASCFILE="../data/95_vasculature.csv" TRANSFILE="../data/95_transitions.csv" FINGERPRINTS="../data/95_fingerprints.csv"
../bin/MehlissaCancer --simulationDuration=$SIMDURATION --simulationStep=$SIMSTEP --numCancerCells=$NUMCANCERCELLS --numCarTCells=$NUMCARTCELLS --numTCells=$NUMTCELLS --injectionTime=$INJECTTIME --injectionVessel=$INJECTVESSEL --detectionVessel=$DETECTVESSEL --isDeterministic=$ISDETERMINISTIC --simFile=$SIMFILE  --gwFile=$GWFILE --parallel=$PARALLELITY --networkFile=$VASCFILE --transitionsFile=$TRANSFILE --fingerprintFile=$FINGERPRINTS
```

## References

[1] Gunjan Dagar, Ashna Gupta, Tariq Masoodi, Sabah Nisar, Maysaloun Merhi, Sheema Hashem, Ravi Chauhan, Manisha Dagar, Sameer Mirza, Puneet Bagga, Rakesh Kumar, Ammira S. Al-Shabeeb Akil, Muzafar A. Macha, Mohammad Haris, Shahab Uddin, Mayank Singh, and Ajaz A. Bhat. 2023. Harnessing the Potential of CAR-T Cell Therapy: Progress, Challenges, and Future Directions in Hematological and Solid Tumor Treatments. Journal of Translational Medicine 21, 1 (7 2023). https://doi.org/10.1186/s12967-023-04292-3

[2] Víctor M. Pérez-García, Odelaisy León-Triana, María Rosa, and Antonio Pérez-Martínez. 2021. CAR T Cells for T-Cell Leukemias: Insights from Mathematical Models. Communications in Nonlinear Science and Numerical Simulation 96 (May 2021). https://doi.org/10.1016/j.cnsns.2020.105684
