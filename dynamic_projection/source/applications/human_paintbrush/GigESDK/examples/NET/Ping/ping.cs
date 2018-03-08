/*
| ==============================================================================
| Copyright (C) 2011 Allied Vision Technologies.  All Rights Reserved.
|
| Redistribution of this header file, in original or modified form, without
| prior written consent of Allied Vision Technologies is prohibited.
|
|==============================================================================
|
| This sample code seeks out a camera by using its IP address and displays 
! some information about it
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

namespace Ping
{
    class Ping
    {
        private struct tCamera
        {
            public UInt32 Handle;
            public UInt32 S_addr;
            public tCameraInfo Info;
            public tIpSettings Conf;
           
        };

      // Open the Camera by IP.
        static bool CameraOpen(ref tCamera Camera)
        {
            // Open by the address of camera.
            if ((Pv.CameraOpenByAddr(Camera.S_addr, tAccessFlags.eAccessMaster, out Camera.Handle) == tErr.eErrSuccess))
            {
                Pv.CameraInfoByAddr(Camera.S_addr, ref Camera.Info, ref Camera.Conf);
                return true;
            }
            else
                return false;
        }


        // Close the camera.
        static void CameraClose(ref tCamera Camera)
        {
            // Close the camera.
            Pv.CameraClose(Camera.Handle);
        }


        // Display infomation about the camera detected.
        static void DisplayInfo(ref tCamera Camera)
        {
            // Display the serial number and the display name.
            Console.WriteLine("-> {0} - {1}", Camera.Info.SerialString, Camera.Info.DisplayName);

            Console.Write("Mode supported: ");

            if ((Camera.Conf.ConfigModeSupport & (uint)tIpConfig.eIpConfigPersistent.GetTypeCode()) != 0)
                Console.Write("Fixed ");
            if ((Camera.Conf.ConfigModeSupport & (uint)tIpConfig.eIpConfigDhcp.GetTypeCode()) != 0)
                Console.Write("DHCP&AutoIP ");
            if ((Camera.Conf.ConfigModeSupport & (uint)tIpConfig.eIpConfigAutoIp.GetTypeCode()) != 0)
                Console.Write("AutoIP");

            Console.WriteLine();

            // Check to see which mode the camera is in.
            Console.Write("Current mode: ");

            if (Camera.Conf.ConfigMode == tIpConfig.eIpConfigPersistent)
                Console.WriteLine("Fixed");
            else
            if (Camera.Conf.ConfigMode == tIpConfig.eIpConfigDhcp)
                Console.WriteLine("DHCP&AutoIP");
            else
            if (Camera.Conf.ConfigMode == tIpConfig.eIpConfigAutoIp)
                Console.WriteLine("AutoIP");
            else
                Console.WriteLine("none");

            // Display the IP address.
            Camera.S_addr = Camera.Conf.CurrentIpAddress;
            System.Net.IPAddress Address = new IPAddress(Camera.S_addr);
            Console.WriteLine("Current address: {0}",Address);

            // Display the subnet.
            Camera.S_addr = Camera.Conf.CurrentIpSubnet;
            System.Net.IPAddress Subnet = new IPAddress(Camera.S_addr);
            Console.WriteLine("Current subnet: {0}",Subnet);

            //display Gateway
            Camera.S_addr = Camera.Conf.CurrentIpGateway;
            System.Net.IPAddress GateWay = new IPAddress(Camera.S_addr);
            Console.WriteLine("Current gateway: {0}",GateWay);
            
        }


        static void Main(string[] args)
        {
            tErr err = 0;

            // Initialize the API.
            if ((err = Pv.InitializeNoDiscovery()) == 0)
            {
                System.Net.IPAddress IP = System.Net.IPAddress.Parse(args[0]);
                tCamera Camera = new tCamera();

                Camera.S_addr = (uint)IP.Address;

                // Open the camera.
                if (CameraOpen(ref Camera))
                {
                    // List some infomation.
                    DisplayInfo(ref Camera);

                    // Unsetup the camera.
                    CameraClose(ref Camera);
                }
                else 
                    Console.WriteLine("Failed to open camera.");

                // Uninitialize the API.
                Pv.UnInitialize();
            }
                else Console.WriteLine("Failed to initalize.");
        }
    }
}
