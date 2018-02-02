#include "StdAfx.h"
#include "SimpleExcel.h"

COleVariant
covTrue((short)TRUE),
covFalse((short)FALSE),
covOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

CApplication CSimpleExcel::excel_application_;

CSimpleExcel::CSimpleExcel(void):
already_preload_(FALSE)
{
}

CSimpleExcel::~CSimpleExcel(void)
{
    CloseExcelFile();
}

//初始化EXCEL文件，
BOOL CSimpleExcel::InitExcel()
{

    CoUninitialize();
    if(CoInitialize(NULL)==S_FALSE) 
    { 
        AfxMessageBox(_T("初始化COM支持库失败！")); 
        return FALSE; 
    }
    //创建Excel 2000服务器(启动Excel) 
    if (!excel_application_.CreateDispatch(_T("Excel.Application"),NULL)) 
    { 
        AfxMessageBox(_T("创建Excel服务失败,你可能没有安装EXCEL，请检查!")); 
        return FALSE;
    }

    excel_application_.put_DisplayAlerts(FALSE); 
    return TRUE;
}

//
void CSimpleExcel::ReleaseExcel()
{
    excel_application_.Quit();
    excel_application_.ReleaseDispatch();
    excel_application_=NULL;
}

//打开excel文件
BOOL CSimpleExcel::OpenExcelFile(LPCTSTR file_name)
{
    //先关闭
    CloseExcelFile();

    //利用模板文件建立新文档 
    excel_books_.AttachDispatch(excel_application_.get_Workbooks(),true); 

    LPDISPATCH lpDis = NULL;
    lpDis = excel_books_.Add(COleVariant(file_name)); 
    if (lpDis)
    {
        excel_work_book_.AttachDispatch(lpDis); 
        //得到Worksheets 
        excel_sheets_.AttachDispatch(excel_work_book_.get_Worksheets(),true); 

        //记录打开的文件名称
        open_excel_file_ = file_name;

        return TRUE;
    }

    return FALSE;
}

//关闭打开的Excel 文件,默认情况不保存文件
void CSimpleExcel::CloseExcelFile(BOOL if_save)
{
    //如果已经打开，关闭文件
    if (open_excel_file_.IsEmpty() == FALSE)
    {
        //如果保存,交给用户控制,让用户自己存，如果自己SAVE，会出现莫名的等待
        if (if_save)
        {
            ShowInExcel(TRUE);
        }
        else
        {
            //
            excel_work_book_.Close(COleVariant(short(FALSE)),COleVariant(open_excel_file_),covOptional);
            excel_books_.Close();
        }

        //打开文件的名称清空
        open_excel_file_.Empty();
    }



    excel_sheets_.ReleaseDispatch();
    excel_work_sheet_.ReleaseDispatch();
    excel_current_range_.ReleaseDispatch();
    excel_work_book_.ReleaseDispatch();
    excel_books_.ReleaseDispatch();
}

void CSimpleExcel::ShowInExcel(BOOL bShow)
{
    excel_application_.put_Visible(bShow);
    excel_application_.put_UserControl(bShow);
}

BOOL CSimpleExcel::IsCellString( long irow, long icolumn )
{
    CRange range;
    range.AttachDispatch(excel_current_range_.get_Item (COleVariant((long)irow),COleVariant((long)icolumn)).pdispVal, true);
    COleVariant vResult =range.get_Value2();
    //VT_BSTR标示字符串
    if(vResult.vt == VT_BSTR)       
    {
        return TRUE;
    }
    return FALSE;
}

BOOL CSimpleExcel::IsCellInt( long irow, long icolumn )
{
    CRange range;
    range.AttachDispatch(excel_current_range_.get_Item (COleVariant((long)irow),COleVariant((long)icolumn)).pdispVal, true);
    COleVariant vResult =range.get_Value2();
    //好像一般都是VT_R8
    if(vResult.vt == VT_INT || vResult.vt == VT_R8)       
    {
        return TRUE;
    }
    return FALSE;
}

CString CSimpleExcel::GetCellString(long irow, long icolumn)
{

    COleVariant vResult ;
    CString str;
    //字符串
    if (already_preload_ == FALSE)
    {
        CRange range;
        range.AttachDispatch(excel_current_range_.get_Item (COleVariant((long)irow),COleVariant((long)icolumn)).pdispVal, true);
        vResult =range.get_Value2();
        range.ReleaseDispatch();
    }
    //如果数据依据预先加载了
    else
    {
        long read_address[2];
        VARIANT val;
        read_address[0] = irow;
        read_address[1] = icolumn;
        ole_safe_array_.GetElement(read_address, &val);
        vResult = val;
    }

    if(vResult.vt == VT_BSTR)
    {
        str=vResult.bstrVal;
    }
    //整数
    else if (vResult.vt==VT_INT)
    {
        str.Format(_T("%d"),vResult.pintVal);
    }
    //8字节的数字 
    else if (vResult.vt==VT_R8)     
    {
        str.Format(_T("%0.0f"),vResult.dblVal);
    }
    //时间格式
    else if(vResult.vt==VT_DATE)    
    {
        SYSTEMTIME st;
        VariantTimeToSystemTime(vResult.date, &st);
        CTime tm(st); 
        str=tm.Format("%Y-%m-%d");

    }
    //单元格空的
    else if(vResult.vt==VT_EMPTY)   
    {
        str="";
    }  

    return str;
}

double CSimpleExcel::GetCellDouble(long irow, long icolumn)
{
    double rtn_value = 0;
    COleVariant vresult;
    //字符串
    if (already_preload_ == FALSE)
    {
        CRange range;
        range.AttachDispatch(excel_current_range_.get_Item (COleVariant((long)irow),COleVariant((long)icolumn)).pdispVal, true);
        vresult =range.get_Value2();
        range.ReleaseDispatch();
    }
    //如果数据依据预先加载了
    else
    {
        long read_address[2];
        VARIANT val;
        read_address[0] = irow;
        read_address[1] = icolumn;
        ole_safe_array_.GetElement(read_address, &val);
        vresult = val;
    }

    if (vresult.vt==VT_R8)     
    {
        rtn_value = vresult.dblVal;
    }

    return rtn_value;
}

//VT_R8
int CSimpleExcel::GetCellInt(long irow, long icolumn)
{
    int num;
    COleVariant vresult;

    if (already_preload_ == FALSE)
    {
        CRange range;
        range.AttachDispatch(excel_current_range_.get_Item (COleVariant((long)irow),COleVariant((long)icolumn)).pdispVal, true);
        vresult = range.get_Value2();
        range.ReleaseDispatch();
    }
    else
    {
        long read_address[2];
        VARIANT val;
        read_address[0] = irow;
        read_address[1] = icolumn;
        ole_safe_array_.GetElement(read_address, &val);
        vresult = val;
    }
    //
    num = static_cast<int>(vresult.dblVal);

    return num;
}

//得到列的总数
long CSimpleExcel::GetColumnCount()
{
    CRange range;
    CRange usedRange;
    usedRange.AttachDispatch(excel_work_sheet_.get_UsedRange(), true);
    range.AttachDispatch(usedRange.get_Columns(), true);
    long count = range.get_Count();
    usedRange.ReleaseDispatch();
    range.ReleaseDispatch();
    return count;
}

//得到行的总数
long CSimpleExcel::GetRowCount()
{
    CRange range;
    CRange usedRange;
    usedRange.AttachDispatch(excel_work_sheet_.get_UsedRange(), true);
    range.AttachDispatch(usedRange.get_Rows(), true);
    long count = range.get_Count();
    usedRange.ReleaseDispatch();
    range.ReleaseDispatch();
    return count;
}

//按照序号加载Sheet表格,可以提前加载所有的表格内部数据
BOOL CSimpleExcel::LoadSheet(long table_index,BOOL pre_load)
{
    LPDISPATCH lpDis = NULL;
    excel_current_range_.ReleaseDispatch();
    excel_work_sheet_.ReleaseDispatch();
    lpDis = excel_sheets_.get_Item(COleVariant((long)table_index));
    if (lpDis)
    {
        excel_work_sheet_.AttachDispatch(lpDis,true);
        excel_current_range_.AttachDispatch(excel_work_sheet_.get_Cells(), true);
    }
    else
    {
        return FALSE;
    }

    already_preload_ = FALSE;
    //如果进行预先加载
    if (pre_load)
    {
        PreLoadSheet();
        already_preload_ = TRUE;
    }

    return TRUE;
}

//按照名称加载Sheet表格,可以提前加载所有的表格内部数据
BOOL CSimpleExcel::LoadSheet(LPCTSTR sheet,BOOL pre_load)
{
    LPDISPATCH lpDis = NULL;
    excel_current_range_.ReleaseDispatch();
    excel_work_sheet_.ReleaseDispatch();
    lpDis = excel_sheets_.get_Item(COleVariant(sheet));
    if (lpDis)
    {
        excel_work_sheet_.AttachDispatch(lpDis,true);
        excel_current_range_.AttachDispatch(excel_work_sheet_.get_Cells(), true);

    }
    else
    {
        return FALSE;
    }
    //
    already_preload_ = FALSE;
    //如果进行预先加载
    if (pre_load)
    {
        already_preload_ = TRUE;
        PreLoadSheet();
    }

    return TRUE;
}

int CSimpleExcel::GetSheetCount()
{
    return excel_sheets_.get_Count();
}


CString CSimpleExcel::GetSheetName(long table_index)
{
    CWorksheet sheet;
    sheet.AttachDispatch(excel_sheets_.get_Item(COleVariant((long)table_index)),true);
    CString name = sheet.get_Name();
    sheet.ReleaseDispatch();
    return name;
}

void CSimpleExcel::SaveasXSLFile(const CString &xls_file)
{
    excel_work_book_.SaveAs(COleVariant(xls_file),
        covOptional,
        covOptional,
        covOptional,
        covOptional,
        covOptional,
        0,
        covOptional,
        covOptional,
        covOptional,
        covOptional,
        covOptional);
    return;
}

//返回打开的EXCEL文件名称
CString CSimpleExcel::GetOpenFileName()
{
    return open_excel_file_;
}

//取得打开sheet的名称
CString CSimpleExcel::GetLoadSheetName()
{
    return excel_work_sheet_.get_Name();
}

void CSimpleExcel::SetCellString(long irow, long icolumn,CString new_string)
{
    COleVariant new_value(new_string);
    CRange start_range = excel_work_sheet_.get_Range(COleVariant(_T("A1")),covOptional);
    CRange write_range = start_range.get_Offset(COleVariant((long)irow -1),COleVariant((long)icolumn -1) );
    write_range.put_Value2(new_value);
    start_range.ReleaseDispatch();
    write_range.ReleaseDispatch();

}

void CSimpleExcel::SetCellInt(long irow, long icolumn,int new_int)
{
    COleVariant new_value((long)new_int);

    CRange start_range = excel_work_sheet_.get_Range(COleVariant(_T("A1")),covOptional);
    CRange write_range = start_range.get_Offset(COleVariant((long)irow -1),COleVariant((long)icolumn -1) );
    write_range.put_Value2(new_value);
    start_range.ReleaseDispatch();
    write_range.ReleaseDispatch();
}

//取得列的名称，比如27->AA
char *CSimpleExcel::GetColumnName(long icolumn)
{   
    static char column_name[64];
    size_t str_len = 0;

    while(icolumn > 0)
    {
        int num_data = icolumn % 26;
        icolumn /= 26;
        if (num_data == 0)
        {
            num_data = 26;
            icolumn--;
        }
        column_name[str_len] = (char)((num_data-1) + 'A' );
        str_len ++;
    }
    column_name[str_len] = '\0';
    //反转
    _strrev(column_name);

    return column_name;
}

//预先加载
void CSimpleExcel::PreLoadSheet()
{

    CRange used_range;

    used_range = excel_work_sheet_.get_UsedRange();	


    VARIANT ret_ary = used_range.get_Value2();
    if (!(ret_ary.vt & VT_ARRAY))
    {
        return;
    }
    //
    ole_safe_array_.Clear();
    ole_safe_array_.Attach(ret_ary); 
}