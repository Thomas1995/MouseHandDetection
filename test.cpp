#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <unistd.h>
#include "client.h"
#include "include/DataProcessor.h"

using namespace cv;
using namespace std;

#define SAME_ZONE_STEP 20
#define JUMP_STEP 50
#define NO_SELECTION_LIMIT 3

extern int serverSock;

vector< vector<Vec3f> > findZones(vector<Vec3f>& circles) {
  vector< vector<Vec3f> > zones;

  for(auto c : circles) {
    bool foundMatch = false;
    for(int i = 0; i < zones.size(); ++i) {
      if( fabs(zones[i].back()[0] - c[0]) + fabs(zones[i].back()[1] - c[1]) <= SAME_ZONE_STEP ) {
        foundMatch = true;
        zones[i].push_back(c);
        break;
      }
    }

    if(!foundMatch) {
      vector<Vec3f> tmp;
      tmp.push_back(c);
      zones.push_back(tmp);
    }
  }

  return zones;
}

Vec3f lastCircle;
int counter = NO_SELECTION_LIMIT;

void chooseCircle(vector<Vec3f>& circles) {

  vector< vector<Vec3f> > zones = findZones(circles);

  int j = -1, mx = 0;
  for(int i = 0; i < zones.size(); ++i) {
    if(mx < zones[i].size()) {
      mx = zones[i].size();
      j = i;
    }
  }

  if(j >= 0) {
    vector<Vec3f> tmp;

    if( counter >= NO_SELECTION_LIMIT || fabs(zones[j].front()[0] - lastCircle[0])
    + fabs(zones[j].front()[1] - lastCircle[1]) <= JUMP_STEP ) {
      lastCircle = zones[j].front();
      tmp.push_back(zones[j].front());
      counter = 0;
    }
    else {
      ++counter;
    }

    circles = tmp;
  }
}

int main(int argc, char** argv)
{
    connect_to_server(argc, argv);
    identify();
    cout << "Built with OpenCV " << CV_VERSION << endl;
    Mat image;
    VideoCapture capture;
    capture.open(0);
    if(capture.isOpened())
    {
        cout << "Capture is opened" << endl;
        for(;;)
        {
            capture >> image;
            if(image.empty())
                break;

            Mat frame;
            GaussianBlur(image, frame, Size(0, 0), 3);
            addWeighted(image, 1.5, frame, -0.5, 0, frame);
            image = frame;

            /*Mat kernel = (Mat_<float>(3,3) <<
              0,  1, 0,
              1, -2, 1,
              0,  1, 0);
            int window_size = 3;*/

            //filter2D(image, image, -1, kernel);

            Mat cimg;
            cvtColor(image, cimg, CV_BGR2GRAY);

            vector<Vec3f> circles;
            HoughCircles(cimg, circles, CV_HOUGH_GRADIENT, 1, 1,
                    120, 25, 30, 50 // change the last two parameters
                    // (min_radius & max_radius) to detect larger circles
                  );

            chooseCircle(circles);

            for( size_t i = 0; i < circles.size(); i++ )
            {
                Vec3i c = circles[i];
                DataProcessor::instance()->SendCursorData(c[0], c[1], 1, serverSock);
                circle( image, Point(c[0], c[1]), c[2], Scalar(0,0,255), 3, LINE_AA);
                circle( image, Point(c[0], c[1]), 2, Scalar(0,255,0), 3, LINE_AA);
            }

            imshow("Sample", image);
            if(waitKey(10) >= 0)
                break;
        }
    }
    else
    {
        cout << "No capture" << endl;
        image = Mat::zeros(480, 640, CV_8UC1);
        imshow("Sample", image);
        waitKey(0);
    }
    return 0;
}
