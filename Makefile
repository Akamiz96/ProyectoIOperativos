
imgconc: image.c BMP.exe BMP2.exe
	gcc image.c -o imgconc -lpthread

BMP.exe:
	gcc BMP.c -o BMP.exe

BMP2.exe:
	gcc BMP2.c -o BMP2.exe

clean:
	rm imgconc
