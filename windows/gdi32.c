/******************************************************************************
 * @file            gdi32.c
 *
 * Released to the public domain.
 *
 * Anyone and anything may copy, edit, publish, use, compile, sell and
 * distribute this work and all its parts in any form for any purpose,
 * commercial and non-commercial, without any restrictions, without
 * complying with any conditions and by any means.
 *****************************************************************************/
#include <windows.h>
#include <pos.h>

#include "dllsyscall.h"
#define INDEX_START 25

BOOL WINAPI BitBlt (HDC hdc,
                    int x,
                    int y,
                    int cx,
                    int cy,
                    HDC hdcSrc,
                    int x1,
                    int y1,
                    DWORD rop)
{
    return (BOOL)system_call_9 (INDEX_START,
                                hdc,
                                x,
                                y,
                                cx,
                                cy,
                                hdcSrc,
                                x1,
                                y1,
                                rop);
}

HDC WINAPI CreateCompatibleDC (HDC hdc)
{
    return (HDC)system_call_1 (INDEX_START + 1, hdc);
}

HBITMAP WINAPI CreateDIBSection (HDC hdc,
                                 const BITMAPINFO *pbmi,
                                 UINT usage,
                                 VOID **ppvBits,
                                 HANDLE hSection,
                                 DWORD offset)
{
    return (HBITMAP)system_call_6 (INDEX_START + 2,
                                   hdc,
                                   pbmi,
                                   usage,
                                   ppvBits,
                                   hSection,
                                   offset);
}

HBRUSH WINAPI CreateSolidBrush (COLORREF color)
{
    return (HBRUSH)system_call_1 (INDEX_START + 3, color);
}

BOOL WINAPI DeleteDC (HDC hdc)
{
    return (BOOL)system_call_1 (INDEX_START + 4, hdc);
}

BOOL WINAPI DeleteObject (HGDIOBJ ho)
{
    return (BOOL)system_call_1 (INDEX_START + 5, ho);
}

BOOL WINAPI Rectangle (HDC hdc,
                       int left,
                       int top,
                       int right,
                       int bottom)
{
    return (BOOL)system_call_5 (INDEX_START + 6,
                                hdc,
                                left,
                                top,
                                right,
                                bottom);
}

HGDIOBJ WINAPI SelectObject (HDC hdc,
                             HGDIOBJ h)
{
    return (HGDIOBJ)system_call_2 (INDEX_START + 7,
                                   hdc,
                                   h);
}

BOOL WINAPI TextOutA (HDC hdc,
                      int x,
                      int y,
                      LPCSTR lpString,
                      int c)
{
    return (BOOL)system_call_5 (INDEX_START + 8,
                                hdc,
                                x,
                                y,
                                lpString,
                                c);
}

/* auto-genned dummy functions */
void WINAPI AbortDoc(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "AbortDoc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AbortPath(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "AbortPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AddFontMemResourceEx(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "AddFontMemResourceEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AddFontResourceA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "AddFontResourceA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AddFontResourceExA(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "AddFontResourceExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AddFontResourceExW(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "AddFontResourceExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AddFontResourceTracking(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "AddFontResourceTracking unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AddFontResourceW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "AddFontResourceW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AngleArc(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "AngleArc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AnimatePalette(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "AnimatePalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI AnyLinkedFonts(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "AnyLinkedFonts unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Arc(void)
{
    unsigned int len = 19;
    PosWriteFile(1, "Arc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ArcTo(void)
{
    unsigned int len = 21;
    PosWriteFile(1, "ArcTo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BRUSHOBJ_hGetColorTransform(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "BRUSHOBJ_hGetColorTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BRUSHOBJ_pvAllocRbrush(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "BRUSHOBJ_pvAllocRbrush unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BRUSHOBJ_pvGetRbrush(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "BRUSHOBJ_pvGetRbrush unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BRUSHOBJ_ulGetBrushColor(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "BRUSHOBJ_ulGetBrushColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BeginGdiRendering(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "BeginGdiRendering unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI BeginPath(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "BeginPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CLIPOBJ_bEnum(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "CLIPOBJ_bEnum unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CLIPOBJ_cEnumStart(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CLIPOBJ_cEnumStart unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CLIPOBJ_ppoGetPath(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CLIPOBJ_ppoGetPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CancelDC(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "CancelDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CheckColorsInGamut(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CheckColorsInGamut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ChoosePixelFormat(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "ChoosePixelFormat unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Chord(void)
{
    unsigned int len = 21;
    PosWriteFile(1, "Chord unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ClearBitmapAttributes(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "ClearBitmapAttributes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ClearBrushAttributes(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ClearBrushAttributes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CloseEnhMetaFile(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CloseEnhMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CloseFigure(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "CloseFigure unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CloseMetaFile(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "CloseMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ColorCorrectPalette(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "ColorCorrectPalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ColorMatchToTarget(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "ColorMatchToTarget unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CombineRgn(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "CombineRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CombineTransform(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CombineTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ConfigureOPMProtectedOutput(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "ConfigureOPMProtectedOutput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CopyEnhMetaFileA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CopyEnhMetaFileA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CopyEnhMetaFileW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CopyEnhMetaFileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CopyMetaFileA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "CopyMetaFileA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CopyMetaFileW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "CopyMetaFileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateBitmap(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "CreateBitmap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateBitmapFromDxSurface(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "CreateBitmapFromDxSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateBitmapFromDxSurface2(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "CreateBitmapFromDxSurface2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateBitmapIndirect(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "CreateBitmapIndirect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateBrushIndirect(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "CreateBrushIndirect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateColorSpaceA(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "CreateColorSpaceA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateColorSpaceW(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "CreateColorSpaceW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateCompatibleBitmap(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "CreateCompatibleBitmap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDCA(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CreateDCA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDCExW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "CreateDCExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDCW(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CreateDCW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDIBPatternBrush(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "CreateDIBPatternBrush unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDIBPatternBrushPt(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "CreateDIBPatternBrushPt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDIBitmap(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "CreateDIBitmap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDPIScaledDIBSection(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "CreateDPIScaledDIBSection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateDiscardableBitmap(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "CreateDiscardableBitmap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateEllipticRgn(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "CreateEllipticRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateEllipticRgnIndirect(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "CreateEllipticRgnIndirect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateEnhMetaFileA(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CreateEnhMetaFileA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateEnhMetaFileW(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CreateEnhMetaFileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateFontA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "CreateFontA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateFontIndirectA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "CreateFontIndirectA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateFontIndirectExA(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "CreateFontIndirectExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateFontIndirectExW(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "CreateFontIndirectExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateFontIndirectW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "CreateFontIndirectW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateFontW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "CreateFontW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateHalftonePalette(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "CreateHalftonePalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateHatchBrush(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CreateHatchBrush unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateICA(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CreateICA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateICW(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CreateICW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateMetaFileA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "CreateMetaFileA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateMetaFileW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "CreateMetaFileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateOPMProtectedOutput(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "CreateOPMProtectedOutput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateOPMProtectedOutputs(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "CreateOPMProtectedOutputs unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreatePalette(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "CreatePalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreatePatternBrush(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CreatePatternBrush unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreatePen(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "CreatePen unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreatePenIndirect(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "CreatePenIndirect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreatePolyPolygonRgn(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "CreatePolyPolygonRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreatePolygonRgn(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "CreatePolygonRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateRectRgn(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "CreateRectRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateRectRgnIndirect(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "CreateRectRgnIndirect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateRoundRectRgn(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "CreateRoundRectRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateScalableFontResourceA(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "CreateScalableFontResourceA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateScalableFontResourceW(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "CreateScalableFontResourceW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateScaledCompatibleBitmap(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "CreateScaledCompatibleBitmap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI CreateSessionMappedDIBSection(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "CreateSessionMappedDIBSection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTAbandonSwapChain(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTAbandonSwapChain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTAcquireKeyedMutex(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTAcquireKeyedMutex unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTAcquireKeyedMutex2(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTAcquireKeyedMutex2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTAcquireSwapChain(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTAcquireSwapChain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTAddSurfaceToSwapChain(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTAddSurfaceToSwapChain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTAdjustFullscreenGamma(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTAdjustFullscreenGamma unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCacheHybridQueryValue(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTCacheHybridQueryValue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCancelPresents(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTCancelPresents unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTChangeVideoMemoryReservation(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTChangeVideoMemoryReservation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCheckExclusiveOwnership(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "D3DKMTCheckExclusiveOwnership unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCheckMonitorPowerState(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTCheckMonitorPowerState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCheckMultiPlaneOverlaySupport(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTCheckMultiPlaneOverlaySupport unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCheckMultiPlaneOverlaySupport2(void)
{
    unsigned int len = 52;
    PosWriteFile(1, "D3DKMTCheckMultiPlaneOverlaySupport2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCheckMultiPlaneOverlaySupport3(void)
{
    unsigned int len = 52;
    PosWriteFile(1, "D3DKMTCheckMultiPlaneOverlaySupport3 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCheckOcclusion(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTCheckOcclusion unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCheckSharedResourceAccess(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "D3DKMTCheckSharedResourceAccess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCheckSinglePlaneForMultiPlaneOverlaySupport(void)
{
    unsigned int len = 65;
    PosWriteFile(1, "D3DKMTCheckSinglePlaneForMultiPlaneOverlaySupport unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCheckVidPnExclusiveOwnership(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTCheckVidPnExclusiveOwnership unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCloseAdapter(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "D3DKMTCloseAdapter unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTConfigureSharedResource(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "D3DKMTConfigureSharedResource unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTConnectDoorbell(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "D3DKMTConnectDoorbell unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateAllocation(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTCreateAllocation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateAllocation2(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTCreateAllocation2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateBundleObject(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTCreateBundleObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateContext(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTCreateContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateContextVirtual(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTCreateContextVirtual unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateDCFromMemory(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTCreateDCFromMemory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateDevice(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "D3DKMTCreateDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateDoorbell(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTCreateDoorbell unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateHwContext(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "D3DKMTCreateHwContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateHwQueue(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTCreateHwQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateKeyedMutex(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTCreateKeyedMutex unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateKeyedMutex2(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTCreateKeyedMutex2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateNativeFence(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTCreateNativeFence unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateOutputDupl(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTCreateOutputDupl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateOverlay(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTCreateOverlay unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreatePagingQueue(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTCreatePagingQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateProtectedSession(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTCreateProtectedSession unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateSwapChain(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "D3DKMTCreateSwapChain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateSynchronizationObject(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "D3DKMTCreateSynchronizationObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateSynchronizationObject2(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTCreateSynchronizationObject2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTCreateTrackedWorkload(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTCreateTrackedWorkload unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDDisplayEnum(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "D3DKMTDDisplayEnum unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyAllocation(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTDestroyAllocation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyAllocation2(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTDestroyAllocation2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyContext(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTDestroyContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyDCFromMemory(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "D3DKMTDestroyDCFromMemory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyDevice(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTDestroyDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyDoorbell(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "D3DKMTDestroyDoorbell unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyHwContext(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTDestroyHwContext unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyHwQueue(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTDestroyHwQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyKeyedMutex(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTDestroyKeyedMutex unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyOutputDupl(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTDestroyOutputDupl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyOverlay(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTDestroyOverlay unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyPagingQueue(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTDestroyPagingQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyProtectedSession(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "D3DKMTDestroyProtectedSession unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroySynchronizationObject(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTDestroySynchronizationObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDestroyTrackedWorkload(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTDestroyTrackedWorkload unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDispMgrCreate(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTDispMgrCreate unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDispMgrOperation(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTDispMgrOperation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDispMgrSourceOperation(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTDispMgrSourceOperation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDispMgrTargetOperation(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTDispMgrTargetOperation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDisplayPortOperation(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTDisplayPortOperation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTDuplicateHandle(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "D3DKMTDuplicateHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTEnumAdapters(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "D3DKMTEnumAdapters unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTEnumAdapters2(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTEnumAdapters2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTEnumAdapters3(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTEnumAdapters3 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTEnumProcesses(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTEnumProcesses unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTEscape(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "D3DKMTEscape unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTEvict(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "D3DKMTEvict unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTExtractBundleObject(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "D3DKMTExtractBundleObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTFlipOverlay(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "D3DKMTFlipOverlay unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTFlushHeapTransitions(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTFlushHeapTransitions unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTFreeGpuVirtualAddress(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTFreeGpuVirtualAddress unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetAllocationPriority(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTGetAllocationPriority unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetAvailableTrackedWorkloadIndex(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "D3DKMTGetAvailableTrackedWorkloadIndex unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetCachedHybridQueryValue(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "D3DKMTGetCachedHybridQueryValue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetContextInProcessSchedulingPriority(void)
{
    unsigned int len = 59;
    PosWriteFile(1, "D3DKMTGetContextInProcessSchedulingPriority unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetContextSchedulingPriority(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTGetContextSchedulingPriority unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetDWMVerticalBlankEvent(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTGetDWMVerticalBlankEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetDeviceState(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTGetDeviceState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetDisplayModeList(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTGetDisplayModeList unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetMemoryBudgetTarget(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTGetMemoryBudgetTarget unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetMultiPlaneOverlayCaps(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTGetMultiPlaneOverlayCaps unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetMultisampleMethodList(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTGetMultisampleMethodList unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetOverlayState(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "D3DKMTGetOverlayState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetPostCompositionCaps(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTGetPostCompositionCaps unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetPresentHistory(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTGetPresentHistory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetPresentQueueEvent(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTGetPresentQueueEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetProcessDeviceRemovalSupport(void)
{
    unsigned int len = 52;
    PosWriteFile(1, "D3DKMTGetProcessDeviceRemovalSupport unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetProcessList(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTGetProcessList unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetProcessSchedulingPriorityBand(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "D3DKMTGetProcessSchedulingPriorityBand unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetProcessSchedulingPriorityClass(void)
{
    unsigned int len = 55;
    PosWriteFile(1, "D3DKMTGetProcessSchedulingPriorityClass unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetProperties(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTGetProperties unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetResourcePresentPrivateDriverData(void)
{
    unsigned int len = 57;
    PosWriteFile(1, "D3DKMTGetResourcePresentPrivateDriverData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetRuntimeData(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTGetRuntimeData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetScanLine(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "D3DKMTGetScanLine unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetSetSwapChainMetadata(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "D3DKMTGetSetSwapChainMetadata unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetSharedPrimaryHandle(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTGetSharedPrimaryHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetSharedResourceAdapterLuid(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTGetSharedResourceAdapterLuid unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetSwapChainSurfacePhysicalAddress(void)
{
    unsigned int len = 56;
    PosWriteFile(1, "D3DKMTGetSwapChainSurfacePhysicalAddress unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetTrackedWorkloadStatistics(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTGetTrackedWorkloadStatistics unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTGetYieldPercentage(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTGetYieldPercentage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTInvalidateActiveVidPn(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTInvalidateActiveVidPn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTInvalidateCache(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "D3DKMTInvalidateCache unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTLock(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "D3DKMTLock unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTLock2(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "D3DKMTLock2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTMakeResident(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "D3DKMTMakeResident unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTMapGpuVirtualAddress(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTMapGpuVirtualAddress unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTMarkDeviceAsError(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTMarkDeviceAsError unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTNetDispGetNextChunkInfo(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "D3DKMTNetDispGetNextChunkInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTNetDispQueryMiracastDisplayDeviceStatus(void)
{
    unsigned int len = 61;
    PosWriteFile(1, "D3DKMTNetDispQueryMiracastDisplayDeviceStatus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTNetDispQueryMiracastDisplayDeviceSupport(void)
{
    unsigned int len = 62;
    PosWriteFile(1, "D3DKMTNetDispQueryMiracastDisplayDeviceSupport unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTNetDispStartMiracastDisplayDevice(void)
{
    unsigned int len = 55;
    PosWriteFile(1, "D3DKMTNetDispStartMiracastDisplayDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTNetDispStartMiracastDisplayDevice2(void)
{
    unsigned int len = 56;
    PosWriteFile(1, "D3DKMTNetDispStartMiracastDisplayDevice2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTNetDispStartMiracastDisplayDeviceEx(void)
{
    unsigned int len = 57;
    PosWriteFile(1, "D3DKMTNetDispStartMiracastDisplayDeviceEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTNetDispStopMiracastDisplayDevice(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "D3DKMTNetDispStopMiracastDisplayDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTNotifyWorkSubmission(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTNotifyWorkSubmission unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOfferAllocations(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTOfferAllocations unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenAdapterFromDeviceName(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "D3DKMTOpenAdapterFromDeviceName unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenAdapterFromGdiDisplayName(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTOpenAdapterFromGdiDisplayName unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenAdapterFromHdc(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTOpenAdapterFromHdc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenAdapterFromLuid(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "D3DKMTOpenAdapterFromLuid unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenBundleObjectNtHandleFromName(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "D3DKMTOpenBundleObjectNtHandleFromName unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenKeyedMutex(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTOpenKeyedMutex unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenKeyedMutex2(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "D3DKMTOpenKeyedMutex2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenKeyedMutexFromNtHandle(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "D3DKMTOpenKeyedMutexFromNtHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenNativeFenceFromNtHandle(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "D3DKMTOpenNativeFenceFromNtHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenNtHandleFromName(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTOpenNtHandleFromName unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenProtectedSessionFromNtHandle(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "D3DKMTOpenProtectedSessionFromNtHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenResource(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "D3DKMTOpenResource unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenResource2(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTOpenResource2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenResourceFromNtHandle(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTOpenResourceFromNtHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenSwapChain(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTOpenSwapChain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenSyncObjectFromNtHandle(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "D3DKMTOpenSyncObjectFromNtHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenSyncObjectFromNtHandle2(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "D3DKMTOpenSyncObjectFromNtHandle2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenSyncObjectNtHandleFromName(void)
{
    unsigned int len = 52;
    PosWriteFile(1, "D3DKMTOpenSyncObjectNtHandleFromName unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOpenSynchronizationObject(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "D3DKMTOpenSynchronizationObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOutputDuplGetFrameInfo(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTOutputDuplGetFrameInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOutputDuplGetMetaData(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTOutputDuplGetMetaData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOutputDuplGetPointerShapeData(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTOutputDuplGetPointerShapeData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOutputDuplPresent(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTOutputDuplPresent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOutputDuplPresentToHwQueue(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "D3DKMTOutputDuplPresentToHwQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTOutputDuplReleaseFrame(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTOutputDuplReleaseFrame unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTPinDirectFlipResources(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTPinDirectFlipResources unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTPinResources(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "D3DKMTPinResources unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTPollDisplayChildren(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "D3DKMTPollDisplayChildren unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTPresent(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "D3DKMTPresent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTPresentMultiPlaneOverlay(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTPresentMultiPlaneOverlay unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTPresentMultiPlaneOverlay2(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "D3DKMTPresentMultiPlaneOverlay2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTPresentMultiPlaneOverlay3(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "D3DKMTPresentMultiPlaneOverlay3 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTPresentRedirected(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTPresentRedirected unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryAdapterInfo(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTQueryAdapterInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryAllocationResidency(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTQueryAllocationResidency unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryClockCalibration(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTQueryClockCalibration unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryFSEBlock(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTQueryFSEBlock unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryProcessOfferInfo(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTQueryProcessOfferInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryProtectedSessionInfoFromNtHandle(void)
{
    unsigned int len = 59;
    PosWriteFile(1, "D3DKMTQueryProtectedSessionInfoFromNtHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryProtectedSessionStatus(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "D3DKMTQueryProtectedSessionStatus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryRemoteVidPnSourceFromGdiDisplayName(void)
{
    unsigned int len = 62;
    PosWriteFile(1, "D3DKMTQueryRemoteVidPnSourceFromGdiDisplayName unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryResourceInfo(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTQueryResourceInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryResourceInfoFromNtHandle(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTQueryResourceInfoFromNtHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryStatistics(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "D3DKMTQueryStatistics unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryVidPnExclusiveOwnership(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTQueryVidPnExclusiveOwnership unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTQueryVideoMemoryInfo(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTQueryVideoMemoryInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTReclaimAllocations(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTReclaimAllocations unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTReclaimAllocations2(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "D3DKMTReclaimAllocations2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTRegisterTrimNotification(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTRegisterTrimNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTRegisterVailProcess(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "D3DKMTRegisterVailProcess unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTReleaseKeyedMutex(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "D3DKMTReleaseKeyedMutex unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTReleaseKeyedMutex2(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTReleaseKeyedMutex2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTReleaseProcessVidPnSourceOwners(void)
{
    unsigned int len = 53;
    PosWriteFile(1, "D3DKMTReleaseProcessVidPnSourceOwners unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTReleaseSwapChain(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTReleaseSwapChain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTRemoveSurfaceFromSwapChain(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "D3DKMTRemoveSurfaceFromSwapChain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTRender(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "D3DKMTRender unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTReserveGpuVirtualAddress(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTReserveGpuVirtualAddress unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTResetTrackedWorkloadStatistics(void)
{
    unsigned int len = 52;
    PosWriteFile(1, "D3DKMTResetTrackedWorkloadStatistics unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetAllocationPriority(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTSetAllocationPriority unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetContextInProcessSchedulingPriority(void)
{
    unsigned int len = 59;
    PosWriteFile(1, "D3DKMTSetContextInProcessSchedulingPriority unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetContextSchedulingPriority(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTSetContextSchedulingPriority unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetDisplayMode(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTSetDisplayMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetDisplayPrivateDriverFormat(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTSetDisplayPrivateDriverFormat unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetDodIndirectSwapchain(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "D3DKMTSetDodIndirectSwapchain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetFSEBlock(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "D3DKMTSetFSEBlock unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetGammaRamp(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "D3DKMTSetGammaRamp unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetHwProtectionTeardownRecovery(void)
{
    unsigned int len = 53;
    PosWriteFile(1, "D3DKMTSetHwProtectionTeardownRecovery unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetMemoryBudgetTarget(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTSetMemoryBudgetTarget unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetMonitorColorSpaceTransform(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTSetMonitorColorSpaceTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetProcessDeviceRemovalSupport(void)
{
    unsigned int len = 52;
    PosWriteFile(1, "D3DKMTSetProcessDeviceRemovalSupport unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetProcessSchedulingPriorityBand(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "D3DKMTSetProcessSchedulingPriorityBand unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetProcessSchedulingPriorityClass(void)
{
    unsigned int len = 55;
    PosWriteFile(1, "D3DKMTSetProcessSchedulingPriorityClass unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetProperties(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTSetProperties unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetQueuedLimit(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTSetQueuedLimit unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetStablePowerState(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "D3DKMTSetStablePowerState unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetStereoEnabled(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "D3DKMTSetStereoEnabled unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetSyncRefreshCountWaitTarget(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTSetSyncRefreshCountWaitTarget unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetVidPnSourceHwProtection(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "D3DKMTSetVidPnSourceHwProtection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetVidPnSourceOwner(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "D3DKMTSetVidPnSourceOwner unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetVidPnSourceOwner1(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTSetVidPnSourceOwner1 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetVidPnSourceOwner2(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "D3DKMTSetVidPnSourceOwner2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSetYieldPercentage(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "D3DKMTSetYieldPercentage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTShareObjects(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "D3DKMTShareObjects unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSharedPrimaryLockNotification(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTSharedPrimaryLockNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSharedPrimaryUnLockNotification(void)
{
    unsigned int len = 53;
    PosWriteFile(1, "D3DKMTSharedPrimaryUnLockNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSignalSynchronizationObject(void)
{
    unsigned int len = 49;
    PosWriteFile(1, "D3DKMTSignalSynchronizationObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSignalSynchronizationObject2(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTSignalSynchronizationObject2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSignalSynchronizationObjectFromCpu(void)
{
    unsigned int len = 56;
    PosWriteFile(1, "D3DKMTSignalSynchronizationObjectFromCpu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSignalSynchronizationObjectFromGpu(void)
{
    unsigned int len = 56;
    PosWriteFile(1, "D3DKMTSignalSynchronizationObjectFromGpu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSignalSynchronizationObjectFromGpu2(void)
{
    unsigned int len = 57;
    PosWriteFile(1, "D3DKMTSignalSynchronizationObjectFromGpu2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSubmitCommand(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTSubmitCommand unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSubmitCommandToHwQueue(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTSubmitCommandToHwQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSubmitPresentBltToHwQueue(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "D3DKMTSubmitPresentBltToHwQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSubmitPresentToHwQueue(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "D3DKMTSubmitPresentToHwQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSubmitSignalSyncObjectsToHwQueue(void)
{
    unsigned int len = 54;
    PosWriteFile(1, "D3DKMTSubmitSignalSyncObjectsToHwQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTSubmitWaitForSyncObjectsToHwQueue(void)
{
    unsigned int len = 55;
    PosWriteFile(1, "D3DKMTSubmitWaitForSyncObjectsToHwQueue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTTrimProcessCommitment(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTTrimProcessCommitment unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUnOrderedPresentSwapChain(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "D3DKMTUnOrderedPresentSwapChain unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUnlock(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "D3DKMTUnlock unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUnlock2(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "D3DKMTUnlock2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUnpinDirectFlipResources(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTUnpinDirectFlipResources unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUnpinResources(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTUnpinResources unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUnregisterTrimNotification(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "D3DKMTUnregisterTrimNotification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUpdateAllocationProperty(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "D3DKMTUpdateAllocationProperty unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUpdateGpuVirtualAddress(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "D3DKMTUpdateGpuVirtualAddress unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUpdateOverlay(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "D3DKMTUpdateOverlay unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTUpdateTrackedWorkload(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "D3DKMTUpdateTrackedWorkload unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTVailConnect(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "D3DKMTVailConnect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTVailDisconnect(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "D3DKMTVailDisconnect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTVailPromoteCompositionSurface(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTVailPromoteCompositionSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTWaitForIdle(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "D3DKMTWaitForIdle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTWaitForSynchronizationObject(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "D3DKMTWaitForSynchronizationObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTWaitForSynchronizationObject2(void)
{
    unsigned int len = 51;
    PosWriteFile(1, "D3DKMTWaitForSynchronizationObject2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTWaitForSynchronizationObjectFromCpu(void)
{
    unsigned int len = 57;
    PosWriteFile(1, "D3DKMTWaitForSynchronizationObjectFromCpu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTWaitForSynchronizationObjectFromGpu(void)
{
    unsigned int len = 57;
    PosWriteFile(1, "D3DKMTWaitForSynchronizationObjectFromGpu unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTWaitForVerticalBlankEvent(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "D3DKMTWaitForVerticalBlankEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI D3DKMTWaitForVerticalBlankEvent2(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "D3DKMTWaitForVerticalBlankEvent2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DDCCIGetCapabilitiesString(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "DDCCIGetCapabilitiesString unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DDCCIGetCapabilitiesStringLength(void)
{
    unsigned int len = 48;
    PosWriteFile(1, "DDCCIGetCapabilitiesStringLength unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DDCCIGetTimingReport(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "DDCCIGetTimingReport unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DDCCIGetVCPFeature(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "DDCCIGetVCPFeature unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DDCCISaveCurrentSettings(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "DDCCISaveCurrentSettings unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DDCCISetVCPFeature(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "DDCCISetVCPFeature unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DPtoLP(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "DPtoLP unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdCreateFullscreenSprite(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "DdCreateFullscreenSprite unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdDestroyFullscreenSprite(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "DdDestroyFullscreenSprite unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry0(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry0 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry1(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry1 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry10(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry10 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry11(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry11 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry12(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry12 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry13(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry13 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry14(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry14 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry15(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry15 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry16(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry16 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry17(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry17 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry18(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry18 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry19(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry19 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry2(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry20(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry20 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry21(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry21 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry22(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry22 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry23(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry23 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry24(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry24 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry25(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry25 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry26(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry26 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry27(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry27 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry28(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry28 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry29(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry29 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry3(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry3 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry30(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry30 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry31(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry31 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry32(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry32 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry33(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry33 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry34(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry34 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry35(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry35 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry36(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry36 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry37(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry37 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry38(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry38 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry39(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry39 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry4(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry4 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry40(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry40 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry41(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry41 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry42(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry42 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry43(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry43 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry44(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry44 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry45(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry45 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry46(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry46 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry47(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry47 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry48(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry48 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry49(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry49 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry5(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry5 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry50(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry50 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry51(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry51 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry52(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry52 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry53(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry53 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry54(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry54 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry55(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry55 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry56(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "DdEntry56 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry6(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry6 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry7(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry7 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry8(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry8 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdEntry9(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "DdEntry9 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdNotifyFullscreenSpriteUpdate(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "DdNotifyFullscreenSpriteUpdate unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DdQueryVisRgnUniqueness(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "DdQueryVisRgnUniqueness unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeleteColorSpace(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "DeleteColorSpace unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeleteEnhMetaFile(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "DeleteEnhMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeleteMetaFile(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "DeleteMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DescribePixelFormat(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "DescribePixelFormat unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroyOPMProtectedOutput(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "DestroyOPMProtectedOutput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DestroyPhysicalMonitorInternal(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "DestroyPhysicalMonitorInternal unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeviceCapabilitiesExA(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "DeviceCapabilitiesExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DeviceCapabilitiesExW(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "DeviceCapabilitiesExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DrawEscape(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "DrawEscape unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI DwmCreatedBitmapRemotingOutput(void)
{
    unsigned int len = 46;
    PosWriteFile(1, "DwmCreatedBitmapRemotingOutput unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Ellipse(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "Ellipse unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnableEUDC(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "EnableEUDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EndDoc(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "EndDoc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EndFormPage(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "EndFormPage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EndGdiRendering(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "EndGdiRendering unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EndPage(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "EndPage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EndPath(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "EndPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngAcquireSemaphore(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EngAcquireSemaphore unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngAlphaBlend(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EngAlphaBlend unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngAssociateSurface(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EngAssociateSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngBitBlt(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "EngBitBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngCheckAbort(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EngCheckAbort unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngComputeGlyphSet(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "EngComputeGlyphSet unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngCopyBits(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "EngCopyBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngCreateBitmap(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "EngCreateBitmap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngCreateClip(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EngCreateClip unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngCreateDeviceBitmap(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "EngCreateDeviceBitmap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngCreateDeviceSurface(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "EngCreateDeviceSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngCreatePalette(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "EngCreatePalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngCreateSemaphore(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "EngCreateSemaphore unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngDeleteClip(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EngDeleteClip unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngDeletePalette(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "EngDeletePalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngDeletePath(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EngDeletePath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngDeleteSemaphore(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "EngDeleteSemaphore unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngDeleteSurface(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "EngDeleteSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngEraseSurface(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "EngEraseSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngFillPath(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "EngFillPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngFindResource(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "EngFindResource unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngFreeModule(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EngFreeModule unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngGetCurrentCodePage(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "EngGetCurrentCodePage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngGetDriverName(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "EngGetDriverName unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngGetPrinterDataFileName(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "EngGetPrinterDataFileName unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngGradientFill(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "EngGradientFill unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngLineTo(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "EngLineTo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngLoadModule(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EngLoadModule unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngLockSurface(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "EngLockSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngMarkBandingSurface(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "EngMarkBandingSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngMultiByteToUnicodeN(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "EngMultiByteToUnicodeN unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngMultiByteToWideChar(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "EngMultiByteToWideChar unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngPaint(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "EngPaint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngPlgBlt(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "EngPlgBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngQueryEMFInfo(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "EngQueryEMFInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngQueryLocalTime(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "EngQueryLocalTime unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngReleaseSemaphore(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EngReleaseSemaphore unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngStretchBlt(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EngStretchBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngStretchBltROP(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "EngStretchBltROP unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngStrokeAndFillPath(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "EngStrokeAndFillPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngStrokePath(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EngStrokePath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngTextOut(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "EngTextOut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngTransparentBlt(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "EngTransparentBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngUnicodeToMultiByteN(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "EngUnicodeToMultiByteN unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngUnlockSurface(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "EngUnlockSurface unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EngWideCharToMultiByte(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "EngWideCharToMultiByte unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumEnhMetaFile(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "EnumEnhMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumFontFamiliesA(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "EnumFontFamiliesA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumFontFamiliesExA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EnumFontFamiliesExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumFontFamiliesExW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "EnumFontFamiliesExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumFontFamiliesW(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "EnumFontFamiliesW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumFontsA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "EnumFontsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumFontsW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "EnumFontsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumICMProfilesA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "EnumICMProfilesA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumICMProfilesW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "EnumICMProfilesW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumMetaFile(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "EnumMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EnumObjects(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "EnumObjects unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EqualRgn(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "EqualRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Escape(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "Escape unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EudcLoadLinkW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "EudcLoadLinkW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI EudcUnloadLinkW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "EudcUnloadLinkW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExcludeClipRect(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "ExcludeClipRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExtCreatePen(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "ExtCreatePen unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExtCreateRegion(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "ExtCreateRegion unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExtEscape(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "ExtEscape unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExtFloodFill(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "ExtFloodFill unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExtSelectClipRgn(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "ExtSelectClipRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExtTextOutA(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ExtTextOutA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ExtTextOutW(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ExtTextOutW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FONTOBJ_cGetAllGlyphHandles(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "FONTOBJ_cGetAllGlyphHandles unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FONTOBJ_cGetGlyphs(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "FONTOBJ_cGetGlyphs unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FONTOBJ_pQueryGlyphAttrs(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "FONTOBJ_pQueryGlyphAttrs unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FONTOBJ_pfdg(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "FONTOBJ_pfdg unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FONTOBJ_pifi(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "FONTOBJ_pifi unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FONTOBJ_pvTrueTypeFontFile(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "FONTOBJ_pvTrueTypeFontFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FONTOBJ_pxoGetXform(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "FONTOBJ_pxoGetXform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FONTOBJ_vGetInfo(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "FONTOBJ_vGetInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FillPath(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "FillPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FillRgn(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "FillRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FixBrushOrgEx(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "FixBrushOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FlattenPath(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "FlattenPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FloodFill(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "FloodFill unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FontIsLinked(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "FontIsLinked unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI FrameRgn(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "FrameRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Gdi32DllInitialize(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "Gdi32DllInitialize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiAddFontResourceW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GdiAddFontResourceW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiAddGlsBounds(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiAddGlsBounds unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiAddGlsRecord(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiAddGlsRecord unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiAddInitialFonts(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GdiAddInitialFonts unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiAlphaBlend(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiAlphaBlend unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiArtificialDecrementDriver(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "GdiArtificialDecrementDriver unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiBatchLimit(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiBatchLimit unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiCleanCacheDC(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiCleanCacheDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiComment(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GdiComment unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConsoleTextOut(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GdiConsoleTextOut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertAndCheckDC(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GdiConvertAndCheckDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertBitmap(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiConvertBitmap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertBitmapV5(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GdiConvertBitmapV5 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertBrush(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiConvertBrush unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertDC(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GdiConvertDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertEnhMetaFile(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GdiConvertEnhMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertFont(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GdiConvertFont unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertMetaFilePict(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GdiConvertMetaFilePict unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertPalette(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GdiConvertPalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertRegion(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiConvertRegion unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiConvertToDevmodeW(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GdiConvertToDevmodeW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiCreateLocalEnhMetaFile(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "GdiCreateLocalEnhMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiCreateLocalMetaFilePict(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GdiCreateLocalMetaFilePict unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiCurrentProcessSplWow64(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "GdiCurrentProcessSplWow64 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiDeleteLocalDC(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiDeleteLocalDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiDeleteSpoolFileHandle(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GdiDeleteSpoolFileHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiDescribePixelFormat(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GdiDescribePixelFormat unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiDisableUMPDSandboxing(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GdiDisableUMPDSandboxing unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiDllInitialize(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiDllInitialize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiDrawStream(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiDrawStream unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEndDocEMF(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GdiEndDocEMF unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEndPageEMF(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiEndPageEMF unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry1(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GdiEntry1 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry10(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GdiEntry10 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry11(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GdiEntry11 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry12(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GdiEntry12 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry13(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GdiEntry13 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry14(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GdiEntry14 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry15(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GdiEntry15 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry16(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GdiEntry16 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry2(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GdiEntry2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry3(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GdiEntry3 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry4(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GdiEntry4 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry5(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GdiEntry5 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry6(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GdiEntry6 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry7(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GdiEntry7 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry8(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GdiEntry8 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiEntry9(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GdiEntry9 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiFixUpHandle(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GdiFixUpHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiFlush(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "GdiFlush unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiFullscreenControl(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GdiFullscreenControl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetBatchLimit(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiGetBatchLimit unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetBitmapBitsSize(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GdiGetBitmapBitsSize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetCharDimensions(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GdiGetCharDimensions unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetCodePage(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GdiGetCodePage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetDC(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "GdiGetDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetDevmodeForPage(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GdiGetDevmodeForPage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetEntry(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GdiGetEntry unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetLocalBrush(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiGetLocalBrush unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetLocalDC(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiGetLocalDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetLocalFont(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiGetLocalFont unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetPageCount(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiGetPageCount unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetPageHandle(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiGetPageHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetSpoolFileHandle(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GdiGetSpoolFileHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetSpoolMessage(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GdiGetSpoolMessage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGetVariationStoreDelta(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "GdiGetVariationStoreDelta unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiGradientFill(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiGradientFill unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiHandleBeingTracked(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GdiHandleBeingTracked unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiInitSpool(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GdiInitSpool unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiInitializeLanguagePack(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "GdiInitializeLanguagePack unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiIsMetaFileDC(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiIsMetaFileDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiIsMetaPrintDC(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiIsMetaPrintDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiIsPlayMetafileDC(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GdiIsPlayMetafileDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiIsScreenDC(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiIsScreenDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiIsUMPDSandboxingEnabled(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GdiIsUMPDSandboxingEnabled unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiLoadType1Fonts(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GdiLoadType1Fonts unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiPlayDCScript(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiPlayDCScript unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiPlayEMF(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GdiPlayEMF unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiPlayJournal(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GdiPlayJournal unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiPlayPageEMF(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GdiPlayPageEMF unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiPlayPrivatePageEMF(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GdiPlayPrivatePageEMF unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiPlayScript(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiPlayScript unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiPrinterThunk(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiPrinterThunk unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiProcessSetup(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiProcessSetup unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiQueryFonts(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiQueryFonts unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiQueryTable(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiQueryTable unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiRealizationInfo(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GdiRealizationInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiReleaseDC(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GdiReleaseDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiReleaseLocalDC(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GdiReleaseLocalDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiResetDCEMF(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GdiResetDCEMF unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiSetAttrs(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GdiSetAttrs unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiSetBatchLimit(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiSetBatchLimit unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiSetLastError(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiSetLastError unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiSetPixelFormat(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GdiSetPixelFormat unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiSetServerAttr(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GdiSetServerAttr unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiStartDocEMF(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GdiStartDocEMF unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiStartPageEMF(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiStartPageEMF unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiSupportsFontChangeEvent(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GdiSupportsFontChangeEvent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiSwapBuffers(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GdiSwapBuffers unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiTrackHCreate(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiTrackHCreate unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiTrackHDelete(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GdiTrackHDelete unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiTransparentBlt(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GdiTransparentBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiValidateHandle(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GdiValidateHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GdiWaitForTextReady(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GdiWaitForTextReady unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GditGetCallerTLStorage(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GditGetCallerTLStorage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GditPopCallerInfo(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GditPopCallerInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GditPushCallerInfo(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GditPushCallerInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetArcDirection(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetArcDirection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetAspectRatioFilterEx(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetAspectRatioFilterEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBitmapAttributes(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetBitmapAttributes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBitmapBits(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetBitmapBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBitmapDimensionEx(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetBitmapDimensionEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBitmapDpiScaleValue(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetBitmapDpiScaleValue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBkColor(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetBkColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBkMode(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GetBkMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBoundsRect(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetBoundsRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBrushAttributes(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetBrushAttributes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetBrushOrgEx(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetBrushOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCOPPCompatibleOPMInformation(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "GetCOPPCompatibleOPMInformation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCertificate(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetCertificate unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCertificateByHandle(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetCertificateByHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCertificateSize(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetCertificateSize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCertificateSizeByHandle(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetCertificateSizeByHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharABCWidthsA(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetCharABCWidthsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharABCWidthsFloatA(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetCharABCWidthsFloatA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharABCWidthsFloatI(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetCharABCWidthsFloatI unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharABCWidthsFloatW(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetCharABCWidthsFloatW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharABCWidthsI(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetCharABCWidthsI unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharABCWidthsW(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetCharABCWidthsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharWidth32A(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetCharWidth32A unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharWidth32W(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetCharWidth32W unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharWidthA(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetCharWidthA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharWidthFloatA(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetCharWidthFloatA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharWidthFloatW(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetCharWidthFloatW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharWidthI(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetCharWidthI unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharWidthInfo(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetCharWidthInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharWidthW(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetCharWidthW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharacterPlacementA(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetCharacterPlacementA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCharacterPlacementW(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetCharacterPlacementW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipBox(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetClipBox unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetClipRgn(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetClipRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetColorAdjustment(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetColorAdjustment unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetColorSpace(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetColorSpace unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCurrentDpiInfo(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetCurrentDpiInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCurrentObject(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetCurrentObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetCurrentPositionEx(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetCurrentPositionEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDCBrushColor(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetDCBrushColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDCDpiScaleValue(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetDCDpiScaleValue unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDCOrgEx(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetDCOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDCPenColor(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetDCPenColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDIBColorTable(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetDIBColorTable unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDIBits(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GetDIBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDeviceCaps(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetDeviceCaps unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetDeviceGammaRamp(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetDeviceGammaRamp unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetETM(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "GetETM unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEUDCTimeStamp(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetEUDCTimeStamp unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEUDCTimeStampExW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetEUDCTimeStampExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnhMetaFileA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetEnhMetaFileA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnhMetaFileBits(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetEnhMetaFileBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnhMetaFileDescriptionA(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetEnhMetaFileDescriptionA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnhMetaFileDescriptionW(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "GetEnhMetaFileDescriptionW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnhMetaFileHeader(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetEnhMetaFileHeader unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnhMetaFilePaletteEntries(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "GetEnhMetaFilePaletteEntries unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnhMetaFilePixelFormat(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "GetEnhMetaFilePixelFormat unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetEnhMetaFileW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetEnhMetaFileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFontAssocStatus(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetFontAssocStatus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFontData(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "GetFontData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFontFileData(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetFontFileData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFontFileInfo(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetFontFileInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFontLanguageInfo(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetFontLanguageInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFontRealizationInfo(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetFontRealizationInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFontResourceInfoW(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetFontResourceInfoW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetFontUnicodeRanges(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "GetFontUnicodeRanges unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGlyphIndicesA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetGlyphIndicesA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGlyphIndicesW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetGlyphIndicesW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGlyphOutline(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetGlyphOutline unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGlyphOutlineA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetGlyphOutlineA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGlyphOutlineW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetGlyphOutlineW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGlyphOutlineWow(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetGlyphOutlineWow unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetGraphicsMode(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetGraphicsMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetHFONT(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "GetHFONT unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetICMProfileA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetICMProfileA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetICMProfileW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetICMProfileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKerningPairs(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetKerningPairs unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKerningPairsA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetKerningPairsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetKerningPairsW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetKerningPairsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLayout(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GetLayout unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLogColorSpaceA(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetLogColorSpaceA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetLogColorSpaceW(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetLogColorSpaceW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMapMode(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetMapMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMetaFileA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetMetaFileA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMetaFileBitsEx(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetMetaFileBitsEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMetaFileW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetMetaFileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMetaRgn(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetMetaRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetMiterLimit(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetMiterLimit unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetNearestColor(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetNearestColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetNearestPaletteIndex(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetNearestPaletteIndex unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetNumberOfPhysicalMonitors(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "GetNumberOfPhysicalMonitors unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetOPMInformation(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetOPMInformation unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetOPMRandomNumber(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetOPMRandomNumber unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetObjectA(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetObjectA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetObjectType(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetObjectType unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetObjectW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "GetObjectW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetOutlineTextMetricsA(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetOutlineTextMetricsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetOutlineTextMetricsW(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetOutlineTextMetricsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPaletteEntries(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetPaletteEntries unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPath(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "GetPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPhysicalMonitorDescription(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "GetPhysicalMonitorDescription unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPhysicalMonitors(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetPhysicalMonitors unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPixel(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "GetPixel unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPixelFormat(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetPixelFormat unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetPolyFillMode(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetPolyFillMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetProcessSessionFonts(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "GetProcessSessionFonts unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetROP2(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "GetROP2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRandomRgn(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetRandomRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRasterizerCaps(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetRasterizerCaps unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRegionData(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "GetRegionData unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRelAbs(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GetRelAbs unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetRgnBox(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "GetRgnBox unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetStockObject(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetStockObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetStretchBltMode(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetStretchBltMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetStringBitmapA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetStringBitmapA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetStringBitmapW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetStringBitmapW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSuggestedOPMProtectedOutputArraySize(void)
{
    unsigned int len = 55;
    PosWriteFile(1, "GetSuggestedOPMProtectedOutputArraySize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemPaletteEntries(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "GetSystemPaletteEntries unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetSystemPaletteUse(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetSystemPaletteUse unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextAlign(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetTextAlign unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextCharacterExtra(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetTextCharacterExtra unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextCharset(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetTextCharset unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextCharsetInfo(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetTextCharsetInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextColor(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetTextColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextExtentExPointA(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetTextExtentExPointA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextExtentExPointI(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetTextExtentExPointI unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextExtentExPointW(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetTextExtentExPointW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextExtentExPointWPri(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "GetTextExtentExPointWPri unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextExtentPoint32A(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetTextExtentPoint32A unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextExtentPoint32W(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "GetTextExtentPoint32W unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextExtentPointA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetTextExtentPointA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextExtentPointI(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetTextExtentPointI unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextExtentPointW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "GetTextExtentPointW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextFaceA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetTextFaceA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextFaceAliasW(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetTextFaceAliasW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextFaceW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetTextFaceW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextMetricsA(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetTextMetricsA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTextMetricsW(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "GetTextMetricsW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetTransform(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "GetTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetViewportExtEx(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetViewportExtEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetViewportOrgEx(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "GetViewportOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWinMetaFileBits(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "GetWinMetaFileBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowExtEx(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetWindowExtEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWindowOrgEx(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "GetWindowOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI GetWorldTransform(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "GetWorldTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HT_Get8BPPFormatPalette(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "HT_Get8BPPFormatPalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI HT_Get8BPPMaskPalette(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "HT_Get8BPPMaskPalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InternalDeleteDC(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "InternalDeleteDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InternalDeleteObject(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "InternalDeleteObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IntersectClipRect(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "IntersectClipRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI InvertRgn(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "InvertRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsValidEnhMetaRecord(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "IsValidEnhMetaRecord unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI IsValidEnhMetaRecordOffExt(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "IsValidEnhMetaRecordOffExt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LPtoDP(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "LPtoDP unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LineDDA(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "LineDDA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LineTo(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "LineTo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkDrawTextEx(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "LpkDrawTextEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkEditControl(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "LpkEditControl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkExtTextOut(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "LpkExtTextOut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkGetCharacterPlacement(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "LpkGetCharacterPlacement unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkGetEditControl(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "LpkGetEditControl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkGetTextExtentExPoint(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "LpkGetTextExtentExPoint unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkInitialize(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "LpkInitialize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkPSMTextOut(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "LpkPSMTextOut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkPresent(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "LpkPresent unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkTabbedTextOut(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "LpkTabbedTextOut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkUseGDIWidthCache(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "LpkUseGDIWidthCache unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkpEditControlSize(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "LpkpEditControlSize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI LpkpInitializeEditControl(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "LpkpInitializeEditControl unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MaskBlt(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "MaskBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MirrorRgn(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "MirrorRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ModerncoreCreateICW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "ModerncoreCreateICW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ModerncoreDeleteDC(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "ModerncoreDeleteDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ModerncoreGdiInit(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "ModerncoreGdiInit unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ModifyWorldTransform(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ModifyWorldTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI MoveToEx(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "MoveToEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI NamedEscape(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "NamedEscape unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OffsetClipRgn(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "OffsetClipRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OffsetRgn(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "OffsetRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OffsetViewportOrgEx(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "OffsetViewportOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI OffsetWindowOrgEx(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "OffsetWindowOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PATHOBJ_bEnum(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "PATHOBJ_bEnum unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PATHOBJ_bEnumClipLines(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "PATHOBJ_bEnumClipLines unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PATHOBJ_vEnumStart(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "PATHOBJ_vEnumStart unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PATHOBJ_vEnumStartClipLines(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "PATHOBJ_vEnumStartClipLines unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PATHOBJ_vGetBounds(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "PATHOBJ_vGetBounds unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PaintRgn(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "PaintRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PatBlt(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "PatBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PathToRegion(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PathToRegion unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Pie(void)
{
    unsigned int len = 19;
    PosWriteFile(1, "Pie unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PlayEnhMetaFile(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "PlayEnhMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PlayEnhMetaFileRecord(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "PlayEnhMetaFileRecord unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PlayMetaFile(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PlayMetaFile unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PlayMetaFileRecord(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "PlayMetaFileRecord unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PlgBlt(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "PlgBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PolyBezier(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "PolyBezier unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PolyBezierTo(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PolyBezierTo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PolyDraw(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "PolyDraw unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PolyPatBlt(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "PolyPatBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PolyPolygon(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "PolyPolygon unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PolyPolyline(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PolyPolyline unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PolyTextOutA(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PolyTextOutA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PolyTextOutW(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "PolyTextOutW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Polygon(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "Polygon unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI Polyline(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "Polyline unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PolylineTo(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "PolylineTo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PtInRegion(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "PtInRegion unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI PtVisible(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "PtVisible unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI QueryFontAssocStatus(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "QueryFontAssocStatus unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RealizePalette(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "RealizePalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RectInRegion(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "RectInRegion unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RectVisible(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "RectVisible unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveFontMemResourceEx(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "RemoveFontMemResourceEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveFontResourceA(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RemoveFontResourceA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveFontResourceExA(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "RemoveFontResourceExA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveFontResourceExW(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "RemoveFontResourceExW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveFontResourceTracking(void)
{
    unsigned int len = 42;
    PosWriteFile(1, "RemoveFontResourceTracking unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RemoveFontResourceW(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "RemoveFontResourceW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ResetDCA(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "ResetDCA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ResetDCW(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "ResetDCW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ResizePalette(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "ResizePalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RestoreDC(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "RestoreDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI RoundRect(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "RoundRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI STROBJ_bEnum(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "STROBJ_bEnum unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI STROBJ_bEnumPositionsOnly(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "STROBJ_bEnumPositionsOnly unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI STROBJ_bGetAdvanceWidths(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "STROBJ_bGetAdvanceWidths unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI STROBJ_dwGetCodePage(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "STROBJ_dwGetCodePage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI STROBJ_vEnumStart(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "STROBJ_vEnumStart unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SaveDC(void)
{
    unsigned int len = 22;
    PosWriteFile(1, "SaveDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScaleRgn(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "ScaleRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScaleValues(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ScaleValues unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScaleViewportExtEx(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "ScaleViewportExtEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScaleWindowExtEx(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "ScaleWindowExtEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptApplyDigitSubstitution(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "ScriptApplyDigitSubstitution unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptApplyLogicalWidth(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "ScriptApplyLogicalWidth unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptBreak(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ScriptBreak unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptCPtoX(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ScriptCPtoX unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptCacheGetHeight(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ScriptCacheGetHeight unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptFreeCache(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "ScriptFreeCache unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptGetCMap(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "ScriptGetCMap unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptGetFontAlternateGlyphs(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "ScriptGetFontAlternateGlyphs unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptGetFontFeatureTags(void)
{
    unsigned int len = 40;
    PosWriteFile(1, "ScriptGetFontFeatureTags unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptGetFontLanguageTags(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "ScriptGetFontLanguageTags unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptGetFontProperties(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "ScriptGetFontProperties unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptGetFontScriptTags(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "ScriptGetFontScriptTags unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptGetGlyphABCWidth(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "ScriptGetGlyphABCWidth unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptGetLogicalWidths(void)
{
    unsigned int len = 38;
    PosWriteFile(1, "ScriptGetLogicalWidths unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptGetProperties(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "ScriptGetProperties unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptIsComplex(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "ScriptIsComplex unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptItemize(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "ScriptItemize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptItemizeOpenType(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "ScriptItemizeOpenType unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptJustify(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "ScriptJustify unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptLayout(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "ScriptLayout unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptPlace(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ScriptPlace unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptPlaceOpenType(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "ScriptPlaceOpenType unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptPositionSingleGlyph(void)
{
    unsigned int len = 41;
    PosWriteFile(1, "ScriptPositionSingleGlyph unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptRecordDigitSubstitution(void)
{
    unsigned int len = 45;
    PosWriteFile(1, "ScriptRecordDigitSubstitution unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptShape(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ScriptShape unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptShapeOpenType(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "ScriptShapeOpenType unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptStringAnalyse(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "ScriptStringAnalyse unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptStringCPtoX(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "ScriptStringCPtoX unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptStringFree(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "ScriptStringFree unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptStringGetLogicalWidths(void)
{
    unsigned int len = 44;
    PosWriteFile(1, "ScriptStringGetLogicalWidths unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptStringGetOrder(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ScriptStringGetOrder unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptStringOut(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "ScriptStringOut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptStringValidate(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "ScriptStringValidate unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptStringXtoCP(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "ScriptStringXtoCP unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptString_pLogAttr(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "ScriptString_pLogAttr unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptString_pSize(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "ScriptString_pSize unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptString_pcOutChars(void)
{
    unsigned int len = 39;
    PosWriteFile(1, "ScriptString_pcOutChars unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptSubstituteSingleGlyph(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "ScriptSubstituteSingleGlyph unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptTextOut(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "ScriptTextOut unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ScriptXtoCP(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "ScriptXtoCP unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SelectBrushLocal(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SelectBrushLocal unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SelectClipPath(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SelectClipPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SelectClipRgn(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SelectClipRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SelectFontLocal(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SelectFontLocal unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SelectPalette(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SelectPalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetAbortProc(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetAbortProc unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetArcDirection(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetArcDirection unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetBitmapAttributes(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SetBitmapAttributes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetBitmapBits(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetBitmapBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetBitmapDimensionEx(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "SetBitmapDimensionEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetBkColor(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "SetBkColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetBkMode(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "SetBkMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetBoundsRect(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetBoundsRect unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetBrushAttributes(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetBrushAttributes unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetBrushOrgEx(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetBrushOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetColorAdjustment(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetColorAdjustment unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetColorSpace(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetColorSpace unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDCBrushColor(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetDCBrushColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDCPenColor(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetDCPenColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDIBColorTable(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetDIBColorTable unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDIBits(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "SetDIBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDIBitsToDevice(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SetDIBitsToDevice unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetDeviceGammaRamp(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetDeviceGammaRamp unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetEnhMetaFileBits(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetEnhMetaFileBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetFontEnumeration(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetFontEnumeration unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetGraphicsMode(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetGraphicsMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetICMMode(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "SetICMMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetICMProfileA(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetICMProfileA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetICMProfileW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetICMProfileW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetLayout(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "SetLayout unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetLayoutWidth(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetLayoutWidth unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMagicColors(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetMagicColors unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMapMode(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "SetMapMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMapperFlags(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetMapperFlags unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMetaFileBitsEx(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SetMetaFileBitsEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMetaRgn(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "SetMetaRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetMiterLimit(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "SetMiterLimit unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetOPMSigningKeyAndSequenceNumbers(void)
{
    unsigned int len = 50;
    PosWriteFile(1, "SetOPMSigningKeyAndSequenceNumbers unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPaletteEntries(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SetPaletteEntries unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPixel(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "SetPixel unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPixelFormat(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetPixelFormat unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPixelV(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "SetPixelV unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetPolyFillMode(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "SetPolyFillMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetROP2(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "SetROP2 unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetRectRgn(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "SetRectRgn unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetRelAbs(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "SetRelAbs unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetStretchBltMode(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SetStretchBltMode unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetSystemPaletteUse(void)
{
    unsigned int len = 35;
    PosWriteFile(1, "SetSystemPaletteUse unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetTextAlign(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetTextAlign unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetTextCharacterExtra(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "SetTextCharacterExtra unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetTextColor(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "SetTextColor unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetTextJustification(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "SetTextJustification unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetViewportExtEx(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetViewportExtEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetViewportOrgEx(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "SetViewportOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetVirtualResolution(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "SetVirtualResolution unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWinMetaFileBits(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "SetWinMetaFileBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowExtEx(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetWindowExtEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWindowOrgEx(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "SetWindowOrgEx unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SetWorldTransform(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "SetWorldTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI StartDocA(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "StartDocA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI StartDocW(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "StartDocW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI StartFormPage(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "StartFormPage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI StartPage(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "StartPage unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI StretchBlt(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "StretchBlt unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI StretchDIBits(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "StretchDIBits unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI StrokeAndFillPath(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "StrokeAndFillPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI StrokePath(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "StrokePath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI SwapBuffers(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "SwapBuffers unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TextOutW(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "TextOutW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI TranslateCharsetInfo(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "TranslateCharsetInfo unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnloadNetworkFonts(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "UnloadNetworkFonts unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UnrealizeObject(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "UnrealizeObject unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UpdateColors(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "UpdateColors unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UpdateICMRegKeyA(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "UpdateICMRegKeyA unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UpdateICMRegKeyW(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "UpdateICMRegKeyW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UspAllocCache(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "UspAllocCache unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UspAllocTemp(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "UspAllocTemp unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI UspFreeMem(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "UspFreeMem unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI WidenPath(void)
{
    unsigned int len = 25;
    PosWriteFile(1, "WidenPath unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI XFORMOBJ_bApplyXform(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "XFORMOBJ_bApplyXform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI XFORMOBJ_iGetXform(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "XFORMOBJ_iGetXform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI XLATEOBJ_cGetPalette(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "XLATEOBJ_cGetPalette unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI XLATEOBJ_hGetColorTransform(void)
{
    unsigned int len = 43;
    PosWriteFile(1, "XLATEOBJ_hGetColorTransform unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI XLATEOBJ_iXlate(void)
{
    unsigned int len = 31;
    PosWriteFile(1, "XLATEOBJ_iXlate unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI XLATEOBJ_piVector(void)
{
    unsigned int len = 33;
    PosWriteFile(1, "XLATEOBJ_piVector unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI bCreateDCW(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "bCreateDCW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI bDeleteLDC(void)
{
    unsigned int len = 26;
    PosWriteFile(1, "bDeleteLDC unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI bInitSystemAndFontsDirectoriesW(void)
{
    unsigned int len = 47;
    PosWriteFile(1, "bInitSystemAndFontsDirectoriesW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI bMakePathNameW(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "bMakePathNameW unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI cGetTTFFromFOT(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "cGetTTFFromFOT unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI fpClosePrinter(void)
{
    unsigned int len = 30;
    PosWriteFile(1, "fpClosePrinter unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ftsWordBreak(void)
{
    unsigned int len = 28;
    PosWriteFile(1, "ftsWordBreak unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI gCookie(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "gCookie unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI gMaxGdiHandleCount(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "gMaxGdiHandleCount unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI gW32PID(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "gW32PID unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI g_systemCallFilterId(void)
{
    unsigned int len = 36;
    PosWriteFile(1, "g_systemCallFilterId unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI gdiPlaySpoolStream(void)
{
    unsigned int len = 34;
    PosWriteFile(1, "gdiPlaySpoolStream unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI ghICM(void)
{
    unsigned int len = 21;
    PosWriteFile(1, "ghICM unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI hGetPEBHandle(void)
{
    unsigned int len = 29;
    PosWriteFile(1, "hGetPEBHandle unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI pGdiDevCaps(void)
{
    unsigned int len = 27;
    PosWriteFile(1, "pGdiDevCaps unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI pGdiSharedHandleTable(void)
{
    unsigned int len = 37;
    PosWriteFile(1, "pGdiSharedHandleTable unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI pGdiSharedMemory(void)
{
    unsigned int len = 32;
    PosWriteFile(1, "pGdiSharedMemory unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI pldcGet(void)
{
    unsigned int len = 23;
    PosWriteFile(1, "pldcGet unimplemented\r\n", len, &len);
    for (;;) ;
}
void WINAPI vSetPldc(void)
{
    unsigned int len = 24;
    PosWriteFile(1, "vSetPldc unimplemented\r\n", len, &len);
    for (;;) ;
}
/* end of auto-genned functions */
