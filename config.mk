#---------------------------------------------------------------------
#  $Header:$
#  Copyright (c) 2000-2007 Vivotek Inc. All rights reserved.
#
#  +-----------------------------------------------------------------+
#  | THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY ONLY BE USED |
#  | AND COPIED IN ACCORDANCE WITH THE TERMS AND CONDITIONS OF SUCH  |
#  | A LICENSE AND WITH THE INCLUSION OF THE THIS COPY RIGHT NOTICE. |
#  | THIS SOFTWARE OR ANY OTHER COPIES OF THIS SOFTWARE MAY NOT BE   |
#  | PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY OTHER PERSON. THE   |
#  | OWNERSHIP AND TITLE OF THIS SOFTWARE IS NOT TRANSFERRED.        |
#  |                                                                 |
#  | THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT   |
#  | ANY PRIOR NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY |
#  | VIVOTEK INC.                                                    |
#  +-----------------------------------------------------------------+
#
#  Project name         : ONVIF
#  Module name          : onvifeventd
#  Module description   : ONVIF event handler daemon
#  Author               : Jerry Kuo
#  Created at           : 2009/12/17
#  $History:$
#
#---------------------------------------------------------------------

#---------------------------------------------------------------------
# Configurations
#---------------------------------------------------------------------

# general configuration, for each configuration
#---------------------------------------------------------------------
PLATFORM 	= _LINUX
INCDIR	 	= $(PROJ_ROOT)/sqlite3/sqlite3.${OSEXTENSION}/inc
SYSDEFS  	= 
CFLAGS	 	= 
USRDEFS  	=
BUILD_CONF  = armlinux_release
LINKFLAGS   = -ldl -lsqlite3
LIBS		= $(common_lib) $(sqlite3_lib)
TOOL_CONFIG = ARMLinux


# specific configuration
#---------------------------------------------------------------------
ifeq "$(BUILD_CONF)" "linux_release"
LIBS 		= common 
CUSTOMLIBS 	= osisolate
CFLAGS	 	= -O3 -D_NDEBUG
endif

#---------------------------------------------------------------------
ifeq "$(BUILD_CONF)" "linux_debug"
CUSTOMLIBS 	= common
CFLAGS		= -O0 
endif

#---------------------------------------------------------------------
ifeq "$(BUILD_CONF)" "vcwin32_release"
PLATFORM 	= _WIN32_
TOOL_CONFIG = VCWin32
LIBS 		= common
CFLAGS	 	= /O2 -D_NDEBUG
endif

#---------------------------------------------------------------------
ifeq "$(BUILD_CONF)" "armlinux_debug"
TOOL_CONFIG = ARMLinux
CFLAGS		+= -O0 -g
endif

#---------------------------------------------------------------------
ifeq "$(BUILD_CONF)" "armlinux_release"
TOOL_CONFIG = ARMLinux
CFLAGS	 	= -O3 -D_NDEBUG -pthread
endif

#---------------------------------------------------------------------
# tool settings
#---------------------------------------------------------------------

include $(MAKEINC)/tools.mk
ifeq "$(TOOL_CONFIG)" "CONFIG_NAME"
# you can add your additional tools configuration here
endif
