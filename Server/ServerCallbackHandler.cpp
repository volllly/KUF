#include "ServerCallbackHandler.h"
#include <iostream>

using namespace std;

ServerCallbackHandler::ServerCallbackHandler()
{
  myDll.Init();
}

ServerCallbackHandler::~ServerCallbackHandler()
{
  myDll.ShutDown();
}


void ServerCallbackHandler::NewConnectionCB(const char *hostname)
{
  std::cout << "got new connection from " << hostname << std::endl;
  /*int values[8];

  for (int i = 0; i < 255; ++i)
  {
    values[0] = 1;
    values[1] = 255;

    values[2] = 3;
    values[3] = i;

    values[4] = 4;
    values[5] = i;

    values[6] = 5;
    values[7] = i;

    if (!myDll.SetChannelValue(values, 4))
      cerr << "error setting values" << endl;

    Sleep(10);
  }

  values[0] = 1;
  values[1] = 0;

  values[2] = 2;
  values[3] = 0;

  values[4] = 3;
  values[5] = 0;

  values[6] = 4;
  values[7] = 0;
  */
}

void ServerCallbackHandler::ConnectionLost()
{
  std::cout << "connection lost" << std::endl;
}

void ServerCallbackHandler::DataReceived(const char *data, unsigned len)
{
  // here we rely on the fact that the data is a string!
  std::cout << "got new data >>" << data << "<< len " << len << std::endl;

  // just send this back to the client
  std::string strdata = std::string(data);

  if (myComm) {
      if (strdata.find("config") != std::string::npos) {
        std::string sent = std::string("211: 1\r\n");
        myComm->WriteToPartner(sent.c_str(), sent.length() + 1);
      } else if (strdata.find("set") != std::string::npos) {
          strdata = strdata.substr(strdata.find(":") + 1, -1);
          std::string sent = std::string("203:" + strdata);
          myComm->WriteToPartner(sent.c_str(), sent.length() + 1);
      } else if (strdata.find("connect") != std::string::npos || strdata.find("version") != std::string::npos) {
          std::string sent = std::string("210: 1\r\n");
          myComm->WriteToPartner(sent.c_str(), sent.length() + 1);
      }
  }
}
