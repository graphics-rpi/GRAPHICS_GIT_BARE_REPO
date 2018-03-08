/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Prosilica is prohibited.
|
|==============================================================================
|
| This sample code, opens the first camera plugged in and streams from it until 
| X is pressed or until the camera is unplugged.
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

namespace StreamPnp
{
    class StreamPnp
    {

        // Place holder for all the data being used.
        private struct tCamera
        {
            public UInt32 UID;
            public UInt32 Handle;
            public string Serial;
        }


        static private tFrameCallback FrameCB = FrameDoneCB;
        static private bool Close = false;
        // Global camera.
        static tCamera GCamera = new tCamera();


        // Allows for time to be kept.
        [DllImport("kernel32.dll", CharSet = CharSet.Auto, ExactSpelling = true)]
        public static extern int GetTickCount();


        // Keeps track of data being streamed from the camera.
        static void ThreadFunc()
        {
            tErr Err;
            UInt32 Dropped, Done, Completed;
            UInt32 Total, Missed, Errs;
            int Before, Now, Elapsed;
            double Fps;
            float Rate;

            Dropped = 0;
            Missed = 0;
            Completed = 0;
            Errs = 0;
            Rate = 0;
            Fps = 0;
            Elapsed = 0;
            Total = 0;
            Done = 0;

            // Start timing.
            Before = GetTickCount();

            // Get some infomation about the streaming process. 
            while (Close == false && ((Err = Pv.AttrUint32Get(GCamera.Handle, "StatFramesCompleted", ref Completed)) == 0)
                && ((Err = Pv.AttrUint32Get(GCamera.Handle, "StatFramesDropped", ref Dropped)) == 0)
                && ((Err = Pv.AttrUint32Get(GCamera.Handle, "StatPacketsMissed", ref Missed)) == 0)
                && ((Err = Pv.AttrUint32Get(GCamera.Handle, "StatPacketsErroneous", ref Errs)) == 0)
                && ((Err = Pv.AttrFloat32Get(GCamera.Handle, "StatFrameRate", ref Rate)) == 0))
            {


                // Store the elapsed time. 
                Now = GetTickCount();

                // Keep track of the total amount of frames completed.
                Total += (Completed - Done);

                // Keep track of the total time that has elapsed.
                Elapsed += (Now - Before);

                // Updates the Fps rate every 500 milliseconds.
                if (Elapsed >= 500)
                {
                    Fps = (double)(Total * 1000.0) / (double)Elapsed;
                    Elapsed = 0;
                    Total = 0;
                }

                // Display the current infomation.
                Console.WriteLine("Completed : {0} Dropped : {1} Missed : {2} Err {3} Rate: {4:.00} <{5:.00}>", Completed, Dropped, Missed, Errs, Rate, Fps);

                Before = GetTickCount();
                Done = Completed;
                Thread.Sleep(20);
            }
            Console.WriteLine();
        }


        // Spawn a new thread.
        static void SpawnThread()
        {
            Thread Thread = new Thread(new ThreadStart(ThreadFunc));
            // Start the thread.
            Thread.Start();
        }


        // Callbacks called when the camera is plugged/unplugged.
        static void LinkCB(IntPtr Context, tInterface Interface, tLinkEvent Event, UInt32 UniqueId)
        {
            switch (Event)
            {
                case tLinkEvent.eLinkAdd:
                    {
                        System.Console.WriteLine("Camera {0} has been plugged in.", UniqueId);
                        HandleCameraPlugged(UniqueId);
                        break;
                    }
                case tLinkEvent.eLinkRemove:
                    {
                        System.Console.WriteLine("Camera {0} has been unplugged.", UniqueId);
                        HandleCameraUnplugged(UniqueId);
                        break;
                    }
            }
        }


        // Action taken if the camera is unplugged.
        static void HandleCameraUnplugged(UInt32 UniqueId)
        {
            if (GCamera.UID == UniqueId)
            {
                CameraStop();
                CameraClose();


                GCamera.Handle = 0;
                GCamera.UID = 0;
                Close = true;
            }
        }


        // Action taken if the camera is plugged in. 
        static void HandleCameraPlugged(UInt32 UniqueId)
        {
            if (GCamera.UID == 0)
            {
                GCamera.UID = UniqueId;

                if (CameraGrab())
                {
                    // Open the camera.
                    if (CameraSetup())
                    {
                        Console.WriteLine("Camera {0} is now open.", UniqueId);

                        // Start streaming from the camera.
                        if (CameraStart())
                        {
                            Console.WriteLine("Camera is now streaming.Press 'X' to terminate, or unplug the camera.");

                            // Spawn the thread while the camera is streaming.
                            SpawnThread();
                        }
                        else
                            Console.WriteLine("The camera failed to start streaming.");
                    }
                    else
                        Console.WriteLine("The camera failed to be setup.");
                }
                else
                    Console.WriteLine("The camera was not grabbed.");
            }
        }


        // Callback for when the frame is completed.
        static void FrameDoneCB(IntPtr pFrame)
        {

            // Marshal the pointer into a frame structure.
            tFrame Frame = (tFrame)Marshal.PtrToStructure(pFrame, typeof(tFrame));

            // If the frame was completed (or if data were missing/lost), re-enqueue it.
            if (Frame.Status == tErr.eErrSuccess ||
                Frame.Status == tErr.eErrDataLost ||
                Frame.Status == tErr.eErrDataMissing)
            
                Pv.CaptureQueueFrame(GCamera.Handle, pFrame, FrameCB);          
        }


        // Get the UID  and Serial number of the first camera listed.
        static bool CameraGrab()
        {
            UInt32 count, connected = 0;
            tCameraInfo[] list = new tCameraInfo[1];

            if ((count = Pv.CameraList(list, 1, ref connected)) != 0)
            {
                GCamera.UID = list[0].UniqueId;
                GCamera.Serial = list[0].SerialString;
                Console.WriteLine("Camera {0} grabbed. ", GCamera.Serial);
                return true;
            }
            else
                return false;
        }


        // Close the camera.
        static void CameraClose()
        {
            // Clear the queue.
            Console.WriteLine("Clearing the queue...");
            Pv.CaptureQueueClear(GCamera.Handle);
            // close the camera
            Console.WriteLine("The camera is now closed.");
            Pv.CameraClose(GCamera.Handle);
        }


        // Setup the camera up for streaming.
        static bool CameraStart()
        {
           
            UInt32 FrameSize = 0;

            // Adjust packet size for optimal performance. 
            Pv.CaptureAdjustPacketSize(GCamera.Handle, 8228);

            // Determines how big the frame buffers should be.
            if (Pv.AttrUint32Get(GCamera.Handle, "TotalBytesPerFrame", ref FrameSize) == 0)
            {

                tFrame Frame = new tFrame();
                byte[] Buffer = new byte[FrameSize];
                GCHandle pBuffer = GCHandle.Alloc(Buffer, GCHandleType.Pinned);

                // Set the frame's fields.
                // Handle to the Camera.
                Frame.Context.Field0 = new IntPtr(GCamera.Handle);
                // Address of the pinned object.		
                Frame.ImageBuffer = pBuffer.AddrOfPinnedObject();
                // Buffer size.
                Frame.ImageBufferSize = FrameSize;

                // Start the capture mode.
                if (Pv.CaptureStart(GCamera.Handle) == 0)
                {
                    // Set the camera in continuous acquisition mode,and in "Freerun".
                    if (Pv.AttrEnumSet(GCamera.Handle, "FrameStartTriggerMode", "Freerun") == 0)
                    {
                        // Set the acquisition mode into continuous.
                        if (Pv.CommandRun(GCamera.Handle, "AcquisitionStart") != 0)
                        {
                            // If that fails, reset the camera to non-capture mode.
                            Pv.CaptureEnd(GCamera.Handle);

                            Console.WriteLine("Failed to start.");
                            return false;
                        }
                        else
                        {
                            // Pin down a copy of the frame structure.
                            GCHandle pFrame = GCHandle.Alloc(Frame, GCHandleType.Pinned);

                            // Enqueue the frame.
                            Pv.CaptureQueueFrame(GCamera.Handle, pFrame.AddrOfPinnedObject(), FrameCB);

                            return true;
                        }
                    }
                    else
                        return false;
                }
                else
                    return false;
            }
            else
                return false;
        }


        // Stop the camera from streaming.
        static void CameraStop()
        {
            Console.WriteLine("The streaming is now being stopped.");
            Pv.CommandRun(GCamera.Handle, "AcquisitionStop");
            Pv.CaptureEnd(GCamera.Handle);
            // Clear the queue.
            Pv.CaptureQueueClear(GCamera.Handle);
        }


        // Open the camera.
        static bool CameraSetup()
        {
            return Pv.CameraOpen(GCamera.UID, tAccessFlags.eAccessMaster, out GCamera.Handle) == 0;
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

            while (true)
            {

                // Start a console read operation. Do not display the input.
                cki = Console.ReadKey(true);
                // Exit if the user pressed the 'X' key.
                if (cki.Key == ConsoleKey.X)
                {
                    // Announce the name of the key that was pressed.
                    Console.WriteLine("Key pressed: {0}\n", cki.Key);
                    Close = true;
                    CameraStop();
                    CameraClose();
                    break;
                }
            }
        }


        // Allow the camera to continue streaming until the user terminates it. 
        static void WaitForEver()
        {
            while (Close == false)
                Abort();
        }


        static void Main(string[] args)
        {
            tErr err = 0;

            // Initialize the API.
            if ((err = Pv.Initialize()) == 0)
            {

                tLinkCallback lLinkCB = new tLinkCallback(LinkCB);

                // Register callbacks.
                Pv.LinkCallbackRegister(lLinkCB, tLinkEvent.eLinkAdd, IntPtr.Zero);
                Pv.LinkCallbackRegister(lLinkCB, tLinkEvent.eLinkRemove, IntPtr.Zero);

                // Wait for the user to terminate or to unplug the camera.
                WaitForEver();

                // Unregister the callbacks.
                Pv.LinkCallbackUnregister(lLinkCB, tLinkEvent.eLinkAdd);
                Pv.LinkCallbackUnregister(lLinkCB, tLinkEvent.eLinkRemove);

                // Uninitialize the API.
                Pv.UnInitialize();

            }
            else
            {
                Console.Write("Failed to initialize the API : ");
                Console.WriteLine(err);
            }

        }

    }
}