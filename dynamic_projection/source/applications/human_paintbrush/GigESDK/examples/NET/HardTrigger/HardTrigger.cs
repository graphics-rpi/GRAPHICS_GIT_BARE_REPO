/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Allied Vision Technologies is prohibited.
|
|==============================================================================
|
| This sample code, opens the first camera found on the host computer and sets it
| for capturing. It then waits for a hardware trigger and saves the frame to a
| BMP file if the capture was successful.
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

namespace HardTrigger
{
	class HardTrigger
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

        static bool close=false;
        static tCamera GCamera = new tCamera();

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


        // Spawn a new thread to allow user to quit application.
        static void SpawnAbortThread()
        {
            Thread Thread = new Thread(new ThreadStart(Abort));
            // Start the thread.
            Thread.Start();
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


		// Get the UID and Serial Number of the first camera listed.
		static bool CameraGet()
		{
			UInt32 count,connected = 0;
			tCameraInfo[] list = new tCameraInfo[1];
            count = Pv.CameraList(list, 1, ref connected);
			if(count == 1)
			{
				GCamera.UID = list[0].UniqueId;
                GCamera.Serial = list[0].SerialString;
                Console.WriteLine("Got camera: {0}",GCamera.Serial);
				return true;
			}
			else
				return false;
		}


		// Open the camera.
		static bool CameraOpen()
		{
			return Pv.CameraOpen(GCamera.UID,tAccessFlags.eAccessMaster,out GCamera.Handle) == 0;
		}


		// Close the camera.
		static void CameraClose()
		{
			// Reset the trigger mode.
			Pv.AttrEnumSet(GCamera.Handle,"FrameStartTriggerMode","Freerun");
			// Close the camera.
			Pv.CameraClose(GCamera.Handle);
			// Reset the handle.
			GCamera.Handle = 0;

            Console.WriteLine("The Camera is now closed.");
		}


		// Setup the camera for capture.
		static bool CameraSetup()
		{
			UInt32 FrameSize = 0;

			// Get the byte size of the buffer.
			if(Pv.AttrUint32Get(GCamera.Handle,"TotalBytesPerFrame",ref FrameSize) == 0)
			{
				GCamera.Buffer = new byte[FrameSize];
				GCamera.GC     = GCHandle.Alloc(GCamera.Buffer,GCHandleType.Pinned);
                // Address of pinned buffer.
				GCamera.Frame.ImageBuffer     = GCamera.GC.AddrOfPinnedObject(); 
                // Buffer size.
				GCamera.Frame.ImageBufferSize = FrameSize;						

				// Start the capture mode.
				if(Pv.CaptureStart(GCamera.Handle) == 0)
				{
                    // Set the acquisition mode into continuous and hardware trigger mode, using SyncIn2 (non-isolated).
                    if (Pv.AttrEnumSet(GCamera.Handle, "FrameStartTriggerMode", "SyncIn2") == 0)

                     // Set the acquisition mode into continuous and hardware trigger mode, using SyncIn1 (isolated).
                     // if (Pv.AttrEnumSet(GCamera.Handle, "FrameStartTriggerMode", "SyncIn1") == 0)
					
                    {			
						// Set the acquisition mode into continuous.
						if(Pv.CommandRun(GCamera.Handle,"AcquisitionStart") != 0)
						{
							// if that fails, reset the camera to non capture mode.
							Pv.CaptureEnd(GCamera.Handle);
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
		static bool CameraSnap()
		{
			bool result;
			GCHandle pFrame = GCHandle.Alloc(GCamera.Frame,GCHandleType.Pinned);
			tFrameCallback FrameCB = new tFrameCallback(FrameDummyCB);
			
			// Queue the frame.
			if(Pv.CaptureQueueFrame(GCamera.Handle,pFrame.AddrOfPinnedObject(),FrameCB) == 0)
			{

				// Trigger the capture.
				
                    Console.WriteLine("Waiting for a frame ...");

					// Wait for the frame to be returned.
                    while (Pv.CaptureWaitForFrameDone(GCamera.Handle, pFrame.AddrOfPinnedObject(), 1500) == tErr.eErrTimeout)
                    {
                        if (close == false)
                        {
                            Console.WriteLine("Still waiting ...");
                        }
                        else break;
                    }
						// Copy the frame structure back in the frame we have.
						GCamera.Frame = (tFrame)Marshal.PtrToStructure(pFrame.AddrOfPinnedObject(),typeof(tFrame));
	
						// Check the status flag.
						if(!(result = (GCamera.Frame.Status == tErr.eErrSuccess)))
							Console.WriteLine("Frame captured un-succesfully {0}",GCamera.Frame.Status);	
				
			}
			else
				result = false;

			pFrame.Free();

			return result;
		}


		// Convert the raw data in the frame's buffer into the bitmap's data. This method doesn't support 
		// the following Pixel formats: eFmtRgb48, eFmtYuv411 and eFmtYuv444.
		static unsafe bool Frame2Data(ref BitmapData Data)
		{
			switch(GCamera.Frame.Format)
			{
				case tImageFormat.eFmtMono8:
				{
					UInt32 lOffset = 0;
					UInt32 lPos = 0;
					byte* lDst = (byte *)Data.Scan0;
					
					while(lOffset<GCamera.Frame.ImageBufferSize)
					{
						lDst[lPos]   = GCamera.Buffer[lOffset];
						lDst[lPos+1] = GCamera.Buffer[lOffset];
						lDst[lPos+2] = GCamera.Buffer[lOffset];

						lOffset++;
						lPos += 3;

						// Take care of the padding in the destination bitmap.
						if((lOffset % GCamera.Frame.Width) == 0)
							lPos += (UInt32)Data.Stride - (GCamera.Frame.Width * 3);
					}

					return true;
				}
				case tImageFormat.eFmtMono16:
				{
					UInt32 lOffset = 0;
					UInt32 lPos = 0;
					byte* lDst = (byte *)Data.Scan0;
					byte bitshift = (byte)((int)GCamera.Frame.BitDepth - 8);
					UInt16* lSrc = (UInt16*)GCamera.Frame.ImageBuffer;
					
					while(lOffset<GCamera.Frame.Width * GCamera.Frame.Height)
					{
						lDst[lPos]   = (byte)(lSrc[lOffset] >> bitshift);
						lDst[lPos+1] = lDst[lPos];
						lDst[lPos+2] = lDst[lPos];

						lOffset++;
						lPos += 3;

						// Take care of the padding in the destination bitmap.
						if((lOffset % GCamera.Frame.Width) == 0)
							lPos += (UInt32)Data.Stride - (GCamera.Frame.Width * 3);
					}

					return true;
				}
				case tImageFormat.eFmtBayer8:
				{
					UInt32 WidthSize = GCamera.Frame.Width * 3;
					GCHandle pFrame = GCHandle.Alloc(GCamera.Frame,GCHandleType.Pinned);
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
					UInt32 WidthSize = GCamera.Frame.Width * 3;
					UInt32 lOffset   = 0;
					byte bitshift    = (byte)((int)GCamera.Frame.BitDepth - 8);
					UInt16* lSrc     = (UInt16*)GCamera.Frame.ImageBuffer;
					byte*   lDst     = (byte*)GCamera.Frame.ImageBuffer;
					UInt32 remainder = (((WidthSize + 3U) & ~3U) - WidthSize);
					GCHandle pFrame;

					GCamera.Frame.Format = tImageFormat.eFmtBayer8;

					pFrame = GCHandle.Alloc(GCamera.Frame,GCHandleType.Pinned);
					
					// Shift the pixel.
					while(lOffset<GCamera.Frame.Width * GCamera.Frame.Height)
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
					
					while(lOffset<GCamera.Frame.ImageBufferSize)
					{
						// Copy the data.
						lDst[lPos]   = GCamera.Buffer[lOffset+2];
						lDst[lPos+1] = GCamera.Buffer[lOffset+1];
						lDst[lPos+2] = GCamera.Buffer[lOffset];

						lOffset+= 3;
						lPos   += 3;

						// Take care of the padding in the destination bitmap.
						if((lOffset % (GCamera.Frame.Width * 3)) == 0)
							lPos += (UInt32)Data.Stride - (GCamera.Frame.Width * 3);
					}
					
					return true;
				}
				case tImageFormat.eFmtRgb48:
				{
					UInt32 lOffset = 0;
					UInt32 lPos    = 0;
					UInt32 lLength = GCamera.Frame.ImageBufferSize / sizeof(UInt16);
					UInt16* lSrc   = (UInt16*)GCamera.Frame.ImageBuffer;
					byte* lDst     = (byte *)Data.Scan0;
					byte bitshift  = (byte)((int)GCamera.Frame.BitDepth - 8);
					
					while(lOffset < lLength)
					{
						// Copy the data.
						lDst[lPos]   = (byte)(lSrc[lOffset+2] >> bitshift);
						lDst[lPos+1] = (byte)(lSrc[lOffset+1] >> bitshift);
						lDst[lPos+2] = (byte)(lSrc[lOffset] >> bitshift);

						lOffset+= 3;
						lPos   += 3;

						// Take care of the padding in the destination bitmap.
						if((lOffset % (GCamera.Frame.Width * 3)) == 0)
							lPos += (UInt32)Data.Stride - (GCamera.Frame.Width * 3);
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

					while(lOffset<GCamera.Frame.ImageBufferSize)
					{
						u  = GCamera.Buffer[lOffset++];
						y1 = GCamera.Buffer[lOffset++];
						y2 = GCamera.Buffer[lOffset++];
						v  = GCamera.Buffer[lOffset++];
						y3 = GCamera.Buffer[lOffset++];
						y4 = GCamera.Buffer[lOffset++]; 					
		
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

					while(lOffset<GCamera.Frame.ImageBufferSize)
					{
						u  = GCamera.Buffer[lOffset++];
						y1 = GCamera.Buffer[lOffset++];
						v  = GCamera.Buffer[lOffset++];
						y2 = GCamera.Buffer[lOffset++];
						
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

					while(lOffset<GCamera.Frame.ImageBufferSize)
					{
						u  = GCamera.Buffer[lOffset++];
						y1 = GCamera.Buffer[lOffset++];
						v  = GCamera.Buffer[lOffset++];
						lOffset++;
						y2 = GCamera.Buffer[lOffset++];
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
		static bool FrameSave(string file)
		{
			Bitmap lBitmap = new Bitmap((int)GCamera.Frame.Width,(int)GCamera.Frame.Height,PixelFormat.Format24bppRgb);
			Rectangle lRect = new Rectangle(new Point(0,0),new Size((int)GCamera.Frame.Width,(int)GCamera.Frame.Height));
			BitmapData lData = lBitmap.LockBits(lRect,ImageLockMode.ReadWrite,PixelFormat.Format24bppRgb);
			
			if(Frame2Data(ref lData))
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


        // Stop the streaming.
        static void CameraStop()
        {
            Pv.CommandRun(GCamera.Handle, "AcquisitionStop");
            Pv.CaptureEnd(GCamera.Handle);
        }


        // Handler for the Abort function.
        static void myHandler(object sender, ConsoleCancelEventArgs args)
        {
        }


        // Allows the user to stop the streaming. 
        static void Abort()
        {
            ConsoleKeyInfo cki;

            // Establish an event handler to process key press events.
            Console.CancelKeyPress += new ConsoleCancelEventHandler(myHandler);
            // Prompt the user.

            while (true)
            {

                // Start a console read operation. Do not display the input.
                cki = Console.ReadKey(true);
                // Exit if the user pressed the 'X' key.
                if (cki.Key == ConsoleKey.X)
                {
                    // Announce the name of the key that was pressed.
                    Console.Write("Key pressed: {0}\n", cki.Key);
                    close = true;
                    break;
                }
            }
        }


		static void Main(string[] args)
		{
			tErr err = 0;

			// Initialize the API.
			if((err = Pv.Initialize()) == 0)
			{

                // Wait for a camera to be detected.
				WaitForCamera();

                // Obtains a camera from the list.
				if(CameraGet())
				{
                    // Open the first camera detected.
					if(CameraOpen())
					{
                        // Set the camera up for triggering.
						if(CameraSetup())
						{

                            // Allows the user to terminate program.
                            SpawnAbortThread();

                            UInt32 Count = 0;
                            String filename;

                            Console.WriteLine("The camera is ready now, press 'X' to terminate program.");
                            while (close == false)
                            {

                                // Snaps a frame.
                                if (CameraSnap())
                                {
                                    filename = "Snap";
                                    filename += Count++;
                                    filename += ".bmp";

                                    if (close == false)
                                    {
                                        // Saves the snapped frame.
                                        if (FrameSave(filename) == false)
                                            Console.WriteLine("Failed to save the image.");

                                        else
                                            Console.WriteLine("Frame saved under filename: {0}", filename);
                                    }
                                }
                                else
                                    Console.WriteLine("Failed to snap an image.");
                            }

                            // Stop the streaming.
                            CameraStop();  
                            
						}
						else
							Console.WriteLine("Failed to setup the camera.");

                        // Close the camera.
						CameraClose();
					}
					else
						Console.WriteLine("Camera {0} failed to be open.",GCamera.UID);
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
