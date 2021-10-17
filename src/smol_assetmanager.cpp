#include <smol/smol.h>
#include <smol/smol_log.h>
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

  Image* AssetManager::createCheckersImage(int width, int height, int squareCount)
  {
    // Create a procedural checker texture
    const int texWidth = width;
    const int texHeight = height;
    const int squareSize = texWidth / squareCount;
    const int sizeInBytes = texWidth * texHeight * 3;
    unsigned char *buffer = new unsigned char[sizeInBytes + sizeof(Image)];//TODO(marcio): Use our own memory manager here
    unsigned char *texData = buffer + sizeof(Image);
    unsigned char *pixel = texData;

    for (int i = 0; i < texWidth; i++)
    {
      for (int j = 0; j < texHeight; j++)
      {
        int x = i / squareSize;
        int y = j / squareSize;
        int squareNumber = x * squareCount + y;

        unsigned char color;
        bool isOdd = (squareNumber & 1);
        if (x & 1)
        {
          color = (isOdd) ? 0xAA : 0x55;
        }
        else
        {
          color = (isOdd) ? 0x55 : 0xAA;
        }

        *pixel++ = color;
        *pixel++ = color;
        *pixel++ = color;
      }
    }

    Image* image = (Image*)buffer;
    image->width = texWidth;
    image->height = texHeight;
    image->bitsPerPixel = 24;
    image->data = (char*) texData;
    return image;
  }

  Image* AssetManager::loadImageBitmap(const char* fileName)
  {
    const size_t imageHeaderSize = sizeof(Image);
    char* buffer = Platform::loadFileToBuffer(fileName, nullptr, imageHeaderSize, imageHeaderSize);

    if (buffer == nullptr)
    {
      debugLogError("Failed to load image '%s': Unable to find or read from file", fileName);
      return AssetManager::createCheckersImage(800, 600);
    }

    BitmapHeader* bitmap = (BitmapHeader*) (buffer + imageHeaderSize);
    Image* image = (Image*) buffer;

    if (bitmap->type != BITMAP_SIGNATURE)
    {
      debugLogError("Failed to load image '%s': Invalid bitmap file", fileName);
      Platform::unloadFileBuffer(buffer);
      return nullptr;
    }

    if (bitmap->compression != BITMAP_COMPRESSION_BI_BITFIELDS)
    {
      debugLogError("Failed to load image '%s': Unsuported bitmap compression", fileName);
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
      debugLogError("Failed to load image '%s': Unsuported bitmap bit count", fileName);
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
