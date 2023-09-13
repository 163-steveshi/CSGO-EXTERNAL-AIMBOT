objects = memory.h vector.h main.cc

aimbot: $(objects)
	g++ -g -o aimbot $(objects)

clean:
	rm -f aimbot *.h 
