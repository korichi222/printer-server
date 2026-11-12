#include <windows.h>

#include <stdint.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
HDC pdc,dc;
int width, height;
int bw, bh;

void BlitCentered(int x,int y,int w,int h)
{
    float scaleFacter = fabs(w-bw) > fabs(h-bh) ?w/(float)bw:h/(float)bh;
    float scaledWidth = bw*scaleFacter;
    float scaledHeight = bh*scaleFacter;
    float destx = x+(w-scaledWidth)*0.5f;
    float desty = y+(h-scaledHeight)*0.5f;
    StretchBlt(pdc,(int)destx,(int)desty,(int)scaledWidth,scaledHeight,dc,0,0,bw,bh,SRCCOPY);
}

int main(int argc,char **argv)
{
    if(argc < 2)
    {
        printf("usage: %s <filename> <pages> <divs>\n",argv[0]);
        return 1;
    }
    int pageCount = 1;
    int divs = 0;
    if(argc > 2)
    {
        char *endptr;
        long num = strtol(argv[2], &endptr, 10);
        if(*endptr == '\0') {
            pageCount = num;
        }else{
            printf("usage: %s <filename> <pages> <divs>\n",argv[0]);
            return 1;
        }
    }
    if(argc > 3)
    {
        char *endptr;
        long num = strtol(argv[3], &endptr, 10);
        if(*endptr == '\0') {
            divs = num;
        }else{
            printf("usage: %s <filename> <pages> <divs>\n",argv[0]);
            return 1;
        }
    }

    DWORD Returned = 0;
    DWORD Needed = 0;
    PRINTER_INFO_1 pinfo[100];
    if(!EnumPrinters(PRINTER_ENUM_LOCAL,"",1,(LPBYTE)pinfo,sizeof(pinfo),&Needed,&Returned))
    {
        return 1;
    }
    HANDLE ph;
    OpenPrinter(pinfo[3].pName,&ph,NULL);
    int dmSize = DocumentPropertiesA(0,ph,pinfo[3].pName,NULL,NULL,0);
    PDEVMODEA dm = malloc(dmSize);
    DocumentPropertiesA(0,ph,pinfo[3].pName,dm,NULL,DM_OUT_BUFFER);
    dm->dmCopies = pageCount;
    DocumentPropertiesA(0,ph,pinfo[3].pName,dm,dm,DM_OUT_BUFFER|DM_IN_BUFFER );
    pdc = CreateDCA("WINSPOOL",pinfo[3].pName,NULL,dm);
    float PageW = (float)GetDeviceCaps(pdc, HORZRES);
    float PageH = (float)GetDeviceCaps(pdc, VERTRES);

    DOCINFO docInfo ={
          .cbSize = sizeof(DOCINFOW),
          .lpszDocName = "Test Doc",
    };
    int jobid = StartDoc(pdc,&docInfo);
    StartPage(pdc);
    dc = CreateCompatibleDC(pdc);
    int bc;
    uint8_t* imgdata = stbi_load(argv[1], &width, &height, &bc,4);
    int flip = (divs%2)^(width > height);
    if(flip)
    {
        bw  = height;
        bh = width;
    }
    else
    {
        bw = width;
        bh  = height;
    }
    BITMAPINFO bmi = {
        .bmiHeader = {
            .biSize = sizeof(BITMAPINFOHEADER),
            .biWidth  = bw,
            .biHeight = -bh,
            .biPlanes = 1,
            .biBitCount = 32,
            .biCompression = BI_RGB,
            .biSizeImage = 0,
        }
    };

    uint8_t *destBitmap;
    HBITMAP hbitmap = CreateDIBSection(dc,&bmi,DIB_RGB_COLORS,&destBitmap,0,0);
    if(flip)
    {

        for(int i = 0; i < width;i++)
        {
            for(int j = 0; j < height;j++)
            {
                int spi = i+j*width;
                int dpi = j+(width-1-i)*height;
                destBitmap[dpi*4+2] = imgdata[spi*4];
                destBitmap[dpi*4+1] = imgdata[spi*4+1];
                destBitmap[dpi*4] = imgdata[spi*4+2];
                destBitmap[dpi*4+3] = 0xff;
            }
        }
    }
    else
    {
        for(int i = 0; i < width*height;i++)
        {
            destBitmap[i*4+2] = imgdata[i*4];
            destBitmap[i*4+1] = imgdata[i*4+1];
            destBitmap[i*4] = imgdata[i*4+2];
            destBitmap[i*4+3] = 0xff;
        }
    }
    HBITMAP hbmOld = (HBITMAP)SelectObject(dc, hbitmap);
    if(divs == 0)
    {
        BlitCentered(0,0,PageW,PageH);
    }
    if(divs == 1)
    {
        BlitCentered(0,0,PageW,PageH/2);
        BlitCentered(0,PageH/2,PageW,PageH/2);
    }
    if(divs == 2)
    {
        BlitCentered(0,0,PageW/2,PageH/2);
        BlitCentered(0,PageH/2,PageW/2,PageH/2);
        BlitCentered(PageW/2,PageH/2,PageW/2,PageH/2);
        BlitCentered(PageW/2,0,PageW/2,PageH/2);
    }
    SelectObject(dc, hbmOld);
    EndPage(pdc);
    EndDoc(pdc);

}