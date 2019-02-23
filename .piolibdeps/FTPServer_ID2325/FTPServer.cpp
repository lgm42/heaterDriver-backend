
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

#include "FTPServer.h"
#include "FTPRequestHandler.h"

//#define __DEBUG
#include "dbg/dbg.h"

FTPServer::FTPServer() : m_commandSocket(), m_dataSocket()
{
    setWelcomeMessage("--- mbed FTP Server ---");

    m_commandSocket.setOnEvent(this, &FTPServer::onCommandSocketEvent);
    m_dataSocket.setOnEvent(this, &FTPServer::onDataSocketEvent);
}

FTPServer::~FTPServer()
{
}

void FTPServer::bind(int commandPort, int dataPort)
{
    m_commandPort = commandPort;
    m_dataPort = dataPort;

    Host commandHost(IpAddr(127,0,0,1), commandPort, "localhost");
    error = m_commandSocket.bind(commandHost);
    if(checkError("1")) return;
    error = m_commandSocket.listen();
    if(checkError("2")) return;
    
    Host dataHost(IpAddr(127,0,0,1), dataPort, "localhost");
    error = m_dataSocket.bind(dataHost);
    if(checkError("3")) return;
    error = m_dataSocket.listen();
    if(checkError("4")) return;
}

  
  
void FTPServer::onCommandSocketEvent(TCPSocketEvent e)
{
    DBG("Command Event %d\r\n", e);

    if(e==TCPSOCKET_ACCEPT)
    {
        DBG("Event TCPSOCKET_ACCEPT\r\n");
        
        TCPSocket* pTCPSocket;
        Host* client = new Host();

        if( !!m_commandSocket.accept(client, &pTCPSocket) )
        {
            DBG("HTTPServer::onCommandSocketEvent : Could not accept connection.\r\n");
            return; //Error in accept, discard connection
        }

        FTPRequestHandler* pHandler = new FTPRequestHandler(this, client, pTCPSocket); //TCPSocket ownership is passed to handler
        
        IpAddr ip = client->getIp();
        long l = 0;
        l |= ip[0] & 0xFF;
        l <<= 8;
        l |= ip[1] & 0xFF;
        l <<= 8;
        l |= ip[2] & 0xFF;
        l <<= 8;
        l |= ip[3] & 0xFF;
        connections[l] = pHandler;
    }
}

void FTPServer::onDataSocketEvent(TCPSocketEvent e)
{
    DBG("Data Event %d\r\n", e);

    if(e==TCPSOCKET_ACCEPT)
    {
        DBG("Event TCPSOCKET_ACCEPT\r\n");
        
        TCPSocket* pTCPSocket;
        Host* client = new Host();

        if( !!m_dataSocket.accept(client, &pTCPSocket) )
        {
            DBG("HTTPServer::onDataSocketEvent : Could not accept connection.\r\n");
            return; //Error in accept, discard connection
        }
        
        IpAddr ip = client->getIp();
        long l = 0;
        l |= ip[0] & 0xFF;
        l <<= 8;
        l |= ip[1] & 0xFF;
        l <<= 8;
        l |= ip[2] & 0xFF;
        l <<= 8;
        l |= ip[3] & 0xFF;
        FTPRequestHandler* handler = connections[l];
        if(handler)
        {
            handler->acceptDataSocket(pTCPSocket);
        }
        else
        {
            DBG("No handler found for data connection.");
            pTCPSocket->close();
            delete pTCPSocket;
        }
    }
}


void FTPServer::setWelcomeMessage(string message)
{
    m_message = message;
}

string& FTPServer::getWelcomeMessage()
{
    return m_message;
}

void FTPServer::addUser(FTPUser& user)
{
    users.push_back(user);
}

FTPUser* FTPServer::checkUser(const string& username, const string& password)
{
    for(list<FTPUser>::iterator it = users.begin(); it != users.end(); it++)
    {
        FTPUser& user = *it;
        if(username.compare(user.getUsername()) == 0 && password.compare(user.getPassword()) == 0)
        {
            return &user;
        }
    }
    
    return NULL;
}

bool FTPServer::checkError(char* msg)
{
    if(error != TCPSOCKET_OK)
    {
        if(error == TCPSOCKET_SETUP)
        {
            DBG("%s: Error TCPSOCKET_SETUP\r\n", msg);
        }
        else if(error == TCPSOCKET_TIMEOUT)
        {
            DBG("%s: Error TCPSOCKET_TIMEOUT\r\n", msg);
        }
        else if(error == TCPSOCKET_IF)
        {
            DBG("%s: Error TCPSOCKET_IF\r\n", msg);
        }
        else if(error == TCPSOCKET_MEM)
        {
            DBG("%s: Error TCPSOCKET_MEM\r\n", msg);
        }
        else if(error == TCPSOCKET_INUSE)
        {
            DBG("%s: Error TCPSOCKET_INUSE\r\n", msg);
        }
        else if(error == TCPSOCKET_EMPTY)
        {
            DBG("%s: Error TCPSOCKET_EMPTY\r\n", msg);
        }
        else if(error == TCPSOCKET_RST)
        {
            DBG("%s: Error TCPSOCKET_RST\r\n", msg);
        }
        return true;
    }
    return false;
}


