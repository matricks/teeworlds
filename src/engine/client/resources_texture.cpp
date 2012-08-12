#include <engine/resources_texture.h>

#include <engine/external/pnglite/pnglite.h>

unsigned int CResourceHandler_Texture::PngReadFunc(void *pOutput, unsigned long size, unsigned long numel, void *pUserPtr)
{
	unsigned char **pData = reinterpret_cast<unsigned char**>(pUserPtr);
	unsigned long TotalSize = size*numel;
	if(pOutput)
		mem_copy(pOutput, *pData, TotalSize);
	(*pData) += TotalSize;
	return TotalSize;
}

// called from the main thread
CResource *CResourceHandler_Texture::Create(IResources::CResourceId Id)
{
	return new CResource_Texture;
}

// called from job thread
bool CResourceHandler_Texture::Load(CResource *pResource, void *pData, unsigned DataSize)
{
	CResource_Texture *pTexture = static_cast<CResource_Texture*>(pResource);

	png_t Png;
	
	int Error = png_open(&Png, PngReadFunc, &pData); // ignore_convention
	if(Error != PNG_NO_ERROR)
	{
		dbg_msg("graphics", "failed to open file. name='%s'", pResource->Name());
		return false;// 0;
	}

	if(Png.depth != 8 || (Png.color_type != PNG_TRUECOLOR && Png.color_type != PNG_TRUECOLOR_ALPHA)) // ignore_convention
	{
		dbg_msg("graphics", "invalid format. filename='%s'", pResource->Name());
		return false;// 0;
	}

	unsigned char *pBuffer = (unsigned char *)mem_alloc(Png.width * Png.height * Png.bpp, 1); // ignore_convention
	png_get_data(&Png, pBuffer); // ignore_convention

	pTexture->m_ImageInfo.m_Width = Png.width; // ignore_convention
	pTexture->m_ImageInfo.m_Height = Png.height; // ignore_convention
	if(Png.color_type == PNG_TRUECOLOR) // ignore_convention
		pTexture->m_ImageInfo.m_Format = CImageInfo::FORMAT_RGB;
	else if(Png.color_type == PNG_TRUECOLOR_ALPHA) // ignore_convention
		pTexture->m_ImageInfo.m_Format = CImageInfo::FORMAT_RGBA;
	pTexture->m_ImageInfo.m_pData = pBuffer;
	return true;
}

// called from the main thread
bool CResourceHandler_Texture::Insert(CResource *pResource)
{
	CResource_Texture *pTexture = static_cast<CResource_Texture*>(pResource);
	CImageInfo *pInfo = &pTexture->m_ImageInfo;
	pTexture->m_Handle = m_pGraphics->LoadTextureRaw(pInfo->m_Width, pInfo->m_Height, pInfo->m_Format, pInfo->m_pData, pInfo->m_Format, 0);
	// free the texture data
	mem_free(pInfo->m_pData);
	pInfo->m_pData = 0;

	return true;
}

bool CResourceHandler_Texture::Destroy(CResource *pResource)
{
	CResource_Texture *pTexture = static_cast<CResource_Texture*>(pResource);
	m_pGraphics->UnloadTexture(pTexture->m_Handle);
	pTexture->m_Handle = IGraphics::CTextureHandle();
	return true;
}
