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

            cv::Mat hsv_image;
            medianBlur(image, image, 5);
            cv::cvtColor(image, hsv_image, cv::COLOR_BGR2HSV);
            // Threshold the HSV image, keep only the red pixels
            cv::Mat lower_red_hue_range;
            cv::Mat upper_red_hue_range;
            cv::inRange(hsv_image, cv::Scalar(0, 100, 100), cv::Scalar(50, 255, 255), lower_red_hue_range);
            cv::inRange(hsv_image, cv::Scalar(160, 100, 100), cv::Scalar(179, 255, 255), upper_red_hue_range);
            cv::Mat red_hue_image;
            cv::addWeighted(lower_red_hue_range, 1.0, upper_red_hue_range, 1.0, 0.0, red_hue_image);

            cv::GaussianBlur(red_hue_image, red_hue_image, cv::Size(9, 9), 2, 2);
            std::vector<cv::Vec3f> circles;

            cv::HoughCircles(red_hue_image, circles, CV_HOUGH_GRADIENT, 1, red_hue_image.rows/8, 100, 20, 20, 30);

            Mat cimg;
            for(size_t current_circle = 0; current_circle < circles.size(); ++current_circle) {
                cv::Point center(std::round(circles[current_circle][0]), std::round(circles[current_circle][1]));
                int radius = std::round(circles[current_circle][2]);

                cv::circle(image, center, radius, cv::Scalar(0, 255, 0), 5);
            }

            imshow("Sample", image);
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
