cmake_minimum_required(VERSION 3.15...3.30)

project(
  MEHLISSA_2.0
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "../lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "../lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "../bin")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp -g")

find_package(Boost COMPONENTS program_options REQUIRED)
find_package(OpenMP REQUIRED)
add_library(MehlissaLib 
  bloodcircuit/BloodCircuit.cc  bloodcircuit/BloodCircuit.h
  bloodcircuit/Bloodstream.cc  bloodcircuit/Bloodstream.h
  bloodcircuit/BloodVessel.cc  bloodcircuit/BloodVessel.h
  particles/CancerCell.cc  particles/CancerCell.h
  particles/CarTCell.cc  particles/CarTCell.h
  particles/TCell.cc  particles/TCell.h
  particles/Particle.cc  particles/Particle.h
  particles/Nanocollector.cc  particles/Nanocollector.h
  particles/Nanolocator.cc  particles/Nanolocator.h
  particles/Nanoparticle.cc  particles/Nanoparticle.h
  utils/GlobalTimer.cc  utils/GlobalTimer.h
  utils/IDCounter.cc  utils/IDCounter.h
  utils/Position.cc  utils/Position.h
  utils/Printer.cc  utils/Printer.h
  utils/Randomizer.cc  utils/Randomizer.h
  utils/RandomStream.cc  utils/RandomStream.h
  experiments/Simulator.cc  experiments/Simulator.h
)
add_executable(MehlissaCancer experiments/start-cartcelltherapy.cc
)
target_include_directories(MehlissaCancer PUBLIC lib/boost_1_82_0)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(MehlissaCancer PRIVATE MehlissaLib 
                                             ${Boost_LIBRARIES} 
                                             ${OpenMP_LIBRARIES})

set_property(TARGET MehlissaLib PROPERTY CXX_STANDARD 23)
set_property(TARGET MehlissaCancer PROPERTY CXX_STANDARD 23)
