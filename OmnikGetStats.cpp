
#include "OmnikGetStats.h"
#include "EnergyMonitorConfig.h"

#include <cstring>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <arpa/inet.h>
#include <errno.h>

using namespace std;

namespace
{
    const string cOmnikSerialNumberConfigName   = "OmnikSN";
    const string cOmnikIPConfigName             = "OmnikIP";
    
    const int cOmnikPortNumber = 8899;

    const int cUpdateInterval = 300;
}

OmnikGetStats::OmnikGetStats(const EnergyMonitorConfig& config) :
    myOmnikTemperature(.0),
    myOmnikPower(.0),
    myOmnikGeneratedToday(.0),
    myOmnikGeneratedTotal(.0),
    myOmnikTotalHours(.0),
    myLastUpdateTime(0)
{
    myOmnikSerialNumber = atol(config.getValue(cOmnikSerialNumberConfigName).c_str());
    myOmnikIPAddress    = config.getValue(cOmnikIPConfigName);
}

bool OmnikGetStats::updateStats()
{
    bool result = false;

    if (time(NULL) > (myLastUpdateTime + cUpdateInterval))
    {
        result = getStats();

        if (result)
        {
            parseStats();
        }

        myLastUpdateTime = time(NULL);
    }

    return result;
}

bool OmnikGetStats::getStats()
{
	// generate the magic message
	// first 4 bytes are fixed x68 x02 x40 x30
	// next 8 bytes are the reversed serial number twice(hex)
	// next 2 bytes are fixed x01 x00
	// next byte is a checksum (2x each binary number form the serial number + 115)
	// last byte is fixed x16

	char magicMessage[] = {0x68, 0x02, 0x40, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x16};
	int checksum = 0;

	for (int i = 0; i < 4; ++i)
    {
		magicMessage[i + 4] = magicMessage[i + 8] = ((myOmnikSerialNumber >> (8 * i)) & 0xff);
		checksum += magicMessage[i + 4];
	}

	checksum *= 2;
	checksum += 115;
	checksum &= 0xff;

	magicMessage[14] = checksum;

	// Now create a TCP socket for communication with the Omnik
	int socket_desc;
	if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        cout << "Could not create TCP socket" << endl;
		return false;
	}

	struct sockaddr_in server;
	server.sin_addr.s_addr = inet_addr(myOmnikIPAddress.c_str());
	server.sin_family = AF_INET;
	server.sin_port = htons(cOmnikPortNumber);

	//Connect to remote server
	if (connect(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        cout << "TCP connect error: " << errno << endl;
		return false;
	}

	//Send the magic message
	if (send(socket_desc, magicMessage, 16, 0) < 0)
    {
		cout << "Send failed" << endl;
		return false;
	}

	//Receive a reply from the server
	memset(myOmnikReply, 0, sizeof(myOmnikReply));
	if (recv(socket_desc, myOmnikReply, sizeof(myOmnikReply), 0) < 0)
    {
		cout << "recv failed" << endl;
		return false;
	}

	return true;
}

void OmnikGetStats::parseStats()
{
    myOmnikTemperature    = ctonr(&myOmnikReply[31], 2, 10);
    myOmnikGeneratedToday = ctonr(&myOmnikReply[69], 2, 100);
    myOmnikGeneratedTotal = ctonr(&myOmnikReply[71], 4, 10);
    myOmnikTotalHours     = ctonr(&myOmnikReply[75], 4, 1);

    myOmnikPower = .0;
    for (int i = 0; i < 3; ++i)
    {
        if (ctonr(&myOmnikReply[33 + 2 * i], 2, 1) != 0)
        {
            myOmnikPower += ctonr(&myOmnikReply[59 + 2 * i], 2, 1);
        }
    }
}

double OmnikGetStats::ctonr(unsigned char* src, int nrofbytes, int div)
{
	if (nrofbytes <= 0 || nrofbytes > 4)
    {
		return -1;
    }

    double result = .0;
	int flag = 0;

	for (int i = nrofbytes; i > 0; --i)
    {
		result += (double)(src[i - 1] * pow(256, nrofbytes - i));
		if (src[i - 1] == 0xff)
        {
			flag++;
        }
	}

	if (flag == nrofbytes) // all oxff
    {
		result = 0;
    }

	result /= (double)div;

	return result;
}
