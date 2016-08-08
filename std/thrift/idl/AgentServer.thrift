namespace cpp hawkeye

service AgentServerService{
 map<string, map<i32,string>> find_cameras(),
 i32					exec_program(1: string cmdline),
 i32					kill_program(1: i64 process_id),
}

