
/*
Copyright (c) 2011 Robert Ellis (holistic [at] robellis [dot] org [dot] uk)
 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef FTP_REQUEST_HANDLER_H
#define FTP_REQUEST_HANDLER_H


#include "FTPServer.h"
#include "FileSystemInterface.h"
#include "core/netservice.h"

#include "mbed.h"

#define FTP_REQUEST_TIMEOUT 300000
#define REQUEST_BUFFER_LENGTH 128
#define START_DIRECTORY "/"

#include <string>
using std::string;



enum FtpCommand 
{ 
    NOOP, 
    MLSD 
};



class FTPRequestHandler : public NetService
{
public:

    FTPRequestHandler(FTPServer* pSvr, Host* pCommandClient, TCPSocket* pCommandSocket);
    virtual ~FTPRequestHandler();
  
    void acceptDataSocket(TCPSocket* pDataSocket);
  
private:

    void acceptCommandSocket(TCPSocket* pCommandSocket);
    void close();
    bool openDataSocket();
    void closeDataSocket();

    void onCommandSocketEvent(TCPSocketEvent e);
    void onDataSocketEvent(TCPSocketEvent e);
    void onTimeout();

    void sendReply(int replyCode, const char* format, ...);
    void sendReply(const char* format, ...);
    void sendData(const char* data, int length);

    int getRequest();
    void handleRequest();
    void handleResponse();
    
    bool isAuthenticated();
    void sendSyntaxError();
    
    
    void cmdUser(const string& username);
    void cmdPass(const string& password);
    void cmdQuit();
    
    void cmdFeat();
    
    void cmdPort(const string& ip_port);
    void cmdPasv();
    void cmdType(const string& type, const string& arg);
    
    void cmdPwd();
    void cmdCwd(const string& dir);
    void cmdCdup();
    void cmdList();
    void cmdMlsd();
    
  
  
    FTPServer* m_pSvr;
    Host* m_pCommandClient;
    Host* m_pDataClient;
    TCPSocket* m_pCommandSocket;
    TCPSocket* m_pDataSocket;
    
    FileSystemInterface fileSystem;
  
    string request;
    bool m_closed;
    FTPUser m_user;
    bool m_authenticated;
    FtpCommand m_command;
    
    string m_data_mlsd;
    
};

#endif
