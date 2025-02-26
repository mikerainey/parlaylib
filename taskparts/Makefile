# binaries go here:
INSTALL_PATH=bin
# root folder of taskparts repository:
TASKPARTS_PATH=../../taskparts
PARLAYLIB_PATH=../include
# Other parameters:
#   HWLOC_INCLUDE_PREFIX           include directives for hwloc (optionally empty)
#   HWLOC_LIBRARY_PREFIX           linker directives for hwloc (optionally empty)
#   TASKPARTS_ELASTIC_PREFIX       build configuration for elastic work stealing

.PHONY: install_folder
install_folder:
	mkdir -p $(INSTALL_PATH)

TASKPARTS_INCLUDE_PATH=$(TASKPARTS_PATH)/include/
TASKPARTS_POSIX_INCLUDE_PATH=$(TASKPARTS_INCLUDE_PATH)/taskparts/posix
TASKPARTS_X64_INCLUDE_PATH=$(TASKPARTS_INCLUDE_PATH)/taskparts/x64
TASKPARTS_BENCHMARK_PATH=.
TASKPARTS_EXAMPLE_PATH=../example

INCLUDE_FILES=\
	$(wildcard $(TASKPARTS_INCLUDE_PATH)/taskparts/*.hpp) \
	$(wildcard $(TASKPARTS_POSIX_INCLUDE_PATH)/*.hpp) \
	$(wildcard $(TASKPARTS_X64_INCLUDE_PATH)/*.hpp) \
	$(wildcard $(TASKPARTS_BENCHMARK_PATH)/*.hpp) \
	$(wildcard $(TASKPARTS_EXAMPLE_PATH)/*.hpp)

INCLUDE_PREFIX=\
	-I $(TASKPARTS_INCLUDE_PATH) \
	-I $(PARLAYLIB_PATH) \
	$(HWLOC_INCLUDE_PREFIX)

LINKER_PREFIX=\
	-pthread \
	$(HWLOC_LIBRARY_PREFIX)

# WARNING: flag -fno-stack-protector needed for native fork join
COMMON_COMPILE_PREFIX=\
	$(INCLUDE_PREFIX) \
	-std=c++17 \
	-m64 \
	-fno-stack-protector \
	-DTASKPARTS_POSIX \
	-DTASKPARTS_X64 \
	$(TASKPARTS_ELASTIC_PREFIX) \
	-DPARLAY_TASKPARTS

OPT_COMPILE_PREFIX=\
	$(COMMON_COMPILE_PREFIX) \
	-O3 \
	-march=native \
	-fno-asynchronous-unwind-tables \
	-fomit-frame-pointer \
	-DNDEBUG

OPT_PREFIX=\
	$(OPT_COMPILE_PREFIX)
STA_PREFIX=\
	$(OPT_COMPILE_PREFIX) \
	-DTASKPARTS_STATS
LOG_PREFIX=\
	$(STA_PREFIX) \
	-DTASKPARTS_LOG
DBG_PREFIX=\
	$(COMMON_COMPILE_PREFIX) \
	-Og \
	-g3 \
	-DTASKPARTS_STATS \
	-DTASKPARTS_LOG

# Binaries for native fork join
# -----------------------------

%.opt: %.cpp $(INCLUDE_FILES) install_folder
	$(CXX) $(OPT_PREFIX) -o $(INSTALL_PATH)/$@ $< $(LINKER_PREFIX)
%.sta: %.cpp $(INCLUDE_FILES) install_folder
	$(CXX) $(STA_PREFIX) -o $(INSTALL_PATH)/$@ $< $(LINKER_PREFIX)
%.log: %.cpp $(INCLUDE_FILES) install_folder
	$(CXX) $(LOG_PREFIX) -o $(INSTALL_PATH)/$@ $< $(LINKER_PREFIX)
%.dbg: %.cpp $(INCLUDE_FILES) install_folder
	$(CXX) $(DBG_PREFIX) -o $(INSTALL_PATH)/$@ $< $(LINKER_PREFIX)

# Binaries for tpal, task parallel assembly language
# --------------------------------------------------

%.tpalopt: %.cpp $(INCLUDE_FILES) install_folder
	$(CXX) $(OPT_PREFIX) $(basename $<)_manual.s -o $(INSTALL_PATH)/$@ $< $(LINKER_PREFIX)
%.tpalsta: %.cpp $(INCLUDE_FILES) install_folder
	$(CXX) $(STA_PREFIX) $(basename $<)_manual.s -o $(INSTALL_PATH)/$@ $< $(LINKER_PREFIX)
%.tpallog: %.cpp $(INCLUDE_FILES) install_folder
	$(CXX) $(LOG_PREFIX) $(basename $<)_manual.s -o $(INSTALL_PATH)/$@ $< $(LINKER_PREFIX)
%.tpaldbg: %.cpp $(INCLUDE_FILES) install_folder
	$(CXX) $(DBG_PREFIX) $(basename $<)_manual.s -o $(INSTALL_PATH)/$@ $< $(LINKER_PREFIX)

clean:
	rm -rf bin
