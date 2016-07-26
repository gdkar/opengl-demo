CFLAGS+= -std=gnu11 -pthread -O3 -Wall -Wextra -Igl3w/ -lglfw
CXXFLAGS+= -std=gnu++14 -g -ggdb -pthread -O3 -Wall -Wextra -Igl3w/ -lglfw
LDLIBS+= -L/usr/lib/x86_64-linux-gnu -shared -pie $(shell pkg-config --libs glfw3) -lGL -ldl -lrt -lm -lpthread -pthread
CC:=g++

demoxx : demoxx.cpp gl3w/gl3w.c
run : demo
	./$^

run-on-mesa : demo
	LIBGL_ALWAYS_SOFTWARE=1 ./$^

clean :
	$(RM) demo demo.exe *.o
