oh_brother: compress.o main.o parameters.o pcl.o pjl.o
	cc -o oh_brother compress.o main.o parameters.o pcl.o pjl.o

compress.o: compress.c compress.h parameters.h
main.o: main.c pcl.h pjl.h parameters.h
parameters.o: parameters.c parameters.h
pcl.o: pcl.c pcl.h compress.h parameters.h
pjl.o: pjl.c pjl.h parameters.h

clean:
	rm -f *.o oh_brother

.PHONY: clean
