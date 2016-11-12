#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <unistd.h>
#include "client.h"
#include "include/DataProcessor.h"
#include <deque>

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
LIMITS blue_limit(77, 132, 141, 256, 54, 256);
LIMITS orange_limit(0, 25, 206, 256, 0, 256);
LIMITS green_limit(30, 43, 126, 256, 0, 256);

const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

#define SAME_ZONE_STEP 20
#define JUMP_STEP 50
#define NO_SELECTION_LIMIT 3
#define KEYBOARD_SECTION 50

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
deque<Point> Q;

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

      Q.push_back(lastPoint);
      if(Q.size() >= 10)
        Q.pop_front();

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
      for (auto p : cont)
        if (p.x >= FRAME_WIDTH / 2)
          nrp++;
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
int rightclick;
int prevClick;
bool moved = true;
int moveCount = 0;

double getDist(double x1, double y1, double x2, double y2) {
  return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

int getClick() {
  /*if( dist(Q.front(), Q.back()) <= 30) {
    if(moved)
      click = 1 - click;
    if(++moveCount % 5 == 0)
      moved = false;
  }
  else {
    moved = true;
  }*/

  if (Q.size() < 12)
    return click;

  double meanX = 0;
  double meanY = 0;
  for (auto& it : Q) {
    meanX += it.x;
    meanY += it.y;
  }

  meanX /= Q.size();
  meanY /= Q.size();

  double maxDist = 45;

  int count = 0;
  for (auto it : Q) {
    if (getDist(it.x, it.y, meanX, meanY) > maxDist)
      count++;
    if (count > Q.size() / 5)
      return click;
  }
  Q.clear();
  click = 1 - click;
  return click;

}

vector<vector<int>> keyboard;
long long currentTime = 0;
long long tmpTime = 0;
long long limitTime = 2000;
int enterKey = 0;
char currentLetter = 0;

long long currentLetterTime = 0;
long long tmpLetterTime = 0;
long long limitLetterTime = 1000;

long long debug = 0;

bool shiftPressed, ctrlPressed, ctrlshiftPressed, lastComm, capsLockOn;

vector<string> keys = {
  "a", "b", "c", "d", "e", "f", "g", "h",
  "i", "j", "k", "l", "m", "n", "o", "p",
  "q", "r", "s", "t", "u", "v", "w", "x",
  "y", "z", "spc", "ent", "ctr", "shf", "ctsh", "bks"
};

vector<char> keyCodes {
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
  'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
  'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
  'y', 'z', ' ', '\n', 1, 2, 3, 8
};

int main(int argc, char** argv)
{
  tmpTime = DataProcessor::getTime();
  //connect_to_server(argc, argv);
  //identify();

  keyboard.resize('z' - 'a' + 1);

  Mat cameraFeed;
  Mat threshold;
  Mat HSV;

  VideoCapture capture;

  capture.open(0);
  capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
  capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

  LIMITS lim = orange_limit;
  int nothing = 100;

  while(true) {
    //printf("time img %lu\n", DataProcessor::getTime());
    capture.read(cameraFeed);
    int dump;
    Mat dst;
    flip(cameraFeed, dst, 1);
    cameraFeed = dst;

    //Mat src = cameraFeed;
    Mat src;
    cv::GaussianBlur(cameraFeed, src, cv::Size(0, 0), 3);
    cv::addWeighted(cameraFeed, 1.5, src, -0.5, 0, src);
    cameraFeed = src;
    cvtColor(cameraFeed, HSV, COLOR_BGR2HSV);

    inRange(HSV, Scalar(blue_limit.H_MIN, blue_limit.S_MIN, blue_limit.V_MIN),
    Scalar(blue_limit.H_MAX, blue_limit.S_MAX, blue_limit.V_MAX), threshold);
    morphOps(threshold);
    vector<Point> clickPoints = trackObject(threshold, dump);

     dump = 0;
    for (auto p : clickPoints)
      if (p.x >= FRAME_WIDTH * 0.75)
        dump++;

    if (dump < 1) {
      click = 1;
      currentTime = 0;
      tmpTime = DataProcessor::getTime();
    }
    else {
      click = 0;
      currentTime += DataProcessor::getTime() - tmpTime;
      tmpTime = DataProcessor::getTime();
    }

    /*if (currentTime > limitTime && nothing <= 5) {
        enterKey = 1 - enterKey;
        currentTime = 0;
    }*/

    inRange(HSV, Scalar(lim.H_MIN, lim.S_MIN, lim.V_MIN),
	   Scalar(lim.H_MAX, lim.S_MAX, lim.V_MAX), threshold);

    morphOps(threshold);

    int attempts = 1;
    while (attempts != 0) {
      auto points = trackObject(threshold, nothing);

      attempts--;
      if (points.size() == 0)
        continue;

      //choosePoint(points);
      Q.push_back(points.front());
      if (Q.size() > 20)
        Q.pop_front();

      bool hasEnteredKey = false;
      char enteredKey;

      if (enterKey && click) {
        int xRatio = FRAME_WIDTH / 10;
        int yRatio = FRAME_HEIGHT / 6;
        int x = points.front().x / xRatio;
        int y = points.front().y / yRatio;

        if(y == 0) {
          goto JUMP;
        }

        int idx = (y - 1) * 8 + x;
        if(idx < 0)
          goto JUMP;
        if(idx > keys.size())
          goto JUMP;

        char newLetter = keyCodes[idx];

        if (currentLetter == newLetter) {
          currentLetterTime += DataProcessor::getTime() - tmpLetterTime;
        } else {
          currentLetterTime = 0;
        }
        currentLetter = newLetter;
        tmpLetterTime = DataProcessor::getTime();
        //if (currentLetterTime >= limitLetterTime) {
        if (prevClick != click) {
          if(currentLetter == 8) {
            ctrlPressed = shiftPressed = ctrlshiftPressed = false;
          }

          if(currentLetter == 1) {
            if(shiftPressed == false)
              ctrlPressed = true;
            else
              ctrlshiftPressed = true,
              shiftPressed = false;
          }
          else if(currentLetter == 2) {
            if(ctrlPressed == false)
              shiftPressed = true;
            else
              ctrlshiftPressed = true,
              ctrlPressed = false;
          }
          else if(currentLetter == 3) {
            ctrlshiftPressed = true;
            shiftPressed = ctrlPressed = false;
          }
          else {
            if(shiftPressed) {
              if(currentLetter >= 'a' && currentLetter <= 'z' && !capsLockOn)
                currentLetter -= 'a' - 'A';

              cerr << currentLetter;
              lastComm = false;
            }
            else if(ctrlPressed) {
              if(!lastComm) cerr<<"\n";
              cerr << "CTRL + " << currentLetter << "\n";
              lastComm = true;
            }
            else if(ctrlshiftPressed) {
              if(!lastComm) cerr<<"\n";
              cerr << "CTRL + SHIFT + " << currentLetter << "\n";
              lastComm = true;
            }
            else {
              if(capsLockOn) {
                if(currentLetter >= 'a' && currentLetter <= 'z')
                  currentLetter -= 'a' - 'A';

                cerr << currentLetter;
              }
              else
                cerr << currentLetter;
              lastComm = false;

              if(currentLetter == 8) {
                cerr << ' ' << (char)8;
              }
            }

            ctrlPressed = shiftPressed = ctrlshiftPressed = false;
          }

          //fflush(cout);
          hasEnteredKey = true;
          enteredKey = currentLetter;
          currentLetterTime = 0;
        }
      }

JUMP:

      if(!points.empty()) {
        auto p = points.front();
        //cout << "p.x: " << p.x << " " << "p.y: " << p.y << endl;
        //DataProcessor::instance()->SendCursorData(p.x, p.y, click/*getClick()*/, serverSock);
        if(!hasEnteredKey)
          enteredKey = 0;

        rightclick = 0;
        //DataProcessor::instance()->SendInputDataWin(p.x, p.y, enteredKey, click, rightclick, serverSock);

        if(p.x <= KEYBOARD_SECTION && FRAME_HEIGHT - p.y <= KEYBOARD_SECTION)
          capsLockOn = true;
        if(FRAME_WIDTH - p.x <= KEYBOARD_SECTION && FRAME_HEIGHT - p.y <= KEYBOARD_SECTION)
          capsLockOn = false;

        circle(cameraFeed, p, 10, Scalar(0, 0, 255));
        break;
      }
    }

    enterKey = 1;

    if (enterKey) {
      int xRatio = FRAME_WIDTH / 10;
      int yRatio = FRAME_HEIGHT / 6;
      for (int i = 1; i <= 8; i++)
        line(cameraFeed, Point(i * xRatio, yRatio), Point(i * xRatio - 2, FRAME_HEIGHT - yRatio), Scalar(0, 255, 0));

      for (int i = 1; i <= 5; i++)
        line(cameraFeed, Point(0, i * yRatio), Point(FRAME_WIDTH - 2 * xRatio, i * yRatio), Scalar(0, 255, 0));

      int nr = 0;
      for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 8; i++) {
          string s = keys[nr++];

          putText(cameraFeed, s.c_str(), cvPoint(xRatio * i + 5, yRatio * (j + 2) - yRatio / 2),
              FONT_HERSHEY_COMPLEX, 0.8, cvScalar(0,0,255), 1, CV_AA);
        }
      }
    }

    if(!capsLockOn) {
      line(cameraFeed, Point(0, FRAME_HEIGHT - KEYBOARD_SECTION),
        Point(KEYBOARD_SECTION, FRAME_HEIGHT - KEYBOARD_SECTION), Scalar(0, 0, 255));
      line(cameraFeed, Point(KEYBOARD_SECTION, FRAME_HEIGHT),
        Point(KEYBOARD_SECTION, FRAME_HEIGHT - KEYBOARD_SECTION), Scalar(0, 0, 255));
    }
    else {
      line(cameraFeed, Point(FRAME_WIDTH - KEYBOARD_SECTION, FRAME_HEIGHT),
        Point(FRAME_WIDTH - KEYBOARD_SECTION, FRAME_HEIGHT - KEYBOARD_SECTION), Scalar(0, 0, 255));
      line(cameraFeed, Point(FRAME_WIDTH, FRAME_HEIGHT - KEYBOARD_SECTION),
        Point(FRAME_WIDTH - KEYBOARD_SECTION, FRAME_HEIGHT - KEYBOARD_SECTION), Scalar(0, 0, 255));
    }

    for (auto p : clickPoints)
      circle(cameraFeed, p, 10, Scalar(0, 255, 0));
    imshow("Camera", cameraFeed);
    prevClick = click;
    waitKey(10);
  }

  return 0;
}
