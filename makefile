
MAXJOBS := 8
MAXLOAD := 4 

EXECUTABLES := main tests
BUILDTYPES := debug release

# avoids removements
.SECONDARY : 

help:
	@echo "all tags testhpp"
	@echo "clean-all clean-tags" $(foreach B, $(BUILDTYPES), clean-$(B))
	@echo "build-all" $(foreach B, $(BUILDTYPES), build-all-exe-$(B)) $(foreach E, $(EXECUTABLES), build-all-bld-$(E))
	@echo $(foreach E, $(EXECUTABLES), $(foreach B, $(BUILDTYPES), build-$(B)-$(E)))
	@echo $(foreach R, run gdb val, $(foreach E, $(EXECUTABLES), $(foreach B, $(BUILDTYPES), $(R)-$(B)-$(E))))
	@echo doxygen clean-doxygen

all : testhpp tags build-all

all-debug : testhpp tags debug

all-release : testhpp tags release

testhpp : 
	dev_bin/testhpp.sh

clean-all : clean-tags clean-debug clean-release clean-doxygen

clean-tags :
	rm -vrf tags

clean-% :
	rm -vrf cmake-$*/cmake-files

clean-doxygen :
	rm -rf doxygen

tags : src/*.hpp src/*.cpp src/main/*.cpp src/tests/*.cpp
	ctags-exuberant --extra=+q --fields=+a+i --recurse --c++-kinds=+p .

doxygen : src/* src/*/* | doxygen-dir
	cd doxygen; doxygen rmDoxygenConfigFile.txt
	
doxygen-dir :
	mkdir -p doxygen
	dev_bin/setupDoxygen.sh

build-all : $(foreach E, $(EXECUTABLES), build-all-bld-$(E))
	true

build-all-bld-% : $(foreach B, $(BUILDTYPES), build-$(B)-%)
	true

build-all-exe-% : $(foreach E, $(EXECUTABLES), build-%-$(E))
	true

build-debug-% : cmake-debug/cmake-files/rangematcher-% | testhpp
	true

build-release-% : cmake-release/cmake-files/rangematcher-% | testhpp
	true

cmake-debug/cmake-files/rangematcher-% : src/*.hpp src/*.cpp src/%/*.cpp
	rm -vf $@
	mkdir -p $(dir $@)
	cd $(dir $@); cmake ..
	make -j $(MAXJOBS) -l $(MAXLOAD) -C $(dir $@) $(notdir $@)

gdb-debug-% : cmake-debug/cmake-files/rangematcher-%
	gdb $?

cmake-release/cmake-files/rangematcher-% : src/*.hpp src/*.cpp src/%/*.cpp
	rm -vf $@
	mkdir -p $(dir $@)
	cd $(dir $@); cmake ..
	make -j $(MAXJOBS) -l $(MAXLOAD) -C $(dir $@) $(notdir $@)


gdb-release-% : cmake-release/cmake-files/rangematcher-%
	gdb $?


run-%-main : cmake-%/cmake-files/rangematcher-main
	rlwrap -pgreen $?

run-%-tests : cmake-%/cmake-files/rangematcher-tests
	$?

val-debug-% : cmake-debug/cmake-files/rangematcher-%
	valgrind $?

val-release-% : cmake-release/cmake-files/rangematcher-%
	valgrind $?

