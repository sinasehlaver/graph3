all:	
	g++ -g -o main main.cpp -lglfw -lglew -framework OpenGL

clean:
	rm -rf *.o