# MEHLISSA 2.0

For example:

``
cd src
cmake .
make clean all
NUMOFNANOBOTS=0 INJECTVESSEL=68 DETECTVESSEL=29 NUMCANCERCELLS=300000 NUMCARTCELLS=2000 NUMTCELLS=300000 SIMDURATION=100 SIMSTEP=1 INJECTTIME=20 ISDETERMINISTIC=false PARALLELITY=1 SIMFILE="../output/csvNano_cancer001.csv" GWFILE="../output/gwNano_cancer001.csv" VASCFILE="../data/95_vasculature.csv" TRANSFILE="../data/95_transitions.csv" FINGERPRINTS="../data/95_fingerprints.csv"
../bin/MehlissaCancer --simulationDuration=$SIMDURATION --simulationStep=$SIMSTEP --numCancerCells=$NUMCANCERCELLS --numCarTCells=$NUMCARTCELLS --numTCells=$NUMTCELLS --injectionTime=$INJECTTIME --injectionVessel=$INJECTVESSEL --detectionVessel=$DETECTVESSEL --isDeterministic=$ISDETERMINISTIC --simFile=$SIMFILE  --gwFile=$GWFILE --parallel=$PARALLELITY --networkFile=$VASCFILE --transitionsFile=$TRANSFILE --fingerprintFile=$FINGERPRINTS
``
