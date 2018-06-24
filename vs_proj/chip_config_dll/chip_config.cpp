// fuck_chip_dll.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "chip_config_dll.h"
#include "json_node.h"
#include <memory>
#include <map>
#include <vector>
#include <wx/file.h>
#include <wx/filefn.h>

#define DEBUG 0

// pin constants
const wxString PINNUM = wxT("pinNum");
const wxString SELECT = wxT("select");
const wxString DIRECTION = wxT("Direction");
const wxString PULLUP = wxT("Pullup");
const wxString DEBOUNCE = wxT("Debounce");

// uart constants
const wxString BAUD = wxT("baud");
const wxString MODE = wxT("uart mode");
const wxString PARITY = wxT("parity");
const wxString STOP_BIT = wxT("stop bits");
const wxString UART_BIT = wxT("uart bits");

const wxString PIN_NAME_FILE = wxT("pin_name.json");

wxUint8 *json_content;
wxFile file;

#if DEBUG
wxFile log_file;
const wxString LOG_FILE = wxT("debug.log");
#endif

const wxString PIN_NUM = wxT("pin_num");
const wxString PIN_SELECT = wxT("pin_select");
const wxString PIN_FUNC = wxT("pin_func");
// ���ǵ���������һ��ʾ��
CHIP_CONFIG_DLL_API int nchip_config_dll=0;

// ���ǵ���������һ��ʾ����
CHIP_CONFIG_DLL_API int fnchip_config_dll(void)
{
    return 42;
}

// �����ѵ�����Ĺ��캯����
// �й��ඨ�����Ϣ������� fuck_chip_dll.h
Cchip_config_dll::Cchip_config_dll()
{
    return;
}

wxString thisDllDirPath()
{
    wxString thisPath = L"";
    WCHAR path[MAX_PATH];
    HMODULE hm;
    if( GetModuleHandleExW( GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            (LPWSTR) &thisDllDirPath, &hm ) )
    {
        GetModuleFileNameW( hm, path, sizeof(path) );
        thisPath = wxFileName(path).GetPath();
        //PathRemoveFileSpecW( path );
        //thisPath = CStringW( path );
        //if( !thisPath.IsEmpty() &&
        //    thisPath.GetAt( thisPath.GetLength()-1 ) != '\\' )
        //    thisPath += L"\\";
    }
    //else if( _DEBUG ) std::wcout << L"GetModuleHandle Error: " << GetLastError() << std::endl;

    //if( _DEBUG ) std::wcout << L"thisDllDirPath: [" << CStringW::PCXSTR( thisPath ) << L"]" << std::endl;
    return thisPath;
}
typedef std::shared_ptr<JSONRoot> JsonRootPtr;
typedef std::shared_ptr<wxString> wxStringPtr;
std::vector<JsonRootPtr> jsons;
std::vector<wxStringPtr> jsonStrs;
extern "C" {
    int pin_num_file_init(void)
    {
        file.Open(thisDllDirPath() + "/" + PIN_NAME_FILE, wxFile::read);
        if (!(file.IsOpened())) {
            return -1;
        }

        file.Seek(0);

        wxFileOffset filesize = file.Length();
        json_content = new wxUint8[filesize];

        if (file.Read((void *)json_content, filesize) != filesize) {
            file.Close();
            delete[] json_content;
            return -1;
        }
        return 0;
    }
    int map_pin_fun(wxString pinNum, wxString select, wxString *pin_num, wxString *pin_func)
    {

        wxString input(json_content, wxConvUTF8);
        JSONRoot jsonPinInfo(input);
        JSONElement rootPin = jsonPinInfo.toElement();
        JSONElement pin = rootPin.firstChild();
        wxString PinNum, PinSelect, PinFunc;

        while (pin.isOk()) {
            JSONElement property = pin.firstChild();
            while (property.isOk()) {
                if (property.getName() == PIN_NUM) {
                    PinNum = property.toString();
                }
                else if (property.getName() == PIN_SELECT) {
                    PinSelect = property.toString();
                }
                else if (property.getName() == PIN_FUNC) {
                    PinFunc = property.toString();
                }
                property = pin.nextChild();
            }

#if DEBUG
            wxString log = "pinNum: " + pinNum + "\n";
            log << "PinNum:" + PinNum + "\n";
#endif
            if (pinNum == PinNum) {
                if (select == PinSelect) {
#if DEBUG
                    log << "select2: " + select + "\n";
                    log << "PinSelect2:" + PinSelect + "\n";
                    log_file.Write(log);
#endif
                    *pin_num = PinFunc.BeforeFirst('_');
                    *pin_func = PinFunc;
                    return 0;
                }
            }

            pin = rootPin.nextChild();
        }
        return -1;
    }
    CHIP_CONFIG_DLL_API void OnInit()
    {
        //do nothing
    }

    CHIP_CONFIG_DLL_API void OnUninit()
    {

    }
    CHIP_CONFIG_DLL_API char * GetErrorMsg()
    {
        return "there is no error message.";
    }
    CHIP_CONFIG_DLL_API void pin_init_code(void)
    {
    }
    CHIP_CONFIG_DLL_API char * pin_init_code_gen(char* jsonStr)
    {
        pin_num_file_init();
        wxString input(jsonStr, wxConvUTF8);
        JSONRoot jsonPinInfo(input);
        JSONElement rootPin = jsonPinInfo.toElement();
        JSONElement pin = rootPin.firstChild();
        wxString pinNum, select, direct, pullup, dubounce;
        wxString pin_num, pin_func;
        wxString funcBody = "";

#if DEBUG
        int ret =log_file.Create(thisDllDirPath() + "/" + LOG_FILE, true);
        log_file.Open(thisDllDirPath() + "/" + LOG_FILE, wxFile::write);
        wxString log = "???\n!!!\n";
        log_file.Write(log);
#endif

        while (pin.isOk()) {
            JSONElement property = pin.firstChild();
            while (property.isOk()) {
                if (property.getName() == PINNUM) {
                    pinNum = property.toString();
                }
                else if (property.getName() == SELECT) {
                    select = property.toString();
                }
                else if (property.getName() == DIRECTION) {
                    direct = property.toString();
                }
                else if (property.getName() == PULLUP) {
                    pullup = property.toString();
                }
                else if (property.getName() == DEBOUNCE) {
                    dubounce = property.toString();
                }

                property = pin.nextChild();
            }
            if (map_pin_fun(pinNum, select, &pin_num, &pin_func) != 0) {
                pin = rootPin.nextChild();
                continue;
            }
             funcBody << wxT("\tdrv_pinmux_config(") + pin_num + wxT(", ") + pin_func + wxT(");");
             funcBody << "\n";
            pin = rootPin.nextChild();
        }
//        funcBody << thisDllDirPath();

        // prepare to generate the code
        JSONRoot *root = new JSONRoot(cJSON_Object);
        wxString funcName = "pinmux_init";
        root->toElement().addProperty(funcName, funcBody);
        jsons.push_back(JsonRootPtr(root));
        wxString *jstr = new wxString(root->toElement().format());
        jsonStrs.push_back(wxStringPtr(jstr));
        file.Close();
        delete[] json_content;

#if DEBUG
        /* Close Log File */
        log_file.Close();
#endif

        return (char*)jstr->mb_str(wxConvUTF8).data();
    }
}