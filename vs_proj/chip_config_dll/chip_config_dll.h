// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� FUCK_CHIP_DLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// CHIP_CONFIG_DLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef CHIP_CONFIG_DLL_EXPORTS
#define CHIP_CONFIG_DLL_API __declspec(dllexport)
#else
#define CHIP_CONFIG_DLL_API __declspec(dllimport)
#endif

// �����Ǵ� fuck_chip_dll.dll ������
class CHIP_CONFIG_DLL_API Cchip_config_dll {
public:
	Cchip_config_dll(void);
	// TODO:  �ڴ�������ķ�����
};

extern CHIP_CONFIG_DLL_API int nchip_config_dll;

CHIP_CONFIG_DLL_API int fnchip_config_dll(void);
