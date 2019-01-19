
help:
	@echo "all all-debug all-release tags debug release clean"

all : tags debug release

all-debug : tags debug 

all-release : tags release

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
	mkdir -p $(dir $@)
	cd $(dir $@); cmake ..
	make -C $(dir $@) -j8 

release : cmake-release/cmake-files/rangematcher

cmake-release/cmake-files/rangematcher : src/*.hpp src/*.cpp
	mkdir -p $(dir $@)
	cd $(dir $@); cmake ..
	make -C $(dir $@) -j8 

