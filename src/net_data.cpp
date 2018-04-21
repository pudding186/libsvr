//#ifdef WIN32
//#include <WinSock2.h>
//#else
//#include <string.h>
//#endif

#include "../include/net_data.hpp"

CNetData::CNetData()
{
    m_pBuf	= NULL;
    m_iSize = 0;
    m_iPos	= 0;
}

CNetData::~CNetData()
{
}

void CNetData::Prepare(char *pNetData, int iSize)
{
    m_pBuf	= pNetData;
    m_iSize	= iSize;
    m_iPos	= 0;
}

void CNetData::Reset()
{
    m_iPos = 0;
}

char * CNetData::GetData()
{
    return m_pBuf;
}

int CNetData::GetDataLen()
{
    return m_iPos;
}

int CNetData::AddByte(unsigned char byByte)
{
    if(m_iPos + (int)sizeof(byByte) > m_iSize)
        return -1;

    *(unsigned char*)(m_pBuf + m_iPos) = byByte;
    m_iPos += sizeof(byByte);

    return m_iPos; 
}

int CNetData::DelByte(unsigned char &byByte)
{
    if(m_iPos + (int)sizeof(byByte) > m_iSize)
        return -1;

    byByte = *(unsigned char*)(m_pBuf + m_iPos);
    m_iPos += sizeof(byByte);

    return m_iPos;
}

int CNetData::AddChar(char chChar)
{
    if(m_iPos + (int)sizeof(chChar) > m_iSize)
        return -1;

    *(char*)(m_pBuf + m_iPos) = chChar;
    m_iPos += sizeof(chChar);

    return m_iPos; 
}

int CNetData::DelChar(char &chChar)
{
    if(m_iPos + (int)sizeof(chChar) > m_iSize)
        return -1;

    chChar = *(char*)(m_pBuf + m_iPos);
    m_iPos += sizeof(chChar);

    return m_iPos; 
}

int CNetData::AddWord(unsigned short wWord)
{
    if(m_iPos + (int)sizeof(wWord) > m_iSize)
        return -1;

    //*(unsigned short*)(m_pBuf + m_iPos) = htons(wWord);
    *(unsigned short*)(m_pBuf + m_iPos) = wWord;
    m_iPos += sizeof(wWord);

    return m_iPos; 
}

int CNetData::DelWord(unsigned short &wWord)
{
    if(m_iPos + (int)sizeof(wWord) > m_iSize)
        return -1;

    //wWord = ntohs(*(unsigned short*)(m_pBuf + m_iPos));
    wWord = *(unsigned short*)(m_pBuf + m_iPos);
    m_iPos += sizeof(wWord);

    return m_iPos; 
}

int CNetData::AddShort(short shShort)
{
    if(m_iPos + (int)sizeof(shShort) > m_iSize)
        return -1;

    //*(short*)(m_pBuf + m_iPos) = htons(shShort);
    *(short*)(m_pBuf + m_iPos) = shShort;
    m_iPos += sizeof(shShort);

    return m_iPos;
}

int CNetData::DelShort(short &shShort)
{
    if(m_iPos + (int)sizeof(shShort) > m_iSize)
        return -1;

    //shShort = ntohs(*(short*)(m_pBuf + m_iPos));
    shShort = *(short*)(m_pBuf + m_iPos);
    m_iPos += sizeof(shShort);

    return m_iPos;
}

int CNetData::AddDword(unsigned int dwDword)
{
    if(m_iPos + (int)sizeof(dwDword) > m_iSize)
        return -1;

    //*(unsigned int*)(m_pBuf + m_iPos) = htonl(dwDword);
    *(unsigned int*)(m_pBuf + m_iPos) = dwDword;
    m_iPos += sizeof(dwDword);

    return m_iPos;
}

int CNetData::DelDword(unsigned int &dwDword)
{
    if(m_iPos + (int)sizeof(dwDword) > m_iSize)
        return -1;

    //dwDword = ntohl(*(unsigned int*)(m_pBuf + m_iPos));
    dwDword = *(unsigned int*)(m_pBuf + m_iPos);
    m_iPos += sizeof(dwDword);

    return m_iPos;
}

int CNetData::AddInt(int iInt)
{
    if(m_iPos + (int)sizeof(iInt) > m_iSize)
        return -1;

    //*(int*)(m_pBuf + m_iPos) = htonl(iInt);
    *(int*)(m_pBuf + m_iPos) = iInt;
    m_iPos += sizeof(iInt);

    return m_iPos;
}

int CNetData::DelInt(int &iInt)
{
    if(m_iPos + (int)sizeof(iInt) > m_iSize)
        return -1;

    //iInt = ntohl(*(int*)(m_pBuf + m_iPos));
    iInt = *(int*)(m_pBuf + m_iPos);
    m_iPos += sizeof(iInt);

    return m_iPos;
}

int CNetData::AddInt64( signed long long iInt64 )
{
    if (m_iPos + (int)sizeof(iInt64) > m_iSize)
        return -1;

    *(signed long long*)(m_pBuf + m_iPos) = iInt64;
    m_iPos += sizeof(iInt64);

    return m_iPos;
}

int CNetData::DelInt64( signed long long &iInt64 )
{
    if (m_iPos + (int)sizeof(iInt64) > m_iSize)
        return -1;

    iInt64 = *(signed long long*)(m_pBuf + m_iPos);
    m_iPos += sizeof(iInt64);

    return m_iPos;
}

int CNetData::AddQword( unsigned long long qwQword )
{
    if (m_iPos + (int)sizeof(qwQword) > m_iSize)
        return -1;
    *(unsigned long long*)(m_pBuf + m_iPos) = qwQword;
    m_iPos += sizeof(qwQword);

    return m_iPos;
}

int CNetData::DelQword( unsigned long long &qwQword )
{
    if (m_iPos + (int)sizeof(qwQword) > m_iSize)
        return -1;

    qwQword = *(unsigned long long*)(m_pBuf + m_iPos);
    m_iPos += sizeof(qwQword);

    return m_iPos;
}

int CNetData::AddString(const char *pszString, int iSize)
{
    int iLen = (int)strnlen(pszString, iSize-1);

    if(m_iPos + (int)sizeof(unsigned short) + iLen > m_iSize)
        return -1;

    if(-1 == AddWord((unsigned short)iLen))
        return -1;

    memcpy(m_pBuf + m_iPos, pszString, iLen);
    m_iPos += iLen;

    return m_iPos;
}

int CNetData::DelString(char *pszOut, int iSize)
{
    unsigned short wLen = 0;
    if(-1 == DelWord(wLen))
        return -1;

    if(m_iPos + (int)wLen > m_iSize)
        return -1;

    if( int(wLen + 1) > iSize )
        return -1;

    memcpy(pszOut, m_pBuf+m_iPos, wLen);
    pszOut[wLen] = '\0';
    m_iPos += wLen;

    return m_iPos;
}

int CNetData::AddBlob( const char *pszData, int iSize )
{
    if (m_iPos + iSize > m_iSize)
        return -1;

    memcpy(m_pBuf + m_iPos, pszData, iSize);

    m_iPos += iSize;

    return m_iPos;
}

int CNetData::DelBlob( char *pszOut, int iSize )
{
    if (m_iPos + iSize > m_iSize)
        return -1;

    memcpy(pszOut, m_pBuf+m_iPos, iSize);
    m_iPos += iSize;

    return m_iPos;
}
//int CNetData::AddBlob( const blob & Blob )
//{
//    int iLen = (int)Blob.size();
//
//    if (m_iPos + (int)sizeof(int) + iLen > m_iSize)
//        return -1;
//
//    if (-1 == AddInt(iLen))
//        return -1;
//
//    memcpy(m_pBuf + m_iPos, Blob.c_str(), iLen);
//    m_iPos += iLen;
//
//    return m_iPos;
//}
//
//int CNetData::DelBlob( blob & Blob )
//{
//    int iLen = 0;
//    if (-1 == DelInt(iLen))
//        return -1;
//
//    if (m_iPos + iLen > m_iSize)
//        return -1;
//
//    Blob.clear();
//    Blob.append(m_pBuf+m_iPos, iLen);
//    m_iPos += iLen;
//
//    return m_iPos;
//}

//int CNetData::Strnlen(const char *pszString, int iSize)
//{
//    int i;
//    const char *ptr = pszString;
//
//    for(i = 0; i < iSize; i++)
//    {
//        if('\0' == *ptr)
//        {
//            return i;
//        }
//        ptr++;
//    }
//
//    return iSize;
//}





