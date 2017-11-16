all:
	g++ \
	 my-interp.cc \
	 -g \
	 -I. \
	 -L./lib \
	 -lwabt \
	 -o bin/my-interp