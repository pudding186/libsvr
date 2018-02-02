// excel_testDlg.cpp : implementation file
//

#include "stdafx.h"
#include "excel_test.h"
#include "excel_testDlg.h"
#include "SimpleExcel.h"

#include <Mmsystem.h>
#include <map>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cexcel_testDlg dialog




Cexcel_testDlg::Cexcel_testDlg(CWnd* pParent /*=NULL*/)
	: CDialog(Cexcel_testDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cexcel_testDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_EXCEL, m_excel_list);
    DDX_Control(pDX, IDC_LIST_RESULT, m_result_box);
}

BEGIN_MESSAGE_MAP(Cexcel_testDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
    //ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_EXCEL, &Cexcel_testDlg::OnLvnItemchangedList1)
    ON_BN_CLICKED(IDCANCEL, &Cexcel_testDlg::OnClose)
    ON_BN_CLICKED(IDC_BUTTON_LOAD, &Cexcel_testDlg::OnBnClickedButtonLoad)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BUTTON_CODE, &Cexcel_testDlg::OnBnClickedButtonCode)
    ON_BN_CLICKED(IDC_BUTTON_XML, &Cexcel_testDlg::OnBnClickedButtonXml)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_MENU_ALL_CANCEL, &Cexcel_testDlg::OnMenuAllCancel)
    ON_COMMAND(ID_MENU_ALL_SELECT, &Cexcel_testDlg::OnMenuAllSelect)
    ON_COMMAND(ID_MENU_ALL_REVERSE, &Cexcel_testDlg::OnMenuAllReverse)
    ON_COMMAND(ID_MENU_LOAD, &Cexcel_testDlg::OnMenuLoad)
    ON_COMMAND(ID_RESULT_CLEAR, &Cexcel_testDlg::OnMenuClear)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// Cexcel_testDlg message handlers

//typedef bool (*pfn_file)(const char* path, const char* file, void* user_data);
//typedef bool (*pfn_dir)(const char* dir, void* user_data);
//
//extern bool (for_each_file)(const char* dir, pfn_file do_file, pfn_dir do_dir, void* user_data);

bool open_file(const char* path, const char* file, void* user_data)
{
    Cexcel_testDlg* dlg = (Cexcel_testDlg*)user_data;

    return dlg->open_excel(path, file);
}

BOOL Cexcel_testDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    ListView_SetExtendedListViewStyle(m_excel_list.GetSafeHwnd(), m_excel_list.GetExStyle() | LVS_EX_CHECKBOXES);
    m_excel_list.InsertColumn(0,"Excel",LVCFMT_LEFT,150);

    CMarkupSTL xml;
    xml.Load("./table_gen_cfg.xml");

    xml.ResetMainPos();

    if (xml.FindElem("path"))
    {
        xml.IntoElem();
        xml.ResetMainPos();
        if (xml.FindElem("excel_path"))
        {
            m_excel_path = xml.GetAttrib("name");
        }
        xml.ResetMainPos();
        if (xml.FindElem("code_path"))
        {
            m_code_path = xml.GetAttrib("name");
        }
        xml.ResetMainPos();
        if (xml.FindElem("xml_path"))
        {
            m_xml_path = xml.GetAttrib("name");
        }
        xml.OutOfElem();
    }

    m_excel_file_list.clear();

    const char* ptr = m_excel_path.c_str() + m_excel_path.length()-1;
    if (*ptr == '\\')
    {
        *(char*)ptr = '/';
    }
    else if (*ptr != '/')
    {
        m_excel_path.append("/");
    }

    ptr = m_code_path.c_str() + m_code_path.length()-1;
    
    if (*ptr == '\\')
    {
        *(char*)ptr = '/';
    }
    else if (*ptr != '/')
    {
        m_code_path.append("/");
    }

    ptr = m_xml_path.c_str() + m_xml_path.length()-1;
    if (*ptr == '\\')
    {
        *(char*)ptr = '/';
    }
    else if (*ptr != '/')
    {
        m_xml_path.append("/");
    }

    for_each_file(m_excel_path.c_str(), open_file, 0, this);
    //m_excel_list.InsertColumn(1,"Age",LVCFMT_LEFT,200);

    for (size_t i = 0; i < m_excel_file_list.size(); i++)
    {
        std::string name = m_excel_file_list[i];

        name = name.substr(0, name.length() - strlen(".xlsx"));
        m_excel_list.InsertItem(i, name.c_str());
    }
    //m_excel_list.InsertItem(0, "God");
    //m_excel_list.InsertItem(1, "YuDi");

    //m_excel_list.GetCheck(0);

    //m_excel_list.SetCheck(1, TRUE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Cexcel_testDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Cexcel_testDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

std::string excel_to_xml(const std::string& excel_path, CTableMaker& table_server, CTableMaker& table_client, const std::string& xml_path)
{
    try
    {
    std::map<std::string, long> col_name_2_idx;
    std::map<long, std::string> col_idx_2_name;

    CSimpleExcel excel;

    if (!excel.OpenExcelFile(excel_path.c_str()))
    {
        char err[512];
        sprintf_s(err, "Open Excel: %s fail", excel_path.c_str());
        return err;
    }

    if (!excel.LoadSheet("content", TRUE))
    {
        return "Open sheet def fail";
    }

    long row_count = excel.GetRowCount();
    long col_count = excel.GetColumnCount();

    
    for (long col = 1; col <= col_count; col++)
    {
        CString str = excel.GetCellString(1, col);
        if (str.GetLength())
        {
            col_name_2_idx[str.GetBuffer()] = col;
            col_idx_2_name[col] = str.GetBuffer();
        }
    }

    if (col_name_2_idx.size() != col_idx_2_name.size())
    {
        return "有重复的数据列";
    }

    table_column_info svr_col_info = table_server.get_table_columns();
    table_column_info cli_col_info = table_client.get_table_columns();

    table_column_info all_col_info;

    for (table_column_info::iterator it = svr_col_info.begin();
        it != svr_col_info.end(); ++it)
    {
        column_info& info = it->second;

        if (col_name_2_idx.find(info.m_col_name) == col_name_2_idx.end())
        {
            std::string err = "content 中没有" + info.m_col_name + "列数据";
            return err;
        }

        all_col_info[info.m_col_name] = info;
    }

    for (table_column_info::iterator it = cli_col_info.begin();
        it != cli_col_info.end(); ++it)
    {
        column_info& info = it->second;

        if (col_name_2_idx.find(info.m_col_name) == col_name_2_idx.end())
        {
            std::string err = "content 中没有" + info.m_col_name + "列数据";
            return err;
        }

        all_col_info[info.m_col_name] = info;
    }

    std::string xml_file_path = xml_path + table_server.get_table_name() + ".xml";

    FILE* xml_file = fopen(xml_file_path.c_str(), "wb");

    if (!xml_file)
    {
        
        std::string err = "打开" + xml_file_path + "失败" + strerror(errno);
        return err;
    }

    fprintf(xml_file, "<?xml version=\"1.0\" encoding=\"gb2312\" standalone=\"yes\"?>\r\n");
    fprintf(xml_file, "<root>\r\n");

    std::vector<std::string> row_content;
    row_content.reserve(col_name_2_idx.size());
    for (long row = 2; row <= row_count; row++)
    {
        //fprintf(xml_file, "  <content ");

        row_content.clear();
        //for (table_column_info::iterator it = all_col_info.begin();
        //    it != all_col_info.end(); ++it)

        for (std::map<long, std::string>::iterator it = col_idx_2_name.begin();
            it != col_idx_2_name.end(); ++it)
        {
            std::string content = it->second + "=\"" + excel.GetCellString(row, it->first).GetBuffer() + "\" ";

            if (content.find("EXCEL_STRING") != std::string::npos)
            {
                row_content.clear();
                break;
            }
            else
            {
                row_content.push_back(content);
            }
            //fprintf(xml_file, "%s=\"%s\" ", it->second.c_str(), excel.GetCellString(row, it->first).GetBuffer());

        }

        if (row_content.size())
        {
            fprintf(xml_file, "  <content ");
        }

        for (size_t i = 0; i < row_content.size(); i++)
        {
            fprintf(xml_file, "%s", row_content[i].c_str());
        }

        if (row_content.size())
        {
            fprintf(xml_file, "/>\r\n");
        }

        //fprintf(xml_file, "/>\r\n");
    }
    fprintf(xml_file, "</root>\r\n");

    fclose(xml_file);

    return "";
    }
    catch (CException* e)
    {
        char err_info[1024];
        e->GetErrorMessage(err_info, sizeof(err_info));

        return err_info;
    }
}

std::string excel_to_table(const std::string& excel_path, CTableMaker& table_server, CTableMaker& table_client)
{
    try
    {
    std::map<std::string, long> col_name_2_idx;
    std::map<std::string, std::string> data_type_to_c;

    data_type_to_c["UINT8"] = "unsigned char";
    data_type_to_c["INT8"] = "char";

    data_type_to_c["UINT16"] = "unsigned short";
    data_type_to_c["INT16"] = "short";

    data_type_to_c["UINT32"] = "unsigned int";
    data_type_to_c["INT32"] = "int";

    data_type_to_c["UINT64"] = "unsigned long long";
    data_type_to_c["INT64"] = "long long";

    CSimpleExcel excel;

    if (!excel.OpenExcelFile(excel_path.c_str()))
    {
        char err[512];
        sprintf_s(err, "Open Excel: %s fail", excel_path.c_str());
        return err;
    }

    if (!excel.LoadSheet("def", TRUE))
    {
        return "Open sheet def fail";
    }


    long row_count = excel.GetRowCount();
    long col_count = excel.GetColumnCount();

    for (long col = 1; col < col_count; col++)
    {
        CString str = excel.GetCellString(1, col);
        col_name_2_idx[str.GetBuffer()] = col;
    }

    {
        std::map<std::string, long>::iterator it_col_check;
        it_col_check = col_name_2_idx.find("字段名");
        if (it_col_check == col_name_2_idx.end())
        {
            return "没有找到 字段名 列";
        }
        it_col_check = col_name_2_idx.find("中文");
        if (it_col_check == col_name_2_idx.end())
        {
            return "没有找到 中文 列";
        }
        it_col_check = col_name_2_idx.find("数据类型");
        if (it_col_check == col_name_2_idx.end())
        {
            return "没有找到 数据类型 列";
        }

        it_col_check = col_name_2_idx.find("主键");
        if (it_col_check == col_name_2_idx.end())
        {
            return "没有找到 主键 列";
        }

        it_col_check = col_name_2_idx.find("客户端用");
        if (it_col_check == col_name_2_idx.end())
        {
            return "没有找到 客户端用 列";
        }
    }

    column_info info;
    key_info info_k;

    for (long row = 2; row <= row_count; row++)
    {
        info.m_col_name = excel.GetCellString(row, col_name_2_idx["字段名"]).GetBuffer();
        info.m_col_note = excel.GetCellString(row, col_name_2_idx["中文"]).GetBuffer();
        info.m_col_type = excel.GetCellString(row, col_name_2_idx["数据类型"]).GetBuffer();
        int gen_type = atoi(excel.GetCellString(row, col_name_2_idx["客户端用"]).GetBuffer());
        int is_key = atoi(excel.GetCellString(row, col_name_2_idx["主键"]).GetBuffer());

        if (info.m_col_name != "")
        {
            std::map<std::string, std::string>::iterator it_data_check = data_type_to_c.find(info.m_col_type);
            if (it_data_check == data_type_to_c.end())
            {
                if (info.m_col_type.find("char[") == std::string::npos)
                {
                    char err[512];
                    sprintf_s(err, "%s type %s 无效的数据类型", info.m_col_name.c_str(), info.m_col_type.c_str());
                    return err;
                }
                else
                {
                    info.m_col_type = "char*";
                    info.m_data_type = col_string;
                }
            }
            else
            {
                info.m_col_type = it_data_check->second;


                if (info.m_col_type.find("unsigned char") != std::string::npos)
                {
                    info.m_data_type = col_uint8;
                }
                else if (info.m_col_type.find("char") != std::string::npos)
                {
                    info.m_data_type = col_int8;
                }
                else if(info.m_col_type.find("unsigned short") != std::string::npos)
                {
                    info.m_data_type = col_uint16;
                }
                else if(info.m_col_type.find("short") != std::string::npos)
                {
                    info.m_data_type = col_int16;
                }
                else if(info.m_col_type.find("unsigned int") != std::string::npos)
                {
                    info.m_data_type = col_uint32;
                }
                else if(info.m_col_type.find("int") != std::string::npos)
                {
                    info.m_data_type = col_int32;
                }
                else if(info.m_col_type.find("unsigned long") != std::string::npos)
                {
                    info.m_data_type = col_uint64;
                }
                else if(info.m_col_type.find("long") != std::string::npos)
                {
                    info.m_data_type = col_int64;
                }
            }
        }

        if (is_key == 1)
        {
            info_k.m_key_list.insert(info.m_col_name);
            info_k.m_key_order.push_back(info.m_col_name);
        }
        else
        {
            if (info.m_col_name == "KeyName" &&
                info.m_col_type == "char*")
            {
                key_info info_key_name;
                info_key_name.m_key_list.insert(info.m_col_name);
                info_key_name.m_key_order.push_back(info.m_col_name);
                table_server.add_key(info_key_name);
                table_client.add_key(info_key_name);
            }
        }

        if (gen_type == 1 || gen_type == 3)
        {
            
            std::string err_info = table_server.add_column(info);
            if (err_info.length())
            {
                return err_info;
            }
        }

        if (gen_type == 2 || gen_type == 3)
        {
            std::string err_info = table_client.add_column(info);
            if (err_info.length())
            {
                return err_info;
            }
        }
    }

    if (info_k.m_key_list.size() != info_k.m_key_order.size())
    {
        return "key 字段有重复";
    }
    table_server.add_key(info_k);
    table_client.add_key(info_k);



    return "";
    }
    catch (CException* e)
    {
        char err_info[1024];
        e->GetErrorMessage(err_info, sizeof(err_info));

        return err_info;
    }
}

//void Cexcel_testDlg::OnBnClickedOk()
//{
//    // TODO: Add your control notification handler code here
//    //OnOK();
//    std::vector<std::string> table_list;
//    table_list.push_back("att");
//    table_list.push_back("att_ext");
//    table_list.push_back("global");
//    table_list.push_back("item");
//    table_list.push_back("attpool");
//    table_list.push_back("equip");
//    table_list.push_back("role");
//    table_list.push_back("hero");
//    table_list.push_back("title");
//    table_list.push_back("monster");
//    table_list.push_back("skill");
//    table_list.push_back("buff");
//    table_list.push_back("buff_group_relation");
//    table_list.push_back("npc");
//    table_list.push_back("shop_info");
//    table_list.push_back("shop");
//    table_list.push_back("suit");
//    table_list.push_back("map");
//    table_list.push_back("castle");
//    table_list.push_back("servant");
//    table_list.push_back("mall");
//    table_list.push_back("drop");
//    table_list.push_back("dig");
//    table_list.push_back("campaign");
//    table_list.push_back("vip");
//    table_list.push_back("vipshop");
//    table_list.push_back("npc_quest_binding");
//
//    for (size_t i = 0; i < table_list.size(); i++)
//    {
//        std::string name = table_list[i];
//        CTableMaker* table_server = new CTableMaker(name);
//        CTableMaker* table_client = new CTableMaker(name);
//        std::string path = "D:\\mu_table\\";
//        path += name;
//        path += ".xlsx";
//        std::string err = excel_to_table(path, *table_server, *table_client);
//
//        if (err.length())
//        {
//            MessageBox("shit", 0, 0);
//            return;
//        }
//
//        err = excel_to_xml(path, *table_server, *table_client, "./");
//
//        if (err.length())
//        {
//            MessageBox("shit3", 0, 0);
//            return;
//        }
//
//
//        if (!table_server->gen_code("./"))
//        {
//            MessageBox("shit2", 0, 0);
//            return;
//        }
//
//        delete table_server;
//        delete table_client;
//    }
//}

void Cexcel_testDlg::OnLvnItemchangedList1( NMHDR *pNMHDR, LRESULT *pResult )
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	/*
	typedef struct tagNMLISTVIEW
	{
	NMHDR   hdr;
	int     iItem;
	int     iSubItem;
	UINT    uNewState;
	UINT    uOldState;
	UINT    uChanged;
	POINT   ptAction;
	LPARAM  lParam;
	} NMLISTVIEW, *LPNMLISTVIEW;
	*/
	CString csTrace;

	if((pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(1)) /* old state : unchecked */
		&& (pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(2)) /* new state : checked */
		)
	{
		csTrace.Format("Item %d is checked\n", pNMLV->iItem);
		TRACE(csTrace);
		//SetDlgItemText(IDC_TRACE, csTrace);
	}
	else if((pNMLV->uOldState & INDEXTOSTATEIMAGEMASK(2)) /* old state : checked */
		&& (pNMLV->uNewState & INDEXTOSTATEIMAGEMASK(1)) /* new state : unchecked */ 
		)
	{
		csTrace.Format("Item %d is unchecked\n", pNMLV->iItem);
		TRACE(csTrace);
		//SetDlgItemText(IDC_TRACE, csTrace);
	}
	else // if you don't click on the check-box
	{
		TRACE("Item %d does't change the check-status\n", pNMLV->iItem);
	}


	*pResult = 0;
}

bool Cexcel_testDlg::open_excel( const char* path, const char* file )
{
    const char* ptr = strstr(file, ".xlsx");

    size_t file_len = strlen(file);
    if (ptr)
    {
        if ((ptr + strlen(".xlsx")) == (file + file_len))
        {
            m_excel_file_list.push_back(file);
        }
    }

    return true;
}


//std::string Cexcel_testDlg::_load_excel( const std::string& excel_path )
//{
//    std::map<std::string, long> col_name_2_idx;
//    std::map<std::string, std::string> data_type_to_c;
//
//
//}

void DoEvents()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Cexcel_testDlg::show_mutil_line( const std::string& msg, CXListBox::Color tc, CXListBox::Color bc )
{
    char  str[2048];
    WCHAR w_str[1024];
    int w_len = MultiByteToWideChar(CP_ACP, 0, msg.c_str(), msg.length(), w_str, 1024);

    WCHAR* ptr = w_str;
    WCHAR* end = w_str + w_len;

    while (ptr < end)
    {
        WCHAR* ptr_end = ptr + 60;

        if (ptr_end >= end)
        {
            int len = WideCharToMultiByte(CP_ACP, 0, ptr, end-ptr, str, 2048, NULL, NULL);
            str[len] = 0;
            //m_result_box.AddString(str);
            m_result_box.AddLine(tc, bc, str);
            DoEvents();
            return;
        }
        else
        {
            WCHAR tmp = *ptr_end;
            *ptr_end = 0;

            int len = WideCharToMultiByte(CP_ACP, 0, ptr, ptr_end-ptr, str, 2048, NULL, NULL);
            str[len] = 0;
            //m_result_box.AddString(str);
            m_result_box.AddLine(tc, bc, str);

            *ptr_end = tmp;
            ptr = ptr_end;
        }
    }
}



void Cexcel_testDlg::OnClose()
{
    // TODO: Add your message handler code here and/or call default

    //CDialog::OnClose();
    OnCancel();
}

void Cexcel_testDlg::OnBnClickedButtonLoad()
{
    // TODO: Add your control notification handler code here
    for (std::map<std::string, CTableMaker*>::iterator it = m_maker_client_list.begin();
        it != m_maker_client_list.end(); ++it)
    {
        delete it->second;
    }

    m_maker_client_list.clear();

    for (std::map<std::string, CTableMaker*>::iterator it = m_maker_server_list.begin();
        it != m_maker_server_list.end(); ++it)
    {
        delete it->second;
    }

    m_maker_server_list.clear();



    float total_elapse = 0;

    bool all_load_success = true;

    for (size_t i = 0; i < m_excel_file_list.size(); i++)
    {
        if (m_excel_list.GetCheck(i))
        {
            std::string excel_path = m_excel_path + m_excel_file_list[i];

            std::string table_name = m_excel_file_list[i];

            table_name = table_name.substr(0, table_name.length() - strlen(".xlsx"));

            CTableMaker* table_server = new CTableMaker(table_name);
            CTableMaker* table_client = new CTableMaker(table_name);

            UINT32 tick = ::timeGetTime();
            std::string msg = "加载";
            msg += m_excel_file_list[i];
            msg += "    ";
            //m_result_box.AddString(msg.c_str());
            m_result_box.AddLine(CXListBox::Blue, CXListBox::White, msg.c_str());
            DoEvents();
            //show_mutil_line(msg);

            std::string err = excel_to_table(excel_path, *table_server, *table_client);
            if (err.length())
            {
                msg.append("×    ");
                msg.append(err);
                m_result_box.DeleteString(m_result_box.GetCount()-1);
                m_result_box.InsertString(m_result_box.GetCount(), msg.c_str(), CXListBox::Red, CXListBox::White);
                DoEvents();
                //show_mutil_line(err);

                delete table_client;
                delete table_server;
                all_load_success = false;
            }
            else
            {
                msg.append("√    ");
                //m_result_box.InsertString(print_idx, msg.c_str());
                //char msg[1024];
                float elapse = timeGetTime() - tick;
                total_elapse += elapse;
                msg.append("耗时");
                char sztmp[256];
                sprintf_s(sztmp, "%.2f秒", elapse/1000);
                msg.append(sztmp);
                m_result_box.DeleteString(m_result_box.GetCount()-1);
                m_result_box.InsertString(m_result_box.GetCount(), msg.c_str(), CXListBox::Green, CXListBox::White);
                DoEvents();

                //sprintf_s(msg, "耗时%.2f 秒", elapse/1000);
                //m_result_box.AddString(msg);
                //show_mutil_line(std::string(msg));
                //m_maker_client_list.push_back(table_client);
                //m_maker_server_list.push_back(table_server);
                m_maker_client_list[table_name] = table_client;
                m_maker_server_list[table_name] = table_server;
            }

            //m_result_box.SetCaretIndex(m_result_box.GetCount()-1, FALSE);
        }
    }

    char szmsg[512];
    if (all_load_success)
    {
        sprintf_s(szmsg, "加载%u张表格, 总耗时%.2f秒 所有选择表格加载成功", m_maker_client_list.size(), total_elapse/1000);
        show_mutil_line(szmsg, CXListBox::Green, CXListBox::White);
    }
    else
    {
        sprintf_s(szmsg, "加载%u张表格, 总耗时%.2f秒 有部分表格加载失败请检查", m_maker_client_list.size(), total_elapse/1000);

        MessageBox(szmsg, "", MB_OK | MB_ICONERROR);

        show_mutil_line(szmsg, CXListBox::Red, CXListBox::White);
    }
    
    //show_mutil_line(szmsg, Color tc = Black, Color bc = White);
}

void Cexcel_testDlg::OnBnClickedButtonCode()
{
    // TODO: Add your control notification handler code here
    char msg[1024];
    float total_elapse = 0;
    bool all_load_success = true;
    for (std::map<std::string, CTableMaker*>::iterator it = m_maker_server_list.begin();
        it != m_maker_server_list.end(); ++it)
    {
        std::string err;
        UINT32 tick = timeGetTime();
        float elapse = 0;
        sprintf_s(msg, "生成%s代码", it->second->get_table_name().c_str());
        //m_result_box.AddString(msg);
        m_result_box.AddLine(CXListBox::Blue, CXListBox::White, msg);
        DoEvents();
        if (it->second->gen_code(m_code_path, err))
        {
            elapse = timeGetTime() - tick;
            total_elapse += elapse;
            //std::string msg = "生成" + it->second->get_table_name() + "代码 成功";
            sprintf_s(msg, "生成%s代码    √    耗时%.2f秒", it->second->get_table_name().c_str(), elapse/1000);
            m_result_box.DeleteString(m_result_box.GetCount()-1);
            m_result_box.InsertString(m_result_box.GetCount(), msg, CXListBox::Green, CXListBox::White);
            DoEvents();
        }
        else
        {
            elapse = timeGetTime() - tick;
            total_elapse += elapse;
            all_load_success = false;
            //std::string msg = "生成" + it->second->get_table_name() + "代码 失败 原因:" + err;
            sprintf_s(msg, "生成%s代码    ×    耗时%.2f秒 :%s", it->second->get_table_name().c_str(), elapse/1000, err.c_str());
            m_result_box.DeleteString(m_result_box.GetCount()-1);
            m_result_box.InsertString(m_result_box.GetCount(), msg, CXListBox::Red, CXListBox::White);
            DoEvents();
        }

        //m_result_box.SetCaretIndex(m_result_box.GetCount()-1, FALSE);
    }

    if (all_load_success)
    {
        sprintf_s(msg, "所有表格代码生成成功 耗时%.2f秒", total_elapse/1000);
        show_mutil_line(msg, CXListBox::Green, CXListBox::White);
    }
    else
    {
        sprintf_s(msg, "部分表格代码生成 请检查 耗时%.2f秒", total_elapse/1000);
        MessageBox(msg, "", MB_OK | MB_ICONERROR);
        show_mutil_line(msg, CXListBox::Red, CXListBox::White);
    }
}

void Cexcel_testDlg::OnBnClickedButtonXml()
{
    // TODO: Add your control notification handler code here
    char msg[1024];
    float total_elapse = 0;
    bool all_load_success = true;
    for (size_t i = 0; i < m_excel_file_list.size(); i++)
    {
        if (m_excel_list.GetCheck(i))
        {
            std::string excel_path = m_excel_path + m_excel_file_list[i];

            std::string table_name = m_excel_file_list[i];

            table_name = table_name.substr(0, table_name.length() - strlen(".xlsx"));

            std::map<std::string, CTableMaker*>::iterator it_svr = m_maker_server_list.find(table_name);
            std::map<std::string, CTableMaker*>::iterator it_cli = m_maker_client_list.find(table_name);

            if (it_svr == m_maker_server_list.end() ||
                it_cli == m_maker_client_list.end())
            {
                continue;
            }


            UINT32 tick = timeGetTime();
            sprintf_s(msg, "生成%s.xml", it_svr->second->get_table_name().c_str());
            //m_result_box.AddString(msg);
            m_result_box.AddLine(CXListBox::Blue, CXListBox::White, msg);
            DoEvents();
            std::string err = excel_to_xml(excel_path, *(it_svr->second), *(it_cli->second), m_xml_path);

            float elapse = timeGetTime() - tick;

            total_elapse += elapse;

            if (err.length())
            {
                all_load_success = false;
                //std::string msg = "生成" + it_svr->second->get_table_name() + ".xml 失败 原因:" + err;
                sprintf_s(msg, "生成%s.xml    ×    耗时%.2f秒 :%s", it_svr->second->get_table_name().c_str(), elapse/1000, err.c_str());
                m_result_box.DeleteString(m_result_box.GetCount()-1);
                m_result_box.InsertString(m_result_box.GetCount(), msg, CXListBox::Red, CXListBox::White);
                int count = m_result_box.GetCount();
                DoEvents();
            }
            else
            {
                //std::string msg = "生成" + it_svr->second->get_table_name() + ".xml 成功";
                CTableMaker* pp = it_svr->second;
                sprintf_s(msg, "生成%s.xml    √    耗时%.2f秒", it_svr->second->get_table_name().c_str(), elapse/1000);
                m_result_box.DeleteString(m_result_box.GetCount()-1);
                m_result_box.InsertString(m_result_box.GetCount(), msg, CXListBox::Green, CXListBox::White);
                DoEvents();
            }

            //m_result_box.SetCaretIndex(m_result_box.GetCount()-1, FALSE);
        }
    }

    if (all_load_success)
    {
        sprintf_s(msg, "所有表格xml生成成功 耗时%.2f秒", total_elapse/1000);
        show_mutil_line(msg, CXListBox::Green, CXListBox::White);
    }
    else
    {
        sprintf_s(msg, "部分表格xml生成 耗时%.2f秒 请检查", total_elapse/1000);
        MessageBox(msg, "", MB_OK|MB_ICONERROR);
        show_mutil_line(msg, CXListBox::Red, CXListBox::White);
    }
}

void Cexcel_testDlg::OnContextMenu(CWnd* pWnd, CPoint point)
{
    // TODO: Add your message handler code here
    CRect rect;
    GetDlgItem(IDC_LIST_EXCEL)->GetWindowRect(&rect);

    if (rect.PtInRect(point))
    {
        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_MENU_EXCEL));

        CMenu* popup = menu.GetSubMenu(0);

        popup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this);

        return;
    }

    GetDlgItem(IDC_LIST_RESULT)->GetWindowRect(&rect);

    if (rect.PtInRect(point))
    {
        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_MENU_RESULT));

        CMenu* popup = menu.GetSubMenu(0);

        popup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this);

        return;
    }
}

void Cexcel_testDlg::OnMenuAllCancel()
{
    // TODO: Add your command handler code here
    for (size_t i = 0; i < m_excel_file_list.size(); i++)
    {
        m_excel_list.SetCheck(i, FALSE);
    }
}

void Cexcel_testDlg::OnMenuAllSelect()
{
    // TODO: Add your command handler code here
    for (size_t i = 0; i < m_excel_file_list.size(); i++)
    {
        m_excel_list.SetCheck(i, TRUE);
    }
}

void Cexcel_testDlg::OnMenuAllReverse()
{
    // TODO: Add your command handler code here
    for (size_t i = 0; i < m_excel_file_list.size(); i++)
    {
        if (m_excel_list.GetCheck(i))
        {
            m_excel_list.SetCheck(i, FALSE);
        }
        else
        {
            m_excel_list.SetCheck(i, TRUE);
        }
    }
}

void Cexcel_testDlg::OnMenuLoad()
{
    OnBnClickedButtonLoad();
}

void Cexcel_testDlg::OnMenuClear()
{
    m_result_box.ResetContent();
}

HBRUSH Cexcel_testDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    // TODO:  Change any attributes of the DC here
    switch (pWnd->GetDlgCtrlID())
    {
    case IDC_LIST_RESULT:
        {
            OutputDebugStr("lalal");
        }
    	break;
    }

    // TODO:  Return a different brush if the default is not desired
    return hbr;
}
