/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Allied Vision Technologies is prohibited.
|
|==============================================================================
|
| This sample code opens the first camera found on the host computer and streams
| frames from it until the user terminates it.
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

namespace Stream
{
    class Stream
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

        private static bool close = false;


        // Callback called when the camera is plugged/unplugged.
        static void LinkCB(IntPtr Context, tInterface Interface, tLinkEvent Event, UInt32 UniqueId)
        {
            switch (Event)
            {
                    
                case tLinkEvent.eLinkAdd:
                    {
                        Console.WriteLine("");
                        Console.WriteLine("Camera {0} is plugged in.", UniqueId);
                        break;
                    }
                case tLinkEvent.eLinkRemove:
                    {
                        Console.WriteLine("");
                        Console.WriteLine("Camera {0} is unplugged.", UniqueId);
                        break;
                    }
            }
        }


        // Frame callback dummy. 
        static void FrameDummyCB(IntPtr pFrame)
        {
        }


        // Callback called when a frame is done.
        static void FrameDoneCB(ref tFrame pFrame, ref tCamera Camera)
        {
            GCHandle pFrame1 = GCHandle.Alloc(Camera.Frame, GCHandleType.Pinned);

            tFrameCallback FrameCB = new tFrameCallback(FrameDummyCB);

            // If the frame was completed, re-enqueue it.
            if (pFrame.Status != tErr.eErrUnplugged && pFrame.Status != tErr.eErrCancelled)
                Pv.CaptureQueueFrame(Camera.Handle, pFrame1.AddrOfPinnedObject(), FrameCB);
        }


        // Wait for a camera to be detected.
        static void WaitForCamera()
        {
            Console.Write("Waiting for a camera ");
            while (Pv.CameraCount() == 0)
            {
                Console.Write(".");
                Thread.Sleep(250);
            }
            Console.WriteLine(" ");
        }


        // Get the UID and Serial Number of the first camera listed.
        static bool CameraGrab(ref tCamera Camera)
        {
            UInt32 count, connected = 0;
            tCameraInfo[] list = new tCameraInfo[1];

            if ((count = Pv.CameraList(list, 1, ref connected)) != 0)
            {
                Camera.UID = list[0].UniqueId;
                Camera.Serial = list[0].SerialString;
                Console.WriteLine("Grabbing camera: {0} ",Camera.Serial);
                return true;

            }
            else
                return false;
        }


        // Open the camera.
        static bool CameraSetup(ref tCamera Camera)
        {
            return Pv.CameraOpen(Camera.UID, tAccessFlags.eAccessMaster, out Camera.Handle) == 0;
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
            Console.WriteLine("Camera is closed.");
            // Delete the allocated buffer.
            Camera.GC.Free();
            // Reset the handle.
            Camera.Handle = 0;
        }


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
            Console.WriteLine("Press the 'X' key to stop the streaming.");

            while (true)
            {
 
                // Start a console read operation. Do not display the input.
                cki = Console.ReadKey(true);
                // Exit if the user pressed the 'X' key.
                if (cki.Key == ConsoleKey.X)
                {
                    // Announce the name of the key that was pressed.
                    Console.WriteLine("Key pressed: {0}\n", cki.Key);
                    close = true;
                    break;
                }
            }
        }



        // Setup the camera for capture.
        static bool CameraStart(ref tCamera Camera)
        {
            UInt32 FrameSize = 0;

            // Adjust packet for optimal preformance.
            Pv.CaptureAdjustPacketSize(Camera.Handle, 8228);

            // Get the bytes size of the buffer.
            if (Pv.AttrUint32Get(Camera.Handle, "TotalBytesPerFrame", ref FrameSize) == 0)
            {
                Camera.Buffer = new byte[FrameSize];
                Camera.GC = GCHandle.Alloc(Camera.Buffer, GCHandleType.Pinned);
                // Address of the pinned buffer.
                Camera.Frame.ImageBuffer = Camera.GC.AddrOfPinnedObject(); 
                // Buffer size.
                Camera.Frame.ImageBufferSize = FrameSize;						

                // Start the capture mode.
                if (Pv.CaptureStart(Camera.Handle) == 0)
                {
                    //  Set the acquisition mode into continuous.
                    if (Pv.CommandRun(Camera.Handle, "AcquisitionStart") != 0)
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
                return false;
        }


        // Stop the streaming.
        static void CameraStop(ref tCamera Camera)
        {
            Console.WriteLine("Streaming is being stopped.");
            Pv.CommandRun(Camera.Handle, "AcquisitionStop");
            Pv.CaptureEnd(Camera.Handle);
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
                    Console.WriteLine("Failed to register callback.");
                if (Pv.LinkCallbackRegister(lLinkCB, tLinkEvent.eLinkRemove, IntPtr.Zero) != 0)
                    Console.WriteLine("Failed to register callback.");


                WaitForCamera();

                if (CameraGrab(ref Camera))
                {
                    if (CameraSetup(ref Camera))
                    {
                        // Stream as long as CTRL C isn't pressed.
                        while (close == false)
                        {
                            if (CameraStart(ref Camera))
                            {
                                Console.WriteLine("Camera is now streaming.");

                                // Stream the Camera until the use stops it. 
                                Abort();
                                CameraStop(ref Camera);
                            }

                            else
                                Console.WriteLine("Failed to start the streaming process.");

                            CameraClose(ref Camera);
                        }
                    }

                    else
                        Console.WriteLine("Camera {0} failed to be setup.", Camera.UID);
                }
                else
                    Console.WriteLine("Failed to find a camera.");

                //remove callbacks
                if (Pv.LinkCallbackUnregister(lLinkCB, tLinkEvent.eLinkAdd) != 0)
                    Console.WriteLine("Failed to unregister the callback.");
                if (Pv.LinkCallbackUnregister(lLinkCB, tLinkEvent.eLinkRemove) != 0)
                    Console.WriteLine("Failed to unregister the callback.");


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
