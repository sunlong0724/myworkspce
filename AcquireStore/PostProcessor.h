#ifndef __POST_PROCESSOR_H__
#define __POST_PROCESSOR_H__
#include "MyThread.h"
#include "utils.h"
#include <opencv2\opencv.hpp>

#define MAX_IMAGE_COUNT 10



class CPostProcessor : public CMyThread {
public:
	void set_sink_callback(SinkDataCallback _cb, void *ctx) {
		m_sink_cb = _cb;
		m_sink_cb_ctx = ctx;
	}
	void set_parameters(uint16_t image_w, uint16_t image_h, uint16_t image_resized_w, uint16_t image_resized_h) {
		m_image_w = image_w;
		m_image_h = image_h;
		m_image_resized_w = image_resized_w;
		m_image_resized_h = image_resized_h;
	}

	int64_t writeImageData(char* buffer, int len) {
		return RingBuffer_write(m_ring_buffer, buffer, len);
	}
protected:
	virtual void run() {
		m_bayer_image = m_rgb_image = m_rgb_image_resized = NULL;
		m_image_h = m_image_w = m_image_resized_h = m_image_resized_w = 0;

		int64_t	s = sizeof int64_t * sizeof int64_t + m_image_w * m_image_w;
		m_ring_buffer = RingBuffer_create(s * MAX_IMAGE_COUNT);
		m_buffer.resize(s, 0x00);

		m_bayer_image = cvCreateImageHeader(cvSize(m_image_w, m_image_h), 8, 1);
		cvSetData(m_bayer_image, &m_buffer[sizeof int64_t + sizeof int64_t], m_image_w);

		m_rgb_image = cvCreateImage(cvSize(m_image_w, m_image_h), 8, 3);

		while (!m_exited) {
			if (-1 == RingBuffer_read(m_ring_buffer, m_buffer.data(), m_buffer.size())) {
				sleep(2);
				continue;
			}

			cvCvtColor(m_bayer_image, m_rgb_image, CV_BayerRG2RGB);
			cvResize(m_rgb_image, m_rgb_image_resized);
			//FIXME: rgb 2 bayer 


			//cvShowImage("win", pProcessor->m_rgb_image_resized);
			//cvWaitKey(1);
			//m_sink_cb()
			if (m_sink_cb) {
				std::vector<char> buffer(320 * 240, 0x00);
				m_sink_cb((unsigned char*)buffer.data(), buffer.size(), m_sink_cb_ctx);
			}
		}
		cvRelease((void**)&m_rgb_image);
		cvRelease((void**)&m_bayer_image);
		cvRelease((void**)&m_rgb_image_resized);

		RingBuffer_destroy(m_ring_buffer);
	}

private:
	uint16_t			m_image_w;
	uint16_t			m_image_h;

	uint16_t			m_image_resized_w;
	uint16_t			m_image_resized_h;

	SinkDataCallback    m_sink_cb;
	void*				m_sink_cb_ctx;

	std::vector<char>	m_buffer;
	RingBuffer			*m_ring_buffer;

	IplImage*			m_bayer_image;
	IplImage*			m_rgb_image;
	IplImage*			m_rgb_image_resized;
};

#endif // __POST_PROCESSOR_H__
