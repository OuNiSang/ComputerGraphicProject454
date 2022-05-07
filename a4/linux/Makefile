# If you want text output in the window, you need to install the open
# source freetype package.  On Linux, do "sudo apt-get install libfreetype6-dev".

# If you don't have freetype, use this:

LDFLAGS  = -L. -lglfw -lGL -ldl
CXXFLAGS = -g -DLINUX -Wall -Wno-deprecated -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable -std=c++11

# If you have installed the freetype package, use this:

#LDFLAGS  = -L. -lglfw -lGL -ldl -lfreetype
#CXXFLAGS = -g -DLINUX -Wall -Wno-deprecated -Wno-sign-compare -std=c++11 -DHAVE_FREETYPE -I/usr/include/freetype2

vpath %.cpp ../src
vpath %.c   ../src/glad/src

OBJS =	linalg.o arcball.o font.o sphere.o triangle.o light.o eye.o object.o gpuProgram.o photonmap.o axes.o arrow.o bbox.o drawSegs.o glad.o \
	material.o texture.o vertex.o rtWindow.o main.o scene.o pixelZoom.o

EXEC = rt

all:	$(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(OBJS) $(LDFLAGS) 

#glad.o:	glad.c
#	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *~ $(EXEC) $(OBJS) Makefile.bak

depend:	
	makedepend -Y ../src/*.h ../src/*.cpp 2> /dev/null
	@echo ""
	@echo "Now edit the Makefile to remove the ../src/ prefix of all of the dependences for the .o files"
	@echo ""

# DO NOT DELETE

arcball.o: ../src/headers.h ../src/glad/include/glad/glad.h
arcball.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
arrow.o: ../src/object.h ../src/linalg.h ../src/material.h
arrow.o: ../src/texture.h ../src/headers.h
arrow.o: ../src/glad/include/glad/glad.h
arrow.o: ../src/glad/include/KHR/khrplatform.h ../src/seq.h
arrow.o: ../src/gpuProgram.h
axes.o: ../src/linalg.h ../src/gpuProgram.h ../src/headers.h
axes.o: ../src/glad/include/glad/glad.h
axes.o: ../src/glad/include/KHR/khrplatform.h ../src/seq.h
bbox.o: ../src/linalg.h
drawSegs.o: ../src/headers.h ../src/glad/include/glad/glad.h
drawSegs.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
drawSegs.o: ../src/gpuProgram.h ../src/seq.h
eye.o: ../src/linalg.h
gpuProgram.o: ../src/headers.h ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/seq.h
headers.o: ../src/glad/include/glad/glad.h
headers.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
light.o: ../src/linalg.h ../src/sphere.h ../src/seq.h
light.o: ../src/gpuProgram.h ../src/headers.h
light.o: ../src/glad/include/glad/glad.h
light.o: ../src/glad/include/KHR/khrplatform.h ../src/object.h
light.o: ../src/material.h ../src/texture.h
main.o: ../src/seq.h ../src/scene.h ../src/linalg.h ../src/object.h
main.o: ../src/material.h ../src/texture.h ../src/headers.h
main.o: ../src/glad/include/glad/glad.h
main.o: ../src/glad/include/KHR/khrplatform.h ../src/gpuProgram.h
main.o: ../src/light.h ../src/sphere.h ../src/eye.h ../src/photonmap.h
main.o: ../src/priority.h ../src/axes.h ../src/drawSegs.h
main.o: ../src/arrow.h ../src/rtWindow.h ../src/main.h
main.o: ../src/arcball.h ../src/pixelZoom.h
material.o: ../src/linalg.h ../src/texture.h ../src/headers.h
material.o: ../src/glad/include/glad/glad.h
material.o: ../src/glad/include/KHR/khrplatform.h ../src/seq.h
material.o: ../src/gpuProgram.h
object.o: ../src/linalg.h ../src/material.h ../src/texture.h
object.o: ../src/headers.h ../src/glad/include/glad/glad.h
object.o: ../src/glad/include/KHR/khrplatform.h ../src/seq.h
object.o: ../src/gpuProgram.h
photonmap.o: ../src/headers.h ../src/glad/include/glad/glad.h
photonmap.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
photonmap.o: ../src/seq.h ../src/priority.h
pixelZoom.o: ../src/gpuProgram.h ../src/headers.h
pixelZoom.o: ../src/glad/include/glad/glad.h
pixelZoom.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
pixelZoom.o: ../src/seq.h
rtWindow.o: ../src/main.h ../src/arcball.h ../src/headers.h
rtWindow.o: ../src/glad/include/glad/glad.h
rtWindow.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
rtWindow.o: ../src/scene.h ../src/seq.h ../src/object.h
rtWindow.o: ../src/material.h ../src/texture.h ../src/gpuProgram.h
rtWindow.o: ../src/light.h ../src/sphere.h ../src/eye.h
rtWindow.o: ../src/photonmap.h ../src/priority.h ../src/axes.h
rtWindow.o: ../src/drawSegs.h ../src/arrow.h
scene.o: ../src/seq.h ../src/linalg.h ../src/object.h
scene.o: ../src/material.h ../src/texture.h ../src/headers.h
scene.o: ../src/glad/include/glad/glad.h
scene.o: ../src/glad/include/KHR/khrplatform.h ../src/gpuProgram.h
scene.o: ../src/light.h ../src/sphere.h ../src/eye.h
scene.o: ../src/photonmap.h ../src/priority.h ../src/axes.h
scene.o: ../src/drawSegs.h ../src/arrow.h
sphere.o: ../src/linalg.h ../src/seq.h ../src/gpuProgram.h
sphere.o: ../src/headers.h ../src/glad/include/glad/glad.h
sphere.o: ../src/glad/include/KHR/khrplatform.h ../src/object.h
sphere.o: ../src/material.h ../src/texture.h
texture.o: ../src/headers.h ../src/glad/include/glad/glad.h
texture.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
texture.o: ../src/seq.h
triangle.o: ../src/object.h ../src/linalg.h ../src/material.h
triangle.o: ../src/texture.h ../src/headers.h
triangle.o: ../src/glad/include/glad/glad.h
triangle.o: ../src/glad/include/KHR/khrplatform.h ../src/seq.h
triangle.o: ../src/gpuProgram.h ../src/vertex.h
vertex.o: ../src/linalg.h
arcball.o: ../src/headers.h ../src/glad/include/glad/glad.h
arcball.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
arcball.o: ../src/arcball.h
arrow.o: ../src/headers.h ../src/glad/include/glad/glad.h
arrow.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
arrow.o: ../src/arrow.h ../src/object.h ../src/material.h
arrow.o: ../src/texture.h ../src/seq.h ../src/gpuProgram.h
axes.o: ../src/headers.h ../src/glad/include/glad/glad.h
axes.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
axes.o: ../src/axes.h ../src/gpuProgram.h ../src/seq.h
bbox.o: ../src/headers.h ../src/glad/include/glad/glad.h
bbox.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
bbox.o: ../src/bbox.h ../src/drawSegs.h ../src/gpuProgram.h
bbox.o: ../src/seq.h ../src/main.h
drawSegs.o: ../src/headers.h ../src/glad/include/glad/glad.h
drawSegs.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
drawSegs.o: ../src/drawSegs.h ../src/gpuProgram.h ../src/seq.h
eye.o: ../src/headers.h ../src/glad/include/glad/glad.h
eye.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
eye.o: ../src/eye.h ../src/main.h
font.o: ../src/headers.h ../src/glad/include/glad/glad.h
font.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/gpuProgram.h ../src/headers.h
gpuProgram.o: ../src/glad/include/glad/glad.h
gpuProgram.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
gpuProgram.o: ../src/seq.h
light.o: ../src/headers.h ../src/glad/include/glad/glad.h
light.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
light.o: ../src/light.h ../src/sphere.h ../src/seq.h
light.o: ../src/gpuProgram.h ../src/object.h ../src/material.h
light.o: ../src/texture.h ../src/main.h
linalg.o: ../src/linalg.h
main.o: ../src/headers.h ../src/glad/include/glad/glad.h
main.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
main.o: ../src/rtWindow.h ../src/main.h ../src/arcball.h
main.o: ../src/scene.h ../src/seq.h ../src/object.h ../src/material.h
main.o: ../src/texture.h ../src/gpuProgram.h ../src/light.h
main.o: ../src/sphere.h ../src/eye.h ../src/photonmap.h
main.o: ../src/priority.h ../src/axes.h ../src/drawSegs.h
main.o: ../src/arrow.h ../src/font.h ../src/pixelZoom.h
material.o: ../src/headers.h ../src/glad/include/glad/glad.h
material.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
material.o: ../src/material.h ../src/texture.h ../src/seq.h
material.o: ../src/gpuProgram.h ../src/main.h
object.o: ../src/headers.h ../src/glad/include/glad/glad.h
object.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
object.o: ../src/object.h ../src/material.h ../src/texture.h
object.o: ../src/seq.h ../src/gpuProgram.h ../src/main.h
photonmap.o: ../src/headers.h ../src/glad/include/glad/glad.h
photonmap.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
photonmap.o: ../src/photonmap.h ../src/seq.h ../src/priority.h
photonmap.o: ../src/drawSegs.h ../src/gpuProgram.h ../src/scene.h
photonmap.o: ../src/object.h ../src/material.h ../src/texture.h
photonmap.o: ../src/light.h ../src/sphere.h ../src/eye.h ../src/axes.h
photonmap.o: ../src/arrow.h ../src/main.h ../src/priority.cpp
pixelZoom.o: ../src/pixelZoom.h ../src/gpuProgram.h ../src/headers.h
pixelZoom.o: ../src/glad/include/glad/glad.h
pixelZoom.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
pixelZoom.o: ../src/seq.h
priority.o: ../src/priority.h
rtWindow.o: ../src/rtWindow.h ../src/main.h ../src/arcball.h
rtWindow.o: ../src/headers.h ../src/glad/include/glad/glad.h
rtWindow.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
rtWindow.o: ../src/scene.h ../src/seq.h ../src/object.h
rtWindow.o: ../src/material.h ../src/texture.h ../src/gpuProgram.h
rtWindow.o: ../src/light.h ../src/sphere.h ../src/eye.h
rtWindow.o: ../src/photonmap.h ../src/priority.h ../src/axes.h
rtWindow.o: ../src/drawSegs.h ../src/arrow.h
scene.o: ../src/headers.h ../src/glad/include/glad/glad.h
scene.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
scene.o: ../src/scene.h ../src/seq.h ../src/object.h ../src/material.h
scene.o: ../src/texture.h ../src/gpuProgram.h ../src/light.h
scene.o: ../src/sphere.h ../src/eye.h ../src/photonmap.h
scene.o: ../src/priority.h ../src/axes.h ../src/drawSegs.h
scene.o: ../src/arrow.h ../src/rtWindow.h ../src/main.h
scene.o: ../src/arcball.h ../src/triangle.h ../src/vertex.h
scene.o: ../src/font.h
sphere.o: ../src/sphere.h ../src/linalg.h ../src/seq.h
sphere.o: ../src/gpuProgram.h ../src/headers.h
sphere.o: ../src/glad/include/glad/glad.h
sphere.o: ../src/glad/include/KHR/khrplatform.h ../src/object.h
sphere.o: ../src/material.h ../src/texture.h ../src/main.h
texture.o: ../src/headers.h ../src/glad/include/glad/glad.h
texture.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
texture.o: ../src/texture.h ../src/seq.h
triangle.o: ../src/headers.h ../src/glad/include/glad/glad.h
triangle.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
triangle.o: ../src/triangle.h ../src/object.h ../src/material.h
triangle.o: ../src/texture.h ../src/seq.h ../src/gpuProgram.h
triangle.o: ../src/vertex.h ../src/main.h
vertex.o: ../src/headers.h ../src/glad/include/glad/glad.h
vertex.o: ../src/glad/include/KHR/khrplatform.h ../src/linalg.h
vertex.o: ../src/vertex.h ../src/main.h
