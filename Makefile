
imgconc: image.o
	gcc -o imgconc image.o -lpthread

image.o: image.c
	gcc -c image.c

clean:
	rm *.o
