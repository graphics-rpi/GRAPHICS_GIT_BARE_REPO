VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "ListAttributes"
   ClientHeight    =   6990
   ClientLeft      =   1050
   ClientTop       =   1650
   ClientWidth     =   3990
   LinkTopic       =   "Form1"
   ScaleHeight     =   6990
   ScaleWidth      =   3990
   Begin VB.ListBox List1 
      Enabled         =   0   'False
      Height          =   3375
      Left            =   120
      TabIndex        =   6
      Top             =   2400
      Width           =   3735
   End
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
         Caption         =   "Open"
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
   Begin VB.Label Label9 
      Height          =   255
      Left            =   1200
      TabIndex        =   11
      Top             =   6600
      Width           =   2655
   End
   Begin VB.Label Label8 
      Height          =   255
      Left            =   1200
      TabIndex        =   10
      Top             =   6240
      Width           =   2655
   End
   Begin VB.Label Label7 
      Caption         =   "Value :"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   -1  'True
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   120
      TabIndex        =   9
      Top             =   6600
      Width           =   855
   End
   Begin VB.Label Label5 
      Caption         =   "Category :"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   -1  'True
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   120
      TabIndex        =   8
      Top             =   6240
      Width           =   855
   End
   Begin VB.Label Label1 
      Caption         =   "Type :"
      BeginProperty Font 
         Name            =   "MS Sans Serif"
         Size            =   8.25
         Charset         =   0
         Weight          =   400
         Underline       =   -1  'True
         Italic          =   0   'False
         Strikethrough   =   0   'False
      EndProperty
      Height          =   255
      Left            =   120
      TabIndex        =   7
      Top             =   5880
      Width           =   855
   End
   Begin VB.Label Label4 
      Height          =   255
      Left            =   1200
      TabIndex        =   5
      Top             =   5880
      Width           =   2655
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
Dim FirstTime As Boolean
Dim Gone As Boolean

'' Open the given camera
Private Function CameraOpen(ByVal UniqueId As Long) As Boolean

    If CameraHdl = 0 Then
        If PvCameraOpen(CameraUID, ePvAccessMonitor, CameraHdl) = ePvErrSuccess Then
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
            Line = BytesToString(Table(i).SerialString) + " (" + BytesToString(Table(i).DisplayName) + ")"
            List.AddItem (Line)
            List.ItemData(List.NewIndex) = Table(i).UniqueId
    Next
    
End Sub

Private Sub ListAttributes()

    Dim Numbers As Long
    Dim i As Long
    Dim Attributes() As String
    
    If PvAttrList(CameraHdl, Attributes, Numbers) = ePvErrSuccess Then
    
        List1.Clear
        
        For i = 0 To Numbers - 1
        
            List1.AddItem (Attributes(i))
        
        Next
        
    End If

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
            
                Call ListAttributes
                
                List.Enabled = False
                Command1.Enabled = False
                List1.Enabled = True
                Command2.Caption = "Close"
            
            Else
            
                Call MsgBox("Sorry, this camera couldn't be open.", vbCritical Or vbOKOnly, "Error")
            
            End If
               
        Else
        
            Call CameraClose
           
            Label4.Caption = ""
            Label8.Caption = ""
            Label9.Caption = ""
            List1.Clear
            List1.Enabled = False
            List.Enabled = True
            Command1.Enabled = True
            Command2.Caption = "Open"
        
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

End Sub

'' Called when the Form is unloaded
Private Sub Form_Unload(Cancel As Integer)

    '' if the camera is still open, we will close it
    If CameraHdl <> 0 Then
    
        Call Command2_Click
        
    End If
        
    Call PvUnInitialize

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

Private Sub List1_Click()

    Dim Info As tPvAttributeInfo
    
    If PvAttrInfo(CameraHdl, List1.Text, Info) = ePvErrSuccess Then
    
        Label8.Caption = Info.Category
    
        Select Case Info.Datatype

            Case Is = ePvDatatypeCommand
        
                Label4.Caption = "Command"
                Label9.Caption = "n/a"
        
            Case Is = ePvDatatypeString
            
                Dim SValue As String
                
                Label4.Caption = "String"
                
                If PvAttrStringGet(CameraHdl, List1.Text, SValue) = ePvErrSuccess Then
                    Label9.Caption = SValue
                Else
                    Label9.Caption = "Error!"
                End If
            
            Case Is = ePvDatatypeEnum
            
                Dim EValue As String
                
                Label4.Caption = "Enumerate"
                
                If PvAttrEnumGet(CameraHdl, List1.Text, EValue) = ePvErrSuccess Then
                    Label9.Caption = EValue
                Else
                    Label9.Caption = "Error!"
                End If
            
            Case Is = ePvDatatypeUint32
            
                Dim UValue As Long
                
                Label4.Caption = "Number"
                
                If PvAttrUint32Get(CameraHdl, List1.Text, UValue) = ePvErrSuccess Then
                    Label9.Caption = UValue
                Else
                    Label9.Caption = "Error!"
                End If
                   
            Case Is = ePvDatatypeFloat32
            
                Dim FValue As Single
                
                Label4.Caption = "Float"
                
                If PvAttrFloat32Get(CameraHdl, List1.Text, FValue) = ePvErrSuccess Then
                    Label9.Caption = FValue
                Else
                    Label9.Caption = "Error!"
                End If
    
        End Select
    
    Else
    
        Call MsgBox("Sorry, we were not able to retreive this attribute informations", vbCritical Or vbOKOnly, "Error")
    
    End If

End Sub

'' Called when the timer expires
Private Sub Timer1_Timer()

    '' trigger the display of the cameras list the first time
    If FirstTime Then
        Timer1.Interval = 250
        FirstTime = False
        Call RefreshList
    Else
        
        If CameraHdl <> 0 Then
        
            Dim Ignore As Long
            
            If PvCaptureQuery(CameraHdl, Ignore) = ePvErrUnplugged Then
            
                Call Command2_Click
                
                Call MsgBox("Sorry, the connection to the camera was lost", vbInformation Or vbOKOnly, "Error")
            
            End If
        
        End If
        
    End If
    
End Sub
