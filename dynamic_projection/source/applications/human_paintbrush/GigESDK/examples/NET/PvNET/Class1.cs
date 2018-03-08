/*
| ==============================================================================
| Copyright (C) 2006-2011 Prosilica.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Prosilica is prohibited.
|
|==============================================================================
|
| This file define the .NET interface to Prosilica Vision API
|
|==============================================================================
|
| THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
| WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
| NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
| DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
| INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
| LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
| OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF
| LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
| NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
| EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
|
|==============================================================================
*/

using System;
using System.Text;
using System.Runtime.InteropServices;

namespace PvNET
{
	/// <summary>
	/// .NET interface to Prosilica Vision API
	/// </summary>
	
	public enum tErr :uint
	{
		eErrSuccess       = 0,        // No error
		eErrCameraFault   = 1,        // Unexpected camera fault
		eErrInternalFault = 2,        // Unexpected fault in PvApi or driver
		eErrBadHandle     = 3,        // Camera handle is invalid
		eErrBadParameter  = 4,        // Bad parameter to API call
		eErrBadSequence   = 5,        // Sequence of API calls is incorrect
		eErrNotFound      = 6,        // Camera or attribute not found
		eErrAccessDenied  = 7,        // Camera cannot be opened in the specified mode
		eErrUnplugged     = 8,        // Camera was unplugged
		eErrInvalidSetup  = 9,        // Setup is invalid (an attribute is invalid)
		eErrResources     = 10,       // System/network resources or memory not available
		eErrBandwidth     = 11,       // 1394 bandwidth not available
		eErrQueueFull     = 12,       // Too many frames on queue
		eErrBufferTooSmall= 13,       // Frame buffer is too small
		eErrCancelled     = 14,       // Frame cancelled by user
		eErrDataLost      = 15,       // The data for the frame was lost
		eErrDataMissing   = 16,       // Some data in the frame is missing
		eErrTimeout       = 17,       // Timeout during wait
		eErrOutOfRange    = 18,       // Attribute value is out of the expected range
		eErrWrongType     = 19,       // Attribute is not this type (wrong access function) 
		eErrForbidden     = 20,       // Attribute write forbidden at this time
		eErrUnavailable   = 21,       // Attribute is not available at this time	
	};

	public enum tAccessFlags :uint
	{
		eAccessMonitor        = 2, // Monitor access: no control, read & listen only
		eAccessMaster         = 4  // Master access: full control
	};

	public enum tInterface :uint
	{
		eInterfaceFirewire    = 1,
		eInterfaceEthernet    = 2
	};

	public enum tLinkEvent :uint
	{
		eLinkAdd          = 1, // A camera was plugged in
		eLinkRemove       = 2, // A camera was unplugged
	}; 

	public enum tDatatype :uint
	{
		eDatatypeUnknown    = 0,
		eDatatypeCommand    = 1,
		eDatatypeRaw        = 2,
		eDatatypeString     = 3,
		eDatatypeEnum       = 4,
		eDatatypeUint32     = 5,
		eDatatypeFloat32    = 6,
        eDatatypeInt64      = 7,
        eDatatypeBoolean    = 8

	};

	public enum tIpConfig :uint
	{
		eIpConfigPersistent   = 1,            // Use persistent IP settings
		eIpConfigDhcp         = 2,            // Use DHCP, fallback to AutoIP
		eIpConfigAutoIp       = 4             // Use AutoIP only
	};

	public enum tImageFormat :uint
	{
        eFmtMono8           = 0,            // Monochrome, 8 bits
        eFmtMono16          = 1,            // Monochrome, 16 bits, data is LSB aligned
        eFmtBayer8          = 2,            // Bayer-color, 8 bits
        eFmtBayer16         = 3,            // Bayer-color, 16 bits, data is LSB aligned
        eFmtRgb24           = 4,            // RGB, 8 bits x 3
        eFmtRgb48           = 5,            // RGB, 16 bits x 3, data is LSB aligned
        eFmtYuv411          = 6,            // YUV 411
        eFmtYuv422          = 7,            // YUV 422
        eFmtYuv444          = 8,            // YUV 444
        eFmtBgr24           = 9,            // BGR, 8 bits x 3
        eFmtRgba32          = 10,           // RGBA, 8 bits x 4
        eFmtBgra32          = 11,           // BGRA, 8 bits x 4
        eFmtMono12Packed    = 12,           // Monochrome, 12 bits, 
        eFmtBayer12Packed   = 13            // Bayer-color, 12 bits, packed 
	};

	public enum tBayerPattern :uint
	{
		eBayerRGGB        = 0,            // First line RGRG, second line GBGB...
		eBayerGBRG        = 1,            // First line GBGB, second line RGRG...
		eBayerGRBG        = 2,            // First line GRGR, second line BGBG...
		eBayerBGGR        = 3             // First line BGBG, second line GRGR...
	};

	[StructLayout(LayoutKind.Sequential, Pack=8, CharSet=CharSet.Ansi)]
	public struct tIpSettings
	{
		// IP configuration mode: persistent, DHCP & AutoIp, or AutoIp only.
		public tIpConfig	ConfigMode;
		// IP configuration mode supported by the camera
		public UInt32       ConfigModeSupport;

		// Current IP configuration.  Ignored for PvCameraIpSettingsChange().  All
		// values are in network byte order (i.e. big endian).
		public UInt32       CurrentIpAddress;
		public UInt32       CurrentIpSubnet;
		public UInt32       CurrentIpGateway;

		// Persistent IP configuration.  See "ConfigMode" to enable persistent IP
		// settings.  All values are in network byte order.
		public UInt32       PersistentIpAddr;
		public UInt32       PersistentIpSubnet;
		public UInt32       PersistentIpGateway;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst=8)]
		public UInt32[]		_reserved1;         // Always zero
	};

	[StructLayout(LayoutKind.Sequential, Pack=8, CharSet=CharSet.Ansi)]
	public struct tCameraInfo
	{
		public UInt32       UniqueId;         // Unique value for each camera
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst=32)]
		public string		SerialString;     // Camera's serial number
		public UInt32       PartNumber;       // Camera part number
		public UInt32       PartVersion;      // Camera part version
		public UInt32       PermittedAccess;  // A combination of tPvAccessFlags
		public UInt32       InterfaceId;      // Unique value for each interface or bus
		public tInterface	InterfaceType;    // Interface type; see tPvInterface
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst=16)]
		public string		DisplayName;     // People-friendly camera name
		[MarshalAs(UnmanagedType.ByValArray, SizeConst=4)]
		public UInt32[]		_reserved;         // Always zero
	};

	[StructLayout(LayoutKind.Sequential, Pack=8, CharSet=CharSet.Ansi)]
	public struct tAttributeInfo
	{
		public tDatatype	Datatype;       // Data type
		public UInt32       Flags;          // Combination of tPvAttribute flags
        public IntPtr       Category;       // Advanced: see documentation
        public IntPtr       Impact;         // Advanced: see documentation
		[MarshalAs(UnmanagedType.ByValArray, SizeConst=4)]
		public UInt32[]		_reserved;      // Always zero
	};

	[StructLayout(LayoutKind.Sequential, Pack=8, CharSet=CharSet.Ansi)]
	public struct tFrameCtx
	{
		public IntPtr Field0;
		public IntPtr Field1;
		public IntPtr Field2;
		public IntPtr Field3;				
	};

	[StructLayout(LayoutKind.Sequential, Pack=8, CharSet=CharSet.Ansi)]
	public struct tFrameRes1
	{
		public UInt32 _reserved00;
		public UInt32 _reserved01;
		public UInt32 _reserved02;
		public UInt32 _reserved03;
		public UInt32 _reserved04;
		public UInt32 _reserved05;
		public UInt32 _reserved06;
		public UInt32 _reserved07;					
	};

	[StructLayout(LayoutKind.Sequential, Pack=8, CharSet=CharSet.Ansi)]
	public struct tFrameRes2
	{
		public UInt32 _reserved00;
		public UInt32 _reserved01;
		public UInt32 _reserved02;
		public UInt32 _reserved03;
		public UInt32 _reserved04;
		public UInt32 _reserved05;
		public UInt32 _reserved06;
		public UInt32 _reserved07;
		public UInt32 _reserved08;
		public UInt32 _reserved09;
		public UInt32 _reserved10;
		public UInt32 _reserved11;
		public UInt32 _reserved12;
		public UInt32 _reserved13;
		public UInt32 _reserved14;
		public UInt32 _reserved15;
		public UInt32 _reserved16;
		public UInt32 _reserved17;
		public UInt32 _reserved18;
		public UInt32 _reserved19;
		public UInt32 _reserved20;
		public UInt32 _reserved21;
		public UInt32 _reserved22;
		public UInt32 _reserved23;
		public UInt32 _reserved24;
		public UInt32 _reserved25;
		public UInt32 _reserved26;
		public UInt32 _reserved27;
		public UInt32 _reserved28;
		public UInt32 _reserved29;
		public UInt32 _reserved30;
		public UInt32 _reserved31;				
	};

	[StructLayout(LayoutKind.Sequential, Pack=8, CharSet=CharSet.Ansi)]
	public struct tFrame
	{
		//----- In -----
		public IntPtr			ImageBuffer;		// Your image buffer
		public UInt32			ImageBufferSize;    // Size of your image buffer in bytes
		public IntPtr           AncillaryBuffer;    // Your buffer to capture associated 
		//   header & trailer data for this image.
		public UInt32			AncillaryBufferSize;// Size of your ancillary buffer in bytes
		//   (can be 0 for no buffer).
		public tFrameCtx		Context;

		tFrameRes1				_Reserved1;			// Reserved data (always 0)

		//----- Out -----

		public tErr				Status;             // Status of this frame

		public UInt32			ImageSize;          // Image size, in bytes
		public UInt32			AncillarySize;      // Ancillary data size, in bytes

		public UInt32			Width;              // Image width
		public UInt32			Height;             // Image height
		public UInt32			RegionX;            // Start of readout region (left)
		public UInt32			RegionY;            // Start of readout region (top)
		public tImageFormat		Format;             // Image format
		public UInt32			BitDepth;           // Number of significant bits
		public tBayerPattern	BayerPattern;       // Bayer pattern, if bayer format

		public UInt32			FrameCount;         // Rolling frame counter
		public UInt32			TimestampLo;        // Time stamp, lower 32-bits
		public UInt32			TimestampHi;        // Time stamp, upper 32-bits

		tFrameRes2				_Reserved2;			// Reserved data (always 0)
	};

	public delegate void tLinkCallback(IntPtr Context,tInterface Interface,tLinkEvent Event,UInt32 UniqueId);
	public delegate void tFrameCallback(IntPtr pFrame);

    [StructLayout(LayoutKind.Sequential, Pack = 8, CharSet = CharSet.Ansi)]
    public struct tCameraEvent
    {
        public UInt32           EventId;        // Event ID
        public UInt32           TimestampLo;    // Time stamp, lower 32-bits
        public UInt32           TimestampHi;    // Time stamp, upper 32-bits         
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
        public UInt32[]         Data;           // Event data
        public IntPtr           ExtraData;      // Event extra-data (NULL if none)   
    };

    public delegate void tCameraEventCallback(IntPtr Context,UInt32 Camera,[MarshalAs(UnmanagedType.LPArray)] [In] tCameraEvent[] EventList,UInt32 EventListLength);

	public class Pv
	{
		[DllImport("PvAPI.dll", EntryPoint="PvInitialize",ExactSpelling=false)]
		public static extern tErr Initialize();

        [DllImport("PvAPI.dll", EntryPoint = "PvInitializeNoDiscovery", ExactSpelling = false)]
        public static extern tErr InitializeNoDiscovery();	

		[DllImport("PvAPI.dll", EntryPoint="PvUnInitialize",ExactSpelling=false)]
		public static extern void UnInitialize();

		[DllImport("PvAPI.dll", EntryPoint="PvVersion",ExactSpelling=false)]
		public static extern void Version(ref UInt32 Major ,ref UInt32 Minor);

		[DllImport("PvAPI.dll", EntryPoint="PvLinkCallbackRegister",ExactSpelling=false)]
		public static extern tErr LinkCallbackRegister(tLinkCallback Callback,tLinkEvent Event,IntPtr Context);	

		[DllImport("PvAPI.dll", EntryPoint="PvLinkCallbackUnRegister",ExactSpelling=false)]
		public static extern tErr LinkCallbackUnregister(tLinkCallback Callback,tLinkEvent Event);	

		[DllImport("PvAPI.dll", EntryPoint="PvCameraCount",ExactSpelling=false)]
		public static extern UInt32 CameraCount();

		[DllImport("PvAPI.dll", EntryPoint="PvCameraInfo",ExactSpelling=false)]
		public static extern tErr CameraInfo(UInt32 UniqueId,ref tCameraInfo Info);

		[DllImport("PvAPI.dll", EntryPoint="PvCameraInfoByAddr",ExactSpelling=false)]
		public static extern tErr CameraInfoByAddr(UInt32 IpAddr,ref tCameraInfo Info,ref tIpSettings IpSettings);

		[DllImport("PvAPI.dll", EntryPoint="PvCameraList",ExactSpelling=false)]
		public static extern UInt32 CameraList([MarshalAs(UnmanagedType.LPArray)] [In,Out] tCameraInfo[] Info,UInt32 ListLength,ref UInt32 ConnectedNum);

		[DllImport("PvAPI.dll", EntryPoint="PvCameraListUnreachable",ExactSpelling=false)]
		public static extern UInt32 CameraListUnreachable([MarshalAs(UnmanagedType.LPArray)] [In,Out] tCameraInfo[] Info,UInt32 ListLength,ref UInt32 ConnectedNum);

		[DllImport("PvAPI.dll", EntryPoint="PvCameraIpSettingsGet",ExactSpelling=false)]
		public static extern tErr CameraIpSettingsGet(UInt32 UniqueId,ref tIpSettings Settings);

		[DllImport("PvAPI.dll", EntryPoint="PvCameraIpSettingsChange",ExactSpelling=false)]
		public static extern tErr CameraIpSettingsSet(UInt32 UniqueId,ref tIpSettings Settings);

		[DllImport("PvAPI.dll", EntryPoint="PvCameraOpen",ExactSpelling=false)]
		public static extern tErr CameraOpen(UInt32 UniqueId,tAccessFlags AccessFlag,out UInt32 Camera);

		[DllImport("PvAPI.dll", EntryPoint="PvCameraOpenByAddr",ExactSpelling=false)]
		public static extern tErr CameraOpenByAddr(UInt32 IpAddr,tAccessFlags AccessFlag,out UInt32 Camera);

		[DllImport("PvAPI.dll", EntryPoint="PvCameraClose",ExactSpelling=false)]
		public static extern tErr CameraClose(UInt32 Camera);

		[DllImport("PvAPI.dll", EntryPoint="PvCaptureStart",ExactSpelling=false)]
		public static extern tErr CaptureStart(UInt32 Camera);

		[DllImport("PvAPI.dll", EntryPoint="PvCaptureEnd",ExactSpelling=false)]
		public static extern tErr CaptureEnd(UInt32 Camera);

		[DllImport("PvAPI.dll", EntryPoint="PvCaptureQuery",ExactSpelling=false)]
		public static extern tErr CaptureQuery(UInt32 Camera,ref UInt32 IsStarted);

		[DllImport("PvAPI.dll", EntryPoint="PvCaptureAdjustPacketSize",ExactSpelling=false)]
		public static extern tErr CaptureAdjustPacketSize(UInt32 Camera,UInt32 MaxSize);

		[DllImport("PvAPI.dll", EntryPoint="PvCaptureQueueFrame",ExactSpelling=false)]
		public static extern tErr CaptureQueueFrame(UInt32 Camera,[In,Out] IntPtr Frame,tFrameCallback Callback);

		[DllImport("PvAPI.dll", EntryPoint="PvCaptureQueueClear",ExactSpelling=false)]
		public static extern tErr CaptureQueueClear(UInt32 Camera);

		[DllImport("PvAPI.dll", EntryPoint="PvCaptureWaitForFrameDone",ExactSpelling=false)]
		public static extern tErr CaptureWaitForFrameDone(UInt32 Camera,[In,Out]IntPtr Frame,UInt32 Timeout);

        [DllImport("PvAPI.dll", EntryPoint = "PvCameraEventCallbackRegister", ExactSpelling = false)]
        public static extern tErr CameraEventCallbackRegister(UInt32 Camera,[In, Out] tCameraEventCallback Callback,IntPtr Context);

        [DllImport("PvAPI.dll", EntryPoint = "PvCameraEventCallbackUnRegister", ExactSpelling = false)]
        public static extern tErr CameraEventCallbackUnRegister(UInt32 Camera,[In, Out] tCameraEventCallback Callback);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrList",ExactSpelling=false)]
		public static extern tErr AttrList(UInt32 Camera,ref IntPtr ListPtr,ref UInt32 Length);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrInfo",ExactSpelling=false)]
		public static extern tErr AttrInfo(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,ref tAttributeInfo Info);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrExists",ExactSpelling=false)]
		public static extern tErr AttrExists(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrIsAvailable",ExactSpelling=false)]
		public static extern tErr AttrIsAvailable(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrIsValid",ExactSpelling=false)]
		public static extern tErr AttrIsValid(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrRangeEnum",ExactSpelling=false)]
		public static extern tErr AttrRangeEnum(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,StringBuilder String,UInt32 BufferSize,ref UInt32 Size);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrRangeUint32",ExactSpelling=false)]
		public static extern tErr AttrRangeUint32(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,ref UInt32 Min,ref UInt32 Max);

        [DllImport("PvAPI.dll", EntryPoint= "PvAttrRangeInt64", ExactSpelling = false)]
        public static extern tErr AttrRangeInt64(UInt32 Camera, [MarshalAs(UnmanagedType.LPStr)] String Name, ref Int64 Min, ref Int64 Max); 

        [DllImport("PvAPI.dll", EntryPoint="PvAttrRangeFloat32",ExactSpelling=false)]
		public static extern tErr AttrRangeFloat32(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,ref float Min,ref float Max);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrStringGet",ExactSpelling=false)]
		public static extern tErr AttrStringGet(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,StringBuilder String,UInt32 BufferSize,ref UInt32 Size);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrStringSet",ExactSpelling=false)]
		public static extern tErr AttrStringSet(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,[MarshalAs(UnmanagedType.LPStr)] String Value);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrEnumGet",ExactSpelling=false)]
		public static extern tErr AttrEnumGet(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,StringBuilder String,UInt32 BufferSize,ref UInt32 Size);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrEnumSet",ExactSpelling=false)]
		public static extern tErr AttrEnumSet(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,[MarshalAs(UnmanagedType.LPStr)] String Value);

        [DllImport("PvAPI.dll", EntryPoint = "PvAttrBooleanGet", ExactSpelling = false)]
        public static extern tErr AttrBooleanGet(UInt32 Camera, [MarshalAs(UnmanagedType.LPStr)] String Name, [In, Out] ref Boolean Value);

        [DllImport("PvAPI.dll", EntryPoint = "PvAttrBooleanSet", ExactSpelling = false)]
        public static extern tErr AttrBooleanSet(UInt32 Camera, [MarshalAs(UnmanagedType.LPStr)] String Name, Boolean Value);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrUint32Get",ExactSpelling=false)]
		public static extern tErr AttrUint32Get(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,[In,Out] ref UInt32 Value);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrUint32Set",ExactSpelling=false)]
		public static extern tErr AttrUint32Set(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,UInt32 Value);

        [DllImport("PvAPI.dll", EntryPoint = "PvAttrInt64Get", ExactSpelling = false)]
        public static extern tErr AttrInt64Get(UInt32 Camera, [MarshalAs(UnmanagedType.LPStr)] String Name, [In, Out] ref Int64 Value);

        [DllImport("PvAPI.dll", EntryPoint = "PvAttrInt64Set", ExactSpelling = false)]
        public static extern tErr AttrInt64Set(UInt32 Camera, [MarshalAs(UnmanagedType.LPStr)] String Name, Int64 Value);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrFloat32Get",ExactSpelling=false)]
		public static extern tErr AttrFloat32Get(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,ref float Value);

		[DllImport("PvAPI.dll", EntryPoint="PvAttrFloat32Set",ExactSpelling=false)]
		public static extern tErr AttrFloat32Set(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name,float Value);

		[DllImport("PvAPI.dll", EntryPoint="PvCommandRun",ExactSpelling=false)]
		public static extern tErr CommandRun(UInt32 Camera,[MarshalAs(UnmanagedType.LPStr)] String Name);

		[DllImport("PvAPI.dll", EntryPoint="PvUtilityColorInterpolate",ExactSpelling=false)]
		public static extern void ColorInterpolate([In,Out]IntPtr pFrame,IntPtr BufferRed,IntPtr BufferGreen,IntPtr BufferBlue,UInt32 PixelPadding,UInt32 LinePadding);

        [DllImport("PvAPI.dll", EntryPoint = "_Pv_Factory_Test_11", ExactSpelling = false)]
        public static extern UInt32 RegisterWrite(UInt32 Camera, UInt32 NumWrites, [MarshalAs(UnmanagedType.LPArray)] [In] UInt32[] AddressArray, [MarshalAs(UnmanagedType.LPArray)] [In] UInt32[] DataArray, ref UInt32 NumComplete);

        [DllImport("PvAPI.dll", EntryPoint = "_Pv_Factory_Test_10", ExactSpelling = false)]
        public static extern UInt32 RegisterRead(UInt32 Camera, UInt32 NumReads, [MarshalAs(UnmanagedType.LPArray)] [In] UInt32[] AddressArray, [MarshalAs(UnmanagedType.LPArray)] [Out] UInt32[] DataArray, ref UInt32 NumComplete);

    }
}
