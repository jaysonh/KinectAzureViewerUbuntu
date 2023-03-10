#include <k4a/k4a.h>

#include <opencv4/opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

using namespace cv;

int main()
{
    // getting the capture
    k4a_capture_t capture;

    const int32_t TIMEOUT_IN_MS = 1000;


    uint32_t deviceCount = k4a_device_get_installed_count();
    k4a_device_t device = NULL;
    if (deviceCount == 0)
    {
        printf("No k4a devices attached!\n");
        return 1;
    }else
    {
        printf("%i devices found\n", deviceCount);
        for (uint8_t deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++)
        {
            if (K4A_RESULT_SUCCEEDED != k4a_device_open(deviceIndex, &device))
            {
                printf("%d: Failed to open device\n", deviceIndex);
                continue;
            }else
            {
                size_t serial_size = 0;
                k4a_device_get_serialnum(device, NULL, &serial_size);

                // Allocate memory for the serial, then acquire it
                char *serial = (char*)(malloc(serial_size));
                k4a_device_get_serialnum(device, serial, &serial_size);
                printf("Device Found: %s\n", serial);
            }
            k4a_device_close(device);
        }
    }

    // Open the first plugged in Kinect device
    if (K4A_FAILED(k4a_device_open(K4A_DEVICE_DEFAULT, &device)))
    {
        printf("Failed to open k4a device!\n");
        return 1;
    }

    // Get the size of the serial number
    size_t serial_size = 0;
    k4a_device_get_serialnum(device, NULL, &serial_size);

    // Allocate memory for the serial, then acquire it
    char *serial = (char*)(malloc(serial_size));
    k4a_device_get_serialnum(device, serial, &serial_size);
    printf("Opened device: %s\n", serial);
    free(serial);

    // Configure a stream of 4096x3072 BRGA color data at 15 frames per second
    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    config.camera_fps       = K4A_FRAMES_PER_SECOND_30;
    config.color_format     = K4A_IMAGE_FORMAT_COLOR_BGRA32;
    config.color_resolution = K4A_COLOR_RESOLUTION_1080P;
    config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    config.synchronized_images_only = true;

    // Start the camera with the given configuration
    k4a_device_start_cameras(device, &config);

    int frame_count = 0;

    bool running = true;
    while( running )
    {
        k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS);
        k4a_image_t color_image = k4a_capture_get_color_image(capture);

        uint8_t* color_buffer = k4a_image_get_buffer(color_image);

        int rows = k4a_image_get_height_pixels(color_image);
        int cols = k4a_image_get_width_pixels(color_image);

        cv::Mat color(rows, cols, CV_8UC4, (void*)color_buffer, cv::Mat::AUTO_STEP);

        cv::imshow("color", color);

        k4a_image_t depth_image = k4a_capture_get_depth_image(capture);
        //uint8_t* depth_buffer = k4a_image_get_buffer(depth_image);
        //int depth_rows = k4a_image_get_height_pixels(depth_image);
        //int depth_cols = k4a_image_get_width_pixels (depth_image);
        ////cv::Mat depth(depth_rows, depth_cols, CV_8UC4, (void*)depth_buffer, cv::Mat::AUTO_STEP);
        //cv::Mat depth(depth_rows,depth_cols, CV_8UC1, (void*)depth_buffer, cv::Mat::AUTO_STEP);
        //cv::imshow("depth image",depth);
        uint8_t* buffer = k4a_image_get_buffer( depth_image );
        int32_t width = k4a_image_get_width_pixels( depth_image );
        int32_t height = k4a_image_get_height_pixels( depth_image );
        cv::Mat depth_mat = cv::Mat( height, width, CV_16UC1, reinterpret_cast<uint16_t*>( buffer ) );
        depth_mat.convertTo( depth_mat, CV_8U, 255.0 / 5000.0, 0.0 );
        cv::imshow("depth image",depth_mat);
        
        if(cv::waitKey(10) == 27)
        {
            running = false;
        }
    }
    cv::destroyAllWindows();

    k4a_device_stop_cameras(device);
    k4a_device_close(device);
    return 0;
}
