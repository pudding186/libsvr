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

#define ERRORINFO_ITEM(strName, strType, strCount, strRefer, strId, strSelect, strLength, strComment, strArray)\
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
                                    if (!strArray.empty())\
                                    {\
                                        m_strErrorInfo += "comment=\""+strArray+"\" ";\
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
        m_strErrorInfo = "加载协议文件"+strXML+"失败! ";
        m_strErrorInfo += oXml.GetError();
        goto ERROR_DEAL;
    }

    //获取协议模块名作为文件名
    FIND_ELEM("protocol_define");
    GET_ATTRIB("name");

    strHppFile = strOutPutPath + strVal + ".hpp";
    strCppFile = strOutPutPath + strVal + ".cpp";
    pHppFile = fopen(strHppFile.c_str(), "wb");
    pCppFile = fopen(strCppFile.c_str(), "wb");
    if (NULL == pHppFile || NULL == pCppFile)
        goto ERROR_DEAL;

    fprintf(pHppFile, "#pragma once\r\n\r\n");
    fprintf(pHppFile, "#include \"net_data.hpp\"\r\n\r\n");
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
        m_strErrorInfo = "加载协议文件"+strPkg+"失败!";
        goto ERROR_DEAL;
    }

    CMarkupSTL& rXml = oXml;

    FIND_ELEM("protocol_define");
    INTO_ELEM();

    //查找宏定义
    rXml.ResetMainPos();
    while (rXml.FindElem("macro"))
    {
        std::string strName = rXml.GetAttrib("name");
        std::string strValue = rXml.GetAttrib("value");
        std::string strComment = rXml.GetAttrib("comment");
        if (strName.empty() ||strValue.empty())
        {
            m_strErrorInfo = "宏定义错误 name 或 value 属性为空";
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
                m_strErrorInfo += "重定义";
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
            m_strErrorInfo += "type属性的类型未知，只能定义为struct";
            return false;
        }

        if (!rXml.IntoElem())
        {
            m_strErrorInfo += "进入子级失败";
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
                m_strErrorInfo += "name属性值重复";
                return false;
            }
            else
                mapItem[vecAttrib[eName]] = vecAttrib;
        }

        if (!rXml.OutOfElem())
        {
            m_strErrorInfo += "从子集返回失败";
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
    //查找所有数据定义
    fprintf(pHppFile, "//===============包含的其他协议文件===============\r\n");
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
    //查找所有的宏定义
    fprintf(pHppFile, "//===============宏定义开始===============\r\n");
    rXml.ResetMainPos();
    while (rXml.FindElem("macro"))
    {
        std::string strName = rXml.GetAttrib("name");
        std::string strValue = rXml.GetAttrib("value");
        std::string strComment = rXml.GetAttrib("comment");
        if (strName.empty() ||strValue.empty())
        {
            m_strErrorInfo = "宏定义错误 name 或 value 属性为空";
            return false;
        }

        if (strComment.empty())
            fprintf(pHppFile, "#define %-*s %s\r\n", 30, strName.c_str(), strValue.c_str());
        else
            fprintf(pHppFile, "#define %-*s %s //%s\r\n", 30, strName.c_str(), strValue.c_str(), strComment.c_str());
        m_setMacro.insert(strName);
    }
    fprintf(pHppFile, "//===============宏定义结束===============\r\n\r\n");
    return true;
}

bool CProtocolMaker::__WriteData( FILE* pHppFile, CMarkupSTL& rXml )
{
    //查找所有数据定义
    fprintf(pHppFile, "//===============数据定义开始===============\r\n");
    //fprintf(pHppFile, "#pragma warning( push )\r\n#pragma warning( disable : 4512)\r\n");
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
                m_strErrorInfo += "重定义";
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
                m_strErrorInfo += " 重定义";
                return false;
            }
            fprintf(pHppFile, "struct %s:protocol_base{\r\n", strName.c_str());
            //fprintf(pHppFile,"\tconst %-*s moudleid;\r\n", 19, "unsigned short");
            //fprintf(pHppFile,"\tconst %-*s protocolid;\r\n", 19, "unsigned short");
            eDataType = eProtocol;
        }
        else
        {
            m_strErrorInfo += "type属性的类型未知，只能定义为struct,union,protocol";
            return false;
        }

        if (!rXml.IntoElem())
        {
            m_strErrorInfo += "进入子级失败";
            return false;
        }

        CItem mapItem;

        while (rXml.FindElem("item"))
        {
            CAttrib vecAttrib(9);
            vecAttrib[eName] = rXml.GetAttrib("name");
            vecAttrib[eType] = rXml.GetAttrib("type");
            vecAttrib[eCount] = rXml.GetAttrib("count");
            vecAttrib[eRefer] = rXml.GetAttrib("refer");
            vecAttrib[eId] = rXml.GetAttrib("id");
            vecAttrib[eSelect] = rXml.GetAttrib("select");
            vecAttrib[eComment] = rXml.GetAttrib("comment");
            vecAttrib[eLength] = rXml.GetAttrib("length");
            vecAttrib[eArray] = rXml.GetAttrib("array");

            CItem::iterator it  = mapItem.find(vecAttrib[eName]);
            if (it != mapItem.end())
            {
                m_strErrorInfo += "name属性值重复";
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
            m_strErrorInfo += "从子集返回失败";
            return false;
        }

        if (strType == "struct")
        {
			fprintf(pHppFile, "\tbool EnCode(NetEnCode& net_data);\r\n\tbool DeCode(NetDeCode& net_data);\r\n");
            m_mapStruct[strName] = mapItem;
        }
        else if (strType == "union")
        {
            m_mapUnion[strName] = mapItem;
        }
        else if (strType == "protocol")
        {
            //fprintf(pHppFile, "\t%s():moudleid(%s),protocolid(%d){}\r\n", strName.c_str(), m_strMoudleID.c_str(), m_vecProtocol.size());
            fprintf(pHppFile, "\t%s():protocol_base(%s, %d){}\r\n", strName.c_str(), m_strMoudleID.c_str(), m_vecProtocol.size());
			fprintf(pHppFile, "\tbool EnCode(NetEnCode& net_data);\r\n\tbool DeCode(NetDeCode& net_data);\r\n");
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
    //fprintf(pHppFile, "#pragma warning( pop ) \r\n");
    fprintf(pHppFile, "//===============数据定义结束===============\r\n");
    return true;
}

bool CProtocolMaker::__WriteDataFunction( FILE* pHppFile, FILE* pCppFile, CMarkupSTL& rXml )
{
    fprintf(pHppFile, "//===============打包解包函数定义开始===============\r\n");
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
    fprintf(pHppFile, "//===============打包解包函数定义结束===============\r\n");
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
    std::string strArray = rXml.GetAttrib("array");

    ERRORINFO_ITEM(strName, strType, strCount, strRefer, strId, strSelect, strLength, strComment, strArray);

    if (strName.empty() || strType.empty())
    {
        m_strErrorInfo += "中 name属性和type属性不能为空";
        return false;
    }

    if (eUnion == eDataType)
    {
        if (!__IsStruct(strType))
        {
            m_strErrorInfo += "union中的成员必须是struct";
            return false;
        }

        if (strId.empty())
        {
            m_strErrorInfo += "union中id属性不能为空";
            return false;
        }

        if (!__IsAllDigit(strId))
        {
            if (!__IsMacro(strId))
            {
                m_strErrorInfo += "union中id必须为数字或者是定义的宏";
                return false;
            }
        }

        if (!strCount.empty())
        {
            m_strErrorInfo += "union中不能有数组";
            return false;
        }

        if (!strArray.empty())
        {
            m_strErrorInfo += "union中不能有数组";
            return false;
        }

        if (!strRefer.empty())
        {
            m_strErrorInfo += "union中不能有refer属性";
            return false;
        }

        if (!strSelect.empty())
        {
            m_strErrorInfo += "union中不能有select属性";
            return false;
        }

        if (!strLength.empty())
        {
            m_strErrorInfo += "union中不能有length属性";
            return false;
        }
    }
    

    if (eStruct == eDataType || eProtocol == eDataType)
    {
        if (__IsKeyType(strType) || __IsStruct(strType))
        {
            if (!strArray.empty())
            {
                if (!strCount.empty())
                {
                    m_strErrorInfo += "有array属性就不能有count属性";
                    return false;
                }

                if (!strRefer.empty())
                {
                    m_strErrorInfo += "有array属性就不能有refer属性";
                    return false;
                }

                if (strArray != "uint8" &&
                    strArray != "uint16" &&
                    strArray != "uint32" &&
                    strArray != "uint64")
                {
                    m_strErrorInfo += "array属性值必须为 uint8, uint16, uint32, uint64 中的一种";
                    return false;
                }
            }

            if (!strCount.empty())
            {
                if (!__IsAllDigit(strCount))
                {
                    if (!__IsMacro(strCount))
                    {
                        m_strErrorInfo += "item中count必须为数字或者是定义的宏";
                        return false;
                    }
                }
            }

            if (!strRefer.empty())
            {
                if (strCount.empty())
                {
                    m_strErrorInfo += "item中有refer属性前必须先有count属性";
                    return false;
                }

                CItem::iterator it = mapItem.find(strRefer);
                if (it == mapItem.end())
                {
                    m_strErrorInfo += "item中refer的属性值未定义";
                    return false;
                }
                else
                {
                    if (!__IsKeyType((it->second)[eType]))
                    {
                        m_strErrorInfo += "item中refer的属性值必须是基础数据类型的变量";
                        return false;
                    }

                    if (!(it->second)[eCount].empty())
                    {
                        m_strErrorInfo += "item中refer的属性值不能是数组";
                        return false;
                    }
                }
            }
            if (!strId.empty())
            {
                m_strErrorInfo += "只有data是union才能有此属性";
                return false;
            }
            if (!strSelect.empty())
            {
                m_strErrorInfo += "只有type=union才能有此属性";
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
                            m_strErrorInfo += "item中length必须为数字或者是定义的宏";
                            return false;
                        }
                    }
                    else
                    {
                        if (atoi(strLength.c_str()) > 65535)
                        {
                            m_strErrorInfo += "string类型的length属性最大长度不能超过65535";
                            return false;
                        }
                    }
                }
                else
                {
                    m_strErrorInfo += "只有type=string才能有此属性";
                    return false;
                }
            }
        }
        else if (__IsUnion(strType))
        {
            if (strSelect.empty())
            {
                m_strErrorInfo += "union中必须有select属性";
                return false;
            }
            else
            {
                CItem::iterator it = mapItem.find(strSelect);
                if (it == mapItem.end())
                {
                    m_strErrorInfo += "item中select的属性值未定义";
                    return false;
                }
                else
                {
                    if (!__IsKeyType((it->second)[eType]))
                    {
                        m_strErrorInfo += "item中select的属性值必须是基础数据类型的变量";
                        return false;
                    }

                    if (!(it->second)[eCount].empty())
                    {
                        m_strErrorInfo += "item中select的属性值不能为数组";
                        return false;
                    }
                }
            }
            if (!strCount.empty())
            {
                m_strErrorInfo += "type=union不能有count属性";
                return false;
            }
            if (!strRefer.empty())
            {
                m_strErrorInfo += "type=union不能有refer属性";
                return false;
            }
            if (!strArray.empty())
            {
                m_strErrorInfo += "type=union不能有array属性";
                return false;
            }
            if (!strId.empty())
            {
                m_strErrorInfo += "type=union不能有id属性";
                return false;
            }
            if (!strLength.empty())
            {
                m_strErrorInfo += "type=union不能有length属性";
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

    if (strCount.empty() && strArray.empty())
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
        if (!strCount.empty())
        {
            if (strType == "string")
            {
                m_strErrorInfo += "type=string不能有count属性";
                return false;
            }
            fprintf(pHppFile, "\t%-*s %s[%s];", 25, strCStyle.c_str(), strName.c_str(), strCount.c_str());
        }

        if (!strArray.empty())
        {
            std::string strArrayCtype;
            if (strArray == "uint8")
            {
                strArrayCtype = "unsigned char";
            }
            else if (strArray == "uint16")
            {
                strArrayCtype = "unsigned short";
            }
            else if (strArray == "uint32")
            {
                strArrayCtype = "unsigned int";
            }
            else if (strArray == "uint64")
            {
                strArrayCtype = "unsigned long long";
            }


            if (strType == "string")
            {
                m_strErrorInfo += "type=string不能有array属性";
                return false;
            }
            std::string array_name = "DataArray<" + strCStyle + ", " + strArrayCtype + ">";
            fprintf(pHppFile, "\t%-*s %s;", 25, array_name.c_str(), strName.c_str());
        }
    }

    if (!strComment.empty())
    {
        fprintf(pHppFile, " //%s", strComment.c_str());
    }
    fprintf(pHppFile, "\r\n");


    return true;
}

std::string __integral_to_c_type(std::string& str_integral)
{
	if (str_integral == "uint8")
	{
		return "unsigned char";
	}
	else if (str_integral == "int8")
	{
		return "char";
	}
	else if (str_integral == "uint16")
	{
		return "unsigned short";
	}
	else if (str_integral == "int16")
	{
		return "short";
	}
	else if (str_integral == "uint32")
	{
		return "unsigned int";
	}
	else if (str_integral == "int32")
	{
		return "int";
	}
	else if (str_integral == "uint64")
	{
		return "unsigned long long";
	}
	else if (str_integral == "int64")
	{
		return "long long";
	}

	return "";
}

bool __is_integral(std::string& strType)
{
	if (strType == "uint8"	||
		strType == "int8"	||
		strType == "uint16" ||
		strType == "int16"	||
		strType == "uint32" ||
		strType == "int32"	||
		strType == "uint64" ||
		strType == "int64" )
	{
		return true;
	}

	return false;
}

bool CProtocolMaker::__WriteStructProtocolEnCodeFunc(CMarkupSTL& rXml, FILE* pHppFile, FILE* pCppFile, bool bProtocol)
{
	std::string strName = rXml.GetAttrib("name");
	fprintf(pCppFile, "bool %s::EnCode(NetEnCode& net_data)\r\n{\r\n", strName.c_str());
	
	bool is_empty = true;
	rXml.IntoElem();
	while (rXml.FindElem("item"))
	{
		is_empty = false;
		std::string strName = rXml.GetAttrib("name");
		std::string strType = rXml.GetAttrib("type");
		std::string strCount = rXml.GetAttrib("count");
		std::string strRefer = rXml.GetAttrib("refer");
		std::string strSelect = rXml.GetAttrib("select");
		std::string strArray = rXml.GetAttrib("array");
		std::string strEnFuncName = "";

		//if (is_integral(strType))
		//{
		//	strEnFuncName = "AddIntegral"
		//}
		
		if (strType == "string")
		{
			fprintf(pCppFile, "\tnet_data.AddString(%s, sizeof(%s));\r\n\r\n", strName.c_str(), strName.c_str());
		}
		else if (strCount.empty() && strArray.empty())
		{
			if (__is_integral(strType))
			{
				fprintf(pCppFile, "\tnet_data.AddIntegral(%s);\r\n\r\n", strName.c_str());
			}
			else
			{
				fprintf(pCppFile, "\t%s.EnCode(net_data);", strName.c_str());
			}
		}
		else
		{
			if (!strArray.empty())
			{
				//if (__is_integral(strType))
				//{
				//	fprintf(pCppFile, "\tnet_data.AddIntegral(%s.size());\r\n", strName.c_str());
				//	fprintf(pCppFile, "\tnet_data.AddBlob(&%s[0], %s.size() * %s.size_of_data());", strName.c_str(), strName.c_str(), strName.c_str());
				//}
				//else
				//{
				//	fprintf(pCppFile, "\tfor (%s i = 0; i < %s.size(); i++)\r\n", __integral_to_c_type(strArray).c_str(), strName.c_str());
				//	fprintf(pCppFile, "\t{\r\n");
				//	fprintf(pCppFile, "\t\t%s[i].EnCode(net_data);\r\n", strName.c_str());
				//	fprintf(pCppFile, "\t}\r\n\r\n");
				//}
				fprintf(pCppFile, "\tnet_data.AddArray(%s);\r\n\r\n", strName.c_str());
			}

			if (!strCount.empty())
			{
				if (__is_integral(strType))
				{
					if (strRefer.empty())
					{
						fprintf(pCppFile, "\t{\r\n\t\tint iCount = %s;\r\n", strCount.c_str());
					}
					else
					{
						fprintf(pCppFile, "\t{\r\n\t\tint iCount = (((%s) < ((int)this->%s)) ? (%s) : ((int)this->%s));\r\n", strCount.c_str(), strRefer.c_str(), strCount.c_str(), strRefer.c_str());
					}

					fprintf(pCppFile, "\t\tnet_data.AddBlob((char*)this->%s, iCount*sizeof(%s));\r\n\t}\r\n", strName.c_str(), __integral_to_c_type(strType).c_str());
				}
				else
				{
					fprintf(pCppFile, "\tfor(int i = 0; i < %s; i++)\r\n\t{\r\n", strCount.c_str());
					if (!strRefer.empty())
					{
						fprintf(pCppFile, "\t\tif(i >= (int)this->%s)\r\n\t\t\tbreak;\r\n", strRefer.c_str());
					}

					fprintf(pCppFile, "\t\t%s[i].EnCode(net_data);\r\n\t}\r\n\r\n", strName.c_str());
				}
			}
		}
	}
	rXml.OutOfElem();

	if (is_empty)
	{
		fprintf(pCppFile, "\tnet_data;\r\n");
	}

	fprintf(pCppFile, "\treturn true;\r\n");
	fprintf(pCppFile, "}\r\n");
	return true;
}

bool CProtocolMaker::__WriteStructProtocolDeCodeFunc(CMarkupSTL& rXml, FILE* pHppFile, FILE* pCppFile, bool bProtocol)
{
	std::string strName = rXml.GetAttrib("name");
	fprintf(pCppFile, "bool %s::DeCode(NetDeCode& net_data)\r\n{\r\n", strName.c_str());
	bool is_empty = true;
	rXml.IntoElem();
	while (rXml.FindElem("item"))
	{
		is_empty = false;
		std::string strName = rXml.GetAttrib("name");
		std::string strType = rXml.GetAttrib("type");
		std::string strCount = rXml.GetAttrib("count");
		std::string strRefer = rXml.GetAttrib("refer");
		std::string strSelect = rXml.GetAttrib("select");
		std::string strArray = rXml.GetAttrib("array");

		if (strType == "string")
		{
			fprintf(pCppFile, "\tif (!net_data.DelString(%s, sizeof(%s)))\r\n\t\treturn false;\r\n\r\n", strName.c_str(), strName.c_str());
		}
		else if (strCount.empty() && strArray.empty())
		{
			if (__is_integral(strType))
			{
				fprintf(pCppFile, "\tif (!net_data.DelIntegral(%s))\r\n\t\treturn false;\r\n\r\n", strName.c_str());
			}
			else
			{
				fprintf(pCppFile, "\tif (!%s.DeCode(net_data))\r\n\t\treturn false;\r\n\r\n", strName.c_str());
			}
		}
		else
		{
			if (!strArray.empty())
			{
				//if (__is_integral(strType))
				//{
				//	fprintf(pCppFile, "\t{\r\n");
				//	fprintf(pCppFile, "\t\t%s array_size = 0;\r\n", __integral_to_c_type(strArray).c_str());
				//	fprintf(pCppFile, "\t\tif (!net_data.DelIntegral(array_size))\r\n\t\t\treturn false;\r\n");
				//	fprintf(pCppFile, "\t\t%s.resize(array_size);\r\n", strName.c_str());
				//	fprintf(pCppFile, "\t\tif (!net_data.DelBlob(&%s[0], array_size*%s.size_of_data()))\r\n\t\t\treturn false;\r\n", strName.c_str(), strName.c_str());
				//	fprintf(pCppFile, "\t}\r\n\r\n");
				//}
				//else
				//{
				//	fprintf(pCppFile, "\t{\r\n");
				//	fprintf(pCppFile, "\t\t%s array_size = 0;\r\n", __integral_to_c_type(strArray).c_str());
				//	fprintf(pCppFile, "\t\tif (!net_data.DelIntegral(array_size))\r\n\t\t\treturn false;\r\n");
				//	fprintf(pCppFile, "\t\t%s.resize(array_size);\r\n", strName.c_str());
				//	fprintf(pCppFile, "\t\tfor (%s i = 0; i < %s.size(); i++)\r\n", __integral_to_c_type(strArray).c_str(), strName.c_str());
				//	fprintf(pCppFile, "\t\t{\r\n");
				//	fprintf(pCppFile, "\t\t\tif (!%s[i].DeCode(net_data))\r\n\t\t\t\treturn false;\r\n", strName.c_str());
				//	fprintf(pCppFile, "\t\t}\r\n\r\n");
				//	fprintf(pCppFile, "\t}\r\n\r\n");
				//}
				fprintf(pCppFile, "\tif(!net_data.DelArray(%s))\r\n\t\treturn false;\r\n\r\n", strName.c_str());
			}

			if (!strCount.empty())
			{
				if (__is_integral(strType))
				{
					if (strRefer.empty())
					{
						fprintf(pCppFile, "\t{\r\n\t\tint iCount = %s;\r\n\t\tif(iCount < 0)\r\n\t\t\treturn false;\r\n", strCount.c_str());
					}
					else
					{
						fprintf(pCppFile, "\t{\r\n\t\tint iCount = (((%s) < ((int)this->%s)) ? (%s) : ((int)this->%s));\r\n\t\tif(iCount < 0)\r\n\t\t\treturn false;\r\n", strCount.c_str(), strRefer.c_str(), strCount.c_str(), strRefer.c_str());
					}

					fprintf(pCppFile, "\t\tif(!net_data.DelBlob((char*)this->%s, iCount*sizeof(%s)))\r\n\t\t\treturn false;\r\n\t}\r\n\r\n", strName.c_str(), __integral_to_c_type(strType).c_str());
				}
				else
				{
					fprintf(pCppFile, "\tfor(int i = 0; i < %s; i++)\r\n\t{\r\n", strCount.c_str());
					if (!strRefer.empty())
					{
						fprintf(pCppFile, "\t\tif(i >= (int)this->%s)\r\n\t\t\tbreak;\r\n", strRefer.c_str());
					}

					fprintf(pCppFile, "\t\tif(!%s[i].DeCode(net_data))\r\n\t\t\treturn false;\r\n\t}\r\n\r\n", strName.c_str());
				}
			}
		}
	}
	rXml.OutOfElem();

	if (is_empty)
	{
		fprintf(pCppFile, "\tnet_data;\r\n");
	}

	fprintf(pCppFile, "\treturn true;\r\n");
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

    //fprintf(pHppFile, "typedef int (*EnCodeFunc%s)(void *pHost, CNetData* poNetData);\r\ntypedef int (*DeCodeFunc%s)(void *pHost, CNetData* poNetData);\r\n\r\n", strProtocolName.c_str(), strProtocolName.c_str());
    fprintf(pHppFile, "class C%s\r\n{\r\npublic:\r\n\tC%s();\r\n\t~C%s();\r\n", strProtocolName.c_str(), strProtocolName.c_str(), strProtocolName.c_str());
    //添加成员函数
    //fprintf(pHppFile, "\tint BuildProtocol(void* pHost, char *pNet, int iNetSize);\r\n\r\n");
    //fprintf(pHppFile, "\tbool HandleProtocol(char *pNet, int iNetSize, void* pHost);\r\n\r\n");
	fprintf(pHppFile, "\tbool BuildProtocol(protocol_base* proto, NetEnCode& net_data);\r\n\r\n");
	fprintf(pHppFile, "\tbool HandleProtocol(NetDeCode& net_data);\r\n\r\n");
    fprintf(pHppFile, "\tstatic inline unsigned short GetModuleID(void){ return %s; }\r\n\r\n", m_strMoudleID.c_str());
    fprintf(pHppFile, "\tstatic inline unsigned short GetProtocolNum(void){ return %d; }\r\n\r\n", m_vecProtocol.size());
    fprintf(pHppFile, "\tstatic const unsigned short module_id = %s;\r\n\r\n", m_strMoudleID.c_str());
    fprintf(pHppFile, "\tstatic const unsigned short protocol_num = %d;\r\n\r\n", m_vecProtocol.size());
    //添加各协议的回调纯虚函数
    fprintf(pHppFile, "//===============以下协议回调函数需要使用者来实现===============\r\n");
    for (int i = 0; i < (int)m_vecProtocol.size(); i++)
    {
        fprintf(pHppFile, "\tvirtual void OnRecv_%s(%s& rstProtocol){ rstProtocol; };\r\n", m_vecProtocol[i].c_str(), m_vecProtocol[i].c_str());
    }
    //fprintf(pHppFile, "private:\r\n\tEnCodeFunc%s m_EnCodeFuncArray[%d];\r\n\tEnCodeFunc%s m_DeCodeFuncArray[%d];\r\n\tchar* m_EnCodeBuffer;\r\n\tchar* m_DeCodeBuffer;\r\n\tsize_t m_EnCodeBufferSize;\r\n\tsize_t m_DeCodeBufferSize;\r\n};\r\n", strProtocolName.c_str(), m_mapProtocol.size(), strProtocolName.c_str(), m_mapProtocol.size());
	fprintf(pHppFile, "private:\r\n\t void* m_protocol_buffer;\r\n");
	fprintf(pHppFile, "\r\n};\r\n");

    //构造函数
    fprintf(pCppFile, "C%s::C%s()\r\n{\r\n\tsize_t max_protocol_size = 0;\r\n", strProtocolName.c_str(), strProtocolName.c_str());
    //for (int i = 0; i < (int)m_vecProtocol.size(); i++)
    //{
    //    fprintf(pCppFile, "\tm_EnCodeFuncArray[%d] = &EnCode%s;\r\n", i, m_vecProtocol[i].c_str());
    //    fprintf(pCppFile, "\tm_DeCodeFuncArray[%d] = &DeCode%s;\r\n", i, m_vecProtocol[i].c_str());
    //}
	for (size_t i = 0; i < m_vecProtocol.size(); i++)
	{
		fprintf(pCppFile, "\tif (sizeof(%s) > max_protocol_size)\r\n\t\tmax_protocol_size = sizeof(%s);\r\n\r\n", m_vecProtocol[i].c_str(), m_vecProtocol[i].c_str());
	}

	fprintf(pCppFile, "\tm_protocol_buffer = S_MALLOC(max_protocol_size);\r\n");

    fprintf(pCppFile, "}\r\n\r\n");
    //析构函数
	fprintf(pCppFile, "C%s::~C%s()\r\n{\r\n", strProtocolName.c_str(), strProtocolName.c_str());
	//for (int i = 0; i < (int)m_vecProtocol.size(); i++)
	//{
	//    fprintf(pCppFile, "\tm_EnCodeFuncArray[%d] = &EnCode%s;\r\n", i, m_vecProtocol[i].c_str());
	//    fprintf(pCppFile, "\tm_DeCodeFuncArray[%d] = &DeCode%s;\r\n", i, m_vecProtocol[i].c_str());
	//}
	fprintf(pCppFile, "\tif (m_protocol_buffer)\r\n");
	fprintf(pCppFile, "\t{\r\n");
	fprintf(pCppFile, "\t\tS_FREE(m_protocol_buffer);\r\n");
	fprintf(pCppFile, "\t\tm_protocol_buffer = 0;\r\n");
	fprintf(pCppFile, "\t}\r\n");

	fprintf(pCppFile, "}\r\n\r\n");
    //fprintf(pCppFile, "C%s::~C%s()\r\n{\r\n\tif (m_EnCodeBuffer)\r\n\t{\r\n\t\tS_FREE(m_EnCodeBuffer);\r\n\t}\r\n\r\n\tif (m_DeCodeBuffer)\r\n\t{\r\n\t\tS_FREE(m_DeCodeBuffer);\r\n\t}\r\n}\r\n\r\n", strProtocolName.c_str(), strProtocolName.c_str());

    //构建协议函数
    //fprintf(pCppFile, "int C%s::BuildProtocol(void* pHost, char *pNet, int iNetSize)\r\n{\r\n", strProtocolName.c_str());
	fprintf(pCppFile, "bool C%s::BuildProtocol(protocol_base* proto, NetEnCode& net_data)\r\n{\r\n", strProtocolName.c_str());
	fprintf(pCppFile, "\tif (proto->module_id != %s)\r\n", m_strMoudleID.c_str());
	fprintf(pCppFile, "\t\treturn false;\r\n\r\n");
	fprintf(pCppFile, "\tnet_data.AddIntegral(proto->module_id);\r\n");
	fprintf(pCppFile, "\tnet_data.AddIntegral(proto->protocol_id);\r\n\r\n");

	fprintf(pCppFile, "\tproto->EnCode(net_data);\r\n\r\n");

	//fprintf(pCppFile, "\tswitch(proto->protocol_id)\r\n\t{\r\n");

	//for (size_t i = 0; i < m_vecProtocol.size(); i++)
	//{
	//	fprintf(pCppFile, "\tcase %zu:\r\n", i);
	//	fprintf(pCppFile, "\t{\r\n");
	//	fprintf(pCppFile, "\t\t((%s*)proto)->EnCode(net_data);\r\n", m_vecProtocol[i].c_str());
	//	fprintf(pCppFile, "\t}\r\n\tbreak;\r\n");
	//}

	//fprintf(pCppFile, "\tdefault:\r\n\t\treturn false;\r\n\t}\r\n\r\n");



	fprintf(pCppFile, "\treturn true;\r\n\r\n");
	fprintf(pCppFile, "}\r\n\r\n");
    //fprintf(pCppFile, "\tCNetData m_oData;\r\n");
    //fprintf(pCppFile, "\tm_oData.Prepare(pNet, iNetSize);\r\n");
    //fprintf(pCppFile, "\tif (*(unsigned short*)pHost != %s)\r\n\t{\r\n\t\treturn -1;\r\n\t}\r\n", m_strMoudleID.c_str());
    //fprintf(pCppFile, "\tif (*(unsigned short*)((char*)pHost+sizeof(unsigned short)) >= sizeof(m_EnCodeFuncArray)/sizeof(EnCodeFunc%s))\r\n\t{\r\n\t\treturn -1;\r\n\t}\r\n\treturn m_EnCodeFuncArray[*(unsigned short*)((char*)pHost+sizeof(unsigned short))](pHost, &m_oData);\r\n}\r\n\r\n", strProtocolName.c_str());


    //处理协议函数
    fprintf(pCppFile, "bool C%s::HandleProtocol(NetDeCode& net_data)\r\n{\r\n", strProtocolName.c_str());
	fprintf(pCppFile, "\tunsigned short m_id = 0;\r\n");
	fprintf(pCppFile, "\tunsigned short p_id = 0;\r\n");
	fprintf(pCppFile, "\tsize_t net_data_pos = net_data.CurPos();\r\n\r\n");
	fprintf(pCppFile, "\tif (!net_data.DelIntegral(m_id) || !net_data.DelIntegral(p_id))\r\n");
	fprintf(pCppFile, "\t{\r\n");
	fprintf(pCppFile, "\t\tnet_data.Reset(net_data_pos);\r\n");
	fprintf(pCppFile, "\t\treturn false;\r\n");
	fprintf(pCppFile, "\t}\r\n\r\n");
	fprintf(pCppFile, "\tif (m_id != %s)\r\n", m_strMoudleID.c_str());
	fprintf(pCppFile, "\t{\r\n");
	fprintf(pCppFile, "\t\tnet_data.Reset(net_data_pos);\r\n");
	fprintf(pCppFile, "\t\treturn false;\r\n");
	fprintf(pCppFile, "\t}\r\n\r\n");
	fprintf(pCppFile, "\tswitch(p_id)\r\n\t{\r\n");

	for (size_t i = 0; i < m_vecProtocol.size(); i++)
	{
		fprintf(pCppFile, "\tcase %zu:\r\n", i);
		fprintf(pCppFile, "\t{\r\n");
		//fprintf(pCppFile, "\t\t%s* proto = (%s*)m_protocol_buffer;\r\n", m_vecProtocol[i].c_str(), m_vecProtocol[i].c_str());
		//fprintf(pCppFile, "\t\tnew(proto)%s();\r\n", m_vecProtocol[i].c_str());
		fprintf(pCppFile, "\t\t%s* proto = new(m_protocol_buffer)%s();\r\n", m_vecProtocol[i].c_str(), m_vecProtocol[i].c_str());
		fprintf(pCppFile, "\t\tif (proto->DeCode(net_data))\r\n");
		fprintf(pCppFile, "\t\t{\r\n");
		fprintf(pCppFile, "\t\t\tOnRecv_%s(*proto);\r\n", m_vecProtocol[i].c_str());
		fprintf(pCppFile, "\t\t\tproto->~%s();\r\n", m_vecProtocol[i].c_str());
		fprintf(pCppFile, "\t\t\treturn true;\r\n");
		fprintf(pCppFile, "\t\t}\r\n\t\telse\r\n\t\t{\r\n");
		fprintf(pCppFile, "\t\t\tproto->~%s();\r\n", m_vecProtocol[i].c_str());
		fprintf(pCppFile, "\t\t\tnet_data.Reset(net_data_pos);\r\n");
		fprintf(pCppFile, "\t\t\treturn false;\r\n");
		fprintf(pCppFile, "\t\t}\r\n");
		fprintf(pCppFile, "\t}\r\n\tbreak;\r\n");
	}

	fprintf(pCppFile, "\tdefault:\r\n\t\treturn false;\r\n\t}\r\n\r\n");
	fprintf(pCppFile, "}\r\n\r\n");



    //fprintf(pCppFile, "\tCNetData m_oData;\r\n");
    //fprintf(pCppFile, "\tm_oData.Prepare(pNet, iNetSize);\r\n\r\n\tunsigned short moudleid = 0;\r\n\tunsigned short protocolid = 0;\r\n\tif (m_oData.DelWord(moudleid) < 0)\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\tif (moudleid != %s)\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\tif (m_oData.DelWord(protocolid) < 0)\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\tif (protocolid >= sizeof(m_DeCodeFuncArray)/sizeof(DeCodeFunc%s))\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\r\n\tm_oData.Prepare(pNet, iNetSize);\r\n\r\n\tif (m_DeCodeFuncArray[protocolid](pHost, &m_oData) < 0)\r\n\t{\r\n\t\treturn false;\r\n\t}\r\n\r\n",m_strMoudleID.c_str(), strProtocolName.c_str());

    //fprintf(pCppFile, "\tswitch(protocolid)\r\n\t{\r\n");
    //for (int i = 0; i < (int)m_vecProtocol.size(); i++)
    //{
    //    fprintf(pCppFile, "\tcase %d:\r\n\t\tOnRecv_%s(*(%s*)pHost);\r\n\t\tbreak;\r\n", i, m_vecProtocol[i].c_str(), m_vecProtocol[i].c_str());
    //}
    //fprintf(pCppFile, "\tdefault:\r\n\t\treturn false;\r\n\t}\r\n\r\n\treturn true;\r\n}\r\n\r\n");
    //协议回调函数

    return true;
}

std::string& CProtocolMaker::GetErrorInfo( void )
{
    return m_strErrorInfo;
}


