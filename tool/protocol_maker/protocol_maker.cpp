// protocol_maker.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "protocolmaker.h"

CProtocolMaker gProtocolMaker;

int _tmain(int argc, _TCHAR* argv[])
{
    char* pszXml = NULL;
    char* pszPath = NULL;
    if (3 == argc)
    {
        pszXml = argv[1];
        pszPath = argv[2];
    }

    if (2 == argc)
    {
        pszXml = argv[0];
        pszPath = argv[1];
    }
    if (!gProtocolMaker.MakeProtocol( pszXml, pszPath))
    {
        printf("make %s fail! errinfo:\n", pszXml);
        printf(gProtocolMaker.GetErrorInfo().c_str());
		printf("\n");
        return -1;
    }
	else
	{
		printf("make %s success!\n", pszXml);
	}

	return 0;
}

