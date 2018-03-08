/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Allied Vision Technologies is prohibited.
|
|==============================================================================
|
| This sample code opens the first camera found on the host computer and sets it
| for capturing. It then waits for the user to press a key before triggering the
| camera and saving the frame to a BMP file in the bin/release directory if the 
| capture was successful.
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
using System.Threading;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Drawing.Imaging;

using PvNET;

namespace SoftTrigger
{
	class SoftTrigger
	{
		// Place holder for all the data being used.
		private struct tCamera 
		{
			public UInt32	UID;
			public UInt32	Handle;
			public tFrame	Frame;
			public GCHandle GC;
			public byte[]	Buffer;
            public string   Serial; 
		};


		static void YUV2RGB(int y,int u,int v,ref int r,ref int g,ref int b)
		{
			// u and v are +-0.5.
			u -= 128;
			v -= 128;

			// Conversion (clamped to 0.255).
			r = Math.Min(Math.Max(0,(int)(y + 1.370705 * (float)v)),255);
			g = Math.Min(Math.Max(0,(int)(y - 0.698001 * (float)v - 0.337633 * (float)u)),255);
			b = Math.Min(Math.Max(0,(int)(y + 1.732446 * (float)u)),255);
		}


		// Frame callback dummy. 
		static void FrameDummyCB(IntPtr pFrame)
		{
		}


		// Wait for a camera to be detected.
		static void WaitForCamera()
		{
			Console.Write("Waiting for a camera ");
			while(Pv.CameraCount() == 0)
			{
				Console.Write(".");
				Thread.Sleep(250);
			}
			Console.WriteLine("");			
		}


        // Wait for the user to give a command.
        static bool WaitForUserToQuitOrSnap()
        {
            char c;

            do
            {
                c = (char)Console.Read();

            } 
            while (c != 'q' && c != 's');

            return c == 's';
        }


		// Get the UID and Serial Number of the first camera listed.
		static bool CameraGet(ref tCamera Camera)
		{
			UInt32 count,connected = 0;
			tCameraInfo[] list = new tCameraInfo[1];

			if((count = Pv.CameraList(list,1,ref connected)) != 0)
			{
				Camera.UID = list[0].UniqueId;
                Camera.Serial = list[0].SerialString;
                Console.WriteLine("Got camera: {0}",Camera.Serial);
				return true;
			}
			else
				return false;
		}


		// Open the camera.
		static bool CameraOpen(ref tCamera Camera)
		{
			return Pv.CameraOpen(Camera.UID,tAccessFlags.eAccessMaster,out Camera.Handle) == 0;
		}


		// Close the camera.
		static void CameraClose(ref tCamera Camera)
		{
			// Reset the trigger mode.
			Pv.AttrEnumSet(Camera.Handle,"FrameStartTriggerMode","Freerun");
			// Close the camera.
			Pv.CameraClose(Camera.Handle);
			// Delete the allocated buffer.
			Camera.GC.Free();
			// Reset the handle.
			Camera.Handle = 0;
		}


		// Setup the camera for capture.
		static bool CameraSetup(ref tCamera Camera)
		{
			UInt32 FrameSize = 0;

			// Get the byte size of the buffer.
			if(Pv.AttrUint32Get(Camera.Handle,"TotalBytesPerFrame",ref FrameSize) == 0)
			{
				Camera.Buffer = new byte[FrameSize];
				Camera.GC     = GCHandle.Alloc(Camera.Buffer,GCHandleType.Pinned);
                // Address of pinned buffer.
				Camera.Frame.ImageBuffer     = Camera.GC.AddrOfPinnedObject(); 
                // Buffer size.
				Camera.Frame.ImageBufferSize = FrameSize;						

				// Start the capture mode.
				if(Pv.CaptureStart(Camera.Handle) == 0)
				{
					// Set the camera in software acquisition mode.
					if(Pv.AttrEnumSet(Camera.Handle,"FrameStartTriggerMode","Software") == 0)
					{			
						// Set the acquisition mode into continuous.
						if(Pv.CommandRun(Camera.Handle,"AcquisitionStart") != 0)
						{
							// if that fails, reset the camera to non capture mode.
							Pv.CaptureEnd(Camera.Handle);
							return false;
						}
						else
							return true;
					}
					else
						return false;
				}
				else
					return true;
			}
			else
				return false;
		}


		// Snap a frame from the camera.
		static bool CameraSnap(ref tCamera Camera)
		{
			bool result;
			GCHandle pFrame = GCHandle.Alloc(Camera.Frame,GCHandleType.Pinned);
			tFrameCallback FrameCB = new tFrameCallback(FrameDummyCB);
			
			// Queue the frame.
			if(Pv.CaptureQueueFrame(Camera.Handle,pFrame.AddrOfPinnedObject(),FrameCB) == 0)
			{
                Console.WriteLine("Triggering the camera ...");

				// Trigger the capture.
				if(Pv.CommandRun(Camera.Handle,"FrameStartTriggerSoftware") == 0)
				{
                    Console.WriteLine("Waiting for the frame to be done ...");

					// Wait for the frame to be returned.
					while(Pv.CaptureWaitForFrameDone(Camera.Handle,pFrame.AddrOfPinnedObject(),10) == tErr.eErrTimeout)
					
                        Console.WriteLine("Still waiting ...");

						// Copy the frame structure back in the frame we have.
						Camera.Frame = (tFrame)Marshal.PtrToStructure(pFrame.AddrOfPinnedObject(),typeof(tFrame));
	
						// Check the status flag.
						if(!(result = (Camera.Frame.Status == tErr.eErrSuccess)))
							Console.WriteLine("Frame captured un-succesfully {0}",Camera.Frame.Status);
					
				}
				else
				{
					Pv.CaptureQueueClear(Camera.Handle);
					result = false;
				}
			}
			else
				result = false;

			pFrame.Free();

			return result;
		}


		// Convert the raw data in the frame's buffer into the bitmap's data, this method doesn't support 
		// the following Pixel formats: eFmtRgb48, eFmtYuv411 and eFmtYuv444.
		static unsafe bool Frame2Data(ref tCamera Camera,ref BitmapData Data)
		{
			switch(Camera.Frame.Format)
			{
				case tImageFormat.eFmtMono8:
				{
					UInt32 lOffset = 0;
					UInt32 lPos = 0;
					byte* lDst = (byte *)Data.Scan0;
					
					while(lOffset<Camera.Frame.ImageBufferSize)
					{
						lDst[lPos]   = Camera.Buffer[lOffset];
						lDst[lPos+1] = Camera.Buffer[lOffset];
						lDst[lPos+2] = Camera.Buffer[lOffset];

						lOffset++;
						lPos += 3;

						// Take care of the padding in the destination bitmap.
						if((lOffset % Camera.Frame.Width) == 0)
							lPos += (UInt32)Data.Stride - (Camera.Frame.Width * 3);
					}

					return true;
				}
				case tImageFormat.eFmtMono16:
				{
					UInt32 lOffset = 0;
					UInt32 lPos = 0;
					byte* lDst = (byte *)Data.Scan0;
					byte bitshift = (byte)((int)Camera.Frame.BitDepth - 8);
					UInt16* lSrc = (UInt16*)Camera.Frame.ImageBuffer;
					
					while(lOffset<Camera.Frame.Width * Camera.Frame.Height)
					{
						lDst[lPos]   = (byte)(lSrc[lOffset] >> bitshift);
						lDst[lPos+1] = lDst[lPos];
						lDst[lPos+2] = lDst[lPos];

						lOffset++;
						lPos += 3;

						// Take care of the padding in the destination bitmap.
						if((lOffset % Camera.Frame.Width) == 0)
							lPos += (UInt32)Data.Stride - (Camera.Frame.Width * 3);
					}

					return true;
				}
				case tImageFormat.eFmtBayer8:
				{
					UInt32 WidthSize = Camera.Frame.Width * 3;
					GCHandle pFrame = GCHandle.Alloc(Camera.Frame,GCHandleType.Pinned);
					UInt32 remainder = (((WidthSize + 3U) & ~3U) - WidthSize);
								
					// Interpolate the colors.
					IntPtr pRed   = (IntPtr)((byte *)Data.Scan0 + 2);
					IntPtr pGreen = (IntPtr)((byte *)Data.Scan0 + 1);
					IntPtr pBlue  = (IntPtr)((byte *)Data.Scan0);
					Pv.ColorInterpolate(pFrame.AddrOfPinnedObject(),pRed,pGreen,pBlue,2,remainder);

					pFrame.Free();

					return true;
				}
				case tImageFormat.eFmtBayer16:
				{
					UInt32 WidthSize = Camera.Frame.Width * 3;
					UInt32 lOffset   = 0;
					byte bitshift    = (byte)((int)Camera.Frame.BitDepth - 8);
					UInt16* lSrc     = (UInt16*)Camera.Frame.ImageBuffer;
					byte*   lDst     = (byte*)Camera.Frame.ImageBuffer;
					UInt32 remainder = (((WidthSize + 3U) & ~3U) - WidthSize);
					GCHandle pFrame;

					Camera.Frame.Format = tImageFormat.eFmtBayer8;

					pFrame = GCHandle.Alloc(Camera.Frame,GCHandleType.Pinned);
					
					// Shift the pixel.
					while(lOffset<Camera.Frame.Width * Camera.Frame.Height)
						lDst[lOffset] = (byte)(lSrc[lOffset++] >> bitshift);

					// Interpolate the colors.
					IntPtr pRed   = (IntPtr)((byte *)Data.Scan0 + 2);
					IntPtr pGreen = (IntPtr)((byte *)Data.Scan0 + 1);
					IntPtr pBlue  = (IntPtr)((byte *)Data.Scan0);
					Pv.ColorInterpolate(pFrame.AddrOfPinnedObject(),pRed,pGreen,pBlue,2,remainder);

					pFrame.Free();

					return true;
				}
				case tImageFormat.eFmtRgb24:
				{
					UInt32 lOffset = 0;
					UInt32 lPos = 0;
					byte* lDst = (byte *)Data.Scan0;
					
					while(lOffset<Camera.Frame.ImageBufferSize)
					{
						// Copy the data.
						lDst[lPos]   = Camera.Buffer[lOffset+2];
						lDst[lPos+1] = Camera.Buffer[lOffset+1];
						lDst[lPos+2] = Camera.Buffer[lOffset];

						lOffset+= 3;
						lPos   += 3;

						// Take care of the padding in the destination bitmap.
						if((lOffset % (Camera.Frame.Width * 3)) == 0)
							lPos += (UInt32)Data.Stride - (Camera.Frame.Width * 3);
					}
					
					return true;
				}
				case tImageFormat.eFmtRgb48:
				{
					UInt32 lOffset = 0;
					UInt32 lPos    = 0;
					UInt32 lLength = Camera.Frame.ImageBufferSize / sizeof(UInt16);
					UInt16* lSrc   = (UInt16*)Camera.Frame.ImageBuffer;
					byte* lDst     = (byte *)Data.Scan0;
					byte bitshift  = (byte)((int)Camera.Frame.BitDepth - 8);
					
					while(lOffset < lLength)
					{
						// Copy the data.
						lDst[lPos]   = (byte)(lSrc[lOffset+2] >> bitshift);
						lDst[lPos+1] = (byte)(lSrc[lOffset+1] >> bitshift);
						lDst[lPos+2] = (byte)(lSrc[lOffset] >> bitshift);

						lOffset+= 3;
						lPos   += 3;

						// Take care of the padding in the destination bitmap.
						if((lOffset % (Camera.Frame.Width * 3)) == 0)
							lPos += (UInt32)Data.Stride - (Camera.Frame.Width * 3);
					}

					return true;
				}
				case tImageFormat.eFmtYuv411:
				{
					UInt32 lOffset = 0;
					UInt32 lPos = 0;
					byte* lDst = (byte *)Data.Scan0;
					int y1,y2,y3,y4,u,v;
					int r,g,b;

					r = g = b = 0;

					while(lOffset<Camera.Frame.ImageBufferSize)
					{
						u  = Camera.Buffer[lOffset++];
						y1 = Camera.Buffer[lOffset++];
						y2 = Camera.Buffer[lOffset++];
						v  = Camera.Buffer[lOffset++];
						y3 = Camera.Buffer[lOffset++];
						y4 = Camera.Buffer[lOffset++]; 					
		
						YUV2RGB(y1,u,v,ref r,ref g,ref b);
						lDst[lPos++] = (byte)b;
						lDst[lPos++] = (byte)g;
						lDst[lPos++] = (byte)r;
						YUV2RGB(y2,u,v,ref r,ref g,ref b);
						lDst[lPos++] = (byte)b;
						lDst[lPos++] = (byte)g;
						lDst[lPos++] = (byte)r;
						YUV2RGB(y3,u,v,ref r,ref g,ref b);
						lDst[lPos++] = (byte)b;
						lDst[lPos++] = (byte)g;
						lDst[lPos++] = (byte)r;
						YUV2RGB(y4,u,v,ref r,ref g,ref b);
						lDst[lPos++] = (byte)b;
						lDst[lPos++] = (byte)g;
						lDst[lPos++] = (byte)r;						
					}

					return true;
				}
				case tImageFormat.eFmtYuv422:
				{
					UInt32 lOffset = 0;
					UInt32 lPos = 0;
					byte* lDst = (byte *)Data.Scan0;
					int y1,y2,u,v;
					int r,g,b;

					r = g = b = 0;

					while(lOffset<Camera.Frame.ImageBufferSize)
					{
						u  = Camera.Buffer[lOffset++];
						y1 = Camera.Buffer[lOffset++];
						v  = Camera.Buffer[lOffset++];
						y2 = Camera.Buffer[lOffset++];
						
						YUV2RGB(y1,u,v,ref r,ref g,ref b);
						lDst[lPos++] = (byte)b;
						lDst[lPos++] = (byte)g;
						lDst[lPos++] = (byte)r;
						YUV2RGB(y2,u,v,ref r,ref g,ref b);
						lDst[lPos++] = (byte)b;
						lDst[lPos++] = (byte)g;
						lDst[lPos++] = (byte)r;
					}

					return true;
				}
				case tImageFormat.eFmtYuv444:
				{
					UInt32 lOffset = 0;
					UInt32 lPos = 0;
					byte* lDst = (byte *)Data.Scan0;
					int y1,y2,u,v;
					int r,g,b;

					r = g = b = 0;

					while(lOffset<Camera.Frame.ImageBufferSize)
					{
						u  = Camera.Buffer[lOffset++];
						y1 = Camera.Buffer[lOffset++];
						v  = Camera.Buffer[lOffset++];
						lOffset++;
						y2 = Camera.Buffer[lOffset++];
						lOffset++;
						
						YUV2RGB(y1,u,v,ref r,ref g,ref b);
						lDst[lPos++] = (byte)b;
						lDst[lPos++] = (byte)g;
						lDst[lPos++] = (byte)r;
						YUV2RGB(y2,u,v,ref r,ref g,ref b);
						lDst[lPos++] = (byte)b;
						lDst[lPos++] = (byte)g;
						lDst[lPos++] = (byte)r;
					}

					return true;
				}
				default:
					return false;
			}
		}


		// Save a frame to a file.
		static bool FrameSave(ref tCamera Camera,string file)
		{
			Bitmap lBitmap = new Bitmap((int)Camera.Frame.Width,(int)Camera.Frame.Height,PixelFormat.Format24bppRgb);
			Rectangle lRect = new Rectangle(new Point(0,0),new Size((int)Camera.Frame.Width,(int)Camera.Frame.Height));
			BitmapData lData = lBitmap.LockBits(lRect,ImageLockMode.ReadWrite,PixelFormat.Format24bppRgb);
			
			if(Frame2Data(ref Camera,ref lData))
			{
				lBitmap.UnlockBits(lData);
				lBitmap.Save(file);
				return true;
			}
			else
			{
				lBitmap.UnlockBits(lData);
				return false;
			}
		}


		static void Main(string[] args)
		{
			tErr err = 0;

			// Initialize the API.
			if((err = Pv.Initialize()) == 0)
			{
				tCamera Camera = new tCamera();

                // Wait for a camera to be detected.
				WaitForCamera();

				if(CameraGet(ref Camera))
				{
                    // Open the first camera detected.
					if(CameraOpen(ref Camera))
					{
                        // Set the camera up for triggering.
						if(CameraSetup(ref Camera))
						{
							UInt32 Count = 0;
							String filename;

							Console.WriteLine("The camera is ready now. Press Q to quit or S to take a picture.");

                            while (WaitForUserToQuitOrSnap())
                            {

                                if (CameraSnap(ref Camera))
                                {
                                    filename = "Snap";
                                    filename += Count++;
                                    filename += ".bmp";

                                    if (FrameSave(ref Camera, filename) == false)
                                        Console.WriteLine("Failed to save the image.");
                                    else
                                        Console.WriteLine("Frame saved.");
                                        Console.WriteLine("The camera is waiting for a trigger. Press Q to quit or S to take a picture.");
                                }
                                else
                                    Console.WriteLine("Failed to snap an image.");
                            }
							
						}
						else
							Console.WriteLine("Failed to setup the camera.");

						CameraClose(ref Camera);
					}
					else
						Console.WriteLine("Camera {0} failed to be open.",Camera.UID);
				}
				else
					Console.WriteLine("Failed to get a camera.");

				Pv.UnInitialize();
			}
			else
			{
				Console.Write("Failed to initialize the API : ");	
				Console.WriteLine(err);
			}

			Thread.Sleep(800);
		}
	}
}
