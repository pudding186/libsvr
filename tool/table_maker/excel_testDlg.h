// excel_testDlg.h : header file
//

#pragma once
#include "SimpleExcel.h"
#include <string>
#include <vector>
#include <map>
#include "tablemaker.h"
#include "markupstl.h"
#include "XListBox.h"

// Cexcel_testDlg dialog
class Cexcel_testDlg : public CDialog
{
// Construction
public:
	Cexcel_testDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_EXCEL_TEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    CListCtrl   m_excel_list;
    CXListBox   m_result_box;
    std::string m_excel_path;
    std::string m_code_path;
    std::string m_xml_path;
    std::vector<std::string> m_excel_file_list;
    std::map<std::string, CTableMaker*>m_maker_server_list;
    std::map<std::string, CTableMaker*>m_maker_client_list;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    bool open_excel(const char* path, const char* file);
    void show_mutil_line(const std::string& msg, CXListBox::Color tc = CXListBox::Black, CXListBox::Color bc = CXListBox::White);
private:
    
    afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
public:
    afx_msg void OnBnClickedButtonLoad();
    afx_msg void OnClose();
    afx_msg void OnBnClickedButtonCode();
    afx_msg void OnBnClickedButtonXml();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnMenuAllCancel();
    afx_msg void OnMenuAllSelect();
    afx_msg void OnMenuAllReverse();
    afx_msg void OnMenuLoad();
    afx_msg void OnMenuClear();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
