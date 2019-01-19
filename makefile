
help:
	@echo "all all-debug all-release tags debug release clean testhpp run-debug run-release run-gdb"

all : testhpp tags debug release

all-debug : testhpp tags debug 

all-release : testhpp tags release

testhpp : 
	dev_bin/testhpp.sh

clean : clean-tags clean-debug clean-release

clean-tags :
	rm -vrf tags

clean-debug :
	rm -vrf cmake-debug/cmake-files

clean-release :
	rm -vrf cmake-release/cmake-files

tags : src/*.hpp src/*.cpp
	ctags-exuberant --extra=+q --fields=+a+i --recurse --c++-kinds=+p .

debug : cmake-debug/cmake-files/rangematcher

cmake-debug/cmake-files/rangematcher : src/*.hpp src/*.cpp
	rm -vf $@
	mkdir -p $(dir $@)
	cd $(dir $@); cmake ..
	make -C $(dir $@) -j8 

run-debug : cmake-debug/cmake-files/rangematcher
	$?

run-gdb : cmake-debug/cmake-files/rangematcher
	gdb $?

release : cmake-release/cmake-files/rangematcher

cmake-release/cmake-files/rangematcher : src/*.hpp src/*.cpp
	rm -vf $@
	mkdir -p $(dir $@)
	cd $(dir $@); cmake ..
	make -C $(dir $@) -j8 

run-release : cmake-release/cmake-files/rangematcher
	$?


