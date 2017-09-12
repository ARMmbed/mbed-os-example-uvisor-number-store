###########################################################################
#
#  Copyright (c) 2013-2015, ARM Limited, All Rights Reserved
#  SPDX-License-Identifier: Apache-2.0
#
#  Licensed under the Apache License, Version 2.0 (the "License"); you may
#  not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################
# App
APP:=$(notdir $(shell 'pwd'))

# Toolchain
PREFIX:=arm-none-eabi-
GDB:=$(PREFIX)gdb
OBJDUMP:=$(PREFIX)objdump

MBED_TARGET:=K64F
#MBED_TARGET:=EFM32GG_STK3700
#MBED_TARGET:=ARM_MPS2_ARMv8MML
MBED_TOOLCHAIN:=GCC_ARM

UVISOR_LIB:=mbed-os/features/FEATURE_UVISOR

# set JLINK CPU based on ARCH
ifneq (,$(findstring K64F,$(MBED_TARGET)))
	CPU:=MK64FN1M0XXX12
	UVISOR_PLATFORM:=kinetis
	UVISOR_ELF:=configuration_kinetis_cortex_m4_0x1fff0000.elf
	DEBUG_HOST:=localhost:2331
endif
ifneq (,$(findstring EFM32GG_STK3700,$(MBED_TARGET)))
	CPU:=EFM32GG990F1024
	UVISOR_PLATFORM:=efm32
	UVISOR_ELF:=configuration_efm32_cortex_m3_p1.elf
	DEBUG_HOST:=localhost:2331
endif
ifneq (,$(findstring ARM_MPS2_ARMv8MML,$(MBED_TARGET)))
	UVISOR_PLATFORM:=armv8mml
	UVISOR_ELF:=configuration_armv8mml_cortex_m81.elf
	DEBUG_HOST=$(shell docker ps | grep "${DOCKER_IMAGE}" | grep -oh '[0-9]\+\.[0-9]\+\.[0-9]\+\.[0-9]\+\:[0-9]\+')
endif

# JLink settings
JLINK:=$(SEGGER)JLinkExe
JLINK_CFG:=-Device $(CPU) -if SWD
JLINK_SERVER:=$(SEGGER)JLinkGDBServer

# Paths
TARGET:=./BUILD/$(MBED_TARGET)/$(MBED_TOOLCHAIN)/$(APP)
TARGET_ELF:=$(TARGET).elf
TARGET_BIN:=$(TARGET).bin
TARGET_ASM:=$(TARGET).asm
DEBUG_ELF:=debug.elf

# detect SEGGER JLINK mass storages
SEGGER_DET:=Segger.html
FW_DIR:=$(wildcard /Volumes/JLINK/$(FW_DET))
ifeq ("$(wildcard $(SEGGER_DIR))","")
	FW_DIR:=$(wildcard /run/media/$(USER)/JLINK/$(FW_DET))
endif
ifeq ("$(wildcard $(FW_DIR))","")
	FW_DIR:=$(wildcard /var/run/media/$(USER)/JLINK/$(FW_DET))
endif

# detect mbed DAPLINK mass storages
MBED_DET:=MBED.HTM
ifeq ("$(wildcard $(FW_DIR))","")
	FW_DIR:=$(wildcard /Volumes/*/$(MBED_DET))
endif
ifeq ("$(wildcard $(FW_DIR))","")
	FW_DIR:=$(wildcard /run/media/$(USER)/*/$(MBED_DET))
endif
ifeq ("$(wildcard $(FW_DIR))","")
	FW_DIR:=$(wildcard /var/run/media/$(USER)/*/$(MBED_DET))
endif
ifeq ("$(wildcard $(FW_DIR))","")
	FW_DIR:=./
else
	FW_DIR:=$(dir $(firstword $(FW_DIR)))
endif

# Read uVisor symbols.
GDB_DEBUG_UVISOR=add-symbol-file $(DEBUG_ELF) __UVISOR_FLASH_START

# GDB scripts
include Makefile.scripts

.PHONY: all clean uvisor_clean debug release uvisor debug gdbserver gdb.script

all: release install

clean:
	rm -rf BUILD gdb.script $(DEBUG_ELF) firmware.asm firmware.bin

install: $(TARGET_BIN)
	-cp $^ $(FW_DIR)firmware.bin
	sync

debug:
	cp $(UVISOR_LIB)/importer/TARGET_IGNORE/uvisor/platform/$(UVISOR_PLATFORM)/debug/$(UVISOR_ELF) $(DEBUG_ELF)
	mbed compile -t $(MBED_TOOLCHAIN) -m $(MBED_TARGET) -j 0 $(NEO) --profile mbed-os/tools/profiles/debug.json

release:
	cp $(UVISOR_LIB)/importer/TARGET_IGNORE/uvisor/platform/$(UVISOR_PLATFORM)/release/$(UVISOR_ELF) $(DEBUG_ELF)
	mbed compile -t $(MBED_TOOLCHAIN) -m $(MBED_TARGET) -j 0 $(NEO)

objdump: $(TARGET_ASM)

$(TARGET_ASM): $(TARGET_ELF)
	$(OBJDUMP) -d $^ > $@

#	$(JLINK_SERVER) $(JLINK_CFG)
#	simulate -d FVP_MPS2_Cortex-M33 $(TARGET_ELF)
gdbserver:
	$(JLINK_SERVER) $(JLINK_CFG)

simtrace:
	simulate -dt FVP_MPS2_Cortex-M33 $(TARGET_ELF)

trace:
	simulate -t FVP_MPS2_Cortex-M33 $(TARGET_ELF)

uvisor:
	make -C $(UVISOR_LIB) clean all

uvisor_clean:
	make -C $(UVISOR_LIB) clean
	 git -C $(UVISOR_LIB) reset --hard

gdb: gdb.script
	$(GDB) -tui -x $<

gdb.script:
	@echo "$$__SCRIPT_GDB" > $@
