#/********************************************************************************************************************** 
# * The LookingGlass Project
# *
# * Author: 
# *	Alessandro Febretti
# *********************************************************************************************************************/ 
PROJECT(LookingGlass)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

#######################################################################################################################
# Add subdirectiories
ADD_SUBDIRECTORY(src/3rdparty/libconfig)
ADD_SUBDIRECTORY(src/lglass)
