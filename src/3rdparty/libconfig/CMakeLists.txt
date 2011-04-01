#######################################################################################################################
# THE LOOKING GLASS VISUALIZATION TOOLSET
#---------------------------------------------------------------------------------------------------------------------
# Author: 
#	Alessandro Febretti							Electronic Visualization Laboratory, University of Illinois at Chicago
# Contact & Web:
#  febret@gmail.com							http://febretpository.hopto.org
#---------------------------------------------------------------------------------------------------------------------
# Looking Glass has been built as part of the ENDURANCE Project (http://www.evl.uic.edu/endurance/).
# ENDURANCE is supported by the NASA ASTEP program under Grant NNX07AM88G and by the NSF USAP.
#######################################################################################################################

#######################################################################################################################
SET (Sources
        libconfig.c
        scanctx.c
        scanner.c
        strbuf.c
        grammar.c
        libconfigcpp.cpp)

#######################################################################################################################
# Compile as a static library
ADD_DEFINITIONS( -DLIBCONFIG_STATIC -DLIBCONFIGXX_STATIC)
        
#######################################################################################################################
# Setup compile info
ADD_LIBRARY(libconfig ${Sources})
