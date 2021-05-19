#include <smol/smol.h>
#include <smol/smol_platform.h>
#include <smol/smol_assetmanager.h>

namespace smol
{
  static const int BITMAP_SIGNATURE = 0x4D42; // 'BM'
  static const int BITMAP_COMPRESSION_BI_BITFIELDS = 3;

#pragma pack(push, 1)
  struct BitmapHeader {
    unsigned short  type;
    unsigned int    bitmapSize;
    unsigned short  reserved1;
    unsigned short  reserved2;
    unsigned int    offBits;
    unsigned int    size;
    unsigned int    width;
    unsigned int    height;
    unsigned short  planes;
    unsigned short  bitCount;
    unsigned int    compression;
    unsigned int    sizeImage;
    unsigned int    xPelsPerMeter;
    unsigned int    yPelsPerMeter;
    unsigned int    clrUsed;
    unsigned int    clrImportant;
  };
#pragma pack(pop)

  Image* AssetManager::loadImageBitmap(const char* fileName)
  {
    const size_t imageHeaderSize = sizeof(Image);
    char* buffer = Platform::loadFileToBuffer(fileName, nullptr, imageHeaderSize, imageHeaderSize);
    BitmapHeader* bitmap = (BitmapHeader*) (buffer + imageHeaderSize);
    Image* image = (Image*) buffer;

    if (bitmap->type != BITMAP_SIGNATURE)
    {
      LogError("Invalid bitmap file");
      Platform::unloadFileBuffer(buffer);
      return nullptr;
    }

    if (bitmap->compression != BITMAP_COMPRESSION_BI_BITFIELDS)
    {
      LogError("Unsuported bitmap compression");
      Platform::unloadFileBuffer(buffer);
      return nullptr;
    }

    image->width = bitmap->width;
    image->height = bitmap->height;
    image->bitsPerPixel = bitmap->bitCount;
    image->data = sizeof(BitmapHeader) + (char*) bitmap;

    if (bitmap->bitCount == 24 || bitmap->bitCount == 32)
    {
      // get color masks 
      unsigned int* maskPtr = (unsigned int*) (sizeof(BitmapHeader) + (char*)bitmap);
      unsigned int rMask = *maskPtr++;
      unsigned int gMask = *maskPtr++;
      unsigned int bMask = *maskPtr++;
      unsigned int aMask = ~(rMask | gMask | bMask);

      unsigned int rMaskShift = (rMask == 0xFF000000) ? 24 : (rMask == 0xFF0000) ? 16 : (rMask == 0xFF00) ? 8 : 0;
      unsigned int gMaskShift = (gMask == 0xFF000000) ? 24 : (gMask == 0xFF0000) ? 16 : (gMask == 0xFF00) ? 8 : 0;
      unsigned int bMaskShift = (bMask == 0xFF000000) ? 24 : (bMask == 0xFF0000) ? 16 : (bMask == 0xFF00) ? 8 : 0;
      unsigned int aMaskShift = (aMask == 0xFF000000) ? 24 : (aMask == 0xFF0000) ? 16 : (aMask == 0xFF00) ? 8 : 0;

      const int numPixels = image->width * image->height;
      const int bytesPerPixel = image->bitsPerPixel / 8;

      for(int i = 0; i < numPixels; ++i)
      {
        unsigned int* pixelPtr =(unsigned int*) (i * bytesPerPixel + image->data);
        unsigned int pixel = *pixelPtr;
        unsigned int r = (pixel & rMask) >> rMaskShift;
        unsigned int g = (pixel & gMask) >> gMaskShift;
        unsigned int b = (pixel & bMask) >> bMaskShift;
        unsigned int a = (pixel & aMask) >> aMaskShift;
        unsigned int color = a << 24 | b << 16 | g << 8 | r;
        *pixelPtr = color;
      }
    }
    else if (bitmap->bitCount != 16)
    {
      LogError("Unsuported bitmap bit count");
      unloadImage(image);
      return nullptr;
    }

    return (Image*) buffer;
  }

  void AssetManager::unloadImage(Image* image)
  {
    Platform::unloadFileBuffer((const char*)image);
  }
}
