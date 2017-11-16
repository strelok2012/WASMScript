all:
	git submodule update --init --recursive
	cd wabt && $(MAKE) gcc-release 
	g++ \
	 my-interp.cc \
	 -g \
	 -I./wabt \
	 -I./wabt/out/gcc/Release \
	 -L./wabt/out/gcc/Release \
	 -lwabt \
	 -o my-interp
clean:
	rm -f my-interp
	cd wabt && $(MAKE) clean
