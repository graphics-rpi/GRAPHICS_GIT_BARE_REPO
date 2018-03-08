Attribute VB_Name = "Bitmap"
'==============================================================================
' Copyright (C) 2007 Prosilica.  All Rights Reserved.
'
' Redistribution of this header file, in original or modified form, without
' prior written consent of Prosilica is prohibited.
'
'=============================================================================
'
' File:         Bitmap.bas
'
' Project/lib:  PvAPI
'
' Target:       Visual Basic 6.0
'
' Description:  Provides Simple Bitmap support within Visual Basic, including
'               conversion of a frame into a Bitmap.
'
' Notes:
'
' The functions here are:
'
' Function CreateDIB(uWidth As Long, uHeight As Long, ByRef Bitmap As DIBitmap) As Boolean
'
' Function DeleteDIB(ByRef Bitmap As DIBitmap) As Boolean
'
' Function SaveDIB(ByRef Bitmap As DIBitmap, ByRef FilePath As String) As Boolean
'
' Function FillInDIB(ByRef Bitmap As DIBitmap, ByRef Frame As tPvFrame) As Boolean
'
'==============================================================================
'
' THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
' WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
' NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
' DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
' INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
' LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
' OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF
' LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
' NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
' EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'
'==============================================================================
' dd/mon/yy     Notes
'------------------------------------------------------------------------------
' 11/Oct/07     Original.
'==============================================================================

Option Explicit

''===== TYPE DEFINITIONS ======================================================

Private Type SafeArrayBound
    cElements As Long
    lLbound As Long
End Type

Private Type SafeArray
    cDims As Integer
    fFeatures As Integer
    cbElements As Long
    cLocks As Long
    pvData As Long
    rgsabound(0 To 1) As SafeArrayBound
End Type

Private Type ExBytes

    Data() As Byte
    Spec As SafeArray

End Type

Private Type ExShorts

    Data() As Integer
    Spec As SafeArray

End Type

Public Type RgbQuad

    rgbBlue As Byte
    rgbGreen As Byte
    rgbRed As Byte
    rgbReserved As Byte

End Type

Public Type DIBitmapInfoHeader

    biSize          As Long
    biWidth         As Long
    biHeight        As Long
    biPlanes        As Integer
    biBitCount      As Integer
    biCompression   As Long
    biSizeImage     As Long
    biXPelsPerMeter As Long
    biYPelsPerMeter As Long
    biClrUsed       As Long
    biClrImportant  As Long
    
End Type

Private Type DIBitmapFileHeader
    bfType       As Integer
    bfSize       As Long
    bfReserved1  As Integer
    bfReserved2  As Integer
    bfOffBits    As Long
End Type


Public Type DIBitmapInfo

    bmiHeader    As DIBitmapInfoHeader
    bmiColors(1) As RgbQuad

End Type

Public Type DIBitmap

    hHandle As Long
    Info    As DIBitmapInfo
    Bits    As ExBytes

End Type

''===== GDI32 FUNCTIONS DEFINITIONS =========================================

Declare Function GetObject Lib "gdi32" Alias "GetObjectA" (ByVal hObject As Long, ByVal nCount As Long, ByRef lpObject As Any) As Long

Declare Function GetBitmapBits Lib "gdi32" (ByVal hBitmap As Long, ByVal dwCount As Long, ByRef lpBits As Any) As Long

Declare Function SetBitmapBits Lib "gdi32" (ByVal hBitmap As Long, ByVal dwCount As Long, ByRef lpBits As Any) As Long

Declare Function CreateDIBSection Lib "gdi32" (ByVal hdc As Long, ByRef pbmi As DIBitmapInfo, ByVal iUsage As Long, ByRef pBits As Long, ByVal hSection As Long, ByVal dwOffset As Long) As Long

Declare Function DeleteObject Lib "gdi32" (ByVal hObject As Long) As Boolean

''===== KERNEL32 FUNCTIONS DEFINITIONS =========================================

Declare Sub Memcpy Lib "kernel32" Alias "RtlMoveMemory" (ByVal pDest As Long, ByVal pSrc As Long, ByVal ByteLen As Long)

Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" (pDest As Any, pSrc As Any, ByVal ByteLen As Long)


''===== FUNCTIONS DEFINITIONS =========================================

Private Function VarPtrArray(arr As Variant) As Long
    CopyMemory VarPtrArray, ByVal VarPtr(arr) + 8, 4
End Function

'' compute requiered padding for Windows' bitmap
Function Padding(uWidth As Long) As Long

    Padding = ((uWidth + 3) And (Not 3)) - uWidth

End Function

'' return minimum of two values
Function min(ByVal uVal1 As Long, ByVal uVal2 As Long) As Long

    If uVal1 > uVal2 Then
        min = uVal2
    Else
        min = uVal1
    End If

End Function

'' return maximum of two values
Function max(ByVal uVal1 As Long, ByVal uVal2 As Long) As Long

    If uVal1 > uVal2 Then
        max = uVal1
    Else
        max = uVal2
    End If

End Function

'' return a value shifted right by a specified number of bits
Function BitShift(ByVal uValue As Integer, ByVal Shift As Byte) As Byte

    Dim i As Long

    For i = 1 To Shift

        uValue = uValue \ 2
    
    Next
        
    BitShift = uValue

End Function

'' convert a YUV value to RGB
Sub YUV2RGB(ByVal Y As Long, ByVal U As Long, ByVal V As Long, ByRef R As Long, ByRef G As Long, ByRef B As Long)

    U = U - 128
    V = V - 128
    
    R = min(max(0, Y + 1.370705 * V), 255)
    G = min(max(0, Y - 0.698001 * V - 0.337633 * U), 255)
    B = min(max(0, Y + 1.732446 * U), 255)

End Sub

'' link a local bytes array to a given buffer
Sub ExBytesSetup(ByRef ExArray As ExBytes, ByVal Address As Long, ByVal Length As Long)

    With ExArray.Spec
        .cbElements = 1
        .cDims = 1
        .rgsabound(0).lLbound = 0
        .rgsabound(0).cElements = Length
        .pvData = Address
    End With
            
    CopyMemory ByVal VarPtrArray(ExArray.Data), VarPtr(ExArray.Spec), 4

End Sub

'' unlink a local bytes array to a given buffer
Sub ExBytesClear(ByRef ExArray As ExBytes)

    CopyMemory ByVal VarPtrArray(ExArray.Data), 0&, 4

End Sub

'' link a local shorts array to a given buffer
Sub ExShortsSetup(ByRef ExArray As ExShorts, ByVal Address As Long, ByVal Length As Long)

    With ExArray.Spec
        .cbElements = 2
        .cDims = 1
        .rgsabound(0).lLbound = 0
        .rgsabound(0).cElements = Length / 2
        .pvData = Address
    End With
            
    CopyMemory ByVal VarPtrArray(ExArray.Data), VarPtr(ExArray.Spec), 4

End Sub

'' unlink a local shorts array to a given buffer
Sub ExShortsClear(ByRef ExArray As ExShorts)

    CopyMemory ByVal VarPtrArray(ExArray.Data), 0&, 4

End Sub

'' Create a DIB bitmap of a given dimension (24bpp)
Function CreateDIB(uWidth As Long, uHeight As Long, ByRef Bitmap As DIBitmap) As Boolean

    Dim uLength As Long
    Dim bResult As Boolean
    Dim Bits As Long
    
    uLength = (((uWidth * 3) + 3) And (Not 3)) * uHeight

    If Bitmap.hHandle <> 0 Then
        bResult = False
    Else
        
        Bitmap.Info.bmiHeader.biSize = 40            ' sizeof(BitmapInfoHeader)
        Bitmap.Info.bmiHeader.biPlanes = 1
        Bitmap.Info.bmiHeader.biCompression = 0      ' BI_RGB
        Bitmap.Info.bmiHeader.biSizeImage = uLength
        Bitmap.Info.bmiHeader.biXPelsPerMeter = 72
        Bitmap.Info.bmiHeader.biYPelsPerMeter = 72
        Bitmap.Info.bmiHeader.biClrUsed = 0
        Bitmap.Info.bmiHeader.biClrImportant = 0
        Bitmap.Info.bmiHeader.biWidth = uWidth
        Bitmap.Info.bmiHeader.biHeight = 0 - uHeight
        Bitmap.Info.bmiHeader.biBitCount = 24
        
        Bitmap.hHandle = CreateDIBSection(0, Bitmap.Info, 0, Bits, 0, 0)
        
        If Bitmap.hHandle <> 0 Then
        
            Call ExBytesSetup(Bitmap.Bits, Bits, uLength)
                 
            bResult = True
        Else
            bResult = False
        End If
        
    End If
    
    CreateDIB = bResult

End Function

'' delete an existing bitmap
Function DeleteDIB(ByRef Bitmap As DIBitmap) As Boolean

    Dim bResult As Boolean

    If Bitmap.hHandle = 0 Then
        bResult = False
    Else
    
        Call ExBytesClear(Bitmap.Bits)

        DeleteObject (Bitmap.hHandle)
    
        Bitmap.hHandle = 0
        
        bResult = True
        
    End If
        
    DeleteDIB = bResult

End Function

'' save an existing bitmap to a file (as a BMP)
Function SaveDIB(ByRef Bitmap As DIBitmap, ByRef FilePath As String) As Boolean

    Dim bResult As Boolean

    If Bitmap.hHandle = 0 Then
        bResult = False
    Else
    
        Dim hFile As Long
        Dim Header As DIBitmapFileHeader
        
        Header.bfType = 19778 '' "BM"
        Header.bfOffBits = Len(Header) + Len(Bitmap.Info)
        Header.bfSize = Header.bfOffBits + Bitmap.Info.bmiHeader.biSizeImage
        Header.bfReserved1 = 0
        Header.bfReserved2 = 0
        
        bResult = False
        
        On Error GoTo ErrorHandler
        
        hFile = FreeFile
        Open FilePath For Binary Access Write As #hFile
            Put #hFile, , Header
            Put #hFile, , Bitmap.Info
            Put #hFile, , Bitmap.Bits.Data
        Close #hFile
        
        bResult = True
        
ErrorHandler:
        
    End If
        
    SaveDIB = bResult
   
End Function

'' fill an existing bitmap with the content of a frame
Function FillInDIB(ByRef Bitmap As DIBitmap, ByRef Frame As tPvFrame) As Boolean

    Dim Remainder As Long
    Dim Shift As Byte
    Dim Color As Byte
    Dim Shorts As ExShorts
    Dim Bytes As ExBytes
    Dim Y1 As Long, Y2 As Long, Y3 As Long, Y4 As Long, U As Long, V As Long
    Dim R As Long
    Dim G As Long
    Dim B As Long
    Dim RowLen As Long
    Dim i As Long, j As Long, k As Long
    
    RowLen = Frame.Width * 3
    Remainder = Padding(RowLen)
    Shift = Frame.BitDepth - 8

    Select Case Frame.format

    Case Is = ePvFmtMono8
    
        Call ExBytesSetup(Bytes, Frame.ImageBuffer, Frame.ImageBufferSize)
               
        i = 0
        k = 0
               
        Do
        
            For j = 0 To Frame.Width - 1
            
                Bitmap.Bits.Data(k) = Bytes.Data(i)
                Bitmap.Bits.Data(k + 1) = Bytes.Data(i)
                Bitmap.Bits.Data(k + 2) = Bytes.Data(i)
                
                i = i + 1
                k = k + 3
            
            Next
            
            k = k + Remainder
        
        Loop While i <= UBound(Bytes.Data)
        
        Call ExBytesClear(Bytes)
        
        FillInDIB = True
        
    Case Is = ePvFmtMono16
    
        Call ExShortsSetup(Shorts, Frame.ImageBuffer, Frame.ImageBufferSize)
               
        i = 0
        k = 0
               
        Do
        
            For j = 0 To Frame.Width - 1
                          
                Color = BitShift(Shorts.Data(i), Shift)
                                           
                Bitmap.Bits.Data(k) = Color
                Bitmap.Bits.Data(k + 1) = Color
                Bitmap.Bits.Data(k + 2) = Color
                
                i = i + 1
                k = k + 3
            
            Next
            
            k = k + Remainder
        
        Loop While i <= UBound(Shorts.Data)
        
        Call ExShortsClear(Shorts)
        
        FillInDIB = True
    
    Case Is = ePvFmtBayer8
    
        Call PvUtilityColorInterpolate(Frame, VarPtr(Bitmap.Bits.Data(2)), VarPtr(Bitmap.Bits.Data(1)), VarPtr(Bitmap.Bits.Data(0)), 2, Remainder)
    
        FillInDIB = True
    
    Case Is = ePvFmtBayer16
        
        Call ExShortsSetup(Shorts, Frame.ImageBuffer, Frame.ImageBufferSize)
        Call ExBytesSetup(Bytes, Frame.ImageBuffer, Frame.ImageBufferSize)
                 
        For i = 0 To UBound(Shorts.Data)
        
            Bytes.Data(i) = BitShift(Shorts.Data(i), Shift)
            
        Next
        
        Frame.format = ePvFmtBayer8
    
        Call PvUtilityColorInterpolate(Frame, VarPtr(Bitmap.Bits.Data(2)), VarPtr(Bitmap.Bits.Data(1)), VarPtr(Bitmap.Bits.Data(0)), 2, Remainder)
        
        Call ExShortsClear(Shorts)
        Call ExBytesClear(Bytes)
        
        FillInDIB = True
        
    Case Is = ePvFmtRgb24
    
        Call ExBytesSetup(Bytes, Frame.ImageBuffer, Frame.ImageBufferSize)
        
        k = 0
        i = 0
        
        Do
        
            For j = 0 To Frame.Width - 1
                                                                             
                Bitmap.Bits.Data(k) = Bytes.Data(i + 2)
                Bitmap.Bits.Data(k + 1) = Bytes.Data(i + 1)
                Bitmap.Bits.Data(k + 2) = Bytes.Data(i)
                    
                i = i + 3
                k = k + 3
                
            Next
                
            k = k + Remainder
        
        Loop While i <= UBound(Bytes.Data)
        
        Call ExBytesClear(Bytes)
        
        FillInDIB = True
    
    Case Is = ePvFmtBgr24
    
        Call ExBytesSetup(Bytes, Frame.ImageBuffer, Frame.ImageBufferSize)
        
        k = 0
        i = 0
        
        Do
        
            Call Memcpy(VarPtr(Bitmap.Bits.Data(i)), VarPtr(Bytes.Data(k)), RowLen)
            k = k + RowLen
            i = i + RowLen + Remainder
            
        Loop While k <= UBound(Bytes.Data)
        
        Call ExBytesClear(Bytes)
        
        FillInDIB = True
    
    Case Is = ePvFmtRgba32
    
        Call ExBytesSetup(Bytes, Frame.ImageBuffer, Frame.ImageBufferSize)
        
        k = 0
        i = 0
        
        Do
        
            For j = 0 To Frame.Width - 1
                                                                             
                Bitmap.Bits.Data(k) = Bytes.Data(i + 2)
                Bitmap.Bits.Data(k + 1) = Bytes.Data(i + 1)
                Bitmap.Bits.Data(k + 2) = Bytes.Data(i)
                    
                i = i + 4
                k = k + 3
                
            Next
                
            k = k + Remainder
        
        Loop While i <= UBound(Bytes.Data)
        
        Call ExBytesClear(Bytes)
        
        FillInDIB = True
    
    Case Is = ePvFmtBgra32
    
        Call ExBytesSetup(Bytes, Frame.ImageBuffer, Frame.ImageBufferSize)
        
        k = 0
        i = 0
        
        Do
        
            For j = 0 To Frame.Width - 1
                                                                     
                Bitmap.Bits.Data(k) = Bytes.Data(i)
                Bitmap.Bits.Data(k + 1) = Bytes.Data(i + 1)
                Bitmap.Bits.Data(k + 2) = Bytes.Data(i + 2)
                    
                i = i + 4
                k = k + 3
                
            Next
                
            k = k + Remainder
        
        Loop While i <= UBound(Bytes.Data)
        
        Call ExBytesClear(Bytes)
        
        FillInDIB = True
    
    Case Is = ePvFmtRgb48
    
        Call ExShortsSetup(Shorts, Frame.ImageBuffer, Frame.ImageBufferSize)
        
        k = 0
        i = 0
        
        Do
        
            For j = 0 To Frame.Width - 1
                                                                             
                Bitmap.Bits.Data(k) = BitShift(Shorts.Data(i + 2), Shift)
                Bitmap.Bits.Data(k + 1) = BitShift(Shorts.Data(i + 1), Shift)
                Bitmap.Bits.Data(k + 2) = BitShift(Shorts.Data(i), Shift)
                    
                i = i + 3
                k = k + 3
                
            Next
                
            k = k + Remainder
        
        Loop While i <= UBound(Shorts.Data)
        
        Call ExShortsClear(Shorts)
        
        FillInDIB = True
    
    Case Is = ePvFmtYuv411
    
        Call ExBytesSetup(Bytes, Frame.ImageBuffer, Frame.ImageBufferSize)
        
        k = 0
        i = 0
        
        Do
        
            U = Bytes.Data(i)
            Y1 = Bytes.Data(i + 1)
            Y2 = Bytes.Data(i + 2)
            V = Bytes.Data(i + 3)
            Y3 = Bytes.Data(i + 4)
            Y4 = Bytes.Data(i + 5)
            i = i + 6
             
            Call YUV2RGB(Y1, U, V, R, G, B)
            Bitmap.Bits.Data(k) = B
            Bitmap.Bits.Data(k + 1) = G
            Bitmap.Bits.Data(k + 2) = R
            k = k + 3
            Call YUV2RGB(Y2, U, V, R, G, B)
            Bitmap.Bits.Data(k) = B
            Bitmap.Bits.Data(k + 1) = G
            Bitmap.Bits.Data(k + 2) = R
            k = k + 3
            Call YUV2RGB(Y3, U, V, R, G, B)
            Bitmap.Bits.Data(k) = B
            Bitmap.Bits.Data(k + 1) = G
            Bitmap.Bits.Data(k + 2) = R
            k = k + 3
            Call YUV2RGB(Y4, U, V, R, G, B)
            Bitmap.Bits.Data(k) = B
            Bitmap.Bits.Data(k + 1) = G
            Bitmap.Bits.Data(k + 2) = R
            k = k + 3
            
        Loop While i <= UBound(Bytes.Data)
        
        Call ExBytesClear(Bytes)
        
        FillInDIB = True
        
    Case Is = ePvFmtYuv422
    
        Call ExBytesSetup(Bytes, Frame.ImageBuffer, Frame.ImageBufferSize)
        
        k = 0
        i = 0
        
        Do
        
            U = Bytes.Data(i)
            Y1 = Bytes.Data(i + 1)
            V = Bytes.Data(i + 2)
            Y2 = Bytes.Data(i + 3)
            i = i + 4
             
            Call YUV2RGB(Y1, U, V, R, G, B)
            Bitmap.Bits.Data(k) = B
            Bitmap.Bits.Data(k + 1) = G
            Bitmap.Bits.Data(k + 2) = R
            k = k + 3
            Call YUV2RGB(Y2, U, V, R, G, B)
            Bitmap.Bits.Data(k) = B
            Bitmap.Bits.Data(k + 1) = G
            Bitmap.Bits.Data(k + 2) = R
            k = k + 3
                
        Loop While i <= UBound(Bytes.Data)
        
        Call ExBytesClear(Bytes)
        
        FillInDIB = True
    
    Case Is = ePvFmtYuv444
    
        Call ExBytesSetup(Bytes, Frame.ImageBuffer, Frame.ImageBufferSize)
        
        k = 0
        i = 0
        
        Do
        
            U = Bytes.Data(i)
            Y1 = Bytes.Data(i + 1)
            V = Bytes.Data(i + 2)
            Y2 = Bytes.Data(i + 4)
            i = i + 6
             
            Call YUV2RGB(Y1, U, V, R, G, B)
            Bitmap.Bits.Data(k) = B
            Bitmap.Bits.Data(k + 1) = G
            Bitmap.Bits.Data(k + 2) = R
            k = k + 3
            Call YUV2RGB(Y2, U, V, R, G, B)
            Bitmap.Bits.Data(k) = B
            Bitmap.Bits.Data(k + 1) = G
            Bitmap.Bits.Data(k + 2) = R
            k = k + 3
                
        Loop While i <= UBound(Bytes.Data)
        
        Call ExBytesClear(Bytes)
        
        FillInDIB = True
    
    
    Case Else

        FillInDIB = False
    
    End Select
    
End Function
