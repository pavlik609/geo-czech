CC = g++
WEBCC = emcc

ASSETS = --preload-file LiberationMono-Regular.ttf --preload-file img --preload-file geos 

linux: 
	$(CC) main.cpp -o main -lGL -lraylib -lm
#source "/home/pavel/sources/emsdk/emsdk_env.sh"
web:
	$(WEBCC) -o geo.html main.cpp -Os -Wall ./libwebraylib.a -I. -I./raylib -L. -L./lwebraylib.a -s USE_GLFW=3 -s ASYNCIFY -sASSERTIONS --shell-file /home/pavel/sources/raylib/src/minshell.html $(ASSETS) -sINITIAL_MEMORY=33947648 -DPLATFORM_WEB
	#$(WEBCC) -o game.html main.cpp -Os -Wall ./path-to/libraylib.a -I. -Ipath-to-raylib-h -L. -Lpath-to-libraylib-a -s USE_GLFW=3 -s ASYNCIFY --shell-file $HOME/sources/raylib/src/minshell.html -DPLATFORM_WEB
