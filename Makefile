CC = mpicc
CXX = mpic++

##################################################################

MAKE = make
AR = ar qcv
RANLIB = ranlib
RM = rm -rf
LN = ln -s
LNDIR = ln -s
DIFF = diff
OEXT = o
LEXT = a
EEXT =
LDFLAGS = -lconfig++
CFLAGS = -g -O3 -DKITFOX_DEBUG_ -DKITFOX_MPI_DEBUG_ -I./ 

##################################################################

DRAMSIM_DIR=DRAMSim2-2.2.2
DRAMSIM_TAR=v2.2.2.tar.gz

DSENT_DIR=dsent0.91
DSENT_TAR=dsent0.91.zip

INTSIM_DIR=intsim_alpha
INTSIM_TAR=intsim_alpha.tar.gz

MCPAT_DIR=McPAT08release
MCPAT_TAR=mcpat0.8_r274.tar.gz

TSV_DIR=tsv_v1.0
TSV_TAR=tsv_v1.0.tar.gz

TDICE_DIR=3d-ice
TDICE_TAR=3d-ice-2.1.zip
SUPERLU_DIR=SuperLU_4.3
SUPERLU_TAR=superlu_4.3.tar.gz

HOTSPOT_DIR=HotSpot-5.02
HOTSPOT_TAR=HotSpot-5.02.tar.gz

MICROFLUIDICS_DIR=Microfluidics
MICROFLUIDICS_TAR=Microfluidics.tar.gz

FAILURE_DIR=failure_v1.0
FAILURE_TAR=failure_v1.0.tar.gz

##################################################################

# SOURCES
SRCS = \
queue.cc \
library.cc \
component.cc \
configuration.cc \
threader.cc \
communicator/kitfox-server.cc \
kitfox.cc

# HEADERS
HDRS = \
kitfox-defs.h \
queue.h \
library.h \
component.h \
configuration.h \
threader.h \
communicator/kitfox-net.h \
communicator/kitfox-server.h \
communicator/kitfox-client.h \
kitfox.h

# MAIN DIRECTORY OBJECTS
SRC_OBJS = \
queue.$(OEXT) \
library.$(OEXT) \
component.$(OEXT) \
configuration.$(OEXT) \
threader.$(OEXT)

# KITFOX OBJECT
KITFOX_OBJS = kitfox.$(OEXT)

# SERVER OBJECTS
KITFOX_SRV_OBJS = \
communicator/kitfox-server.$(OEXT)

# LIBRARY OBJECTS
	LIBRARY_LDFLAGS = -L./
ifneq ($(wildcard libdramsim.a),)
	ENERGYLIB_DRAMSIM_LDFLAGS += -ldramsim
	ENERGYLIB_DRAMSIM_CFLAGS += -Ilibrary/energylib_dramsim -Ilibrary/energylib_dramsim/$(DRAMSIM_DIR) -DENERGYLIB_DRAMSIM_I
	ENERGYLIB_DRAMSIM_OBJS += library/energylib_dramsim/energylib_dramsim.o
	ENERGYLIB_DRAMSIM_HDRS += library/energylib_dramsim/energylib_dramsim.h
	ENERGYLIB_DRAMSIM_SRCS += library/energylib_dramsim/energylib_dramsim.cc
	ENERGYLIB_DRAMSIM_ARCS += libdramsim.a
	LIBRARY_LDFLAGS += $(ENERGYLIB_DRAMSIM_LDFLAGS)
	LIBRARY_CFLAGS += $(ENERGYLIB_DRAMSIM_CFLAGS)
	LIBRARY_OBJS += $(ENERGYLIB_DRAMSIM_OBJS)
	LIBRARY_HDRS += $(ENERGYLIB_DRAMSIM_HDRS)
	LIBRARY_SRCS += $(ENERGYLIB_DRAMSIM_SRCS)
	LIBRARY_ARCS += $(ENERGYLIB_DRAMSIM_ARCS)
endif
ifneq ($(wildcard libdsent.a),)
	ENERGYLIB_DSENT_LDFLAGS += -ldsent
	ENERGYLIB_DSENT_CFLAGS += -Ilibrary/energylib_dsent -Ilibrary/energylib_dsent/$(DSENT_DIR) -Ilibrary/energylib_dsent/$(DSENT_DIR)/libutil -Ilibrary/energylib_dsent/$(DSENT_DIR)/model -Ilibrary/energylib_dsent/$(DSENT_DIR)/tech -Ilibrary/energylib_dsent/$(DSENT_DIR)/util -DENERGYLIB_DSENT_I
	ENERGYLIB_DSENT_OBJS += library/energylib_dsent/energylib_dsent.o
	ENERGYLIB_DSENT_HDRS += library/energylib_dsent/energylib_dsent.h
	ENERGYLIB_DSENT_SRCS += library/energylib_dsent/energylib_dsent.cc
	ENERGYLIB_DSENT_ARCS += libdsent.a
	LIBRARY_LDFLAGS += $(ENERGYLIB_DSENT_LDFLAGS)
	LIBRARY_CFLAGS += $(ENERGYLIB_DSENT_CFLAGS)
	LIBRARY_OBJS += $(ENERGYLIB_DSENT_OBJS)
	LIBRARY_HDRS += $(ENERGYLIB_DSENT_HDRS)
	LIBRARY_SRCS += $(ENERGYLIB_DSENT_SRCS)
	LIBRARY_ARCS += $(ENERGYLIB_DSENT_ARCS)
endif
ifneq ($(wildcard libintsim.a),)
	ENERGYLIB_INTSIM_LDFLAGS += -lintsim
	ENERGYLIB_INTSIM_CFLAGS += -Ilibrary/energylib_intsim -Ilibrary/energylib_intsim/$(INTSIM_DIR) -DENERGYLIB_INTSIM_I
	ENERGYLIB_INTSIM_OBJS += library/energylib_intsim/energylib_intsim.o
	ENERGYLIB_INTSIM_HDRS += library/energylib_intsim/energylib_intsim.h
	ENERGYLIB_INTSIM_SRCS += library/energylib_intsim/energylib_intsim.cc
	ENERGYLIB_INTSIM_ARCS += libintsim.a
	LIBRARY_LDFLAGS += $(ENERGYLIB_INTSIM_LDFLAGS)
	LIBRARY_CFLAGS += $(ENERGYLIB_INTSIM_CFLAGS)
	LIBRARY_OBJS += $(ENERGYLIB_INTSIM_OBJS)
	LIBRARY_HDRS += $(ENERGYLIB_INTSIM_HDRS)
	LIBRARY_SRCS += $(ENERGYLIB_INTSIM_SRCS)
	LIBRARY_ARCS += $(ENERGYLIB_INTSIM_ARCS)
endif
ifneq ($(wildcard libmcpat.a),)
	ENERGYLIB_MCPAT_LDFLAGS += -lmcpat -pthread
	ENERGYLIB_MCPAT_CFLAGS += -Ilibrary/energylib_mcpat -Ilibrary/energylib_mcpat/$(MCPAT_DIR) -Ilibrary/energylib_mcpat/$(MCPAT_DIR)/cacti -DENERGYLIB_MCPAT_I
	ENERGYLIB_MCPAT_OBJS += library/energylib_mcpat/energylib_mcpat.o
	ENERGYLIB_MCPAT_HDRS += library/energylib_mcpat/energylib_mcpat.h
	ENERGYLIB_MCPAT_SRCS += library/energylib_mcpat/energylib_mcpat.cc
	ENERGYLIB_MCPAT_ARCS += libmcpat.a
	LIBRARY_LDFLAGS += $(ENERGYLIB_MCPAT_LDFLAGS)
	LIBRARY_CFLAGS += $(ENERGYLIB_MCPAT_CFLAGS)
	LIBRARY_OBJS += $(ENERGYLIB_MCPAT_OBJS)
	LIBRARY_HDRS += $(ENERGYLIB_MCPAT_HDRS)
	LIBRARY_SRCS += $(ENERGYLIB_MCPAT_SRCS)
	LIBRARY_ARCS += $(ENERGYLIB_MCPAT_ARCS)
endif
ifneq ($(wildcard libtsv.a),)
	ENERGYLIB_TSV_LDFLAGS += -ltsv
	ENERGYLIB_TSV_CFLAGS += -Ilibrary/energylib_tsv -Ilibrary/energylib_tsv/$(TSV_DIR) -DENERGYLIB_TSV_I
	ENERGYLIB_TSV_OBJS += library/energylib_tsv/energylib_tsv.o
	ENERGYLIB_TSV_HDRS += library/energylib_tsv/energylib_tsv.h
	ENERGYLIB_TSV_SRCS += library/energylib_tsv/energylib_tsv.cc
	ENERGYLIB_TSV_ARCS += libtsv.a
	LIBRARY_LDFLAGS += $(ENERGYLIB_TSV_LDFLAGS)
	LIBRARY_CFLAGS += $(ENERGYLIB_TSV_CFLAGS)
	LIBRARY_OBJS += $(ENERGYLIB_TSV_OBJS)
	LIBRARY_HDRS += $(ENERGYLIB_TSV_HDRS)
	LIBRARY_SRCS += $(ENERGYLIB_TSV_SRCS)
	LIBRARY_ARCS += $(ENERGYLIB_TSV_ARCS)
endif
ifneq ($(wildcard lib3dice.a),)
	THERMALLIB_3DICE_LDFLAGS += -l3dice
	THERMALLIB_3DICE_CFLAGS += -Ilibrary/thermallib_3dice -Ilibrary/thermallib_3dice/$(TDICE_DIR)/sources -Ilibrary/thermallib_3dice/$(TDICE_DIR)/bison -Ilibrary/thermallib_3dice/$(TDICE_DIR)/flex -Ilibrary/thermallib_3dice/$(SUPERLU_DIR)/SRC -DTHERMALLIB_3DICE_I
	THERMALLIB_3DICE_OBJS += library/thermallib_3dice/thermallib_3dice.o
	THERMALLIB_3DICE_HDRS += library/thermallib_3dice/thermallib_3dice.h
	THERMALLIB_3DICE_SRCS += library/thermallib_3dice/thermallib_3dice.cc
	THERMALLIB_3DICE_ARCS += lib3dice.a
	LIBRARY_LDFLAGS += $(THERMALLIB_3DICE_LDFLAGS)
	LIBRARY_CFLAGS += $(THERMALLIB_3DICE_CFLAGS)
	LIBRARY_OBJS += $(THERMALLIB_3DICE_OBJS)
	LIBRARY_HDRS += $(THERMALLIB_3DICE_HDRS)
	LIBRARY_SRCS += $(THERMALLIB_3DICE_SRCS)
	LIBRARY_ARCS += $(THERMALLIB_3DICE_ARCS)
endif
ifneq ($(wildcard libhotspot.a),)
	THERMALLIB_HOTSPOT_LDFLAGS += -lhotspot
	THERMALLIB_HOTSPOT_CFLAGS += -Ilibrary/thermallib_hotspot -Ilibrary/thermallib_hotspot/$(HOTSPOT_DIR) -DTHERMALLIB_HOTSPOT_I
	THERMALLIB_HOTSPOT_OBJS += library/thermallib_hotspot/thermallib_hotspot.o
	THERMALLIB_HOTSPOT_HDRS += library/thermallib_hotspot/thermallib_hotspot.h
	THERMALLIB_HOTSPOT_SRCS += library/thermallib_hotspot/thermallib_hotspot.cc
	THERMALLIB_HOTSPOT_ARCS += libhotspot.a
	LIBRARY_LDFLAGS += $(THERMALLIB_HOTSPOT_LDFLAGS)
	LIBRARY_CFLAGS += $(THERMALLIB_HOTSPOT_CFLAGS)
	LIBRARY_OBJS += $(THERMALLIB_HOTSPOT_OBJS)
	LIBRARY_HDRS += $(THERMALLIB_HOTSPOT_HDRS)
	LIBRARY_SRCS += $(THERMALLIB_HOTSPOT_SRCS)
	LIBRARY_ARCS += $(THERMALLIB_HOTSPOT_ARCS)
endif
ifneq ($(wildcard libmicrofluidics.a),)
	THERMALLIB_MICROFLUIDICS_LDFLAGS += -lmicrofluidics
	THERMALLIB_MICROFLUIDICS_CFLAGS += -Ilibrary/thermallib_microfluidics -Ilibrary/thermallib_microfluidics/$(MICROFLUIDICS_DIR) -DTHERMALLIB_MICROFLUIDICS_I
	THERMALLIB_MICROFLUIDICS_OBJS += library/thermallib_microfluidics/thermallib_microfluidics.o
	THERMALLIB_MICROFLUIDICS_HDRS += library/thermallib_microfluidics/thermallib_microfluidics.h
	THERMALLIB_MICROFLUIDICS_SRCS += library/thermallib_microfluidics/thermallib_microfluidics.cc
	THERMALLIB_MICROFLUIDICS_ARCS += libmicrofluidics.a
	LIBRARY_LDFLAGS += $(THERMALLIB_MICROFLUIDICS_LDFLAGS)
	LIBRARY_CFLAGS += $(THERMALLIB_MICROFLUIDICS_CFLAGS)
	LIBRARY_OBJS += $(THERMALLIB_MICROFLUIDICS_OBJS)
	LIBRARY_HDRS += $(THERMALLIB_MICROFLUIDICS_HDRS)
	LIBRARY_SRCS += $(THERMALLIB_MICROFLUIDICS_SRCS)
	LIBRARY_ARCS += $(THERMALLIB_MICROFLUIDICS_ARCS)
endif
ifneq ($(wildcard libfailure.a),)
	RELIABILITYLIB_FAILURE_LDFLAGS += -lfailure
	RELIABILITYLIB_FAILURE_CFLAGS += -Ilibrary/reliabilitylib_failure -Ilibrary/reliabilitylib_failure/$(FAILURE_DIR) -DRELIABILITYLIB_FAILURE_I
	RELIABILITYLIB_FAILURE_OBJS += library/reliabilitylib_failure/reliabilitylib_failure.o
	RELIABILITYLIB_FAILURE_HDRS += library/reliabilitylib_failure/reliabilitylib_failure.h
	RELIABILITYLIB_FAILURE_SRCS += library/reliabilitylib_failure/reliabilitylib_failure.cc
	RELIABILITYLIB_FAILURE_ARCS += libfailure.a
	LIBRARY_LDFLAGS += $(RELIABILITYLIB_FAILURE_LDFLAGS)
	LIBRARY_CFLAGS += $(RELIABILITYLIB_FAILURE_CFLAGS)
	LIBRARY_OBJS += $(RELIABILITYLIB_FAILURE_OBJS)
	LIBRARY_HDRS += $(RELIABILITYLIB_FAILURE_HDRS)
	LIBRARY_SRCS += $(RELIABILITYLIB_FAILURE_SRCS)
	LIBRARY_ARCS += $(RELIABILITYLIB_FAILURE_ARCS)
endif

.PHONY: doc all clean distclean

# DEFAULT
default: kitfox

# DOCUMENTATION
doc: 
	cd doc/; make

# MODELS
models: dramsim dsent intsim mcpat tsv 3dice hotspot microfluidics failure

# TEST
test:
	make kitfox
ifneq ($(ENERGYLIB_DRAMSIM_OBJS),)
	rm -f test-dramsim
	make test-dramsim
endif
ifneq ($(ENERGYLIB_DSENT_OBJS),)
	rm -f test-dsent
	make test-dsent
endif
ifneq ($(ENERGYLIB_INTSIM_OBJS),)
	rm -f test-intsim
	make test-intsim
endif
ifneq ($(ENERGYLIB_MCPAT_OBJS),)
	rm -f test-mcpat
	make test-mcpat
endif
ifneq ($(ENERGYLIB_TSV_OBJS),)
	rm -f test-tsv
	make test-tsv
endif
ifneq ($(THERMALLIB_3DICE_OBJS),)
	rm -f test-3dice
	make test-3dice
endif
ifneq ($(THERMALLIB_HOTSPOT_OBJS),)
	rm -f test-hotspot
	make test-hotspot
endif
ifneq ($(THERMALLIB_MICROFLUIDICS_OBJS),)
	rm -f test-microfluidics
	make test-microfluidics
endif
ifneq ($(RELIABILITYLIB_FAILURE_OBJS),)
	rm -f test-failure
	make test-failure
endif

ifneq ($(ENERGYLIB_DRAMSIM_OBJS),)
	@echo "\n===== Testing DRAMSIM v2.2.2 ====="
	./test-dramsim
	@echo "===== Testing DRAMSIM v2.2.2 completed =====\n"
endif
ifneq ($(ENERGYLIB_DSENT_OBJS),)
	@echo "\n===== Testing DSENT v0.91 ====="
	./test-dsent
	@echo "===== Testing DSENT v0.91 completed =====\n"
endif
ifneq ($(ENERGYLIB_INTSIM_OBJS),)
	@echo "\n===== Testing IntSim v1.0 ====="
	./test-intsim
	@echo "===== Testing IntSim v1.0 completed =====\n"
endif
ifneq ($(ENERGYLIB_MCPAT_OBJS),)
	@echo "\n===== Testing McPAT v0.8 ====="
	./test-mcpat
	@echo "===== Testing McPAT v0.8 completed =====\n"
endif
ifneq ($(ENERGYLIB_TSV_OBJS),)
	@echo "\n===== Testing TSV v1.0 ====="
	./test-tsv
	@echo "===== Testing TSV v1.0 completed =====\n"
endif
ifneq ($(THERMALLIB_3DICE_OBJS),)
	@echo "\n===== Testing 3D-ICE v2.1 ====="
	./test-3dice
	@echo "===== Testing 3D-ICE v2.1 completed =====\n"
endif
ifneq ($(THERMALLIB_HOTSPOT_OBJS),)
	@echo "\n===== Testing HotSpot v5.02 ====="
	./test-hotspot
	@echo "===== Testing HotSpot v5.02 completed =====\n"
endif
ifneq ($(THERMALLIB_MICROFLUIDICS_OBJS),)
	@echo "\n===== Testing Compact Microfluidics v1.0 ====="
	./test-microfluidics
	@echo "===== Testing Compact Microfluidics v1.0 completed =====\n"
endif
ifneq ($(RELIABILITYLIB_FAILURE_OBJS),)
	@echo "\n===== Testing Failure v1.0 ====="
	./test-failure
	@echo "===== Testing Failure v1.0 completed =====\n"
endif

# TESTER BINARIES
test-dramsim$(EEXT): test_main/test-dramsim.$(OEXT)
	$(CXX) -o test-dramsim$(EEXT) $(CFLAGS) $(LIBRARY_CFLAGS) $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS) test_main/test-dramsim.$(OEXT) $(LIBRARY_LDFLAGS) $(LDFLAGS)
	
test-dsent$(EEXT): test_main/test-dsent.$(OEXT)
	$(CXX) -o test-dsent$(EEXT) $(CFLAGS) $(LIBRARY_CFLAGS) $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS) test_main/test-dsent.$(OEXT) $(LIBRARY_LDFLAGS) $(LDFLAGS)
	
test-intsim$(EEXT): test_main/test-intsim.$(OEXT)
	$(CXX) -o test-intsim$(EEXT) $(CFLAGS) $(LIBRARY_CFLAGS) $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS) test_main/test-intsim.$(OEXT) $(LIBRARY_LDFLAGS) $(LDFLAGS)

test-mcpat$(EEXT): test_main/test-mcpat.$(OEXT)
	$(CXX) -o test-mcpat$(EEXT) $(CFLAGS) $(LIBRARY_CFLAGS) $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS) test_main/test-mcpat.$(OEXT) $(LIBRARY_LDFLAGS) $(LDFLAGS)
	
test-tsv$(EEXT): test_main/test-tsv.$(OEXT)
	$(CXX) -o test-tsv$(EEXT) $(CFLAGS) $(LIBRARY_CFLAGS) $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS) test_main/test-tsv.$(OEXT) $(LIBRARY_LDFLAGS) $(LDFLAGS)
	
test-3dice$(EEXT): test_main/test-3dice.$(OEXT)
	$(CXX) -o test-3dice$(EEXT) $(CFLAGS) $(LIBRARY_CFLAGS) $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS) test_main/test-3dice.$(OEXT) $(LIBRARY_LDFLAGS) $(LDFLAGS)

test-hotspot$(EEXT): test_main/test-hotspot.$(OEXT)
	$(CXX) -o test-hotspot$(EEXT) $(CFLAGS) $(LIBRARY_CFLAGS) $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS) test_main/test-hotspot.$(OEXT) $(LIBRARY_LDFLAGS) $(LDFLAGS)
	
test-microfluidics$(EEXT): test_main/test-microfluidics.$(OEXT)
	$(CXX) -o test-microfluidics$(EEXT) $(CFLAGS) $(LIBRARY_CFLAGS) $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS) test_main/test-microfluidics.$(OEXT) $(LIBRARY_LDFLAGS) $(LDFLAGS)
	
test-failure$(EEXT): test_main/test-failure.$(OEXT)
	$(CXX) -o test-failure$(EEXT) $(CFLAGS) $(LIBRARY_CFLAGS) $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS) test_main/test-failure.$(OEXT) $(LIBRARY_LDFLAGS) $(LDFLAGS)

# TESTER OBJS
test_main/test.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test.$(OEXT) -c test_main/test.cc

test_main/test-dramsim.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test-dramsim.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test-dramsim.$(OEXT) -c test_main/test-dramsim.cc
	
test_main/test-dsent.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test-dsent.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test-dsent.$(OEXT) -c test_main/test-dsent.cc
	
test_main/test-intsim.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test-intsim.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test-intsim.$(OEXT) -c test_main/test-intsim.cc

test_main/test-mcpat.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test-mcpat.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test-mcpat.$(OEXT) -c test_main/test-mcpat.cc

test_main/test-tsv.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test-tsv.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test-tsv.$(OEXT) -c test_main/test-tsv.cc

test_main/test-3dice.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test-3dice.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test-3dice.$(OEXT) -c test_main/test-3dice.cc
		
test_main/test-hotspot.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test-hotspot.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test-hotspot.$(OEXT) -c test_main/test-hotspot.cc

test_main/test-microfluidics.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test-microfluidics.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test-microfluidics.$(OEXT) -c test_main/test-microfluidics.cc
    
test_main/test-failure.$(OEXT): $(KITFOX_OBJS) $(LIBRARY_OBJS) $(LIBRARY_ARCS) test_main/test-failure.cc
	$(CXX) $(CFLAGS) -I$(CURDIR) $(LIBRARY_CFLAGS) -o test_main/test-failure.$(OEXT) -c test_main/test-failure.cc

# LIBRARY OBJECTS
library/energylib_dramsim/energylib_dramsim.$(OEXT): $(ENERGYLIB_DRAMSIM_HDRS) $(ENERGYLIB_DRAMSIM_SRCS) $(SRC_OBJS) $(LIBRARY_ARCS)
	$(CXX) $(CFLAGS) $(ENERGYLIB_DRAMSIM_CFLAGS) -o $*.$(OEXT) -c $*.cc
	
library/energylib_dsent/energylib_dsent.$(OEXT): $(ENERGYLIB_DSENT_HDRS) $(ENERGYLIB_DSENT_SRCS) $(SRC_OBJS) $(LIBRARY_ARCS)
	$(CXX) $(CFLAGS) $(ENERGYLIB_DSENT_CFLAGS) -o $*.$(OEXT) -c $*.cc
	
library/energylib_intsim/energylib_intsim.$(OEXT): $(ENERGYLIB_INTSIM_HDRS) $(ENERGYLIB_INTSIM_SRCS) library/energylib_intsim/$(INTSIM_DIR)/libintsim.a $(SRC_OBJS)
	$(CXX) $(CFLAGS) $(ENERGYLIB_INTSIM_CFLAGS) -o $*.$(OEXT) -c $*.cc

library/energylib_mcpat/energylib_mcpat.$(OEXT): $(ENERGYLIB_MCPAT_HDRS) $(ENERGYLIB_MCPAT_SRCS) $(SRC_OBJS) $(LIBRARY_ARCS)
	$(CXX) $(CFLAGS) $(ENERGYLIB_MCPAT_CFLAGS) -o $*.$(OEXT) -c $*.cc
	
library/energylib_tsv/energylib_tsv.$(OEXT): $(ENERGYLIB_TSV_HDRS) $(ENERGYLIB_TSV_SRCS) $(SRC_OBJS) $(LIBRARY_ARCS)
	$(CXX) $(CFLAGS) $(ENERGYLIB_TSV_CFLAGS) -o $*.$(OEXT) -c $*.cc
	
library/thermallib_3dice/thermallib_3dice.$(OEXT): $(THERMALLIB_3DICE_HDRS) $(THERMALLIB_3DICE_SRCS) $(SRC_OBJS) $(LIBRARY_ARCS)
	$(CXX) $(CFLAGS) $(THERMALLIB_3DICE_CFLAGS) -o $*.$(OEXT) -c $*.cc

library/thermallib_hotspot/thermallib_hotspot.$(OEXT): $(THERMALLIB_HOTSPOT_HDRS) $(THERMALLIB_HOTSPOT_SRCS) $(SRC_OBJS) $(LIBRARY_ARCS)
	$(CXX) $(CFLAGS) $(THERMALLIB_HOTSPOT_CFLAGS) -o $*.$(OEXT) -c $*.cc

library/thermallib_hotspot/thermallib_microfluidics.$(OEXT): $(THERMALLIB_MICROFLUIDICS_HDRS) $(THERMALLIB_MICROFLUIDICS_SRCS) $(SRC_OBJS) $(LIBRARY_ARCS)
	$(CXX) $(CFLAGS) $(THERMALLIB_MICROFLUIDICS_CFLAGS) -o $*.$(OEXT) -c $*.cc
	
library/reliabilitylib_failure/reliabilitylib_failure.$(OEXT): $(RELIABILITYLIB_FAILURE_HDRS) $(RELIABILITYLIB_FAILURE_SRCS) $(SRC_OBJS) $(LIBRARY_ARCS)
	$(CXX) $(CFLAGS) $(RELIABILITYLIB_FAILURE_CFLAGS) -o $*.$(OEXT) -c $*.cc

# SERVER OBJECTS
communicator/ei-server.$(OEXT): $(HDRS) $(SRCS)
	$(CXX) $(CFLAGS) -o $*.$(OEXT) -c $*.cc

# KITFOX OBJECT
kitfox.$(OEXT): $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(LIBRARY_OBJS) $(HDRS) $(SRCS)
	$(CXX) $(CFLAGS) $(LIBRARY_CFLAGS) -o kitfox.$(OEXT) -c kitfox.cc

.cc.$(OEXT): $(HDRS) $(SRCS)
	$(CXX) $(CFLAGS) -o $*.$(OEXT) -c $*.cc

KitFox:
	make kitfox
    
kitfox: $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS)
	$(RM) libKitFox.a
	ar qcv libKitFox.a $(LIBRARY_OBJS) $(SRC_OBJS) $(KITFOX_SRV_OBJS) $(KITFOX_OBJS)
	ranlib libKitFox.a
    
# BUILD EXTERNAL LIBRARIES
DRAMSim:
	make dramsim
dramsim:
	mkdir -p library/energylib_dramsim/tarball
ifneq ($(wildcard library/energylib_dramsim/$(DRAMSIM_DIR)),)
ifneq ($(wildcard library/energylib_dramsim/$(DRAMSIM_DIR)/$(DRAMSIM_DIR).patch),)
	$(RM) library/energylib_dramsim/$(DRAMSIM_DIR)
	make dramsim
else
	#cd library/energylib_dramsim/$(DRAMSIM_DIR)/; cp ../modified/Makefile ./;
	#cd library/energylib_dramsim/$(DRAMSIM_DIR)/; cp ../modified/*.h ./;
	cd library/energylib_dramsim/$(DRAMSIM_DIR)/; patch -fp1 <../patch/$(DRAMSIM_DIR).patch
	cd library/energylib_dramsim/$(DRAMSIM_DIR)/; make clean
	cd library/energylib_dramsim/$(DRAMSIM_DIR)/; make lib
	cd library/energylib_dramsim/$(DRAMSIM_DIR)/; cp ../patch/$(DRAMSIM_DIR).patch ./
	$(RM) libdramsim libdramsim.a; mkdir libdramsim
	cd libdramsim; ar x ../library/energylib_dramsim/$(DRAMSIM_DIR)/libdramsim.a
	cd libdramsim; ar qcv libdramsim.a *.o; ranlib libdramsim.a; mv libdramsim.a ../
	$(RM) libdramsim
endif
else
ifneq ($(wildcard library/energylib_dramsim/tarball/$(DRAMSIM_TAR)),)
	cd library/energylib_dramsim/; tar xf tarball/$(DRAMSIM_TAR)
	make dramsim
else
	cd library/energylib_dramsim/tarball; wget https://github.com/dramninjasUMD/DRAMSim2/archive/$(DRAMSIM_TAR)
	make dramsim
endif
endif

DSENT:
	make dsent
dsent:
	mkdir -p library/energylib_dsent/tarball
ifneq ($(wildcard library/energylib_dsent/$(DSENT_DIR)),)
ifneq ($(wildcard library/energylib_dsent/$(DSENT_DIR)/$(DSENT_DIR).patch),)
	$(RM) library/energylib_dsent/$(DSENT_DIR)
	make dsent
else
	#cd library/energylib_dsent/$(DSENT_DIR)/; cp ../modified/Makefile ./;
	#cd library/energylib_dsent/$(DSENT_DIR)/; cp ../modified/*.h ./; cp ../modified/*.cc ./
	#cd library/energylib_dsent/$(DSENT_DIR)/libutil/; rm *.h *.cc; cp ../../modified/libutil/*.h ./; cp ../../modified/libutil/*.cc ./
	#cd library/energylib_dsent/$(DSENT_DIR)/model/; cp ../../modified/model/*.h ./; cp ../../modified/model/*.cc ./
	#cd library/energylib_dsent/$(DSENT_DIR)/model/electrical/; cp ../../../modified/model/electrical/*.cc ./
	#cd library/energylib_dsent/$(DSENT_DIR)/model/electrical/router/; cp ../../../../modified/model/electrical/router/*.cc ./
	#cd library/energylib_dsent/$(DSENT_DIR)/tech/; cp ../../modified/tech/*.h ./;
	#cd library/energylib_dsent/$(DSENT_DIR)/util/; cp ../../modified/util/*.h ./; cp ../../modified/util/*.cc ./
	cd library/energylib_dsent/$(DSENT_DIR)/; patch -fp1 <../patch/$(DSENT_DIR).patch
	cd library/energylib_dsent/$(DSENT_DIR)/; make clean
	cd library/energylib_dsent/$(DSENT_DIR)/; make lib
	cd library/energylib_dsent/$(DSENT_DIR)/; cp ../patch/$(DSENT_DIR).patch ./
	$(RM) libdsent libdsent.a; mkdir libdsent
	cd libdsent; ar x ../library/energylib_dsent/$(DSENT_DIR)/libdsent.a
	cd libdsent; ar x ../library/energylib_dsent/$(DSENT_DIR)/libutil/libutil.a
	cd libdsent; ar qcv libdsent.a *.o; ranlib libdsent.a; mv libdsent.a ../
	$(RM) libdsent
endif
else
ifneq ($(wildcard library/energylib_dsent/tarball/$(DSENT_TAR)),)
	cd library/energylib_dsent/; unzip tarball/$(DSENT_TAR)
	make dsent
else
	@echo "ERROR: library/energylib_dsent/tarball/$(DSENT_TAR) not found"
endif
endif

intsim:
ifneq ($(wildcard library/energylib_intsim/$(INTSIM_DIR)),)
	cd library/energylib_intsim/$(INTSIM_DIR)/; make clean
	cd library/energylib_intsim/$(INTSIM_DIR)/; make lib
	$(RM) libintsim libintsim.a; mkdir libintsim
	cd libintsim; ar x ../library/energylib_intsim/$(INTSIM_DIR)/libintsim.a
	cd libintsim; ar qcv libintsim.a *.o; ranlib libintsim.a; mv libintsim.a ../
	$(RM) libintsim
else
ifneq ($(wildcard library/energylib_intsim/tarball/$(INTSIM_TAR)),)
	cd library/energylib_intsim/; tar xf tarball/$(INTSIM_TAR)
	make intsim
else
	@echo "ERROR: library/energylib_intsim/tarball/$(INTSIM_TAR) not found"
endif
endif

McPAT:
	make mcpat
mcpat:
	mkdir -p library/energylib_mcpat/tarball
ifneq ($(wildcard library/energylib_mcpat/$(MCPAT_DIR)),)
ifneq ($(wildcard library/energylib_mcpat/$(MCPAT_DIR)/$(MCPAT_DIR).patch),)
	$(RM) library/energylib_mcpat/$(MCPAT_DIR)
	make mcpat
else
	#cd library/energylib_mcpat/$(MCPAT_DIR)/; cp ../patch/makefile ./; cp ../patch/mcpat.mk ./
	#cd library/energylib_mcpat/$(MCPAT_DIR)/; cp ../patch/*.cc ./; cp ../patch/*.h ./
	#cd library/energylib_mcpat/$(MCPAT_DIR)/cacti/; cp ../../patch/cacti/*.cc ./; cp ../../patch/cacti/*.h ./
	cd library/energylib_mcpat/$(MCPAT_DIR)/; patch -fp1 <../patch/$(MCPAT_DIR).patch
	cd library/energylib_mcpat/$(MCPAT_DIR)/; make clean    
	cd library/energylib_mcpat/$(MCPAT_DIR)/; make lib
	cd library/energylib_mcpat/$(MCPAT_DIR)/; cp ../patch/$(MCPAT_DIR).patch ./
	$(RM) libmcpat libmcpat.a; mkdir libmcpat
	cd libmcpat; ar x ../library/energylib_mcpat/$(MCPAT_DIR)/libmcpat.a
	cd libmcpat; ar qcv libmcpat.a *.o; ranlib libmcpat.a; mv libmcpat.a ../
	$(RM) libmcpat
endif
else 
ifneq ($(wildcard library/energylib_mcpat/tarball/$(MCPAT_TAR)),)
	cd library/energylib_mcpat/; tar xf tarball/$(MCPAT_TAR)
	make mcpat
else
	cd library/energylib_mcpat/tarball; wget http://www.hpl.hp.com/research/mcpat/$(MCPAT_TAR)
	make mcpat
endif
endif

TSV:
	make tsv
tsv:
ifneq ($(wildcard library/energylib_tsv/$(TSV_DIR)),)
	cd library/energylib_tsv/$(TSV_DIR)/; make clean
	cd library/energylib_tsv/$(TSV_DIR)/; make lib
	$(RM) libtsv libtsv.a; mkdir libtsv
	cd libtsv; ar x ../library/energylib_tsv/$(TSV_DIR)/libtsv.a
	cd libtsv; ar qcv libtsv.a *.o; ranlib libtsv.a; mv libtsv.a ../
	$(RM) libtsv
else 
ifneq ($(wildcard library/energylib_tsv/tarball/$(TSV_TAR)),)
	cd library/energylib_tsv/; tar xf tarball/$(TSV_TAR)
	make tsv
else
	@echo "ERROR: library/energylib_tsv/tarball/$(TSV_TAR) not found"
endif
endif

3d-ice:
	make 3dice
3dice:
	mkdir -p library/thermallib_3dice/tarball
ifneq ($(wildcard library/thermallib_3dice/$(SUPERLU_DIR)),)
ifneq ($(wildcard library/thermallib_3dice/$(SUPERLU_DIR)/$(SUPERLU_DIR).patch),)
	$(RM) library/thermallib_3dice/$(SUPERLU_DIR)
	make 3dice
else
ifneq ($(wildcard library/thermallib_3dice/$(TDICE_DIR)),)
ifneq ($(wildcard library/thermallib_3dice/$(TDICE_DIR)/$(TDICE_DIR).patch),)
	$(RM) library/thermallib_3dice/$(TDICE_DIR)
	make 3dice
else
	#cd library/thermallib_3dice/$(SUPERLU_DIR)/; cp ../patch/make.inc ./;
	cd library/thermallib_3dice/$(SUPERLU_DIR)/; patch -fp1 <../patch/$(SUPERLU_DIR).patch
	cd library/thermallib_3dice/$(SUPERLU_DIR)/; make clean
	cd library/thermallib_3dice/$(SUPERLU_DIR)/; make blaslib; make superlulib
	cd library/thermallib_3dice/$(SUPERLU_DIR)/; cp ../patch/$(SUPERLU_DIR).patch ./
	#cd library/thermallib_3dice/$(TDICE_DIR)/; cp ../patch/makefile.def ./; cp ../patch/*.h include/; cp ../patch/*.c sources/;
	cd library/thermallib_3dice/$(TDICE_DIR)/; patch -fp1 <../patch/$(TDICE_DIR).patch
	cd library/thermallib_3dice/$(TDICE_DIR)/; make clean
	cd library/thermallib_3dice/$(TDICE_DIR)/; make lib
	cd library/thermallib_3dice/$(TDICE_DIR)/; cp ../patch/$(TDICE_DIR).patch ./
	$(RM) lib3dice lib3dice.a; mkdir lib3dice
	cd lib3dice; ar x ../library/thermallib_3dice/$(TDICE_DIR)/lib/libthreed-ice-2.1.a
	cd lib3dice; ar x ../library/thermallib_3dice/$(SUPERLU_DIR)/lib/libblas.a
	cd lib3dice; ar x ../library/thermallib_3dice/$(SUPERLU_DIR)/lib/libsuperlu_4.3.a
	cd lib3dice; ar qcv lib3dice.a *.o; ranlib lib3dice.a; mv lib3dice.a ../
	$(RM) lib3dice
endif
else 
ifneq ($(wildcard library/thermallib_3dice/tarball/$(TDICE_TAR)),)
	cd library/thermallib_3dice/; unzip tarball/$(TDICE_TAR)
	make 3dice
else
	cd library/thermallib_3dice/tarball; wget http://esl.epfl.ch/files/content/sites/esl/files/3dice/releases/$(TDICE_TAR)
	make 3dice
endif
endif
endif
else 
ifneq ($(wildcard library/thermallib_3dice/tarball/$(SUPERLU_TAR)),)
	cd library/thermallib_3dice/; tar xf tarball/$(SUPERLU_TAR)
	make 3dice
else
	cd library/thermallib_3dice/tarball; wget http://crd-legacy.lbl.gov/~xiaoye/SuperLU/$(SUPERLU_TAR)
	make 3dice
endif
endif

Hotspot:
	make hotspot
hotspot:
	mkdir -p library/thermallib_hotspot/tarball
ifneq ($(wildcard library/thermallib_hotspot/$(HOTSPOT_DIR)),)
ifneq ($(wildcard library/thermallib_hotspot/$(HOTSPOT_DIR)/$(HOTSPOT_DIR).patch),)
	$(RM) library/thermallib_hotspot/$(HOTSPOT_DIR)
	make hotspot
else
	#cd library/thermallib_hotspot/$(HOTSPOT_DIR)/; cp ../patch/* ./;
	cd library/thermallib_hotspot/$(HOTSPOT_DIR)/; patch -fp1 <../patch/$(HOTSPOT_DIR).patch
	cd library/thermallib_hotspot/$(HOTSPOT_DIR)/; make clean    
	cd library/thermallib_hotspot/$(HOTSPOT_DIR)/; make lib
	cd library/thermallib_hotspot/$(HOTSPOT_DIR)/; cp ../patch/$(HOTSPOT_DIR).patch ./
	$(RM) libhotspot libhotspot.a; mkdir libhotspot
	cd libhotspot; ar x ../library/thermallib_hotspot/$(HOTSPOT_DIR)/libhotspot.a
	cd libhotspot; ar qcv libhotspot.a *.o; ranlib libhotspot.a; mv libhotspot.a ../
	$(RM) libhotspot
endif
else 
ifneq ($(wildcard library/thermallib_hotspot/tarball/$(HOTSPOT_TAR)),)
	cd library/thermallib_hotspot/; tar xf tarball/$(HOTSPOT_TAR)
	make hotspot
else
	cd library/thermallib_hotspot/tarball; wget http://lava.cs.virginia.edu/HotSpot/grab/$(HOTSPOT_TAR)
	make hotspot
endif
endif

Microfluidics:
	make microfluidics
microfluidics:
ifneq ($(wildcard library/thermallib_microfluidics/$(MICROFLUIDICS_DIR)),)
	cd library/thermallib_microfluidics/$(MICROFLUIDICS_DIR); make clean
	cd library/thermallib_microfluidics/$(MICROFLUIDICS_DIR); make lib
	$(RM) libmicrofluidics libmicrofluidics.a; mkdir libmicrofluidics
	cd libmicrofluidics; ar x ../library/thermallib_microfluidics/$(MICROFLUIDICS_DIR)/libmicrofluidics.a
	cd libmicrofluidics; ar qcv libmicrofluidics.a *.o; ranlib libmicrofluidics.a; mv libmicrofluidics.a ../
	$(RM) libmicrofluidics
else
ifneq ($(wildcard library/thermallib_microfluidics/tarball/$(MICROFLUIDICS_TAR)),)
	cd library/thermallib_microfluidics/; tar xf tarball/$(MICROFLUIDICS_TAR)
	make microfluidics
else
	@echo "ERROR: library/thermallib_microfluidics/tarball/$(MICROFLUIDICS_TAR) not found"
endif
endif

failure:
ifneq ($(wildcard library/reliabilitylib_failure/$(FAILURE_DIR)),)
	cd library/reliabilitylib_failure/$(FAILURE_DIR)/; make clean    
	cd library/reliabilitylib_failure/$(FAILURE_DIR)/; make lib
	$(RM) libfailure libfailure.a; mkdir libfailure
	cd libfailure; ar x ../library/reliabilitylib_failure/$(FAILURE_DIR)/libfailure.a
	cd libfailure; ar qcv libfailure.a *.o; ranlib libfailure.a; mv libfailure.a ../
	$(RM) libfailure
else 
ifneq ($(wildcard library/reliabilitylib_failure/tarball/$(FAILURE_TAR)),)
	cd library/reliabilitylib_failure/; tar xf tarball/$(FAILURE_TAR)
	make failure
else
	@echo "ERROR: library/reliabilitylib_failure/tarball/$(FAILURE_TAR) not found"
endif
endif

filelist:
	@echo $(HDRS) $(KITFOX_HDRS) $(LIBRARY_HDRS)
	@echo $(SRCS) $(KITFOX_SRCS) $(LIBRARY_SRCS)

clean:
	$(RM) libKitFox.a test test-* test_main/*.o *.out *.o *~
	cd communicator/; $(RM) *.o *~
	cd library/energylib_dramsim/; $(RM) *.o *~
	cd library/energylib_dsent/; $(RM) *.o *~
	cd library/energylib_intsim/; $(RM) *.o *~
	cd library/energylib_mcpat/; $(RM) *.o *~
	cd library/energylib_tsv/; $(RM) *.o *~
	cd library/thermallib_3dice/; $(RM) *.o *~
	cd library/thermallib_hotspot/; $(RM) *.o *~
	cd library/thermallib_microfluidics/; $(RM) *.o *~
	cd library/reliabilitylib_failure; $(RM) *.o *~
	cd config/; $(RM) *.out *~

distclean:
	$(RM) *.a
ifneq ($(wildcard library/energylib_dramsim/$(DRAMSIM_DIR)),)
	cd library/energylib_dramsim/$(DRAMSIM_DIR)/; make clean
	$(RM) library/energylib_dramsim/$(DRAMSIM_DIR)
endif
ifneq ($(wildcard library/energylib_dsent/$(DSENT_DIR)),)
	cd library/energylib_dsent/$(DSENT_DIR)/; make clean
	$(RM) library/energylib_dsent/$(DSENT_DIR)
endif
ifneq ($(wildcard library/energylib_intsim/$(INTSIM_DIR)),)
	cd library/energylib_intsim/$(INTSIM_DIR)/; make clean
	$(RM) library/energylib_intsim/$(INTSIM_DIR)
endif
ifneq ($(wildcard library/energylib_mcpat/$(MCPAT_DIR)),)
	cd library/energylib_mcpat/$(MCPAT_DIR)/; make clean
	$(RM) library/energylib_mcpat/$(MCPAT_DIR)
endif
ifneq ($(wildcard library/energylib_tsv/$(TSV_DIR)),)
	cd library/energylib_tsv/$(TSV_DIR)/; make clean
	$(RM) library/energylib_tsv/$(TSV_DIR)
endif
ifneq ($(wildcard library/thermallib_3dice/$(SUPERLU_DIR)),)
	cd library/thermallib_3dice/$(SUPERLU_DIR)/; make clean
	$(RM) library/thermallib_3dice/$(SUPERLU_DIR)
endif
ifneq ($(wildcard library/thermallib_3dice/$(TDICE_DIR)),)
	cd library/thermallib_3dice/$(TDICE_DIR)/; make clean
	$(RM) library/thermallib_3dice/$(TDICE_DIR)
endif
ifneq ($(wildcard library/thermallib_hotspot/$(HOTSPOT_DIR)),)
	cd library/thermallib_hotspot/$(HOTSPOT_DIR)/; make clean
	$(RM) library/thermallib_hotspot/$(HOTSPOT_DIR)
endif
ifneq ($(wildcard library/thermallib_microfluidics/$(MICROFLUIDICS_DIR)),)
	cd library/thermallib_microfluidics/$(MICROFLUIDICS_DIR)/; make clean
	$(RM) library/thermallib_microfluidics/$(MICROFLUIDICS_DIR)
endif
ifneq ($(wildcard library/reliabilitylib_failure/$(FAILURE_DIR)),)
	cd library/reliabilitylib_failure/$(FAILURE_DIR)/; make clean
	$(RM) library/reliabilitylib_failure/$(FAILURE_DIR)
endif
	make clean
	cd doc/; make distclean
