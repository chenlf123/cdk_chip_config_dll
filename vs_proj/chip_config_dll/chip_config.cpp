// fuck_chip_dll.cpp : 定义 DLL 应用程序的导出函数。
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
const wxString PIN_NAME = wxT("pin_name");
const wxString PIN_SELECT = wxT("pin_select");
const wxString PIN_FUNC = wxT("pin_func");
// 这是导出变量的一个示例
CHIP_CONFIG_DLL_API int nchip_config_dll=0;

// 这是导出函数的一个示例。
CHIP_CONFIG_DLL_API int fnchip_config_dll(void)
{
    return 42;
}

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 fuck_chip_dll.h
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
    }

    return thisPath;
}
typedef std::shared_ptr<JSONRoot> JsonRootPtr;
typedef std::shared_ptr<wxString> wxStringPtr;
std::vector<JsonRootPtr> jsons;
std::vector<wxStringPtr> jsonStrs;
extern "C" {
    int pin_num_file_init(void)
    {
#if DEBUG
		wxString log = "";
		log_file.Write("enter pin_num_file_init\n");
#endif
        file.Open(thisDllDirPath() + "/" + PIN_NAME_FILE, wxFile::read);
        if (!(file.IsOpened())) {
#if DEBUG
			log = "error 1\n";
			log_file.Write(log);
#endif
            return -1;
        }
        file.Seek(0);

        wxFileOffset filesize = file.Length();
        json_content = new wxUint8[filesize];

        if (file.Read((void *)json_content, filesize) != filesize) {
            file.Close();
            delete[] json_content;
#if DEBUG
			log = "error 2\n";
			log_file.Write(log);
#endif
            return -1;
        }
		file.Close();

//		log_file.Write(json_content);

        return 0;
    }
    int map_pin_fun(wxString pinNum, wxString select, wxString *pin_num, wxString *pin_func)
    {
        wxString input(json_content, wxConvUTF8);
        JSONRoot jsonPinInfo(input);
        JSONElement rootPin = jsonPinInfo.toElement();
        JSONElement pin = rootPin.firstChild();
        wxString PinNum, PinName, PinSelect, PinFunc;

		while (pin.isOk()) {
            JSONElement property = pin.firstChild();
            while (property.isOk()) {
                if (property.getName() == PIN_NUM) {
                    PinNum = property.toString();
                }
                if (property.getName() == PIN_NAME) {
                    PinName = property.toString();
                }
                else if (property.getName() == PIN_SELECT) {
                    PinSelect = property.toString();
                }
                else if (property.getName() == PIN_FUNC) {
                    PinFunc = property.toString();
                }
                property = pin.nextChild();
            }

            if (pinNum == PinNum) {
                if (select == PinSelect) {
                    *pin_num = PinName;
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
#if DEBUG
        int ret =log_file.Create(thisDllDirPath() + "/" + LOG_FILE, true);

        log_file.Open(thisDllDirPath() + "/" + LOG_FILE, wxFile::write);
        wxString log = "???\n!!!\n";
        log_file.Write(log);
#endif
        pin_num_file_init();
        wxString input(jsonStr, wxConvUTF8);
        JSONRoot jsonPinInfo(input);
        JSONElement rootPin = jsonPinInfo.toElement();
        JSONElement pin = rootPin.firstChild();
        wxString pinNum, select, direct, pullup, dubounce;
        wxString pin_name, pin_func;
        wxString funcBody = "";

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
            if (map_pin_fun(pinNum, select, &pin_name, &pin_func) != 0) {
                pin = rootPin.nextChild();
                continue;
            }
             funcBody << wxT("    drv_pinmux_config(") + pin_name + wxT(", ") + pin_func + wxT(");");
             funcBody << "\n";
            pin = rootPin.nextChild();
        }

		funcBody << "\n    void GPIO_init(void);\n    GPIO_init();\n";

		funcBody << wxT("}\n\n");

		pin = rootPin.firstChild();
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
            if (map_pin_fun(pinNum, select, &pin_name, &pin_func) != 0) {
                pin = rootPin.nextChild();
                continue;
            }

			if (select.Contains("GPIO")) {
				funcBody << wxT("gpio_pin_handle_t GPIO_") + pin_name + wxT("_handler;\n");
			}

			pin = rootPin.nextChild();
        }

		funcBody << wxT("\n");
		funcBody << wxT("void GPIO_init(void)\n{");

		pin = rootPin.firstChild();
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
            if (map_pin_fun(pinNum, select, &pin_name, &pin_func) != 0) {
                pin = rootPin.nextChild();
                continue;
            }
			wxString _pullup, _direct;
			if (select.Contains("GPIO")) {
				if (pullup == "pull none") {
					_pullup = wxT("GPIO_MODE_PULLNONE");
				} else if (pullup == "open drain") {
					_pullup = wxT("GPIO_MODE_OPEN_DRAIN");
				} else if (pullup == "pulldown") {
					_pullup = wxT("GPIO_MODE_PULLDOWM");
				} else if (pullup == "pullup") {
					_pullup = wxT("GPIO_MODE_PULLUP");
				} else if (pullup == "push pull") {
					_pullup = wxT("GPIO_MODE_PUSH_PULL");
				}

				if (direct == "input") {
					_direct = wxT("GPIO_DIRECTION_INPUT");
				} else if (direct == "output") {
					_direct = wxT("GPIO_DIRECTION_OUTPUT");
				}

				funcBody << wxT("\n    GPIO_") + pin_name + wxT("_handler = csi_gpio_pin_initialize(") + pin_name + wxT(", NULL);");
				funcBody << wxT("\n    csi_gpio_pin_config(");
				funcBody << wxT("GPIO_") + pin_name + wxT("_handler, ") + _pullup + wxT(", ") + _direct + ");\n";
			}

			pin = rootPin.nextChild();
        }

//		funcBody << wxT("\n");

//        funcBody << thisDllDirPath();

        // prepare to generate the code
        JSONRoot *root = new JSONRoot(cJSON_Object);
        wxString funcName = "PIN_init";
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

	CHIP_CONFIG_DLL_API char * system_clock_code_gen(char* jsonStr)
	{
        wxString input(jsonStr, wxConvUTF8);
        JSONRoot jsonPinInfo(input);
        JSONElement rootDev = jsonPinInfo.toElement();
        JSONElement dev = rootDev.firstChild();
		wxString funcBody = "";
		wxString clock_source, freq;

		while (dev.isOk()) {
			if (dev.getName() == "Clock source") {
				if (dev.toString() == "External RC") {
				  	clock_source = "EHS_CLK";
				} else if (dev.toString() == "Internal RC") {
				    clock_source = "IHS_CLK";
				}
			} else if (dev.getName() == "Freq (MHz)") {
				freq = dev.toString();
			}
			dev = rootDev.nextChild();
		}
		funcBody << wxT("\tdrv_set_sys_freq(") + clock_source + wxT(", ") + "CLK_" + freq + "M" + wxT(");");
		funcBody << "\n";

        // prepare to generate the code
        JSONRoot *root = new JSONRoot(cJSON_Object);
        wxString funcName = "system_clock_init";
        root->toElement().addProperty(funcName, funcBody);
        jsons.push_back(JsonRootPtr(root));
        wxString *jstr = new wxString(root->toElement().format());
        jsonStrs.push_back(wxStringPtr(jstr));

        return (char*)jstr->mb_str(wxConvUTF8).data();
	}
}
