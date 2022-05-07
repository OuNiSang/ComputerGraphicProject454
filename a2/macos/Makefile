LDFLAGS = -L. -lglfw -ldl # -lpthread
CXXFLAGS = -g -std=c++11 --stdlib=libc++ -Wall -Wno-write-strings -Wno-parentheses -Wno-self-assign -Wno-c++11-extensions -Wno-unused-variable -DMACOS #-pthread

vpath %.cpp ../src
vpath %.c   ../src/glad/src

OBJS = shader.o gpuProgram.o linalg.o wavefront.o renderer.o gbuffer.o axes.o glad.o

EXEC = shader

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

axes.o: ../src/linalg.h ../src/gpuProgram.h ../src/headers.h
axes.o: ../src/glad/include/glad/glad.h
axes.o: ../src/glad/include/KHR/khrplatform.h
gpuProgram.o: ../src/headers.h ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
headers.o: ../src/glad/include/glad/glad.h
headers.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
renderer.o: ../src/wavefront.h ../src/headers.h
renderer.o: ../src/glad/include/glad/glad.h
renderer.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
renderer.o: ../src/seq.h ../src/shadeMode.h ../src/gpuProgram.h
renderer.o: ../src/gbuffer.h
seq.o: ../src/headers.h ../src/glad/include/glad/glad.h
seq.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
shader.o: ../src/linalg.h
wavefront.o: ../src/headers.h ../src/glad/include/glad/glad.h
wavefront.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
wavefront.o: ../src/seq.h ../src/shadeMode.h ../src/gpuProgram.h
axes.o: ../src/headers.h ../src/glad/include/glad/glad.h
axes.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
axes.o: ../src/axes.h ../src/gpuProgram.h
font.o: ../src/headers.h ../src/glad/include/glad/glad.h
font.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
font.o: ../src/gpuProgram.h
gbuffer.o: ../src/headers.h ../src/glad/include/glad/glad.h
gbuffer.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gbuffer.o: ../src/gbuffer.h ../src/font.h ../src/shader.h
gpuProgram.o: ../src/gpuProgram.h ../src/headers.h
gpuProgram.o: ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
linalg.o: ../src/linalg.h
renderer.o: ../src/headers.h ../src/glad/include/glad/glad.h
renderer.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
renderer.o: ../src/renderer.h ../src/wavefront.h ../src/seq.h
renderer.o: ../src/shadeMode.h ../src/gpuProgram.h ../src/gbuffer.h
renderer.o: ../src/shader.h
shader.o: ../src/headers.h ../src/glad/include/glad/glad.h
shader.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
shader.o: ../src/wavefront.h ../src/seq.h ../src/shadeMode.h
shader.o: ../src/gpuProgram.h ../src/renderer.h ../src/gbuffer.h
shader.o: ../src/shader.h ../src/font.h ../src/axes.h
wavefront.o: ../src/headers.h ../src/glad/include/glad/glad.h
wavefront.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
wavefront.o: ../src/gpuProgram.h ../src/wavefront.h ../src/seq.h
wavefront.o: ../src/shadeMode.h
