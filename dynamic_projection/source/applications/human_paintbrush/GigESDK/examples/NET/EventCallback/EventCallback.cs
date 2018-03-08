/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Prosilica is prohibited.
|
|==============================================================================
|
| This sample code, open the first camera found on the host computer and streams
| frames from it for 10 seconds while receiving events from the camera.
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



namespace EventCallback
{
    class EventCallback
    {

        // Place holder for all the data being used.
        private struct tCamera
        {
            public UInt32 UID;
            public UInt32 Handle;
            public string Serial;
        }


        // Global camera.
        static tCamera GCamera = new tCamera();


        // Callbacks called when the camera is plugged/unplugged.
        static void LinkCB(IntPtr Context, tInterface Interface, tLinkEvent Event, UInt32 UniqueId)
        {
            switch (Event)
            {
                case tLinkEvent.eLinkAdd:
                    {
                        Console.WriteLine();
                        Console.WriteLine("Camera {0} has been plugged in.", UniqueId);

                        break;
                    }
                case tLinkEvent.eLinkRemove:
                    {
                        Console.WriteLine();
                        Console.WriteLine("Camera {0} has been unplugged.", UniqueId);

                        break;

                    }
            }
        }
        
        
        // Event callback. This is called by PvApi when a camera event occurs.
        static void EventDone(IntPtr Context, UInt32 Handle, tCameraEvent[] EventList, UInt32 EventListLength)
        {
          
                Console.WriteLine("Event ID: {0}.", EventList[0].EventId);  
        }


        // Callback for when the frame is completed.
        static void FrameDoneCB(IntPtr pFrame)
        {

            // Marshal the pointer into a frame structure.
            tFrame Frame = (tFrame)Marshal.PtrToStructure(pFrame, typeof(tFrame));

            Console.WriteLine("Frame recieved {0}.", (uint)Frame.Status);

            // If the frame was completed (or if data were missing/lost), re-enqueue it.
            if (Frame.Status != tErr.eErrUnplugged &&
                Frame.Status != tErr.eErrCancelled)
            {
                Pv.CaptureQueueFrame(GCamera.Handle, pFrame, FrameDoneCB);
            }
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
            Thread.Sleep(1000);
            // Unsetup event channel.
            Pv.AttrUint32Set(GCamera.Handle, "EventsEnable1", 0);
            Pv.CameraEventCallbackUnRegister(GCamera.Handle, EventDone);

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
            tFrameCallback lFrameCB = new tFrameCallback(FrameDoneCB);
            tCameraEventCallback lEventCB = new tCameraEventCallback(EventDone);
            UInt32 FrameSize = 0;
            IntPtr Context = IntPtr.Zero;


            if (Pv.AttrExists(GCamera.Handle, "EventsEnable1") == tErr.eErrNotFound)
            {
                Console.WriteLine("This camera does not support event notifications.");
                return false;
            }

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

                // Setup the event channel.
                Pv.AttrUint32Set(GCamera.Handle, "EventsEnable1", 0);
                Pv.AttrEnumSet(GCamera.Handle, "EventSelector", "AcquisitionStart");
                Pv.AttrEnumSet(GCamera.Handle, "EventNotification", "On");
                Pv.AttrEnumSet(GCamera.Handle, "EventSelector", "AcquisitionEnd");
                Pv.AttrEnumSet(GCamera.Handle, "EventNotification", "On");
                Pv.AttrEnumSet(GCamera.Handle, "EventSelector", "FrameTrigger");
                Pv.AttrEnumSet(GCamera.Handle, "EventNotification", "On");


                if (Pv.CameraEventCallbackRegister(GCamera.Handle, lEventCB, Context) != tErr.eErrSuccess)
                {
                    Console.WriteLine("There was an error accessing the driver.");
                    return false;
                }

                // Start the capture mode.
                if (Pv.CaptureStart(GCamera.Handle) == 0)
                {
                    if (Pv.AttrFloat32Set(GCamera.Handle, "FrameRate", 5) == 0)
                    {

                        // Set the camera in continuous acquisition mode,and in "Freerun".
                        if (Pv.AttrEnumSet(GCamera.Handle, "FrameStartTriggerMode", "FixedRate") == 0)
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
                                Pv.CaptureQueueFrame(GCamera.Handle, pFrame.AddrOfPinnedObject(), lFrameCB);

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


        // Wait for a camera to be detected.
        static void WaitForCamera()
        {
            Console.Write("Waiting for a camera ");
            while (Pv.CameraCount() == 0)
            {
                Console.Write(".");
                Thread.Sleep(250);
            }
            Console.WriteLine("");
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

                WaitForCamera();

                if (CameraGrab())
                {
                    if (CameraSetup())
                    {
                        if (CameraStart())
                        {
                            Console.WriteLine("The camera will be streaming for the next 10 seconds.");
                            Thread.Sleep(10000);
                            CameraStop();
                        }
                        else
                            Console.WriteLine("The camera failed to start streaming.");

                        CameraClose();
                    }
                    else
                        Console.WriteLine("The camera could not be setup.");
                }
                else
                    Console.WriteLine("The was no camera grabbed.");


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
