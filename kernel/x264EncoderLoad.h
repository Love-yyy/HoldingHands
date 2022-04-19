#include <Windows.h>

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef __int64            int64_t;

typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned __int64   uint64_t;

#include "x264_config.h"

#include "x264.h"

extern X264_API void (*_x264_picture_init)(x264_picture_t *pic);
extern X264_API int (*_x264_picture_alloc)(x264_picture_t *pic, int i_csp, int i_width, int i_height);
extern X264_API void (*_x264_picture_clean)(x264_picture_t *pic);

extern X264_API int (*_x264_param_default_preset)(x264_param_t *, const char *preset, const char *tune);
extern X264_API int (*_x264_param_apply_profile)(x264_param_t *, const char *profile);

extern X264_API x264_t * (*_x264_encoder_open_163)(x264_param_t *);
extern X264_API void (*_x264_encoder_close)(x264_t *);

extern X264_API int (*_x264_encoder_encode)( x264_t *, x264_nal_t **pp_nal, int *pi_nal, x264_picture_t *pic_in, x264_picture_t *pic_out );

bool x264EncoderLoad(wchar_t szPath[], wchar_t szError[]);