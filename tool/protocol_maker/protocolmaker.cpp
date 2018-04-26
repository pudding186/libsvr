#include "StdAfx.h"
#include "protocolmaker.h"
//#include <string>
//
#define FIND_ELEM(elem)             oXml.ResetMainPos();\
                                    if (!oXml.FindElem(elem))\
                                    {\
                                        m_strErrorInfo = "Can't Find Element <elem>";\
                                        goto ERROR_DEAL;\
                                    }

#define GET_ATTRIB(attrib)          strVal = oXml.GetAttrib(attrib);\
                                    if (strVal.empty())\
                                    {\
                                        m_strErrorInfo = "Can't Find Attrib attrib";\
                                        goto ERROR_DEAL;\
                                    }

#define INTO_ELEM()                 if (!oXml.IntoElem())\
                                        goto ERROR_DEAL;

#define OUTOF_ELEM()                if (!oXml.OutOfElem())\
                                        goto ERROR_DEAL;

#define ERRORINFO_DATA(strType,strName,strComment)\
                                    m_strErrorInfo = "<data ";\
                                    if (!strType.empty())\
                                    {\
                                        m_strErrorInfo +="type=\""+strType+"\" ";\
                                    }\
                                    if (!strName.empty())\
                                    {\
                                        m_strErrorInfo +="name=\""+strName+"\" ";\
                                    }\
                                    if (!strComment.empty())\
                                    {\
                                        m_strErrorInfo +="comment=\""+strComment+"\" ";\
                                    }\
                                    m_strErrorInfo += ">\r\n";

#define ERRORINFO_ITEM(strName, strType, strCount, strRefer, strId, strSelect, strLength, strComment)\
                                    m_strErrorInfo +="\t<item ";\
                                    if (!strName.empty())\
                                    {\
                                        m_strErrorInfo += "name=\""+strName+"\" ";\
                                    }\
                                    if (!strType.empty())\
                                    {\
                                        m_strErrorInfo += "type=\""+strType+"\" ";\
                                    }\
                                    if (!strCount.empty())\
                                    {\
                                        m_strErrorInfo += "count=\""+strCount+"\" ";\
                                    }\
                                    if (!strRefer.empty())\
                                    {\
                                        m_strErrorInfo += "refer=\""+strRefer+"\" ";\
                                    }\
                                    if (!strId.empty())\
                                    {\
                                        m_strErrorInfo += "id=\""+strId+"\" ";\
                                    }\
                                    if (!strSelect.empty())\
                                    {\
                                        m_strErrorInfo += "select=\""+strSelect+"\" ";\
                                    }\
                                    if (!strLength.empty())\
                                    {\
                                    m_strErrorInfo += "length=\""+strLength+"\" ";\
                                    }\
                                    if (!strComment.empty())\
                                    {\
                                        m_strErrorInfo += "comment=\""+strComment+"\" ";\
                                    }\
                                    m_strErrorInfo +="/>\r\n";

CProtocolMaker::CProtocolMaker(void)
{
    
}

CProtocolMaker::~CProtocolMaker(void)
{
}

bool CProtocolMaker::MakeProtocol( const std::string& strXML, const std::string& strOutPutPath )
{
    CMarkupSTL oXml;
    std::string strVal;
    std::string strHppFile;
    std::string strCppFile;
    std::string strMouleName;
    FILE* pHppFile = NULL;
    FILE* pCppFile = NULL;


    if (!oXml.Load(strXML.c_str()))
    {
        m_strErrorInfo = "����Э���ļ�"+strXML+"ʧ��!";
        goto ERROR_DEAL;
    }

    //��ȡЭ��ģ������Ϊ�ļ���
    FIND_ELEM("protocol_define");
    GET_ATTRIB("name");

    strHppFile = strOutPutPath + strVal + ".hpp";
    strCppFile = strOutPutPath + strVal + ".cpp";
    pHppFile = fopen(strHppFile.c_str(), "wb");
    pCppFile = fopen(strCppFile.c_str(), "wb");
    if (NULL == pHppFile || NULL == pCppFile)
        goto ERROR_DEAL;

    fprintf(pHppFile, "#pragma once\r\n\r\n");
    fprintf(pHppFile, "#include \"netdata.h\"\r\n\r\n");
    fprintf(pCppFile, "#include \"%s.hpp\"\r\n\r\n", strVal.c_str());

    strMouleName = strVal;

    GET_ATTRIB("moudleid");
    m_strMoudleID = strVal;
    if (!__IsAllDigit(m_strMoudleID))
    {
        m_strErrorInfo="moulleid is not digital!";
        goto ERROR_DEAL;
    }

    INTO_ELEM();

    if (!__WritePackge(pHppFile, oXml, strOutPutPath))
        goto ERROR_DEAL;

    if (!__WriteMacro(pHppFile, oXml))
        goto ERROR_DEAL;

    if (!__WriteData(pHppFile, oXml))
        goto ERROR_DEAL;

    if (!__WriteDataFunction(pHppFile, pCppFile, oXml))
        goto ERROR_DEAL;
    OUTOF_ELEM();

    if (!__WriteProtocolClass(strMouleName, pHppFile, pCppFile))
    {
        goto ERROR_DEAL;
    }

    return true;

ERROR_DEAL:
    return false;
}

bool CProtocolMaker::__IsKeyType( const std::string& strType )
{
    //if (strType == "char"
    //    || strType == "byte"
    //    || strType == "short"
    //    || strType == "word"
    //    || strType == "int"
    //    || strType == "int64"
    //    || strType == "dword"
    //    || strType == "qword"
    //    || strType == "string")
    if (strType == "int8"
        || strType == "uint8"
        || strType == "int16"
        || strType == "uint16"
        || strType == "int32"
        || strType == "int64"
        || strType == "uint32"
        || strType == "uint64"
        || strType == "string")
    {
        return true;
    }

    return false;
}

bool CProtocolMaker::__IsStruct( const std::string& strType )
{
    CData::iterator it = m_mapStruct.find(strType);
    if (it == m_mapStruct.end())
    {
        return false;
    }

    return true;
}

bool CProtocolMaker::__IsUnion( const std::string& strType )
{
    CData::iterator it = m_mapUnion.find(strType);
    if (it == m_mapUnion.end())
    {
        return false;
    }

    return true;
}

bool CProtocolMaker::__IsMacro( const std::string& strType )
{
    CDataSet::iterator it = m_setMacro.find(strType);
    if (it == m_setMacro.end())
    {
        return false;
    }

    return true;
}

bool CProtocolMaker::__IsAllDigit( const std::string& strTest )
{
    if (strTest.empty())
    {
        return false;
    }

    const char* pTr = strTest.c_str();

    for (size_t i = 0; i < strTest.length(); i++)
    {
        if (!isdigit(pTr[i]))
        {
            return false;
        }
    }

    return true;
}

bool CProtocolMaker::__CheckPackge( const std::string& strPkg )
{
    CMarkupSTL oXml;
    std::string strVal;
    //std::string strHppFile;
    //std::string strCppFile;
    //std::string strMouleName;
    //FILE* pHppFile = NULL;
    //FILE* pCppFile = NULL;


    if (!oXml.Load(strPkg.c_str()))
    {
        m_strErrorInfo = "����Э���ļ�"+strPkg+"ʧ��!";
        goto ERROR_DEAL;
    }

    CMarkupSTL& rXml = oXml;

    FIND_ELEM("protocol_define");
    INTO_ELEM();

    //���Һ궨��
    rXml.ResetMainPos();
    while (rXml.FindElem("macro"))
    {
        std::string strName = rXml.GetAttrib("name");
        std::string strValue = rXml.GetAttrib("value");
        std::string strComment = rXml.GetAttrib("comment");
        if (strName.empty() ||strValue.empty())
        {
            m_strErrorInfo = "�궨����� name �� value ����Ϊ��";
            return false;
        }
        m_setMacro.insert(strName);
    }
    rXml.ResetMainPos();
    while (rXml.FindElem("data"))
    {
        std::string strName = rXml.GetAttrib("name");
        std::string strType = rXml.GetAttrib("type");
        std::string strComment = rXml.GetAttrib("comment");
        EDataType eDataType = eStruct;

        ERRORINFO_DATA(strType, strName, strComment);

        if (strType == "struct")
        {
            if (m_mapStruct.find(strName) != m_mapStruct.end())
            {
                m_strErrorInfo += "�ض���";
                return false;
            }
            eDataType = eStruct;
        }
        else if (strType == "protocol")
        {
            continue;
        }
        else
        {
            m_strErrorInfo += "type���Ե�����δ֪��ֻ�ܶ���Ϊstruct";
            return false;
        }

        if (!rXml.IntoElem())
        {
            m_strErrorInfo += "�����Ӽ�ʧ��";
            return false;
        }

        CItem mapItem;

        while (rXml.FindElem("item"))
        {
            CAttrib vecAttrib(8);
            vecAttrib[eName] = rXml.GetAttrib("name");
            vecAttrib[eType] = rXml.GetAttrib("type");
            vecAttrib[eCount] = rXml.GetAttrib("count");
            vecAttrib[eRefer] = rXml.GetAttrib("refer");
            vecAttrib[eId] = rXml.GetAttrib("id");
            vecAttrib[eSelect] = rXml.GetAttrib("select");
            vecAttrib[eComment] = rXml.GetAttrib("comment");
            vecAttrib[eLength] = rXml.GetAttrib("length");

            CItem::iterator it  = mapItem.find(vecAttrib[eName]);
            if (it != mapItem.end())
            {
                m_strErrorInfo += "name����ֵ�ظ�";
                return false;
            }
            else
                mapItem[vecAttrib[eName]] = vecAttrib;
        }

        if (!rXml.OutOfElem())
        {
            m_strErrorInfo += "���Ӽ�����ʧ��";
            return false;
        }

        if (strType == "struct")
        {
            m_mapStruct[strName] = mapItem;
        }
        else
        {
            return false;
        }
    }
    OUTOF_ELEM();

    

    return true;
ERROR_DEAL:
    return false;
}

bool CProtocolMaker::__WritePackge( FILE* pHppFile, CMarkupSTL& rXml, const std::string& strPackgePath )
{
    //�����������ݶ���
    fprintf(pHppFile, "//===============����������Э���ļ�===============\r\n");
    rXml.ResetMainPos();
    while (rXml.FindElem("package"))
    {
        std::string strPackage = rXml.GetAttrib("name");

        if (!__CheckPackge(strPackgePath+strPackage+".xml"))
        {
            return false;
        }

        fprintf(pHppFile, "#include \"%s.hpp\"\r\n ", strPackage.c_str());
    }

    return true;
}

bool CProtocolMaker::__WriteMacro( FILE* pHppFile, CMarkupSTL& rXml )
{
    //�������еĺ궨��
    fprintf(pHppFile, "//===============�궨�忪ʼ===============\r\n");
    rXml.ResetMainPos();
    while (rXml.FindElem("macro"))
    {
        std::string strName = rXml.GetAttrib("name");
        std::string strValue = rXml.GetAttrib("value");
        std::string strComment = rXml.GetAttrib("comment");
        if (strName.empty() ||strValue.empty())
        {
            m_strErrorInfo = "�궨����� name �� value ����Ϊ��";
            return false;
        }

        if (strComment.empty())
            fprintf(pHppFile, "#define %-*s %s\r\n", 30, strName.c_str(), strValue.c_str());
        else
            fprintf(pHppFile, "#define %-*s %s //%s\r\n", 30, strName.c_str(), strValue.c_str(), strComment.c_str());
        m_setMacro.insert(strName);
    }
    fprintf(pHppFile, "//===============�궨�����===============\r\n\r\n");
    return true;
}

bool CProtocolMaker::__WriteData( FILE* pHppFile, CMarkupSTL& rXml )
{
    //�����������ݶ���
    fprintf(pHppFile, "//===============���ݶ��忪ʼ===============\r\n");
    fprintf(pHppFile, "#pragma warning( push )\r\n#pragma warning( disable : 4512)\r\n");
    rXml.ResetMainPos();
    while (rXml.FindElem("data"))
    {
        std::string strName = rXml.GetAttrib("name");
        std::string strType = rXml.GetAttrib("type");
        std::string strComment = rXml.GetAttrib("comment");
        EDataType eDataType = eStruct;

        ERRORINFO_DATA(strType, strName, strComment);

        if (strType == "struct")
        {
            if (m_mapStruct.find(strName) != m_mapStruct.end())
            {
                m_strErrorInfo += "�ض���";
                return false;
            }

            //fprintf(pHppFile, "typedef struct struct_%s{\r\n", strName.c_str());
            fprintf(pHppFile, "struct %s{\r\n", strName.c_str());
            eDataType = eStruct;
        }
        else if (strType == "union")
        {
            fprintf(pHppFile, "union %s{\r\n", strName.c_str());
            eDataType = eUnion;
        }
        else if (strType == "protocol")
        {
            if (m_mapProtocol.find(strName) != m_mapProtocol.end())
            {
                m_strErrorInfo += " �ض���";
                return false;
            }
            fprintf(pHppFile, "struct %s{\r\n", strName.c_str());
            fprintf(pHppFile,"\tconst %-*s moudleid;\r\n", 19, "unsigned short");
            fprintf(pHppFile,"\tconst %-*s protocolid;\r\n", 19, "unsigned short");
            eDataType = eProtocol;
        }
        else
        {
            m_strErrorInfo += "type���Ե�����δ֪��ֻ�ܶ���Ϊstruct,union,protocol";
            return false;
        }

        if (!rXml.IntoElem())
        {
            m_strErrorInfo += "�����Ӽ�ʧ��";
            return false;
        }

        CItem mapItem;

        while (rXml.FindElem("item"))
        {
            CAttrib vecAttrib(8);
            vecAttrib[eName] = rXml.GetAttrib("name");
            vecAttrib[eType] = rXml.GetAttrib("type");
            vecAttrib[eCount] = rXml.GetAttrib("count");
            vecAttrib[eRefer] = rXml.GetAttrib("refer");
            vecAttrib[eId] = rXml.GetAttrib("id");
            vecAttrib[eSelect] = rXml.GetAttrib("select");
            vecAttrib[eComment] = rXml.GetAttrib("comment");
            vecAttrib[eLength] = rXml.GetAttrib("length");

            CItem::iterator it  = mapItem.find(vecAttrib[eName]);
            if (it != mapItem.end())
            {
                m_strErrorInfo += "name����ֵ�ظ�";
                return false;
            }
            else
                mapItem[vecAttrib[eName]] = vecAttrib;
        }

        rXml.ResetMainPos();
        while (rXml.FindElem("item"))
        {
            if (!__WriteItem(pHppFile, rXml, eDataType, mapItem))
            {
                return false;
            }
        }

        if (!rXml.OutOfElem())
        {
            m_strErrorInfo += "���Ӽ�����ʧ��";
            return false;
        }

        if (strType == "struct")
        {
            m_mapStruct[strName] = mapItem;
        }
        else if (strType == "union")
        {
            m_mapUnion[strName] = mapItem;
        }
        else if (strType == "protocol")
        {
            fprintf(pHppFile, "\t%s():moudleid(%s),protocolid(%d){}\r\n", strName.c_str(), m_strMoudleID.c_str(), m_vecProtocol.size());
            //fprintf(pHppFile, "\tprotocol_%s():protocolid(%d){}\r\n", strName.c_str(), m_vecProtocol.size());
            m_mapStruct[strName] = mapItem;
            m_mapProtocol[strName] = mapItem;
            m_vecProtocol.push_back(strName);
        }
        else
        {
            return false;
        }

        //fprintf(pHppFile, "}%s;\r\n\r\n", strName.c_str());
        fprintf(pHppFile, "};\r\n\r\n");
    }
    fprintf(pHppFile, "#pragma warning( pop ) \r\n");
    fprintf(pHppFile, "//===============���ݶ������===============\r\n");
    return true;
}

bool CProtocolMaker::__WriteDataFunction( FILE* pHppFile, FILE* pCppFile, CMarkupSTL& rXml )
{
    fprintf(pHppFile, "//===============�������������忪ʼ===============\r\n");
    rXml.ResetMainPos();
    while (rXml.FindElem("data"))
    {
        std::string strType = rXml.GetAttrib("type");
        if (strType == "struct")
        {
            if (!__WriteStructProtocolEnCodeFunc(rXml, pHppFile, pCppFile, false))
            {
                return false;
            }
            rXml.ResetChildPos();
            if (!__WriteStructProtocolDeCodeFunc(rXml, pHppFile, pCppFile, false))
            {
                return false;
            }
        }
        else if (strType == "protocol")
        {
            if (!__WriteStructProtocolEnCodeFunc(rXml, pHppFile, pCppFile, true))
            {
                return false;
            }
            rXml.ResetChildPos();
            if (!__WriteStructProtocolDeCodeFunc(rXml, pHppFile, pCppFile, true))
            {
                return false;
            }
        }
        else if (strType == "union")
        {
            if (!__WriteUnionEnCodeFunc(rXml, pHppFile, pCppFile))
            {
                return false;
            }
            rXml.ResetChildPos();
            if (!__WriteUnionDeCodeFunc(rXml, pHppFile, pCppFile))
            {
                return false;
            }
        }
        else
        {
            return false;
        }

    }
    fprintf(pHppFile, "//===============�����������������===============\r\n");
    return true;
}

bool CProtocolMaker::__WriteItem( FILE* pHppFile, CMarkupSTL& rXml, EDataType eDataType, CItem& mapItem)
{
    std::string strName = rXml.GetAttrib("name");
    std::string strType = rXml.GetAttrib("type");
    std::string strCount = rXml.GetAttrib("count");
    std::string strRefer = rXml.GetAttrib("refer");
    std::string strId = rXml.GetAttrib("id");
    std::string strSelect = rXml.GetAttrib("select");
    std::string strComment = rXml.GetAttrib("comment");
    std::string strLength = rXml.GetAttrib("length");

    ERRORINFO_ITEM(strName, strType, strCount, strRefer, strId, strSelect, strLength, strComment);

    if (strName.empty() || strType.empty())
    {
        m_strErrorInfo += "�� name���Ժ�type���Բ���Ϊ��";
        return false;
    }

    if (eUnion == eDataType)
    {
        if (!__IsStruct(strType))
        {
            m_strErrorInfo += "union�еĳ�Ա������struct";
            return false;
        }

        if (strId.empty())
        {
            m_strErrorInfo += "union��id���Բ���Ϊ��";
            return false;
        }

        if (!__IsAllDigit(strId))
        {
            if (!__IsMacro(strId))
            {
                m_strErrorInfo += "union��id����Ϊ���ֻ����Ƕ���ĺ�";
                return false;
            }
        }

        if (!strCount.empty())
        {
            m_strErrorInfo += "union�в���������";
            return false;
        }

        if (!strRefer.empty())
        {
            m_strErrorInfo += "union�в�����refer����";
            return false;
        }

        if (!strSelect.empty())
        {
            m_strErrorInfo += "union�в�����select����";
            return false;
        }

        if (!strLength.empty())
        {
            m_strErrorInfo += "union�в�����length����";
            return false;
        }
    }
    

    if (eStruct == eDataType || eProtocol == eDataType)
    {
        if (__IsKeyType(strType) || __IsStruct(strType))
        {
            if (!strCount.empty())
            {
                if (!__IsAllDigit(strCount))
                {
                    if (!__IsMacro(strCount))
                    {
                        m_strErrorInfo += "item��count����Ϊ���ֻ����Ƕ���ĺ�";
                        return false;
                    }
                }
            }

            if (!strRefer.empty())
            {
                if (strCount.empty())
                {
                    m_strErrorInfo += "item����refer����ǰ��������count����";
                    return false;
                }

                CItem::iterator it = mapItem.find(strRefer);
                if (it == mapItem.end())
                {
                    m_strErrorInfo += "item��refer������ֵδ����";
                    return false;
                }
                else
                {
                    if (!__IsKeyType((it->second)[eType]))
                    {
                        m_strErrorInfo += "item��refer������ֵ�����ǻ����������͵ı���";
                        return false;
                    }

                    if (!(it->second)[eCount].empty())
                    {
                        m_strErrorInfo += "item��refer������ֵ����������";
                        return false;
                    }
                }
            }
            if (!strId.empty())
            {
                m_strErrorInfo += "ֻ��data��union�����д�����";
                return false;
            }
            if (!strSelect.empty())
            {
                m_strErrorInfo += "ֻ��type=union�����д�����";
                return false;
            }

            if (!strLength.empty())
            {
                if (strType == "string")
                {
                    if (!__IsAllDigit(strLength))
                    {
                        if (!__IsMacro(strLength))
                        {
                            m_strErrorInfo += "item��length����Ϊ���ֻ����Ƕ���ĺ�";
                            return false;
                        }
                    }
                    else
                    {
                        if (atoi(strLength.c_str()) > 65535)
                        {
                            m_strErrorInfo += "string���͵�length������󳤶Ȳ��ܳ���65535";
                            return false;
                        }
                    }
                }
                else
                {
                    m_strErrorInfo += "ֻ��type=string�����д�����";
                    return false;
                }
            }
        }
        else if (__IsUnion(strType))
        {
            if (strSelect.empty())
            {
                m_strErrorInfo += "union�б�����select����";
                return false;
            }
            else
            {
                CItem::iterator it = mapItem.find(strSelect);
                if (it == mapItem.end())
                {
                    m_strErrorInfo += "item��select������ֵδ����";
                    return false;
                }
                else
                {
                    if (!__IsKeyType((it->second)[eType]))
                    {
                        m_strErrorInfo += "item��select������ֵ�����ǻ����������͵ı���";
                        return false;
                    }

                    if (!(it->second)[eCount].empty())
                    {
                        m_strErrorInfo += "item��select������ֵ����Ϊ����";
                        return false;
                    }
                }
            }
            if (!strCount.empty())
            {
                m_strErrorInfo += "type=union������count����";
                return false;
            }
            if (!strRefer.empty())
            {
                m_strErrorInfo += "type=union������refer����";
                return false;
            }
            if (!strId.empty())
            {
                m_strErrorInfo += "type=union������id����";
                return false;
            }
            if (!strLength.empty())
            {
                m_strErrorInfo += "type=union������length����";
                return false;
            }
        }
        else
        {
            m_strErrorInfo += "Unknow Type "+strType;
            return false;
        }
    }

    std::string strCStyle;
    if (strType == "uint8")
    {
        strCStyle = "unsigned char";
    }
    else if (strType == "int8")
    {
        strCStyle = "char";
    }
    else if (strType == "uint16")
    {
        strCStyle = "unsigned short";
    }
    else if (strType == "int16")
    {
        strCStyle = "short";
    }
    else if (strType == "uint32")
    {
        strCStyle = "unsigned int";
    }
    else if (strType == "int32")
    {
        strCStyle = "int";
    }
    else if (strType == "uint64")
    {
        strCStyle = "unsigned long long";
    }
    else if (strType == "int64")
    {
        strCStyle = "signed long long";
    }
    else if (strType == "string")
    {
        strCStyle = "char";
    }
    else
    {
        strCStyle = strType;
    }

    if (strCount.empty())
    {
        if (strType == "string")
        {
            if (!strLength.empty())
                fprintf(pHppFile, "\t%-*s %s[%s];", 25, strCStyle.c_str(), strName.c_str(), strLength.c_str());
            else
                fprintf(pHppFile, "\t%-*s %s[256];", 25, strCStyle.c_str(), strName.c_str());
        }
        else
        {
            fprintf(pHppFile, "\t%-*s %s;", 25, strCStyle.c_str(), strName.c_str());
        }
    }
    else
    {
        if (strType == "string")
        {
            m_strErrorInfo += "type=string������count����";
            return false;
        }
        fprintf(pHppFile, "\t%-*s %s[%s];", 25, strCStyle.c_str(), strName.c_str(), strCount.c_str());
    }

    if (!strComment.empty())
    {
        fprintf(pHppFile, " //%s", strComment.c_str());
    }
    fprintf(pHppFile, "\r\n");


    return true;
}

bool CProtocolMaker::__WriteStructProtocolEnCodeFunc( CMarkupSTL& rXml, FILE* pHppFile, FILE* pCppFile, bool bProtocol )
{
    std::string strName = rXml.GetAttrib("name");
    fprintf(pHppFile, "int EnCode%s(void* pHost, CNetData* poNetData);\r\n", strName.c_str());
    fprintf(pCppFile, "int EnCode%s(void* pHost, CNetData* poNetData)\r\n{\r\n", strName.c_str());
    fprintf(pCppFile, "\t%s* pstIn = (%s*)pHost;\r\n\r\n", strName.c_str(), strName.c_str());
    if (bProtocol)
    {
        fprintf(pCppFile, "\tif(-1==poNetData->AddWord(pstIn->moudleid))\r\n\t\treturn -1;\r\n\r\n");
        fprintf(pCppFile, "\tif(-1==poNetData->AddWord(pstIn->protocolid))\r\n\t\treturn -1;\r\n\r\n");
    }
    rXml.IntoElem();
    while (rXml.FindElem("item"))
    {
        std::string strName = rXml.GetAttrib("name");
        std::string strType = rXml.GetAttrib("type");
        std::string strCount = rXml.GetAttrib("count");
        std::string strRefer = rXml.GetAttrib("refer");
        std::string strSelect = rXml.GetAttrib("select");
        std::string strEnFuncName = "";

        if (strType == "uint8")
        {
            strEnFuncName = "AddByte";
        }
        else if (strType == "int8")
        {
            strEnFuncName = "AddChar";
        }
        else if (strType == "uint16")
        {
            strEnFuncName = "AddWord";
        }
        else if (strType == "int16")
        {
            strEnFuncName = "AddShort";
        }
        else if (strType == "uint32")
        {
            strEnFuncName = "AddDword";
        }
        else if (strType == "int32")
        {
            strEnFuncName = "AddInt";
        }
        else if (strType == "int64")
        {
            strEnFuncName = "AddInt64";
        }
        else if (strType == "uint64")
        {
            strEnFuncName = "AddQword";
        }
        //else if (strType == "blob")
        //{
        //    strEnFuncName = "AddBlob";
        //}

        if (__IsUnion(strType))
        {
            fprintf(pCppFile, "\tif(-1 == EnCode%s(&pstIn->%s, pstIn->%s, poNetData))\r\n\t\treturn -1;\r\n\r\n", strType.c_str(), strName.c_str(), strSelect.c_str());
        }
        else
        {
            if (strType == "string")
            {
                fprintf(pCppFile, "\tif(-1 == poNetData->AddString(pstIn->%s, sizeof(pstIn->%s)))\r\n\t\treturn -1;\r\n\r\n", strName.c_str(), strName.c_str());
            }
            else if (strCount.empty())
            {
                if (__IsKeyType(strType))
                {
                    fprintf(pCppFile, "\tif(-1 == poNetData->%s(pstIn->%s))\r\n\t\treturn -1;\r\n\r\n", strEnFuncName.c_str(), strName.c_str());
                }
                else
                {
                    fprintf(pCppFile, "\tif(-1 == EnCode%s(&pstIn->%s, poNetData))\r\n\t\treturn -1;\r\n\r\n", strType.c_str(), strName.c_str());
                }
            }
            else
            {
                if (__IsKeyType(strType))
                {
                    if (strRefer.empty())
                    {
                        fprintf(pCppFile, "\t{\r\n\t\tint iCount = %s;\r\n", strCount.c_str());
                        }
                        else
                        {
                            fprintf(pCppFile, "\t{\r\n\t\tint iCount = (((%s) < ((int)pstIn->%s)) ? (%s) : ((int)pstIn->%s));\r\n", strCount.c_str(), strRefer.c_str(), strCount.c_str(), strRefer.c_str());
                        }

                        std::string strCStyle = "";

                        if (strType == "uint8")
                        {
                            strCStyle = "unsigned char";
                        }
                        else if (strType == "int8")
                        {
                            strCStyle = "char";
                        }
                        else if (strType == "uint16")
                        {
                            strCStyle = "unsigned short";
                        }
                        else if (strType == "int16")
                        {
                            strCStyle = "short";
                        }
                        else if (strType == "uint32")
                        {
                            strCStyle = "unsigned int";
                        }
                        else if (strType == "int32")
                        {
                            strCStyle = "int";
                        }
                        else if (strType == "uint64")
                        {
                            strCStyle = "unsigned long long";
                        }
                        else if (strType == "int64")
                        {
                            strCStyle = "signed long long";
                        }

                        fprintf(pCppFile, "\t\tif(-1 == poNetData->AddBlob((char*)pstIn->%s, iCount*sizeof(%s)))\r\n\t\t\treturn -1;\r\n\t}\r\n", strName.c_str(), strCStyle.c_str());
                    }
                    else
                    {
                        fprintf(pCppFile, "\tfor(int i = 0; i < %s; i++)\r\n\t{\r\n", strCount.c_str());
                    if (!strRefer.empty())
                    {
                        fprintf(pCppFile, "\t\tif(i >= (int)pstIn->%s)\r\n\t\t\tbreak;\r\n", strRefer.c_str());
                    }

                    fprintf(pCppFile, "\t\tif(-1 == EnCode%s(&pstIn->%s[i], poNetData))\r\n\t\t\treturn -1;\r\n\t}\r\n\r\n", strType.c_str(), strName.c_str());
                }
            }
        }
    }
    rXml.OutOfElem();
    fprintf(pCppFile, "\treturn poNetData->GetDataLen();\r\n");
    fprintf(pCppFile, "}\r\n");
    return true;
}

bool CProtocolMaker::__WriteStructProtocolDeCodeFunc( CMarkupSTL& rXml, FILE* pHppFile, FILE* pCppFile, bool bProtocol )
{
    std::string strName = rXml.GetAttrib("name");
    fprintf(pHppFile, "int DeCode%s(void* pHost, CNetData* poNetData);\r\n", strName.c_str());
    fprintf(pCppFile, "int DeCode%s(void* pHost, CNetData* poNetData)\r\n{\r\n", strName.c_str());
    fprintf(pCppFile, "\t%s* pstOut = (%s*)pHost;\r\n\r\n", strName.c_str(), strName.c_str());
    if (bProtocol)
    {
        fprintf(pCppFile, "\tif(-1==poNetData->DelWord((unsigned short&)pstOut->moudleid))\r\n\t\treturn -1;\r\n\r\n");
        fprintf(pCppFile, "\tif(-1==poNetData->DelWord((unsigned short&)pstOut->protocolid))\r\n\t\treturn -1;\r\n\r\n");
    }
    rXml.IntoElem();
    while (rXml.FindElem("item"))
    {
        std::string strName = rXml.GetAttrib("name");
        std::string strType = rXml.GetAttrib("type");
        std::string strCount = rXml.GetAttrib("count");
        std::string strRefer = rXml.GetAttrib("refer");
        std::string strSelect = rXml.GetAttrib("select");
        std::string strEnFuncName = "";

        if (strType == "uint8")
        {
            strEnFuncName = "DelByte";
        }
        else if (strType == "int8")
        {
            strEnFuncName = "DelChar";
        }
        else if (strType == "uint16")
        {
            strEnFuncName = "DelWord";
        }
        else if (strType == "int16")
        {
            strEnFuncName = "DelShort";
        }
        else if (strType == "uint32")
        {
            strEnFuncName = "DelDword";
        }
        else if (strType == "int32")
        {
            strEnFuncName = "DelInt";
        }
        else if (strType == "int64")
        {
            strEnFuncName = "DelInt64";
        }
        else if (strType == "uint64")
        {
            strEnFuncName = "DelQword";
        }

        if (__IsUnion(strType))
        {
            fprintf(pCppFile, "\tif(-1 == DeCode%s(&pstOut->%s, pstOut->%s, poNetData))\r\n\t\treturn -1;\r\n\r\n", strType.c_str(), strName.c_str(), strSelect.c_str());
        }
        else
        {
            if (strType == "string")
            {
                fprintf(pCppFile, "\tif(-1 == poNetData->DelString(pstOut->%s, sizeof(pstOut->%s)))\r\n\t\treturn -1;\r\n\r\n", strName.c_str(), strName.c_str());
            }
            else if (strCount.empty())
            {
                if (__IsKeyType(strType))
                {
                    fprintf(pCppFile, "\tif(-1 == poNetData->%s(pstOut->%s))\r\n\t\treturn -1;\r\n\r\n", strEnFuncName.c_str(), strName.c_str());
                }
                else
                {
                    fprintf(pCppFile, "\tif(-1 == DeCode%s(&pstOut->%s, poNetData))\r\n\t\treturn -1;\r\n\r\n", strType.c_str(), strName.c_str());
                }
            }
            else
            {
                if (__IsKeyType(strType))
                {
                    if (strRefer.empty())
                    {
                        fprintf(pCppFile, "\t{\r\n\t\tint iCount = %s;\r\n\t\tif(iCount < 0)\r\n\t\t\treturn -1;\r\n", strCount.c_str());
                        }
                        else
                        {
                            fprintf(pCppFile, "\t{\r\n\t\tint iCount = (((%s) < ((int)pstOut->%s)) ? (%s) : ((int)pstOut->%s));\r\n\t\tif(iCount < 0)\r\n\t\t\treturn -1;\r\n", strCount.c_str(), strRefer.c_str(), strCount.c_str(), strRefer.c_str());
                        }

                        std::string strCStyle = "";

                        if (strType == "uint8")
                        {
                            strCStyle = "unsigned char";
                        }
                        else if (strType == "int8")
                        {
                            strCStyle = "char";
                        }
                        else if (strType == "uint16")
                        {
                            strCStyle = "unsigned short";
                        }
                        else if (strType == "int16")
                        {
                            strCStyle = "short";
                        }
                        else if (strType == "uint32")
                        {
                            strCStyle = "unsigned int";
                        }
                        else if (strType == "int32")
                        {
                            strCStyle = "int";
                        }
                        else if (strType == "uint64")
                        {
                            strCStyle = "unsigned long long";
                        }
                        else if (strType == "int64")
                        {
                            strCStyle = "signed long long";
                        }

                        fprintf(pCppFile, "\t\tif(-1 == poNetData->DelBlob((char*)pstOut->%s, iCount*sizeof(%s)))\r\n\t\t\treturn -1;\r\n\t}\r\n", strName.c_str(), strCStyle.c_str());
                    }
                    else
                    {
                        fprintf(pCppFile, "\tfor(int i = 0; i < %s; i++)\r\n\t{\r\n", strCount.c_str());
                    if (!strRefer.empty())
                    {
                        fprintf(pCppFile, "\t\tif(i >= (int)pstOut->%s)\r\n\t\t\tbreak;\r\n", strRefer.c_str());
                    }

                    fprintf(pCppFile, "\t\tif(-1 == DeCode%s(&pstOut->%s[i], poNetData))\r\n\t\t\treturn -1;\r\n\t}\r\n\r\n", strType.c_str(), strName.c_str());
                }
            }
        }
    }
    rXml.OutOfElem();
    fprintf(pCppFile, "\treturn sizeof(*pstOut);\r\n");
    fprintf(pCppFile, "}\r\n");

    return true;
}

bool CProtocolMaker::__WriteUnionEnCodeFunc( CMarkupSTL& rXml, FILE* pHppFile, FILE* pCppFile )
{
    std::string strName = rXml.GetAttrib("name");
    fprintf(pHppFile, "int EnCode%s(void* pHost, int iSelect, CNetData* poNetData);\r\n", strName.c_str());
    fprintf(pCppFile, "int EnCode%s(void* pHost, int iSelect, CNetData* poNetData)\r\n{\r\n\tswitch(iSelect){\r\n", strName.c_str());
    rXml.IntoElem();
    while (rXml.FindElem("item"))
    {
        std::string strId = rXml.GetAttrib("id");
        std::string strType = rXml.GetAttrib("type");
        if (strId.empty() || (!__IsStruct(strType)))
        {
            return false;
        }

        fprintf(pCppFile, "\tcase %s: return EnCode%s(pHost, poNetData);\r\n", strId.c_str(), strType.c_str());
    }
    rXml.OutOfElem();

    fprintf(pCppFile, "\tdefault: return -1;\r\n\t}\r\n}\r\n");
    return true;
}

bool CProtocolMaker::__WriteUnionDeCodeFunc( CMarkupSTL& rXml, FILE* pHppFile, FILE* pCppFile )
{
    std::string strName = rXml.GetAttrib("name");
    fprintf(pHppFile, "int DeCode%s(void* pHost, int iSelect, CNetData* poNetData);\r\n", strName.c_str());
    fprintf(pCppFile, "int DeCode%s(void* pHost, int iSelect, CNetData* poNetData)\r\n{\r\n\tswitch(iSelect){\r\n", strName.c_str());
    rXml.IntoElem();
    while (rXml.FindElem("item"))
    {
        std::string strId = rXml.GetAttrib("id");
        std::string strType = rXml.GetAttrib("type");
        if (strId.empty() || (!__IsStruct(strType)))
        {
            return false;
        }

        fprintf(pCppFile, "\tcase %s: return DeCode%s(pHost, poNetData);\r\n", strId.c_str(), strType.c_str());
    }
    rXml.OutOfElem();
    fprintf(pCppFile, "\tdefault: return -1;\r\n\t}\r\n}\r\n");
    return true;
}

bool CProtocolMaker::__WriteProtocolClass( const std::string& strProtocolName, FILE* pHppFile, FILE* pCppFile )
{
	if (m_vecProtocol.empty())
	{
		return true;
	}
    fprintf(pHppFile, "typedef int (*EnCodeFunc%s)(void *pHost, CNetData* poNetData);\r\ntypedef int (*DeCodeFunc%s)(void *pHost, CNetData* poNetData);\r\n\r\n", strProtocolName.c_str(), strProtocolName.c_str());
    fprintf(pHppFile, "class C%s\r\n{\r\npublic:\r\n\tC%s();\r\n\t~C%s();\r\n", strProtocolName.c_str(), strProtocolName.c_str(), strProtocolName.c_str());
    //��ӳ�Ա����
    fprintf(pHppFile, "\tint BuildProtocol(void* pHost, char *pNet, int iNetSize);\r\n\r\n");
    fprintf(pHppFile, "\tbool HandleProtocol(char *pNet, int iNetSize, void* pHost);\r\n\r\n");
    fprintf(pHppFile, "\tstatic inline unsigned short GetModuleID(void){ return %s; }\r\n\r\n", m_strMoudleID.c_str());
    fprintf(pHppFile, "\tstatic inline unsigned short GetProtocolNum(void){ return %d; }\r\n\r\n", m_vecProtocol.size());
    fprintf(pHppFile, "\tstatic const unsigned short module_id = %s;\r\n\r\n", m_strMoudleID.c_str());
    fprintf(pHppFile, "\tstatic const unsigned short protocol_num = %d;\r\n\r\n", m_vecProtocol.size());
    //��Ӹ�Э��Ļص����麯��
    fprintf(pHppFile, "//===============����Э��ص�������Ҫʹ������ʵ��===============\r\n");
    for (int i = 0; i < (int)m_vecProtocol.size(); i++)
    {
        fprintf(pHppFile, "\tvirtual void OnRecv_%s(%s& rstProtocol){ rstProtocol; };\r\n", m_vecProtocol[i].c_str(), m_vecProtocol[i].c_str());
    }
    fprintf(pHppFile, "private:\r\n\tEnCodeFunc%s m_EnCodeFuncArray[%d];\r\n\tEnCodeFunc%s m_DeCodeFuncArray[%d];\r\n};\r\n", strProtocolName.c_str(), m_mapProtocol.size(), strProtocolName.c_str(), m_mapProtocol.size());

    //���캯��
    fprintf(pCppFile, "C%s::C%s()\r\n{\r\n", strProtocolName.c_str(), strProtocolName.c_str());
    for (int i = 0; i < (int)m_vecProtocol.size(); i++)
    {
        fprintf(pCppFile, "\tm_EnCodeFuncArray[%d] = &EnCode%s;\r\n", i, m_vecProtocol[i].c_str());
        fprintf(pCppFile, "\tm_DeCodeFuncArray[%d] = &DeCode%s;\r\n\r\n", i, m_vecProtocol[i].c_str());
    }
    fprintf(pCppFile, "}\r\n\r\n");
    //��������
    fprintf(pCppFile, "C%s::~C%s()\r\n{\r\n}\r\n\r\n", strProtocolName.c_str(), strProtocolName.c_str());

    //����Э�麯��
    fprintf(pCppFile, "int C%s::BuildProtocol(void* pHost, char *pNet, int iNetSize)\r\n{\r\n", strProtocolName.c_str());
    fprintf(pCppFile, "\tCNetData m_oData;\r\n");
    fprintf(pCppFile, "\tm_oData.Prepare(pNet, iNetSize);\r\n");
    fprintf(pCppFile, "\tif (*(unsigned short*)pHost != %s)\r\n\t{\r\n\t\treturn -1;\r\n\t}\r\n", m_strMoudleID.c_str());
    fprintf(pCppFile, "\tif (*(unsigned short*)((char*)pHost+sizeof(unsigned short)) >= sizeof(m_EnCodeFuncArray)/sizeof(EnCodeFunc%s))\r\n\t{\r\n\t\treturn -1;\r\n\t}\r\n\treturn m_EnCodeFuncArray[*(unsigned short*)((char*)pHost+sizeof(unsigned short))](pHost, &m_oData);\r\n}\r\n\r\n", strProtocolName.c_str());


    //����Э�麯��
    fprintf(pCppFile, "bool C%s::HandleProtocol(char *pNet, int iNetSize, void* pHost)\r\n{\r\n", strProtocolName.c_str());
    fprintf(pCppFile, "\tCNetData m_oData;\r\n");
    fprintf(pCppFile, "\tm_oData.Prepare(pNet, iNetSize);\r\n\r\n\tunsigned short moudleid = 0;\r\n\tunsigned short protocolid = 0;\r\n\tif (m_oData.DelWord(moudleid) < 0)\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\tif (moudleid != %s)\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\tif (m_oData.DelWord(protocolid) < 0)\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\tif (protocolid >= sizeof(m_DeCodeFuncArray)/sizeof(DeCodeFunc%s))\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\r\n\tm_oData.Prepare(pNet, iNetSize);\r\n\r\n\tif (m_DeCodeFuncArray[protocolid](pHost, &m_oData) < 0)\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\r\n",m_strMoudleID.c_str(), strProtocolName.c_str());

    fprintf(pCppFile, "\tswitch(protocolid)\r\n\t{\r\n");
    for (int i = 0; i < (int)m_vecProtocol.size(); i++)
    {
        fprintf(pCppFile, "\tcase %d:\r\n\t\tOnRecv_%s(*(%s*)pHost);\r\n\t\tbreak;\r\n", i, m_vecProtocol[i].c_str(), m_vecProtocol[i].c_str());
    }
    fprintf(pCppFile, "\tdefault:\r\n\t\treturn false;\r\n\t}\r\n\r\n\treturn true;\r\n}\r\n\r\n");
    //Э��ص�����

    return true;
}

std::string& CProtocolMaker::GetErrorInfo( void )
{
    return m_strErrorInfo;
}


