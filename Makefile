SOURCE_GAME=client.cpp webcamdet.cpp src/*.cpp
SOURCE_WIN=client.cpp webcamdet_win.cpp src/*.cpp
EXE=exe
game:
	g++ -O3 -std=c++11 -I/usr/local/include/opencv -I/usr/local/include/opencv2 -L/usr/local/lib/ $(SOURCE_GAME) -lpthread -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching -lopencv_imgcodecs -lopencv_videoio -lopencv_flann -lopencv_photo -lopencv_shape -lopencv_superres -lopencv_videostab -o $(EXE)
win:
	g++ -O3 -std=c++11 -I/usr/local/include/opencv -I/usr/local/include/opencv2 -L/usr/local/lib/ $(SOURCE_WIN) -lpthread -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching -lopencv_imgcodecs -lopencv_videoio -lopencv_flann -lopencv_photo -lopencv_shape -lopencv_superres -lopencv_videostab -o $(EXE)
run:
	./$(EXE) 127.0.0.1 10000
clean:
	rm -f $(EXE)
