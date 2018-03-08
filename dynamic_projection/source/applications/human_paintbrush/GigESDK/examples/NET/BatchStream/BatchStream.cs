/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Allied Vision Technologies is prohibited.
|
|==============================================================================
|
| This sample code opens the first camera found on the host computer and captures
| and holds in memory 600 frames from it, using the camera in FixedRate mode. This
| sample uses the camera the way it was last setup.
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
using PvNET;

namespace BatchStream
{
	class BatchStream
	{
		// Place holder for all the data we're using.
		private struct tCamera 
		{
			public UInt32	UID;
			public UInt32	Handle;
			public tFrame	Frame;
			public GCHandle GC;
			public byte[]	Buffer;
            public string   Serial;   
		};


		// Frame callback dummy. 
		static void FrameDummyCB(IntPtr pFrame)
		{ 
		}


		// Wait for a camera to be detected.
		static void WaitForCamera()
		{
			Console.Write("Waiting for a camera");
			while(Pv.CameraCount() == 0)
			{
				System.Console.Write(".");
				Thread.Sleep(250);
			}
			Console.WriteLine();			
		}


		// Get the UID and Serial number of the first camera listed.
		static bool CameraGet(ref tCamera Camera)
		{
			UInt32 count,connected = 0;
			tCameraInfo[] list = new tCameraInfo[1];

			if((count = Pv.CameraList(list,1,ref connected)) != 0)
			{
				Camera.UID = list[0].UniqueId;
                Camera.Serial = list[0].SerialString;
                Console.WriteLine("Grabbing camera " + Camera.Serial);
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
            // Dequeue all the frame still queued (this will block until they all have been dequeued).
            Pv.CaptureQueueClear(Camera.Handle);
			// Reset the trigger mode.
			Pv.AttrEnumSet(Camera.Handle,"FrameStartTriggerMode","Freerun");
			// Close the camera.
			Pv.CameraClose(Camera.Handle);
            Console.WriteLine("camera is closed, deleting the frames now");
			// Delete the allocated buffer.
			Camera.GC.Free();
			// Reset the handle.
			Camera.Handle = 0;
		}

       
		// Setup the camera for capture.
		static bool CameraSetup(ref tCamera Camera)
		{
			UInt32 FrameSize = 0;
            UInt32 FrameCount = 600;

			// Get the byte size of the buffer.
			if(Pv.AttrUint32Get(Camera.Handle,"TotalBytesPerFrame",ref FrameSize) == 0)
			{
				Camera.Buffer = new byte[FrameSize];
				Camera.GC     = GCHandle.Alloc(Camera.Buffer,GCHandleType.Pinned);
                // Address of the pinned buffer
				Camera.Frame.ImageBuffer     = Camera.GC.AddrOfPinnedObject(); 
				// Buffer size.
                Camera.Frame.ImageBufferSize = FrameSize;						

                Console.WriteLine("We have allocated {0} frames of {1} bytes each.", FrameCount,FrameSize);

				// Start the capture mode.
				if(Pv.CaptureStart(Camera.Handle) == 0)
				{
					// Set the camera in software acquisition mode.
					if(Pv.AttrEnumSet(Camera.Handle,"FrameStartTriggerMode","Software") == 0)
					{			
						// Set the acquisition mode into continuous.
						if(Pv.CommandRun(Camera.Handle,"AcquisitionStart") != 0)
						{
							// If that fails, reset the camera to non capture mode.
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
            GCHandle pFrame = GCHandle.Alloc(Camera.Frame, GCHandleType.Pinned);
            tFrameCallback FrameCB = new tFrameCallback(FrameDummyCB);

            // Adjust packet for optimal preformance.
            Pv.CaptureAdjustPacketSize(Camera.Handle, 8228);

            // Queue the frame.
            if (Pv.CaptureQueueFrame(Camera.Handle, pFrame.AddrOfPinnedObject(), FrameCB) == 0)
            {
                // Trigger the capture.
                if (Pv.CommandRun(Camera.Handle, "FrameStartTriggerSoftware") == 0)
                {
                    // Then wait for the frame to be returned.
                    if (Pv.CaptureWaitForFrameDone(Camera.Handle, pFrame.AddrOfPinnedObject(), 0) == 0)
                    {

                        // Check the status flag.
                        if (!(result = (Camera.Frame.Status == tErr.eErrSuccess)))
                            Console.WriteLine("Frame captured un-succesfully {0}.", Camera.Frame.Status);
                    
                    }
                    else
                    {
                        Pv.CaptureQueueClear(Camera.Handle);
                        result = false;
                        
                    }
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


        // Stop the streaming.
        static void CameraStop(ref tCamera Camera)
        {
            Console.WriteLine("Streaming is stopping.");
            Pv.CommandRun(Camera.Handle, "AcquisitionStop");
            Pv.CaptureEnd(Camera.Handle);
        }


		static void Main(string[] args)
		{
			tErr err = 0;

			// Initialize the API.
			if((err = Pv.Initialize()) == 0)
			{
				tCamera Camera = new tCamera();
                UInt32 FrameCount = 600;

                // Wait for a camera to be detected.
				WaitForCamera();

				if(CameraGet(ref Camera))
				{
                    // Open the first camera.
					if(CameraOpen(ref Camera))
					{
						// Set the camera up for streaming.
						if(CameraSetup(ref Camera))
						{
                            UInt32 Count = 0;


                            while (Count <= FrameCount)

                            // Start streaming.
                            if (CameraSnap(ref Camera))
                            {
                                
                                Console.Write("[{0:000}]",Count);
                                UInt32 status = (UInt32)(Camera.Frame.Status);
                                Console.WriteLine(" Status = {0:00} ",status);
                                Count++;
                            }
                            CameraStop(ref Camera);

							
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
