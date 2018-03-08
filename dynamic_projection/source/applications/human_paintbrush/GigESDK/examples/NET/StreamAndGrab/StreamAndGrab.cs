/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Allied Vision Technologies is prohibited.
|
|==============================================================================
|
| This sample code opens the first camera found on the host computer and 
| alternates between streaming and grabbing of a single frame.
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

namespace StreamAndGrab
{
    class StreamAndGrab
    {

        // Place holder for all the data being used.
        private struct tCamera
        {
            public UInt32 UID;
            public UInt32 Handle;
            public tFrame Frame;
            public GCHandle GC;
            public byte[] Buffer;
            public string Serial;

        }


        // Callback called when the camera is plugged/unplugged.
        static void LinkCB(IntPtr Context, tInterface Interface, tLinkEvent Event, UInt32 UniqueId)
        {
            switch (Event)
            {
                case tLinkEvent.eLinkAdd:
                    {
                        Console.WriteLine("Camera {0} plugged in.", UniqueId);
                        break;
                    }
                case tLinkEvent.eLinkRemove:
                    {
                        Console.WriteLine("Camera {0} unplugged.", UniqueId);
                        break;
                    }
            }
        }


        // frame callback dummy for the stream process.
        static void FrameDummyCBStream(IntPtr pFrame)
        {
            tCamera Camera = new tCamera();

            // copy the frame structure back in the frame we have
            Camera.Frame = (tFrame)Marshal.PtrToStructure(pFrame, typeof(tFrame));

            UInt32 status = (UInt32)(Camera.Frame.Status);
            Console.WriteLine("Frame returned with status = {0:0} ", status);    
        }


        static void FrameDummyCBSnap(IntPtr pFrame)
        {  
        }


        // Wait for a camera to be detected.
        static void WaitForCamera()
        {
            Console.WriteLine("Waiting for a camera ");
            while (Pv.CameraCount() == 0)
            {
                Console.Write(".");
                Thread.Sleep(250);
            }
            Console.WriteLine("");
        }


        // get the UID and Serial Number of the first camera listed.
        static bool CameraGrab(ref tCamera Camera)
        {
            UInt32 count, connected = 0;
            tCameraInfo[] list = new tCameraInfo[1];

            if ((count = Pv.CameraList(list, 1, ref connected)) != 0)
            {
                Camera.UID = list[0].UniqueId;
                Camera.Serial = list[0].SerialString;
                Console.WriteLine("Grabbing camera: {0}", Camera.Serial);
                return true;

            }
            else
                return false;
        }


        // Close the camera.
        static void CameraClose(ref tCamera Camera)
        {
            // Reset the trigger mode.
            Pv.AttrEnumSet(Camera.Handle, "FrameStartTriggerMode", "Freerun");
            // Clear the queue.
            Console.WriteLine("Clearing the queue...");
            Pv.CaptureQueueClear(Camera.Handle);
            // Close the camera.
            Pv.CameraClose(Camera.Handle);
            Console.WriteLine("Closing the camera.");
            // Delete the allocated buffer.
            Camera.GC.Free();
            // Reset the handle.
            Camera.Handle = 0;
        }


        // Setup the camera for capture.
        static bool CameraSetup(ref tCamera Camera)
        {
            UInt32 FrameSize = 0;

            if (Pv.CameraOpen(Camera.UID, tAccessFlags.eAccessMaster, out Camera.Handle) == 0)
            {
                // Get the bytes size of the buffer.
                if (Pv.AttrUint32Get(Camera.Handle, "TotalBytesPerFrame", ref FrameSize) == 0)
                {
                    Camera.Buffer = new byte[FrameSize];
                    Camera.GC = GCHandle.Alloc(Camera.Buffer, GCHandleType.Pinned);
                    // Address of the pinned buffer.
                    Camera.Frame.ImageBuffer = Camera.GC.AddrOfPinnedObject();
                    // Buffer size.
                    Camera.Frame.ImageBufferSize = FrameSize;					
                    return true;
                }
                else
                    return false;
            }
            else
            {
                Pv.CameraClose(Camera.Handle);
                Camera.Handle = 0;
                return false;
            }
        }


        // Stop the streaming.
        static void CameraStop(ref tCamera Camera)
        {
            Pv.CommandRun(Camera.Handle, "AcquisitionStop");
            Pv.CaptureEnd(Camera.Handle);
            Pv.CaptureQueueClear(Camera.Handle);
        }


        // Unsetup the camera.
        static void CameraUnsetup(ref tCamera Camera)
        {
            Console.WriteLine("Closing the camera now...");
            // Close the camera.
            Pv.CameraClose(Camera.Handle);
            Console.WriteLine("Closed.");
            // Reset the handle.
            Camera.Handle = 0;
        }


        // Start streaming frames.
        static bool DoStream(ref tCamera Camera)
        {
            tErr Err;
            UInt32 FrameCount = 0;
            GCHandle pFrame = GCHandle.Alloc(Camera.Frame, GCHandleType.Pinned);
            tFrameCallback FrameCB = new tFrameCallback(FrameDummyCBStream);
            uint timeout = 1000;

            // Adjust packet for optimal preformance.
            Pv.CaptureAdjustPacketSize(Camera.Handle, 8228);

            Console.WriteLine("Streaming has started...");

                // Set the camera in acquisition mode.
                if (((Err = Pv.CaptureStart(Camera.Handle)) == 0))
                {
                    if ((Pv.CommandRun(Camera.Handle, "AcquisitionStart")) != 0)
                    {
                        // If that fails, reset the camera to non capture mode.
                        Pv.CaptureEnd(Camera.Handle);
                        Console.WriteLine("PvCommandRun() failed!");
                        return false;
                    }
                    else
                    {
                        // Stream for a total of 30 frames.
                        while (FrameCount < 100)
                        {
                            if (Pv.CaptureQueueFrame(Camera.Handle, pFrame.AddrOfPinnedObject(), FrameCB) == 0)
                            {
                                Pv.CaptureWaitForFrameDone(Camera.Handle, pFrame.AddrOfPinnedObject(), timeout);
                                FrameCount++;
                            }
                        }
                        // After the 30 frames, stop streaming.
                        CameraStop(ref Camera);
                        Console.WriteLine("Streaming stopped.");
                        return true;
                    }
                }
                else
                {
                    Console.WriteLine("PvCaptureStart() failed with error code {0}." ,Err);
                    return false;
                }
            }
           
        // Snap a frame.
        static bool DoSnap(ref tCamera Camera)
        {
            
            GCHandle pFrame = GCHandle.Alloc(Camera.Frame, GCHandleType.Pinned);
            tFrameCallback FrameCB = new tFrameCallback(FrameDummyCBSnap);
            uint timeout = 1000;

            // Set the camera is acquisition mode.
            if ((Pv.CaptureStart(Camera.Handle)) == 0)
            {
                // Set the acquisition mode to continuous.
                if ((Pv.CommandRun(Camera.Handle, "AcquisitionStart")) != 0)
                {
                    // If that fails,reset the camera to non capture mode.
                    Pv.CaptureEnd(Camera.Handle);
                        return false;
                }
                else
                    {
                        bool failed = false;

                            // Enqueue the single frame.
                            if (Pv.CaptureQueueFrame(Camera.Handle, pFrame.AddrOfPinnedObject(), FrameCB) == 0)
                            {

                                Console.WriteLine("Waiting for the frame...");
                                failed = Pv.CaptureWaitForFrameDone(Camera.Handle, pFrame.AddrOfPinnedObject(), timeout) != 0;
                                Console.WriteLine("Frame is done...");
                                Console.WriteLine("Snapping done.");
                            }
                            else
                                failed = true;

                            Pv.CommandRun(Camera.Handle, "AcquisitionStop");
                            Pv.CaptureEnd(Camera.Handle);
                            return !failed;       
                    }
            }
            else
                return false;
        }


           
        
        static void Main(string[] args)
        {
            tErr err = 0;

            // Initialize the API.
            if ((err = Pv.Initialize()) == 0)
            {
                tCamera Camera = new tCamera();
                tLinkCallback lLinkCB = new tLinkCallback(LinkCB);
                

                // Add callbacks for camera events.
                if (Pv.LinkCallbackRegister(lLinkCB, tLinkEvent.eLinkAdd, IntPtr.Zero) != 0)
                    Console.WriteLine("Failed to register the callback.");
                if (Pv.LinkCallbackRegister(lLinkCB, tLinkEvent.eLinkRemove, IntPtr.Zero) != 0)
                    Console.WriteLine("Failed to register the callback.");

                // Wait for a camera to be plugged in.
                WaitForCamera();

                // Grab a camera from the list.
                if (CameraGrab(ref Camera))
                {
                    // Setup the camera.
                    if (CameraSetup(ref Camera))
                    {
                            DoSnap(ref Camera);
                            DoSnap(ref Camera);
                            DoStream(ref Camera);
                            DoStream(ref Camera);
                            DoSnap(ref Camera);
                            DoStream(ref Camera);
                            DoStream(ref Camera);
  
                            // Unsetup the camera.
                             CameraUnsetup(ref Camera);
                            }

                        else
                            Console.WriteLine("Camera {0} failed to be setup.", Camera.UID);
                    }
                    else
                        Console.WriteLine("Failed to find a camera.");

                    // Remove callbacks.
                    if (Pv.LinkCallbackUnregister(lLinkCB, tLinkEvent.eLinkAdd) != 0)
                        Console.WriteLine("Failed to unregister the callback.");
                    if (Pv.LinkCallbackUnregister(lLinkCB, tLinkEvent.eLinkRemove) != 0)
                        Console.WriteLine("Failed to unregister the callback.");

                // Uninitialize the API.
                    Pv.UnInitialize();
                }
                else
                {
                    Console.Write("Failed to initialize the API.");
                    Console.WriteLine(err);
                }
        }
    }
}
