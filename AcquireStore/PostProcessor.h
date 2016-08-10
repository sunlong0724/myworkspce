#ifndef __POST_PROCESSOR_H__
#define __POST_PROCESSOR_H__
#include "MyThread.h"
#include "utils.h"
#include <opencv2\opencv.hpp>




class CPostProcessor : public CMyThread {
public:
	CPostProcessor():m_sink_cb(NULL), m_sink_cb_ctx(NULL),m_set_parameter_flag(FALSE), m_ring_buffer(NULL){}

	void set_sink_callback(SinkDataCallback _cb, void *ctx) {
		m_sink_cb = _cb;
		m_sink_cb_ctx = ctx;
	}
	void set_parameters(uint16_t image_w, uint16_t image_h, uint16_t image_resized_w, uint16_t image_resized_h) {
		if (m_set_parameter_flag == FALSE) {
			m_image_w = image_w;
			m_image_h = image_h;
			m_image_resized_w = image_resized_w;
			m_image_resized_h = image_resized_h;
			m_set_parameter_flag = TRUE;
		}
	}

	int64_t writeImageData(char* buffer, int len) {
		if (m_ring_buffer == NULL) {
			return 0;
		}
		return RingBuffer_write(m_ring_buffer, buffer, len);
	}

	void resized_image(std::vector<char> &src, std::vector<char> &resized_buf) {

	}
protected:
	virtual void run() {
		timeBeginPeriod(1);
		m_bayer_image_resized =  NULL;
		

		int64_t	s = GET_IMAGE_BUFFER_SIZE(m_image_w, m_image_h);
		m_ring_buffer = RingBuffer_create(s * 1);
		m_buffer.resize(s, 0x00);

		while (!m_exited) {

			if (m_set_parameter_flag) {
				//if (m_bayer_image_resized) {
				//	cvRelease((void**)&m_bayer_image_resized);
				//	m_bayer_image_resized = NULL;
				//}
				//m_bayer_image_resized = cvCreateImageHeader(cvSize(m_image_resized_w, m_image_resized_h), 8, 1);
				m_buffer_resized.resize(GET_IMAGE_BUFFER_SIZE(m_image_resized_w, m_image_resized_h), 0x00);
				//cvSetData(m_bayer_image_resized, &m_buffer_resized[FRAME_DATA_START], m_image_resized_w);

				m_set_parameter_flag = FALSE;
			}

			if ( NULL == m_ring_buffer || -1 == RingBuffer_read(m_ring_buffer, m_buffer.data(), m_buffer.size())) {
				sleep(2);
				continue;
			}

			resized_iamge();

			if (m_sink_cb) {
				m_sink_cb((unsigned char*)m_buffer_resized.data(), m_buffer_resized.size(), m_sink_cb_ctx);
			}
		}
		//if (m_bayer_image_resized)
		//	cvRelease((void**)&m_bayer_image_resized);
		RingBuffer_destroy(m_ring_buffer);
		timeEndPeriod(1);
	}

private:
	//RGRG	RGRG RGRG RGRG RGRG RGRG
	//GBGB	GBGB GBGB GBGB GBGB GBGB
	//RGRG	RGRG RGRG RGRG RGRG RGRG
	//GBGB	GBGB GBGB GBGB GBGB GBGB

	//RGRG	RGRG RGRG RGRG RGRG RGRG
	//GBGB	GBGB GBGB GBGB GBGB GBGB
	//RGRG	RGRG RGRG RGRG RGRG RGRG
	//GBGB	GBGB GBGB GBGB GBGB GBGB

	//RGRG	RGRG RGRG RGRG RGRG RGRG
	//GBGB	GBGB GBGB GBGB GBGB GBGB
	//RGRG	RGRG RGRG RGRG RGRG RGRG
	//GBGB	GBGB GBGB GBGB GBGB GBGB


	void resized_iamge() {
		int k = 0;
		memcpy(m_buffer_resized.data(), m_buffer.data(), FRAME_DATA_START);
		unsigned char*p = (unsigned char*)(&m_buffer_resized[FRAME_DATA_START]);

		unsigned char* src = (unsigned char*)(&m_buffer[FRAME_DATA_START]);
		for (int i = 0; i < m_image_h/4; ++i) {
			if (i % 2 == 0) {
				for (int j = 0; j < m_image_w/4; ++j ) {
					if (j % 2 == 0) {
						//R
						p[k++] = src[m_image_w*(2 + i * 4) + (j * 4 + 2)];
					}
					else {
						//G = (G1+G2)/2
						p[k++] = (src[m_image_w *(1 + i * 4) + (j * 4 + 2)] + src[m_image_w *(2 + i*4) + 4 * j + 1]) / 2;
					}
				}
			}
			else {
				for (int j = 0; j < m_image_w / 4; ++j) {
					if (j % 2 == 0) {
						//G = (G1+G2)/2
						p[k++] = (src[m_image_w *(1 + i * 4) + (j * 4 + 2)] + src[m_image_w *(2 + i * 4) + 4 * j + 1]) / 2;
					}
					else {
						//B
						p[k++] = src[m_image_w*(1 + i * 4) + (j * 4 + 2)];
					}
				}
			}
		}
	}

public:
	void resized_iamge(unsigned char* src_buffer,int src_w, int src_h, unsigned char* resized_buffer) {
		int k = 0;
		memcpy(resized_buffer, src_buffer, FRAME_DATA_START);
		unsigned char*p = (unsigned char*)(&resized_buffer[FRAME_DATA_START]);

		unsigned char* src = (unsigned char*)(&src_buffer[FRAME_DATA_START]);
		for (int i = 0; i < src_h / 4; ++i) {
			if (i % 2 == 0) {
				for (int j = 0; j < src_w / 4; ++j) {
					if (j % 2 == 0) {
						//R
						p[k++] = src[src_w*(2 + i * 4) + (j * 4 + 2)];
					}
					else {
						//G = (G1+G2)/2
						p[k++] = (src[src_w *(1 + i * 4) + (j * 4 + 2)] + src[src_w *(2 + i * 4) + 4 * j + 1]) / 2;
					}
				}
			}
			else {
				for (int j = 0; j < src_w / 4; ++j) {
					if (j % 2 == 0) {
						//G = (G1+G2)/2
						p[k++] = (src[src_w *(1 + i * 4) + (j * 4 + 2)] + src[src_w *(2 + i * 4) + 4 * j + 1]) / 2;
					}
					else {
						//B
						p[k++] = src[src_w*(1 + i * 4) + (j * 4 + 2)];
					}
				}
			}
		}
	}

public:
	std::vector<char>   m_buffer_resized;
	uint16_t			m_image_resized_w;
	uint16_t			m_image_resized_h;
private:
	uint16_t			m_image_w;
	uint16_t			m_image_h;

	//uint16_t			m_image_resized_w;
	//uint16_t			m_image_resized_h;

	BOOL				m_set_parameter_flag;

	SinkDataCallback    m_sink_cb;
	void*				m_sink_cb_ctx;

	std::vector<char>	m_buffer;
	//std::vector<char>   m_buffer_resized;

	RingBuffer			*m_ring_buffer;

	IplImage*			m_bayer_image_resized;
};

#endif // __POST_PROCESSOR_H__
