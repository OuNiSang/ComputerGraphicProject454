EXEC = volren

vpath %.cpp ../src
vpath %.c   ../src/glad/src

OBJS = main.o volume.o cube.o gpuProgram.o linalg.o gbuffer.o strokefont.o fg_stroke.o glad.o

LDFLAGS = -L. -lglfw -lGL -ldl
CXXFLAGS = -g -std=c++11 -Wall -Wno-write-strings -Wno-unused-result -Wno-parentheses -Wno-sequence-point -DLINUX
CXX      = g++

all:    $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS) $(LDFLAGS) 

clean:
	rm -f *~ $(EXEC) $(OBJS) Makefile.bak

depend:	
	makedepend -Y ../src/*.h ../src/*.cpp 2> /dev/null
	@echo ""
	@echo "Now edit the Makefile to remove the ../src/ prefix of all of the dependences for the .o files"
	@echo ""

# DO NOT DELETE

gpuProgram.o: ../src/headers.h ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/seq.h
headers.o: ../src/glad/include/glad/glad.h
headers.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
seq.o: ../src/headers.h ../src/glad/include/glad/glad.h
seq.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
strokefont.o: ../src/headers.h ../src/glad/include/glad/glad.h
strokefont.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
strokefont.o: ../src/gpuProgram.h ../src/seq.h
volume.o: ../src/headers.h ../src/glad/include/glad/glad.h
volume.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
volume.o: ../src/main.h ../src/gpuProgram.h ../src/seq.h
volume.o: ../src/gbuffer.h
cube.o: ../src/headers.h ../src/glad/include/glad/glad.h
cube.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
cube.o: ../src/cube.h ../src/main.h
fg_stroke.o: ../src/strokefont.h ../src/headers.h
fg_stroke.o: ../src/glad/include/glad/glad.h
fg_stroke.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
fg_stroke.o: ../src/gpuProgram.h ../src/seq.h
gbuffer.o: ../src/headers.h ../src/glad/include/glad/glad.h
gbuffer.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gbuffer.o: ../src/gbuffer.h ../src/strokefont.h ../src/gpuProgram.h
gbuffer.o: ../src/seq.h
gpuProgram.o: ../src/gpuProgram.h ../src/headers.h
gpuProgram.o: ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/seq.h
linalg.o: ../src/linalg.h
main.o: ../src/headers.h ../src/glad/include/glad/glad.h
main.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
main.o: ../src/volume.h ../src/main.h ../src/gpuProgram.h ../src/seq.h
main.o: ../src/gbuffer.h ../src/strokefont.h
strokefont.o: ../src/strokefont.h ../src/headers.h
strokefont.o: ../src/glad/include/glad/glad.h
strokefont.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
strokefont.o: ../src/gpuProgram.h ../src/seq.h ../src/fg_stroke.h
volume.o: ../src/headers.h ../src/glad/include/glad/glad.h
volume.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
volume.o: ../src/volume.h ../src/main.h ../src/gpuProgram.h
volume.o: ../src/seq.h ../src/gbuffer.h ../src/cube.h
volume.o: ../src/strokefont.h
