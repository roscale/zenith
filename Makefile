all:
	g++ main.cpp -lglfw -lGL -I . -L . -lflutter_engine -o embedder