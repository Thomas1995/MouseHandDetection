#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <unistd.h>

using namespace cv;
using namespace std;

void drawText(Mat & image);

int main()
{
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
            drawText(image);

            Mat cimg;
            cvtColor(image, cimg, CV_BGR2GRAY);
            vector<Vec3f> circles;
            HoughCircles(cimg, circles, CV_HOUGH_GRADIENT, 1, 1,
                    50, 10, 20, 30 // change the last two parameters
                    // (min_radius & max_radius) to detect larger circles
                    );
            for( size_t i = 0; i < circles.size(); i++ )
            {
                Vec3i c = circles[i];
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
        drawText(image);
        imshow("Sample", image);
        waitKey(0);
    }
    return 0;
}

void drawText(Mat & image)
{
    putText(image, "Hello OpenCV",
            Point(20, 50),
            FONT_HERSHEY_COMPLEX, 1, // font face and scale
            Scalar(255, 255, 255), // white
            1, LINE_AA); // line thickness and type
}
