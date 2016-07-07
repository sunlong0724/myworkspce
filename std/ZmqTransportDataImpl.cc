#include "ZmqTransportDataImpl.h"
#include "zmq.h"
#include <string>
#include <chrono>

#include <opencv2/opencv.hpp>

int64_t get_current_time_in_ms() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void create_zqm_ctx(CustomData* custom_data, int32_t buffer_len,const std::string& server_ip, uint16_t port, int type) {
	//  Connect to task ventilator
	custom_data->context = zmq_ctx_new();

	std::string addr("tcp://");
	

	if (ZMQ_PUB == type) {
		addr.append("*:" + std::to_string(port));
		custom_data->socket = zmq_socket(custom_data->context, ZMQ_PUB);
		//zmq_socket_set_sndhwm(self->mailbox, PEER_EXPIRED * 100);
		int sndhwm = 2000;
		zmq_setsockopt(custom_data->socket, ZMQ_SNDHWM, &sndhwm, sizeof sndhwm);
		zmq_bind(custom_data->socket, addr.c_str());
	}
	else {//ZMQ_SUB
		addr.append(server_ip + ":" + std::to_string(port));
		custom_data->socket = zmq_socket(custom_data->context, ZMQ_SUB);
		zmq_connect(custom_data->socket, addr.c_str());
		zmq_setsockopt(custom_data->socket, ZMQ_SUBSCRIBE,	"", 0);
		//int recvdhwm = 5000;
		//zmq_setsockopt(data->socket, ZMQ_RCVHWM, &recvdhwm, sizeof recvdhwm);
	}

	custom_data->display_frame_rate = custom_data->last_display_no = custom_data->recv_seq =  custom_data->counter = 0L;
	custom_data->last_recv_seq = 0L;
	std::vector<char> tmp(sizeof int64_t + sizeof int64_t + buffer_len, 0x00);
	custom_data->buffer = tmp;

	custom_data->store_file_flag = FALSE;
	custom_data->hFile = NULL;

	if (custom_data->hFile == NULL) {
		custom_data->hFile = CreateFile(custom_data->path_name, GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
		if (!custom_data->hFile) {
			fprintf(stderr, "fopen failed!!!\n");
			//exit(0);
		}
	}
}

void destroy_zqm_ctx(CustomData* ctx) {
	zmq_close(ctx->socket);
	zmq_ctx_destroy(ctx->context);
}

int set_display_frame_rate0(CustomData* ctx, int64_t dispaly_rate, int64_t grab_rate) {
	ctx->frame_gap = (grab_rate+1) / dispaly_rate;
	return ctx->display_frame_rate = dispaly_rate;
}

int set_store_flag(CustomData* ctx, BOOL flag) {
	return ctx->store_file_flag = flag;
}

int32_t send_data(CustomData* ctx, char* data, int data_len) {
	int ret = zmq_send(ctx->socket, ctx->buffer.data(), ctx->buffer.size(), 0);

	if (ret < 0) {
		int a = 0;
	}
	return ret;
}

//IplImage* image = NULL;
//IplImage * pRGB = NULL;

int32_t recv_data(CustomData* ctx) {
	int ret = zmq_recv(ctx->socket, (void*)ctx->buffer.data(), ctx->buffer.size(), 0);

	int64_t timestamp = 0;
	memcpy(&timestamp, ctx->buffer.data(), sizeof int64_t);
	memcpy(&ctx->recv_seq, &ctx->buffer[sizeof int64_t], sizeof int64_t);

	//printf("recv %d bytes, seq:%lld, timestamp:%lld\n", ret, ctx->seq, ctx->last_seq, timestamp);
	//if (!image) {
	//	image = cvCreateImageHeader(cvSize(1280, 720), 8, 1);
	//	pRGB = cvCreateImage(cvSize(1280, 720), 8, 3);
	//}
	//cvSetData(image, &ctx->buffer[sizeof int64_t + sizeof int64_t], 1280);
	//cvCvtColor(image, pRGB, CV_BayerRG2RGB);
	//cvShowImage("win", pRGB);
	//cvWaitKey(1);


	if (ctx->recv_seq != ctx->last_recv_seq + ctx->frame_gap) {
		printf("##########recv %d bytes, seq:%lld,last_seq:%lld,frame_gap:%lld, timestamp:%ll\n", ret, ctx->recv_seq, ctx->last_recv_seq, ctx->frame_gap, timestamp);
	}
	else {
		printf("recv %d bytes, seq:%lld,last_seq:%lld, timestamp:%lld\n", ret, ctx->recv_seq, ctx->last_recv_seq, timestamp);
	}
	ctx->last_recv_seq = ctx->recv_seq;

	return ctx->buffer.size();
}

int SinkBayerDatasCallbackImpl(unsigned char* buffer, int buffer_len, void* context) {
	CustomData* ctx = (CustomData*)context;

	//get a frame per 5
	if (ctx->counter != 0 && ctx->frame_gap != 0 &&  ctx->last_display_no + ctx->frame_gap != ctx->counter) {
		++ctx->counter;
		return 0;
	}

	int64_t timestamp = get_current_time_in_ms();
	memcpy(&ctx->buffer[0], &timestamp, sizeof timestamp);
	memcpy(&ctx->buffer[sizeof int64_t], &ctx->counter, sizeof int64_t);
	memcpy(&ctx->buffer[sizeof int64_t + sizeof int64_t], buffer, buffer_len);

	//store
	if (ctx->store_file_flag) {
		DWORD nBytesWritten = 0;
		BOOL ret = WriteFile(ctx->hFile, ctx->buffer.data(), ctx->buffer.size(), &nBytesWritten, NULL);

		LARGE_INTEGER FileSize;
		BOOL r = GetFileSizeEx(ctx->hFile, &FileSize);
		if (!r) {
			printf("GetFileSizeEx failed\n");
			return FALSE;
		}

		const static int64_t MAX_RAW_VIDEO_FILE_SIZE = 1024 * 1024 * 1024 * 10;//10GB
		if (FileSize.QuadPart >= MAX_RAW_VIDEO_FILE_SIZE) {
			SetFilePointer(ctx->hFile, 0, NULL, FILE_BEGIN);
		}
	}

	int ret = send_data(ctx, (char*)buffer, buffer_len);
	if (ret < 0) {
		int a = 0;
	}
	printf("snd %d bytes, seq:%lld, displayno:%lld,timestamp:%lld\n", ret, ctx->counter, ctx->counter, timestamp);
	ctx->last_display_no = ctx->counter;
	++ctx->counter;
	return ret;
}

