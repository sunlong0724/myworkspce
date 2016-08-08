//	SapMyProcessing.cpp : implementation file
//

#include "SapMyProcessing.h"


//
// Constructor/Destructor
//
SapMyProcessing::SapMyProcessing(SapBuffer *pBuffers, SapColorConversion* pColorConv, SapProCallback pCallback, void *pContext)
: m_ColorConv(pColorConv), SapProcessing(pBuffers, pCallback, pContext)
{
}

//
// Processing Control
//
BOOL SapMyProcessing::Run(){
	if (m_ColorConv->IsEnabled() && m_ColorConv->IsSoftwareEnabled()){
		m_ColorConv->Convert(GetIndex());
		//SapFormat sf = m_ColorConv->GetOutputFormat();//RGB888
		//fprintf(stdout, "%s %d\r\n", __FUNCTION__, sf);
		//fprintf(stdout, "%s index:%d\r\n", __FUNCTION__, index);
	}

	return TRUE;
}


