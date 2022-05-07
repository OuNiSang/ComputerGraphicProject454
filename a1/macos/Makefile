LDFLAGS = -L. -lglfw -ldl # -lpthread
CXXFLAGS = -g -std=c++11 --stdlib=libc++ -Wall -Wno-write-strings -Wno-parentheses -Wno-self-assign -Wno-c++11-extensions -DMACOS #-pthread

vpath %.cpp ../src
vpath %.c   ../src/glad/src

OBJS = main.o world.o centipede.o mushroom.o player.o dart.o linalg.o gpuProgram.o strokefont.o fg_stroke.o glad.o

EXEC = centipede

all:    $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS) $(LDFLAGS) 

glad.o: ../src/glad/src/glad.c

clean:
	rm -f  *~ $(EXEC) $(OBJS) Makefile.bak

depend:	
	makedepend -Y ../src/*.h ../src/*.cpp 2> /dev/null
	@echo ""
	@echo "Now edit the Makefile to remove the ../src/ prefix of all of the dependences for the .o files"
	@echo ""

# DO NOT DELETE

centipede.o: ../src/centipede.h ../src/headers.h
centipede.o: ../src/glad/include/glad/glad.h
centipede.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
centipede.o: ../src/main.h ../src/gpuProgram.h ../src/drawbuffer.h
centipede.o: ../src/seq.h ../src/worldDefs.h ../src/world.h
centipede.o: ../src/mushroom.h ../src/player.h ../src/dart.h
dart.o: ../src/dart.h ../src/headers.h ../src/glad/include/glad/glad.h
dart.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
dart.o: ../src/drawbuffer.h ../src/seq.h ../src/worldDefs.h
dart.o: ../src/main.h ../src/gpuProgram.h
fg_stroke.o: ../src/strokefont.h ../src/headers.h
fg_stroke.o: ../src/glad/include/glad/glad.h
fg_stroke.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
fg_stroke.o: ../src/gpuProgram.h
gpuProgram.o: ../src/gpuProgram.h ../src/headers.h
gpuProgram.o: ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
linalg.o: ../src/linalg.h
main.o: ../src/headers.h ../src/glad/include/glad/glad.h
main.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
main.o: ../src/gpuProgram.h ../src/world.h ../src/main.h ../src/seq.h
main.o: ../src/centipede.h ../src/drawbuffer.h ../src/worldDefs.h
main.o: ../src/mushroom.h ../src/player.h ../src/dart.h
main.o: ../src/strokefont.h
mushroom.o: ../src/mushroom.h ../src/headers.h
mushroom.o: ../src/glad/include/glad/glad.h
mushroom.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
mushroom.o: ../src/drawbuffer.h ../src/seq.h ../src/main.h
mushroom.o: ../src/gpuProgram.h ../src/worldDefs.h
player.o: ../src/player.h ../src/headers.h
player.o: ../src/glad/include/glad/glad.h
player.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
player.o: ../src/drawbuffer.h ../src/seq.h ../src/worldDefs.h
player.o: ../src/main.h ../src/gpuProgram.h
strokefont.o: ../src/strokefont.h ../src/headers.h
strokefont.o: ../src/glad/include/glad/glad.h
strokefont.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
strokefont.o: ../src/gpuProgram.h ../src/fg_stroke.h
world.o: ../src/world.h ../src/headers.h
world.o: ../src/glad/include/glad/glad.h
world.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
world.o: ../src/main.h ../src/gpuProgram.h ../src/seq.h
world.o: ../src/centipede.h ../src/drawbuffer.h ../src/worldDefs.h
world.o: ../src/mushroom.h ../src/player.h ../src/dart.h
world.o: ../src/strokefont.h
