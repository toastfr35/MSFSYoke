all : clean YokeInput.exe

YokeInput.exe : YokeInput.cpp arduino_com.cpp MSFS_com.cpp
	g++ -o YokeInput.exe $? SimConnect.dll -O0 -g

test : *.cpp *.h
	g++ -o test.exe YokeInput.cpp arduino_com.cpp -O0 -g -DNOSIM
	./test.exe

run : YokeInput.exe SimConnect.dll
	./YokeInput.exe

clean:
	rm -f YokeInput.exe

