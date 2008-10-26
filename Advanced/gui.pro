#############################################
#
# Example for using Precompiled Headers
#
#############################################
TEMPLATE  = app
LANGUAGE  = C++
CONFIG	 += console precompile_header

HEADERS	  = dialog.h
SOURCES	  = gui.cpp dialog.cpp
FORMS	  = gui.ui

