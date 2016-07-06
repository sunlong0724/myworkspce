#ifndef __SEND_DATA_IMPL_H__
#define __SEND_DATA_IMPL_H__

#include <string>
#include <vector>

typedef struct _CustomData {

	void		*context;
	void		*socket;
	std::vector<char> buffer;

	int64_t		counter;
	int64_t		frame_gap;
	int64_t		display_frame_rate;
	int64_t		last_display_no;

	int64_t		recv_seq;
	int64_t		last_recv_seq;


}CustomData;

int64_t get_current_time_in_ms();

void create_zqm_ctx(CustomData* data, int32_t buffer_len,const std::string& ip, uint16_t port, int type);

void destroy_zqm_ctx(CustomData* data);

int set_display_frame_rate0(CustomData* ctx, int64_t dispaly_rate, int64_t grab_rate);

int32_t send_data(CustomData* ctx, char* data, int data_len);

int32_t recv_data(CustomData* ctx);

int SinkBayerDatasCallbackImpl(unsigned char* buffer, int buffer_len, void* ctx);

#endif