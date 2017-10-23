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
#  Project name         : NONE
#  Module name          : ONVIF event daemon application
#  Module description   : ONVIF event daemon application
#  Author               : Louis
#  Created at           : 2010/02/09
#  $History:$
#
#---------------------------------------------------------------------

# environment settings
include $(MAKEINC)/default.mk

include config.mk
include $(MAKEINC)/apps.mk

# install variables
package_bin_dir = /usr/bin
host_package_bin_dir=${ROOTFSDIR}${package_bin_dir}
INSTALLED_APP := $(host_package_bin_dir)/$(MODNAME).$(EXESUFFIX)
NEW_APP := $(host_package_bin_dir)/$(MODNAME)

# Targets
all: $(LIBS) $(CUSTOMLIBS) app
	
install:
	echo $(APPTARGET)
	install $(APPTARGET) $(host_package_bin_dir)
	$(MV) $(INSTALLED_APP) $(NEW_APP)
	$(STRIP) $(NEW_APP)	
