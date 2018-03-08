/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Allied Vision Technologies is prohibited.
|
|==============================================================================
|
| This sample code resets a given camera, as specified by its IP address.
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
using PvNET;

namespace ResetCamera
{
    class ResetCamera
    {
        private struct tCamera
        {
            public UInt32 Handle;
            public UInt32 UID;
            public UInt32 S_addr;
            public tIpSettings Conf;
            public tErr lErr;
            public string Serial;
            public string DisplayName;
        };


        // Wait for a camera to be detected.
        static void WaitForCamera()
        {
            Console.Write("Waiting for a camera ");
            while (Pv.CameraCount() == 0)
            {
                Console.Write(".");
                Thread.Sleep(250);
            }
            Console.WriteLine();
        }


        // Get the first camera found.
        static bool CameraGrab()
        {
            UInt32 connected = 0;
            tCameraInfo[] list = new tCameraInfo[1];

            return (Pv.CameraList(list, 1, ref connected) >= 1);
        }


        // Get the UID and Serial Number of the first camera listed.
        static bool CameraGet(ref tCamera Camera)
        {
            UInt32 count, connected = 0;
            tCameraInfo[] list = new tCameraInfo[1];

            if ((count = Pv.CameraList(list, 1, ref connected)) != 0)
            {
                Camera.UID = list[0].UniqueId;
                Camera.Serial = list[0].SerialString;
                Camera.DisplayName = list[0].DisplayName;
               
                return true;
            }
            else
                return false;
        }


        // Open the Camera by IP.
        static bool CameraOpen(ref tCamera Camera)
        {
            // Get the camera's IP configuration.
            if ((Camera.lErr = Pv.CameraIpSettingsGet(Camera.UID, ref Camera.Conf)) == tErr.eErrSuccess)
            
                Camera.S_addr = Camera.Conf.CurrentIpAddress;

            // Open by the address of camera.
            if ((Pv.CameraOpenByAddr(Camera.S_addr, tAccessFlags.eAccessMaster, out Camera.Handle) == tErr.eErrSuccess))
            {
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
            // Close the camera.
            Pv.CameraClose(Camera.Handle);
        }


        // Reset the camera.
        static void CameraReset(ref tCamera Camera)
        {
            UInt32 [] Address = new UInt32 [1];
            // Register address.
            Address[0] = 0x10008;
            UInt32 [] Value = new UInt32 [1];
            // Hard-reset value.
            Value[0] = 2;
            UInt32 NumComplete = 0;

            Pv.RegisterWrite(Camera.Handle, 1, Address, Value, ref NumComplete);

            Console.WriteLine("The camera: {0} was reset.", Camera.DisplayName);
            
        }


        static void Main(string[] args)
        {
            tErr err = 0;
            tCameraInfo[] List = new tCameraInfo[10];

            // Initialize the API.
            if ((err = Pv.Initialize()) == 0)
            {
                tCamera Camera = new tCamera();

                // Wait for a camera to be detected.
                WaitForCamera();

                // Get infomation from the camera.
                CameraGet(ref Camera);

                if (CameraGrab())
                {
                        // Open the camera.
                        if (CameraOpen(ref Camera))
                        {
                            // Reset the camera.
                            CameraReset(ref Camera);
                            
                            // Unsetup the camera.
                            CameraClose(ref Camera);
                        }

                        else 
                            Console.WriteLine("Failed to open camera.");
                }
                else 
                    Console.WriteLine("Failed to grab camera.");

                    // Uninitialize the API.
                    Pv.UnInitialize();
            }
                else Console.WriteLine("Failed to initalize.");
        }
    }
}
