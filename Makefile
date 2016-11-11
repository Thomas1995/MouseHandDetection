SOURCE=client.cpp src/*.cpp
EXE=client
build:
	g++ -I/usr/local/include/opencv -I/usr/local/include/opencv2 -L/usr/local/lib/ $(SOURCE) -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml -lopencv_video -lopencv_features2d -lopencv_calib3d -lopencv_objdetect -lopencv_stitching -lopencv_imgcodecs -lopencv_videoio -lopencv_flann -lopencv_photo -lopencv_shape -lopencv_superres -lopencv_videostab -o $(EXE)
run:
	./$(EXE) 10000 127.0.0.1
clean:
	rm -f $(EXE)

