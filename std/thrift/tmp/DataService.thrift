namespace cpp hawkeye

struct MinMaxStruct {
  1: double min
  2: double max
}

service DataService{

 i32					set_display_frame_rate(1: string camera_name, 2: i32 display_frame_rate, 3: i32 grab_frame_rate),

 
 i32 					set_exposure_time(1: string camera_name, 2: double microseconds),
 i32 					set_gain_by_sensor_all(1: string camera_name, 2: double gain),
 i32 					set_gain_by_sensor_analog(1: string camera_name, 2: double gain),
 i32 					set_gain_by_sensor_digital(1: string camera_name, 2: double gain),
 i32 					set_frame_rate(1: string camera_name, 2: double rate),
 i32					enable_turbo_transfer(1: string camera_name, 2: i32 enabled),
 i32					set_pixel_fmt(1: string camera_name, 2: string pixel_fmt),
 i32					set_offset_x(1: string camera_name, 2: i32 offset_x),
 i32					set_offset_y(1: string camera_name, 2: i32 offset_y),
 i32					set_image_width(1: string camera_name, 2: i32 width),
 i32					set_image_height(1: string camera_name, 2: i32 height),
 
 
 double 			get_exposure_time(1: string camera_name),
 double 			get_gain_by_sensor_all(1: string camera_name),
 double 			get_gain_by_sensor_analog(1: string camera_name),
 double 			get_gain_by_sensor_digital(1: string camera_name), 
 double 			get_frame_rate(1: string camera_name), 
 
 MinMaxStruct get_exposure_time_range(1: string camera_name),
 MinMaxStruct get_gain_range_by_sensor_all(1: string camera_name),
 MinMaxStruct get_gain_range_by_sensor_analog(1: string camera_name),
 MinMaxStruct get_gain_range_by_sensor_digital(1: string camera_name),
 MinMaxStruct get_frame_rate_range(1: string camera_name),
 
 i32				  get_image_width(1: string camera_name),
 i32					get_image_height(1: string camera_name),
 i32					get_height_max(1: string camera_name),
 i32					get_width_max(1: string camera_name),
 i32					get_offset_x(1: string camera_name),
 i32					get_offset_y(1: string camera_name),
 i32					is_enabled_turbo_transfer(1: string camera_name),
 string				get_pixel_fmt(1: string camera_name),
 string				get_user_defined_name(1: string camera_name),
 string				get_device_serial_number(1: string camera_name),
 string				get_current_ip_address(1: string camera_name),
 
 double				get_grab_fps(1: string camera_name),
 double 			get_process_fps(1: string camera_name),
 
 string				save_feature(1: string camera_name),
 i32  				update_feature(1: string camera_name, 2: string content),
 
 i64					ping_server(1: i64 seq),//_return == seq+1
 
}

