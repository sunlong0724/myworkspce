namespace cpp hawkeye

struct MinMaxStruct {
  1: double min
  2: double max
}

service PlaybackCtrlService{

 i32					get_data_port();
 i32					set_play_frame_rate(1: i32 play_frame_rate, 2: i32 sample_gap),
 i32					set_play_frame_resolution(1: i32 w, 2: i32 h),
 i32					set_store_file(1: i32 flag),
 
 i32					start_grab(),
 i32					stop_grab(),

 i32					play_pause(),
 i32					play_live(1: i32 play_frame_rate, 2: i32 sample_gap),
 i32					play_forward(1: i32 play_frame_rate, 2: i32 sample_gap),
 i32					play_backward(1: i32 play_frame_rate, 2: i32 sample_gap),		
 i32					play_from_a2b(1: i64 from, 2: i64 to),
 
 double       get_camera_grab_fps(),
 double				get_soft_grab_fps(),
 double				get_soft_snd_fps(),
 double       get_file_write_fps(),
 
 i32 					set_exposure_time(1: double microseconds),
 i32 					set_gain_by_sensor_all(1: double gain),
 i32 					set_gain_by_sensor_analog(1: double gain),
 i32 					set_gain_by_sensor_digital(1: double gain),
 i32 					set_frame_rate(1: double rate),
 i32					enable_turbo_transfer(1: i32 enabled),
 i32					set_pixel_fmt(1: string pixel_fmt),
 i32					set_offset_x(1: i32 offset_x),
 i32					set_offset_y(1: i32 offset_y),
 i32					set_image_width(1: i32 width),
 i32					set_image_height(1: i32 height),
 
 
 double 			get_exposure_time(),
 double 			get_gain_by_sensor_all(),
 double 			get_gain_by_sensor_analog(),
 double 			get_gain_by_sensor_digital(), 
 double 			get_frame_rate(), 
 
 MinMaxStruct get_exposure_time_range(),
 MinMaxStruct get_gain_range_by_sensor_all(),
 MinMaxStruct get_gain_range_by_sensor_analog(),
 MinMaxStruct get_gain_range_by_sensor_digital(),
 MinMaxStruct get_frame_rate_range(),
 
 i32				  get_image_width(),
 i32					get_image_height(),
 i32					get_height_max(),
 i32					get_width_max(),
 i32					get_offset_x(),
 i32					get_offset_y(),
 i32					is_enabled_turbo_transfer(),
 string				get_pixel_fmt(),
 string				get_user_defined_name(),
 string				get_device_serial_number(),
 string				get_current_ip_address(),
 
 double				get_grab_fps(),
 double 			get_process_fps(),
 
 string				save_feature(),
 i32  				update_feature(1: string content),
 
}

