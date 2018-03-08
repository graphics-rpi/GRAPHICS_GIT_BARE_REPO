VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Stream"
   ClientHeight    =   2760
   ClientLeft      =   1050
   ClientTop       =   1650
   ClientWidth     =   3990
   LinkTopic       =   "Form1"
   ScaleHeight     =   2760
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
      Begin VB.CommandButton Command2 
         Caption         =   "Stream!"
         Enabled         =   0   'False
         Height          =   255
         Left            =   2520
         TabIndex        =   4
         Top             =   1800
         Width           =   1095
      End
      Begin VB.CommandButton Command1 
         Caption         =   "Refresh list"
         Height          =   255
         Left            =   120
         TabIndex        =   2
         Top             =   1800
         Width           =   1095
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

Option Explicit

Dim CameraUID As Long
Dim CameraHdl As Long
Dim Frames(0 To 4) As tPvFrame
Dim Buffers() As Byte
Dim FirstTime As Boolean
Dim Abort As Boolean
Dim Gone As Boolean

'' Open the given camera
Private Function CameraOpen(ByVal UniqueID As Long) As Boolean

    If CameraHdl = 0 Then
        If PvCameraOpen(CameraUID, ePvAccessMaster, CameraHdl) = ePvErrSuccess Then
            Gone = False
            CameraOpen = True
        Else
            CameraOpen = False
        End If
    Else
        CameraOpen = False
    End If

End Function

'' Close the camera currently open
Private Function CameraClose() As Boolean

    If CameraHdl <> 0 Then
        Call PvCameraClose(CameraHdl)
        CameraHdl = 0
        CameraClose = True
    Else
        CameraClose = False
    End If

End Function

'' Allocate the requiered image buffers
Private Function CameraPrep() As Boolean

    Dim result As Boolean
    Dim Length As Long
    Dim i As Long
    
    result = False

    If PvAttrUint32Get(CameraHdl, "TotalBytesPerFrame", Length) = ePvErrSuccess Then
   
        ReDim Buffers(Length, 0 To 4)
        
        For i = 0 To 4
        
            Frames(i).Context(0) = CameraHdl
            Frames(i).ImageBuffer = VarPtr(Buffers(0, i))
            Frames(i).ImageBufferSize = Length
        
        Next
        
        result = True
        
    End If
    
    CameraPrep = result
    
End Function

'' Get the camera streaming (Freerun)
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

'' Request the camera to stop streaming
Private Function CameraStop() As Boolean

    Call PvCommandRun(CameraHdl, "AcquisitionStop")
    
    If PvCaptureEnd(CameraHdl) = ePvErrSuccess Then
        CameraStop = True
    Else
        CameraStop = False
    End If
    
End Function

'' Queue several frames for capture by the camera
Private Sub CameraQueue()

    Dim i As Long

    For i = 0 To 4
        Call PvCaptureQueueFrame(CameraHdl, Frames(i), 0)
    Next
    
End Sub

'' Performs the "frame loop" on the camera currently open
Private Sub CameraLoop()

    Dim i As Long
    Dim err As tPvErr

    While Not Abort
    
        For i = 0 To 4
            
            Do
            
                '' allow VB to do other things
                DoEvents
            
                '' wait for the given frame to be acquired
                err = PvCaptureWaitForFrameDone(CameraHdl, Frames(i), 10)
                If err <> ePvErrTimeout And Not Abort Then
                    If err = ePvErrUnplugged Then
                        Abort = True
                        Gone = True
                    Else
                        '' re-enqueue it
                        Call PvCaptureQueueFrame(CameraHdl, Frames(i), 0)
                    End If
                End If
                
            Loop While err = ePvErrTimeout And Not Abort
            
        Next
    
    Wend

End Sub

'' Dequeue all currently queued frames
Private Function CameraDequeue() As Boolean

    Call PvCaptureQueueClear(CameraHdl)

End Function

'' Refresh the list of visible cameras
Private Sub RefreshList()

    Dim Num As Long
    Dim Filled As Long
    Dim Table(0 To 15) As tPvCameraInfo
    Dim Line As String
    Dim i As Long
    
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


'' refresh the list of cameras when the user clicks on the "refreh" button
Private Sub Command1_Click()
    Call RefreshList
End Sub

'' start or stop the streaming when the user clicks on the corresponding button
Private Sub Command2_Click()

    If CameraUID <> 0 Then
    
        If CameraHdl = 0 Then
        
            If CameraOpen(CameraUID) Then
        
                If CameraPrep() Then
        
                    If CameraStart() Then
        
                        List.Enabled = False
                        Command1.Enabled = False
                        Command2.Caption = "Stop!"
                        
                        Call CameraQueue
                        
                        Call CameraLoop
                        
                        Call CameraDequeue
            
                        Call CameraStop
            
                        Call CameraClose
        
                        Command2.Caption = "Stream!"
                        List.Enabled = True
                        Command1.Enabled = True
                        
                        If Gone Then
                            Call MsgBox("Sorry, connection to the camera was lost", vbInformation Or vbOKOnly, "Error")
                        End If
                        
                        GoTo noError
        
                    Else
        
                        GoTo AnError
        
                    End If
        
AnError:
                    Call CameraClose
                    
                    Call MsgBox("Sorry, there was an error", vbCritical Or vbOKOnly, "Error")
                
noError:
                
                End If
        
            Else
            
                Call MsgBox("Sorry, the camera you selected couldn't be open", vbCritical Or vbOKOnly, "Error")
        
            End If
        
        Else
        
            Abort = True
        
        End If
    
    End If

End Sub

'' Called when the Form is loaded
Private Sub Form_Load()

    If PvInitialize() <> ePvErrSuccess Then
    
        Call MsgBox("Sorry, the PvAPI failed to be initialized.", vbCritical Or vbOKOnly, "Error")
    
    End If
    
    CameraUID = 0
    FirstTime = True
    Abort = False

End Sub

'' Called when the Form is unloaded
Private Sub Form_Unload(Cancel As Integer)

    '' if the camera is still open, we won't allow the user to close the application
    If CameraHdl <> 0 Then
    
        Call MsgBox("You need to stop the streaming first", vbExclamation And vbOKOnly, "Sorry but ...")
        Cancel = 1
    
    Else

        Cancel = 0
        Call PvUnInitialize

    End If

End Sub

'' Called when the user select a camera from the cameras list
Private Sub List_Click()

    Dim Info As tPvCameraInfo
    Dim i As Long
    
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

'' Called when the timer expires
Private Sub Timer1_Timer()

    '' trigger the display of the cameras list the first time
    If FirstTime Then
        Timer1.Interval = 300
        FirstTime = False
        Call RefreshList
    Else
        '' otherwise displays some streaming statistics when the camera is opened
        If CameraHdl <> 0 Then
        
            Dim Completed As Long
            Dim Dropped As Long
            Dim Missed As Long
            Dim Received As Long
                    
            Call PvAttrUint32Get(CameraHdl, "StatFramesCompleted", Completed)
            Call PvAttrUint32Get(CameraHdl, "StatFramesDropped", Dropped)
            Call PvAttrUint32Get(CameraHdl, "StatPacketsMissed", Missed)
            Call PvAttrUint32Get(CameraHdl, "StatPacketsReceived", Received)
                             
            Label4.Caption = "Frames : " & Completed & " " & Dropped & " Packets: " & Received & " " & Missed
         
        Else
    
            Label4.Caption = ""
    
        End If
    End If
    
End Sub
