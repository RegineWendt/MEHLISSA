build_lib_example(
  NAME start-mehlissa
  SOURCE_FILES start-mehlissa
    
  LIBRARIES_TO_LINK 
    ${libmehlissa}
    ${libinternet}
    ${libcore}
    ${libmobility}
    ${libwifi}
    ${libnetwork}
)
build_lib_example(
  NAME start-cartcelltherapy
  SOURCE_FILES start-cartcelltherapy
    
  LIBRARIES_TO_LINK 
    ${libmehlissa}
    ${libinternet}
    ${libcore}
    ${libmobility}
    ${libwifi}
    ${libnetwork}
)
