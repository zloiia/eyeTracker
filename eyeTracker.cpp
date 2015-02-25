// eyeTracker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "opencv2/opencv.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <thread>
#include <vector>
#include <list>
#include <sstream>


using namespace cv;
using namespace std;

typedef std::list<Point> res_t;
typedef std::list<thread> workers_t;


///------- template matching -----------------------------------------------------------------------------------------------

Mat TplMatch(Mat &img, Mat &mytemplate)
{
	Mat result;

	matchTemplate(img, mytemplate, result, CV_TM_SQDIFF_NORMED);
	normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

	return result;
}


///------- Localizing the best match with minMaxLoc ------------------------------------------------------------------------

Point minmax(Mat &result)
{
	double minVal, maxVal;
	Point  minLoc, maxLoc, matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	matchLoc = minLoc;

	return matchLoc;
}


///------- tracking --------------------------------------------------------------------------------------------------------

void track(cv::Mat original, cv::Mat &templ, res_t::iterator it )
{
	Mat result = TplMatch(original, templ);
	Point match = minmax(result);
	*it = match;
}


///------- Main() ----------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cerr << "Not find argument" << endl;
		return -1;
	}

	std::string videofile(argv[1]);
	//std::cout << videofile << std::endl;
	VideoCapture cap;
	cap.open(videofile);
	if (!cap.isOpened())
	{
		cout << "Unable to open video file" << endl;    return -1;
	}

	Mat roiImg = imread("template.png", 1);
	if (roiImg.empty())
	{
		return 0;
	}

	unsigned int cores = std::thread::hardware_concurrency();
	res_t show_results(cores);
	
	workers_t workers;

	workers.resize(cores);
	show_results.resize(cores);
	while (1)
	{
		bool stop = false;
		workers.clear();
		for (res_t::iterator i = show_results.begin(); i != show_results.end(); ++i)
		{
			Mat or;
			*i = Point();
			cap >> or;
			if (or.empty())
			{
				stop = true;
				break;
			}
			workers.push_back (thread(track, or, roiImg, i));
		}

		
		//ждем, пока все треды закончат работу
		for (workers_t::iterator i = workers.begin(); i != workers.end(); ++i)
		{
			(*i).join();
			
		}

		res_t::iterator it = show_results.begin();
		for (workers_t::iterator i = workers.begin(); i != workers.end(); ++i)
		{
			std::cout << (*it).x << " ; " << (*it).y << std::endl;
			++it;
		}
		
		if (stop) break;

	}
	
	//std::cin >> cores;

	return 0;
}