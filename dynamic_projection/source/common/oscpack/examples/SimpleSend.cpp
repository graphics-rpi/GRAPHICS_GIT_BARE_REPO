/* 
    Simple example of sending an OSC message using oscpack.
*/

#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"
#include <iostream>
using namespace std;


#define ADDRESS "128.113.243.57"
#define PORT 7002

#define OUTPUT_BUFFER_SIZE 1024

int main(int argc, char* argv[])
{
    UdpTransmitSocket transmitSocket( IpEndpointName( ADDRESS, PORT ) );
    
    char buffer[OUTPUT_BUFFER_SIZE];
    
	for (int i=0; i<127; i+=3){
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		p << osc::BeginBundleImmediate
		  << osc::BeginMessage( "piano" ) << i << 0 << 0 << 0 << osc::EndMessage;
		transmitSocket.Send( p.Data(), p.Size() );
		usleep(100000);
	}
	for (int i=0; i<127; i+=3){
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		p << osc::BeginBundleImmediate
		  << osc::BeginMessage( "piano" ) << 127 << i << 0 << 0 << osc::EndMessage;
		transmitSocket.Send( p.Data(), p.Size() );
		usleep(100000);
	}
	for (int i=0; i<127; i+=3){
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		p << osc::BeginBundleImmediate
		  << osc::BeginMessage( "piano" ) << 127 << 127 << i << 0 << osc::EndMessage;
		transmitSocket.Send( p.Data(), p.Size() );
		usleep(100000);
	}
	for (int i=0; i<127; i+=3){
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		p << osc::BeginBundleImmediate
		  << osc::BeginMessage( "piano" ) << 127 << 127 << 127 << i << osc::EndMessage;
		transmitSocket.Send( p.Data(), p.Size() );
		usleep(100000);
	}
	
	//==================================================================================
	
	
	for (int i=127; i>=0; i-=3){
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		p << osc::BeginBundleImmediate
		  << osc::BeginMessage( "piano" ) << i << i << i << 127 << osc::EndMessage;
		transmitSocket.Send( p.Data(), p.Size() );
		usleep(100000);
	}
	for (int i=127; i>=0; i-=3){
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		p << osc::BeginBundleImmediate
		  << osc::BeginMessage( "piano" ) << 0 << 0 << (-1*(i-127)) << i << osc::EndMessage;
		transmitSocket.Send( p.Data(), p.Size() );
		usleep(100000);
	}
	for (int i=127; i>=0; i-=3){
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		p << osc::BeginBundleImmediate
		  << osc::BeginMessage( "piano" ) << 0 << (-1*(i-127)) << i << 0 << osc::EndMessage;
		transmitSocket.Send( p.Data(), p.Size() );
		usleep(100000);
	}
	for (int i=127; i>=0; i-=3){
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		p << osc::BeginBundleImmediate
		  << osc::BeginMessage( "piano" ) << (-1*(i-127)) << i << 0 << 0 << osc::EndMessage;
		transmitSocket.Send( p.Data(), p.Size() );
		usleep(100000);
	}
	//=================================================================================
	for (int i=0; i<127; i+=3){
		osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
		p << osc::BeginBundleImmediate
		  << osc::BeginMessage( "piano" ) << 127 << i << i << i << osc::EndMessage;
		transmitSocket.Send( p.Data(), p.Size() );
		usleep(100000);
	}
	
    
//    for (int i=30; i<100; i+=4){
//    	osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
//    	p << osc::BeginBundleImmediate
//    	  << osc::BeginMessage( "piano" ) << 1 << i << osc::EndMessage;
//    	transmitSocket.Send( p.Data(), p.Size() );
//    	usleep(120000);
//    }
    //    << osc::BeginMessage( "bla2" ) 
    //        << true << 24 << (float)10.8 << "world" << osc::EndMessage;
    //cout << "hereerer" << endl;
    //p << osc::EndBundle;
    //cout << "here" << endl;
    //transmitSocket.Send( p.Data(), p.Size() );
}

