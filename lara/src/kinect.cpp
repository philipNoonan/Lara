#include "kinect.hpp"
#include <iostream>

#include <chrono>

// Constructor
kinect::kinect( const uint32_t index )
    : device_index( index )
{
    // Initialize
    initialize();
}

kinect::~kinect()
{
    // Finalize
    finalize();
}

// Initialize
void kinect::initialize()
{
    // Initialize Sensor
    initialize_sensor();
}

// Initialize Sensor
inline void kinect::initialize_sensor()
{
    // Get Connected Devices
    const int32_t device_count = k4a::device::get_installed_count();
    if( device_count == 0 ){
        throw k4a::error( "Failed to found device!" );
    }

    // Open Default Device
    device = k4a::device::open( device_index );

    // Start Cameras with Configuration
    device_configuration = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    device_configuration.color_format             = k4a_image_format_t::K4A_IMAGE_FORMAT_COLOR_MJPG;
    device_configuration.color_resolution         = k4a_color_resolution_t::K4A_COLOR_RESOLUTION_1440P;
    device_configuration.depth_mode               = k4a_depth_mode_t::K4A_DEPTH_MODE_WFOV_2X2BINNED;
    device_configuration.synchronized_images_only = true;
    device_configuration.wired_sync_mode          = k4a_wired_sync_mode_t::K4A_WIRED_SYNC_MODE_STANDALONE;
    device_configuration.disable_streaming_indicator = true;
    device.start_cameras( &device_configuration );

    // Get Calibration
    calibration = device.get_calibration( device_configuration.depth_mode, device_configuration.color_resolution );

    // Create Transformation
    transformation = k4a::transformation( calibration );

    ir_image_copy.resize(calibration.depth_camera_calibration.resolution_width * calibration.depth_camera_calibration.resolution_width);
}

// Finalize
void kinect::finalize()
{
    // Destroy Transformation
    transformation.destroy();

    // Stop Cameras
    device.stop_cameras();

    // Close Device
    device.close();

    // Close Window
    //cv::destroyAllWindows();
}

// Run
void kinect::run()
{
    // Main Loop
    while( true ){
        // Update
        update();

        // Draw
        draw();

        // Show
        show();

        // Wait Key
        constexpr int32_t delay = 30;
        //const int32_t key = cv::waitKey( delay );
        //if( key == 'q' ){
        //    break;
        //}
    }
}

// Update
void kinect::update()
{
    // Update Frame
    update_frame();

    // Update Color
    update_color();

    // Update Depth
    update_depth();

    // Update Infra
    update_infra();

    // Update Transformation
   // update_transformation();

    // Release Capture Handle
    capture.reset();
}

// Update Frame
inline void kinect::update_frame()
{
    // Get Capture Frame
    constexpr std::chrono::milliseconds time_out( K4A_WAIT_INFINITE );
    const bool result = device.get_capture( &capture, time_out );
    if( !result ){
        this->~kinect();
    }
}

// Update Color
inline void kinect::update_color()
{
    // Get Color Image
    color_image = capture.get_color_image();
    //std::cout << "color_image size " << color_image.get_size() << std::endl;
    
}

// Update Depth
inline void kinect::update_depth()
{
    // Get Depth Image
    depth_image = capture.get_depth_image();
}

// Update Depth
inline void kinect::update_infra()
{
    // Get Depth Image
    infra_image = capture.get_ir_image();
    uint16_t* ptr = (uint16_t*)infra_image.get_buffer();


    for (int i = 0; i < ir_image_copy.size(); i++) {  
        ir_image_copy[i] = (uint8_t)(((float)(*(ptr + i)) * m_invgainIR) + m_biasIR);
        
        
    }
}

void kinect::set_ir_invgain_bias(float invgain, float bias) {
    m_invgainIR = invgain;
    m_biasIR = bias;
}


// Update Transformation
inline void kinect::update_transformation()
{
    if( !color_image.handle() || !depth_image.handle() ){
        return;
    }

    // Transform Color Image to Depth Camera
    transformed_color_image = transformation.color_image_to_depth_camera( depth_image, color_image );

    // Transform Depth Image to Color Camera
    transformed_depth_image = transformation.depth_image_to_color_camera( depth_image );
}

uint8_t * kinect::get_color_buffer() {
    return color_image.get_buffer();
}

size_t kinect::get_color_buffer_size() {
    return color_image.get_size();
}

uint8_t * kinect::get_depth_buffer() {
    return depth_image.get_buffer();
}

size_t kinect::get_depth_buffer_size() {
    return depth_image.get_size();
}

uint8_t * kinect::get_infra_buffer() {
    return ir_image_copy.data();
}

size_t kinect::get_infra_buffer_size() {
    return ir_image_copy.size();
}

// Draw
void kinect::draw()
{
    // Draw Color
    draw_color();

    // Draw Depth
    draw_depth();

    // Draw Transformation
   // draw_transformation();
}

// Draw Color
inline void kinect::draw_color()
{
    if( !color_image.handle() ){
        return;
    }

    // Get cv::Mat from k4a::image
    //color = k4a::get_mat( color_image );

    // Release Color Image Handle
    color_image.reset();
}

// Draw Depth
inline void kinect::draw_depth()
{
    if( !depth_image.handle() ){
        return;
    }

    // Get cv::Mat from k4a::image
    //depth = k4a::get_mat( depth_image );

    // Release Depth Image Handle
    depth_image.reset();
}

// Draw Transformation
inline void kinect::draw_transformation()
{
    if( !transformed_color_image.handle() || !transformed_depth_image.handle() ){
        return;
    }

    // Get cv::Mat from k4a::image
    // transformed_color = k4a::get_mat( transformed_color_image );
    // transformed_depth = k4a::get_mat( transformed_depth_image );

    // Release Transformed Image Handle
    transformed_color_image.reset();
    transformed_depth_image.reset();
}

// Show
void kinect::show()
{
    // Show Color
    show_color();

    // Show Depth
    show_depth();

    // Show Transformation
   // show_transformation();
}

// Show Color
inline void kinect::show_color()
{
    // if( color.empty() ){
    //     return;
    // }

    // // Show Image
    // const cv::String window_name = cv::format( "color (kinect %d)", device_index );
    // cv::imshow( window_name, color );
}

// Show Depth
inline void kinect::show_depth()
{
    // if( depth.empty() ){
    //     return;
    // }

    // // Scaling Depth
    // depth.convertTo( depth, CV_8U, -255.0 / 5000.0, 255.0 );

    // // Show Image
    // const cv::String window_name = cv::format( "depth (kinect %d)", device_index );
    // cv::imshow( window_name, depth );
}

// Show Transformation
inline void kinect::show_transformation()
{
    // if( transformed_color.empty() || transformed_depth.empty() ){
    //     return;
    // }

    // // Scaling Depth
    // transformed_depth.convertTo( transformed_depth, CV_8U, -255.0 / 5000.0, 255.0 );

    // // Show Image
    // cv::String window_name;
    // window_name = cv::format( "transformed color (kinect %d)", device_index );
    // cv::imshow( window_name, transformed_color );
    // window_name = cv::format( "transformed depth (kinect %d)", device_index );
    // cv::imshow( window_name, transformed_depth );
}
