/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| This code may be used in part, or in whole for your application development.
|
|==============================================================================
|
| Continuously gets the list of cameras and displays it.
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
using System.Net;
using PvNET;

namespace ListCameras
{
   
    class ListCameras
    {
        private struct in_addr
        {
            public uint S_addr;
            public tIpSettings Conf;
            public tErr lErr;
        }


        static void ListingCameras(ref in_addr Camera)
        {
            UInt32 MaxCameraList = 20;
            tCameraInfo[] CameraList = new tCameraInfo[MaxCameraList];
            UInt32 CameraNum = 0;
            UInt32 CameraRle;
            UInt32 Zero = 0;
            bool gstop = true;


            while (gstop)
            {

                Console.WriteLine("***********************************");

                // Get list of reachable cameras.
                CameraNum = Pv.CameraList(CameraList, MaxCameraList, ref Zero);

                // Store how many cameras listed are reachable.
                CameraRle = CameraNum;

                // Append the list of unreachable cameras.
                if (CameraNum < MaxCameraList)
                
                    CameraNum += Pv.CameraListUnreachable(CameraList, (MaxCameraList - CameraNum), ref Zero);
                
                if (CameraNum > 0)
                {

                    // Go through the number of cameras detected.
                    for (UInt32 i = 0; i < CameraNum; i++)
                    {
                        if (i < CameraRle)
                        {
                            // Get the camera's IP configuration.
                            if ((Camera.lErr = Pv.CameraIpSettingsGet(CameraList[i].UniqueId, ref Camera.Conf)) == tErr.eErrSuccess)
                            {
                                Camera.S_addr = Camera.Conf.CurrentIpAddress;
                                System.Net.IPAddress Address = new IPAddress(Camera.S_addr);

                                // Display the camera info.
                                Console.Write("{0}- {1} - Unique ID = {2} IP@ = {3}", CameraList[i].SerialString, CameraList[i].DisplayName, CameraList[i].UniqueId, Address);
                                
                                // Check to see if the camera is being used.
                                if (CameraList[i].PermittedAccess == 6)
                                    Console.WriteLine(" [Available]");
                                else Console.WriteLine(" [In use]");

                                Console.WriteLine();
                            }
                            else
                            Console.Write("{0}- {1} - Unique ID = {2} [Unavailable] ", CameraList[i].SerialString, CameraList[i].DisplayName, CameraList[i].UniqueId);
                        }
                        else 
                            Console.Write("{0}- {1} - Unique ID = {2}", CameraList[i].SerialString, CameraList[i].DisplayName, CameraList[i].UniqueId);

                    }
                    if (CameraNum != CameraRle)
                        Console.WriteLine("Camera is unreachable.");
                }
                else 
                    Console.WriteLine("There is no camera detected.");

                // Wait a certain amount of time before checking again.
                Thread.Sleep(1500);
            }
        }


        static void Main(string[] args)
        {
            tErr err = 0;

            // initialize the API.
            if ((err = Pv.Initialize()) == 0)
            {
                in_addr Camera = new in_addr();

                ListingCameras(ref Camera);

                // Uninitialize the API.
                Pv.UnInitialize();

            }
            else Console.WriteLine("Failed to initialize.");
        }
    }
}
