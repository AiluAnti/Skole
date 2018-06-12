SOURCE = main.c triangle.c drawline.c object.c list.c
HEADER = drawline.h triangle.h object.h list.h teapot_data.h sphere_data.h

# Change this according to the type of system you are using
all: app_linux

app_win32: $(SOURCE) $(HEADER)
	cp SDL32.dll SDL.dll
	gcc -Wall -o app.exe -Iinclude $(SOURCE) SDL.dll

app_win64: $(SOURCE) $(HEADER)
	cp SDL64.dll SDL.dll
	gcc -Wall -o app.exe -Iinclude $(SOURCE) SDL.dll

app_mac: $(SOURCE) $(HEADER)
	gcc -Wall -o app -Iinclude `sdl-config --cflags --libs` $(SOURCE)

app_linux: $(SOURCE) $(HEADER)
	gcc -Wall -o app -Iinclude $(SOURCE) -lSDL -lm -lX11 -g

clean:
	rm -f *.o app.exe app *~
