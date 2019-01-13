
all : tags a.out

tags : *.hpp *.cpp
	ctags-exuberant --extra=+q --fields=+a+i --recurse --c++-kinds=+p .

a.out : *.hpp *.cpp
	g++ -Wall --std=c++14 *.cpp

