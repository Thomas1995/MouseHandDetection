#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <unistd.h>
#include "client.h"
#include "include/DataProcessor.h"

using namespace std;
using namespace cv;

int H_MIN = 0;
int H_MAX = 10;
int S_MIN = 135;
int S_MAX = 177;
int V_MIN = 74;
int V_MAX = 256;

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

#define SAME_ZONE_STEP 20
#define JUMP_STEP 50
#define NO_SELECTION_LIMIT 3

vector< vector<Point> > findZones(vector<Point>& points) {
  vector< vector<Point> > zones;

  for(auto p : points) {
    bool foundMatch = false;
    for(int i = 0; i < zones.size(); ++i) {
      if( fabs(zones[i].back().x - p.x) + fabs(zones[i].back().y - p.y) <= SAME_ZONE_STEP ) {
        foundMatch = true;
        zones[i].push_back(p);
        break;
      }
    }

    if(!foundMatch) {
      vector<Point> tmp;
      tmp.push_back(p);
      zones.push_back(tmp);
    }
  }

  return zones;
}

Point lastPoint;
int counter = NO_SELECTION_LIMIT;

void choosePoint(vector<Point>& points) {

  vector< vector<Point> > zones = findZones(points);

  int j = -1, mx = 0;
  for(int i = 0; i < zones.size(); ++i) {
    if(mx < zones[i].size()) {
      mx = zones[i].size();
      j = i;
    }
  }

  if(j >= 0) {
    vector<Point> tmp;

    if( counter >= NO_SELECTION_LIMIT || fabs(zones[j].front().x - lastPoint.x)
    + fabs(zones[j].front().y - lastPoint.y) <= JUMP_STEP ) {
      lastPoint = zones[j].front();
      tmp.push_back(zones[j].front());
      counter = 0;
    }
    else {
      ++counter;
    }

    points = tmp;
  }
}

vector<Point> trackObject(Mat& threshold, Mat& cameraFeed) {
	vector<Point> points;
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(threshold, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		for (int index = 0; index >= 0; index = hierarchy[index][0]) {
			Moments moment = moments((cv::Mat)contours[index]);
			double area = moment.m00;

			double X = moment.m10 / area;
			double Y = moment.m01 / area;

			points.push_back(Point(X, Y));
		}
	}

	return points;
}

void morphOps(Mat &thresh){
	Mat erodeElement = getStructuringElement( MORPH_RECT, Size(3,3) );
	Mat dilateElement = getStructuringElement( MORPH_RECT, Size(8,8) );

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
}

extern int serverSock;

int main(int argc, char** argv)
{
  connect_to_server(argc, argv);
  identify();

	Mat cameraFeed;
	Mat threshold;
	Mat HSV;

	VideoCapture capture;

	capture.open(0);
	capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);

	while(true) {
		capture.read(cameraFeed);
		Mat src = cameraFeed;

		cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);

		inRange(HSV, Scalar(H_MIN, S_MIN, V_MIN), Scalar(H_MAX, S_MAX, V_MAX), threshold);
	  morphOps(threshold);

		auto points = trackObject(threshold, cameraFeed);
		choosePoint(points);

		if(!points.empty()) {
      auto p = points.front();
      DataProcessor::instance()->SendCursorData(p.x, p.y, 1, serverSock);
			circle(cameraFeed, p, 10, Scalar(0,0,255));
		}

		imshow("Camera", cameraFeed);

		waitKey(10);
	}

	return 0;
}
