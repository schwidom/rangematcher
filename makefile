
all : tags cmake-debug/cmake-files/rangematcher

clean :
	rm -vrf cmake-debug/cmake-files tags

tags : *.hpp *.cpp
	ctags-exuberant --extra=+q --fields=+a+i --recurse --c++-kinds=+p .

cmake-debug/cmake-files/rangematcher : *.hpp *.cpp
	mkdir -p $(dir $@)
	cd $(dir $@); cmake ..
	make -C $(dir $@) -j8 
