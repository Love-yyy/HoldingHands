#include "x264EncoderLoad.h"

X264_API void(*_x264_picture_init)(x264_picture_t *pic) = 0;
X264_API int(*_x264_picture_alloc)(x264_picture_t *pic, int i_csp, int i_width, int i_height) = 0;
X264_API void(*_x264_picture_clean)(x264_picture_t *pic) = 0;

X264_API int(*_x264_param_default_preset)(x264_param_t *, const char *preset, const char *tune) = 0;
X264_API int(*_x264_param_apply_profile)(x264_param_t *, const char *profile) = 0;

X264_API x264_t * (*_x264_encoder_open_163)(x264_param_t *) = 0;
X264_API void(*_x264_encoder_close)(x264_t *) = 0;

X264_API int (*_x264_encoder_encode)( x264_t *, x264_nal_t **pp_nal, int *pi_nal, x264_picture_t *pic_in, x264_picture_t *pic_out );

HMODULE hMsvcr120 = NULL;			//msvcr120.dll
HMODULE hX264_163 = NULL;			//x264-163.dll

bool x264EncoderLoad(wchar_t szPath[], wchar_t szError[])
{
	
	wchar_t szDllName[0x1000] = { 0 };
	//Load msvcr120.dll
	if (hMsvcr120 == NULL)
	{

		wcscpy(szDllName, szPath);
		wcscat(szDllName, L"\\");
		wcscat(szDllName, L"msvcr120.dll");

		hMsvcr120 = LoadLibraryW(szDllName);
		
		//Load Failed!
		if (hMsvcr120 == NULL)
		{	
			wcscpy(szError,L"Load msvcr12.dll failed!");
			return false;
		}
	}
	//load libx264.dll
	if (hX264_163 == NULL)
	{
		wcscpy(szDllName, szPath);
		wcscat(szDllName, L"\\");
		wcscat(szDllName, L"libx264-163.dll");

		hX264_163 = LoadLibraryW(szDllName);
		//Load Failed!
		if (hX264_163 == NULL)
		{	
			wcscpy(szError,L"Load libx264-163.dll failed!");
			return false;
		}
	}
	//Get Function Address;
	//x264_picture_init;
	_x264_picture_init = (X264_API void(*)(x264_picture_t *pic))GetProcAddress(hX264_163, "x264_picture_init");
	if (_x264_picture_init == NULL)
	{	
		wcscpy(szError,L"Get x264_picture_init's addr failed!");
		return false;
	}
	//x264_picture_alloc;
	_x264_picture_alloc = (int(*)(x264_picture_t *pic, int i_csp, int i_width, int i_height))GetProcAddress(hX264_163, "x264_picture_alloc");
	if (_x264_picture_alloc == NULL)
	{	
		wcscpy(szError,L"Get x264_picture_alloc's addr failed!");
		return false;
	}
	//x264_picture_clean
	_x264_picture_clean = (X264_API void(*)(x264_picture_t *pic))GetProcAddress(hX264_163, "x264_picture_clean");
	if (_x264_picture_clean == NULL)
	{	
		wcscpy(szError,L"Get x264_picture_clean's addr failed!");
		return false;
	}
	//x264_param_default_preset
	_x264_param_default_preset = (X264_API int(*)(x264_param_t *, const char *preset, const char *tune))GetProcAddress(hX264_163, "x264_param_default_preset");
	if (_x264_param_default_preset == NULL)
	{	
		wcscpy(szError,L"Get x264_param_default_preset's addr failed!");
		return false;
	}
	//x264_param_apply_profile
	_x264_param_apply_profile = (X264_API int(*)(x264_param_t *, const char *profile))GetProcAddress(hX264_163, "x264_param_apply_profile");
	if (_x264_param_apply_profile == NULL)
	{	
		wcscpy(szError,L"Get x264_param_apply_profile's addr failed!");
		return false;
	}
	//x264_encoder_open_163
	_x264_encoder_open_163 = (X264_API x264_t * (*)(x264_param_t *))GetProcAddress(hX264_163, "x264_encoder_open_163");
	if (_x264_encoder_open_163 == NULL)
	{	
		wcscpy(szError,L"Get x264_encoder_open_163's addr failed!");
		return false;
	}
	//x264_encoder_encode
	_x264_encoder_encode = (X264_API int (*)( x264_t *, x264_nal_t **pp_nal, int *pi_nal, x264_picture_t *pic_in, x264_picture_t *pic_out ))GetProcAddress(hX264_163,"x264_encoder_encode");
	if(_x264_encoder_encode == NULL)
	{
		wcscpy(szError,L"Get x264_encoder_encode's addr failed!");
		return false;
	}
	//x264_encoder_close
	_x264_encoder_close = (X264_API void(*)(x264_t *))GetProcAddress(hX264_163, "x264_encoder_close");
	if (_x264_encoder_close == NULL)
	{	
		wcscpy(szError,L"Get x264_encoder_close's addr failed!");
		return false;
	}

	return true;
}