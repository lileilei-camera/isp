#include "cv.h"
#include "highgui.h"
#include <iostream>
#include <opencv2/opencv.hpp>


// One (and only one) of your C++ files must define CVUI_IMPLEMENTATION
// before the inclusion of cvui.h to ensure its implementaiton is compiled.
#define CVUI_IMPLEMENTATION
#include "../../cvui/cvui.h"

#if 0
#define WINDOW_NAME "CVUI Hello World!"
int cvui_test()
{
	// Create a frame where components will be rendered to.
	cv::Mat frame = cv::Mat(1920, 1080, CV_8UC3);

	// Init cvui and tell it to create a OpenCV window, i.e. cv::namedWindow(WINDOW_NAME).
	cvui::init(WINDOW_NAME);

	while (true) {
		// Fill the frame with a nice color
		frame = cv::Scalar(49, 52, 49);

		// Render UI components to the frame
		cvui::text(frame, 110, 80, "Hello, world!");
		cvui::text(frame, 110, 120, "cvui is awesome!");

		// Update cvui stuff and show everything on the screen
		cvui::imshow(WINDOW_NAME, frame);

        char ch=cv::waitKey(1000);
		if (ch == 'q') {
			break;
		}
	}

	return 0;
}
#elif 1
#define WINDOW_NAME	"Button shortcut"

int cvui_test()
{

	cv::Mat frame = cv::Mat(cv::Size(650, 150), CV_8UC3);



	// Init cvui and tell it to use a value of 20 for cv::waitKey()

	// because we want to enable keyboard shortcut for

	// all components, e.g. button with label "&Quit".

	// If cvui has a value for waitKey, it will call

	// waitKey() automatically for us within cvui::update().

	cvui::init(WINDOW_NAME, 20);



	while (true) {

		frame = cv::Scalar(49, 52, 49);



		cvui::text(frame, 40, 40, "To exit this app click the button below or press Q (shortcut for the button below).");



		// Exit the application if the quit button was pressed.

		// It can be pressed because of a mouse click or because 

		// the user pressed the "q" key on the keyboard, which is

		// marked as a shortcut in the button label ("&Quit").

		if (cvui::button(frame, 300, 80, "&Quit")) {

			break;

		}

		// Since cvui::init() received a param regarding waitKey,

		// there is no need to call cv::waitKey() anymore. cvui::update()

		// will do it automatically.

		cvui::update();

		

		cv::imshow(WINDOW_NAME, frame);

	}



	return 0;

}
#endif

