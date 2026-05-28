// ************************************************************************ //
// WARNING                                                                    
// -------                                                                    
// The types declared in this file were generated from data read from a       
// Type Library. If this type library is explicitly or indirectly (via        
// another type library referring to this type library) re-imported, or the   
// 'Refresh' command of the Type Library Editor activated while editing the   
// Type Library, the contents of this file will be regenerated and all        
// manual modifications will be lost.                                         
// ************************************************************************ //

// C++ TLBWRTR : $Revision:   1.134.1.39  $
// File generated on 2010/3/18 ¤W¤È 11:04:07 from Type Library described below.

// ************************************************************************ //
// Type Lib: C:\Program Files\Euresys\eVision\ActiveX\eVision.ocx (1)
// IID\LCID: {9AE86E03-97BC-11D1-A49B-000021633168}\0
// Helpfile: C:\Program Files\Euresys\eVision\ActiveX\eVisionVB.hlp
// DepndLst: 
//   (1) v2.0 stdole, (C:\WINDOWS\system32\stdole2.tlb)
//   (2) v4.0 StdVCL, (C:\WINDOWS\system32\STDVCL40.DLL)
// Errors:
//   Hint: Symbol 'Click' renamed to '_Click'
//   Hint: Symbol 'Click' renamed to '_Click'
//   Hint: Symbol 'Click' renamed to '_Click'
//   Hint: Symbol 'Click' renamed to '_Click'
//   Hint: Symbol 'Click' renamed to '_Click'
//   Hint: Symbol 'Click' renamed to '_Click'
// ************************************************************************ //

#include <vcl.h>
#pragma hdrstop

#if defined(USING_ATL)
#include <atl\atlvcl.h>
#endif

#include "eVision_OCX.h"

#if !defined(__PRAGMA_PACKAGE_SMART_INIT)
#define      __PRAGMA_PACKAGE_SMART_INIT
#pragma package(smart_init)
#endif

namespace Evision_tlb
{



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEasyMain which
// allows "EasyMain Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEasyMain::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EAE, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EAD, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEasyMain::DEF_CTL_INTF = {0x9AE86EAC, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEasyMain::OptParam;

static inline void ValidCtrCheck(TEasyMain *)
{
   delete new TEasyMain((TComponent*)(0));
};

void __fastcall TEasyMain::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEasyMain::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEasyMainDisp __fastcall TEasyMain::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEasyMain::Render3DBW8(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, float Phi, 
                                       float Psi, float XScale, float YScale, float ZScale, 
                                       long PixelSize)
{
  GetDefaultInterface()->Render3DBW8(BW8SrcImage, BW8DstImage, Phi, Psi, XScale, YScale, ZScale, 
                                     PixelSize);
}

void __fastcall TEasyMain::Render3DC24(LPDISPATCH C24SrcImage, LPDISPATCH BW8ZImage, 
                                       LPDISPATCH C24DstImage, float Phi, float Psi, float XScale, 
                                       float YScale, float ZScale, long PixelSize)
{
  GetDefaultInterface()->Render3DC24(C24SrcImage, BW8ZImage, C24DstImage, Phi, Psi, XScale, YScale, 
                                     ZScale, PixelSize);
}

void __fastcall TEasyMain::RenderColorHistogram(LPDISPATCH C24SrcImage, LPDISPATCH C24DstImage, 
                                                float Phi, float Psi, float XScale, float YScale, 
                                                float ZScale)
{
  GetDefaultInterface()->RenderColorHistogram(C24SrcImage, C24DstImage, Phi, Psi, XScale, YScale, 
                                              ZScale);
}

void __fastcall TEasyMain::OpBW8Cst2(Evision_tlb::enumImgOperators Operation, short GrayCst, 
                                     LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->OpBW8Cst2(Operation, GrayCst, BW8DstImage);
}

void __fastcall TEasyMain::OpBW8Cst23(Evision_tlb::enumImgOperators Operation, short GrayCst, 
                                      LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->OpBW8Cst23(Operation, GrayCst, BW8SrcImage, BW8DstImage);
}

void __fastcall TEasyMain::Op1BW8Cst3(Evision_tlb::enumImgOperators Operation, 
                                      LPDISPATCH BW8SrcImage, short GrayCst, LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->Op1BW8Cst3(Operation, BW8SrcImage, GrayCst, BW8DstImage);
}

void __fastcall TEasyMain::OpC24Cst2(Evision_tlb::enumImgOperators Operation, short RedCst, 
                                     short GreenCst, short BlueCst, LPDISPATCH C24DstImage)
{
  GetDefaultInterface()->OpC24Cst2(Operation, RedCst, GreenCst, BlueCst, C24DstImage);
}

void __fastcall TEasyMain::OpC24Cst23(Evision_tlb::enumImgOperators Operation, short RedCst, 
                                      short GreenCst, short BlueCst, LPDISPATCH C24SrcImage, 
                                      LPDISPATCH C24DstImage)
{
  GetDefaultInterface()->OpC24Cst23(Operation, RedCst, GreenCst, BlueCst, C24SrcImage, C24DstImage);
}

void __fastcall TEasyMain::Op1C24Cst3(Evision_tlb::enumImgOperators Operation, 
                                      LPDISPATCH C24SrcImage, short RedCst, short GreenCst, 
                                      short BlueCst, LPDISPATCH C24DstImage)
{
  GetDefaultInterface()->Op1C24Cst3(Operation, C24SrcImage, RedCst, GreenCst, BlueCst, C24DstImage);
}

void __fastcall TEasyMain::Op12(Evision_tlb::enumImgOperators Operation, LPDISPATCH SrcImage, 
                                LPDISPATCH DstImage)
{
  GetDefaultInterface()->Op12(Operation, SrcImage, DstImage);
}

void __fastcall TEasyMain::Op123(Evision_tlb::enumImgOperators Operation, LPDISPATCH SrcImage1, 
                                 LPDISPATCH SrcImage2, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Op123(Operation, SrcImage1, SrcImage2, DstImage);
}

void __fastcall TEasyMain::Convert(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Convert(SrcImage, DstImage);
}

void __fastcall TEasyMain::GainOffset(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, float Gain, 
                                      float Offset)
{
  GetDefaultInterface()->GainOffset(BW8SrcImage, BW8DstImage, Gain, Offset);
}

void __fastcall TEasyMain::Transform(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, 
                                     long Transformation, TOLEBOOL UseLUT)
{
  GetDefaultInterface()->Transform(BW8SrcImage, BW8DstImage, Transformation, UseLUT);
}

void __fastcall TEasyMain::Lut(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, 
                               LPDISPATCH BW8LUTVector)
{
  GetDefaultInterface()->Lut(BW8SrcImage, BW8DstImage, BW8LUTVector);
}

void __fastcall TEasyMain::Normalize(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, 
                                     float NewAverage, float NewStdDev)
{
  GetDefaultInterface()->Normalize(BW8SrcImage, BW8DstImage, NewAverage, NewStdDev);
}

void __fastcall TEasyMain::Uniformize1(LPDISPATCH BW8SrcImage, short Light, 
                                       LPDISPATCH BW8LightBackgroundImage, LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->Uniformize1(BW8SrcImage, Light, BW8LightBackgroundImage, BW8DstImage);
}

void __fastcall TEasyMain::Uniformize2(LPDISPATCH BW8SrcImage, short Light, 
                                       LPDISPATCH BW8LightBackgroundImage, short Dark, 
                                       LPDISPATCH BW8DarkBackgroundImage, LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->Uniformize2(BW8SrcImage, Light, BW8LightBackgroundImage, Dark, 
                                     BW8DarkBackgroundImage, BW8DstImage);
}

void __fastcall TEasyMain::SetRecursiveAverageLUT(LPDISPATCH BW16LUTVector, float Reduction, 
                                                  float Width)
{
  GetDefaultInterface()->SetRecursiveAverageLUT(BW16LUTVector, Reduction, Width);
}

void __fastcall TEasyMain::RecursiveAverage(LPDISPATCH BW8SrcImage, LPDISPATCH BW16StoreImage, 
                                            LPDISPATCH BW8DstImage, LPDISPATCH BW16LUTVector)
{
  GetDefaultInterface()->RecursiveAverage(BW8SrcImage, BW16StoreImage, BW8DstImage, BW16LUTVector);
}

void __fastcall TEasyMain::ThresholdBW8(LPDISPATCH BW8SrcImage, LPDISPATCH DstImage, long Threshold, 
                                        short LowValue, short HighValue)
{
  GetDefaultInterface()->ThresholdBW8(BW8SrcImage, DstImage, Threshold, LowValue, HighValue);
}

void __fastcall TEasyMain::DoubleThreshold(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, 
                                           long LowThreshold, long HighThreshold, short LowValue, 
                                           short MidValue, short HighValue)
{
  GetDefaultInterface()->DoubleThreshold(BW8SrcImage, BW8DstImage, LowThreshold, HighThreshold, 
                                         LowValue, MidValue, HighValue);
}

void __fastcall TEasyMain::ThresholdC24Lookup(LPDISPATCH C24SrcImage, LPDISPATCH BW8DstImage, 
                                              short MinRedValue, short MinGreenValue, 
                                              short MinBlueValue, short MaxRedValue, 
                                              short MaxGreenValue, short MaxBlueValue, 
                                              LPDISPATCH ColorLookup, short RejectValue, 
                                              short AcceptValue)
{
  GetDefaultInterface()->ThresholdC24Lookup(C24SrcImage, BW8DstImage, MinRedValue, MinGreenValue, 
                                            MinBlueValue, MaxRedValue, MaxGreenValue, MaxBlueValue, 
                                            ColorLookup, RejectValue, AcceptValue);
}

void __fastcall TEasyMain::ThresholdC24(LPDISPATCH C24SrcImage, LPDISPATCH BW8DstImage, 
                                        short MinRedValue, short MinGreenValue, short MinBlueValue, 
                                        short MaxRedValue, short MaxGreenValue, short MaxBlueValue, 
                                        short RejectValue, short AcceptValue)
{
  GetDefaultInterface()->ThresholdC24(C24SrcImage, BW8DstImage, MinRedValue, MinGreenValue, 
                                      MinBlueValue, MaxRedValue, MaxGreenValue, MaxBlueValue, 
                                      RejectValue, AcceptValue);
}

void __fastcall TEasyMain::IsodataThreshold(LPDISPATCH BWHistogramVector, long From, long To, 
                                            long* Threshold)
{
  GetDefaultInterface()->IsodataThreshold(BWHistogramVector, From, To, Threshold);
}

void __fastcall TEasyMain::Convolution(LPDISPATCH SrcImage, LPDISPATCH DstImage, LPDISPATCH Kernel)
{
  GetDefaultInterface()->Convolution(SrcImage, DstImage, Kernel);
}

void __fastcall TEasyMain::Highpass1(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Highpass1(SrcImage, DstImage);
}

void __fastcall TEasyMain::Highpass2(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Highpass2(SrcImage, DstImage);
}

void __fastcall TEasyMain::Lowpass1(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Lowpass1(SrcImage, DstImage);
}

void __fastcall TEasyMain::Lowpass2(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Lowpass2(SrcImage, DstImage);
}

void __fastcall TEasyMain::Lowpass3(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Lowpass3(SrcImage, DstImage);
}

void __fastcall TEasyMain::SobelX(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->SobelX(SrcImage, DstImage);
}

void __fastcall TEasyMain::SobelY(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->SobelY(SrcImage, DstImage);
}

void __fastcall TEasyMain::Sobel(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Sobel(SrcImage, DstImage);
}

void __fastcall TEasyMain::PrewittX(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->PrewittX(SrcImage, DstImage);
}

void __fastcall TEasyMain::PrewittY(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->PrewittY(SrcImage, DstImage);
}

void __fastcall TEasyMain::Prewitt(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Prewitt(SrcImage, DstImage);
}

void __fastcall TEasyMain::Laplacian4(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Laplacian4(SrcImage, DstImage);
}

void __fastcall TEasyMain::Laplacian8(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Laplacian8(SrcImage, DstImage);
}

void __fastcall TEasyMain::Open(LPDISPATCH SrcImage, LPDISPATCH DstImage, long Passes)
{
  GetDefaultInterface()->Open(SrcImage, DstImage, Passes);
}

void __fastcall TEasyMain::Close(LPDISPATCH SrcImage, LPDISPATCH DstImage, long Passes)
{
  GetDefaultInterface()->Close(SrcImage, DstImage, Passes);
}

void __fastcall TEasyMain::Erode(LPDISPATCH SrcImage, LPDISPATCH DstImage, long Passes)
{
  GetDefaultInterface()->Erode(SrcImage, DstImage, Passes);
}

void __fastcall TEasyMain::Dilate(LPDISPATCH SrcImage, LPDISPATCH DstImage, long Passes)
{
  GetDefaultInterface()->Dilate(SrcImage, DstImage, Passes);
}

void __fastcall TEasyMain::BlackTopHat(LPDISPATCH SrcImage, LPDISPATCH DstImage, long Passes)
{
  GetDefaultInterface()->BlackTopHat(SrcImage, DstImage, Passes);
}

void __fastcall TEasyMain::WhiteTopHat(LPDISPATCH SrcImage, LPDISPATCH DstImage, long Passes)
{
  GetDefaultInterface()->WhiteTopHat(SrcImage, DstImage, Passes);
}

void __fastcall TEasyMain::Median(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Median(SrcImage, DstImage);
}

void __fastcall TEasyMain::Thin(LPDISPATCH SrcImage, LPDISPATCH DstImage, LPDISPATCH ThinKernel, 
                                Evision_tlb::enumImgKernelRotationModes KernelRotationMode, 
                                short Iterations)
{
  GetDefaultInterface()->Thin(SrcImage, DstImage, ThinKernel, KernelRotationMode, Iterations);
}

void __fastcall TEasyMain::Thick(LPDISPATCH SrcImage, LPDISPATCH DstImage, LPDISPATCH ThickKernel, 
                                 Evision_tlb::enumImgKernelRotationModes KernelRotationMode, 
                                 short Iterations)
{
  GetDefaultInterface()->Thick(SrcImage, DstImage, ThickKernel, KernelRotationMode, Iterations);
}

void __fastcall TEasyMain::Histogram(LPDISPATCH BW8SrcImage, LPDISPATCH BWHistogramVector)
{
  GetDefaultInterface()->Histogram(BW8SrcImage, BWHistogramVector);
}

float __fastcall TEasyMain::AnalyseHistogram(LPDISPATCH BWHistogramVector, 
                                             Evision_tlb::enumImgHistogramFeatures HistogramFeature, 
                                             long MinIndex, long MaxIndex)
{
  return GetDefaultInterface()->AnalyseHistogram(BWHistogramVector, HistogramFeature, MinIndex, 
                                                 MaxIndex);
}

void __fastcall TEasyMain::Equalize(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->Equalize(BW8SrcImage, BW8DstImage);
}

void __fastcall TEasyMain::ProjectOnARow(LPDISPATCH SrcImage, LPDISPATCH ProjectionVector)
{
  GetDefaultInterface()->ProjectOnARow(SrcImage, ProjectionVector);
}

void __fastcall TEasyMain::ProjectOnAColumn(LPDISPATCH SrcImage, LPDISPATCH ProjectionVector)
{
  GetDefaultInterface()->ProjectOnAColumn(SrcImage, ProjectionVector);
}

void __fastcall TEasyMain::RebuildFrame(LPDISPATCH SrcImage, LPDISPATCH DstImage, long FixedRow)
{
  GetDefaultInterface()->RebuildFrame(SrcImage, DstImage, FixedRow);
}

void __fastcall TEasyMain::RealignFrame(LPDISPATCH SrcImage, LPDISPATCH DstImage, long Offset, 
                                        long FixedRow)
{
  GetDefaultInterface()->RealignFrame(SrcImage, DstImage, Offset, FixedRow);
}

void __fastcall TEasyMain::MatchFrames(LPDISPATCH SrcImage, long FixedRow, long MinOffset, 
                                       long MaxOffset, long* BestOffset)
{
  GetDefaultInterface()->MatchFrames(SrcImage, FixedRow, MinOffset, MaxOffset, BestOffset);
}

void __fastcall TEasyMain::SetupEqualize(LPDISPATCH BWHistogramVector, 
                                         LPDISPATCH BW8VectorLookupTable)
{
  GetDefaultInterface()->SetupEqualize(BWHistogramVector, BW8VectorLookupTable);
}

void __fastcall TEasyMain::ImageToLineSegment(LPDISPATCH SrcImage, LPDISPATCH Vector, long X0, 
                                              long Y0, long X1, long Y1)
{
  GetDefaultInterface()->ImageToLineSegment(SrcImage, Vector, X0, Y0, X1, Y1);
}

void __fastcall TEasyMain::LineSegmentToImage(LPDISPATCH DstImage, LPDISPATCH Vector, long X0, 
                                              long Y0, long X1, long Y1)
{
  GetDefaultInterface()->LineSegmentToImage(DstImage, Vector, X0, Y0, X1, Y1);
}

void __fastcall TEasyMain::ImageToPath(LPDISPATCH SrcImage, LPDISPATCH PathVector)
{
  GetDefaultInterface()->ImageToPath(SrcImage, PathVector);
}

void __fastcall TEasyMain::PathToImage(LPDISPATCH DstImage, LPDISPATCH PathVector)
{
  GetDefaultInterface()->PathToImage(DstImage, PathVector);
}

void __fastcall TEasyMain::ProfileDerivative(LPDISPATCH BW8SrcVector, LPDISPATCH BW8DstVector)
{
  GetDefaultInterface()->ProfileDerivative(BW8SrcVector, BW8DstVector);
}

void __fastcall TEasyMain::GetProfilePeaks(LPDISPATCH BW8VectorProfile, LPDISPATCH PeaksVector, 
                                           long LowThreshold, long HighThreshold, long MinAmplitude, 
                                           long MinArea)
{
  GetDefaultInterface()->GetProfilePeaks(BW8VectorProfile, PeaksVector, LowThreshold, HighThreshold, 
                                         MinAmplitude, MinArea);
}

void __fastcall TEasyMain::PixelMin(LPDISPATCH BW8SrcImage, short* MinPixelValue)
{
  GetDefaultInterface()->PixelMin(BW8SrcImage, MinPixelValue);
}

void __fastcall TEasyMain::PixelMax(LPDISPATCH BW8SrcImage, short* MaxPixelValue)
{
  GetDefaultInterface()->PixelMax(BW8SrcImage, MaxPixelValue);
}

void __fastcall TEasyMain::PixelAverageBW8(LPDISPATCH BW8SrcImage, float* Average)
{
  GetDefaultInterface()->PixelAverageBW8(BW8SrcImage, Average);
}

void __fastcall TEasyMain::PixelAverageC24(LPDISPATCH C24SrcImage, float* Mean0, float* Mean1, 
                                           float* Mean2)
{
  GetDefaultInterface()->PixelAverageC24(C24SrcImage, Mean0, Mean1, Mean2);
}

void __fastcall TEasyMain::PixelStat(LPDISPATCH BW8SrcImage, short* MinPixelValue, 
                                     short* MaxPixelValue, float* Average)
{
  GetDefaultInterface()->PixelStat(BW8SrcImage, MinPixelValue, MaxPixelValue, Average);
}

void __fastcall TEasyMain::PixelStdDevBW8(LPDISPATCH BW8SrcImage, float* StdDev, float* Average)
{
  GetDefaultInterface()->PixelStdDevBW8(BW8SrcImage, StdDev, Average);
}

void __fastcall TEasyMain::PixelStdDevC24(LPDISPATCH C24SrcImage, float* StdDev0, float* StdDev1, 
                                          float* StdDev2, float* Correlation01, float* Correlation12, 
                                          float* Correlation20, float* Mean0, float* Mean1, 
                                          float* Mean2)
{
  GetDefaultInterface()->PixelStdDevC24(C24SrcImage, StdDev0, StdDev1, StdDev2, Correlation01, 
                                        Correlation12, Correlation20, Mean0, Mean1, Mean2);
}

void __fastcall TEasyMain::PixelCount(LPDISPATCH BW8SrcImage, short LowThreshold, 
                                      short HighThreshold, long* PixelsBelow, long* PixelsBetween, 
                                      long* PixelsAbove)
{
  GetDefaultInterface()->PixelCount(BW8SrcImage, LowThreshold, HighThreshold, PixelsBelow, 
                                    PixelsBetween, PixelsAbove);
}

void __fastcall TEasyMain::Area(LPDISPATCH BW8SrcImage, short Threshold, long* PixelsAbove)
{
  GetDefaultInterface()->Area(BW8SrcImage, Threshold, PixelsAbove);
}

void __fastcall TEasyMain::AreaDoubleThreshold(LPDISPATCH BW8SrcImage, short LowThreshold, 
                                               short HighThreshold, long* PixelsBetween)
{
  GetDefaultInterface()->AreaDoubleThreshold(BW8SrcImage, LowThreshold, HighThreshold, PixelsBetween);
}

void __fastcall TEasyMain::PixelVarianceBW8(LPDISPATCH BW8SrcImage, float* Variance, float* Mean)
{
  GetDefaultInterface()->PixelVarianceBW8(BW8SrcImage, Variance, Mean);
}

void __fastcall TEasyMain::PixelVarianceC24(LPDISPATCH C24SrcImage, float* Variance0, 
                                            float* Variance1, float* Variance2, float* Covariance01, 
                                            float* Covariance12, float* Covariance20, float* Mean0, 
                                            float* Mean1, float* Mean2)
{
  GetDefaultInterface()->PixelVarianceC24(C24SrcImage, Variance0, Variance1, Variance2, Covariance01, 
                                          Covariance12, Covariance20, Mean0, Mean1, Mean2);
}

void __fastcall TEasyMain::GravityCenter(LPDISPATCH BW8SrcImage, long Threshold, float* GravityX, 
                                         float* GravityY)
{
  GetDefaultInterface()->GravityCenter(BW8SrcImage, Threshold, GravityX, GravityY);
}

void __fastcall TEasyMain::BinaryMoments1(LPDISPATCH BW8SrcImage, long Threshold, float* M, 
                                          float* Mx, float* My)
{
  GetDefaultInterface()->BinaryMoments1(BW8SrcImage, Threshold, M, Mx, My);
}

void __fastcall TEasyMain::BinaryMoments2(LPDISPATCH BW8SrcImage, long Threshold, float* M, 
                                          float* Mx, float* My, float* Mxx, float* Mxy, float* Myy)
{
  GetDefaultInterface()->BinaryMoments2(BW8SrcImage, Threshold, M, Mx, My, Mxx, Mxy, Myy);
}

void __fastcall TEasyMain::WeighedMoments1(LPDISPATCH BW8SrcImage, float* M, float* Mx, float* My)
{
  GetDefaultInterface()->WeighedMoments1(BW8SrcImage, M, Mx, My);
}

void __fastcall TEasyMain::WeighedMoments2(LPDISPATCH BW8SrcImage, float* M, float* Mx, float* My, 
                                           float* Mxx, float* Mxy, float* Myy)
{
  GetDefaultInterface()->WeighedMoments2(BW8SrcImage, M, Mx, My, Mxx, Mxy, Myy);
}

void __fastcall TEasyMain::ContourBW8(LPDISPATCH BW8SrcImage, 
                                      Evision_tlb::enumImgContourMode ContourMode, long StartX, 
                                      long StartY, 
                                      Evision_tlb::enumImgContourThreshold ThresholdMode, 
                                      long Threshold, 
                                      Evision_tlb::enumImgContourConnexity ConnexityMode, 
                                      LPDISPATCH BW8PathVector)
{
  GetDefaultInterface()->ContourBW8(BW8SrcImage, ContourMode, StartX, StartY, ThresholdMode, 
                                    Threshold, ConnexityMode, BW8PathVector);
}

void __fastcall TEasyMain::VerticalMirror(LPDISPATCH SrcImage)
{
  GetDefaultInterface()->VerticalMirror(SrcImage);
}

void __fastcall TEasyMain::HorizontalMirror(LPDISPATCH SrcImage)
{
  GetDefaultInterface()->HorizontalMirror(SrcImage);
}

void __fastcall TEasyMain::ScaleRotate(LPDISPATCH SrcImage, float SrcPivotX, float SrcPivotY, 
                                       float DstPivotX, float DstPivotY, float ScaleX, float ScaleY, 
                                       float Rotation, LPDISPATCH DstImage, long InterpolationBits)
{
  GetDefaultInterface()->ScaleRotate(SrcImage, SrcPivotX, SrcPivotY, DstPivotX, DstPivotY, ScaleX, 
                                     ScaleY, Rotation, DstImage, InterpolationBits);
}

void __fastcall TEasyMain::Shrink(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Shrink(SrcImage, DstImage);
}

void __fastcall TEasyMain::DequantizeRGB(short SrcRed, short SrcGreen, short SrcBlue, float* DstRed, 
                                         float* DstGreen, float* DstBlue)
{
  GetDefaultInterface()->DequantizeRGB(SrcRed, SrcGreen, SrcBlue, DstRed, DstGreen, DstBlue);
}

void __fastcall TEasyMain::QuantizeRGB(float SrcRed, float SrcGreen, float SrcBlue, short* DstRed, 
                                       short* DstGreen, short* DstBlue)
{
  GetDefaultInterface()->QuantizeRGB(SrcRed, SrcGreen, SrcBlue, DstRed, DstGreen, DstBlue);
}

void __fastcall TEasyMain::DequantizeXYZ(short Red, short Green, short Blue, float* X, float* Y, 
                                         float* Z)
{
  GetDefaultInterface()->DequantizeXYZ(Red, Green, Blue, X, Y, Z);
}

void __fastcall TEasyMain::QuantizeXYZ(float X, float Y, float Z, short* Red, short* Green, 
                                       short* Blue)
{
  GetDefaultInterface()->QuantizeXYZ(X, Y, Z, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToXYZ(float Red, float Green, float Blue, float* X, 
                                               float* Y, float* Z)
{
  GetDefaultInterface()->UnquantizedRGBToXYZ(Red, Green, Blue, X, Y, Z);
}

void __fastcall TEasyMain::UnquantizedXYZToRGB(float X, float Y, float Z, float* Red, float* Green, 
                                               float* Blue)
{
  GetDefaultInterface()->UnquantizedXYZToRGB(X, Y, Z, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToReducedXYZ(float Red, float Green, float Blue, float* X, 
                                                      float* Y, float* Z)
{
  GetDefaultInterface()->UnquantizedRGBToReducedXYZ(Red, Green, Blue, X, Y, Z);
}

void __fastcall TEasyMain::DequantizeYUV(short Red, short Green, short Blue, float* YLuma, 
                                         float* UChroma, float* VChroma)
{
  GetDefaultInterface()->DequantizeYUV(Red, Green, Blue, YLuma, UChroma, VChroma);
}

void __fastcall TEasyMain::QuantizeYUV(float YLuma, float UChroma, float VChroma, short* Red, 
                                       short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizeYUV(YLuma, UChroma, VChroma, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToYUV(float Red, float Green, float Blue, float* YLuma, 
                                               float* UChroma, float* VChroma)
{
  GetDefaultInterface()->UnquantizedRGBToYUV(Red, Green, Blue, YLuma, UChroma, VChroma);
}

void __fastcall TEasyMain::UnquantizedYUVToRGB(float YLuma, float UChroma, float VChroma, float* Red, 
                                               float* Green, float* Blue)
{
  GetDefaultInterface()->UnquantizedYUVToRGB(YLuma, UChroma, VChroma, Red, Green, Blue);
}

void __fastcall TEasyMain::DequantizeYIQ(short Red, short Green, short Blue, float* YLuma, 
                                         float* Inphase, float* Quadrature)
{
  GetDefaultInterface()->DequantizeYIQ(Red, Green, Blue, YLuma, Inphase, Quadrature);
}

void __fastcall TEasyMain::QuantizeYIQ(float YLuma, float Inphase, float Quadrature, short* Red, 
                                       short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizeYIQ(YLuma, Inphase, Quadrature, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToYIQ(float Red, float Green, float Blue, float* YLuma, 
                                               float* Inphase, float* Quadrature)
{
  GetDefaultInterface()->UnquantizedRGBToYIQ(Red, Green, Blue, YLuma, Inphase, Quadrature);
}

void __fastcall TEasyMain::UnquantizedYIQToRGB(float YLuma, float Inphase, float Quadrature, 
                                               float* Red, float* Green, float* Blue)
{
  GetDefaultInterface()->UnquantizedYIQToRGB(YLuma, Inphase, Quadrature, Red, Green, Blue);
}

void __fastcall TEasyMain::DequantizeLSH(short Red, short Green, short Blue, float* Lightness, 
                                         float* Saturation, float* Hue)
{
  GetDefaultInterface()->DequantizeLSH(Red, Green, Blue, Lightness, Saturation, Hue);
}

void __fastcall TEasyMain::QuantizeLSH(float Lightness, float Saturation, float Hue, short* Red, 
                                       short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizeLSH(Lightness, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToLSH(float Red, float Green, float Blue, float* Lightness, 
                                               float* Saturation, float* Hue)
{
  GetDefaultInterface()->UnquantizedRGBToLSH(Red, Green, Blue, Lightness, Saturation, Hue);
}

void __fastcall TEasyMain::UnquantizedLSHToRGB(float Lightness, float Saturation, float Hue, 
                                               float* Red, float* Green, float* Blue)
{
  GetDefaultInterface()->UnquantizedLSHToRGB(Lightness, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::DequantizeVSH(short Red, short Green, short Blue, float* Value, 
                                         float* Saturation, float* Hue)
{
  GetDefaultInterface()->DequantizeVSH(Red, Green, Blue, Value, Saturation, Hue);
}

void __fastcall TEasyMain::QuantizeVSH(float Value, float Saturation, float Hue, short* Red, 
                                       short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizeVSH(Value, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToVSH(float Red, float Green, float Blue, float* Value, 
                                               float* Saturation, float* Hue)
{
  GetDefaultInterface()->UnquantizedRGBToVSH(Red, Green, Blue, Value, Saturation, Hue);
}

void __fastcall TEasyMain::UnquantizedVSHToRGB(float Value, float Saturation, float Hue, float* Red, 
                                               float* Green, float* Blue)
{
  GetDefaultInterface()->UnquantizedVSHToRGB(Value, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::DequantizeISH(short Red, short Green, short Blue, float* Intensity, 
                                         float* Saturation, float* Hue)
{
  GetDefaultInterface()->DequantizeISH(Red, Green, Blue, Intensity, Saturation, Hue);
}

void __fastcall TEasyMain::QuantizeISH(float Intensity, float Saturation, float Hue, short* Red, 
                                       short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizeISH(Intensity, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToISH(float Red, float Green, float Blue, float* Intensity, 
                                               float* Saturation, float* Hue)
{
  GetDefaultInterface()->UnquantizedRGBToISH(Red, Green, Blue, Intensity, Saturation, Hue);
}

void __fastcall TEasyMain::UnquantizedISHToRGB(float Intensity, float Saturation, float Hue, 
                                               float* Red, float* Green, float* Blue)
{
  GetDefaultInterface()->UnquantizedISHToRGB(Intensity, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::DequantizeYSH(short Red, short Green, short Blue, float* Brightness, 
                                         float* Saturation, float* Hue)
{
  GetDefaultInterface()->DequantizeYSH(Red, Green, Blue, Brightness, Saturation, Hue);
}

void __fastcall TEasyMain::QuantizeYSH(float Brightness, float Saturation, float Hue, short* Red, 
                                       short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizeYSH(Brightness, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToYSH(float Red, float Green, float Blue, float* Brightness, 
                                               float* Saturation, float* Hue)
{
  GetDefaultInterface()->UnquantizedRGBToYSH(Red, Green, Blue, Brightness, Saturation, Hue);
}

void __fastcall TEasyMain::UnquantizedYSHToRGB(float Brightness, float Saturation, float Hue, 
                                               float* Red, float* Green, float* Blue)
{
  GetDefaultInterface()->UnquantizedYSHToRGB(Brightness, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::DequantizeLAB(short Red, short Green, short Blue, float* Lightness, 
                                         float* A, float* B)
{
  GetDefaultInterface()->DequantizeLAB(Red, Green, Blue, Lightness, A, B);
}

void __fastcall TEasyMain::QuantizeLAB(float Lightness, float A, float B, short* Red, short* Green, 
                                       short* Blue)
{
  GetDefaultInterface()->QuantizeLAB(Lightness, A, B, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToLAB(float Red, float Green, float Blue, float* Lightness, 
                                               float* A, float* B)
{
  GetDefaultInterface()->UnquantizedRGBToLAB(Red, Green, Blue, Lightness, A, B);
}

void __fastcall TEasyMain::UnquantizedLABToRGB(float Lightness, float A, float B, float* Red, 
                                               float* Green, float* Blue)
{
  GetDefaultInterface()->UnquantizedLABToRGB(Lightness, A, B, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedXYZToLAB(float X, float Y, float Z, float* Lightness, float* A, 
                                               float* B)
{
  GetDefaultInterface()->UnquantizedXYZToLAB(X, Y, Z, Lightness, A, B);
}

void __fastcall TEasyMain::UnquantizedLABToXYZ(float Lightness, float A, float B, float* X, float* Y, 
                                               float* Z)
{
  GetDefaultInterface()->UnquantizedLABToXYZ(Lightness, A, B, X, Y, Z);
}

void __fastcall TEasyMain::DequantizeLCH(short Red, short Green, short Blue, float* Lightness, 
                                         float* Chroma, float* Hue)
{
  GetDefaultInterface()->DequantizeLCH(Red, Green, Blue, Lightness, Chroma, Hue);
}

void __fastcall TEasyMain::QuantizeLCH(float Lightness, float Chroma, float Hue, short* Red, 
                                       short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizeLCH(Lightness, Chroma, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToLCH(float Red, float Green, float Blue, float* Lightness, 
                                               float* Chroma, float* Hue)
{
  GetDefaultInterface()->UnquantizedRGBToLCH(Red, Green, Blue, Lightness, Chroma, Hue);
}

void __fastcall TEasyMain::UnquantizedLCHToRGB(float Lightness, float Chroma, float Hue, float* Red, 
                                               float* Green, float* Blue)
{
  GetDefaultInterface()->UnquantizedLCHToRGB(Lightness, Chroma, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::DequantizeLUV(short Red, short Green, short Blue, float* Lightness, 
                                         float* U, float* V)
{
  GetDefaultInterface()->DequantizeLUV(Red, Green, Blue, Lightness, U, V);
}

void __fastcall TEasyMain::QuantizeLUV(float Lightness, float U, float V, short* Red, short* Green, 
                                       short* Blue)
{
  GetDefaultInterface()->QuantizeLUV(Lightness, U, V, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedRGBToLUV(float Red, float Green, float Blue, float* Lightness, 
                                               float* U, float* V)
{
  GetDefaultInterface()->UnquantizedRGBToLUV(Red, Green, Blue, Lightness, U, V);
}

void __fastcall TEasyMain::UnquantizedLUVToRGB(float Lightness, float U, float V, float* Red, 
                                               float* Green, float* Blue)
{
  GetDefaultInterface()->UnquantizedLUVToRGB(Lightness, U, V, Red, Green, Blue);
}

void __fastcall TEasyMain::UnquantizedXYZToLUV(float X, float Y, float Z, float* Lightness, float* U, 
                                               float* V)
{
  GetDefaultInterface()->UnquantizedXYZToLUV(X, Y, Z, Lightness, U, V);
}

void __fastcall TEasyMain::UnquantizedLUVToXYZ(float Lightness, float U, float V, float* X, float* Y, 
                                               float* Z)
{
  GetDefaultInterface()->UnquantizedLUVToXYZ(Lightness, U, V, X, Y, Z);
}

void __fastcall TEasyMain::QuantizedRGBToXYZ(short Red, short Green, short Blue, short* X, short* Y, 
                                             short* Z)
{
  GetDefaultInterface()->QuantizedRGBToXYZ(Red, Green, Blue, X, Y, Z);
}

void __fastcall TEasyMain::QuantizedXYZToRGB(short X, short Y, short Z, short* Red, short* Green, 
                                             short* Blue)
{
  GetDefaultInterface()->QuantizedXYZToRGB(X, Y, Z, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedRGBToReducedXYZ(short Red, short Green, short Blue, short* X, 
                                                    short* Y, short* Z)
{
  GetDefaultInterface()->QuantizedRGBToReducedXYZ(Red, Green, Blue, X, Y, Z);
}

void __fastcall TEasyMain::QuantizedRGBToYUV(short Red, short Green, short Blue, short* YLuma, 
                                             short* UChroma, short* VChroma)
{
  GetDefaultInterface()->QuantizedRGBToYUV(Red, Green, Blue, YLuma, UChroma, VChroma);
}

void __fastcall TEasyMain::QuantizedYUVToRGB(short YLuma, short UChroma, short VChroma, short* Red, 
                                             short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizedYUVToRGB(YLuma, UChroma, VChroma, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedRGBToYIQ(short Red, short Green, short Blue, short* YLuma, 
                                             short* Inphase, short* Quadrature)
{
  GetDefaultInterface()->QuantizedRGBToYIQ(Red, Green, Blue, YLuma, Inphase, Quadrature);
}

void __fastcall TEasyMain::QuantizedYIQToRGB(short YLuma, short Inphase, short Quadrature, 
                                             short* Red, short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizedYIQToRGB(YLuma, Inphase, Quadrature, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedRGBToLSH(short Red, short Green, short Blue, short* Lightness, 
                                             short* Saturation, short* Hue)
{
  GetDefaultInterface()->QuantizedRGBToLSH(Red, Green, Blue, Lightness, Saturation, Hue);
}

void __fastcall TEasyMain::QuantizedLSHToRGB(short Lightness, short Saturation, short Hue, 
                                             short* Red, short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizedLSHToRGB(Lightness, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedRGBToVSH(short Red, short Green, short Blue, short* Value, 
                                             short* Saturation, short* Hue)
{
  GetDefaultInterface()->QuantizedRGBToVSH(Red, Green, Blue, Value, Saturation, Hue);
}

void __fastcall TEasyMain::QuantizedVSHToRGB(short Value, short Saturation, short Hue, short* Red, 
                                             short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizedVSHToRGB(Value, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedRGBToISH(short Red, short Green, short Blue, short* Intensity, 
                                             short* Saturation, short* Hue)
{
  GetDefaultInterface()->QuantizedRGBToISH(Red, Green, Blue, Intensity, Saturation, Hue);
}

void __fastcall TEasyMain::QuantizedISHToRGB(short Intensity, short Saturation, short Hue, 
                                             short* Red, short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizedISHToRGB(Intensity, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedRGBToYSH(short Red, short Green, short Blue, short* Brightness, 
                                             short* Saturation, short* Hue)
{
  GetDefaultInterface()->QuantizedRGBToYSH(Red, Green, Blue, Brightness, Saturation, Hue);
}

void __fastcall TEasyMain::QuantizedYSHToRGB(short Brightness, short Saturation, short Hue, 
                                             short* Red, short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizedYSHToRGB(Brightness, Saturation, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedRGBToLAB(short Red, short Green, short Blue, short* Luminance, 
                                             short* A, short* B)
{
  GetDefaultInterface()->QuantizedRGBToLAB(Red, Green, Blue, Luminance, A, B);
}

void __fastcall TEasyMain::QuantizedLABToRGB(short Luminance, short A, short B, short* Red, 
                                             short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizedLABToRGB(Luminance, A, B, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedXYZToLAB(short X, short Y, short Z, short* Luminance, short* A, 
                                             short* B)
{
  GetDefaultInterface()->QuantizedXYZToLAB(X, Y, Z, Luminance, A, B);
}

void __fastcall TEasyMain::QuantizedLABToXYZ(short Luminance, short A, short B, short* X, short* Y, 
                                             short* Z)
{
  GetDefaultInterface()->QuantizedLABToXYZ(Luminance, A, B, X, Y, Z);
}

void __fastcall TEasyMain::QuantizedRGBToLCH(short Red, short Green, short Blue, short* Lightness, 
                                             short* Chroma, short* Hue)
{
  GetDefaultInterface()->QuantizedRGBToLCH(Red, Green, Blue, Lightness, Chroma, Hue);
}

void __fastcall TEasyMain::QuantizedLCHToRGB(short Lightness, short Chroma, short Hue, short* Red, 
                                             short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizedLCHToRGB(Lightness, Chroma, Hue, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedRGBToLUV(short Red, short Green, short Blue, short* Luminance, 
                                             short* U, short* V)
{
  GetDefaultInterface()->QuantizedRGBToLUV(Red, Green, Blue, Luminance, U, V);
}

void __fastcall TEasyMain::QuantizedLUVToRGB(short Luminance, short U, short V, short* Red, 
                                             short* Green, short* Blue)
{
  GetDefaultInterface()->QuantizedLUVToRGB(Luminance, U, V, Red, Green, Blue);
}

void __fastcall TEasyMain::QuantizedXYZToLUV(short X, short Y, short Z, short* Luminance, short* U, 
                                             short* V)
{
  GetDefaultInterface()->QuantizedXYZToLUV(X, Y, Z, Luminance, U, V);
}

void __fastcall TEasyMain::QuantizedLUVToXYZ(short Luminance, short U, short V, short* X, short* Y, 
                                             short* Z)
{
  GetDefaultInterface()->QuantizedLUVToXYZ(Luminance, U, V, X, Y, Z);
}

void __fastcall TEasyMain::Compose(LPDISPATCH BW8SrcC0Image, LPDISPATCH BW8SrcC1Image, 
                                   LPDISPATCH BW8SrcC2Image, LPDISPATCH C24DstImage, 
                                   LPDISPATCH ColorLookup)
{
  GetDefaultInterface()->Compose(BW8SrcC0Image, BW8SrcC1Image, BW8SrcC2Image, C24DstImage, 
                                 ColorLookup);
}

void __fastcall TEasyMain::Decompose(LPDISPATCH C24SrcImage, LPDISPATCH BW8DstC0Image, 
                                     LPDISPATCH BW8DstC1Image, LPDISPATCH BW8DstC2Image, 
                                     LPDISPATCH ColorLookup)
{
  GetDefaultInterface()->Decompose(C24SrcImage, BW8DstC0Image, BW8DstC1Image, BW8DstC2Image, 
                                   ColorLookup);
}

void __fastcall TEasyMain::SetComponent(LPDISPATCH BW8SrcImage, LPDISPATCH C24DstImage, 
                                        long Component)
{
  GetDefaultInterface()->SetComponent(BW8SrcImage, C24DstImage, Component);
}

void __fastcall TEasyMain::TransformLookup(LPDISPATCH C24SrcImage, LPDISPATCH C24DstImage, 
                                           LPDISPATCH ColorLookup)
{
  GetDefaultInterface()->TransformLookup(C24SrcImage, C24DstImage, ColorLookup);
}

void __fastcall TEasyMain::TransformQuantized(LPDISPATCH C24SrcImage, LPDISPATCH C24DstImage, 
                                              long Transformation, 
                                              Evision_tlb::enumColorSystems ColorSystemIn, 
                                              Evision_tlb::enumColorSystems ColorSystemOut)
{
  GetDefaultInterface()->TransformQuantized(C24SrcImage, C24DstImage, Transformation, ColorSystemIn, 
                                            ColorSystemOut);
}

void __fastcall TEasyMain::TransformUnquantized(LPDISPATCH C24SrcImage, LPDISPATCH C24DstImage, 
                                                long Transformation, 
                                                Evision_tlb::enumColorSystems ColorSystemIn, 
                                                Evision_tlb::enumColorSystems ColorSystemOut)
{
  GetDefaultInterface()->TransformUnquantized(C24SrcImage, C24DstImage, Transformation, 
                                              ColorSystemIn, ColorSystemOut);
}

void __fastcall TEasyMain::GetComponent(LPDISPATCH C24SrcImage, LPDISPATCH BW8DstImage, 
                                        long Component)
{
  GetDefaultInterface()->GetComponent(C24SrcImage, BW8DstImage, Component);
}

void __fastcall TEasyMain::GetComponentLookup(LPDISPATCH C24SrcImage, LPDISPATCH BW8DstImage, 
                                              long Component, LPDISPATCH ColorLookup)
{
  GetDefaultInterface()->GetComponentLookup(C24SrcImage, BW8DstImage, Component, ColorLookup);
}

void __fastcall TEasyMain::StartTiming(void)
{
  GetDefaultInterface()->StartTiming();
}

long __fastcall TEasyMain::StopTiming(void)
{
  return GetDefaultInterface()->StopTiming();
}

void __fastcall TEasyMain::FromRadians(float* Angle)
{
  GetDefaultInterface()->FromRadians(Angle);
}

void __fastcall TEasyMain::ToRadians(float* Angle)
{
  GetDefaultInterface()->ToRadians(Angle);
}

void __fastcall TEasyMain::ContourArea(LPDISPATCH PathVector, long* Area)
{
  GetDefaultInterface()->ContourArea(PathVector, Area);
}

void __fastcall TEasyMain::ContourGravityCenter(LPDISPATCH PathVector, long* Area, 
                                                float* GravityCenterX, float* GravityCenterY)
{
  GetDefaultInterface()->ContourGravityCenter(PathVector, Area, GravityCenterX, GravityCenterY);
}

void __fastcall TEasyMain::ContourInertia(LPDISPATCH PathVector, long* Area, float* GravityCenterX, 
                                          float* GravityCenterY, float* SigmaX, float* SigmaY, 
                                          float* sigmaXY)
{
  GetDefaultInterface()->ContourInertia(PathVector, Area, GravityCenterX, GravityCenterY, SigmaX, 
                                        SigmaY, sigmaXY);
}

void __fastcall TEasyMain::Contour(LPDISPATCH BW8SrcImage, 
                                   Evision_tlb::enumImgContourMode ContourMode, long StartX, 
                                   long StartY, Evision_tlb::enumImgContourThreshold ThresholdMode, 
                                   long Threshold, 
                                   Evision_tlb::enumImgContourConnexity ConnexityMode, 
                                   LPDISPATCH PathVector)
{
  GetDefaultInterface()->Contour(BW8SrcImage, ContourMode, StartX, StartY, ThresholdMode, Threshold, 
                                 ConnexityMode, PathVector);
}

void __fastcall TEasyMain::Roberts(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Roberts(SrcImage, DstImage);
}

void __fastcall TEasyMain::Distance(LPDISPATCH BW8SrcImage, LPDISPATCH BW16DstImage, long ValOut)
{
  GetDefaultInterface()->Distance(BW8SrcImage, BW16DstImage, ValOut);
}

void __fastcall TEasyMain::ConvertBW16(LPDISPATCH SrcImage, LPDISPATCH DstImage, long Shift)
{
  GetDefaultInterface()->ConvertBW16(SrcImage, DstImage, Shift);
}

void __fastcall TEasyMain::GainOffsetC24(LPDISPATCH C24SrcImage, LPDISPATCH C24DstImage, 
                                         float RedGain, float GreenGain, float BlueGain, 
                                         float RedOffset, float GreenOffset, float BlueOffset)
{
  GetDefaultInterface()->GainOffsetC24(C24SrcImage, C24DstImage, RedGain, GreenGain, BlueGain, 
                                       RedOffset, GreenOffset, BlueOffset);
}

short __fastcall TEasyMain::AutoThresholdLevel(LPDISPATCH BW8SrcImage, 
                                               Evision_tlb::enumImgThresholdModes ThresholdMode, 
                                               float RelativeThreshold)
{
  return GetDefaultInterface()->AutoThresholdLevel(BW8SrcImage, ThresholdMode, RelativeThreshold);
}

void __fastcall TEasyMain::AutoThreshold(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, 
                                         Evision_tlb::enumImgThresholdModes ThresholdMode, 
                                         float RelativeThreshold)
{
  GetDefaultInterface()->AutoThreshold(BW8SrcImage, BW8DstImage, ThresholdMode, RelativeThreshold);
}

void __fastcall TEasyMain::Uniform(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth, 
                                   long HalfHeight)
{
  GetDefaultInterface()->Uniform(SrcImage, DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::VectorUniform(LPDISPATCH BW8SrcVector, LPDISPATCH BW8DstVector, 
                                         long HalfWidth)
{
  GetDefaultInterface()->VectorUniform(BW8SrcVector, BW8DstVector, HalfWidth);
}

float __fastcall TEasyMain::Focusing(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->Focusing(SrcImage);
}

float __fastcall TEasyMain::RmsNoise(LPDISPATCH BW8SrcImage, LPDISPATCH BW8RefImage, 
                                     Evision_tlb::enumImgReferenceNoise ReferenceNoise)
{
  return GetDefaultInterface()->RmsNoise(BW8SrcImage, BW8RefImage, ReferenceNoise);
}

float __fastcall TEasyMain::RmsNoiseAveraged(LPDISPATCH BW8SrcImage, LPDISPATCH BW16RefImage, 
                                             long Count, 
                                             Evision_tlb::enumImgReferenceNoise ReferenceNoise)
{
  return GetDefaultInterface()->RmsNoiseAveraged(BW8SrcImage, BW16RefImage, Count, ReferenceNoise);
}

void __fastcall TEasyMain::SignalNoiseRatio(LPDISPATCH BW8SrcImage, LPDISPATCH BW8RefImage, 
                                            Evision_tlb::enumImgReferenceNoise ReferenceNoise)
{
  GetDefaultInterface()->SignalNoiseRatio(BW8SrcImage, BW8RefImage, ReferenceNoise);
}

void __fastcall TEasyMain::SignalNoiseRatioAveraged(LPDISPATCH BW8SrcImage, LPDISPATCH BW16RefImage, 
                                                    long Count, 
                                                    Evision_tlb::enumImgReferenceNoise ReferenceNoise)
{
  GetDefaultInterface()->SignalNoiseRatioAveraged(BW8SrcImage, BW16RefImage, Count, ReferenceNoise);
}

void __fastcall TEasyMain::RegisterTranslate(LPDISPATCH SrcImage, LPDISPATCH DstImage, 
                                             float SrcPivot0X, float SrcPivot0Y, float DstPivot0X, 
                                             float DstPivot0Y, long InterpolationBits)
{
  GetDefaultInterface()->RegisterTranslate(SrcImage, DstImage, SrcPivot0X, SrcPivot0Y, DstPivot0X, 
                                           DstPivot0Y, InterpolationBits);
}

void __fastcall TEasyMain::Register(LPDISPATCH SrcImage, LPDISPATCH DstImage, float SrcPivot0X, 
                                    float SrcPivot0Y, float SrcPivot1X, float SrcPivot1Y, 
                                    float DstPivot0X, float DstPivot0Y, float DstPivot1X, 
                                    float DstPivot1Y, long InterpolationBits, TOLEBOOL Resize)
{
  GetDefaultInterface()->Register(SrcImage, DstImage, SrcPivot0X, SrcPivot0Y, SrcPivot1X, SrcPivot1Y, 
                                  DstPivot0X, DstPivot0Y, DstPivot1X, DstPivot1Y, InterpolationBits, 
                                  Resize);
}

void __fastcall TEasyMain::EmptyCalibrationPointPairs(void)
{
  GetDefaultInterface()->EmptyCalibrationPointPairs();
}

void __fastcall TEasyMain::AddCalibrationPointPair(float WorldX, float WorldY, float ImageX, 
                                                   float ImageY)
{
  GetDefaultInterface()->AddCalibrationPointPair(WorldX, WorldY, ImageX, ImageY);
}

void __fastcall TEasyMain::SetupLinearCalibration(void)
{
  GetDefaultInterface()->SetupLinearCalibration();
}

void __fastcall TEasyMain::SetupPerspectiveCalibration(void)
{
  GetDefaultInterface()->SetupPerspectiveCalibration();
}

void __fastcall TEasyMain::GetCalibration(float* Axx, float* Axy, float* Ax, float* Ayx, float* Ayy, 
                                          float* Ay, float* Bx, float* By)
{
  GetDefaultInterface()->GetCalibration(Axx, Axy, Ax, Ayx, Ayy, Ay, Bx, By);
}

void __fastcall TEasyMain::GetInvCalibration(float* Axx, float* Axy, float* Ax, float* Ayx, 
                                             float* Ayy, float* Ay, float* Bx, float* By)
{
  GetDefaultInterface()->GetInvCalibration(Axx, Axy, Ax, Ayx, Ayy, Ay, Bx, By);
}

void __fastcall TEasyMain::GetCalibrationAngles(float* AngleX, float* AngleY, float* AngleZ)
{
  GetDefaultInterface()->GetCalibrationAngles(AngleX, AngleY, AngleZ);
}

void __fastcall TEasyMain::LinearCalibration(float WorldX, float WorldY, float* ImageX, 
                                             float* ImageY)
{
  GetDefaultInterface()->LinearCalibration(WorldX, WorldY, ImageX, ImageY);
}

void __fastcall TEasyMain::InvLinearCalibration(float ImageX, float ImageY, float* WorldX, 
                                                float* WorldY)
{
  GetDefaultInterface()->InvLinearCalibration(ImageX, ImageY, WorldX, WorldY);
}

void __fastcall TEasyMain::PerspectiveCalibration(float WorldX, float WorldY, float* ImageX, 
                                                  float* ImageY)
{
  GetDefaultInterface()->PerspectiveCalibration(WorldX, WorldY, ImageX, ImageY);
}

void __fastcall TEasyMain::InvPerspectiveCalibration(float ImageX, float ImageY, float* WorldX, 
                                                     float* WorldY)
{
  GetDefaultInterface()->InvPerspectiveCalibration(ImageX, ImageY, WorldX, WorldY);
}

void __fastcall TEasyMain::LinearCalibrationImg(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->LinearCalibrationImg(BW8SrcImage, BW8DstImage);
}

void __fastcall TEasyMain::InvLinearCalibrationImg(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->InvLinearCalibrationImg(BW8SrcImage, BW8DstImage);
}

void __fastcall TEasyMain::PerspectiveCalibrationImg(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->PerspectiveCalibrationImg(BW8SrcImage, BW8DstImage);
}

void __fastcall TEasyMain::InvPerspectiveCalibrationImg(LPDISPATCH BW8SrcImage, 
                                                        LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->InvPerspectiveCalibrationImg(BW8SrcImage, BW8DstImage);
}

void __fastcall TEasyMain::SymmetricalConvolution(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, 
                                                  LPDISPATCH Kernel)
{
  GetDefaultInterface()->SymmetricalConvolution(BW8SrcImage, BW8DstImage, Kernel);
}

void __fastcall TEasyMain::Uniform3x3(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Uniform3x3(SrcImage, DstImage);
}

void __fastcall TEasyMain::Uniform5x5(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Uniform5x5(SrcImage, DstImage);
}

void __fastcall TEasyMain::Uniform7x7(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Uniform7x7(SrcImage, DstImage);
}

void __fastcall TEasyMain::Gaussian3x3(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Gaussian3x3(SrcImage, DstImage);
}

void __fastcall TEasyMain::Gaussian5x5(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Gaussian5x5(SrcImage, DstImage);
}

void __fastcall TEasyMain::Gaussian7x7(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Gaussian7x7(SrcImage, DstImage);
}

void __fastcall TEasyMain::LocalAverage(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, 
                                        long HalfWidth, long HalfHeight)
{
  GetDefaultInterface()->LocalAverage(BW8SrcImage, BW8DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::LocalDeviation(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, 
                                          long HalfWidth, long HalfHeight)
{
  GetDefaultInterface()->LocalDeviation(BW8SrcImage, BW8DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::DilateBox(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth, 
                                     long HalfHeight)
{
  GetDefaultInterface()->DilateBox(SrcImage, DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::ErodeBox(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth, 
                                    long HalfHeight)
{
  GetDefaultInterface()->ErodeBox(SrcImage, DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::OpenBox(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth, 
                                   long HalfHeight)
{
  GetDefaultInterface()->OpenBox(SrcImage, DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::CloseBox(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth, 
                                    long HalfHeight)
{
  GetDefaultInterface()->CloseBox(SrcImage, DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::WhiteTopHatBox(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth, 
                                          long HalfHeight)
{
  GetDefaultInterface()->WhiteTopHatBox(SrcImage, DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::BlackTopHatBox(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth, 
                                          long HalfHeight)
{
  GetDefaultInterface()->BlackTopHatBox(SrcImage, DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::DilateDisk(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth)
{
  GetDefaultInterface()->DilateDisk(SrcImage, DstImage, HalfWidth);
}

void __fastcall TEasyMain::ErodeDisk(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth)
{
  GetDefaultInterface()->ErodeDisk(SrcImage, DstImage, HalfWidth);
}

void __fastcall TEasyMain::OpenDisk(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth)
{
  GetDefaultInterface()->OpenDisk(SrcImage, DstImage, HalfWidth);
}

void __fastcall TEasyMain::CloseDisk(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth)
{
  GetDefaultInterface()->CloseDisk(SrcImage, DstImage, HalfWidth);
}

void __fastcall TEasyMain::WhiteTopHatDisk(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth)
{
  GetDefaultInterface()->WhiteTopHatDisk(SrcImage, DstImage, HalfWidth);
}

void __fastcall TEasyMain::BlackTopHatDisk(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth)
{
  GetDefaultInterface()->BlackTopHatDisk(SrcImage, DstImage, HalfWidth);
}

void __fastcall TEasyMain::SetCircleWarp(float XCenter, float YCenter, long NumCircles, 
                                         float MinRadius, float MaxRadius, long NumRadii, 
                                         float MinAngle, float MaxAngle, LPDISPATCH BW16XImage, 
                                         LPDISPATCH BW16YImage)
{
  GetDefaultInterface()->SetCircleWarp(XCenter, YCenter, NumCircles, MinRadius, MaxRadius, NumRadii, 
                                       MinAngle, MaxAngle, BW16XImage, BW16YImage);
}

void __fastcall TEasyMain::ComposeRGB(LPDISPATCH BW8SrcC0Image, LPDISPATCH BW8SrcC1Image, 
                                      LPDISPATCH BW8SrcC2Image, LPDISPATCH C24DstImage)
{
  GetDefaultInterface()->ComposeRGB(BW8SrcC0Image, BW8SrcC1Image, BW8SrcC2Image, C24DstImage);
}

void __fastcall TEasyMain::DecomposeRGB(LPDISPATCH C24SrcImage, LPDISPATCH BW8DstC0Image, 
                                        LPDISPATCH BW8DstC1Image, LPDISPATCH BW8DstC2Image)
{
  GetDefaultInterface()->DecomposeRGB(C24SrcImage, BW8DstC0Image, BW8DstC1Image, BW8DstC2Image);
}

void __fastcall TEasyMain::Warp(LPDISPATCH SrcImage, LPDISPATCH DstImage, LPDISPATCH BW16WarpXImage, 
                                LPDISPATCH BW16WarpYImage, long ShiftX, long ShiftY)
{
  GetDefaultInterface()->Warp(SrcImage, DstImage, BW16WarpXImage, BW16WarpYImage, ShiftX, ShiftY);
}

OLE_HANDLE __fastcall TEasyMain::OpenImageBW8DC(LPDISPATCH ImageBW8)
{
  return GetDefaultInterface()->OpenImageBW8DC(ImageBW8);
}

OLE_HANDLE __fastcall TEasyMain::OpenImageC24DC(LPDISPATCH ImageC24)
{
  return GetDefaultInterface()->OpenImageC24DC(ImageC24);
}

void __fastcall TEasyMain::CloseImageBW8DC(LPDISPATCH ImageBW8)
{
  GetDefaultInterface()->CloseImageBW8DC(ImageBW8);
}

void __fastcall TEasyMain::CloseImageC24DC(LPDISPATCH ImageC24)
{
  GetDefaultInterface()->CloseImageC24DC(ImageC24);
}

void __fastcall TEasyMain::Gaussian(LPDISPATCH SrcImage, LPDISPATCH DstImage, long HalfWidth, 
                                    long HalfHeight)
{
  GetDefaultInterface()->Gaussian(SrcImage, DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::LaplacianX(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->LaplacianX(SrcImage, DstImage);
}

void __fastcall TEasyMain::LaplacianY(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->LaplacianY(SrcImage, DstImage);
}

void __fastcall TEasyMain::Gradient(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Gradient(SrcImage, DstImage);
}

void __fastcall TEasyMain::GradientX(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->GradientX(SrcImage, DstImage);
}

void __fastcall TEasyMain::GradientY(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->GradientY(SrcImage, DstImage);
}

void __fastcall TEasyMain::C24ToBayer(LPDISPATCH C24SrcImage, LPDISPATCH BW8DstImage, 
                                      TOLEBOOL EvenCol, TOLEBOOL EvenRow)
{
  GetDefaultInterface()->C24ToBayer(C24SrcImage, BW8DstImage, EvenCol, EvenRow);
}

void __fastcall TEasyMain::BayerToC24(LPDISPATCH BW8SrcImage, LPDISPATCH C24DstImage, 
                                      TOLEBOOL EvenCol, TOLEBOOL EvenRow, TOLEBOOL Interpolate, 
                                      TOLEBOOL Improved)
{
  GetDefaultInterface()->BayerToC24(BW8SrcImage, C24DstImage, EvenCol, EvenRow, Interpolate, 
                                    Improved);
}

void __fastcall TEasyMain::TransformBayer(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage, 
                                          LPDISPATCH ColorLookup, TOLEBOOL EvenCol, TOLEBOOL EvenRow)
{
  GetDefaultInterface()->TransformBayer(BW8SrcImage, BW8DstImage, ColorLookup, EvenCol, EvenRow);
}

void __fastcall TEasyMain::MorphoGradientBox(LPDISPATCH SrcImage, LPDISPATCH DstImage, 
                                             long HalfWidth, long HalfHeight)
{
  GetDefaultInterface()->MorphoGradientBox(SrcImage, DstImage, HalfWidth, HalfHeight);
}

void __fastcall TEasyMain::MorphoGradientDisk(LPDISPATCH SrcImage, LPDISPATCH DstImage, 
                                              long HalfWidth)
{
  GetDefaultInterface()->MorphoGradientDisk(SrcImage, DstImage, HalfWidth);
}

void __fastcall TEasyMain::SetOverlayColor(short RedValue, short GreenValue, short BlueValue)
{
  GetDefaultInterface()->SetOverlayColor(RedValue, GreenValue, BlueValue);
}

void __fastcall TEasyMain::GetOverlayColor(short* pRedValue, short* pGreenValue, short* pBlueValue)
{
  GetDefaultInterface()->GetOverlayColor(pRedValue, pGreenValue, pBlueValue);
}

void __fastcall TEasyMain::Resize(LPDISPATCH SrcImage, LPDISPATCH DstImage)
{
  GetDefaultInterface()->Resize(SrcImage, DstImage);
}

void __fastcall TEasyMain::OpBW16Cst2(Evision_tlb::enumImgOperators Operation, long BW16Constant, 
                                      LPDISPATCH BW16DstImage)
{
  GetDefaultInterface()->OpBW16Cst2(Operation, BW16Constant, BW16DstImage);
}

void __fastcall TEasyMain::RegisterWarp(LPDISPATCH SrcImage, LPDISPATCH DstImage, float SrcPivot0X, 
                                        float SrcPivot0Y, float SrcPivot1X, float SrcPivot1Y, 
                                        float SrcPivot2X, float SrcPivot2Y, float DstPivot0X, 
                                        float DstPivot0Y, float DstPivot1X, float DstPivot1Y, 
                                        float DstPivot2X, float DstPivot2Y, long InterpolationBits)
{
  GetDefaultInterface()->RegisterWarp(SrcImage, DstImage, SrcPivot0X, SrcPivot0Y, SrcPivot1X, 
                                      SrcPivot1Y, SrcPivot2X, SrcPivot2Y, DstPivot0X, DstPivot0Y, 
                                      DstPivot1X, DstPivot1Y, DstPivot2X, DstPivot2Y, 
                                      InterpolationBits);
}

void __fastcall TEasyMain::UniformizeVector1(LPDISPATCH source, long reference, 
                                             LPDISPATCH referenceVector, LPDISPATCH destination, 
                                             TOLEBOOL multiplicative)
{
  GetDefaultInterface()->UniformizeVector1(source, reference, referenceVector, destination, 
                                           multiplicative);
}

void __fastcall TEasyMain::UniformizeVector2(LPDISPATCH source, long lightReference, 
                                             LPDISPATCH lightReferenceVector, long darkReference, 
                                             LPDISPATCH darkReferenceVector, LPDISPATCH destination)
{
  GetDefaultInterface()->UniformizeVector2(source, lightReference, lightReferenceVector, 
                                           darkReference, darkReferenceVector, destination);
}

void __fastcall TEasyMain::Copy(LPDISPATCH source, LPDISPATCH destination)
{
  GetDefaultInterface()->Copy(source, destination);
}

void __fastcall TEasyMain::Overlay(LPDISPATCH source, LPDISPATCH destination, long panX, long panY, 
                                   long redReference, long greenReference, long blueReference)
{
  GetDefaultInterface()->Overlay(source, destination, panX, panY, redReference, greenReference, 
                                 blueReference);
}

void __fastcall TEasyMain::Overlay2(LPDISPATCH source, LPDISPATCH Overlay, LPDISPATCH destination, 
                                    long panX, long panY, long redReference, long greenReference, 
                                    long blueReference)
{
  GetDefaultInterface()->Overlay2(source, Overlay, destination, panX, panY, redReference, 
                                  greenReference, blueReference);
}

long __fastcall TEasyMain::eVision_Malloc(long size)
{
  return GetDefaultInterface()->eVision_Malloc(size);
}

void __fastcall TEasyMain::eVision_Free(long startAddress)
{
  GetDefaultInterface()->eVision_Free(startAddress);
}

void __fastcall TEasyMain::Initialize(void)
{
  GetDefaultInterface()->Initialize();
}

void __fastcall TEasyMain::Terminate(void)
{
  GetDefaultInterface()->Terminate();
}

void __fastcall TEasyMain::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEasyMain::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEasyMain::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEasyMain::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEasyMain::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW8Image which
// allows "EBW8Image Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEBW8Image::EventDispIDs[1] = {
    0x00000001};

int TEBW8Image::TFontIDs[1] = {
    0x0000002E
};

TControlData TEBW8Image::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E0A, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E09, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW8Image::DEF_CTL_INTF = {0x9AE86E08, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBW8Image::OptParam;

static inline void ValidCtrCheck(TEBW8Image *)
{
   delete new TEBW8Image((TComponent*)(0));
};

void __fastcall TEBW8Image::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW8Image::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW8ImageDisp __fastcall TEBW8Image::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEBW8Image::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEBW8Image::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

long __fastcall TEBW8Image::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEBW8Image::SetImagePointer(long Width, long Height, long startAddress, 
                                            long bitsPerRow)
{
  GetDefaultInterface()->SetImagePointer(Width, Height, startAddress, bitsPerRow);
}

void __fastcall TEBW8Image::GetPixel(long X, long Y, short* GrayValue)
{
  GetDefaultInterface()->GetPixel(X, Y, GrayValue);
}

void __fastcall TEBW8Image::SetPixel(long X, long Y, short GrayValue)
{
  GetDefaultInterface()->SetPixel(X, Y, GrayValue);
}

TOLEBOOL __fastcall TEBW8Image::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEBW8Image::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

void __fastcall TEBW8Image::DrawLine(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawLine(X1, Y1, X2, Y2);
}

void __fastcall TEBW8Image::DrawBox(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawBox(X1, Y1, X2, Y2);
}

void __fastcall TEBW8Image::DrawCircle(float X, float Y, float Radius)
{
  GetDefaultInterface()->DrawCircle(X, Y, Radius);
}

void __fastcall TEBW8Image::DrawArc(float X, float Y, float Radius, float Start, float End)
{
  GetDefaultInterface()->DrawArc(X, Y, Radius, Start, End);
}

Evision_tlb::enumFileFormats __fastcall TEBW8Image::LoadFile(BSTR FileName)
{
  return GetDefaultInterface()->LoadFile(FileName);
}

void __fastcall TEBW8Image::SaveFile(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->SaveFile(FileName, FileFormat);
}

void __fastcall TEBW8Image::DrawText(long X, long Y, BSTR TextOut)
{
  GetDefaultInterface()->DrawText(X, Y, TextOut);
}

Evision_tlb::enumFileFormats __fastcall TEBW8Image::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEBW8Image::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

void __fastcall TEBW8Image::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEBW8Image::DrawMaskInImage(void)
{
  GetDefaultInterface()->DrawMaskInImage();
}

void __fastcall TEBW8Image::ClearMask(void)
{
  GetDefaultInterface()->ClearMask();
}

void __fastcall TEBW8Image::UndoMask(void)
{
  GetDefaultInterface()->UndoMask();
}

void __fastcall TEBW8Image::RedoMask(void)
{
  GetDefaultInterface()->RedoMask();
}

void __fastcall TEBW8Image::_Click(long X, long Y)
{
  GetDefaultInterface()->_Click(X, Y);
}

void __fastcall TEBW8Image::DblClick(long X, long Y)
{
  GetDefaultInterface()->DblClick(X, Y);
}

void __fastcall TEBW8Image::SetZoomPan(float fZoomX, float fZoomY, float fPanX, float fPanY)
{
  GetDefaultInterface()->SetZoomPan(fZoomX, fZoomY, fPanX, fPanY);
}

void __fastcall TEBW8Image::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEBW8Image::DrawWithBW8PaletteToDC(OLE_HANDLE hDC, LPDISPATCH BW8Vector, float ZoomX, 
                                                   float ZoomY)
{
  GetDefaultInterface()->DrawWithBW8PaletteToDC(hDC, BW8Vector, ZoomX, ZoomY);
}

void __fastcall TEBW8Image::DrawWithC24PaletteToDC(OLE_HANDLE hDC, LPDISPATCH C24Vector, float ZoomX, 
                                                   float ZoomY)
{
  GetDefaultInterface()->DrawWithC24PaletteToDC(hDC, C24Vector, ZoomX, ZoomY);
}

LPDISPATCH __fastcall TEBW8Image::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW8Image::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW8Image::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW8Image::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW16Image which
// allows "EBW16Image Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEBW16Image::EventDispIDs[1] = {
    0x00000001};

int TEBW16Image::TFontIDs[1] = {
    0x00000023
};

TControlData TEBW16Image::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E0E, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E0D, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW16Image::DEF_CTL_INTF = {0x9AE86E0C, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBW16Image::OptParam;

static inline void ValidCtrCheck(TEBW16Image *)
{
   delete new TEBW16Image((TComponent*)(0));
};

void __fastcall TEBW16Image::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW16Image::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW16ImageDisp __fastcall TEBW16Image::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEBW16Image::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEBW16Image::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

long __fastcall TEBW16Image::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEBW16Image::SetImagePointer(long Width, long Height, long startAddress, 
                                             long bitsPerRow)
{
  GetDefaultInterface()->SetImagePointer(Width, Height, startAddress, bitsPerRow);
}

void __fastcall TEBW16Image::GetPixel(long X, long Y, long* Value)
{
  GetDefaultInterface()->GetPixel(X, Y, Value);
}

void __fastcall TEBW16Image::SetPixel(long X, long Y, long Value)
{
  GetDefaultInterface()->SetPixel(X, Y, Value);
}

TOLEBOOL __fastcall TEBW16Image::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEBW16Image::DrawText(long X, long Y, BSTR TextOut)
{
  GetDefaultInterface()->DrawText(X, Y, TextOut);
}

Evision_tlb::enumFileFormats __fastcall TEBW16Image::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEBW16Image::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

void __fastcall TEBW16Image::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEBW16Image::DrawLine(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawLine(X1, Y1, X2, Y2);
}

void __fastcall TEBW16Image::DrawBox(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawBox(X1, Y1, X2, Y2);
}

void __fastcall TEBW16Image::DrawArc(float X, float Y, float Radius, float Start, float End)
{
  GetDefaultInterface()->DrawArc(X, Y, Radius, Start, End);
}

void __fastcall TEBW16Image::DrawCircle(float X, float Y, float Radius)
{
  GetDefaultInterface()->DrawCircle(X, Y, Radius);
}

void __fastcall TEBW16Image::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

void __fastcall TEBW16Image::ClearMask(void)
{
  GetDefaultInterface()->ClearMask();
}

void __fastcall TEBW16Image::UndoMask(void)
{
  GetDefaultInterface()->UndoMask();
}

void __fastcall TEBW16Image::RedoMask(void)
{
  GetDefaultInterface()->RedoMask();
}

void __fastcall TEBW16Image::_Click(long X, long Y)
{
  GetDefaultInterface()->_Click(X, Y);
}

void __fastcall TEBW16Image::DblClick(long X, long Y)
{
  GetDefaultInterface()->DblClick(X, Y);
}

void __fastcall TEBW16Image::SetZoomPan(float fZoomX, float fZoomY, float fPanX, float fPanY)
{
  GetDefaultInterface()->SetZoomPan(fZoomX, fZoomY, fPanX, fPanY);
}

void __fastcall TEBW16Image::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEBW16Image::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW16Image::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW16Image::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW16Image::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW1Image which
// allows "EBW1Image Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEBW1Image::EventDispIDs[1] = {
    0x00000001};

int TEBW1Image::TFontIDs[1] = {
    0x00000023
};

TControlData TEBW1Image::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x8616C089, 0x921C, 0x4FC9,{ 0xA4, 0x2A, 0x48, 0xB7, 0x72, 0x8A, 0x3C, 0xC3} }, // CoClass
  {0x5BD12BC2, 0xF720, 0x4727,{ 0xAD, 0xEE, 0xAC, 0x07, 0x77, 0xFB, 0x85, 0xC2} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW1Image::DEF_CTL_INTF = {0x44A98082, 0xEB8F, 0x4545,{ 0x80, 0xA4, 0x3F, 0x00, 0xEF, 0x36, 0xE4, 0xB6} };
TNoParam TEBW1Image::OptParam;

static inline void ValidCtrCheck(TEBW1Image *)
{
   delete new TEBW1Image((TComponent*)(0));
};

void __fastcall TEBW1Image::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW1Image::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW1ImageDisp __fastcall TEBW1Image::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEBW1Image::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEBW1Image::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

long __fastcall TEBW1Image::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEBW1Image::SetImagePointer(long Width, long Height, long startAddress, 
                                            long bitsPerRow)
{
  GetDefaultInterface()->SetImagePointer(Width, Height, startAddress, bitsPerRow);
}

void __fastcall TEBW1Image::GetPixel(long X, long Y, long* Value)
{
  GetDefaultInterface()->GetPixel(X, Y, Value);
}

void __fastcall TEBW1Image::SetPixel(long X, long Y, long Value)
{
  GetDefaultInterface()->SetPixel(X, Y, Value);
}

TOLEBOOL __fastcall TEBW1Image::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEBW1Image::DrawText(long X, long Y, BSTR TextOut)
{
  GetDefaultInterface()->DrawText(X, Y, TextOut);
}

Evision_tlb::enumFileFormats __fastcall TEBW1Image::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEBW1Image::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

void __fastcall TEBW1Image::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEBW1Image::DrawLine(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawLine(X1, Y1, X2, Y2);
}

void __fastcall TEBW1Image::DrawBox(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawBox(X1, Y1, X2, Y2);
}

void __fastcall TEBW1Image::DrawArc(float X, float Y, float Radius, float Start, float End)
{
  GetDefaultInterface()->DrawArc(X, Y, Radius, Start, End);
}

void __fastcall TEBW1Image::DrawCircle(float X, float Y, float Radius)
{
  GetDefaultInterface()->DrawCircle(X, Y, Radius);
}

void __fastcall TEBW1Image::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

void __fastcall TEBW1Image::ClearMask(void)
{
  GetDefaultInterface()->ClearMask();
}

void __fastcall TEBW1Image::UndoMask(void)
{
  GetDefaultInterface()->UndoMask();
}

void __fastcall TEBW1Image::RedoMask(void)
{
  GetDefaultInterface()->RedoMask();
}

void __fastcall TEBW1Image::_Click(long X, long Y)
{
  GetDefaultInterface()->_Click(X, Y);
}

void __fastcall TEBW1Image::DblClick(long X, long Y)
{
  GetDefaultInterface()->DblClick(X, Y);
}

void __fastcall TEBW1Image::SetZoomPan(float fZoomX, float fZoomY, float fPanX, float fPanY)
{
  GetDefaultInterface()->SetZoomPan(fZoomX, fZoomY, fPanX, fPanY);
}

void __fastcall TEBW1Image::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEBW1Image::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW1Image::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW1Image::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW1Image::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEC15Image which
// allows "EC15Image Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEC15Image::EventDispIDs[1] = {
    0x00000001};

int TEC15Image::TFontIDs[1] = {
    0x0000002C
};

TControlData TEC15Image::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x45B7F01D, 0xD1C8, 0x4232,{ 0xBA, 0x25, 0x5A, 0x6F, 0x0D, 0x15, 0xA5, 0xCE} }, // CoClass
  {0xF7FDCE30, 0xAF11, 0x487C,{ 0xAB, 0x61, 0xC4, 0xB2, 0x41, 0xBF, 0x2C, 0xEC} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEC15Image::DEF_CTL_INTF = {0xD98F2DF1, 0x30C9, 0x4B11,{ 0xA0, 0x49, 0xD9, 0xEF, 0x35, 0xAE, 0x30, 0xBA} };
TNoParam TEC15Image::OptParam;

static inline void ValidCtrCheck(TEC15Image *)
{
   delete new TEC15Image((TComponent*)(0));
};

void __fastcall TEC15Image::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEC15Image::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEC15ImageDisp __fastcall TEC15Image::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEC15Image::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEC15Image::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

long __fastcall TEC15Image::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEC15Image::SetImagePointer(long Width, long Height, long startAddress, 
                                            long bitsPerRow)
{
  GetDefaultInterface()->SetImagePointer(Width, Height, startAddress, bitsPerRow);
}

void __fastcall TEC15Image::GetPixel(long X, long Y, short* RedValue, short* GreenValue, 
                                     short* BlueValue)
{
  GetDefaultInterface()->GetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

void __fastcall TEC15Image::SetPixel(long X, long Y, short RedValue, short GreenValue, 
                                     short BlueValue)
{
  GetDefaultInterface()->SetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

TOLEBOOL __fastcall TEC15Image::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEC15Image::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

void __fastcall TEC15Image::DrawLine(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawLine(X1, Y1, X2, Y2);
}

void __fastcall TEC15Image::DrawBox(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawBox(X1, Y1, X2, Y2);
}

void __fastcall TEC15Image::DrawCircle(float X, float Y, float Radius)
{
  GetDefaultInterface()->DrawCircle(X, Y, Radius);
}

void __fastcall TEC15Image::DrawArc(float X, float Y, float Radius, float Start, float End)
{
  GetDefaultInterface()->DrawArc(X, Y, Radius, Start, End);
}

void __fastcall TEC15Image::DrawText(long X, long Y, BSTR TextOut)
{
  GetDefaultInterface()->DrawText(X, Y, TextOut);
}

void __fastcall TEC15Image::SaveFile(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->SaveFile(FileName, FileFormat);
}

Evision_tlb::enumFileFormats __fastcall TEC15Image::LoadFile(BSTR FileName)
{
  return GetDefaultInterface()->LoadFile(FileName);
}

void __fastcall TEC15Image::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

Evision_tlb::enumFileFormats __fastcall TEC15Image::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEC15Image::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEC15Image::SetZoomPan(float fZoomX, float fZoomY, float fPanX, float fPanY)
{
  GetDefaultInterface()->SetZoomPan(fZoomX, fZoomY, fPanX, fPanY);
}

void __fastcall TEC15Image::ClearMask(void)
{
  GetDefaultInterface()->ClearMask();
}

void __fastcall TEC15Image::UndoMask(void)
{
  GetDefaultInterface()->UndoMask();
}

void __fastcall TEC15Image::RedoMask(void)
{
  GetDefaultInterface()->RedoMask();
}

void __fastcall TEC15Image::_Click(long X, long Y)
{
  GetDefaultInterface()->_Click(X, Y);
}

void __fastcall TEC15Image::DblClick(long X, long Y)
{
  GetDefaultInterface()->DblClick(X, Y);
}

void __fastcall TEC15Image::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEC15Image::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEC15Image::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEC15Image::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEC15Image::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEC16Image which
// allows "EC16Image Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEC16Image::EventDispIDs[1] = {
    0x00000001};

int TEC16Image::TFontIDs[1] = {
    0x0000002C
};

TControlData TEC16Image::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0xA06066E5, 0xA362, 0x41AE,{ 0xAE, 0xFA, 0xE1, 0x1C, 0xD6, 0x5A, 0xD6, 0x52} }, // CoClass
  {0x85175DD6, 0x5C1C, 0x4A96,{ 0x9C, 0x4E, 0x38, 0x5B, 0x39, 0x49, 0xC3, 0xE0} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEC16Image::DEF_CTL_INTF = {0xB9AC91B2, 0x64C4, 0x40A2,{ 0x80, 0xC0, 0x88, 0xF2, 0x8A, 0x74, 0x94, 0xD8} };
TNoParam TEC16Image::OptParam;

static inline void ValidCtrCheck(TEC16Image *)
{
   delete new TEC16Image((TComponent*)(0));
};

void __fastcall TEC16Image::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEC16Image::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEC16ImageDisp __fastcall TEC16Image::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEC16Image::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEC16Image::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

long __fastcall TEC16Image::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEC16Image::SetImagePointer(long Width, long Height, long startAddress, 
                                            long bitsPerRow)
{
  GetDefaultInterface()->SetImagePointer(Width, Height, startAddress, bitsPerRow);
}

void __fastcall TEC16Image::GetPixel(long X, long Y, short* RedValue, short* GreenValue, 
                                     short* BlueValue)
{
  GetDefaultInterface()->GetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

void __fastcall TEC16Image::SetPixel(long X, long Y, short RedValue, short GreenValue, 
                                     short BlueValue)
{
  GetDefaultInterface()->SetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

TOLEBOOL __fastcall TEC16Image::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEC16Image::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

void __fastcall TEC16Image::DrawLine(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawLine(X1, Y1, X2, Y2);
}

void __fastcall TEC16Image::DrawBox(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawBox(X1, Y1, X2, Y2);
}

void __fastcall TEC16Image::DrawCircle(float X, float Y, float Radius)
{
  GetDefaultInterface()->DrawCircle(X, Y, Radius);
}

void __fastcall TEC16Image::DrawArc(float X, float Y, float Radius, float Start, float End)
{
  GetDefaultInterface()->DrawArc(X, Y, Radius, Start, End);
}

void __fastcall TEC16Image::DrawText(long X, long Y, BSTR TextOut)
{
  GetDefaultInterface()->DrawText(X, Y, TextOut);
}

void __fastcall TEC16Image::SaveFile(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->SaveFile(FileName, FileFormat);
}

Evision_tlb::enumFileFormats __fastcall TEC16Image::LoadFile(BSTR FileName)
{
  return GetDefaultInterface()->LoadFile(FileName);
}

void __fastcall TEC16Image::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

Evision_tlb::enumFileFormats __fastcall TEC16Image::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEC16Image::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEC16Image::SetZoomPan(float fZoomX, float fZoomY, float fPanX, float fPanY)
{
  GetDefaultInterface()->SetZoomPan(fZoomX, fZoomY, fPanX, fPanY);
}

void __fastcall TEC16Image::ClearMask(void)
{
  GetDefaultInterface()->ClearMask();
}

void __fastcall TEC16Image::UndoMask(void)
{
  GetDefaultInterface()->UndoMask();
}

void __fastcall TEC16Image::RedoMask(void)
{
  GetDefaultInterface()->RedoMask();
}

void __fastcall TEC16Image::_Click(long X, long Y)
{
  GetDefaultInterface()->_Click(X, Y);
}

void __fastcall TEC16Image::DblClick(long X, long Y)
{
  GetDefaultInterface()->DblClick(X, Y);
}

void __fastcall TEC16Image::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEC16Image::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEC16Image::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEC16Image::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEC16Image::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEC24Image which
// allows "EC24Image Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEC24Image::EventDispIDs[1] = {
    0x00000001};

int TEC24Image::TFontIDs[1] = {
    0x0000002C
};

TControlData TEC24Image::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E16, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E15, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEC24Image::DEF_CTL_INTF = {0x9AE86E14, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEC24Image::OptParam;

static inline void ValidCtrCheck(TEC24Image *)
{
   delete new TEC24Image((TComponent*)(0));
};

void __fastcall TEC24Image::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEC24Image::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEC24ImageDisp __fastcall TEC24Image::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEC24Image::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEC24Image::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

long __fastcall TEC24Image::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEC24Image::SetImagePointer(long Width, long Height, long startAddress, 
                                            long bitsPerRow)
{
  GetDefaultInterface()->SetImagePointer(Width, Height, startAddress, bitsPerRow);
}

void __fastcall TEC24Image::GetPixel(long X, long Y, short* RedValue, short* GreenValue, 
                                     short* BlueValue)
{
  GetDefaultInterface()->GetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

void __fastcall TEC24Image::SetPixel(long X, long Y, short RedValue, short GreenValue, 
                                     short BlueValue)
{
  GetDefaultInterface()->SetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

TOLEBOOL __fastcall TEC24Image::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEC24Image::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

void __fastcall TEC24Image::DrawLine(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawLine(X1, Y1, X2, Y2);
}

void __fastcall TEC24Image::DrawBox(float X1, float Y1, float X2, float Y2)
{
  GetDefaultInterface()->DrawBox(X1, Y1, X2, Y2);
}

void __fastcall TEC24Image::DrawCircle(float X, float Y, float Radius)
{
  GetDefaultInterface()->DrawCircle(X, Y, Radius);
}

void __fastcall TEC24Image::DrawArc(float X, float Y, float Radius, float Start, float End)
{
  GetDefaultInterface()->DrawArc(X, Y, Radius, Start, End);
}

void __fastcall TEC24Image::DrawText(long X, long Y, BSTR TextOut)
{
  GetDefaultInterface()->DrawText(X, Y, TextOut);
}

void __fastcall TEC24Image::SaveFile(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->SaveFile(FileName, FileFormat);
}

Evision_tlb::enumFileFormats __fastcall TEC24Image::LoadFile(BSTR FileName)
{
  return GetDefaultInterface()->LoadFile(FileName);
}

void __fastcall TEC24Image::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

Evision_tlb::enumFileFormats __fastcall TEC24Image::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEC24Image::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEC24Image::DrawMaskInImage(void)
{
  GetDefaultInterface()->DrawMaskInImage();
}

void __fastcall TEC24Image::ClearMask(void)
{
  GetDefaultInterface()->ClearMask();
}

void __fastcall TEC24Image::UndoMask(void)
{
  GetDefaultInterface()->UndoMask();
}

void __fastcall TEC24Image::RedoMask(void)
{
  GetDefaultInterface()->RedoMask();
}

void __fastcall TEC24Image::_Click(long X, long Y)
{
  GetDefaultInterface()->_Click(X, Y);
}

void __fastcall TEC24Image::DblClick(long X, long Y)
{
  GetDefaultInterface()->DblClick(X, Y);
}

void __fastcall TEC24Image::SetZoomPan(float fZoomX, float fZoomY, float fPanX, float fPanY)
{
  GetDefaultInterface()->SetZoomPan(fZoomX, fZoomY, fPanX, fPanY);
}

void __fastcall TEC24Image::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEC24Image::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEC24Image::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEC24Image::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEC24Image::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW8ROI which
// allows "EBW8ROI Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEBW8ROI::EventDispIDs[1] = {
    0x00000001};

TControlData TEBW8ROI::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E26, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E25, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW8ROI::DEF_CTL_INTF = {0x9AE86E24, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBW8ROI::OptParam;

static inline void ValidCtrCheck(TEBW8ROI *)
{
   delete new TEBW8ROI((TComponent*)(0));
};

void __fastcall TEBW8ROI::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW8ROI::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW8ROIDisp __fastcall TEBW8ROI::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEBW8ROI::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEBW8ROI::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TEBW8ROI::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

void __fastcall TEBW8ROI::SetPlacement(long OriginX, long OriginY, long Width, long Height)
{
  GetDefaultInterface()->SetPlacement(OriginX, OriginY, Width, Height);
}

long __fastcall TEBW8ROI::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEBW8ROI::GetPixel(long X, long Y, short* GrayValue)
{
  GetDefaultInterface()->GetPixel(X, Y, GrayValue);
}

void __fastcall TEBW8ROI::SetPixel(long X, long Y, short GrayValue)
{
  GetDefaultInterface()->SetPixel(X, Y, GrayValue);
}

TOLEBOOL __fastcall TEBW8ROI::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEBW8ROI::DrawFrame(Evision_tlb::enumFramePosition FramePosition, 
                                    TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->DrawFrame(FramePosition, DrawHandles);
}

void __fastcall TEBW8ROI::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

Evision_tlb::enumHandleNames __fastcall TEBW8ROI::HitTest(long X, long Y)
{
  return GetDefaultInterface()->HitTest(X, Y);
}

void __fastcall TEBW8ROI::Drag(Evision_tlb::enumHandleNames Handle, long X, long Y)
{
  GetDefaultInterface()->Drag(Handle, X, Y);
}

short __fastcall TEBW8ROI::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEBW8ROI::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

void __fastcall TEBW8ROI::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEBW8ROI::DrawFrameToDC(OLE_HANDLE hDC, Evision_tlb::enumFramePosition FramePosition, 
                                        TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->DrawFrameToDC(hDC, FramePosition, DrawHandles);
}

LPDISPATCH __fastcall TEBW8ROI::Get_ParentImage()
{
  return GetDefaultInterface()->get_ParentImage();
}

void __fastcall TEBW8ROI::Set_ParentImage(LPDISPATCH param)
{
  GetDefaultInterface()->set_ParentImage(param);
}

LPDISPATCH __fastcall TEBW8ROI::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW8ROI::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW8ROI::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW8ROI::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW16ROI which
// allows "EBW16ROI Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEBW16ROI::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E2A, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E29, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW16ROI::DEF_CTL_INTF = {0x9AE86E28, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBW16ROI::OptParam;

static inline void ValidCtrCheck(TEBW16ROI *)
{
   delete new TEBW16ROI((TComponent*)(0));
};

void __fastcall TEBW16ROI::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW16ROI::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW16ROIDisp __fastcall TEBW16ROI::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEBW16ROI::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEBW16ROI::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TEBW16ROI::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

void __fastcall TEBW16ROI::SetPlacement(long OriginX, long OriginY, long Width, long Height)
{
  GetDefaultInterface()->SetPlacement(OriginX, OriginY, Width, Height);
}

long __fastcall TEBW16ROI::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEBW16ROI::GetPixel(long X, long Y, short* Value)
{
  GetDefaultInterface()->GetPixel(X, Y, Value);
}

void __fastcall TEBW16ROI::SetPixel(long X, long Y, short Value)
{
  GetDefaultInterface()->SetPixel(X, Y, Value);
}

TOLEBOOL __fastcall TEBW16ROI::Void(void)
{
  return GetDefaultInterface()->Void();
}

Evision_tlb::enumFileFormats __fastcall TEBW16ROI::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEBW16ROI::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

void __fastcall TEBW16ROI::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEBW16ROI::Get_ParentImage()
{
  return GetDefaultInterface()->get_ParentImage();
}

void __fastcall TEBW16ROI::Set_ParentImage(LPDISPATCH param)
{
  GetDefaultInterface()->set_ParentImage(param);
}

LPDISPATCH __fastcall TEBW16ROI::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW16ROI::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW16ROI::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW16ROI::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW1ROI which
// allows "EBW1ROI Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEBW1ROI::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x7003DBB0, 0x53EA, 0x4109,{ 0x91, 0xEB, 0xBD, 0x24, 0x3A, 0x15, 0x09, 0x93} }, // CoClass
  {0x70CCFDBF, 0xCBFF, 0x43D4,{ 0xAC, 0x6A, 0xAB, 0x9C, 0x5B, 0x41, 0x29, 0x0D} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW1ROI::DEF_CTL_INTF = {0x8C912CD0, 0xA3CA, 0x4D7B,{ 0x81, 0x90, 0xDC, 0xEC, 0x03, 0x8A, 0x6B, 0xBE} };
TNoParam TEBW1ROI::OptParam;

static inline void ValidCtrCheck(TEBW1ROI *)
{
   delete new TEBW1ROI((TComponent*)(0));
};

void __fastcall TEBW1ROI::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW1ROI::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW1ROIDisp __fastcall TEBW1ROI::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEBW1ROI::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEBW1ROI::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TEBW1ROI::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

void __fastcall TEBW1ROI::SetPlacement(long OriginX, long OriginY, long Width, long Height)
{
  GetDefaultInterface()->SetPlacement(OriginX, OriginY, Width, Height);
}

long __fastcall TEBW1ROI::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEBW1ROI::GetPixel(long X, long Y, short* Value)
{
  GetDefaultInterface()->GetPixel(X, Y, Value);
}

void __fastcall TEBW1ROI::SetPixel(long X, long Y, short Value)
{
  GetDefaultInterface()->SetPixel(X, Y, Value);
}

TOLEBOOL __fastcall TEBW1ROI::Void(void)
{
  return GetDefaultInterface()->Void();
}

Evision_tlb::enumFileFormats __fastcall TEBW1ROI::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEBW1ROI::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

void __fastcall TEBW1ROI::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEBW1ROI::Get_ParentImage()
{
  return GetDefaultInterface()->get_ParentImage();
}

void __fastcall TEBW1ROI::Set_ParentImage(LPDISPATCH param)
{
  GetDefaultInterface()->set_ParentImage(param);
}

LPDISPATCH __fastcall TEBW1ROI::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW1ROI::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW1ROI::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW1ROI::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEC15ROI which
// allows "EC15ROI Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEC15ROI::EventDispIDs[1] = {
    0x00000001};

TControlData TEC15ROI::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x20019AD8, 0x4FB2, 0x4236,{ 0x82, 0xF5, 0x6E, 0x98, 0x57, 0x9C, 0xE2, 0x3D} }, // CoClass
  {0x60D6359B, 0x2601, 0x4D1A,{ 0x9E, 0xB0, 0xA2, 0x5C, 0xD5, 0xF8, 0x53, 0xDB} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEC15ROI::DEF_CTL_INTF = {0x2F632DF7, 0xB24E, 0x407E,{ 0x96, 0xD2, 0xFE, 0xF8, 0xC1, 0xB9, 0x4A, 0xF9} };
TNoParam TEC15ROI::OptParam;

static inline void ValidCtrCheck(TEC15ROI *)
{
   delete new TEC15ROI((TComponent*)(0));
};

void __fastcall TEC15ROI::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEC15ROI::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEC15ROIDisp __fastcall TEC15ROI::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEC15ROI::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEC15ROI::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TEC15ROI::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

void __fastcall TEC15ROI::SetPlacement(long OriginX, long OriginY, long Width, long Height)
{
  GetDefaultInterface()->SetPlacement(OriginX, OriginY, Width, Height);
}

long __fastcall TEC15ROI::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEC15ROI::GetPixel(long X, long Y, short* RedValue, short* GreenValue, 
                                   short* BlueValue)
{
  GetDefaultInterface()->GetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

void __fastcall TEC15ROI::SetPixel(long X, long Y, short RedValue, short GreenValue, short BlueValue)
{
  GetDefaultInterface()->SetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

TOLEBOOL __fastcall TEC15ROI::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEC15ROI::DrawFrame(Evision_tlb::enumFramePosition FramePosition, 
                                    TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->DrawFrame(FramePosition, DrawHandles);
}

void __fastcall TEC15ROI::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

Evision_tlb::enumHandleNames __fastcall TEC15ROI::HitTest(long X, long Y)
{
  return GetDefaultInterface()->HitTest(X, Y);
}

void __fastcall TEC15ROI::Drag(Evision_tlb::enumHandleNames Handle, long X, long Y)
{
  GetDefaultInterface()->Drag(Handle, X, Y);
}

Evision_tlb::enumFileFormats __fastcall TEC15ROI::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEC15ROI::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

void __fastcall TEC15ROI::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEC15ROI::DrawFrameToDC(OLE_HANDLE hDC, Evision_tlb::enumFramePosition FramePosition, 
                                        TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->DrawFrameToDC(hDC, FramePosition, DrawHandles);
}

LPDISPATCH __fastcall TEC15ROI::Get_ParentImage()
{
  return GetDefaultInterface()->get_ParentImage();
}

void __fastcall TEC15ROI::Set_ParentImage(LPDISPATCH param)
{
  GetDefaultInterface()->set_ParentImage(param);
}

LPDISPATCH __fastcall TEC15ROI::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEC15ROI::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEC15ROI::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEC15ROI::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEC16ROI which
// allows "EC16ROI Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEC16ROI::EventDispIDs[1] = {
    0x00000001};

TControlData TEC16ROI::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x68E752D7, 0x55F7, 0x4F41,{ 0xB2, 0x77, 0xE8, 0x67, 0x0F, 0x0D, 0x81, 0x8F} }, // CoClass
  {0x71C6DCC6, 0x1D3F, 0x4801,{ 0x92, 0x69, 0x96, 0x77, 0x7F, 0x18, 0x15, 0xB7} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEC16ROI::DEF_CTL_INTF = {0x6FAB19D3, 0xEA7E, 0x45D9,{ 0xB5, 0x60, 0xFD, 0x73, 0x7B, 0x5D, 0xB8, 0x47} };
TNoParam TEC16ROI::OptParam;

static inline void ValidCtrCheck(TEC16ROI *)
{
   delete new TEC16ROI((TComponent*)(0));
};

void __fastcall TEC16ROI::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEC16ROI::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEC16ROIDisp __fastcall TEC16ROI::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEC16ROI::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEC16ROI::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TEC16ROI::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

void __fastcall TEC16ROI::SetPlacement(long OriginX, long OriginY, long Width, long Height)
{
  GetDefaultInterface()->SetPlacement(OriginX, OriginY, Width, Height);
}

long __fastcall TEC16ROI::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEC16ROI::GetPixel(long X, long Y, short* RedValue, short* GreenValue, 
                                   short* BlueValue)
{
  GetDefaultInterface()->GetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

void __fastcall TEC16ROI::SetPixel(long X, long Y, short RedValue, short GreenValue, short BlueValue)
{
  GetDefaultInterface()->SetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

TOLEBOOL __fastcall TEC16ROI::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEC16ROI::DrawFrame(Evision_tlb::enumFramePosition FramePosition, 
                                    TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->DrawFrame(FramePosition, DrawHandles);
}

void __fastcall TEC16ROI::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

Evision_tlb::enumHandleNames __fastcall TEC16ROI::HitTest(long X, long Y)
{
  return GetDefaultInterface()->HitTest(X, Y);
}

void __fastcall TEC16ROI::Drag(Evision_tlb::enumHandleNames Handle, long X, long Y)
{
  GetDefaultInterface()->Drag(Handle, X, Y);
}

Evision_tlb::enumFileFormats __fastcall TEC16ROI::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEC16ROI::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

void __fastcall TEC16ROI::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEC16ROI::DrawFrameToDC(OLE_HANDLE hDC, Evision_tlb::enumFramePosition FramePosition, 
                                        TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->DrawFrameToDC(hDC, FramePosition, DrawHandles);
}

LPDISPATCH __fastcall TEC16ROI::Get_ParentImage()
{
  return GetDefaultInterface()->get_ParentImage();
}

void __fastcall TEC16ROI::Set_ParentImage(LPDISPATCH param)
{
  GetDefaultInterface()->set_ParentImage(param);
}

LPDISPATCH __fastcall TEC16ROI::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEC16ROI::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEC16ROI::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEC16ROI::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEC24ROI which
// allows "EC24ROI Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int   TEC24ROI::EventDispIDs[1] = {
    0x00000001};

TControlData TEC24ROI::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E32, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E31, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  1, &EventDispIDs,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEC24ROI::DEF_CTL_INTF = {0x9AE86E30, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEC24ROI::OptParam;

static inline void ValidCtrCheck(TEC24ROI *)
{
   delete new TEC24ROI((TComponent*)(0));
};

void __fastcall TEC24ROI::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEC24ROI::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEC24ROIDisp __fastcall TEC24ROI::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

TOLEBOOL __fastcall TEC24ROI::SameSize(LPDISPATCH SrcImage)
{
  return GetDefaultInterface()->SameSize(SrcImage);
}

void __fastcall TEC24ROI::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TEC24ROI::SetSize(long Width, long Height)
{
  GetDefaultInterface()->SetSize(Width, Height);
}

void __fastcall TEC24ROI::SetPlacement(long OriginX, long OriginY, long Width, long Height)
{
  GetDefaultInterface()->SetPlacement(OriginX, OriginY, Width, Height);
}

long __fastcall TEC24ROI::GetImagePointer(long OriginX, long OriginY)
{
  return GetDefaultInterface()->GetImagePointer(OriginX, OriginY);
}

void __fastcall TEC24ROI::GetPixel(long X, long Y, short* RedValue, short* GreenValue, 
                                   short* BlueValue)
{
  GetDefaultInterface()->GetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

void __fastcall TEC24ROI::SetPixel(long X, long Y, short RedValue, short GreenValue, short BlueValue)
{
  GetDefaultInterface()->SetPixel(X, Y, RedValue, GreenValue, BlueValue);
}

TOLEBOOL __fastcall TEC24ROI::Void(void)
{
  return GetDefaultInterface()->Void();
}

void __fastcall TEC24ROI::DrawFrame(Evision_tlb::enumFramePosition FramePosition, 
                                    TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->DrawFrame(FramePosition, DrawHandles);
}

void __fastcall TEC24ROI::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

Evision_tlb::enumHandleNames __fastcall TEC24ROI::HitTest(long X, long Y)
{
  return GetDefaultInterface()->HitTest(X, Y);
}

void __fastcall TEC24ROI::Drag(Evision_tlb::enumHandleNames Handle, long X, long Y)
{
  GetDefaultInterface()->Drag(Handle, X, Y);
}

Evision_tlb::enumFileFormats __fastcall TEC24ROI::Load(BSTR FileName)
{
  return GetDefaultInterface()->Load(FileName);
}

void __fastcall TEC24ROI::Save(BSTR FileName, Evision_tlb::enumFileFormats FileFormat)
{
  GetDefaultInterface()->Save(FileName, FileFormat);
}

void __fastcall TEC24ROI::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEC24ROI::DrawFrameToDC(OLE_HANDLE hDC, Evision_tlb::enumFramePosition FramePosition, 
                                        TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->DrawFrameToDC(hDC, FramePosition, DrawHandles);
}

LPDISPATCH __fastcall TEC24ROI::Get_ParentImage()
{
  return GetDefaultInterface()->get_ParentImage();
}

void __fastcall TEC24ROI::Set_ParentImage(LPDISPATCH param)
{
  GetDefaultInterface()->set_ParentImage(param);
}

LPDISPATCH __fastcall TEC24ROI::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEC24ROI::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEC24ROI::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEC24ROI::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEImageSequence which
// allows "EImageSequence Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEImageSequence::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EC6, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EC5, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEImageSequence::DEF_CTL_INTF = {0x9AE86EC4, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEImageSequence::OptParam;

static inline void ValidCtrCheck(TEImageSequence *)
{
   delete new TEImageSequence((TComponent*)(0));
};

void __fastcall TEImageSequence::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEImageSequence::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEImageSequenceDisp __fastcall TEImageSequence::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEImageSequence::Seek(long lngDelta)
{
  GetDefaultInterface()->Seek(lngDelta);
}

void __fastcall TEImageSequence::AddImage(LPDISPATCH SrcImage)
{
  GetDefaultInterface()->AddImage(SrcImage);
}

void __fastcall TEImageSequence::GetImage(LPDISPATCH DstImage)
{
  GetDefaultInterface()->GetImage(DstImage);
}

void __fastcall TEImageSequence::CloseFile(void)
{
  GetDefaultInterface()->CloseFile();
}

void __fastcall TEImageSequence::StartLoading(BSTR pszFileName)
{
  GetDefaultInterface()->StartLoading(pszFileName);
}

void __fastcall TEImageSequence::StartSaving(BSTR pszFileName, long hwndDialogParent)
{
  GetDefaultInterface()->StartSaving(pszFileName, hwndDialogParent);
}

void __fastcall TEImageSequence::AddImageData(long NewValue)
{
  GetDefaultInterface()->AddImageData(NewValue);
}

void __fastcall TEImageSequence::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW8Vector which
// allows "EBW8Vector Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int TEBW8Vector::TFontIDs[1] = {
    0xFFFFFE00
};

TControlData TEBW8Vector::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E42, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E41, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000004,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW8Vector::DEF_CTL_INTF = {0x9AE86E40, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBW8Vector::OptParam;

static inline void ValidCtrCheck(TEBW8Vector *)
{
   delete new TEBW8Vector((TComponent*)(0));
};

void __fastcall TEBW8Vector::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW8Vector::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW8VectorDisp __fastcall TEBW8Vector::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEBW8Vector::Empty(void)
{
  GetDefaultInterface()->Empty();
}

void __fastcall TEBW8Vector::AddElement(short GrayValue)
{
  GetDefaultInterface()->AddElement(GrayValue);
}

void __fastcall TEBW8Vector::GetAt(long Index, short* GrayValue)
{
  GetDefaultInterface()->GetAt(Index, GrayValue);
}

void __fastcall TEBW8Vector::SetAt(long Index, short GrayValue)
{
  GetDefaultInterface()->SetAt(Index, GrayValue);
}

void __fastcall TEBW8Vector::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEBW8Vector::SetYRange(long YMin, long YMax)
{
  GetDefaultInterface()->SetYRange(YMin, YMax);
}

void __fastcall TEBW8Vector::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEBW8Vector::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW8Vector::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW8Vector::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW8Vector::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW16Vector which
// allows "EBW16Vector Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int TEBW16Vector::TFontIDs[1] = {
    0xFFFFFE00
};

TControlData TEBW16Vector::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E46, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E45, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000004,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW16Vector::DEF_CTL_INTF = {0x9AE86E44, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBW16Vector::OptParam;

static inline void ValidCtrCheck(TEBW16Vector *)
{
   delete new TEBW16Vector((TComponent*)(0));
};

void __fastcall TEBW16Vector::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW16Vector::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW16VectorDisp __fastcall TEBW16Vector::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEBW16Vector::Empty(void)
{
  GetDefaultInterface()->Empty();
}

void __fastcall TEBW16Vector::AddElement(short GrayValue)
{
  GetDefaultInterface()->AddElement(GrayValue);
}

void __fastcall TEBW16Vector::GetAt(long Index, short* GrayValue)
{
  GetDefaultInterface()->GetAt(Index, GrayValue);
}

void __fastcall TEBW16Vector::SetAt(long Index, short GrayValue)
{
  GetDefaultInterface()->SetAt(Index, GrayValue);
}

void __fastcall TEBW16Vector::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEBW16Vector::SetYRange(long YMin, long YMax)
{
  GetDefaultInterface()->SetYRange(YMin, YMax);
}

void __fastcall TEBW16Vector::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEBW16Vector::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW16Vector::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW16Vector::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW16Vector::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW32Vector which
// allows "EBW32Vector Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int TEBW32Vector::TFontIDs[1] = {
    0xFFFFFE00
};

TControlData TEBW32Vector::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E4A, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E49, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000004,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW32Vector::DEF_CTL_INTF = {0x9AE86E48, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBW32Vector::OptParam;

static inline void ValidCtrCheck(TEBW32Vector *)
{
   delete new TEBW32Vector((TComponent*)(0));
};

void __fastcall TEBW32Vector::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW32Vector::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW32VectorDisp __fastcall TEBW32Vector::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEBW32Vector::Empty(void)
{
  GetDefaultInterface()->Empty();
}

void __fastcall TEBW32Vector::AddElement(long GrayValue)
{
  GetDefaultInterface()->AddElement(GrayValue);
}

void __fastcall TEBW32Vector::GetAt(long Index, long* GrayValue)
{
  GetDefaultInterface()->GetAt(Index, GrayValue);
}

void __fastcall TEBW32Vector::SetAt(long Index, long GrayValue)
{
  GetDefaultInterface()->SetAt(Index, GrayValue);
}

void __fastcall TEBW32Vector::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEBW32Vector::SetYRange(long YMin, long YMax)
{
  GetDefaultInterface()->SetYRange(YMin, YMax);
}

void __fastcall TEBW32Vector::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEBW32Vector::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW32Vector::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW32Vector::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW32Vector::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEC24Vector which
// allows "EC24Vector Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEC24Vector::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E4E, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E4D, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEC24Vector::DEF_CTL_INTF = {0x9AE86E4C, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEC24Vector::OptParam;

static inline void ValidCtrCheck(TEC24Vector *)
{
   delete new TEC24Vector((TComponent*)(0));
};

void __fastcall TEC24Vector::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEC24Vector::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEC24VectorDisp __fastcall TEC24Vector::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEC24Vector::Empty(void)
{
  GetDefaultInterface()->Empty();
}

void __fastcall TEC24Vector::AddElement(short RedValue, short GreenValue, short BlueValue)
{
  GetDefaultInterface()->AddElement(RedValue, GreenValue, BlueValue);
}

void __fastcall TEC24Vector::GetAt(long Index, short* RedValue, short* GreenValue, short* BlueValue)
{
  GetDefaultInterface()->GetAt(Index, RedValue, GreenValue, BlueValue);
}

void __fastcall TEC24Vector::SetAt(long Index, short RedValue, short GreenValue, short BlueValue)
{
  GetDefaultInterface()->SetAt(Index, RedValue, GreenValue, BlueValue);
}

void __fastcall TEC24Vector::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEC24Vector::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEC24Vector::SetYRange(long YMin, long YMax)
{
  GetDefaultInterface()->SetYRange(YMin, YMax);
}

LPDISPATCH __fastcall TEC24Vector::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEC24Vector::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEC24Vector::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEC24Vector::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEPathVector which
// allows "EPathVector Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEPathVector::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E5E, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E5D, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEPathVector::DEF_CTL_INTF = {0x9AE86E5C, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEPathVector::OptParam;

static inline void ValidCtrCheck(TEPathVector *)
{
   delete new TEPathVector((TComponent*)(0));
};

void __fastcall TEPathVector::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEPathVector::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEPathVectorDisp __fastcall TEPathVector::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEPathVector::Empty(void)
{
  GetDefaultInterface()->Empty();
}

void __fastcall TEPathVector::AddElement(short X, short Y)
{
  GetDefaultInterface()->AddElement(X, Y);
}

void __fastcall TEPathVector::GetAt(long Index, short* X, short* Y)
{
  GetDefaultInterface()->GetAt(Index, X, Y);
}

void __fastcall TEPathVector::SetAt(long Index, short X, short Y)
{
  GetDefaultInterface()->SetAt(Index, X, Y);
}

void __fastcall TEPathVector::Draw(LPDISPATCH Image)
{
  GetDefaultInterface()->Draw(Image);
}

void __fastcall TEPathVector::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEPathVector::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

LPDISPATCH __fastcall TEPathVector::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEPathVector::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEPathVector::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEPathVector::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBWHistogramVector which
// allows "EBWHistogramVector Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
int TEBWHistogramVector::TFontIDs[1] = {
    0xFFFFFE00
};

TControlData TEBWHistogramVector::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E66, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E65, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000004,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  1, TFontIDs,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBWHistogramVector::DEF_CTL_INTF = {0x9AE86E64, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBWHistogramVector::OptParam;

static inline void ValidCtrCheck(TEBWHistogramVector *)
{
   delete new TEBWHistogramVector((TComponent*)(0));
};

void __fastcall TEBWHistogramVector::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBWHistogramVector::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBWHistogramVectorDisp __fastcall TEBWHistogramVector::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEBWHistogramVector::Empty(void)
{
  GetDefaultInterface()->Empty();
}

void __fastcall TEBWHistogramVector::AddElement(long Value)
{
  GetDefaultInterface()->AddElement(Value);
}

void __fastcall TEBWHistogramVector::GetAt(long Index, long* Value)
{
  GetDefaultInterface()->GetAt(Index, Value);
}

void __fastcall TEBWHistogramVector::SetAt(long Index, long Value)
{
  GetDefaultInterface()->SetAt(Index, Value);
}

void __fastcall TEBWHistogramVector::Refresh(void)
{
  GetDefaultInterface()->Refresh();
}

void __fastcall TEBWHistogramVector::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEBWHistogramVector::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBWHistogramVector::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBWHistogramVector::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBWHistogramVector::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBW8PathVector which
// allows "EBW8PathVector Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEBW8PathVector::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E6A, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E69, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBW8PathVector::DEF_CTL_INTF = {0x9AE86E68, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBW8PathVector::OptParam;

static inline void ValidCtrCheck(TEBW8PathVector *)
{
   delete new TEBW8PathVector((TComponent*)(0));
};

void __fastcall TEBW8PathVector::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBW8PathVector::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBW8PathVectorDisp __fastcall TEBW8PathVector::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEBW8PathVector::Empty(void)
{
  GetDefaultInterface()->Empty();
}

void __fastcall TEBW8PathVector::AddElement(short X, short Y, short Pixel)
{
  GetDefaultInterface()->AddElement(X, Y, Pixel);
}

void __fastcall TEBW8PathVector::GetAt(long Index, short* X, short* Y, short* Pixel)
{
  GetDefaultInterface()->GetAt(Index, X, Y, Pixel);
}

void __fastcall TEBW8PathVector::SetAt(long Index, short X, short Y, short Pixel)
{
  GetDefaultInterface()->SetAt(Index, X, Y, Pixel);
}

void __fastcall TEBW8PathVector::Draw(LPDISPATCH Image)
{
  GetDefaultInterface()->Draw(Image);
}

void __fastcall TEBW8PathVector::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEBW8PathVector::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

LPDISPATCH __fastcall TEBW8PathVector::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEBW8PathVector::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEBW8PathVector::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEBW8PathVector::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEC24PathVector which
// allows "EC24PathVector Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEC24PathVector::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E6E, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E6D, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEC24PathVector::DEF_CTL_INTF = {0x9AE86E6C, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEC24PathVector::OptParam;

static inline void ValidCtrCheck(TEC24PathVector *)
{
   delete new TEC24PathVector((TComponent*)(0));
};

void __fastcall TEC24PathVector::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEC24PathVector::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEC24PathVectorDisp __fastcall TEC24PathVector::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEC24PathVector::Empty(void)
{
  GetDefaultInterface()->Empty();
}

void __fastcall TEC24PathVector::AddElement(short X, short Y, short PixelRed, short PixelGreen, 
                                            short PixelBlue)
{
  GetDefaultInterface()->AddElement(X, Y, PixelRed, PixelGreen, PixelBlue);
}

void __fastcall TEC24PathVector::GetAt(short Index, short* X, long* Y, short* PixelRed, 
                                       short* PixelGreen, short* PixelBlue)
{
  GetDefaultInterface()->GetAt(Index, X, Y, PixelRed, PixelGreen, PixelBlue);
}

void __fastcall TEC24PathVector::SetAt(short Index, short X, long Y, short PixelRed, 
                                       short PixelGreen, short PixelBlue)
{
  GetDefaultInterface()->SetAt(Index, X, Y, PixelRed, PixelGreen, PixelBlue);
}

void __fastcall TEC24PathVector::Draw(LPDISPATCH Image)
{
  GetDefaultInterface()->Draw(Image);
}

void __fastcall TEC24PathVector::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEC24PathVector::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

LPDISPATCH __fastcall TEC24PathVector::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEC24PathVector::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEC24PathVector::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEC24PathVector::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEPeaksVector which
// allows "EPeaksVector Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEPeaksVector::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E72, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E71, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEPeaksVector::DEF_CTL_INTF = {0x9AE86E70, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEPeaksVector::OptParam;

static inline void ValidCtrCheck(TEPeaksVector *)
{
   delete new TEPeaksVector((TComponent*)(0));
};

void __fastcall TEPeaksVector::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEPeaksVector::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEPeaksVectorDisp __fastcall TEPeaksVector::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEPeaksVector::Empty(void)
{
  GetDefaultInterface()->Empty();
}

void __fastcall TEPeaksVector::AddElement(long Start, long Length, long Amplitude, long Area, 
                                          float Center)
{
  GetDefaultInterface()->AddElement(Start, Length, Amplitude, Area, Center);
}

void __fastcall TEPeaksVector::GetAt(long Index, long* Start, long* Length, long* Amplitude, 
                                     long* Area, float* Center)
{
  GetDefaultInterface()->GetAt(Index, Start, Length, Amplitude, Area, Center);
}

void __fastcall TEPeaksVector::SetAt(long Index, long Start, long Length, long Amplitude, long Area, 
                                     float Center)
{
  GetDefaultInterface()->SetAt(Index, Start, Length, Amplitude, Area, Center);
}

void __fastcall TEPeaksVector::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEPeaksVector::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEPeaksVector::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEPeaksVector::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEPeaksVector::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEKernel which
// allows "EKernel Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEKernel::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E7A, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E79, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEKernel::DEF_CTL_INTF = {0x9AE86E78, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEKernel::OptParam;

static inline void ValidCtrCheck(TEKernel *)
{
   delete new TEKernel((TComponent*)(0));
};

void __fastcall TEKernel::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEKernel::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEKernelDisp __fastcall TEKernel::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEKernel::GetKernelData(long X, long Y, float* Data)
{
  GetDefaultInterface()->GetKernelData(X, Y, Data);
}

void __fastcall TEKernel::SetKernelData(long X, long Y, float Data)
{
  GetDefaultInterface()->SetKernelData(X, Y, Data);
}

void __fastcall TEKernel::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEKernel::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEKernel::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEKernel::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEKernel::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEMovingAverage which
// allows "EMovingAverage Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEMovingAverage::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EBE, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EBD, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEMovingAverage::DEF_CTL_INTF = {0x9AE86EBC, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEMovingAverage::OptParam;

static inline void ValidCtrCheck(TEMovingAverage *)
{
   delete new TEMovingAverage((TComponent*)(0));
};

void __fastcall TEMovingAverage::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEMovingAverage::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEMovingAverageDisp __fastcall TEMovingAverage::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEMovingAverage::SetSize(long Period, long Width, long Height, TOLEBOOL Internal)
{
  GetDefaultInterface()->SetSize(Period, Width, Height, Internal);
}

void __fastcall TEMovingAverage::GetSize(long* Period, long* Width, long* Height, TOLEBOOL* Internal)
{
  GetDefaultInterface()->GetSize(Period, Width, Height, Internal);
}

void __fastcall TEMovingAverage::Average(LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->Average(BW8DstImage);
}

void __fastcall TEMovingAverage::AverageSrc(LPDISPATCH BW8SrcImage, LPDISPATCH BW8DstImage)
{
  GetDefaultInterface()->AverageSrc(BW8SrcImage, BW8DstImage);
}

void __fastcall TEMovingAverage::Reset(void)
{
  GetDefaultInterface()->Reset();
}

void __fastcall TEMovingAverage::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEMovingAverage::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEMovingAverage::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEMovingAverage::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEMovingAverage::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEColorLookup which
// allows "EColorLookup Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEColorLookup::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E7E, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E7D, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEColorLookup::DEF_CTL_INTF = {0x9AE86E7C, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEColorLookup::OptParam;

static inline void ValidCtrCheck(TEColorLookup *)
{
   delete new TEColorLookup((TComponent*)(0));
};

void __fastcall TEColorLookup::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEColorLookup::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEColorLookupDisp __fastcall TEColorLookup::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEColorLookup::AdjustGainOffset(Evision_tlb::enumColorSystems ColorSystem, 
                                                float Gain0, float Offset0, float Gain1, 
                                                float Offset1, float Gain2, float Offset2)
{
  GetDefaultInterface()->AdjustGainOffset(ColorSystem, Gain0, Offset0, Gain1, Offset1, Gain2, 
                                          Offset2);
}

void __fastcall TEColorLookup::ConvertToRGB(Evision_tlb::enumColorSystems ColorSystem)
{
  GetDefaultInterface()->ConvertToRGB(ColorSystem);
}

void __fastcall TEColorLookup::ConvertFromRGB(Evision_tlb::enumColorSystems ColorSystem)
{
  GetDefaultInterface()->ConvertFromRGB(ColorSystem);
}

void __fastcall TEColorLookup::Calibrate1(short RedValue, short GreenValue, short BlueValue, float X, 
                                          float Y, float Z)
{
  GetDefaultInterface()->Calibrate1(RedValue, GreenValue, BlueValue, X, Y, Z);
}

void __fastcall TEColorLookup::Calibrate3(short RedValue1, short GreenValue1, short BlueValue1, 
                                          float X1, float Y1, float Z1, short RedValue2, 
                                          short GreenValue2, short BlueValue2, float X2, float Y2, 
                                          float Z2, short RedValue3, short GreenValue3, 
                                          short BlueValue3, float X3, float Y3, float Z3)
{
  GetDefaultInterface()->Calibrate3(RedValue1, GreenValue1, BlueValue1, X1, Y1, Z1, RedValue2, 
                                    GreenValue2, BlueValue2, X2, Y2, Z2, RedValue3, GreenValue3, 
                                    BlueValue3, X3, Y3, Z3);
}

void __fastcall TEColorLookup::Calibrate4(short RedValue0, short GreenValue0, short BlueValue0, 
                                          float X0, float Y0, float Z0, short RedValue1, 
                                          short GreenValue1, short BlueValue1, float X1, float Y1, 
                                          float Z1, short RedValue2, short GreenValue2, 
                                          short BlueValue2, float X2, float Y2, float Z2, 
                                          short RedValue3, short GreenValue3, short BlueValue3, 
                                          float X3, float Y3, float Z3)
{
  GetDefaultInterface()->Calibrate4(RedValue0, GreenValue0, BlueValue0, X0, Y0, Z0, RedValue1, 
                                    GreenValue1, BlueValue1, X1, Y1, Z1, RedValue2, GreenValue2, 
                                    BlueValue2, X2, Y2, Z2, RedValue3, GreenValue3, BlueValue3, X3, 
                                    Y3, Z3);
}

void __fastcall TEColorLookup::SetTransform(long Transformation, 
                                            Evision_tlb::enumColorSystems ColorSystemIn, 
                                            Evision_tlb::enumColorSystems ColorSystemOut)
{
  GetDefaultInterface()->SetTransform(Transformation, ColorSystemIn, ColorSystemOut);
}

void __fastcall TEColorLookup::Transform(short SrcRedValue, short SrcGreenValue, short SrcBlueValue, 
                                         short* DstRedValue, short* DstGreenValue, 
                                         short* DstBlueValue)
{
  GetDefaultInterface()->Transform(SrcRedValue, SrcGreenValue, SrcBlueValue, DstRedValue, 
                                   DstGreenValue, DstBlueValue);
}

void __fastcall TEColorLookup::TransformImage(LPDISPATCH C24SrcImage, LPDISPATCH C24DstImage)
{
  GetDefaultInterface()->TransformImage(C24SrcImage, C24DstImage);
}

void __fastcall TEColorLookup::WhiteBalance(float Gain, float Gamma, float BalanceR, float BalanceG, 
                                            float BalanceB)
{
  GetDefaultInterface()->WhiteBalance(Gain, Gamma, BalanceR, BalanceG, BalanceB);
}

void __fastcall TEColorLookup::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEColorLookup::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEColorLookup::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEColorLookup::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEColorLookup::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TECodedImage which
// allows "ECodedImage Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TECodedImage::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E92, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E91, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TECodedImage::DEF_CTL_INTF = {0x9AE86E90, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TECodedImage::OptParam;

static inline void ValidCtrCheck(TECodedImage *)
{
   delete new TECodedImage((TComponent*)(0));
};

void __fastcall TECodedImage::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TECodedImage::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DECodedImageDisp __fastcall TECodedImage::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TECodedImage::SetLowColorThreshold(short Red, short Green, short Blue)
{
  GetDefaultInterface()->SetLowColorThreshold(Red, Green, Blue);
}

void __fastcall TECodedImage::GetLowColorThreshold(short* Red, short* Green, short* Blue)
{
  GetDefaultInterface()->GetLowColorThreshold(Red, Green, Blue);
}

void __fastcall TECodedImage::SetHighColorThreshold(short Red, short Green, short Blue)
{
  GetDefaultInterface()->SetHighColorThreshold(Red, Green, Blue);
}

void __fastcall TECodedImage::GetHighColorThreshold(short* Red, short* Green, short* Blue)
{
  GetDefaultInterface()->GetHighColorThreshold(Red, Green, Blue);
}

void __fastcall TECodedImage::GoToFirstRun(void)
{
  GetDefaultInterface()->GoToFirstRun();
}

void __fastcall TECodedImage::GoToLastRun(void)
{
  GetDefaultInterface()->GoToLastRun();
}

void __fastcall TECodedImage::GoToNextRun(void)
{
  GetDefaultInterface()->GoToNextRun();
}

void __fastcall TECodedImage::GoToPreviousRun(void)
{
  GetDefaultInterface()->GoToPreviousRun();
}

void __fastcall TECodedImage::GetRunData(long* OriginX, long* OriginY, long* Length, long* Class, 
                                         long* RunIndex, long* ObjectIndex)
{
  GetDefaultInterface()->GetRunData(OriginX, OriginY, Length, Class, RunIndex, ObjectIndex);
}

void __fastcall TECodedImage::RemoveAllRuns(void)
{
  GetDefaultInterface()->RemoveAllRuns();
}

void __fastcall TECodedImage::GoToFirstObject(void)
{
  GetDefaultInterface()->GoToFirstObject();
}

void __fastcall TECodedImage::GoToLastObject(void)
{
  GetDefaultInterface()->GoToLastObject();
}

void __fastcall TECodedImage::GoToNextObject(void)
{
  GetDefaultInterface()->GoToNextObject();
}

void __fastcall TECodedImage::GoToPreviousObject(void)
{
  GetDefaultInterface()->GoToPreviousObject();
}

void __fastcall TECodedImage::GoToObjectByIndex(long ObjectIndex)
{
  GetDefaultInterface()->GoToObjectByIndex(ObjectIndex);
}

void __fastcall TECodedImage::GetObjectData(long* Class, long* ObjectIndex, long* NumberOfRuns, 
                                            TOLEBOOL* IsSelected)
{
  GetDefaultInterface()->GetObjectData(Class, ObjectIndex, NumberOfRuns, IsSelected);
}

long __fastcall TECodedImage::GetObjectIndex(void)
{
  return GetDefaultInterface()->GetObjectIndex();
}

void __fastcall TECodedImage::SelectObject(void)
{
  GetDefaultInterface()->SelectObject();
}

void __fastcall TECodedImage::UnselectObject(void)
{
  GetDefaultInterface()->UnselectObject();
}

TOLEBOOL __fastcall TECodedImage::IsObjectSelected(void)
{
  return GetDefaultInterface()->IsObjectSelected();
}

void __fastcall TECodedImage::SelectAllObjects(void)
{
  GetDefaultInterface()->SelectAllObjects();
}

void __fastcall TECodedImage::UnselectAllObjects(void)
{
  GetDefaultInterface()->UnselectAllObjects();
}

void __fastcall TECodedImage::RemoveAllObjects(void)
{
  GetDefaultInterface()->RemoveAllObjects();
}

void __fastcall TECodedImage::GoToFirstObjectRun(void)
{
  GetDefaultInterface()->GoToFirstObjectRun();
}

void __fastcall TECodedImage::GoToLastObjectRun(void)
{
  GetDefaultInterface()->GoToLastObjectRun();
}

void __fastcall TECodedImage::GoToNextObjectRun(void)
{
  GetDefaultInterface()->GoToNextObjectRun();
}

void __fastcall TECodedImage::GoToPreviousObjectRun(void)
{
  GetDefaultInterface()->GoToPreviousObjectRun();
}

long __fastcall TECodedImage::GetNumberOfObjectRuns(void)
{
  return GetDefaultInterface()->GetNumberOfObjectRuns();
}

short __fastcall TECodedImage::GetObjectIntegerFeature(Evision_tlb::enumObjObjectFeatures Feature)
{
  return GetDefaultInterface()->GetObjectIntegerFeature(Feature);
}

long __fastcall TECodedImage::GetObjectLongFeature(Evision_tlb::enumObjObjectFeatures Feature)
{
  return GetDefaultInterface()->GetObjectLongFeature(Feature);
}

float __fastcall TECodedImage::GetObjectSingleFeature(Evision_tlb::enumObjObjectFeatures Feature)
{
  return GetDefaultInterface()->GetObjectSingleFeature(Feature);
}

void __fastcall TECodedImage::BuildRuns(LPDISPATCH SrcImage)
{
  GetDefaultInterface()->BuildRuns(SrcImage);
}

void __fastcall TECodedImage::BuildObjects(LPDISPATCH SrcImage)
{
  GetDefaultInterface()->BuildObjects(SrcImage);
}

void __fastcall TECodedImage::AnalyseObjects(Evision_tlb::enumObjObjectFeatures Feature)
{
  GetDefaultInterface()->AnalyseObjects(Feature);
}

void __fastcall TECodedImage::SelectObjectsUsingIntegerFeature(Evision_tlb::enumObjObjectFeatures Feature, 
                                                               short Min, short Max, 
                                                               Evision_tlb::enumObjSelectOptions Operation)
{
  GetDefaultInterface()->SelectObjectsUsingIntegerFeature(Feature, Min, Max, Operation);
}

void __fastcall TECodedImage::SelectObjectsUsingLongFeature(Evision_tlb::enumObjObjectFeatures Feature, 
                                                            long Min, long Max, 
                                                            Evision_tlb::enumObjSelectOptions Operation)
{
  GetDefaultInterface()->SelectObjectsUsingLongFeature(Feature, Min, Max, Operation);
}

void __fastcall TECodedImage::SelectObjectsUsingSingleFeature(Evision_tlb::enumObjObjectFeatures Feature, 
                                                              float Min, float Max, 
                                                              Evision_tlb::enumObjSelectOptions Operation)
{
  GetDefaultInterface()->SelectObjectsUsingSingleFeature(Feature, Min, Max, Operation);
}

void __fastcall TECodedImage::SelectObjectsUsingPosition(long OriginX, long OriginY, long Width, 
                                                         long Height, 
                                                         Evision_tlb::enumObjSelectByPosition Operation)
{
  GetDefaultInterface()->SelectObjectsUsingPosition(OriginX, OriginY, Width, Height, Operation);
}

void __fastcall TECodedImage::SortObjectsUsingFeature(Evision_tlb::enumObjObjectFeatures Feature, 
                                                      Evision_tlb::enumObjSortOptions Operation)
{
  GetDefaultInterface()->SortObjectsUsingFeature(Feature, Operation);
}

void __fastcall TECodedImage::GetObjectRunData(long* OriginX, long* OriginY, long* Length, 
                                               long* Class, long* RunIndex, long* ObjectIndex)
{
  GetDefaultInterface()->GetObjectRunData(OriginX, OriginY, Length, Class, RunIndex, ObjectIndex);
}

void __fastcall TECodedImage::DrawObject(LPDISPATCH Image)
{
  GetDefaultInterface()->DrawObject(Image);
}

void __fastcall TECodedImage::DrawObjects(LPDISPATCH Image, 
                                          Evision_tlb::enumObjSelectionMode SelectionMode)
{
  GetDefaultInterface()->DrawObjects(Image, SelectionMode);
}

void __fastcall TECodedImage::DrawObjectFeature(LPDISPATCH Image, 
                                                Evision_tlb::enumObjObjectFeatures Feature)
{
  GetDefaultInterface()->DrawObjectFeature(Image, Feature);
}

void __fastcall TECodedImage::DrawObjectsFeature(LPDISPATCH Image, 
                                                 Evision_tlb::enumObjObjectFeatures Feature, 
                                                 Evision_tlb::enumObjSelectionMode SelectionMode)
{
  GetDefaultInterface()->DrawObjectsFeature(Image, Feature, SelectionMode);
}

void __fastcall TECodedImage::BuildLabeledRuns(LPDISPATCH SrcImage)
{
  GetDefaultInterface()->BuildLabeledRuns(SrcImage);
}

void __fastcall TECodedImage::BuildLabeledObjects(LPDISPATCH SrcImage)
{
  GetDefaultInterface()->BuildLabeledObjects(SrcImage);
}

void __fastcall TECodedImage::FeatureAverage(Evision_tlb::enumObjObjectFeatures Feature, 
                                             float* Average)
{
  GetDefaultInterface()->FeatureAverage(Feature, Average);
}

void __fastcall TECodedImage::FeatureVariance(Evision_tlb::enumObjObjectFeatures Feature, 
                                              float* Average, float* Variance)
{
  GetDefaultInterface()->FeatureVariance(Feature, Average, Variance);
}

void __fastcall TECodedImage::FeatureMinimum(Evision_tlb::enumObjObjectFeatures Feature, 
                                             float* Minimum)
{
  GetDefaultInterface()->FeatureMinimum(Feature, Minimum);
}

void __fastcall TECodedImage::FeatureMaximum(Evision_tlb::enumObjObjectFeatures Feature, 
                                             float* Maximum)
{
  GetDefaultInterface()->FeatureMaximum(Feature, Maximum);
}

void __fastcall TECodedImage::ConvexHull(LPDISPATCH PathVector)
{
  GetDefaultInterface()->ConvexHull(PathVector);
}

void __fastcall TECodedImage::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TECodedImage::DrawObjectToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawObjectToDC(hDC);
}

void __fastcall TECodedImage::DrawObjectsToDC(OLE_HANDLE hDC, 
                                              Evision_tlb::enumObjSelectionMode SelectionMode)
{
  GetDefaultInterface()->DrawObjectsToDC(hDC, SelectionMode);
}

void __fastcall TECodedImage::DrawObjectFeatureToDC(OLE_HANDLE hDC, 
                                                    Evision_tlb::enumObjObjectFeatures Feature)
{
  GetDefaultInterface()->DrawObjectFeatureToDC(hDC, Feature);
}

void __fastcall TECodedImage::DrawObjectsFeatureToDC(OLE_HANDLE hDC, 
                                                     Evision_tlb::enumObjObjectFeatures Feature, 
                                                     Evision_tlb::enumObjSelectionMode SelectionMode)
{
  GetDefaultInterface()->DrawObjectsFeatureToDC(hDC, Feature, SelectionMode);
}

void __fastcall TECodedImage::BuildHoles(void)
{
  GetDefaultInterface()->BuildHoles();
}

void __fastcall TECodedImage::SelectHoles(void)
{
  GetDefaultInterface()->SelectHoles();
}

void __fastcall TECodedImage::UnselectHoles(void)
{
  GetDefaultInterface()->UnselectHoles();
}

void __fastcall TECodedImage::SelectObjectHoles(void)
{
  GetDefaultInterface()->SelectObjectHoles();
}

void __fastcall TECodedImage::UnselectObjectHoles(void)
{
  GetDefaultInterface()->UnselectObjectHoles();
}

void __fastcall TECodedImage::GoToHoleParentObject(void)
{
  GetDefaultInterface()->GoToHoleParentObject();
}

TOLEBOOL __fastcall TECodedImage::IsHole(void)
{
  return GetDefaultInterface()->IsHole();
}

void __fastcall TECodedImage::GoToFirstObjectHole(void)
{
  GetDefaultInterface()->GoToFirstObjectHole();
}

void __fastcall TECodedImage::GoToNextHole(void)
{
  GetDefaultInterface()->GoToNextHole();
}

void __fastcall TECodedImage::RemoveHoles(void)
{
  GetDefaultInterface()->RemoveHoles();
}

void __fastcall TECodedImage::RemoveObjectHoles(void)
{
  GetDefaultInterface()->RemoveObjectHoles();
}

void __fastcall TECodedImage::BuildObjectHoles(void)
{
  GetDefaultInterface()->BuildObjectHoles();
}

LPDISPATCH __fastcall TECodedImage::Get_LowImage()
{
  return GetDefaultInterface()->get_LowImage();
}

void __fastcall TECodedImage::Set_LowImage(LPDISPATCH param)
{
  GetDefaultInterface()->set_LowImage(param);
}

LPDISPATCH __fastcall TECodedImage::Get_HighImage()
{
  return GetDefaultInterface()->get_HighImage();
}

void __fastcall TECodedImage::Set_HighImage(LPDISPATCH param)
{
  GetDefaultInterface()->set_HighImage(param);
}

LPDISPATCH __fastcall TECodedImage::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TECodedImage::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TECodedImage::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TECodedImage::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEWorldShape which
// allows "EWorldShape Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEWorldShape::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86ECE, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86ECD, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEWorldShape::DEF_CTL_INTF = {0x9AE86ECC, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEWorldShape::OptParam;

static inline void ValidCtrCheck(TEWorldShape *)
{
   delete new TEWorldShape((TComponent*)(0));
};

void __fastcall TEWorldShape::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEWorldShape::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEWorldShapeDisp __fastcall TEWorldShape::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEWorldShape::EmptyLandmarks(void)
{
  GetDefaultInterface()->EmptyLandmarks();
}

void __fastcall TEWorldShape::AddPoint(float X, float Y)
{
  GetDefaultInterface()->AddPoint(X, Y);
}

void __fastcall TEWorldShape::AddLandmark(float SensorX, float SensorY, float WorldX, float WorldY)
{
  GetDefaultInterface()->AddLandmark(SensorX, SensorY, WorldX, WorldY);
}

void __fastcall TEWorldShape::Calibrate(long un32CalibrationModes)
{
  GetDefaultInterface()->Calibrate(un32CalibrationModes);
}

long __fastcall TEWorldShape::RebuildGrid(float ColumnPitch, float RowPitch, long CenterIndex, 
                                          float WorldCenterX, float WorldCenterY, TOLEBOOL Direct)
{
  return GetDefaultInterface()->RebuildGrid(ColumnPitch, RowPitch, CenterIndex, WorldCenterX, 
                                            WorldCenterY, Direct);
}

void __fastcall TEWorldShape::DrawCrossGrid(LPDISPATCH Img, float fXMin, float XMax, float YMin, 
                                            float YMax, long NumXInt, long NumYInt)
{
  GetDefaultInterface()->DrawCrossGrid(Img, fXMin, XMax, YMin, YMax, NumXInt, NumYInt);
}

void __fastcall TEWorldShape::DrawGrid(LPDISPATCH Image)
{
  GetDefaultInterface()->DrawGrid(Image);
}

void __fastcall TEWorldShape::DrawLandmarks(LPDISPATCH Image)
{
  GetDefaultInterface()->DrawLandmarks(Image);
}

void __fastcall TEWorldShape::HitLandmark(void)
{
  GetDefaultInterface()->HitLandmark();
}

void __fastcall TEWorldShape::DragLandmark(long CursorX, long CursorY)
{
  GetDefaultInterface()->DragLandmark(CursorX, CursorY);
}

void __fastcall TEWorldShape::SetSensorSize(long Width, long Height)
{
  GetDefaultInterface()->SetSensorSize(Width, Height);
}

void __fastcall TEWorldShape::SetFieldSize(float Width, float Height)
{
  GetDefaultInterface()->SetFieldSize(Width, Height);
}

void __fastcall TEWorldShape::SetResolution(float XResolution, float YResolution)
{
  GetDefaultInterface()->SetResolution(XResolution, YResolution);
}

void __fastcall TEWorldShape::SetPerspective(float TiltXAngle, float TiltYAngle, 
                                             float PerspectiveStrength)
{
  GetDefaultInterface()->SetPerspective(TiltXAngle, TiltYAngle, PerspectiveStrength);
}

void __fastcall TEWorldShape::SetDistortion(float DistortionStrength, float DistortionStrength2)
{
  GetDefaultInterface()->SetDistortion(DistortionStrength, DistortionStrength2);
}

void __fastcall TEWorldShape::SetOpticalCenter(float CenterX, float CenterY)
{
  GetDefaultInterface()->SetOpticalCenter(CenterX, CenterY);
}

void __fastcall TEWorldShape::SetSensor(long SensorWidth, long SensorHeight, float FieldWidth, 
                                        float FieldHeight, float CenterX, float CenterY, float Angle, 
                                        float TiltXAngle, float TiltYAngle, 
                                        float PerspectiveStrength, float DistortionStrength, 
                                        float DistortionStrength2, float OpticalCenterX, 
                                        float OpticalCenterY, long RequiredCalibrationModes)
{
  GetDefaultInterface()->SetSensor(SensorWidth, SensorHeight, FieldWidth, FieldHeight, CenterX, 
                                   CenterY, Angle, TiltXAngle, TiltYAngle, PerspectiveStrength, 
                                   DistortionStrength, DistortionStrength2, OpticalCenterX, 
                                   OpticalCenterY, RequiredCalibrationModes);
}

void __fastcall TEWorldShape::Unwarp(LPDISPATCH SrcROI, LPDISPATCH DstRoi, TOLEBOOL Interpolate)
{
  GetDefaultInterface()->Unwarp(SrcROI, DstRoi, Interpolate);
}

void __fastcall TEWorldShape::Draw(LPDISPATCH Image, Evision_tlb::enumInsDrawingMode Mode, 
                                   TOLEBOOL Daughters)
{
  GetDefaultInterface()->Draw(Image, Mode, Daughters);
}

void __fastcall TEWorldShape::UnwarpAfterSetup(LPDISPATCH SrcROI, LPDISPATCH DstRoi, 
                                               TOLEBOOL Interpolate)
{
  GetDefaultInterface()->UnwarpAfterSetup(SrcROI, DstRoi, Interpolate);
}

void __fastcall TEWorldShape::SetupUnwarp(LPDISPATCH SrcROI, TOLEBOOL Interpolate)
{
  GetDefaultInterface()->SetupUnwarp(SrcROI, Interpolate);
}

void __fastcall TEWorldShape::SetAngle(float fAngle, TOLEBOOL bWorld)
{
  GetDefaultInterface()->SetAngle(fAngle, bWorld);
}

void __fastcall TEWorldShape::SensorToWorld(float fSensorPointX, float fSensorPointY, 
                                            float* fWorldPointX, float* fWorldPointY)
{
  GetDefaultInterface()->SensorToWorld(fSensorPointX, fSensorPointY, fWorldPointX, fWorldPointY);
}

void __fastcall TEWorldShape::WorldToSensor(float fWorldPointX, float fWorldPointY, 
                                            float* fSensorPointX, float* fSensorPointY)
{
  GetDefaultInterface()->WorldToSensor(fWorldPointX, fWorldPointY, fSensorPointX, fSensorPointY);
}

void __fastcall TEWorldShape::SetSize(float fSizeX, float fSizeY)
{
  GetDefaultInterface()->SetSize(fSizeX, fSizeY);
}

void __fastcall TEWorldShape::Closest(void)
{
  GetDefaultInterface()->Closest();
}

TOLEBOOL __fastcall TEWorldShape::HitTest(TOLEBOOL bDaughters)
{
  return GetDefaultInterface()->HitTest(bDaughters);
}

void __fastcall TEWorldShape::Drag(long CursorX, long CursorY)
{
  GetDefaultInterface()->Drag(CursorX, CursorY);
}

long __fastcall TEWorldShape::GetHitLandmark(void)
{
  return GetDefaultInterface()->GetHitLandmark();
}

void __fastcall TEWorldShape::SetZoom(float fZoomX, float fZoomY)
{
  GetDefaultInterface()->SetZoom(fZoomX, fZoomY);
}

void __fastcall TEWorldShape::SetPan(float fPanX, float fPanY)
{
  GetDefaultInterface()->SetPan(fPanX, fPanY);
}

void __fastcall TEWorldShape::Process(LPDISPATCH BW8Image, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Process(BW8Image, bDaughters);
}

void __fastcall TEWorldShape::Save(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Save(PathName, bDaughters);
}

void __fastcall TEWorldShape::Load(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Load(PathName, bDaughters);
}

void __fastcall TEWorldShape::SetVisible(TOLEBOOL bVisible, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetVisible(bVisible, bDaughters);
}

void __fastcall TEWorldShape::SetSelectable(TOLEBOOL bSelectable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelectable(bSelectable, bDaughters);
}

void __fastcall TEWorldShape::SetSelected(TOLEBOOL bSelected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelected(bSelected, bDaughters);
}

void __fastcall TEWorldShape::SetDragable(TOLEBOOL bDragable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetDragable(bDragable, bDaughters);
}

void __fastcall TEWorldShape::SetResizable(TOLEBOOL bResizable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetResizable(bResizable, bDaughters);
}

void __fastcall TEWorldShape::SetRotatable(TOLEBOOL bRotatable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetRotatable(bRotatable, bDaughters);
}

void __fastcall TEWorldShape::SetActive(TOLEBOOL bActive, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActive(bActive, bDaughters);
}

void __fastcall TEWorldShape::SetLabeled(TOLEBOOL bLabeled, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetLabeled(bLabeled, bDaughters);
}

void __fastcall TEWorldShape::SetAutoArrange(TOLEBOOL bAutoArrange, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetAutoArrange(bAutoArrange, bDaughters);
}

void __fastcall TEWorldShape::SetFound(TOLEBOOL bFound, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetFound(bFound, bDaughters);
}

void __fastcall TEWorldShape::SetInspected(TOLEBOOL bInspected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetInspected(bInspected, bDaughters);
}

void __fastcall TEWorldShape::SetPassed(TOLEBOOL bPassed, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetPassed(bPassed, bDaughters);
}

void __fastcall TEWorldShape::SetQuickDraw(TOLEBOOL bQuickDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetQuickDraw(bQuickDraw, bDaughters);
}

void __fastcall TEWorldShape::SetOptionalDraw(TOLEBOOL bOptionalDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetOptionalDraw(bOptionalDraw, bDaughters);
}

void __fastcall TEWorldShape::SetUserFlag(TOLEBOOL bUserFlag, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetUserFlag(bUserFlag, bDaughters);
}

void __fastcall TEWorldShape::DisableBehaviorFilter(Evision_tlb::enumInsShapeBehavior Behavior)
{
  GetDefaultInterface()->DisableBehaviorFilter(Behavior);
}

void __fastcall TEWorldShape::EnableBehaviorFilter(Evision_tlb::enumInsShapeBehavior Behavior, 
                                                   TOLEBOOL bValue)
{
  GetDefaultInterface()->EnableBehaviorFilter(Behavior, bValue);
}

void __fastcall TEWorldShape::EnableTypeFilter(long Types)
{
  GetDefaultInterface()->EnableTypeFilter(Types);
}

void __fastcall TEWorldShape::DisableTypeFilter(void)
{
  GetDefaultInterface()->DisableTypeFilter();
}

void __fastcall TEWorldShape::DetachDaughters(void)
{
  GetDefaultInterface()->DetachDaughters();
}

void __fastcall TEWorldShape::SetCenter(float fCenterX, float fCenterY)
{
  GetDefaultInterface()->SetCenter(fCenterX, fCenterY);
}

void __fastcall TEWorldShape::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEWorldShape::DrawCrossGridToDC(OLE_HANDLE hDC, float f32XMin, float f32XMax, 
                                                float f32YMin, float f32YMax, long NumXIntervals, 
                                                long NumYIntervals)
{
  GetDefaultInterface()->DrawCrossGridToDC(hDC, f32XMin, f32XMax, f32YMin, f32YMax, NumXIntervals, 
                                           NumYIntervals);
}

void __fastcall TEWorldShape::DrawToDC(OLE_HANDLE hDC, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                       TOLEBOOL bDaughters)
{
  GetDefaultInterface()->DrawToDC(hDC, DrawingMode, bDaughters);
}

void __fastcall TEWorldShape::DrawGridToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawGridToDC(hDC);
}

void __fastcall TEWorldShape::DrawLandmarksToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawLandmarksToDC(hDC);
}

long __fastcall TEWorldShape::GetShapeNamed(BSTR lpszSearchedValue)
{
  return GetDefaultInterface()->GetShapeNamed(lpszSearchedValue);
}

void __fastcall TEWorldShape::SetCursor(long n32X, long n32Y)
{
  GetDefaultInterface()->SetCursor(n32X, n32Y);
}

void __fastcall TEWorldShape::DragHitShape(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->DragHitShape(nCursorX, nCursorY);
}

long __fastcall TEWorldShape::AutoCalibrateDotGrid(LPDISPATCH sourceImage, float ColumnPitch, 
                                                   float RowPitch, TOLEBOOL testEmpiricalModes)
{
  return GetDefaultInterface()->AutoCalibrateDotGrid(sourceImage, ColumnPitch, RowPitch, 
                                                     testEmpiricalModes);
}

long __fastcall TEWorldShape::Get_Daughter(long Index)
{
  return GetDefaultInterface()->get_Daughter(Index);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEFrameShape which
// allows "EFrameShape Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEFrameShape::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EE6, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EE5, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEFrameShape::DEF_CTL_INTF = {0x9AE86EE4, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEFrameShape::OptParam;

static inline void ValidCtrCheck(TEFrameShape *)
{
   delete new TEFrameShape((TComponent*)(0));
};

void __fastcall TEFrameShape::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEFrameShape::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEFrameShapeDisp __fastcall TEFrameShape::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEFrameShape::SetCenter(float fCenterX, float fCenterY)
{
  GetDefaultInterface()->SetCenter(fCenterX, fCenterY);
}

void __fastcall TEFrameShape::Process(LPDISPATCH BW8Image, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Process(BW8Image, bDaughters);
}

void __fastcall TEFrameShape::Draw(LPDISPATCH BW8Image, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                   TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Draw(BW8Image, DrawingMode, bDaughters);
}

TOLEBOOL __fastcall TEFrameShape::HitTest(TOLEBOOL bDaughters)
{
  return GetDefaultInterface()->HitTest(bDaughters);
}

void __fastcall TEFrameShape::Drag(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->Drag(nCursorX, nCursorY);
}

void __fastcall TEFrameShape::Closest(void)
{
  GetDefaultInterface()->Closest();
}

void __fastcall TEFrameShape::SetVisible(TOLEBOOL bVisible, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetVisible(bVisible, bDaughters);
}

void __fastcall TEFrameShape::SetSelected(TOLEBOOL bSelected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelected(bSelected, bDaughters);
}

void __fastcall TEFrameShape::SetSelectable(TOLEBOOL bSelectable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelectable(bSelectable, bDaughters);
}

void __fastcall TEFrameShape::SetDragable(TOLEBOOL bDragable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetDragable(bDragable, bDaughters);
}

void __fastcall TEFrameShape::SetResizable(TOLEBOOL bResizable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetResizable(bResizable, bDaughters);
}

void __fastcall TEFrameShape::SetRotatable(TOLEBOOL bRotatable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetRotatable(bRotatable, bDaughters);
}

void __fastcall TEFrameShape::SetActive(TOLEBOOL bActive, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActive(bActive, bDaughters);
}

void __fastcall TEFrameShape::SetLabeled(TOLEBOOL bLabeled, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetLabeled(bLabeled, bDaughters);
}

void __fastcall TEFrameShape::SetAutoArrange(TOLEBOOL bAutoArrange, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetAutoArrange(bAutoArrange, bDaughters);
}

void __fastcall TEFrameShape::SetFound(TOLEBOOL bFound, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetFound(bFound, bDaughters);
}

void __fastcall TEFrameShape::SetInspected(TOLEBOOL bInspected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetInspected(bInspected, bDaughters);
}

void __fastcall TEFrameShape::SetPassed(TOLEBOOL bPassed, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetPassed(bPassed, bDaughters);
}

void __fastcall TEFrameShape::SetQuickDraw(TOLEBOOL bQuickDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetQuickDraw(bQuickDraw, bDaughters);
}

void __fastcall TEFrameShape::SetOptionalDraw(TOLEBOOL bOptionalDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetOptionalDraw(bOptionalDraw, bDaughters);
}

void __fastcall TEFrameShape::SetUserFlag(TOLEBOOL bUserFlag, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetUserFlag(bUserFlag, bDaughters);
}

void __fastcall TEFrameShape::Attach(long Mother)
{
  GetDefaultInterface()->Attach(Mother);
}

void __fastcall TEFrameShape::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TEFrameShape::DetachDaughters(void)
{
  GetDefaultInterface()->DetachDaughters();
}

void __fastcall TEFrameShape::Set(float fCenterX, float fCenterY, float fAngle, float fScale)
{
  GetDefaultInterface()->Set(fCenterX, fCenterY, fAngle, fScale);
}

void __fastcall TEFrameShape::SetCursor(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->SetCursor(nCursorX, nCursorY);
}

void __fastcall TEFrameShape::Save(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Save(PathName, bDaughters);
}

void __fastcall TEFrameShape::Load(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Load(PathName, bDaughters);
}

void __fastcall TEFrameShape::SetActualShape(TOLEBOOL bActualShape, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActualShape(bActualShape, bDaughters);
}

void __fastcall TEFrameShape::SetAxis(float fCenterX, float fCenterY, float fAngle, float fScale, 
                                      float fSizeX, float fSizeY)
{
  GetDefaultInterface()->SetAxis(fCenterX, fCenterY, fAngle, fScale, fSizeX, fSizeY);
}

void __fastcall TEFrameShape::SetSize(float fSizeX, float fSizeY)
{
  GetDefaultInterface()->SetSize(fSizeX, fSizeY);
}

void __fastcall TEFrameShape::DrawCrossGrid(LPDISPATCH BW8Image, float fXMin, float fXMax, 
                                            float fYMin, float fYMax, long NumXIntervals, 
                                            long NumYIntervals)
{
  GetDefaultInterface()->DrawCrossGrid(BW8Image, fXMin, fXMax, fYMin, fYMax, NumXIntervals, 
                                       NumYIntervals);
}

void __fastcall TEFrameShape::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEFrameShape::DrawToDC(OLE_HANDLE hDC, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                       TOLEBOOL bDaughters)
{
  GetDefaultInterface()->DrawToDC(hDC, DrawingMode, bDaughters);
}

void __fastcall TEFrameShape::DrawCrossGridToDC(OLE_HANDLE hDC, float fXMin, float fXMax, 
                                                float fYMin, float fYMax, long NumXIntervals, 
                                                long NumYIntervals)
{
  GetDefaultInterface()->DrawCrossGridToDC(hDC, fXMin, fXMax, fYMin, fYMax, NumXIntervals, 
                                           NumYIntervals);
}

long __fastcall TEFrameShape::GetShapeNamed(BSTR lpszSearchedValue)
{
  return GetDefaultInterface()->GetShapeNamed(lpszSearchedValue);
}

void __fastcall TEFrameShape::DragHitShape(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->DragHitShape(nCursorX, nCursorY);
}

long __fastcall TEFrameShape::Get_Daughter(long Index)
{
  return GetDefaultInterface()->get_Daughter(Index);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEPointGauge which
// allows "EPointGauge Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEPointGauge::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86ED2, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86ED1, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEPointGauge::DEF_CTL_INTF = {0x9AE86ED0, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEPointGauge::OptParam;

static inline void ValidCtrCheck(TEPointGauge *)
{
   delete new TEPointGauge((TComponent*)(0));
};

void __fastcall TEPointGauge::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEPointGauge::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEPointGaugeDisp __fastcall TEPointGauge::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEPointGauge::SetTolerance(float fTolerance, float fToleranceAngle)
{
  GetDefaultInterface()->SetTolerance(fTolerance, fToleranceAngle);
}

void __fastcall TEPointGauge::Plot(long hDC, Evision_tlb::enumGgePlotItems DrawItems, float fWidth, 
                                   float fHeight, float fOrgX, float fOrgY)
{
  GetDefaultInterface()->Plot(hDC, DrawItems, fWidth, fHeight, fOrgX, fOrgY);
}

void __fastcall TEPointGauge::SetCenter(float fCenterX, float fCenterY)
{
  GetDefaultInterface()->SetCenter(fCenterX, fCenterY);
}

void __fastcall TEPointGauge::Measure(LPDISPATCH BW8Image)
{
  GetDefaultInterface()->Measure(BW8Image);
}

void __fastcall TEPointGauge::Process(LPDISPATCH BW8Image, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Process(BW8Image, bDaughters);
}

void __fastcall TEPointGauge::Draw(LPDISPATCH BW8Image, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                   TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Draw(BW8Image, DrawingMode, bDaughters);
}

TOLEBOOL __fastcall TEPointGauge::HitTest(TOLEBOOL bDaughters)
{
  return GetDefaultInterface()->HitTest(bDaughters);
}

void __fastcall TEPointGauge::Drag(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->Drag(nCursorX, nCursorY);
}

void __fastcall TEPointGauge::Closest(void)
{
  GetDefaultInterface()->Closest();
}

void __fastcall TEPointGauge::SetVisible(TOLEBOOL bVisible, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetVisible(bVisible, bDaughters);
}

void __fastcall TEPointGauge::SetSelected(TOLEBOOL bSelected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelected(bSelected, bDaughters);
}

void __fastcall TEPointGauge::SetSelectable(TOLEBOOL bSelectable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelectable(bSelectable, bDaughters);
}

void __fastcall TEPointGauge::SetDragable(TOLEBOOL bDragable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetDragable(bDragable, bDaughters);
}

void __fastcall TEPointGauge::SetResizable(TOLEBOOL bResizable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetResizable(bResizable, bDaughters);
}

void __fastcall TEPointGauge::SetRotatable(TOLEBOOL bRotatable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetRotatable(bRotatable, bDaughters);
}

void __fastcall TEPointGauge::SetActive(TOLEBOOL bActive, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActive(bActive, bDaughters);
}

void __fastcall TEPointGauge::SetLabeled(TOLEBOOL bLabeled, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetLabeled(bLabeled, bDaughters);
}

void __fastcall TEPointGauge::SetAutoArrange(TOLEBOOL bAutoArrange, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetAutoArrange(bAutoArrange, bDaughters);
}

void __fastcall TEPointGauge::SetFound(TOLEBOOL bFound, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetFound(bFound, bDaughters);
}

void __fastcall TEPointGauge::SetInspected(TOLEBOOL bInspected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetInspected(bInspected, bDaughters);
}

void __fastcall TEPointGauge::SetPassed(TOLEBOOL bPassed, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetPassed(bPassed, bDaughters);
}

void __fastcall TEPointGauge::SetQuickDraw(TOLEBOOL bQuickDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetQuickDraw(bQuickDraw, bDaughters);
}

void __fastcall TEPointGauge::SetOptionalDraw(TOLEBOOL bOptionalDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetOptionalDraw(bOptionalDraw, bDaughters);
}

void __fastcall TEPointGauge::SetUserFlag(TOLEBOOL bUserFlag, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetUserFlag(bUserFlag, bDaughters);
}

void __fastcall TEPointGauge::Attach(long Mother)
{
  GetDefaultInterface()->Attach(Mother);
}

void __fastcall TEPointGauge::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TEPointGauge::DetachDaughters(void)
{
  GetDefaultInterface()->DetachDaughters();
}

void __fastcall TEPointGauge::Set(float fCenterX, float fCenterY, float fAngle, float fScale)
{
  GetDefaultInterface()->Set(fCenterX, fCenterY, fAngle, fScale);
}

void __fastcall TEPointGauge::SetCursor(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->SetCursor(nCursorX, nCursorY);
}

void __fastcall TEPointGauge::Save(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Save(PathName, bDaughters);
}

void __fastcall TEPointGauge::Load(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Load(PathName, bDaughters);
}

void __fastcall TEPointGauge::SetActualShape(TOLEBOOL bActualShape, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActualShape(bActualShape, bDaughters);
}

void __fastcall TEPointGauge::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEPointGauge::DrawToDC(OLE_HANDLE hDC, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                       TOLEBOOL bDaughters)
{
  GetDefaultInterface()->DrawToDC(hDC, DrawingMode, bDaughters);
}

long __fastcall TEPointGauge::GetShapeNamed(BSTR lpszSearchedValue)
{
  return GetDefaultInterface()->GetShapeNamed(lpszSearchedValue);
}

void __fastcall TEPointGauge::DragHitShape(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->DragHitShape(nCursorX, nCursorY);
}

float __fastcall TEPointGauge::Get_PointXAt(long Index)
{
  return GetDefaultInterface()->get_PointXAt(Index);
}

float __fastcall TEPointGauge::Get_PointYAt(long Index)
{
  return GetDefaultInterface()->get_PointYAt(Index);
}

long __fastcall TEPointGauge::Get_Daughter(long Index)
{
  return GetDefaultInterface()->get_Daughter(Index);
}

long __fastcall TEPointGauge::Get_PeakStartAt(long Index)
{
  return GetDefaultInterface()->get_PeakStartAt(Index);
}

void __fastcall TEPointGauge::Set_PeakStartAt(long Index, long Param2)
{
  GetDefaultInterface()->set_PeakStartAt(Index, Param2);
}

long __fastcall TEPointGauge::Get_PeakLengthAt(long Index)
{
  return GetDefaultInterface()->get_PeakLengthAt(Index);
}

void __fastcall TEPointGauge::Set_PeakLengthAt(long Index, long Param2)
{
  GetDefaultInterface()->set_PeakLengthAt(Index, Param2);
}

long __fastcall TEPointGauge::Get_PeakAmplitudeAt(long Index)
{
  return GetDefaultInterface()->get_PeakAmplitudeAt(Index);
}

void __fastcall TEPointGauge::Set_PeakAmplitudeAt(long Index, long Param2)
{
  GetDefaultInterface()->set_PeakAmplitudeAt(Index, Param2);
}

long __fastcall TEPointGauge::Get_PeakAreaAt(long Index)
{
  return GetDefaultInterface()->get_PeakAreaAt(Index);
}

void __fastcall TEPointGauge::Set_PeakAreaAt(long Index, long Param2)
{
  GetDefaultInterface()->set_PeakAreaAt(Index, Param2);
}

float __fastcall TEPointGauge::Get_PeakCenterAt(long Index)
{
  return GetDefaultInterface()->get_PeakCenterAt(Index);
}

void __fastcall TEPointGauge::Set_PeakCenterAt(long Index, float Param2)
{
  GetDefaultInterface()->set_PeakCenterAt(Index, Param2);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TELineGauge which
// allows "ELineGauge Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TELineGauge::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86ED6, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86ED5, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TELineGauge::DEF_CTL_INTF = {0x9AE86ED4, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TELineGauge::OptParam;

static inline void ValidCtrCheck(TELineGauge *)
{
   delete new TELineGauge((TComponent*)(0));
};

void __fastcall TELineGauge::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TELineGauge::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DELineGaugeDisp __fastcall TELineGauge::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TELineGauge::Plot(long hDC, Evision_tlb::enumGgePlotItems DrawItems, float fWidth, 
                                  float fHeight, float fOrgX, float fOrgY)
{
  GetDefaultInterface()->Plot(hDC, DrawItems, fWidth, fHeight, fOrgX, fOrgY);
}

void __fastcall TELineGauge::SetCenter(float fCenterX, float fCenterY)
{
  GetDefaultInterface()->SetCenter(fCenterX, fCenterY);
}

void __fastcall TELineGauge::Measure(LPDISPATCH BW8Image)
{
  GetDefaultInterface()->Measure(BW8Image);
}

void __fastcall TELineGauge::Process(LPDISPATCH BW8Image, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Process(BW8Image, bDaughters);
}

void __fastcall TELineGauge::Draw(LPDISPATCH BW8Image, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                  TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Draw(BW8Image, DrawingMode, bDaughters);
}

TOLEBOOL __fastcall TELineGauge::HitTest(TOLEBOOL bDaughters)
{
  return GetDefaultInterface()->HitTest(bDaughters);
}

void __fastcall TELineGauge::Drag(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->Drag(nCursorX, nCursorY);
}

void __fastcall TELineGauge::Closest(void)
{
  GetDefaultInterface()->Closest();
}

void __fastcall TELineGauge::SetVisible(TOLEBOOL bVisible, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetVisible(bVisible, bDaughters);
}

void __fastcall TELineGauge::SetSelected(TOLEBOOL bSelected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelected(bSelected, bDaughters);
}

void __fastcall TELineGauge::SetSelectable(TOLEBOOL bSelectable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelectable(bSelectable, bDaughters);
}

void __fastcall TELineGauge::SetDragable(TOLEBOOL bDragable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetDragable(bDragable, bDaughters);
}

void __fastcall TELineGauge::SetResizable(TOLEBOOL bResizable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetResizable(bResizable, bDaughters);
}

void __fastcall TELineGauge::SetRotatable(TOLEBOOL bRotatable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetRotatable(bRotatable, bDaughters);
}

void __fastcall TELineGauge::SetActive(TOLEBOOL bActive, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActive(bActive, bDaughters);
}

void __fastcall TELineGauge::SetLabeled(TOLEBOOL bLabeled, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetLabeled(bLabeled, bDaughters);
}

void __fastcall TELineGauge::SetAutoArrange(TOLEBOOL bAutoArrange, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetAutoArrange(bAutoArrange, bDaughters);
}

void __fastcall TELineGauge::SetFound(TOLEBOOL bFound, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetFound(bFound, bDaughters);
}

void __fastcall TELineGauge::SetInspected(TOLEBOOL bInspected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetInspected(bInspected, bDaughters);
}

void __fastcall TELineGauge::SetPassed(TOLEBOOL bPassed, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetPassed(bPassed, bDaughters);
}

void __fastcall TELineGauge::SetQuickDraw(TOLEBOOL bQuickDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetQuickDraw(bQuickDraw, bDaughters);
}

void __fastcall TELineGauge::SetOptionalDraw(TOLEBOOL bOptionalDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetOptionalDraw(bOptionalDraw, bDaughters);
}

void __fastcall TELineGauge::SetUserFlag(TOLEBOOL bUserFlag, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetUserFlag(bUserFlag, bDaughters);
}

void __fastcall TELineGauge::Attach(long Mother)
{
  GetDefaultInterface()->Attach(Mother);
}

void __fastcall TELineGauge::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TELineGauge::DetachDaughters(void)
{
  GetDefaultInterface()->DetachDaughters();
}

void __fastcall TELineGauge::SetCursor(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->SetCursor(nCursorX, nCursorY);
}

void __fastcall TELineGauge::Save(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Save(PathName, bDaughters);
}

void __fastcall TELineGauge::Load(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Load(PathName, bDaughters);
}

void __fastcall TELineGauge::Set(float fCenterX, float fCenterY, float fLength, float fAngle)
{
  GetDefaultInterface()->Set(fCenterX, fCenterY, fLength, fAngle);
}

void __fastcall TELineGauge::MeasureSample(LPDISPATCH BW8Image, long Index)
{
  GetDefaultInterface()->MeasureSample(BW8Image, Index);
}

void __fastcall TELineGauge::SetActualShape(TOLEBOOL bActualShape, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActualShape(bActualShape, bDaughters);
}

void __fastcall TELineGauge::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TELineGauge::DrawToDC(OLE_HANDLE hDC, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                      TOLEBOOL bDaughters)
{
  GetDefaultInterface()->DrawToDC(hDC, DrawingMode, bDaughters);
}

long __fastcall TELineGauge::GetShapeNamed(BSTR lpszSearchedValue)
{
  return GetDefaultInterface()->GetShapeNamed(lpszSearchedValue);
}

long __fastcall TELineGauge::SamplePointValid(long nIndex)
{
  return GetDefaultInterface()->SamplePointValid(nIndex);
}

float __fastcall TELineGauge::SamplePointX(long nIndex)
{
  return GetDefaultInterface()->SamplePointX(nIndex);
}

float __fastcall TELineGauge::SamplePointY(long nIndex)
{
  return GetDefaultInterface()->SamplePointY(nIndex);
}

void __fastcall TELineGauge::DragHitShape(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->DragHitShape(nCursorX, nCursorY);
}

long __fastcall TELineGauge::Get_Daughter(long Index)
{
  return GetDefaultInterface()->get_Daughter(Index);
}

float __fastcall TELineGauge::Get_PointY(float fFraction)
{
  return GetDefaultInterface()->get_PointY(fFraction);
}

float __fastcall TELineGauge::Get_PointX(float fFraction)
{
  return GetDefaultInterface()->get_PointX(fFraction);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TECircleGauge which
// allows "ECircleGauge Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TECircleGauge::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EDA, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86ED9, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TECircleGauge::DEF_CTL_INTF = {0x9AE86ED8, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TECircleGauge::OptParam;

static inline void ValidCtrCheck(TECircleGauge *)
{
   delete new TECircleGauge((TComponent*)(0));
};

void __fastcall TECircleGauge::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TECircleGauge::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DECircleGaugeDisp __fastcall TECircleGauge::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TECircleGauge::Plot(long hDC, Evision_tlb::enumGgePlotItems DrawItems, float fWidth, 
                                    float fHeight, float fOrgX, float fOrgY)
{
  GetDefaultInterface()->Plot(hDC, DrawItems, fWidth, fHeight, fOrgX, fOrgY);
}

void __fastcall TECircleGauge::SetCenter(float fCenterX, float fCenterY)
{
  GetDefaultInterface()->SetCenter(fCenterX, fCenterY);
}

void __fastcall TECircleGauge::Measure(LPDISPATCH BW8Image)
{
  GetDefaultInterface()->Measure(BW8Image);
}

void __fastcall TECircleGauge::Process(LPDISPATCH BW8Image, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Process(BW8Image, bDaughters);
}

void __fastcall TECircleGauge::Draw(LPDISPATCH BW8Image, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                    TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Draw(BW8Image, DrawingMode, bDaughters);
}

TOLEBOOL __fastcall TECircleGauge::HitTest(TOLEBOOL bDaughters)
{
  return GetDefaultInterface()->HitTest(bDaughters);
}

void __fastcall TECircleGauge::Drag(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->Drag(nCursorX, nCursorY);
}

void __fastcall TECircleGauge::Closest(void)
{
  GetDefaultInterface()->Closest();
}

void __fastcall TECircleGauge::SetVisible(TOLEBOOL bVisible, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetVisible(bVisible, bDaughters);
}

void __fastcall TECircleGauge::SetSelected(TOLEBOOL bSelected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelected(bSelected, bDaughters);
}

void __fastcall TECircleGauge::SetSelectable(TOLEBOOL bSelectable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelectable(bSelectable, bDaughters);
}

void __fastcall TECircleGauge::SetDragable(TOLEBOOL bDragable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetDragable(bDragable, bDaughters);
}

void __fastcall TECircleGauge::SetResizable(TOLEBOOL bResizable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetResizable(bResizable, bDaughters);
}

void __fastcall TECircleGauge::SetRotatable(TOLEBOOL bRotatable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetRotatable(bRotatable, bDaughters);
}

void __fastcall TECircleGauge::SetActive(TOLEBOOL bActive, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActive(bActive, bDaughters);
}

void __fastcall TECircleGauge::SetLabeled(TOLEBOOL bLabeled, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetLabeled(bLabeled, bDaughters);
}

void __fastcall TECircleGauge::SetAutoArrange(TOLEBOOL bAutoArrange, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetAutoArrange(bAutoArrange, bDaughters);
}

void __fastcall TECircleGauge::SetFound(TOLEBOOL bFound, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetFound(bFound, bDaughters);
}

void __fastcall TECircleGauge::SetInspected(TOLEBOOL bInspected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetInspected(bInspected, bDaughters);
}

void __fastcall TECircleGauge::SetPassed(TOLEBOOL bPassed, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetPassed(bPassed, bDaughters);
}

void __fastcall TECircleGauge::SetQuickDraw(TOLEBOOL bQuickDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetQuickDraw(bQuickDraw, bDaughters);
}

void __fastcall TECircleGauge::SetOptionalDraw(TOLEBOOL bOptionalDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetOptionalDraw(bOptionalDraw, bDaughters);
}

void __fastcall TECircleGauge::SetUserFlag(TOLEBOOL bUserFlag, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetUserFlag(bUserFlag, bDaughters);
}

void __fastcall TECircleGauge::Attach(long Mother)
{
  GetDefaultInterface()->Attach(Mother);
}

void __fastcall TECircleGauge::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TECircleGauge::DetachDaughters(void)
{
  GetDefaultInterface()->DetachDaughters();
}

void __fastcall TECircleGauge::SetCursor(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->SetCursor(nCursorX, nCursorY);
}

void __fastcall TECircleGauge::Save(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Save(PathName, bDaughters);
}

void __fastcall TECircleGauge::Load(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Load(PathName, bDaughters);
}

void __fastcall TECircleGauge::MeasureSample(LPDISPATCH BW8Image, long Index)
{
  GetDefaultInterface()->MeasureSample(BW8Image, Index);
}

void __fastcall TECircleGauge::Set(float fCenterX, float fCenterY, float fDiameter, float fOrgAngle, 
                                   float fAmplitude)
{
  GetDefaultInterface()->Set(fCenterX, fCenterY, fDiameter, fOrgAngle, fAmplitude);
}

void __fastcall TECircleGauge::SetActualShape(TOLEBOOL bActualShape, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActualShape(bActualShape, bDaughters);
}

void __fastcall TECircleGauge::SetFull(float fCenterX, float fCenterY, float fDiameter, 
                                       float fOrgAngle, TOLEBOOL bDirect)
{
  GetDefaultInterface()->SetFull(fCenterX, fCenterY, fDiameter, fOrgAngle, bDirect);
}

void __fastcall TECircleGauge::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TECircleGauge::DrawToDC(OLE_HANDLE hDC, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                        TOLEBOOL bDaughters)
{
  GetDefaultInterface()->DrawToDC(hDC, DrawingMode, bDaughters);
}

long __fastcall TECircleGauge::GetShapeNamed(BSTR lpszSearchedValue)
{
  return GetDefaultInterface()->GetShapeNamed(lpszSearchedValue);
}

long __fastcall TECircleGauge::SamplePointValid(long nIndex)
{
  return GetDefaultInterface()->SamplePointValid(nIndex);
}

float __fastcall TECircleGauge::SamplePointX(long nIndex)
{
  return GetDefaultInterface()->SamplePointX(nIndex);
}

float __fastcall TECircleGauge::SamplePointY(long nIndex)
{
  return GetDefaultInterface()->SamplePointY(nIndex);
}

void __fastcall TECircleGauge::DragHitShape(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->DragHitShape(nCursorX, nCursorY);
}

long __fastcall TECircleGauge::Get_Daughter(long Index)
{
  return GetDefaultInterface()->get_Daughter(Index);
}

float __fastcall TECircleGauge::Get_PointX(float fFraction)
{
  return GetDefaultInterface()->get_PointX(fFraction);
}

float __fastcall TECircleGauge::Get_PointY(float fFraction)
{
  return GetDefaultInterface()->get_PointY(fFraction);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TERectangleGauge which
// allows "ERectangleGauge Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TERectangleGauge::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EDE, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EDD, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TERectangleGauge::DEF_CTL_INTF = {0x9AE86EDC, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TERectangleGauge::OptParam;

static inline void ValidCtrCheck(TERectangleGauge *)
{
   delete new TERectangleGauge((TComponent*)(0));
};

void __fastcall TERectangleGauge::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TERectangleGauge::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DERectangleGaugeDisp __fastcall TERectangleGauge::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TERectangleGauge::Plot(long hDC, Evision_tlb::enumGgePlotItems DrawItems, 
                                       float fWidth, float fHeight, float fOrgX, float fOrgY)
{
  GetDefaultInterface()->Plot(hDC, DrawItems, fWidth, fHeight, fOrgX, fOrgY);
}

void __fastcall TERectangleGauge::SetCenter(float fCenterX, float fCenterY)
{
  GetDefaultInterface()->SetCenter(fCenterX, fCenterY);
}

void __fastcall TERectangleGauge::Measure(LPDISPATCH BW8Image)
{
  GetDefaultInterface()->Measure(BW8Image);
}

void __fastcall TERectangleGauge::Process(LPDISPATCH BW8Image, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Process(BW8Image, bDaughters);
}

void __fastcall TERectangleGauge::Draw(LPDISPATCH BW8Image, 
                                       Evision_tlb::enumInsDrawingMode DrawingMode, 
                                       TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Draw(BW8Image, DrawingMode, bDaughters);
}

TOLEBOOL __fastcall TERectangleGauge::HitTest(TOLEBOOL bDaughters)
{
  return GetDefaultInterface()->HitTest(bDaughters);
}

void __fastcall TERectangleGauge::Drag(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->Drag(nCursorX, nCursorY);
}

void __fastcall TERectangleGauge::Closest(void)
{
  GetDefaultInterface()->Closest();
}

void __fastcall TERectangleGauge::SetVisible(TOLEBOOL bVisible, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetVisible(bVisible, bDaughters);
}

void __fastcall TERectangleGauge::SetSelected(TOLEBOOL bSelected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelected(bSelected, bDaughters);
}

void __fastcall TERectangleGauge::SetSelectable(TOLEBOOL bSelectable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelectable(bSelectable, bDaughters);
}

void __fastcall TERectangleGauge::SetDragable(TOLEBOOL bDragable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetDragable(bDragable, bDaughters);
}

void __fastcall TERectangleGauge::SetResizable(TOLEBOOL bResizable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetResizable(bResizable, bDaughters);
}

void __fastcall TERectangleGauge::SetRotatable(TOLEBOOL bRotatable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetRotatable(bRotatable, bDaughters);
}

void __fastcall TERectangleGauge::SetActive(TOLEBOOL bActive, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActive(bActive, bDaughters);
}

void __fastcall TERectangleGauge::SetLabeled(TOLEBOOL bLabeled, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetLabeled(bLabeled, bDaughters);
}

void __fastcall TERectangleGauge::SetAutoArrange(TOLEBOOL bAutoArrange, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetAutoArrange(bAutoArrange, bDaughters);
}

void __fastcall TERectangleGauge::SetFound(TOLEBOOL bFound, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetFound(bFound, bDaughters);
}

void __fastcall TERectangleGauge::SetInspected(TOLEBOOL bInspected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetInspected(bInspected, bDaughters);
}

void __fastcall TERectangleGauge::SetPassed(TOLEBOOL bPassed, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetPassed(bPassed, bDaughters);
}

void __fastcall TERectangleGauge::SetQuickDraw(TOLEBOOL bQuickDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetQuickDraw(bQuickDraw, bDaughters);
}

void __fastcall TERectangleGauge::SetOptionalDraw(TOLEBOOL bOptionalDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetOptionalDraw(bOptionalDraw, bDaughters);
}

void __fastcall TERectangleGauge::SetUserFlag(TOLEBOOL bUserFlag, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetUserFlag(bUserFlag, bDaughters);
}

void __fastcall TERectangleGauge::Attach(long Mother)
{
  GetDefaultInterface()->Attach(Mother);
}

void __fastcall TERectangleGauge::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TERectangleGauge::DetachDaughters(void)
{
  GetDefaultInterface()->DetachDaughters();
}

void __fastcall TERectangleGauge::SetCursor(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->SetCursor(nCursorX, nCursorY);
}

void __fastcall TERectangleGauge::Save(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Save(PathName, bDaughters);
}

void __fastcall TERectangleGauge::Load(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Load(PathName, bDaughters);
}

void __fastcall TERectangleGauge::MeasureSample(LPDISPATCH BW8Image, long Index)
{
  GetDefaultInterface()->MeasureSample(BW8Image, Index);
}

void __fastcall TERectangleGauge::SetActualShape(TOLEBOOL bActualShape, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActualShape(bActualShape, bDaughters);
}

void __fastcall TERectangleGauge::SetSize(float fSizeX, float fSizeY)
{
  GetDefaultInterface()->SetSize(fSizeX, fSizeY);
}

void __fastcall TERectangleGauge::Set(float fCenterX, float fCenterY, float fSizeX, float fSizeY, 
                                      float fAngle)
{
  GetDefaultInterface()->Set(fCenterX, fCenterY, fSizeX, fSizeY, fAngle);
}

void __fastcall TERectangleGauge::SetBox(float fOrgX, float fOrgY, float fEndX, float fEndY)
{
  GetDefaultInterface()->SetBox(fOrgX, fOrgY, fEndX, fEndY);
}

void __fastcall TERectangleGauge::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TERectangleGauge::DrawToDC(OLE_HANDLE hDC, 
                                           Evision_tlb::enumInsDrawingMode DrawingMode, 
                                           TOLEBOOL bDaughters)
{
  GetDefaultInterface()->DrawToDC(hDC, DrawingMode, bDaughters);
}

long __fastcall TERectangleGauge::GetShapeNamed(BSTR lpszSearchedValue)
{
  return GetDefaultInterface()->GetShapeNamed(lpszSearchedValue);
}

long __fastcall TERectangleGauge::SamplePointValid(long nIndex)
{
  return GetDefaultInterface()->SamplePointValid(nIndex);
}

float __fastcall TERectangleGauge::SamplePointX(long nIndex)
{
  return GetDefaultInterface()->SamplePointX(nIndex);
}

float __fastcall TERectangleGauge::SamplePointY(long nIndex)
{
  return GetDefaultInterface()->SamplePointY(nIndex);
}

void __fastcall TERectangleGauge::DragHitShape(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->DragHitShape(nCursorX, nCursorY);
}

long __fastcall TERectangleGauge::Get_Daughter(long Index)
{
  return GetDefaultInterface()->get_Daughter(Index);
}

float __fastcall TERectangleGauge::Get_PointX(float fFractionX, float fFractionY)
{
  return GetDefaultInterface()->get_PointX(fFractionX, fFractionY);
}

float __fastcall TERectangleGauge::Get_PointY(float fFractionX, float fFractionY)
{
  return GetDefaultInterface()->get_PointY(fFractionX, fFractionY);
}

float __fastcall TERectangleGauge::Get_XEdgePointX(float fFraction)
{
  return GetDefaultInterface()->get_XEdgePointX(fFraction);
}

float __fastcall TERectangleGauge::Get_XEdgePointY(float fFraction)
{
  return GetDefaultInterface()->get_XEdgePointY(fFraction);
}

float __fastcall TERectangleGauge::Get_XXEdgePointX(float fFraction)
{
  return GetDefaultInterface()->get_XXEdgePointX(fFraction);
}

float __fastcall TERectangleGauge::Get_XXEdgePointY(float fFraction)
{
  return GetDefaultInterface()->get_XXEdgePointY(fFraction);
}

float __fastcall TERectangleGauge::Get_YEdgePointX(float fFraction)
{
  return GetDefaultInterface()->get_YEdgePointX(fFraction);
}

float __fastcall TERectangleGauge::Get_YEdgePointY(float fFraction)
{
  return GetDefaultInterface()->get_YEdgePointY(fFraction);
}

float __fastcall TERectangleGauge::Get_YYEdgePointX(float fFraction)
{
  return GetDefaultInterface()->get_YYEdgePointX(fFraction);
}

float __fastcall TERectangleGauge::Get_YYEdgePointY(float fFraction)
{
  return GetDefaultInterface()->get_YYEdgePointY(fFraction);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEWedgeGauge which
// allows "EWedgeGauge Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEWedgeGauge::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EE2, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EE1, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEWedgeGauge::DEF_CTL_INTF = {0x9AE86EE0, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEWedgeGauge::OptParam;

static inline void ValidCtrCheck(TEWedgeGauge *)
{
   delete new TEWedgeGauge((TComponent*)(0));
};

void __fastcall TEWedgeGauge::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEWedgeGauge::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEWedgeGaugeDisp __fastcall TEWedgeGauge::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEWedgeGauge::Plot(long hDC, Evision_tlb::enumGgePlotItems DrawItems, float fWidth, 
                                   float fHeight, float fOrgX, float fOrgY)
{
  GetDefaultInterface()->Plot(hDC, DrawItems, fWidth, fHeight, fOrgX, fOrgY);
}

void __fastcall TEWedgeGauge::SetCenter(float fCenterX, float fCenterY)
{
  GetDefaultInterface()->SetCenter(fCenterX, fCenterY);
}

void __fastcall TEWedgeGauge::Measure(LPDISPATCH BW8Image)
{
  GetDefaultInterface()->Measure(BW8Image);
}

void __fastcall TEWedgeGauge::Process(LPDISPATCH BW8Image, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Process(BW8Image, bDaughters);
}

void __fastcall TEWedgeGauge::Draw(LPDISPATCH BW8Image, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                   TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Draw(BW8Image, DrawingMode, bDaughters);
}

TOLEBOOL __fastcall TEWedgeGauge::HitTest(TOLEBOOL bDaughters)
{
  return GetDefaultInterface()->HitTest(bDaughters);
}

void __fastcall TEWedgeGauge::Drag(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->Drag(nCursorX, nCursorY);
}

void __fastcall TEWedgeGauge::Closest(void)
{
  GetDefaultInterface()->Closest();
}

void __fastcall TEWedgeGauge::SetVisible(TOLEBOOL bVisible, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetVisible(bVisible, bDaughters);
}

void __fastcall TEWedgeGauge::SetSelected(TOLEBOOL bSelected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelected(bSelected, bDaughters);
}

void __fastcall TEWedgeGauge::SetSelectable(TOLEBOOL bSelectable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetSelectable(bSelectable, bDaughters);
}

void __fastcall TEWedgeGauge::SetDragable(TOLEBOOL bDragable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetDragable(bDragable, bDaughters);
}

void __fastcall TEWedgeGauge::SetResizable(TOLEBOOL bResizable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetResizable(bResizable, bDaughters);
}

void __fastcall TEWedgeGauge::SetRotatable(TOLEBOOL bRotatable, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetRotatable(bRotatable, bDaughters);
}

void __fastcall TEWedgeGauge::SetActive(TOLEBOOL bActive, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActive(bActive, bDaughters);
}

void __fastcall TEWedgeGauge::SetLabeled(TOLEBOOL bLabeled, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetLabeled(bLabeled, bDaughters);
}

void __fastcall TEWedgeGauge::SetAutoArrange(TOLEBOOL bAutoArrange, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetAutoArrange(bAutoArrange, bDaughters);
}

void __fastcall TEWedgeGauge::SetFound(TOLEBOOL bFound, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetFound(bFound, bDaughters);
}

void __fastcall TEWedgeGauge::SetInspected(TOLEBOOL bInspected, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetInspected(bInspected, bDaughters);
}

void __fastcall TEWedgeGauge::SetPassed(TOLEBOOL bPassed, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetPassed(bPassed, bDaughters);
}

void __fastcall TEWedgeGauge::SetQuickDraw(TOLEBOOL bQuickDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetQuickDraw(bQuickDraw, bDaughters);
}

void __fastcall TEWedgeGauge::SetOptionalDraw(TOLEBOOL bOptionalDraw, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetOptionalDraw(bOptionalDraw, bDaughters);
}

void __fastcall TEWedgeGauge::SetUserFlag(TOLEBOOL bUserFlag, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetUserFlag(bUserFlag, bDaughters);
}

void __fastcall TEWedgeGauge::Attach(long Mother)
{
  GetDefaultInterface()->Attach(Mother);
}

void __fastcall TEWedgeGauge::Detach(void)
{
  GetDefaultInterface()->Detach();
}

void __fastcall TEWedgeGauge::DetachDaughters(void)
{
  GetDefaultInterface()->DetachDaughters();
}

void __fastcall TEWedgeGauge::SetCursor(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->SetCursor(nCursorX, nCursorY);
}

void __fastcall TEWedgeGauge::Save(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Save(PathName, bDaughters);
}

void __fastcall TEWedgeGauge::Load(BSTR PathName, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->Load(PathName, bDaughters);
}

void __fastcall TEWedgeGauge::MeasureSample(LPDISPATCH BW8Image, long Index)
{
  GetDefaultInterface()->MeasureSample(BW8Image, Index);
}

void __fastcall TEWedgeGauge::SetActualShape(TOLEBOOL bActualShape, TOLEBOOL bDaughters)
{
  GetDefaultInterface()->SetActualShape(bActualShape, bDaughters);
}

void __fastcall TEWedgeGauge::Set(float fCenterX, float fCenterY, float fDiameter, float fBreadth, 
                                  float fOrgAngle, float fAmplitude)
{
  GetDefaultInterface()->Set(fCenterX, fCenterY, fDiameter, fBreadth, fOrgAngle, fAmplitude);
}

void __fastcall TEWedgeGauge::SetRadii(float fRadius, float fBreadth)
{
  GetDefaultInterface()->SetRadii(fRadius, fBreadth);
}

void __fastcall TEWedgeGauge::SetDiameters(float fDiameter, float fBreadth)
{
  GetDefaultInterface()->SetDiameters(fDiameter, fBreadth);
}

void __fastcall TEWedgeGauge::SetFull(float fCenterX, float fCenterY, float fDiameter, 
                                      float fBreadth, float fOrgAngle, TOLEBOOL bDirect)
{
  GetDefaultInterface()->SetFull(fCenterX, fCenterY, fDiameter, fBreadth, fOrgAngle, bDirect);
}

void __fastcall TEWedgeGauge::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEWedgeGauge::DrawToDC(OLE_HANDLE hDC, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                       TOLEBOOL bDaughters)
{
  GetDefaultInterface()->DrawToDC(hDC, DrawingMode, bDaughters);
}

long __fastcall TEWedgeGauge::GetShapeNamed(BSTR lpszSearchedValue)
{
  return GetDefaultInterface()->GetShapeNamed(lpszSearchedValue);
}

long __fastcall TEWedgeGauge::SamplePointValid(long nIndex)
{
  return GetDefaultInterface()->SamplePointValid(nIndex);
}

float __fastcall TEWedgeGauge::SamplePointX(long nIndex)
{
  return GetDefaultInterface()->SamplePointX(nIndex);
}

float __fastcall TEWedgeGauge::SamplePointY(long nIndex)
{
  return GetDefaultInterface()->SamplePointY(nIndex);
}

void __fastcall TEWedgeGauge::DragHitShape(long nCursorX, long nCursorY)
{
  GetDefaultInterface()->DragHitShape(nCursorX, nCursorY);
}

long __fastcall TEWedgeGauge::Get_Daughter(long Index)
{
  return GetDefaultInterface()->get_Daughter(Index);
}

float __fastcall TEWedgeGauge::Get_PointX(float fFractionX, float fFractionY)
{
  return GetDefaultInterface()->get_PointX(fFractionX, fFractionY);
}

float __fastcall TEWedgeGauge::Get_PointY(float fFractionX, float fFractionY)
{
  return GetDefaultInterface()->get_PointY(fFractionX, fFractionY);
}

float __fastcall TEWedgeGauge::Get_AEdgePointX(float fFraction)
{
  return GetDefaultInterface()->get_AEdgePointX(fFraction);
}

float __fastcall TEWedgeGauge::Get_AEdgePointY(float fFraction)
{
  return GetDefaultInterface()->get_AEdgePointY(fFraction);
}

float __fastcall TEWedgeGauge::Get_AAEdgePointX(float fFraction)
{
  return GetDefaultInterface()->get_AAEdgePointX(fFraction);
}

float __fastcall TEWedgeGauge::Get_AAEdgePointY(float fFraction)
{
  return GetDefaultInterface()->get_AAEdgePointY(fFraction);
}

float __fastcall TEWedgeGauge::Get_REdgePointX(float fFraction)
{
  return GetDefaultInterface()->get_REdgePointX(fFraction);
}

float __fastcall TEWedgeGauge::Get_REdgePointY(float fFraction)
{
  return GetDefaultInterface()->get_REdgePointY(fFraction);
}

float __fastcall TEWedgeGauge::Get_RREdgePointX(float fFraction)
{
  return GetDefaultInterface()->get_RREdgePointX(fFraction);
}

float __fastcall TEWedgeGauge::Get_RREdgePointY(float fFraction)
{
  return GetDefaultInterface()->get_RREdgePointY(fFraction);
}

float __fastcall TEWedgeGauge::Get_InnerPointX(float fFraction)
{
  return GetDefaultInterface()->get_InnerPointX(fFraction);
}

float __fastcall TEWedgeGauge::Get_InnerPointY(float fFraction)
{
  return GetDefaultInterface()->get_InnerPointY(fFraction);
}

float __fastcall TEWedgeGauge::Get_OuterPointX(float fFraction)
{
  return GetDefaultInterface()->get_OuterPointX(fFraction);
}

float __fastcall TEWedgeGauge::Get_OuterPointY(float fFraction)
{
  return GetDefaultInterface()->get_OuterPointY(fFraction);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEPointMeasure which
// allows "EPointMeasure Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEPointMeasure::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E9E, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E9D, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEPointMeasure::DEF_CTL_INTF = {0x9AE86E9C, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEPointMeasure::OptParam;

static inline void ValidCtrCheck(TEPointMeasure *)
{
   delete new TEPointMeasure((TComponent*)(0));
};

void __fastcall TEPointMeasure::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEPointMeasure::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEPointMeasureDisp __fastcall TEPointMeasure::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEPointMeasure::FindPointAlongLine(LPDISPATCH BW8SrcImage)
{
  GetDefaultInterface()->FindPointAlongLine(BW8SrcImage);
}

void __fastcall TEPointMeasure::Draw(LPDISPATCH Image, Evision_tlb::enumMsrDrawingMode DrawingMode, 
                                     TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->Draw(Image, DrawingMode, DrawHandles);
}

Evision_tlb::enumMsrHandleNames __fastcall TEPointMeasure::HitTest(LPDISPATCH Image, long X, long Y)
{
  return GetDefaultInterface()->HitTest(Image, X, Y);
}

void __fastcall TEPointMeasure::Drag(LPDISPATCH Image, Evision_tlb::enumMsrHandleNames Handle, 
                                     long X, long Y)
{
  GetDefaultInterface()->Drag(Image, Handle, X, Y);
}

void __fastcall TEPointMeasure::SetAllTransitions(void)
{
  GetDefaultInterface()->SetAllTransitions();
}

void __fastcall TEPointMeasure::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEPointMeasure::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEPointMeasure::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEPointMeasure::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEPointMeasure::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}

float __fastcall TEPointMeasure::Get_MeasuredXAt(long Index)
{
  return GetDefaultInterface()->get_MeasuredXAt(Index);
}

float __fastcall TEPointMeasure::Get_MeasuredYAt(long Index)
{
  return GetDefaultInterface()->get_MeasuredYAt(Index);
}

long __fastcall TEPointMeasure::Get_MeasuredGrayStepAt(long Index)
{
  return GetDefaultInterface()->get_MeasuredGrayStepAt(Index);
}

TOLEBOOL __fastcall TEPointMeasure::Get_ValidPointAt(long Index)
{
  return GetDefaultInterface()->get_ValidPointAt(Index);
}

float __fastcall TEPointMeasure::Get_RelativePositionAt(long Index)
{
  return GetDefaultInterface()->get_RelativePositionAt(Index);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TELineMeasure which
// allows "ELineMeasure Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TELineMeasure::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EA2, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EA1, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TELineMeasure::DEF_CTL_INTF = {0x9AE86EA0, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TELineMeasure::OptParam;

static inline void ValidCtrCheck(TELineMeasure *)
{
   delete new TELineMeasure((TComponent*)(0));
};

void __fastcall TELineMeasure::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TELineMeasure::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DELineMeasureDisp __fastcall TELineMeasure::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TELineMeasure::FitLine(LPDISPATCH BW8SrcImage)
{
  GetDefaultInterface()->FitLine(BW8SrcImage);
}

void __fastcall TELineMeasure::Draw(LPDISPATCH Image, Evision_tlb::enumMsrDrawingMode DrawingMode, 
                                    TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->Draw(Image, DrawingMode, DrawHandles);
}

Evision_tlb::enumMsrHandleNames __fastcall TELineMeasure::HitTest(LPDISPATCH Image, long X, long Y)
{
  return GetDefaultInterface()->HitTest(Image, X, Y);
}

void __fastcall TELineMeasure::Drag(LPDISPATCH Image, Evision_tlb::enumMsrHandleNames Handle, long X, 
                                    long Y)
{
  GetDefaultInterface()->Drag(Image, Handle, X, Y);
}

void __fastcall TELineMeasure::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TELineMeasure::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TELineMeasure::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TELineMeasure::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TELineMeasure::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}

float __fastcall TELineMeasure::Get_PointX(short Indice)
{
  return GetDefaultInterface()->get_PointX(Indice);
}

float __fastcall TELineMeasure::Get_PointY(short Indice)
{
  return GetDefaultInterface()->get_PointY(Indice);
}

TOLEBOOL __fastcall TELineMeasure::Get_PointValid(short Indice)
{
  return GetDefaultInterface()->get_PointValid(Indice);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TECircleMeasure which
// allows "ECircleMeasure Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TECircleMeasure::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EA6, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EA5, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TECircleMeasure::DEF_CTL_INTF = {0x9AE86EA4, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TECircleMeasure::OptParam;

static inline void ValidCtrCheck(TECircleMeasure *)
{
   delete new TECircleMeasure((TComponent*)(0));
};

void __fastcall TECircleMeasure::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TECircleMeasure::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DECircleMeasureDisp __fastcall TECircleMeasure::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TECircleMeasure::FitCircle(LPDISPATCH BW8SrcImage)
{
  GetDefaultInterface()->FitCircle(BW8SrcImage);
}

void __fastcall TECircleMeasure::Draw(LPDISPATCH Image, Evision_tlb::enumMsrDrawingMode DrawingMode, 
                                      TOLEBOOL DrawHandles)
{
  GetDefaultInterface()->Draw(Image, DrawingMode, DrawHandles);
}

Evision_tlb::enumMsrHandleNames __fastcall TECircleMeasure::HitTest(LPDISPATCH Image, long X, long Y)
{
  return GetDefaultInterface()->HitTest(Image, X, Y);
}

void __fastcall TECircleMeasure::Drag(LPDISPATCH Image, Evision_tlb::enumMsrHandleNames Handle, 
                                      long X, long Y)
{
  GetDefaultInterface()->Drag(Image, Handle, X, Y);
}

void __fastcall TECircleMeasure::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TECircleMeasure::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TECircleMeasure::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TECircleMeasure::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TECircleMeasure::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}

float __fastcall TECircleMeasure::Get_PointX(short Indice)
{
  return GetDefaultInterface()->get_PointX(Indice);
}

float __fastcall TECircleMeasure::Get_PointY(short Indice)
{
  return GetDefaultInterface()->get_PointY(Indice);
}

TOLEBOOL __fastcall TECircleMeasure::Get_PointValid(short Indice)
{
  return GetDefaultInterface()->get_PointValid(Indice);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEMatch which
// allows "EMatch Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEMatch::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86E96, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86E95, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEMatch::DEF_CTL_INTF = {0x9AE86E94, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEMatch::OptParam;

static inline void ValidCtrCheck(TEMatch *)
{
   delete new TEMatch((TComponent*)(0));
};

void __fastcall TEMatch::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEMatch::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEMatchDisp __fastcall TEMatch::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEMatch::LearnPattern(LPDISPATCH SrcImage)
{
  GetDefaultInterface()->LearnPattern(SrcImage);
}

void __fastcall TEMatch::Match(LPDISPATCH SrcImage)
{
  GetDefaultInterface()->Match(SrcImage);
}

void __fastcall TEMatch::GetPosition(long Index, float* CenterX, float* CenterY, float* Angle, 
                                     float* ScaleX, float* ScaleY, float* Score)
{
  GetDefaultInterface()->GetPosition(Index, CenterX, CenterY, Angle, ScaleX, ScaleY, Score);
}

void __fastcall TEMatch::DrawPosition(LPDISPATCH Image, long Index)
{
  GetDefaultInterface()->DrawPosition(Image, Index);
}

void __fastcall TEMatch::DrawPositions(LPDISPATCH Image)
{
  GetDefaultInterface()->DrawPositions(Image);
}

void __fastcall TEMatch::Load(BSTR PathName)
{
  GetDefaultInterface()->Load(PathName);
}

void __fastcall TEMatch::Save(BSTR PathName)
{
  GetDefaultInterface()->Save(PathName);
}

void __fastcall TEMatch::GetPattern(LPDISPATCH DstImg)
{
  GetDefaultInterface()->GetPattern(DstImg);
}

void __fastcall TEMatch::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEMatch::DrawPositionToDC(OLE_HANDLE hDC, long Index)
{
  GetDefaultInterface()->DrawPositionToDC(hDC, Index);
}

void __fastcall TEMatch::DrawPositionsToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawPositionsToDC(hDC);
}

LPDISPATCH __fastcall TEMatch::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEMatch::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEMatch::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEMatch::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEFind which
// allows "EFind Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEFind::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86ECA, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EC9, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEFind::DEF_CTL_INTF = {0x9AE86EC8, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEFind::OptParam;

static inline void ValidCtrCheck(TEFind *)
{
   delete new TEFind((TComponent*)(0));
};

void __fastcall TEFind::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEFind::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEFindDisp __fastcall TEFind::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEFind::Find(LPDISPATCH BW8SrcRoi)
{
  GetDefaultInterface()->Find(BW8SrcRoi);
}

void __fastcall TEFind::Learn(LPDISPATCH BW8SrcRoi)
{
  GetDefaultInterface()->Learn(BW8SrcRoi);
}

void __fastcall TEFind::SelectInstance(long Index)
{
  GetDefaultInterface()->SelectInstance(Index);
}

void __fastcall TEFind::SetCenter(float CenterX, float CenterY)
{
  GetDefaultInterface()->SetCenter(CenterX, CenterY);
}

void __fastcall TEFind::LearnWithMask(LPDISPATCH BW8SrcRoi, LPDISPATCH DontCare)
{
  GetDefaultInterface()->LearnWithMask(BW8SrcRoi, DontCare);
}

void __fastcall TEFind::Load(BSTR PathName)
{
  GetDefaultInterface()->Load(PathName);
}

void __fastcall TEFind::Save(BSTR PathName)
{
  GetDefaultInterface()->Save(PathName);
}

void __fastcall TEFind::DrawInstances(LPDISPATCH BW8SrcImage, TOLEBOOL Edge)
{
  GetDefaultInterface()->DrawInstances(BW8SrcImage, Edge);
}

void __fastcall TEFind::DrawInstance(LPDISPATCH BW8SrcImage, TOLEBOOL Edge)
{
  GetDefaultInterface()->DrawInstance(BW8SrcImage, Edge);
}

void __fastcall TEFind::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEFind::DrawInstancesToDC(OLE_HANDLE hDC, TOLEBOOL bEdge)
{
  GetDefaultInterface()->DrawInstancesToDC(hDC, bEdge);
}

void __fastcall TEFind::DrawInstanceToDC(OLE_HANDLE hDC, TOLEBOOL bEdge)
{
  GetDefaultInterface()->DrawInstanceToDC(hDC, bEdge);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEOCR which
// allows "EOCR Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEOCR::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EAA, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EA9, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEOCR::DEF_CTL_INTF = {0x9AE86EA8, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEOCR::OptParam;

static inline void ValidCtrCheck(TEOCR *)
{
   delete new TEOCR((TComponent*)(0));
};

void __fastcall TEOCR::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEOCR::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEOCRDisp __fastcall TEOCR::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

long __fastcall TEOCR::GetFirstCharCode(long Index)
{
  return GetDefaultInterface()->GetFirstCharCode(Index);
}

long __fastcall TEOCR::GetSecondCharCode(long Index)
{
  return GetDefaultInterface()->GetSecondCharCode(Index);
}

float __fastcall TEOCR::GetFirstCharDistance(long Index)
{
  return GetDefaultInterface()->GetFirstCharDistance(Index);
}

float __fastcall TEOCR::GetSecondCharDistance(long Index)
{
  return GetDefaultInterface()->GetSecondCharDistance(Index);
}

long __fastcall TEOCR::CharOrgX(long Index)
{
  return GetDefaultInterface()->CharOrgX(Index);
}

long __fastcall TEOCR::CharOrgY(long Index)
{
  return GetDefaultInterface()->CharOrgY(Index);
}

long __fastcall TEOCR::CharDstX(long Index)
{
  return GetDefaultInterface()->CharDstX(Index);
}

long __fastcall TEOCR::CharDstY(long Index)
{
  return GetDefaultInterface()->CharDstY(Index);
}

long __fastcall TEOCR::CharTotalOrgX(long Index)
{
  return GetDefaultInterface()->CharTotalOrgX(Index);
}

long __fastcall TEOCR::CharTotalOrgY(long Index)
{
  return GetDefaultInterface()->CharTotalOrgY(Index);
}

long __fastcall TEOCR::CharTotalDstX(long Index)
{
  return GetDefaultInterface()->CharTotalDstX(Index);
}

long __fastcall TEOCR::CharTotalDstY(long Index)
{
  return GetDefaultInterface()->CharTotalDstY(Index);
}

long __fastcall TEOCR::CharWidth(long Index)
{
  return GetDefaultInterface()->CharWidth(Index);
}

long __fastcall TEOCR::CharHeight(long Index)
{
  return GetDefaultInterface()->CharHeight(Index);
}

void __fastcall TEOCR::AddClasses(long Classes)
{
  GetDefaultInterface()->AddClasses(Classes);
}

void __fastcall TEOCR::EmptyClasses(void)
{
  GetDefaultInterface()->EmptyClasses();
}

void __fastcall TEOCR::RecognizeClasses(LPDISPATCH BW8SrcImage, long MaxChars, BSTR* DstString)
{
  GetDefaultInterface()->RecognizeClasses(BW8SrcImage, MaxChars, DstString);
}

void __fastcall TEOCR::RecognizeClass(LPDISPATCH BW8SrcImage, long MaxChars, 
                                      Evision_tlb::enumOcrClasses Classes, BSTR* DstString)
{
  GetDefaultInterface()->RecognizeClass(BW8SrcImage, MaxChars, Classes, DstString);
}

void __fastcall TEOCR::LoadFont(BSTR PathName)
{
  GetDefaultInterface()->LoadFont(PathName);
}

void __fastcall TEOCR::SaveFont(BSTR PathName)
{
  GetDefaultInterface()->SaveFont(PathName);
}

long __fastcall TEOCR::GetPatternCode(long Index)
{
  return GetDefaultInterface()->GetPatternCode(Index);
}

long __fastcall TEOCR::GetPatternClass(long Index)
{
  return GetDefaultInterface()->GetPatternClass(Index);
}

void __fastcall TEOCR::AddPatternFromImage(LPDISPATCH BW8SrcImage, long OriginX, long OriginY, 
                                           long Width, long Height, long Code, 
                                           Evision_tlb::enumOcrClasses Class)
{
  GetDefaultInterface()->AddPatternFromImage(BW8SrcImage, OriginX, OriginY, Width, Height, Code, 
                                             Class);
}

void __fastcall TEOCR::DrawChar(LPDISPATCH Image, long Index)
{
  GetDefaultInterface()->DrawChar(Image, Index);
}

void __fastcall TEOCR::DrawChars(LPDISPATCH Image)
{
  GetDefaultInterface()->DrawChars(Image);
}

void __fastcall TEOCR::NewFont(long PatternWidth, long PatternHeight)
{
  GetDefaultInterface()->NewFont(PatternWidth, PatternHeight);
}

void __fastcall TEOCR::BuildObjects(LPDISPATCH BW8SrcImage)
{
  GetDefaultInterface()->BuildObjects(BW8SrcImage);
}

void __fastcall TEOCR::FindAllChars(LPDISPATCH BW8SrcImage)
{
  GetDefaultInterface()->FindAllChars(BW8SrcImage);
}

void __fastcall TEOCR::EmptyChars(void)
{
  GetDefaultInterface()->EmptyChars();
}

short __fastcall TEOCR::AddChar(long OrgX, long OrgY, long EndX, long EndY)
{
  return GetDefaultInterface()->AddChar(OrgX, OrgY, EndX, EndY);
}

void __fastcall TEOCR::ReadTextClass(LPDISPATCH BW8SrcImage, long MaxChars, 
                                     Evision_tlb::enumOcrClasses Classes, BSTR* DstString)
{
  GetDefaultInterface()->ReadTextClass(BW8SrcImage, MaxChars, Classes, DstString);
}

void __fastcall TEOCR::ReadTextClasses(LPDISPATCH BW8SrcImage, long MaxChars, BSTR* DstString)
{
  GetDefaultInterface()->ReadTextClasses(BW8SrcImage, MaxChars, DstString);
}

TOLEBOOL __fastcall TEOCR::HitChars(float CursorX, float CursorY, long CharIndex, float ZoomX, 
                                    float ZoomY, float panX, float panY)
{
  return GetDefaultInterface()->HitChars(CursorX, CursorY, CharIndex, ZoomX, ZoomY, panX, panY);
}

void __fastcall TEOCR::LearnPattern(LPDISPATCH BW8SrcImg, long* CharIndex, long Code, 
                                    Evision_tlb::enumOcrClasses Class)
{
  GetDefaultInterface()->LearnPattern(BW8SrcImg, CharIndex, Code, Class);
}

void __fastcall TEOCR::LearnPatternAdded(LPDISPATCH BW8SrcImg, long CharIndex, long Code, 
                                         Evision_tlb::enumOcrClasses Class)
{
  GetDefaultInterface()->LearnPatternAdded(BW8SrcImg, CharIndex, Code, Class);
}

void __fastcall TEOCR::ReadTextClassAdded(LPDISPATCH BW8SrcImage, long MaxChars, 
                                          Evision_tlb::enumOcrClasses Classes, BSTR* DstString)
{
  GetDefaultInterface()->ReadTextClassAdded(BW8SrcImage, MaxChars, Classes, DstString);
}

void __fastcall TEOCR::ReadTextClassesAdded(LPDISPATCH BW8SrcImage, long MaxChars, BSTR* DstString)
{
  GetDefaultInterface()->ReadTextClassesAdded(BW8SrcImage, MaxChars, DstString);
}

void __fastcall TEOCR::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEOCR::RemovePattern(long Index)
{
  GetDefaultInterface()->RemovePattern(Index);
}

void __fastcall TEOCR::DrawCharToDC(OLE_HANDLE hDC, long Index)
{
  GetDefaultInterface()->DrawCharToDC(hDC, Index);
}

void __fastcall TEOCR::DrawCharsToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawCharsToDC(hDC);
}

float __fastcall TEOCR::GetConfidenceRatio(long Index)
{
  return GetDefaultInterface()->GetConfidenceRatio(Index);
}

void __fastcall TEOCR::GetPatternBitmap(LPDISPATCH BW8Image, long Index)
{
  GetDefaultInterface()->GetPatternBitmap(BW8Image, Index);
}

LPDISPATCH __fastcall TEOCR::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEOCR::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEOCR::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEOCR::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEOCV which
// allows "EOCV Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEOCV::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EB2, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EB1, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEOCV::DEF_CTL_INTF = {0x9AE86EB0, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEOCV::OptParam;

static inline void ValidCtrCheck(TEOCV *)
{
   delete new TEOCV((TComponent*)(0));
};

void __fastcall TEOCV::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEOCV::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEOCVDisp __fastcall TEOCV::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEOCV::Load(BSTR PathName)
{
  GetDefaultInterface()->Load(PathName);
}

void __fastcall TEOCV::Save(BSTR PathName)
{
  GetDefaultInterface()->Save(PathName);
}

void __fastcall TEOCV::Inspect(LPDISPATCH BW8SampleImage, long Threshold)
{
  GetDefaultInterface()->Inspect(BW8SampleImage, Threshold);
}

void __fastcall TEOCV::DrawText(LPDISPATCH Image, LPDISPATCH OCVText)
{
  GetDefaultInterface()->DrawText(Image, OCVText);
}

void __fastcall TEOCV::DrawTexts(LPDISPATCH Image, 
                                 Evision_tlb::enumSelectionMode eTextsSelectionMode)
{
  GetDefaultInterface()->DrawTexts(Image, eTextsSelectionMode);
}

void __fastcall TEOCV::DrawTextChars(LPDISPATCH Image, LPDISPATCH OCVText, 
                                     Evision_tlb::enumSelectionMode eCharsSelectionMode)
{
  GetDefaultInterface()->DrawTextChars(Image, OCVText, eCharsSelectionMode);
}

void __fastcall TEOCV::DrawTextsChars(LPDISPATCH Image, 
                                      Evision_tlb::enumSelectionMode eTextsSelectionMode, 
                                      Evision_tlb::enumSelectionMode eCharsSelectionMode)
{
  GetDefaultInterface()->DrawTextsChars(Image, eTextsSelectionMode, eCharsSelectionMode);
}

void __fastcall TEOCV::GetTextParameters(LPDISPATCH OCVText, long TextIndex)
{
  GetDefaultInterface()->GetTextParameters(OCVText, TextIndex);
}

void __fastcall TEOCV::SetTextParameters(LPDISPATCH OCVText, long TextIndex)
{
  GetDefaultInterface()->SetTextParameters(OCVText, TextIndex);
}

void __fastcall TEOCV::GatherTextsParameters(LPDISPATCH OCVText, 
                                             Evision_tlb::enumSelectionMode eTextsSelectionMode)
{
  GetDefaultInterface()->GatherTextsParameters(OCVText, eTextsSelectionMode);
}

void __fastcall TEOCV::ScatterTextsParameters(LPDISPATCH OCVText, 
                                              Evision_tlb::enumSelectionMode eTextsSelectionMode)
{
  GetDefaultInterface()->ScatterTextsParameters(OCVText, eTextsSelectionMode);
}

void __fastcall TEOCV::GetTextCharParameters(LPDISPATCH OCVChar, long TextIndex, long CharIndex)
{
  GetDefaultInterface()->GetTextCharParameters(OCVChar, TextIndex, CharIndex);
}

void __fastcall TEOCV::SetTextCharParameters(LPDISPATCH OCVChar, long TextIndex, long CharIndex)
{
  GetDefaultInterface()->SetTextCharParameters(OCVChar, TextIndex, CharIndex);
}

void __fastcall TEOCV::GatherTextsCharsParameters(LPDISPATCH OCVChar, 
                                                  Evision_tlb::enumSelectionMode eTextsSelectionMode, 
                                                  Evision_tlb::enumSelectionMode eCharsSelectionMode)
{
  GetDefaultInterface()->GatherTextsCharsParameters(OCVChar, eTextsSelectionMode, 
                                                    eCharsSelectionMode);
}

void __fastcall TEOCV::ScatterTextsCharsParameters(LPDISPATCH OCVChar, 
                                                   Evision_tlb::enumSelectionMode eTextsSelectionMode, 
                                                   Evision_tlb::enumSelectionMode eCharsSelectionMode)
{
  GetDefaultInterface()->ScatterTextsCharsParameters(OCVChar, eTextsSelectionMode, 
                                                     eCharsSelectionMode);
}

void __fastcall TEOCV::SelectSampleTexts(long OriginX, long OriginY, long Width, long Height, 
                                         Evision_tlb::enumSelectionMode eTextsSelectionMode)
{
  GetDefaultInterface()->SelectSampleTexts(OriginX, OriginY, Width, Height, eTextsSelectionMode);
}

void __fastcall TEOCV::SelectSampleTextsChars(long OriginX, long OriginY, long Width, long Height, 
                                              Evision_tlb::enumSelectionMode eTextsSelectionMode, 
                                              Evision_tlb::enumSelectionMode eCharsSelectionMode)
{
  GetDefaultInterface()->SelectSampleTextsChars(OriginX, OriginY, Width, Height, eTextsSelectionMode, 
                                                eCharsSelectionMode);
}

void __fastcall TEOCV::ClearStatistics(void)
{
  GetDefaultInterface()->ClearStatistics();
}

void __fastcall TEOCV::UpdateStatistics(void)
{
  GetDefaultInterface()->UpdateStatistics();
}

void __fastcall TEOCV::ComputeDefaultTolerances(LPDISPATCH BW8Image, long Threshold)
{
  GetDefaultInterface()->ComputeDefaultTolerances(BW8Image, Threshold);
}

float __fastcall TEOCV::GetSampleContrast(void)
{
  return GetDefaultInterface()->GetSampleContrast();
}

float __fastcall TEOCV::GetContrastAverage(void)
{
  return GetDefaultInterface()->GetContrastAverage();
}

float __fastcall TEOCV::GetContrastDeviation(void)
{
  return GetDefaultInterface()->GetContrastDeviation();
}

void __fastcall TEOCV::SetContrastTolerance(float fNewValue)
{
  GetDefaultInterface()->SetContrastTolerance(fNewValue);
}

float __fastcall TEOCV::GetConstrastTolerance(void)
{
  return GetDefaultInterface()->GetConstrastTolerance();
}

float __fastcall TEOCV::GetTemplateContrast(void)
{
  return GetDefaultInterface()->GetTemplateContrast();
}

void __fastcall TEOCV::AdjustShiftTolerance(LPDISPATCH BW8Image)
{
  GetDefaultInterface()->AdjustShiftTolerance(BW8Image);
}

void __fastcall TEOCV::AdjustTextsLocationRanges(float fFactor, 
                                                 Evision_tlb::enumSelectionMode eSelection)
{
  GetDefaultInterface()->AdjustTextsLocationRanges(fFactor, eSelection);
}

void __fastcall TEOCV::AdjustTextsQualityRanges(float fFactor, 
                                                Evision_tlb::enumSelectionMode eModeSelection)
{
  GetDefaultInterface()->AdjustTextsQualityRanges(fFactor, eModeSelection);
}

void __fastcall TEOCV::AdjustCharsLocationRanges(float fFactor, 
                                                 Evision_tlb::enumSelectionMode eTextSelection, 
                                                 Evision_tlb::enumSelectionMode eCharSelection)
{
  GetDefaultInterface()->AdjustCharsLocationRanges(fFactor, eTextSelection, eCharSelection);
}

void __fastcall TEOCV::AdjustCharsQualityRanges(float fFacor, 
                                                Evision_tlb::enumSelectionMode eTextSelection, 
                                                Evision_tlb::enumSelectionMode eCharSelection)
{
  GetDefaultInterface()->AdjustCharsQualityRanges(fFacor, eTextSelection, eCharSelection);
}

void __fastcall TEOCV::DrawTemplateChars(LPDISPATCH Image, 
                                         Evision_tlb::enumSelectionMode CharsSelectionMode)
{
  GetDefaultInterface()->DrawTemplateChars(Image, CharsSelectionMode);
}

void __fastcall TEOCV::DrawTemplateObjects(LPDISPATCH Image, 
                                           Evision_tlb::enumSelectionMode ObjectsSelectionMode)
{
  GetDefaultInterface()->DrawTemplateObjects(Image, ObjectsSelectionMode);
}

void __fastcall TEOCV::DrawTemplateTexts(LPDISPATCH Image, 
                                         Evision_tlb::enumSelectionMode TextsSelectionMode)
{
  GetDefaultInterface()->DrawTemplateTexts(Image, TextsSelectionMode);
}

void __fastcall TEOCV::DrawTemplateTextsChars(LPDISPATCH Image, 
                                              Evision_tlb::enumSelectionMode TextsSelectionMode, 
                                              Evision_tlb::enumSelectionMode CharsSelectionMode)
{
  GetDefaultInterface()->DrawTemplateTextsChars(Image, TextsSelectionMode, CharsSelectionMode);
}

void __fastcall TEOCV::CreateTemplateObjects(LPDISPATCH CodedImage, 
                                             Evision_tlb::enumSelectionMode CodedObjectsSelectionMode)
{
  GetDefaultInterface()->CreateTemplateObjects(CodedImage, CodedObjectsSelectionMode);
}

void __fastcall TEOCV::SelectTemplateObjects(long OriginX, long OriginY, long Width, long Height, 
                                             Evision_tlb::enumSelectionMode ObjectsSelectionMode)
{
  GetDefaultInterface()->SelectTemplateObjects(OriginX, OriginY, Width, Height, ObjectsSelectionMode);
}

void __fastcall TEOCV::DeleteTemplateObjects(Evision_tlb::enumSelectionMode ObjectsSelectionMode)
{
  GetDefaultInterface()->DeleteTemplateObjects(ObjectsSelectionMode);
}

void __fastcall TEOCV::SetTemplateImage(LPDISPATCH ROI)
{
  GetDefaultInterface()->SetTemplateImage(ROI);
}

void __fastcall TEOCV::SelectTemplateChars(long OriginX, long OriginY, long Width, long Height, 
                                           Evision_tlb::enumSelectionMode CharsSelectionMode)
{
  GetDefaultInterface()->SelectTemplateChars(OriginX, OriginY, Width, Height, CharsSelectionMode);
}

void __fastcall TEOCV::DeleteTemplateChars(Evision_tlb::enumSelectionMode CharsSelectionMode)
{
  GetDefaultInterface()->DeleteTemplateChars(CharsSelectionMode);
}

void __fastcall TEOCV::CreateTemplateTexts(Evision_tlb::enumSelectionMode CharsSelectionMode)
{
  GetDefaultInterface()->CreateTemplateTexts(CharsSelectionMode);
}

void __fastcall TEOCV::SelectTemplateTexts(long OriginX, long OriginY, long Width, long Height, 
                                           Evision_tlb::enumSelectionMode TextsSelectionMode)
{
  GetDefaultInterface()->SelectTemplateTexts(OriginX, OriginY, Width, Height, TextsSelectionMode);
}

void __fastcall TEOCV::DeleteTemplateTexts(Evision_tlb::enumSelectionMode TextsSelectionMode)
{
  GetDefaultInterface()->DeleteTemplateTexts(TextsSelectionMode);
}

void __fastcall TEOCV::Learn(LPDISPATCH ROI, Evision_tlb::enumSelectionMode TextsSelectionMode, 
                             Evision_tlb::enumSelectionMode CharsSelectionMode)
{
  GetDefaultInterface()->Learn(ROI, TextsSelectionMode, CharsSelectionMode);
}

void __fastcall TEOCV::CreateTemplateChars(Evision_tlb::enumSelectionMode ObjectsSelectionMode, 
                                           Evision_tlb::enumOcvCharCreationModes CreationMode)
{
  GetDefaultInterface()->CreateTemplateChars(ObjectsSelectionMode, CreationMode);
}

void __fastcall TEOCV::ThresholdTemplateImage(LPDISPATCH CodedImage)
{
  GetDefaultInterface()->ThresholdTemplateImage(CodedImage);
}

void __fastcall TEOCV::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEOCV::DrawTextToDC(OLE_HANDLE hDC, LPDISPATCH OCVText)
{
  GetDefaultInterface()->DrawTextToDC(hDC, OCVText);
}

void __fastcall TEOCV::DrawTextsToDC(OLE_HANDLE hDC, 
                                     Evision_tlb::enumSelectionMode eTextsSelectionMode)
{
  GetDefaultInterface()->DrawTextsToDC(hDC, eTextsSelectionMode);
}

void __fastcall TEOCV::DrawTextCharsToDC(OLE_HANDLE hDC, LPDISPATCH OCVText, 
                                         Evision_tlb::enumSelectionMode eCharsSelectionMode)
{
  GetDefaultInterface()->DrawTextCharsToDC(hDC, OCVText, eCharsSelectionMode);
}

void __fastcall TEOCV::DrawTextsCharsToDC(OLE_HANDLE hDC, 
                                          Evision_tlb::enumSelectionMode eTextsSelectionMode, 
                                          Evision_tlb::enumSelectionMode eCharsSelectionMode)
{
  GetDefaultInterface()->DrawTextsCharsToDC(hDC, eTextsSelectionMode, eCharsSelectionMode);
}

void __fastcall TEOCV::DrawTemplateObjectsToDC(OLE_HANDLE hDC, 
                                               Evision_tlb::enumSelectionMode ObjectsSelectionMode)
{
  GetDefaultInterface()->DrawTemplateObjectsToDC(hDC, ObjectsSelectionMode);
}

void __fastcall TEOCV::DrawTemplateCharsToDC(OLE_HANDLE hDC, 
                                             Evision_tlb::enumSelectionMode CharsSelectionMode)
{
  GetDefaultInterface()->DrawTemplateCharsToDC(hDC, CharsSelectionMode);
}

void __fastcall TEOCV::DrawTemplateTextsToDC(OLE_HANDLE hDC, 
                                             Evision_tlb::enumSelectionMode TextsSelectionMode)
{
  GetDefaultInterface()->DrawTemplateTextsToDC(hDC, TextsSelectionMode);
}

void __fastcall TEOCV::DrawTemplateTextsCharsToDC(OLE_HANDLE hDC, 
                                                  Evision_tlb::enumSelectionMode TextsSelectionMode, 
                                                  Evision_tlb::enumSelectionMode CharsSelectionMode)
{
  GetDefaultInterface()->DrawTemplateTextsCharsToDC(hDC, TextsSelectionMode, CharsSelectionMode);
}

void __fastcall TEOCV::GetTextCharPoint(long TextIndex, long CharIndex, long* X, long* Y, 
                                        float ReducedX, float ReducedY, float ZoomX, float ZoomY, 
                                        float panX, float panY)
{
  GetDefaultInterface()->GetTextCharPoint(TextIndex, CharIndex, X, Y, ReducedX, ReducedY, ZoomX, 
                                          ZoomY, panX, panY);
}

void __fastcall TEOCV::GetTextPoint(long n32TextIndex, long* n32X, long* n32Y, float f32ReducedX, 
                                    float f32ReducedY, float f32ZoomX, float f32ZoomY, float f32PanX, 
                                    float f32PanY)
{
  GetDefaultInterface()->GetTextPoint(n32TextIndex, n32X, n32Y, f32ReducedX, f32ReducedY, f32ZoomX, 
                                      f32ZoomY, f32PanX, f32PanY);
}

LPDISPATCH __fastcall TEOCV::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEOCV::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEOCV::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEOCV::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}

long __fastcall TEOCV::Get_NumTextChars(long TextIndex)
{
  return GetDefaultInterface()->get_NumTextChars(TextIndex);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEOCVChar which
// allows "EOCVChar Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEOCVChar::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EB6, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EB5, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEOCVChar::DEF_CTL_INTF = {0x9AE86EB4, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEOCVChar::OptParam;

static inline void ValidCtrCheck(TEOCVChar *)
{
   delete new TEOCVChar((TComponent*)(0));
};

void __fastcall TEOCVChar::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEOCVChar::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEOCVCharDisp __fastcall TEOCVChar::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEOCVChar::ResetParameters(void)
{
  GetDefaultInterface()->ResetParameters();
}

void __fastcall TEOCVChar::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEOCVChar::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEOCVChar::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEOCVChar::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEOCVChar::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEOCVText which
// allows "EOCVText Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEOCVText::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EBA, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EB9, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEOCVText::DEF_CTL_INTF = {0x9AE86EB8, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEOCVText::OptParam;

static inline void ValidCtrCheck(TEOCVText *)
{
   delete new TEOCVText((TComponent*)(0));
};

void __fastcall TEOCVText::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEOCVText::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEOCVTextDisp __fastcall TEOCVText::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEOCVText::ResetParameters(void)
{
  GetDefaultInterface()->ResetParameters();
}

void __fastcall TEOCVText::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

LPDISPATCH __fastcall TEOCVText::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TEOCVText::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}

LPDISPATCH __fastcall TEOCVText::Get_Object()
{
  return GetDefaultInterface()->get_Object();
}

void __fastcall TEOCVText::Set_Object(LPDISPATCH param)
{
  GetDefaultInterface()->set_Object(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEChecker which
// allows "EChecker Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEChecker::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x610FBEB6, 0x9B78, 0x41A5,{ 0xA3, 0x63, 0x9C, 0x26, 0x76, 0x15, 0xE1, 0x35} }, // CoClass
  {0xF5DB038E, 0xA4EE, 0x473F,{ 0xAE, 0x89, 0x69, 0x3F, 0x58, 0x7F, 0x05, 0xAD} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEChecker::DEF_CTL_INTF = {0x96BACF1D, 0xDF2F, 0x4248,{ 0xB5, 0x22, 0x08, 0x6F, 0xE0, 0x0E, 0x6F, 0x4C} };
TNoParam TEChecker::OptParam;

static inline void ValidCtrCheck(TEChecker *)
{
   delete new TEChecker((TComponent*)(0));
};

void __fastcall TEChecker::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEChecker::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DECheckerDisp __fastcall TEChecker::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEChecker::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEChecker::SetTolerance(long ToleranceX, long ToleranceY)
{
  GetDefaultInterface()->SetTolerance(ToleranceX, ToleranceY);
}

void __fastcall TEChecker::Attach(LPDISPATCH Src)
{
  GetDefaultInterface()->Attach(Src);
}

void __fastcall TEChecker::Register(void)
{
  GetDefaultInterface()->Register();
}

void __fastcall TEChecker::Learn(Evision_tlb::enumOcvLearningMode LearningMode)
{
  GetDefaultInterface()->Learn(LearningMode);
}

void __fastcall TEChecker::EmptyPathNames(void)
{
  GetDefaultInterface()->EmptyPathNames();
}

void __fastcall TEChecker::AddPathName(BSTR PathName)
{
  GetDefaultInterface()->AddPathName(PathName);
}

void __fastcall TEChecker::BatchLearn(Evision_tlb::enumOcvLearningMode LearningMode)
{
  GetDefaultInterface()->BatchLearn(LearningMode);
}

void __fastcall TEChecker::Draw(LPDISPATCH BW8Image, Evision_tlb::enumOcvDrawingMode DrawingMode, 
                                TOLEBOOL Handles)
{
  GetDefaultInterface()->Draw(BW8Image, DrawingMode, Handles);
}

void __fastcall TEChecker::DrawToDC(OLE_HANDLE hDC, Evision_tlb::enumOcvDrawingMode DrawingMode, 
                                    TOLEBOOL Handles)
{
  GetDefaultInterface()->DrawToDC(hDC, DrawingMode, Handles);
}

TOLEBOOL __fastcall TEChecker::HitTest(long X, long Y)
{
  return GetDefaultInterface()->HitTest(X, Y);
}

void __fastcall TEChecker::Drag(long X, long Y)
{
  GetDefaultInterface()->Drag(X, Y);
}

void __fastcall TEChecker::DragSpecific(long X, long Y, Evision_tlb::enumROIHit HitROI, 
                                        Evision_tlb::enumHandleNames HitHandle)
{
  GetDefaultInterface()->DragSpecific(X, Y, HitROI, HitHandle);
}

void __fastcall TEChecker::Save(BSTR PathName)
{
  GetDefaultInterface()->Save(PathName);
}

void __fastcall TEChecker::Load(BSTR PathName)
{
  GetDefaultInterface()->Load(PathName);
}

void __fastcall TEChecker::Inspect(LPDISPATCH CodedImage)
{
  GetDefaultInterface()->Inspect(CodedImage);
}

void __fastcall TEChecker::DrawRegisteredImageToDC(OLE_HANDLE hDC, float f32ZoomX, float f32ZoomY)
{
  GetDefaultInterface()->DrawRegisteredImageToDC(hDC, f32ZoomX, f32ZoomY);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEBarCode which
// allows "EBarCode Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEBarCode::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EEA, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EE9, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEBarCode::DEF_CTL_INTF = {0x9AE86EE8, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEBarCode::OptParam;

static inline void ValidCtrCheck(TEBarCode *)
{
   delete new TEBarCode((TComponent*)(0));
};

void __fastcall TEBarCode::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEBarCode::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEBarCodeDisp __fastcall TEBarCode::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEBarCode::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEBarCode::Set(float CenterX, float CenterY, float SizeX, float SizeY, float Angle)
{
  GetDefaultInterface()->Set(CenterX, CenterY, SizeX, SizeY, Angle);
}

void __fastcall TEBarCode::SetReadingCenter(float RelativeX, float RelativeY)
{
  GetDefaultInterface()->SetReadingCenter(RelativeX, RelativeY);
}

void __fastcall TEBarCode::SetReadingSize(float RelativeWidth, float RelativeHeight)
{
  GetDefaultInterface()->SetReadingSize(RelativeWidth, RelativeHeight);
}

void __fastcall TEBarCode::Detect(LPDISPATCH Src)
{
  GetDefaultInterface()->Detect(Src);
}

void __fastcall TEBarCode::Decode(Evision_tlb::enumBrcSymbologies Symbology, BSTR* String)
{
  GetDefaultInterface()->Decode(Symbology, String);
}

void __fastcall TEBarCode::Draw(LPDISPATCH BW8Image, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                TOLEBOOL Daughters)
{
  GetDefaultInterface()->Draw(BW8Image, DrawingMode, Daughters);
}

TOLEBOOL __fastcall TEBarCode::HitTest(TOLEBOOL Daughters)
{
  return GetDefaultInterface()->HitTest(Daughters);
}

void __fastcall TEBarCode::Drag(long CursorX, long CursorY)
{
  GetDefaultInterface()->Drag(CursorX, CursorY);
}

void __fastcall TEBarCode::Read(LPDISPATCH BW8Image, BSTR* String)
{
  GetDefaultInterface()->Read(BW8Image, String);
}

BSTR __fastcall TEBarCode::GetSymbologyName(Evision_tlb::enumBrcSymbologies Symbology)
{
  return GetDefaultInterface()->GetSymbologyName(Symbology);
}

void __fastcall TEBarCode::DrawToDC(OLE_HANDLE hDC, Evision_tlb::enumInsDrawingMode DrawingMode, 
                                    TOLEBOOL Daughters)
{
  GetDefaultInterface()->DrawToDC(hDC, DrawingMode, Daughters);
}

void __fastcall TEBarCode::GetDecodedRectangle(float* f32CenterX, float* f32CenterY, float* f32Width, 
                                               float* f32Height, float* f32Angle)
{
  GetDefaultInterface()->GetDecodedRectangle(f32CenterX, f32CenterY, f32Width, f32Height, f32Angle);
}

void __fastcall TEBarCode::GetDecodedRectangleForSymbology(Evision_tlb::enumBrcSymbologies eSymbology, 
                                                           float* f32CenterX, float* f32CenterY, 
                                                           float* f32Width, float* f32Height, 
                                                           float* f32Angle)
{
  GetDefaultInterface()->GetDecodedRectangleForSymbology(eSymbology, f32CenterX, f32CenterY, 
                                                         f32Width, f32Height, f32Angle);
}

TOLEBOOL __fastcall TEBarCode::GetDecodedDirection(Evision_tlb::enumBrcSymbologies symbol)
{
  return GetDefaultInterface()->GetDecodedDirection(symbol);
}

Evision_tlb::enumBrcSymbologies __fastcall TEBarCode::Get_DecodedSymbology(long Index)
{
  return GetDefaultInterface()->get_DecodedSymbology(Index);
}

float __fastcall TEBarCode::Get_DecodedAngleForSymbology(Evision_tlb::enumBrcSymbologies eSymbology)
{
  return GetDefaultInterface()->get_DecodedAngleForSymbology(eSymbology);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEMatrixCode which
// allows "EMatrixCode Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEMatrixCode::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x9AE86EC2, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // CoClass
  {0x9AE86EC1, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEMatrixCode::DEF_CTL_INTF = {0x9AE86EC0, 0x97BC, 0x11D1,{ 0xA4, 0x9B, 0x00, 0x00, 0x21, 0x63, 0x31, 0x68} };
TNoParam TEMatrixCode::OptParam;

static inline void ValidCtrCheck(TEMatrixCode *)
{
   delete new TEMatrixCode((TComponent*)(0));
};

void __fastcall TEMatrixCode::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEMatrixCode::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEMatrixCodeDisp __fastcall TEMatrixCode::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEMatrixCode::Load(BSTR PathName)
{
  GetDefaultInterface()->Load(PathName);
}

void __fastcall TEMatrixCode::SetCellPitch(float PitchX, float PitchY)
{
  GetDefaultInterface()->SetCellPitch(PitchX, PitchY);
}

void __fastcall TEMatrixCode::ResetCorners(void)
{
  GetDefaultInterface()->ResetCorners();
}

void __fastcall TEMatrixCode::DrawCorners(LPDISPATCH EImageBW8)
{
  GetDefaultInterface()->DrawCorners(EImageBW8);
}

Evision_tlb::enumMxcHandles __fastcall TEMatrixCode::HitTest(LPDISPATCH EImageBW8, long CursorX, 
                                                             long CursorY)
{
  return GetDefaultInterface()->HitTest(EImageBW8, CursorX, CursorY);
}

void __fastcall TEMatrixCode::Drag(LPDISPATCH EImageBW8, Evision_tlb::enumMxcHandles EHandle, 
                                   long CursorX, long CursorY)
{
  GetDefaultInterface()->Drag(EImageBW8, EHandle, CursorX, CursorY);
}

void __fastcall TEMatrixCode::Draw(LPDISPATCH EImageBW8)
{
  GetDefaultInterface()->Draw(EImageBW8);
}

void __fastcall TEMatrixCode::Save(BSTR PathName)
{
  GetDefaultInterface()->Save(PathName);
}

void __fastcall TEMatrixCode::AutoDetect(LPDISPATCH BW8SrcImage, 
                                         Evision_tlb::enumMxcSymbolSize SymbolSize, 
                                         Evision_tlb::enumMxcSymbolOrientation SymbolOrientation, 
                                         Evision_tlb::enumMxcSymbolContrast Contrast, 
                                         BSTR* DstString)
{
  GetDefaultInterface()->AutoDetect(BW8SrcImage, SymbolSize, SymbolOrientation, Contrast, DstString);
}

void __fastcall TEMatrixCode::ReadSymbol(LPDISPATCH BW8SrcImage, BSTR* DstString)
{
  GetDefaultInterface()->ReadSymbol(BW8SrcImage, DstString);
}

void __fastcall TEMatrixCode::GetCellPitch(float* PitchX, float* PitchY)
{
  GetDefaultInterface()->GetCellPitch(PitchX, PitchY);
}

void __fastcall TEMatrixCode::SetPrintGrowthRange(float f32Minimum, float f32Nominal, 
                                                  float f32Maximum)
{
  GetDefaultInterface()->SetPrintGrowthRange(f32Minimum, f32Nominal, f32Maximum);
}

void __fastcall TEMatrixCode::DrawErrors(LPDISPATCH EImageBW8)
{
  GetDefaultInterface()->DrawErrors(EImageBW8);
}

void __fastcall TEMatrixCode::DrawCornersErrors(LPDISPATCH EImageBW8)
{
  GetDefaultInterface()->DrawCornersErrors(EImageBW8);
}

void __fastcall TEMatrixCode::AutoReadSymbol(LPDISPATCH BW8SrcImage, BSTR* DstString)
{
  GetDefaultInterface()->AutoReadSymbol(BW8SrcImage, DstString);
}

void __fastcall TEMatrixCode::AboutBox(void)
{
  GetDefaultInterface()->AboutBox();
}

void __fastcall TEMatrixCode::AutoReadSymbol2(LPDISPATCH Image, BSTR* DstString)
{
  GetDefaultInterface()->AutoReadSymbol2(Image, DstString);
}

void __fastcall TEMatrixCode::DrawToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawToDC(hDC);
}

void __fastcall TEMatrixCode::DrawErrorsToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawErrorsToDC(hDC);
}

void __fastcall TEMatrixCode::DrawCornersToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawCornersToDC(hDC);
}

void __fastcall TEMatrixCode::DrawCornersErrorsToDC(OLE_HANDLE hDC)
{
  GetDefaultInterface()->DrawCornersErrorsToDC(hDC);
}

LPDISPATCH __fastcall TEMatrixCode::Get_sourceImage()
{
  return GetDefaultInterface()->get_sourceImage();
}

void __fastcall TEMatrixCode::Set_sourceImage(LPDISPATCH param)
{
  GetDefaultInterface()->set_sourceImage(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TMatrixCode which
// allows "MatrixCode Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TMatrixCode::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0xB1CE755E, 0x7163, 0x487D,{ 0x9B, 0x01, 0x4C, 0xE7, 0x9A, 0xFF, 0x12, 0xD6} }, // CoClass
  {0x3C8DD6DF, 0x2D25, 0x44CC,{ 0x8B, 0x43, 0x5D, 0x5F, 0x82, 0x48, 0xD0, 0x9D} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TMatrixCode::DEF_CTL_INTF = {0x6BA40113, 0x6782, 0x4920,{ 0xA7, 0x64, 0xD7, 0x16, 0x64, 0x69, 0xF8, 0x1B} };
TNoParam TMatrixCode::OptParam;

static inline void ValidCtrCheck(TMatrixCode *)
{
   delete new TMatrixCode((TComponent*)(0));
};

void __fastcall TMatrixCode::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TMatrixCode::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DMatrixCodeDisp __fastcall TMatrixCode::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TMatrixCode::DrawToDC(OLE_HANDLE hDC, float f32ZoomX, float f32ZoomY, float f32PanX, 
                                      float f32PanY)
{
  GetDefaultInterface()->DrawToDC(hDC, f32ZoomX, f32ZoomY, f32PanX, f32PanY);
}

void __fastcall TMatrixCode::DrawErrorsToDC(OLE_HANDLE hDC, float f32ZoomX, float f32ZoomY, 
                                            float f32PanX, float f32PanY)
{
  GetDefaultInterface()->DrawErrorsToDC(hDC, f32ZoomX, f32ZoomY, f32PanX, f32PanY);
}

void __fastcall TMatrixCode::Save(BSTR FileName)
{
  GetDefaultInterface()->Save(FileName);
}

void __fastcall TMatrixCode::Load(BSTR FileName)
{
  GetDefaultInterface()->Load(FileName);
}

void __fastcall TMatrixCode::Draw(LPDISPATCH Img)
{
  GetDefaultInterface()->Draw(Img);
}

void __fastcall TMatrixCode::DrawErrors(LPDISPATCH Img)
{
  GetDefaultInterface()->DrawErrors(Img);
}

float __fastcall TMatrixCode::GetCornersX(long Index)
{
  return GetDefaultInterface()->GetCornersX(Index);
}

float __fastcall TMatrixCode::GetCornersY(long Index)
{
  return GetDefaultInterface()->GetCornersY(Index);
}

LPDISPATCH __fastcall TMatrixCode::Get_Obj()
{
  return GetDefaultInterface()->get_Obj();
}

void __fastcall TMatrixCode::Set_Obj(LPDISPATCH param)
{
  GetDefaultInterface()->set_Obj(param);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TMatrixCodeReader which
// allows "MatrixCodeReader Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TMatrixCodeReader::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x4DBAF9C2, 0x185E, 0x4E50,{ 0x86, 0x05, 0xA2, 0x8F, 0xBC, 0x6A, 0xF2, 0xAF} }, // CoClass
  {0x156368C4, 0xD6DC, 0x4820,{ 0x8E, 0x6B, 0xAA, 0xDD, 0xAF, 0x8B, 0x75, 0x5A} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TMatrixCodeReader::DEF_CTL_INTF = {0x6722ABD6, 0xDD8B, 0x43D8,{ 0xBA, 0x22, 0x51, 0xE7, 0x35, 0x14, 0x3C, 0x9B} };
TNoParam TMatrixCodeReader::OptParam;

static inline void ValidCtrCheck(TMatrixCodeReader *)
{
   delete new TMatrixCodeReader((TComponent*)(0));
};

void __fastcall TMatrixCodeReader::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TMatrixCodeReader::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DMatrixCodeReaderDisp __fastcall TMatrixCodeReader::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TMatrixCodeReader::Reset(void)
{
  GetDefaultInterface()->Reset();
}

void __fastcall TMatrixCodeReader::Save(BSTR FileName)
{
  GetDefaultInterface()->Save(FileName);
}

void __fastcall TMatrixCodeReader::Load(BSTR FileName)
{
  GetDefaultInterface()->Load(FileName);
}

void __fastcall TMatrixCodeReader::Read(LPDISPATCH EBW8ROI, LPDISPATCH MatrixCode)
{
  GetDefaultInterface()->Read(EBW8ROI, MatrixCode);
}

void __fastcall TMatrixCodeReader::Learn(LPDISPATCH eroibw8, LPDISPATCH MatrixCode)
{
  GetDefaultInterface()->Learn(eroibw8, MatrixCode);
}

void __fastcall TMatrixCodeReader::LearnMore(LPDISPATCH eroibw8, LPDISPATCH MatrixCode)
{
  GetDefaultInterface()->LearnMore(eroibw8, MatrixCode);
}

void __fastcall TMatrixCodeReader::AddSearchLogicalSize(Evision_tlb::enumMatrixCodeLogicalSize elem)
{
  GetDefaultInterface()->AddSearchLogicalSize(elem);
}

void __fastcall TMatrixCodeReader::RemoveSearchLogicalSize(Evision_tlb::enumMatrixCodeLogicalSize elem)
{
  GetDefaultInterface()->RemoveSearchLogicalSize(elem);
}

void __fastcall TMatrixCodeReader::ClearSearchLogicalSize(void)
{
  GetDefaultInterface()->ClearSearchLogicalSize();
}

void __fastcall TMatrixCodeReader::ContainsSearchLogicalSize(Evision_tlb::enumMatrixCodeLogicalSize elem)
{
  GetDefaultInterface()->ContainsSearchLogicalSize(elem);
}

void __fastcall TMatrixCodeReader::AddSearchContrast(Evision_tlb::enumMatrixCodeContrast elem)
{
  GetDefaultInterface()->AddSearchContrast(elem);
}

void __fastcall TMatrixCodeReader::RemoveSearchContrast(Evision_tlb::enumMatrixCodeContrast elem)
{
  GetDefaultInterface()->RemoveSearchContrast(elem);
}

void __fastcall TMatrixCodeReader::ClearSearchContrast(void)
{
  GetDefaultInterface()->ClearSearchContrast();
}

void __fastcall TMatrixCodeReader::ContainsSearchContrast(Evision_tlb::enumMatrixCodeContrast elem)
{
  GetDefaultInterface()->ContainsSearchContrast(elem);
}

void __fastcall TMatrixCodeReader::AddSearchFlipping(Evision_tlb::enumMatrixCodeFlipping elem)
{
  GetDefaultInterface()->AddSearchFlipping(elem);
}

void __fastcall TMatrixCodeReader::RemoveSearchFlipping(Evision_tlb::enumMatrixCodeFlipping elem)
{
  GetDefaultInterface()->RemoveSearchFlipping(elem);
}

void __fastcall TMatrixCodeReader::ClearSearchFlipping(void)
{
  GetDefaultInterface()->ClearSearchFlipping();
}

void __fastcall TMatrixCodeReader::ContainsSearchFlipping(Evision_tlb::enumMatrixCodeFlipping elem)
{
  GetDefaultInterface()->ContainsSearchFlipping(elem);
}

void __fastcall TMatrixCodeReader::AddSearchFamily(Evision_tlb::enumMatrixCodeFamily elem)
{
  GetDefaultInterface()->AddSearchFamily(elem);
}

void __fastcall TMatrixCodeReader::RemoveSearchFamily(Evision_tlb::enumMatrixCodeFamily elem)
{
  GetDefaultInterface()->RemoveSearchFamily(elem);
}

void __fastcall TMatrixCodeReader::ClearSearchFamily(void)
{
  GetDefaultInterface()->ClearSearchFamily();
}

void __fastcall TMatrixCodeReader::ContainsSearchFamily(Evision_tlb::enumMatrixCodeFamily elem)
{
  GetDefaultInterface()->ContainsSearchFamily(elem);
}

TOLEBOOL __fastcall TMatrixCodeReader::Get_LearnMask(Evision_tlb::enumMatrixCodeReaderLearnParams Index)
{
  return GetDefaultInterface()->get_LearnMask(Index);
}

void __fastcall TMatrixCodeReader::Set_LearnMask(Evision_tlb::enumMatrixCodeReaderLearnParams Index, 
                                                 TOLEBOOL Param2)
{
  GetDefaultInterface()->set_LearnMask(Index, Param2);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TEJpegHandler which
// allows "EJpegHandler Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TEJpegHandler::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0xAB965277, 0x3BE7, 0x4D1C,{ 0x95, 0x01, 0x30, 0xC1, 0xFA, 0xC8, 0x1C, 0xF9} }, // CoClass
  {0x4FDCFEA8, 0xA505, 0x47F3,{ 0xBE, 0xF0, 0x48, 0xFF, 0x5B, 0xA2, 0xBB, 0x53} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TEJpegHandler::DEF_CTL_INTF = {0x7CFE9C6F, 0x9836, 0x4938,{ 0xBB, 0xD4, 0xD8, 0x36, 0xC2, 0xDD, 0xBC, 0xB8} };
TNoParam TEJpegHandler::OptParam;

static inline void ValidCtrCheck(TEJpegHandler *)
{
   delete new TEJpegHandler((TComponent*)(0));
};

void __fastcall TEJpegHandler::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TEJpegHandler::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DEJpegHandlerDisp __fastcall TEJpegHandler::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TEJpegHandler::LoadFJfif(BSTR path)
{
  GetDefaultInterface()->LoadFJfif(path);
}

void __fastcall TEJpegHandler::DecompressToImage(LPDISPATCH axC24Img)
{
  GetDefaultInterface()->DecompressToImage(axC24Img);
}

void __fastcall TEJpegHandler::SetJpegFields(long upa, long ups, long doa, long dos)
{
  GetDefaultInterface()->SetJpegFields(upa, ups, doa, dos);
}

void __fastcall TEJpegHandler::UpdateSizeAndTimestamp(long ffs, long nfs, long ts0, long ts1)
{
  GetDefaultInterface()->UpdateSizeAndTimestamp(ffs, nfs, ts0, ts1);
}

void __fastcall TEJpegHandler::SaveFJfif(BSTR path)
{
  GetDefaultInterface()->SaveFJfif(path);
}

void __fastcall TEJpegHandler::SaveSJfif(BSTR path)
{
  GetDefaultInterface()->SaveSJfif(path);
}

void __fastcall TEJpegHandler::LoadSJfif(BSTR path)
{
  GetDefaultInterface()->LoadSJfif(path);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TMjpegAviWriter which
// allows "MjpegAviWriter Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TMjpegAviWriter::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0xA2123CD6, 0xCD98, 0x480D,{ 0x99, 0x58, 0xA9, 0x1B, 0x82, 0xDC, 0x49, 0x77} }, // CoClass
  {0xDA665497, 0xAF48, 0x4807,{ 0xAE, 0x0E, 0xB0, 0x04, 0xC9, 0xA2, 0xB3, 0x1D} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TMjpegAviWriter::DEF_CTL_INTF = {0x12B668DB, 0x7C4E, 0x4662,{ 0xBD, 0x71, 0x48, 0x73, 0xF8, 0x26, 0xDB, 0xE0} };
TNoParam TMjpegAviWriter::OptParam;

static inline void ValidCtrCheck(TMjpegAviWriter *)
{
   delete new TMjpegAviWriter((TComponent*)(0));
};

void __fastcall TMjpegAviWriter::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TMjpegAviWriter::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DMjpegAviWriterDisp __fastcall TMjpegAviWriter::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TMjpegAviWriter::OpenMovie(BSTR path, long Width, long Height, long usPerImage)
{
  GetDefaultInterface()->OpenMovie(path, Width, Height, usPerImage);
}

void __fastcall TMjpegAviWriter::OpenMovieWithSize(BSTR path, long Width, long Height, 
                                                   long usPerImage, long maxSize)
{
  GetDefaultInterface()->OpenMovieWithSize(path, Width, Height, usPerImage, maxSize);
}

void __fastcall TMjpegAviWriter::CloseMovie(void)
{
  GetDefaultInterface()->CloseMovie();
}

long __fastcall TMjpegAviWriter::AddImage(LPDISPATCH axC24Img)
{
  return GetDefaultInterface()->AddImage(axC24Img);
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TAviWriter which
// allows "AviWriter Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TAviWriter::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0x53E6D356, 0x1F66, 0x48C3,{ 0x9E, 0x6B, 0xE3, 0x08, 0x4C, 0xE1, 0x39, 0xA5} }, // CoClass
  {0xED9CDB33, 0xDF09, 0x4704,{ 0x8F, 0x15, 0x1F, 0xA3, 0x32, 0xCD, 0xCD, 0x23} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TAviWriter::DEF_CTL_INTF = {0xCB0014CD, 0x6C68, 0x4EB0,{ 0x8B, 0xA1, 0x81, 0xAD, 0xE9, 0x97, 0x59, 0x48} };
TNoParam TAviWriter::OptParam;

static inline void ValidCtrCheck(TAviWriter *)
{
   delete new TAviWriter((TComponent*)(0));
};

void __fastcall TAviWriter::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TAviWriter::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DAviWriterDisp __fastcall TAviWriter::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TAviWriter::OpenMovie(BSTR FileName, LPDISPATCH camera)
{
  GetDefaultInterface()->OpenMovie(FileName, camera);
}

void __fastcall TAviWriter::OpenMovieWithSize(BSTR FileName, LPDISPATCH camera, long maxSizeInMB, 
                                              long Version)
{
  GetDefaultInterface()->OpenMovieWithSize(FileName, camera, maxSizeInMB, Version);
}

long __fastcall TAviWriter::AddImage(LPDISPATCH Image)
{
  return GetDefaultInterface()->AddImage(Image);
}

void __fastcall TAviWriter::CloseMovie(void)
{
  GetDefaultInterface()->CloseMovie();
}



// *********************************************************************//
// OCX PROXY CLASS IMPLEMENTATION
// (The following variables/methods implement the class TECompressedData which
// allows "ECompressedData Control" to be hosted in CBuilder IDE/apps).
// *********************************************************************//
TControlData TECompressedData::CControlData =
{
  // GUID of CoClass and Event Interface of Control
  {0xD07E78A8, 0xF985, 0x4D92,{ 0x99, 0x0A, 0x87, 0x4A, 0x62, 0xBC, 0x76, 0xAE} }, // CoClass
  {0x646DA4A8, 0x7A66, 0x457C,{ 0x8E, 0x73, 0x85, 0xEB, 0x48, 0xF6, 0xE3, 0x58} }, // Events

  // Count of Events and array of their DISPIDs
  0, NULL,

  // Pointer to Runtime License string
  NULL,  // HRESULT(0x80004005)

  // Flags for OnChanged PropertyNotification
  0x00000000,
  300,// (IDE Version)

  // Count of Font Prop and array of their DISPIDs
  0, NULL,

  // Count of Pict Prop and array of their DISPIDs
  0, NULL,
  0, // Reserved
  0, // Instance count (used internally)
  0, // List of Enum descriptions (internal)
};

GUID     TECompressedData::DEF_CTL_INTF = {0xCC7B39EC, 0x6B7C, 0x46A7,{ 0x93, 0xA7, 0x59, 0xE2, 0x72, 0x47, 0xD8, 0x58} };
TNoParam TECompressedData::OptParam;

static inline void ValidCtrCheck(TECompressedData *)
{
   delete new TECompressedData((TComponent*)(0));
};

void __fastcall TECompressedData::InitControlData()
{
  ControlData = &CControlData;
};

void __fastcall TECompressedData::CreateControl()
{
  if (!m_OCXIntf)
  {
    _ASSERTE(DefaultDispatch);
    DefaultDispatch->QueryInterface(DEF_CTL_INTF, (LPVOID*)&m_OCXIntf);
  }
};

_DECompressedDataDisp __fastcall TECompressedData::GetDefaultInterface()
{
  CreateControl();
  return m_OCXIntf;
};

void __fastcall TECompressedData::_SetHandle(long Handle)
{
  GetDefaultInterface()->_SetHandle(Handle);
}


};     // namespace Evision_tlb


// *********************************************************************//
// The Register function is invoked by the IDE when this module is 
// installed in a Package. It provides the list of Components (including
// OCXes) implemented by this module. The following implementation
// informs the IDE of the OCX proxy classes implemented here.
// *********************************************************************//
namespace Evision_ocx
{

void __fastcall PACKAGE Register()
{
  // [52]
  TComponentClass cls_ocx[] = {
                              __classid(Evision_tlb::TEasyMain), 
                              __classid(Evision_tlb::TEBW8Image), 
                              __classid(Evision_tlb::TEBW16Image), 
                              __classid(Evision_tlb::TEBW1Image), 
                              __classid(Evision_tlb::TEC15Image), 
                              __classid(Evision_tlb::TEC16Image), 
                              __classid(Evision_tlb::TEC24Image), 
                              __classid(Evision_tlb::TEBW8ROI), 
                              __classid(Evision_tlb::TEBW16ROI), 
                              __classid(Evision_tlb::TEBW1ROI), 
                              __classid(Evision_tlb::TEC15ROI), 
                              __classid(Evision_tlb::TEC16ROI), 
                              __classid(Evision_tlb::TEC24ROI), 
                              __classid(Evision_tlb::TEImageSequence), 
                              __classid(Evision_tlb::TEBW8Vector), 
                              __classid(Evision_tlb::TEBW16Vector), 
                              __classid(Evision_tlb::TEBW32Vector), 
                              __classid(Evision_tlb::TEC24Vector), 
                              __classid(Evision_tlb::TEPathVector), 
                              __classid(Evision_tlb::TEBWHistogramVector), 
                              __classid(Evision_tlb::TEBW8PathVector), 
                              __classid(Evision_tlb::TEC24PathVector), 
                              __classid(Evision_tlb::TEPeaksVector), 
                              __classid(Evision_tlb::TEKernel), 
                              __classid(Evision_tlb::TEMovingAverage), 
                              __classid(Evision_tlb::TEColorLookup), 
                              __classid(Evision_tlb::TECodedImage), 
                              __classid(Evision_tlb::TEWorldShape), 
                              __classid(Evision_tlb::TEFrameShape), 
                              __classid(Evision_tlb::TEPointGauge), 
                              __classid(Evision_tlb::TELineGauge), 
                              __classid(Evision_tlb::TECircleGauge), 
                              __classid(Evision_tlb::TERectangleGauge), 
                              __classid(Evision_tlb::TEWedgeGauge), 
                              __classid(Evision_tlb::TEPointMeasure), 
                              __classid(Evision_tlb::TELineMeasure), 
                              __classid(Evision_tlb::TECircleMeasure), 
                              __classid(Evision_tlb::TEMatch), 
                              __classid(Evision_tlb::TEFind), 
                              __classid(Evision_tlb::TEOCR), 
                              __classid(Evision_tlb::TEOCV), 
                              __classid(Evision_tlb::TEOCVChar), 
                              __classid(Evision_tlb::TEOCVText), 
                              __classid(Evision_tlb::TEChecker), 
                              __classid(Evision_tlb::TEBarCode), 
                              __classid(Evision_tlb::TEMatrixCode), 
                              __classid(Evision_tlb::TMatrixCode), 
                              __classid(Evision_tlb::TMatrixCodeReader), 
                              __classid(Evision_tlb::TEJpegHandler), 
                              __classid(Evision_tlb::TMjpegAviWriter), 
                              __classid(Evision_tlb::TAviWriter), 
                              __classid(Evision_tlb::TECompressedData)
                           };
  RegisterComponents("ActiveX", cls_ocx,
                     sizeof(cls_ocx)/sizeof(cls_ocx[0])-1);
}

};     // namespace Evision_ocx
