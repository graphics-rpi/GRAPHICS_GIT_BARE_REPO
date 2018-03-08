VERSION 5.00
Object = "{F9043C88-F6F2-101A-A3C9-08002B2F49FB}#1.2#0"; "COMDLG32.OCX"
Begin VB.Form Form1 
   Caption         =   "Snap"
   ClientHeight    =   2745
   ClientLeft      =   1050
   ClientTop       =   1650
   ClientWidth     =   3990
   LinkTopic       =   "Form1"
   ScaleHeight     =   2745
   ScaleWidth      =   3990
   Begin VB.Timer Timer1 
      Interval        =   600
      Left            =   1800
      Top             =   840
   End
   Begin VB.Frame Frame1 
      Caption         =   "Select a camera:"
      Height          =   2175
      Left            =   120
      TabIndex        =   0
      Top             =   120
      Width           =   3735
      Begin MSComDlg.CommonDialog SaveDialog 
         Left            =   2160
         Top             =   720
         _ExtentX        =   847
         _ExtentY        =   847
         _Version        =   393216
         CancelError     =   -1  'True
         DefaultExt      =   ".bmp"
         DialogTitle     =   "Please select a file to save the frame in ..."
         FileName        =   "MyFrame.bmp"
         Filter          =   "bmp"
         Flags           =   2
      End
      Begin VB.CommandButton Command2 
         Caption         =   "Snap!"
         Enabled         =   0   'False
         Height          =   255
         Left            =   2760
         TabIndex        =   4
         Top             =   1800
         Width           =   855
      End
      Begin VB.CommandButton Command1 
         Caption         =   "Refresh"
         Height          =   255
         Left            =   120
         TabIndex        =   2
         Top             =   1800
         Width           =   855
      End
      Begin VB.ListBox List 
         Height          =   1425
         Left            =   120
         TabIndex        =   1
         Top             =   240
         Width           =   3495
      End
      Begin VB.Label Label3 
         Alignment       =   2  'Center
         Height          =   255
         Left            =   1080
         TabIndex        =   3
         Top             =   1800
         Width           =   1575
      End
   End
   Begin VB.Label Label4 
      Alignment       =   2  'Center
      Height          =   255
      Left            =   120
      TabIndex        =   5
      Top             =   2400
      Width           =   3735
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False

Dim CameraUID As Long
Dim CameraHdl As Long
Dim Frame As tPvFrame
Dim FrameBuffer() As Byte
Dim Bitmap As DIBitmap

Dim GInfo As tPvCameraInfo
Dim GSerial As String

Private Sub Command1_Click()
    Call RefreshList
End Sub

'' prep the streaming
Private Function CameraPrep() As Boolean

    Dim result As Boolean
    
    result = False

    If PvAttrUint32Get(CameraHdl, "TotalBytesPerFrame", Frame.ImageBufferSize) = ePvErrSuccess Then
   
        ReDim FrameBuffer(Frame.ImageBufferSize)
        
        Frame.ImageBuffer = VarPtr(FrameBuffer(0))
        
        result = True
        
    End If
    
    CameraPrep = result
    
End Function

'' start the streaming
Private Function CameraStart() As Boolean

    Dim result As Boolean
    
    result = False
        
    If PvCaptureStart(CameraHdl) = ePvErrSuccess Then
      
        If PvAttrEnumSet(CameraHdl, "FrameStartTriggerMode", "Freerun") = ePvErrSuccess Then
              
            If PvCommandRun(CameraHdl, "AcquisitionStart") = ePvErrSuccess Then
        
                result = True
        
            Else
            
                PvCaptureEnd (CameraHdl)
            
            End If
        
        Else
        
            PvCaptureEnd (CameraHdl)
        
        End If
    
    End If
    
    CameraStart = result
        
End Function

'' stop the streaming
Private Function CameraStop() As Boolean

    discard = PvCommandRun(CameraHdl, "AcquisitionStop")
    If PvCaptureEnd(CameraHdl) = ePvErrSuccess Then
        CameraStop = True
    Else
        CameraStop = False
    End If
    
End Function

'' capture a single frame
Private Function CameraCapture() As Boolean

    Dim result As Boolean
    
    result = False
    
    If PvCaptureQueueFrame(CameraHdl, Frame, 0) = ePvErrSuccess Then
    
        If PvCaptureWaitForFrameDone(CameraHdl, Frame, PVINFINITE) = ePvErrSuccess Then
        
            result = True
        
        End If
    
    End If
    
    CameraCapture = result
    
End Function

'' capture a frame and save it to a BMP file
Private Function CameraSnap(ByRef File As String) As Boolean

    Dim bResult As Boolean
    
    bResult = True

    If PvCameraOpen(CameraUID, ePvAccessMaster, CameraHdl) = ePvErrSuccess Then
    
        If CameraPrep() Then
        
            If CameraStart() Then
            
                If CameraCapture() Then
                
                    If CreateDIB(Frame.Width, Frame.Height, Bitmap) Then
                                                                                                                          
                        If FillInDIB(Bitmap, Frame) Then
                        
                            bResult = SaveDIB(Bitmap, File)
                                                                           
                        Else
                        
                            bResult = False
                                           
                        End If
                                      
                        Call DeleteDIB(Bitmap)
                        
                    Else
                    
                        bResult = False
                
                    End If
                    
                Else
                
                    bResult = False
                
                End If
            
                Call CameraStop
                
            Else
            
                bResult = False
            
            End If
            
        Else
        
            bResult = False
        
        End If
        
        PvCameraClose (CameraHdl)
            
    Else
    
        bResult = False
    
    End If
    
    CameraSnap = bResult

End Function


Private Sub Command2_Click()

    If CameraUID <> 0 Then
    
        If CameraSnap("temp.bmp") Then
        
            Dim Path As String
        
            Path = CurDir$()
        
            On Error GoTo SkipSaving
            
            SaveDialog.ShowSave
            
            Kill SaveDialog.FileName
                        
            On Error GoTo GotError
                        
            Name Path + "\temp.bmp" As SaveDialog.FileName
            
            GoTo SkipSaving
                
GotError:

        Call MsgBox("Sorry, for some reason saving the file failed", vbOKOnly, "Error")
                
SkipSaving:
                        
        Else
        
            Call MsgBox("Sorry, grabing and/or saving a frame failed", vbOKOnly, "Error")
        
        End If
    
    End If

End Sub

Private Sub Form_Load()

    If PvInitialize() <> ePvErrSuccess Then
    
        Call MsgBox("Sorry, the PvAPI failed to be initialized.", vbCritical Or vbOKOnly, "Error")
    
    End If
    
    CameraUID = 0

End Sub

Private Sub Form_Unload(Cancel As Integer)

    Call PvUnInitialize

End Sub

Private Sub List_Click()

    Dim Info As tPvCameraInfo
    
    CameraUID = 0
    
    For i = 0 To List.ListCount - 1
    
        If List.Selected(i) And CameraUID = 0 Then
            CameraUID = List.ItemData(i)
        End If
    
    Next
    
    If CameraUID <> 0 Then
    
        If PvCameraInfo(CameraUID, Info) = ePvErrSuccess Then
            Label3.Caption = BytesToString(Info.SerialString)
            Command2.Enabled = True
        Else
            Command2.Enabled = False
            Label3.Caption = ""
        End If
        
    End If

End Sub

Private Sub RefreshList()

    Dim Num As Long
    Dim Filled As Long
    Dim Table(0 To 15) As tPvCameraInfo
    Dim Line As String
    
    ' clear the list widget
    List.Clear
    
    '' retreive the list of visible camera
    Filled = PvCameraList(Table(0), 16, Num)
        
    '' only add to the list widget the camera that are accessible in "master" mode
    For i = 0 To (Filled - 1)
        If (Table(i).PermittedAccess = ePvAccessMaster) Or (Table(i).PermittedAccess = ePvAccessMaster + ePvAccessMonitor) Then
            Line = BytesToString(Table(i).SerialString) + " (" + BytesToString(Table(i).DisplayName) + ")"
            List.AddItem (Line)
            List.ItemData(List.NewIndex) = Table(i).UniqueID
        End If
    Next
    
End Sub

'' we use a "single-shot" sort of timer to give some time to the API to find any camera before we
'' get the camera list
Private Sub Timer1_Timer()

    Timer1.Enabled = False
    
    Call RefreshList
    
End Sub
