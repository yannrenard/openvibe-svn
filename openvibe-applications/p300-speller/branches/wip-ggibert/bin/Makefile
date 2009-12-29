CC= g++
CFLAGS= -Wall

SRC=../src
INC=../include
LIB=../lib

INCLDIR=-I$(INC) -I/usr/include/ -I/usr/include/freetype2 `sdl-config --cflags` `xml2-config --cflags`
LIBSDIR= -L$(LIB) -L/usr/X11R6/lib `sdl-config --libs` `xml2-config --libs`

LDLIBS= -lGL -lGLU -lfreetype -lm -lvrpn 

EXEC = P300Stimulator

all: $(EXEC)

P300Stimulator: P300Stimulator.o 
	$(CC) -o $@  $(LIB)/* $(INCLDIR) $(LIBSDIR)  $(LDLIBS)

P300Stimulator.o: VrpnListener.o ParallelPort.o VGAPort.o Timer.o XmlParser.o Tools.o Display.o 
	$(CC) $(CFLAGS) -o $(LIB)/$@ -c $(SRC)/P300Stimulator.cpp  $(INCLDIR)
	
VrpnListener.o:
	$(CC) $(CFLAGS) -o $(LIB)/$@ -c $(SRC)/VrpnListener.cpp  $(INCLDIR)
	
Display.o: FreeType.o
	$(CC) $(CFLAGS) -o $(LIB)/$@ -c $(SRC)/Display.cpp  $(INCLDIR)
	
Tools.o:
	$(CC) $(CFLAGS) -o $(LIB)/$@ -c $(SRC)/Tools.cpp  $(INCLDIR)

XmlParser.o:
	$(CC) $(CFLAGS) -o $(LIB)/$@ -c $(SRC)/XmlParser.cpp  $(INCLDIR)
	
FreeType.o:
	$(CC) $(CFLAGS) -o $(LIB)/$@  -c $(SRC)/FreeType.cpp $(INCLDIR) 

VGAPort.o: Port.o
	$(CC) $(CFLAGS) -o $(LIB)/$@ -c $(SRC)/VGAPort.cpp  $(INCLDIR)

ParallelPort.o: Port.o
	$(CC) $(CFLAGS) -o $(LIB)/$@ -c $(SRC)/ParallelPort.cpp  $(INCLDIR)

Port.o:
	$(CC) $(CFLAGS) -o $(LIB)/$@ -c $(SRC)/Port.cpp  $(INCLDIR)	
	
Timer.o:
	$(CC) $(CFLAGS) -o $(LIB)/$@ -c $(SRC)/Timer.cpp  $(INCLDIR)
	
clean:
	rm $(LIB)/*.o
	rm $(EXEC)
