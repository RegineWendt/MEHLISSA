build_lib(
  LIBNAME mehlissa
  
  SOURCE_FILES Bloodvessel.cc
    Bloodstream.cc
    Nanobot.cc
    Bloodcircuit.cc
    PrintNanobots.cc
    Nanocollector.cc
    Nanolocator.cc
    Nanoparticle.cc
   
  HEADER_FILES Bloodvessel.h
    Bloodstream.h
    Nanobot.h
    Bloodcircuit.h
    PrintNanobots.h
    Nanocollector.h
    Nanolocator.h
    Nanoparticle.h
  
  LIBRARIES_TO_LINK
    ${libinternet}
    ${libcore}
    ${libmobility}
    ${libwifi}
    ${libnetwork}
)