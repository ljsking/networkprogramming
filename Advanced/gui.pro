#############################################
#
# Example for using Precompiled Headers
#
#############################################
TEMPLATE  = app
LANGUAGE  = C++
CONFIG	 += console precompile_header

HEADERS	  = dialog.h myThread.h
SOURCES	  = gui.cpp dialog.cpp myThread.cpp
FORMS	  = gui.ui

