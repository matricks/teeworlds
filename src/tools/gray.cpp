/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>
#include <base/math.h>
#include <engine/external/pnglite/pnglite.h>

typedef struct
{
	unsigned char r, g, b, a;
} CPixel;

int GrayFile(const char *pFileName)
{
	png_t Png;
	CPixel *pBuffer = 0;

	png_init(0, 0);
	png_open_file(&Png, pFileName);
	
	pBuffer = (CPixel*)mem_alloc(Png.width*Png.height*sizeof(CPixel), 1);

	png_get_data(&Png, (unsigned char *)pBuffer);
	png_close_file(&Png);

	if(Png.color_type != PNG_TRUECOLOR_ALPHA)
	{
		dbg_msg("gray", "%s: not an RGBA image", pFileName);
		return 1;
	}

	int w = Png.width;
	int h = Png.height;

	// make the texture gray scale
	for(int i = 0; i < w*h; i++)
	{
		int v = (pBuffer[i].r + pBuffer[i].g + pBuffer[i].b)/3;
		pBuffer[i].r = v;
		pBuffer[i].g = v;
		pBuffer[i].b = v;
	}

	// save here
	png_open_file_write(&Png, pFileName);
	png_set_data(&Png, w, h, 8, PNG_TRUECOLOR_ALPHA, (unsigned char *)pBuffer);
	png_close_file(&Png);

	return 0;
}

int main(int argc, const char **argv)
{
	dbg_logger_stdout();
	if(argc == 1)
	{
		dbg_msg("Usage", "%s FILE1 [ FILE2... ]", argv[0]);
		return -1;
	}

	for(int i = 1; i < argc; i++)
		GrayFile(argv[i]);
	return 0;
}
