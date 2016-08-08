#ifndef _SAPMYPROCESSING_H_
#define _SAPMYPROCESSING_H_

//	SapMyProcessing.h : header file
//

#include "SapClassBasic.h"
//
// SapMyProcessing class declaration
//
class SapMyProcessing : public SapProcessing
{
public:
	SapMyProcessing(SapBuffer *pBuffers, SapColorConversion* pColorConv, SapProCallback pCallback, void *pContext);
protected:
	virtual BOOL Run();
private:
	SapColorConversion* m_ColorConv;
}; 

#endif	// _SAPMYPROCESSING_H_

