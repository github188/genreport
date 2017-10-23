include $(MAKCOMM_PATH)/define.mk
include $(PREBUILD_FOLDER)/prebuildlib.mk

MODULE_NAME := genreport
ifneq ($(PLATFORM_SOC),dm8127)
LINK_TYPE := shared
endif

CPPFLAGS:=
CFLAGS += $(PREBUILD_INC)
CFLAGS += -I../inc
CFLAGS += -I$(_WORK_PATH)/commlib/common/inc

DEPENDENCY_LIB := $(PREBUILD_LIBS) 

SRC_PATH = ../src
SRCS := $(wildcard $(SRC_PATH)/*.c)

all: $(MODULE_NAME) 

.PHONY : cleanall
cleanall:
	@echo "\t[CLEAN]\t"
	rm -Rf *.[o] *.map $(MODULE_NAME) $(MODULE_NAME).out $(TMP) 
					
include $(MAKCOMM_PATH)/apps.mk

