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

struct LIMITS {
  int H_MIN;
  int H_MAX;
  int S_MIN;
  int S_MAX;
  int V_MIN;
  int V_MAX;

  LIMITS(int hmn, int hmx, int smn, int smx, int vmn, int vmx)
    : H_MIN(hmn), H_MAX(hmx), S_MIN(smn), S_MAX(smx), V_MIN(vmn), V_MAX(vmx) {}
};

//LIMITS red_limit(0, 10, 135, 177, 74, 256);
//LIMITS red_limit(0, 10, 143, 256, 86, 162);
LIMITS red_limit(0, 14, 196, 222, 0, 255);
LIMITS blue_limit(77, 131, 63, 141, 75, 133);
LIMITS orange_limit(0, 25, 206, 256, 0, 256);
LIMITS green_limit(35, 139, 38, 58, 175, 256);

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

inline double dist(Point a, Point b) {
  return sqrt( (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) );
}

Point chooseClosestTo(Point target, vector<Point>& points) {
  double distMin = dist(target, points[0]);
  int idx = 0;

  for(int i = 1; i < points.size(); ++i) {
    double d = dist(target, points[i]);
    if(d < distMin) {
      distMin = d;
      idx = i;
    }
  }

  return points[idx];
}

Point lastPoint;
bool lastPointExists = false;
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
      Point p;
      if(!lastPointExists) {
        p = zones[j].front();
        lastPointExists = true;
      }
      else {
        p = chooseClosestTo(lastPoint, zones[j]);
      }

      lastPoint = p;
      tmp.push_back(lastPoint);
      counter = 0;
    }
    else {
      ++counter;
    }

    points = tmp;
  }
}

vector<Point> trackObject(Mat& threshold, int& nrp) {
	vector<Point> points;
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(threshold, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

  nrp = 0;
  for(auto cont : contours) {
    nrp += cont.size();
  }

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
int click;

int getClick(Mat& img) {
  LIMITS lim = green_limit;
  Mat HSV, threshold;
  img.copyTo(HSV);

  inRange(HSV, Scalar(lim.H_MIN, lim.S_MIN, lim.V_MIN),
   Scalar(lim.H_MAX, lim.S_MAX, lim.V_MAX), threshold);

  morphOps(threshold);

  int nrp;
  trackObject(threshold, nrp);

  cout << nrp << "\n";

  if(nrp >= 20)
    return 0;

  return 1;
}

int main(int argc, char** argv)
{
  connect_to_server(argc, argv);
  identify();

  Mat cameraFeed;
  Mat threshold;
  Mat HSV;

  VideoCapture capture;

  capture.open(0);
  capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
  capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

  LIMITS lim = orange_limit;

  while(true) {
    capture.read(cameraFeed);

    Mat dst;
    flip(cameraFeed, dst, 1);
    cameraFeed = dst;

    Mat src = cameraFeed;

    cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);

    click = getClick(HSV);

    inRange(HSV, Scalar(lim.H_MIN, lim.S_MIN, lim.V_MIN),
	   Scalar(lim.H_MAX, lim.S_MAX, lim.V_MAX), threshold);

    morphOps(threshold);

    int dump;
    auto points = trackObject(threshold, dump);
    choosePoint(points);

    if(!points.empty()) {
      auto p = points.front();
      DataProcessor::instance()->SendCursorData(p.x, p.y, click, serverSock);
      circle(cameraFeed, p, 10, Scalar(0, 0, 255));
    }

    imshow("Camera", cameraFeed);

    waitKey(10);
  }

  return 0;
}
