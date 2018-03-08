Attribute VB_Name = "PvAPI"
'==============================================================================
' Copyright (C) 2006-2007 Prosilica.  All Rights Reserved.
'
' Redistribution of this header file, in original or modified form, without
' prior written consent of Prosilica is prohibited.
'
'=============================================================================
'
' File:         PvAPI.bas
'
' Project/lib:  PvAPI
'
' Target:       Visual Basic 6.0
'
' Description:  Main header file for PvAPI, the programming interface for
'               Prosilica's GigE and Firewire cameras.
'
' Notes:        GigE support is available now; firewire support to follow.
'
'------------------------------------------------------------------------------
'
' Here are the basic functions:
'
'       PvInitialize()          - load and initialize the PvApi module
'       PvUnInitialize()        - shut down the module
'
'       PvCameraList()          - list the cameras available
'
'       PvCameraOpen()          - open & close your cameras
'       PvCameraClose()
'
'       PvCaptureStart()        - start & end image capture
'       PvCaptureEnd()
'
'       PvCaptureQueueFrame()   - queue a frame buffer for image capture
'
'       PvCaptureQueueClear()   - clear all frames off the queue (i.e. abort)
'
'       PvAttrList()            - list all attributes for this camera
'       PvAttrInfo()            - get information on an attribute
'
' Camera attributes are used to represent controls such as acquisition mode or
' shutter exposure time.  Camera attributes are also used to represent constant
' values like serial numbers, or read-only values like bytes-per-frame.
'
' A few basic attribute datatypes are supported, such as 32-bit unsigned,
' enumerated set, and float.
'
' Here are the basic attributes common to all cameras:
'
'       AcquisitionMode         - idle, or continuous acquisition
'       Width                   - image width in pixels
'       Height                  - image height in pixels
'       PixelFormat             - image data format (ex. mono8, mono16)
'       TotalBytesPerFrame      - number of bytes per image
'
' Most attributes are specific to each camera model and revision, and are
' documented elsewhere.
'
' ATTENTION:
'
' In regard to the usage of callback procedures with VB, you will need to be VERY
' cautious about what you will do in the callback since they will be executed from
' another thread than the one running your VB application. See the following web
' page for more details:
'
' http://support.microsoft.com/default.aspx?scid=KB;EN-US;Q198607&
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
' 03/Oct/07     Original.
'==============================================================================

''===== #DEFINES ==============================================================

Public Const PVINFINITE As Long = &HFFFFFFFF  '' Never timeout

''===== TYPE DEFINITIONS ======================================================

''
'' Error codes, returned by most functions:
''
Public Enum tPvErr

    ePvErrSuccess = 0              '' No error
    ePvErrCameraFault = 1          '' Unexpected camera fault
    ePvErrInternalFault = 2        '' Unexpected fault in PvApi or driver
    ePvErrBadHandle = 3            '' Camera handle is invalid
    ePvErrBadParameter = 4         '' Bad parameter to API call
    ePvErrBadSequence = 5          '' Sequence of API calls is incorrect
    ePvErrNotFound = 6             '' Camera or attribute not found
    ePvErrAccessDenied = 7         '' Camera cannot be opened in the specified mode
    ePvErrUnplugged = 8            '' Camera was unplugged
    ePvErrInvalidSetup = 9         '' Setup is invalid (an attribute is invalid)
    ePvErrResources = 10           '' System/network resources or memory not available
    ePvErrBandwidth = 11           '' 1394 bandwidth not available
    ePvErrQueueFull = 12           '' Too many frames on queue
    ePvErrBufferTooSmall = 13      '' Frame buffer is too small
    ePvErrCancelled = 14           '' Frame cancelled by user
    ePvErrDataLost = 15            '' The data for the frame was lost
    ePvErrDataMissing = 16         '' Some data in the frame is missing
    ePvErrTimeout = 17             '' Timeout during wait
    ePvErrOutOfRange = 18          '' Attribute value is out of the expected range
    ePvErrWrongType = 19           '' Attribute is not this type (wrong access function)
    ePvErrForbidden = 20           '' Attribute write forbidden at this time
    ePvErrUnavailable = 21         '' Attribute is not available at this time
    ePvErrFirewall = 22            '' A firewall is blocking the traffic (Windows only)
    ePvErr_force_32 = &HFFFFFFFF

End Enum

''----- Camera Enumeration & Information --------------------------------------

''
'' Camera access mode.  Used as flags in tPvCameraInfo data, and as the access
'' mode in PvOpenCamera().
''
Public Enum tPvAccessFlags

    ePvAccessMonitor = 2            '' Monitor access: no control, read & listen only
    ePvAccessMaster = 4             '' Master access: full control
    ePvAccess_force_32 = &HFFFFFFFF

End Enum

''
'' Camera interface type (i.e. firewire, ethernet):
''
Public Enum tPvInterface

    ePvInterfaceFirewire = 1
    ePvInterfaceEthernet = 2
    ePvInterface_force_32 = &HFFFFFFFF

End Enum

''
'' Camera information type.
''
Public Type tPvCameraInfo

    UniqueID As Long                '' Unique value for each camera
    SerialString(0 To 31) As Byte   '' Camera's serial number
    PartNumber As Long              '' Camera part number
    PartVersion As Long             '' Camera part version
    PermittedAccess As Long         '' A combination of tPvAccessFlags
    InterfaceId As Long             '' Unique value for each interface or bus
    InterfaceType As tPvInterface   '' Interface type; see tPvInterface
    DisplayName(0 To 15) As Byte    '' People-friendly camera name
    reserved(0 To 3) As Long        '' Always zero

End Type

''
'' IP configuration mode for ethernet cameras.
''
Public Enum tPvIpConfig

    ePvIpConfigPersistent = 1           '' Use persistent IP settings
    ePvIpConfigDhcp = 2                 '' Use DHCP, fallback to AutoIP
    ePvIpConfigAutoIp = 4               '' Use AutoIP only
    ePvIpConfig_force_32 = &HFFFFFFFF

End Enum

''
'' Structure used for PvCameraIpSettingsGet() and PvCameraIpSettingsChange().
''
Public Type tPvIpSettings

    '' IP configuration mode: persistent, DHCP & AutoIp, or AutoIp only.
    ConfigMode As tPvIpConfig
    '' IP configuration mode supported by the camera
    ConfigModeSupport As Long

    '' Current IP configuration.  Ignored for PvCameraIpSettingsChange().  All
    '' values are in network byte order (i.e. big endian).
    CurrentIpAddress As Long
    CurrentIpSubnet As Long
    CurrentIpGateway As Long

    '' Persistent IP configuration.  See "ConfigMode" to enable persistent IP
    '' settings.  All values are in network byte order.
    PersistentIpAddr As Long
    PersistentIpSubnet As Long
    PersistentIpGateway As Long
 
    reserved(0 To 7) As Long

End Type

''
'' Link (aka interface) event type
''
Public Enum tPvLinkEvent

    ePvLinkAdd = 1          '' A camera was plugged in
    ePvLinkRemove = 2       '' A camera was unplugged
    ePvLink_reserved1 = 3
    ePvLink_force_32 = &HFFFFFFFF

End Enum

''----- Image Capture ---------------------------------------------------------

''
'' Frame image format type
''
Public Enum tPvImageFormat

    ePvFmtMono8 = 0                 '' Monochrome, 8 bits
    ePvFmtMono16 = 1                '' Monochrome, 16 bits, data is LSB aligned
    ePvFmtBayer8 = 2                '' Bayer-color, 8 bits
    ePvFmtBayer16 = 3               '' Bayer-color, 16 bits, data is LSB aligned
    ePvFmtRgb24 = 4                 '' RGB, 8 bits x 3
    ePvFmtRgb48 = 5                 '' RGB, 16 bits x 3, data is LSB aligned
    ePvFmtYuv411 = 6                '' YUV 411
    ePvFmtYuv422 = 7                '' YUV 422
    ePvFmtYuv444 = 8                '' YUV 444
    ePvFmtBgr24 = 9                 '' BGR, 8 bits x 3
    ePvFmtRgba32 = 10               '' RGBA, 8 bits x 4
    ePvFmtBgra32 = 11               '' BGRA, 8 bits x 4
    ePvFmtMono12Packed = 12         '' Monochrome, 12 bits, 
    ePvFmtBayer12Packed = 13        '' Bayer-color, 12 bits, packed
    ePvFmt_force_32 = &HFFFFFFFF

End Enum

''
'' Bayer pattern.  Applicable when a Bayer-color camera is sending raw bayer
'' data.
''
Public Enum tPvBayerPattern

    ePvBayerRGGB = 0                '' First line RGRG, second line GBGB...
    ePvBayerGBRG = 1                '' First line GBGB, second line RGRG...
    ePvBayerGRBG = 2                '' First line GRGR, second line BGBG...
    ePvBayerBGGR = 3                '' First line BGBG, second line GRGR...
    ePvBayer_force_32 = &HFFFFFFFF

End Enum

''
'' The frame structure passed to PvQueueFrame().
''
Public Type tPvFrame

    ''----- In -----
    ImageBuffer As Long             '' Your image buffer (Use VarPtr(your_array(0)) to get address of your array)
    ImageBufferSize As Long         '' Size of your image buffer in bytes

    AncillaryBuffer As Long         '' Your buffer to capture associated (Use VarPtr(your_array(0)) to get address of your array)
    AncillaryBufferSize As Long     '' Size of your ancillary buffer in bytes
                                    ''(can be 0 for no buffer).

    Context(0 To 3) As Long         '' For your use
    
    reserved1(0 To 7) As Long

    ''----- Out -----

    Status As tPvErr                '' Status of this frame

    ImageSize As Long               '' Image size, in bytes
    AncillarySize As Long           '' Ancillary data size, in bytes

    Width As Long                   '' Image width
    Height As Long                  '' Image height
    RegionX As Long                 '' Start of readout region (left)
    RegionY As Long                 '' Start of readout region (top)
    format As tPvImageFormat        '' Image format
    BitDepth As Long                '' Number of significant bits
    BayerPattern As tPvBayerPattern '' Bayer pattern, if bayer format

    FrameCount As Long              '' Rolling frame counter
    TimestampLo As Long             '' Time stamp, lower 32-bits
    TimestampHi As Long             '' Time stamp, upper 32-bits

    reserved2(0 To 31) As Long

End Type

''----- Attributes ------------------------------------------------------------

''
'' Attribute data type supported
''
Public Enum tPvDatatype

    ePvDatatypeUnknown = 0
    ePvDatatypeCommand = 1
    ePvDatatypeRaw = 2
    ePvDatatypeString = 3
    ePvDatatypeEnum = 4
    ePvDatatypeUint32 = 5
    ePvDatatypeFloat32 = 6
    ePvDatatypeInt64 = 7
    ePvDatatypeBoolean = 8
    ePvDatatypeforce_32 = &HFFFFFFFF

End Enum

''
'' Attribute flags type
''
Public Enum tPvAttributeFlags

    ePvFlagRead = &H1              '' Read access is permitted
    ePvFlagWrite = &H2             '' Write access is permitted
    ePvFlagVolatile = &H4          '' The camera may change the value any time
    ePvFlagConst = &H8             '' Value is read only and never changes
    ePvFlag_force_32 = &HFFFFFFFF

End Enum

''
'' Attribute information type
''
Public Type tPvAttributeInfo

    Datatype As tPvDatatype     '' Data type
    Flags As Long               '' Combination of tPvAttribute flags
    Category As String          '' Advanced: see documentation
    Impact As String            '' Advanced: see documentation

End Type

''===== INTERNAL TYPES DEFINITIONS ============================================
''
'' The following types are NOT meant to be used directly by yourself
''

Public Type tPvAttributeInfoInternal

    Datatype As tPvDatatype
    Flags As Long
    Category As Long
    Impact As Long
    reserved(0 To 3) As Long

End Type

''===== INTERNAL FUNCTIONS DEFINITIONS =========================================
''
'' The following functions are NOT meant to be used directly by yourself
''
''
Declare Sub CopyMemory Lib "kernel32" Alias "RtlMoveMemory" (ByVal dest As Any, ByVal source As Any, ByVal Bytes As Long)
''
Declare Function PvAttrListInternal Lib "PvApi.dll" Alias "PvAttrList" (ByVal Camera As Long, ByRef pListPtr As Long, ByRef pLength As Long) As tPvErr
''
Declare Function PvAttrInfoInternal Lib "PvApi.dll" Alias "PvAttrInfo" (ByVal Camera As Long, ByVal Name As Long, ByRef pInfo As tPvAttributeInfoInternal) As tPvErr
''
Declare Function PvAttrExistsInternal Lib "PvApi.dll" Alias "PvAttrExists" (ByVal Camera As Long, ByVal Name As Long) As tPvErr
''
Declare Function PvAttrIsAvailableInternal Lib "PvApi.dll" Alias "PvAttrIsAvailable" (ByVal Camera As Long, ByVal Name As Long) As tPvErr
''
Declare Function PvAttrIsValidInternal Lib "PvApi.dll" Alias "PvAttrIsValid" (ByVal Camera As Long, ByVal Name As Long) As tPvErr
''
Declare Function PvAttrRangeEnumInternal Lib "PvApi.dll" Alias "PvAttrRangeEnum" (ByVal Camera As Long, ByVal Name As Long, ByVal Buffer As Long, ByVal Length As Long, ByRef size As Long) As tPvErr
''
Declare Function PvAttrRangeUint32Internal Lib "PvApi.dll" Alias "PvAttrRangeUint32" (ByVal Camera As Long, ByVal Name As Long, ByRef pMin As Long, ByRef pMax As Long) As tPvErr
''
Declare Function PvAttrRangeInt64Internal Lib "PvApi.dll" Alias "PvAttrRangeInt64" (ByVal Camera As Long, ByVal Name As Long, ByRef pMin As Long, ByRef pMax As Long) As tPvErr
''
Declare Function PvAttrRangeFloat32Internal Lib "PvApi.dll" Alias "PvAttrRangeFloat32" (ByVal Camera As Long, ByVal Name As Long, ByRef pMin As Single, ByRef pMax As Single) As tPvErr
''
Declare Function PvCommandRunInternal Lib "PvApi.dll" Alias "PvCommandRun" (ByVal Camera As Long, ByVal Name As Long) As tPvErr
''
Declare Function PvAttrStringGetInternal Lib "PvApi.dll" Alias "PvAttrStringGet" (ByVal Camera As Long, ByVal Name As Long, ByVal Buffer As Long, ByVal Length As Long, ByRef size As Long) As tPvErr
''
Declare Function PvAttrStringSetInternal Lib "PvApi.dll" Alias "PvAttrStringSet" (ByVal Camera As Long, ByVal Name As Long, ByVal Value As Long) As tPvErr
''
Declare Function PvAttrEnumGetInternal Lib "PvApi.dll" Alias "PvAttrEnumGet" (ByVal Camera As Long, ByVal Name As Long, ByVal Buffer As Long, ByVal Length As Long, ByRef size As Long) As tPvErr
''
Declare Function PvAttrEnumSetInternal Lib "PvApi.dll" Alias "PvAttrEnumSet" (ByVal Camera As Long, ByVal Name As Long, ByVal Value As Long) As tPvErr
''
Declare Function PvAttrUint32GetInternal Lib "PvApi.dll" Alias "PvAttrUint32Get" (ByVal Camera As Long, ByVal Name As Long, ByRef Value As Long) As tPvErr
''
Declare Function PvAttrUint32SetInternal Lib "PvApi.dll" Alias "PvAttrUint32Set" (ByVal Camera As Long, ByVal Name As Long, ByVal Value As Long) As tPvErr
''
Declare Function PvAttrBooleanGetInternal Lib "PvApi.dll" Alias "PvAttrBooleanGet" (ByVal Camera As Long, ByVal Name As Long, ByRef Value As Boolean) As tPvErr
''
Declare Function PvAttrBooleanSetInternal Lib "PvApi.dll" Alias "PvAttrBooleanSet" (ByVal Camera As Long, ByVal Name As Long, ByVal Value As Boolean) As tPvErr
''
Declare Function PvAttrInt64GetInternal Lib "PvApi.dll" Alias "PvAttrInt64Get" (ByVal Camera As Long, ByVal Name As Long, ByRef Value As Long) As tPvErr
''
Declare Function PvAttrInt64SetInternal Lib "PvApi.dll" Alias "PvAttrInt64Set" (ByVal Camera As Long, ByVal Name As Long, ByVal Value As Long) As tPvErr
''
Declare Function PvAttrFloat32GetInternal Lib "PvApi.dll" Alias "PvAttrFloat32Get" (ByVal Camera As Long, ByVal Name As Long, ByRef Value As Single) As tPvErr
''
Declare Function PvAttrFloat32SetInternal Lib "PvApi.dll" Alias "PvAttrFloat32Set" (ByVal Camera As Long, ByVal Name As Long, ByVal Value As Single) As tPvErr

''===== FUNCTION DECLARATIONS ===================================================
''
'' The following functions are meant to be used directly by yourself
''

''
'' Function:  PvVersion()
''
'' Purpose:   Retreive the version number of PvAPI
''
'' Arguments:
''
'' [OUT] pMajor As Long, major version number
'' [OUT] pMinor As Long, minor version number
''
'' Return:    none
''
'' This function can be called at anytime, including before the API is initialized.
''
Declare Sub PvVersion Lib "PvApi.dll" (ByRef pMajor As Long, ByRef pMinor As Long)
    
''----- API Initialization ----------------------------------------------------

''
'' Function:  PvInitialize()
''
'' Purpose:   Initialize the PvApi module.  This must be called before any
''            other PvApi function is run.
''
'' Arguments: none
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrInternalFault,   an internal fault occurred
''
Declare Function PvInitialize Lib "PvApi.dll" () As tPvErr

''
'' Function:  PvUnInitialize()
''
'' Purpose:   Uninitialize the API module.  This will free some resources,
''            and shut down network activity if applicable.
''
'' Arguments: none
''
'' Return:    none
''
Declare Sub PvUnInitialize Lib "PvApi.dll" ()

''----- Interface-Link Callback -----------------------------------------------

''
'' Function:  PvLinkCallbackRegister()
''
'' Purpose:   Register a callback for interface events.
''
'' Arguments:
''
'' [ IN] Callback As Long,          Callback function to run when an event occurs (use AddressOf)
'' [ IN] EventType As tPvLinkEvent, Event to trigger the callback
'' [ IN] Context As Long,           For your use: Context is passed to your
''                                  callback function
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrBadSequence,     API isn't initialized
''
'' Here's the rules:
''   - Multiple callback functions may be registered with the same event
''   - The same callback function may be shared by different events
''   - The same callback function with the same event may not be registered twice
''
'' The callback functions are called from a thread within PvApi.  The callbacks
'' are sequenced; i.e. they will not be called simultaneously.
''
'' Your VB function must be defined in a BAS file and of the following form:
''
'' Sub Camera_Event(ByVal Context As Long, ByVal Interface As tPvInterface, ByVal EventType As tPvLinkEvent, ByVal UniqueId As Long)
''
'' End Sub
''
'' Use PvLinkCallbackUnRegister() to stop receiving callbacks.
''
Declare Function PvLinkCallbackRegister Lib "PvApi.dll" (ByVal Callback As Long, ByVal EventType As tPvLinkEvent, ByVal Context As Long) As tPvErr

''
'' Function:  PvLinkCallbackUnRegister()
''
'' Purpose:   Unregister a callback for interface events.
''
'' Arguments:
''
'' [ IN] Callback As Long,          Callback function previously registered (use AddressOf)
'' [ IN] EventType As tPvLinkEvent, Event associated with the callback
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrNotFound,        registered callback was not found
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrBadSequence,     API isn't initialized
''
Declare Function PvLinkCallbackUnRegister Lib "PvApi.dll" (ByVal Callback As Long, ByVal EventType As tPvLinkEvent) As tPvErr

''----- Camera Enumeration & Information --------------------------------------

''
'' Function:  PvCameraList()
''
'' Purpose:   List all the cameras currently visible to PvApi
''
'' Arguments:
''
'' [OUT] pList As tPvCameraInfo, Array of tPvCameraInfo, allocated by
''                                the caller.  The camera list is
''                                copied here.
'' [ IN] ListLength As Long,     Length of the caller's pList array
'' [OUT] pConnectedNum As Long,  Number of cameras found (may be more
''                                than ListLength!) returned here.
''                                May be NULL.
''
'' Return:    Number of pList entries filled, up to ListLength.
''
Declare Function PvCameraList Lib "PvApi.dll" (ByRef pList As tPvCameraInfo, ByVal ListLength As Long, ByRef pConnectedNum As Long) As Long

''
'' Function:  PvCameraListUnreachable()
''
'' Purpose:   List all the cameras currently inaccessable by PvApi.  This lists
''            the ethernet cameras which are connected to the local ethernet
''            network, but are on a different subnet.
''
'' Arguments:
''
'' [OUT] pList As tPvCameraInfo, Array of tPvCameraInfo, allocated by
''                                the caller.  The camera list is
''                                copied here.
'' [ IN] ListLength As Long,     Length of the caller's pList array
'' [OUT] pConnectedNum As Long,  Number of cameras found (may be more
''                                than ListLength!) returned here.
''                                May be NULL.
''
'' Return:    Number of pList entries filled, up to ListLength.
''
Declare Function PvCameraListUnreachable Lib "PvApi.dll" (ByRef pList As tPvCameraInfo, ByVal ListLength As Long, ByRef pConnectedNum As Long) As Long

''
'' Function:  PvCameraCount()
''
'' Purpose:   Number of cameras visible to PvApi (at the time of the call).
''            Does not include unreachable cameras.
''
'' Arguments: none
''
'' Return:    The number of cameras found
''
'' The number of cameras is dynamic, and may change at any time.
''
Declare Function PvCameraCount Lib "PvApi.dll" () As Long

''
'' Function:  PvCameraInfo()
''
'' Purpose:   Retreive information on a given camera
''
'' Arguments:
''
'' [ IN] UniqueId As Long,       Unique ID of the camera
'' [OUT] pInfo As tPvCameraInfo, Structure where the information will be copied
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrNotFound,        the camera was not found (unplugged)
''               ePvErrUnplugged,       the camera was found but unplugged during the
''                                      function call
''               ePvErrBadParameter,    a valid pointer for pInfo was not supplied
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Declare Function PvCameraInfo Lib "PvApi.dll" (ByVal UniqueID As Long, ByRef pInfo As tPvCameraInfo) As tPvErr

''
'' Function:  PvCameraInfoByAddr()
''
'' Purpose:   Retreive information on a camera, by IP address.  This function
''            is required if the ethernet camera is not on the local ethernet
''            network.
''
'' Arguments:
''
'' [ IN] IpAddr As Long,               IP address of camera, in network byte order.
'' [OUT] pInfo As tPvCameraInfo,       The camera information will be copied here.
'' [OUT] pIpSettings As tPvIpSettings, The IP settings will be copied here, NULL pointer OK.
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrNotFound,        the camera was not found
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''               ePvErrBadParameter,    pIpSettings->size is invalid
''
'' The specified camera may not be visible to PvCameraList(); it might be on a
'' different ethernet network.  In this case, communication with the camera is
'' routed to the local gateway.
''
Declare Function PvCameraInfoByAddr Lib "PvApi.dll" (ByVal IpAddr As Long, ByRef pInfo As tPvCameraInfo, ByRef pIpSettings As tPvIpSettings) As tPvErr

''----- Opening & Closing -----------------------------------------------------

''
'' Function:  PvCameraOpen()
''
'' Purpose:   Open the specified camera.  This function must be called before
''            you can control the camera.
''
'' Arguments:
''
'' [ IN] UniqueId As Long,   UniqueId of the camera
'' [ IN] AccessFlag As Long, Access flag {monitor, master}
'' [OUT] pCamera As Long,    Handle to the opened camera returned here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrAccessDenied,    the camera couldn't be open in the requested mode
''               ePvErrNotFound,        the camera was not found (unplugged)
''               ePvErrUnplugged,       the camera was found but unplugged during the
''                                      function call
''               ePvErrBadParameter,    a valid pointer for pCamera was not supplied
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized or camera is alreay open
''
'' If ePvErrSuccess is returned, you must eventually call PvCameraClose().
''
'' Alternatively, under special circumstances, you might open an ethernet
'' camera with PvCameraOpenByAddr().
''
Declare Function PvCameraOpen Lib "PvApi.dll" (ByVal UniqueID As Long, ByVal AccessFlag As Long, ByRef pCamera As Long) As tPvErr

''
'' Function:  PvCameraOpenByAddr()
''
'' Purpose:   Open the specified camera, by IP address.  This function is
''            required, in place of PvCameraOpen(), if the ethernet camera
''            is not on the local ethernet network.
''
'' Arguments:
''
'' [ IN] IpAddr As Long,     IP of the camera
'' [ IN] AccessFlag As Long, Access flag {monitor, master}
'' [OUT] pCamera As Long,    Handle to the opened camera returned here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrAccessDenied,    the camera couldn't be open in the requested mode
''               ePvErrNotFound,        the camera was not found (unplugged)
''               ePvErrUnplugged,       the camera was found but unplugged during the
''                                      function call
''               ePvErrBadParameter,    a valid pointer for pCamera was not supplied
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized or camera is alreay open
''
'' If ePvErrSuccess is returned, you must eventually call PvCameraClose().
''
'' The specified camera may not be visible to PvCameraList(); it might be on a
'' different ethernet network.  In this case, communication with the camera is
'' routed to the local gateway.
''
Declare Function PvCameraOpenByAddr Lib "PvApi.dll" (ByVal IpAddr As Long, ByVal AccessFlag As Long, ByRef pCamera As Long) As tPvErr

''
'' Function:  PvCameraClose()
''
'' Purpose:   Close the specified camera.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle of an opened camera
''
'' Return:    ePvErrBadHandle if the handle is bad, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,    the handle of the camera is invalid
''               ePvErrBadSequence,  API isn't initialized
''
''
Declare Function PvCameraClose Lib "PvApi.dll" (ByVal Camera As Long) As tPvErr

''
'' Function:  PvCameraIpSettingsGet()
''
'' Purpose:   Get IP settings for an ethernet camera.  This command will work
''            for all cameras on the local ethernet network, including
''            "unreachable" cameras.
''
'' Arguments:
''
'' [ IN] UniqueId As Long,            UniqueId of the camera
'' [OUT] pSettings As tPvIpSettings,  Camera settings are copied here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrNotFound,        the camera was not found (or is not
''                                      an ethernet camera)
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''               ePvBadParameter,       pSettings->size is incorrect
''
'' The camera does not have to be opened to run this command.
''
Declare Function PvCameraIpSettingsGet Lib "PvApi.dll" (ByVal UniqueID As Long, ByRef pSettings As tPvIpSettings) As tPvErr


''
'' Function:  PvCameraIpSettingsChange()
''
'' Purpose:   Change the IP settings for an ethernet camera.  This command
''            will work for all cameras on the local ethernet network,
''            including "unreachable" cameras.
''
'' Arguments:
''
'' [ IN] UniqueId As Long,           UniqueId of the camera
'' [ IN] pSettings As tPvIpSettings, New camera settings
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrNotFound,        the camera was not found
''               ePvErrAccessDenied,    the camera was already open
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''               ePvBadParameter,       pSettings->size is incorrect
''
'' This command will fail if any application on any host has opened the camera.
''
Declare Function PvCameraIpSettingsChange Lib "PvApi.dll" (ByVal UniqueID As Long, ByRef pSettings As tPvIpSettings) As tPvErr

''----- Image Capture ---------------------------------------------------------

''
'' Function:  PvCaptureStart()
''
'' Purpose:   Setup the camera interface for image transfer.  This does not
''            necessarily start acquisition.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized or capture already started
''
'' PvCaptureStart() must be run before PvCaptureQueueFrame() is allowed.  But
'' the camera will not acquire images before the AcquisitionMode attribute is
'' set to a non-idle mode.
''
Declare Function PvCaptureStart Lib "PvApi.dll" (ByVal Camera As Long) As tPvErr

''
'' Function:  PvCaptureEnd()
''
'' Purpose:   Disable the image transfer mechanism.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized or capture already stopped
''
'' This cannot be called until the frame queue is empty.
''
Declare Function PvCaptureEnd Lib "PvApi.dll" (ByVal Camera As Long) As tPvErr

''
'' Function:  PvCaptureQuery()
''
'' Purpose:   Check to see if a camera interface is ready to transfer images.
''            I.e. has PvCaptureStart() been called?
''
'' Arguments:
''
'' [ IN] Camera As Long,     Handle to the camera
'' [OUT] pIsStarted As Long, Result returned here: 1=started, 0=disabled
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvBadParameter,       a valid pointer for pIsStarted was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Declare Function PvCaptureQuery Lib "PvApi.dll" (ByVal Camera As Long, ByRef pIsStarted As Long) As tPvErr

''
'' Function:  PvCaptureAdjustPacketSize()
''
'' Purpose:   Determine the maximum packet size supported by the system.
''
'' Arguments:
''
'' [ IN] Camera As Long,            Handle to the camera
'' [ IN] MaximumPacketSize As Long, Upper limit: the packet size will
''                                   not be set higher than this value.
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized or capture already started
''
'' The maximum packet size can be limited by the camera, host adapter, and
'' ethernet switch.
''
'' PvCaptureAdjustPacketSize() cannot be run when capture has started.
''
Declare Function PvCaptureAdjustPacketSize Lib "PvApi.dll" (ByVal Camera As Long, ByVal MaximumPacketSize As Long) As tPvErr

''
'' Function:  PvCaptureQueueFrame()
''
'' Purpose:   Queue a frame buffer for image capture.
''
'' Arguments:
''
'' [ IN] Camera As Long,     Handle to the camera
'' [ IN] pFrame As tPvFrame, Frame to queue
'' [ IN] Callback As Long,   Callback to run when the frame is done (use AddressOf)
''                            may be 0 for no callback.
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrQueueFull,       the frame queue is full
''               ePvErrResources,       resources requested from the OS were not
''                                      available
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized or capture not started
''
'' This function returns immediately.  If ePvErrSuccess is returned, the frame
'' will remain in the queue until it is complete, or aborted due to an error or
'' a call to PvCaptureQueueClear().
''
'' Frames are completed (or aborted) in the order they are queued.
''
'' You can specify a callback function (the "frame-done callback") to occur
'' when the frame is complete, or you can use PvCaptureWaitForFrameDone() to
'' block until the frame is complete. Due to VB using callbacks is NOT recommended.
''
'' Your VB function must be defined in a BAS file and of the following form:
''
'' Sub Frame_Event(ByRef Frame As tPvFrame)
''
'' End Sub
''
'' When the frame callback starts, the tPvFrame data structure is no longer in
'' use and you are free to do with it as you please (for example, reuse or
'' deallocation.)
''
'' Each frame on the queue must have a unique tPvFrame data structure.
''
Declare Function PvCaptureQueueFrame Lib "PvApi.dll" (ByVal Camera As Long, ByRef pFrame As tPvFrame, ByVal Callback As Long) As tPvErr
 
''
'' Function:  PvCaptureQueueClear()
''
'' Purpose:   Empty the frame queue.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
'' Queued frames are returned with status ePvErrCancelled.
''
'' PvCaptureQueueClear() cannot be called from the frame-done callback.
''
'' When this function returns, no more frames are left on the queue and you
'' will not receive another frame callback.
''
Declare Function PvCaptureQueueClear Lib "PvApi.dll" (ByVal Camera As Long) As tPvErr
 
''
'' Function:  PvCaptureWaitForFrameDone()
''
'' Purpose:   Wait for a frame capture to complete.
''
'' Arguments:
''
'' [ IN] Camera As Long,     Handle to the camera
'' [ IN] pFrame As tPvFrame, Frame to wait upon
'' [ IN] Timeout As Long,    Wait timeout (in milliseconds); use
''                            PVINFINITE for no timeout
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrTimeout,         timeout while waiting for the frame
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
'' This function cannot be called from the frame-done callback.
''
'' When this function returns, the frame structure is no longer in use and you
'' are free to do with it as you please (for example, reuse or deallocation).
''
'' If you are using the frame-done callback: this function might return first,
'' or the frame callback might be called first.
''
'' If the specified frame is not on the queue, this function returns
'' ePvErrSuccess, since we do not know if the frame just left the queue.
''
Declare Function PvCaptureWaitForFrameDone Lib "PvApi.dll" (ByVal Camera As Long, ByRef pFrame As tPvFrame, ByVal Timeout As Long) As tPvErr
 
''----- Utility ---------------------------------------------------------------

''
'' Function:  PvUtilityColorInterpolate()
''
'' Purpose:   Perform color interpolation.  Input format is bayer8 or bayer16
''            (raw bayer image).  Output format is RGB or separate color
''            planes.
''
'' Arguments:
''
'' [ IN] pFrame As tPvFrame,   Raw bayer image
'' [OUT] BufferRed As Long,    Red plane for output image
'' [OUT] BufferGreen As Long,  Green plane for output image
'' [OUT] BufferBlue As Long,   Blue plane for output image
'' [ IN] PixelPadding As Long, Padding after each pixel in raw-pixel units
'' [ IN] LinePadding As Long,  Padding after each line in raw-pixel units
''
'' Return:    none
''
'' Caller must allocate the output buffers:
''
''      num_pixels = (((1 + pixel_padding) * width) + line_padding) * height
''
''      buffer_size = raw_pixel_size * num_pixels
''
'' Perhaps the most common way to use this function will be to generate a
'' Win32::StretchDiBits compatible BGR buffer:
''
''      #define ULONG_PADDING(x)                        (((x+3) & ~3) - x)
''
''      unsigned long line_padding = ULONG_PADDING(width*3);
''      unsigned long buffer_size = ((width*3) + line_padding) * height;
''
''      unsigned char* buffer = new unsigned char[buffer_size];
''
''      PvUtilityColorInterpolate(&frame, &buffer[2], &buffer[1],
''                                &buffer[0], 2, line_padding);
''
''
Declare Sub PvUtilityColorInterpolate Lib "PvApi.dll" (ByRef pFrame As tPvFrame, ByVal BufferRed As Long, ByVal BufferGreen As Long, ByVal BufferBlue As Long, ByVal PixelPadding As Long, ByVal LinePadding As Long)

'' Utility function converting an array of bytes into a VB string
Function BytesToString(ByRef Table() As Byte) As String

    Dim Str As String
    Dim i As Long
    
    Str = ""
    i = 0
    
    Do
    
        If Table(i) <> 0 Then
            Str = Str + Chr(Table(i))
            i = i + 1
        Else
            Exit Do
        End If
    
    Loop
    
    BytesToString = Str

End Function

'' Utility function converting a VB string into an array of bytes
Sub StringToBytes(ByRef Str As String, ByRef Buffer() As Byte)

    ReDim Buffer(Len(Str) + 1)
    
    For i = 1 To Len(Str)

        Buffer(i - 1) = Asc(Mid(Str, i, 1))

    Next
    
    Buffer(i - 1) = 0

End Sub

Function UnsignedAdd(ByVal L1 As Long, ByVal L2 As Long) As Long

    Dim L11 As Long, L12 As Byte, L21 As Long, L22 As Byte, L31 As Long, L32 As Byte
    
    L11 = L1 And &HFFFFFF
    L12 = (L1 And &H7F000000) \ &H1000000
    If L1 < 0& Then L12 = L12 Or &H80
    L21 = L2 And &HFFFFFF
    L22 = (L2 And &H7F000000) \ &H1000000
    If L2 < 0& Then L22 = L22 Or &H80
    L32 = L12 + L22
    L31 = L11 + L21
    If (L31 And &H1000000) Then L32 = L32 + 1
    UnsignedAdd = (L31 And &HFFFFFF) + (L32 And &H7F) * &H1000000
    If L32 And &H80 Then UnsignedAdd = UnsignedAdd Or &H80000000
    
End Function

Function BufferToString(ByVal Adr As Long) As String

    Dim OneByte As Byte
    
    Call CopyMemory(VarPtr(OneByte), Adr, 1)
    
    BufferToString = ""
    
    While OneByte <> 0
    
        BufferToString = BufferToString + Chr(OneByte)
        Adr = UnsignedAdd(Adr, 1)
        Call CopyMemory(VarPtr(OneByte), Adr, 1)
    
    Wend

End Function

''----- Attributes ------------------------------------------------------------

''
'' Function:  PvAttrList()
''
'' Purpose:   List all the attributes for a given camera.
''
'' Arguments:
''
'' [ IN] Camera As Long,      Handle to the camera
'' [OUT] Strings() As String, array of strings
'' [OUT] pLength As Long      Length of the list is returned here.  The
''                             length never changes while the camera
''                             remains opened.
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvBadParameter,       a valid pointer for pListPtr was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
'' The attribute list is copied in the provided strings array
''
Function PvAttrList(ByVal Camera As Long, ByRef Strings() As String, ByRef pLength As Long) As tPvErr

    Dim pAttributes As Long
    Dim err As tPvErr

    err = PvAttrListInternal(Camera, pAttributes, pLength)

    If err = ePvErrSuccess Then
            
        Dim OneByte As Byte
        Dim Adr As Long
        Dim i As Long
        Dim j As Long
        
        ReDim Strings(pLength)
                       
        For i = 0 To pLength - 1
        
            Call CopyMemory(VarPtr(Adr), pAttributes + (4 * i), 4)
            Strings(i) = BufferToString(Adr)

        Next
                
    End If
    
    PvAttrList = err
    
End Function

''
'' Function:  PvAttrInfo()
''
'' Purpose:   Retrieve information on an attribute.  This information is
''            static for each camera.
''
'' Arguments:
''
'' [ IN] Camera As Long,            Handle to the camera
'' [ IN] Name As String,            Attribute name
'' [OUT] pInfo As tPvAttributeInfo, The information is copied here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvBadParameter,       a valid pointer for pInfo was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrInfo(ByVal Camera As Long, ByRef Name As String, ByRef pInfo As tPvAttributeInfo) As tPvErr

    Dim Info As tPvAttributeInfoInternal
    Dim err As tPvErr
    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    err = PvAttrInfoInternal(Camera, VarPtr(Str(0)), Info)
    
    If err = ePvErrSuccess Then
    
        pInfo.Datatype = Info.Datatype
        pInfo.Flags = Info.Flags
        
        pInfo.Category = BufferToString(Info.Category)
        If Info.Impact Then
            pInfo.Impact = BufferToString(Info.Impact)
        End If
    
    End If
    
    PvAttrInfo = err

End Function

''
'' Function:  PvAttrExists()
''
'' Purpose:   Check if an attribute exists for the camera.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
''
'' Return:       ePvErrSuccess,         the attribute exists
''               ePvErrNotFound,        the attribute does not exist
''
''            The following error codes may also occur:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrExists(ByVal Camera As Long, ByRef Name As String) As tPvErr

    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvAttrExists = PvAttrExistsInternal(Camera, VarPtr(Str(0)))
    
End Function

''
'' Function:  PvAttrIsAvailable()
''
'' Purpose:   Check if an attribute is available.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
''
'' Return:       ePvErrSuccess,         the attribute is available
''               ePvErrUnavailable,     the attribute is not available
''
''            The following error codes may also occur:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrIsAvailable(ByVal Camera As Long, ByRef Name As String) As tPvErr

    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvAttrIsAvailable = PvAttrIsAvailableInternal(Camera, VarPtr(Str(0)))
    
End Function

''
'' Function:  PvAttrIsValid()
''
'' Purpose:   Check if an attribute's value is valid.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
''
'' Return:       ePvErrSuccess,         the attribute is valid
''               ePvErrOutOfRange,      the attribute is not valid
''
''            The following error codes may also occur:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrOutOfRange,      the requested attribute is not valid
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrIsValid(ByVal Camera As Long, ByRef Name As String) As tPvErr

    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvAttrIsValid = PvAttrIsValidInternal(Camera, VarPtr(Str(0)))
    
End Function

''
'' Function:  PvAttrRangeEnum()
''
'' Purpose:   Get the enumeration set for an enumerated attribute.  The set is
''            returned as a comma separated string containing all allowed
''            values.
''
'' Arguments:
''
'' [ IN] Camera As Long,  Handle to the camera
'' [ IN] Name As String,  Attribute name
'' [OUT] Range As String, string where the range is copied
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pBuffer was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
'' Enumeration sets must be considered dynamic.  For some attributes, the set
'' may change at any time.
''
Function PvAttrRangeEnum(ByVal Camera As Long, ByRef Name As String, ByRef Range As String) As tPvErr

    Dim Str() As Byte
    Dim Buffer() As Byte
    Dim size As Long
    Dim err As tPvErr
    Dim Length As Long
    Dim need As Long
       
    Call StringToBytes(Name, Str)
    
    Length = 64
    
    Do
    
        ReDim Buffer(Length)
    
        err = PvAttrRangeEnumInternal(Camera, VarPtr(Str(0)), VarPtr(Buffer(0)), Length, need)
        
        If err = ePvErrBadParameter Then
            Length = need + 1
        Else
            Exit Do
        End If
        
    Loop
    
    If err = ePvErrSuccess Then
    
        Range = BytesToString(Buffer)
    
    End If
    
    PvAttrRangeEnum = err

End Function

''
'' Function:  PvAttrRangeUint32()
''
'' Purpose:   Get the value range for a uint32 attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
'' [OUT] pMin As Long,   Minimum value returned here
'' [OUT] pMax As Long,   Maximum value returned here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pMin or pMax was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrRangeUint32(ByVal Camera As Long, ByRef Name As String, ByRef pMin As Long, ByRef pMax As Long) As tPvErr
    
    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvAttrRangeUint32 = PvAttrRangeUint32Internal(Camera, VarPtr(Str(0)), pMin, pMax)

End Function

''
'' Function:  PvAttrRangeInt64()
''
'' Purpose:   Get the value range for a int64 attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
'' [OUT] pMin As Long,   Minimum value returned here
'' [OUT] pMax As Long,   Maximum value returned here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pMin or pMax was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrRangeInt64(ByVal Camera As Long, ByRef Name As String, ByRef pMin As Long, ByRef pMax As Long) As tPvErr

    Dim Str() As Byte

    Call StringToBytes(Name, Str)

    PvAttrRangeInt64 = PvAttrRangeInt64Internal(Camera, VarPtr(Str(0)), pMin, pMax)

End Function


''
'' Function:  PvAttrRangeFloat32()
''
'' Purpose:   Get the value range for a float32 attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
'' [OUT] pMin As Single, Minimum value returned here
'' [OUT] pMax As Single, Maximum value returned here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pMin or pMax was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrRangeFloat32(ByVal Camera As Long, ByRef Name As String, ByRef pMin As Single, ByRef pMax As Single) As tPvErr
    
    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvAttrRangeFloat32 = PvAttrRangeFloat32Internal(Camera, VarPtr(Str(0)), pMin, pMax)

End Function

''
'' Function:  PvCommandRun()
''
'' Purpose:   Run a specific command on the camera
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvCommandRun(ByVal Camera As Long, ByRef Name As String) As tPvErr
    
    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvCommandRun = PvCommandRunInternal(Camera, VarPtr(Str(0)))

End Function

''
'' Function:  PvAttrStringGet()
''
'' Purpose:   Get the value of a string attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long,          Handle to the camera
'' [ IN] Name As String,          Attribute name
'' [OUT] value As String,         string where the value is copied
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pBuffer was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrStringGet(ByVal Camera As Long, ByRef Name As String, ByRef Value As String) As tPvErr

    Dim Str() As Byte
    Dim Buffer() As Byte
    Dim size As Long
    Dim err As tPvErr
    Dim Length As Long
    Dim need As Long
       
    Call StringToBytes(Name, Str)
    
    Length = 64
    
    Do
    
        ReDim Buffer(Length)
    
        err = PvAttrStringGetInternal(Camera, VarPtr(Str(0)), VarPtr(Buffer(0)), Length, need)
        
        If err = ePvErrBadParameter Then
            Length = need + 1
        Else
            Exit Do
        End If
        
    Loop
    
    If err = ePvErrSuccess Then
    
        Value = BytesToString(Buffer)
     
    End If
    
    PvAttrStringGet = err

End Function

''
'' Function:  PvAttrStringSet()
''
'' Purpose:   Set the value of a string attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long,  Handle to the camera
'' [ IN] Name As String,  Attribute name
'' [ IN] value As String, value to be set
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvErrForbidden,       the requested attribute forbid this operation
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrStringSet(ByVal Camera As Long, ByRef Name As String, ByRef Value As String) As tPvErr
    
    Dim Str() As Byte
    Dim Val() As Byte
       
    Call StringToBytes(Name, Str)
    Call StringToBytes(Value, Val)
    
    PvAttrStringSet = PvAttrStringSetInternal(Camera, VarPtr(Str(0)), VarPtr(Val(0)))

End Function

''
'' Function:  PvAttrEnumGet()
''
'' Purpose:   Get the value of an enumerated attribute.  The enumeration value
''            is a string.
''
'' Arguments:
''
'' [ IN] Camera As Long,  Handle to the camera
'' [ IN] Name As String,  Attribute name
'' [OUT] value As String, string where the value is copied
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pBuffer was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrEnumGet(ByVal Camera As Long, ByRef Name As String, ByRef Value As String) As tPvErr

    Dim Str() As Byte
    Dim Buffer() As Byte
    Dim size As Long
    Dim err As tPvErr
    Dim Length As Long
    Dim need As Long
       
    Call StringToBytes(Name, Str)
    
    Length = 64
    
    Do
    
        ReDim Buffer(Length)
    
        err = PvAttrEnumGetInternal(Camera, VarPtr(Str(0)), VarPtr(Buffer(0)), Length, need)
        
        If err = ePvErrBadParameter Then
            Length = need + 1
        Else
            Exit Do
        End If
        
    Loop
    
    If err = ePvErrSuccess Then
    
        Value = BytesToString(Buffer)
     
    End If
    
    PvAttrEnumGet = err

End Function

''
'' Function:  PvAttrEnumSet()
''
'' Purpose:   Set the value of an enumerated attribute.  The enumeration value
''            is a string.
''
'' Arguments:
''
'' [ IN] Camera As Long,  Handle to the camera
'' [ IN] Name As String,  Attribute name
'' [ IN] value As String, Value to set
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvErrForbidden,       the requested attribute forbid this operation
''               ePvErrOutOfRange,      the supplied value is out of range
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrEnumSet(ByVal Camera As Long, ByRef Name As String, ByRef Value As String) As tPvErr
    
    Dim Str() As Byte
    Dim Val() As Byte
       
    Call StringToBytes(Name, Str)
    Call StringToBytes(Value, Val)
    
    PvAttrEnumSet = PvAttrEnumSetInternal(Camera, VarPtr(Str(0)), VarPtr(Val(0)))

End Function

''
'' Function:  PvAttrUint32Get()
''
'' Purpose:   Get the value of a uint32 attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
'' [OUT] value As Long,  Value is returned here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pValue was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrUint32Get(ByVal Camera As Long, ByRef Name As String, ByRef Value As Long) As tPvErr

    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvAttrUint32Get = PvAttrUint32GetInternal(Camera, VarPtr(Str(0)), Value)

End Function

''
'' Function:  PvAttrUint32Set()
''
'' Purpose:   Set the value of a uint32 attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
'' [ IN] Value As Long,  Value to set
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvErrForbidden,       the requested attribute forbid this operation
''               ePvErrOutOfRange,      the supplied value is out of range
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrUint32Set(ByVal Camera As Long, ByRef Name As String, ByVal Value As Long) As tPvErr

    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvAttrUint32Set = PvAttrUint32SetInternal(Camera, VarPtr(Str(0)), Value)

End Function

''
'' Function:  PvAttrBooleanGet()
''
'' Purpose:   Get the value of a boolean attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
'' [OUT] value As Long,  Value is returned here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pValue was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrBooleanGet(ByVal Camera As Long, ByRef Name As String, ByRef Value As Boolean) As tPvErr

    Dim Str() As Byte

    Call StringToBytes(Name, Str)

    PvAttrBooleanGet = PvAttrBooleanGetInternal(Camera, VarPtr(Str(0)), Value)

End Function

''
'' Function:  PvAttrBooleanSet()
''
'' Purpose:   Set the value of a boolean attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
'' [ IN] Value As Long,  Value to set
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvErrForbidden,       the requested attribute forbid this operation
''               ePvErrOutOfRange,      the supplied value is out of range
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrBooleanSet(ByVal Camera As Long, ByRef Name As String, ByVal Value As Boolean) As tPvErr

    Dim Str() As Byte

    Call StringToBytes(Name, Str)

    PvAttrBooleanSet = PvAttrBooleanSetInternal(Camera, VarPtr(Str(0)), Value)

End Function

''
'' Function:  PvAttrInt64Get()
''
'' Purpose:   Get the value of a int64 attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
'' [OUT] value As Long,  Value is returned here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pValue was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrInt64Get(ByVal Camera As Long, ByRef Name As String, ByRef Value As Long) As tPvErr

    Dim Str() As Byte

    Call StringToBytes(Name, Str)

    PvAttrInt64Get = PvAttrInt64GetInternal(Camera, VarPtr(Str(0)), Value)

End Function

''
'' Function:  PvAttrInt64Set()
''
'' Purpose:   Set the value of a int64 attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long, Handle to the camera
'' [ IN] Name As String, Attribute name
'' [ IN] Value As Long,  Value to set
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvErrForbidden,       the requested attribute forbid this operation
''               ePvErrOutOfRange,      the supplied value is out of range
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrInt64Set(ByVal Camera As Long, ByRef Name As String, ByVal Value As Long) As tPvErr

    Dim Str() As Byte

    Call StringToBytes(Name, Str)

    PvAttrInt64Set = PvAttrInt64SetInternal(Camera, VarPtr(Str(0)), Value)

End Function

''
'' Function:  PvAttrFloat32Get()
''
'' Purpose:   Get the value of a float32 attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long,  Handle to the camera
'' [ IN] Name As String,  Attribute name
'' [OUT] Value As Single, Value is returned here
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvBadParameter,       a valid pointer for pValue was not supplied
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrFloat32Get(ByVal Camera As Long, ByRef Name As String, ByRef Value As Single) As tPvErr

    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvAttrFloat32Get = PvAttrFloat32GetInternal(Camera, VarPtr(Str(0)), Value)

End Function

''
'' Function:  PvAttrFloat32Set()
''
'' Purpose:   Set the value of a float32 attribute.
''
'' Arguments:
''
'' [ IN] Camera As Long,  Handle to the camera
'' [ IN] Name As String,  Attribute name
'' [ IN] Value As Single, Value to set
''
'' Return:    ePvErrSuccess if no error, otherwise likely to be any of the
''            following error codes:
''
''               ePvErrBadHandle,       the handle of the camera is invalid
''               ePvErrUnplugged,       the camera has been unplugged
''               ePvErrNotFound,        the requested attribute doesn't exist
''               ePvErrWrongType,       the requested attribute is not of the correct type
''               ePvErrForbidden,       the requested attribute forbid this operation
''               ePvErrOutOfRange,      the supplied value is out of range
''               ePvErrInternalFault,   an internal fault occurred
''               ePvErrBadSequence,     API isn't initialized
''
Function PvAttrFloat32Set(ByVal Camera As Long, ByRef Name As String, ByVal Value As Single) As tPvErr

    Dim Str() As Byte
       
    Call StringToBytes(Name, Str)
    
    PvAttrFloat32Set = PvAttrFloat32SetInternal(Camera, VarPtr(Str(0)), Value)

End Function

