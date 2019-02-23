
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

#include "core/netservice.h"
#include "FTPRequestHandler.h"
#include "stringUtils.h"
#include <string.h>
#include <sstream>

//#define __DEBUG
#include "dbg/dbg.h"

FTPRequestHandler::FTPRequestHandler(FTPServer* pSvr, Host* pCommandClient, TCPSocket* pCommandSocket) : 
    NetService(), m_pSvr(pSvr), m_pCommandClient(pCommandClient), m_closed(false), m_authenticated(false), 
    fileSystem()
{
    m_pDataSocket = NULL;
    m_pDataClient = NULL;
    m_command = NOOP;
    acceptCommandSocket(pCommandSocket);
}

FTPRequestHandler::~FTPRequestHandler()
{
    close();
}


void FTPRequestHandler::acceptCommandSocket(TCPSocket* pCommandSocket)
{
    m_pCommandSocket = pCommandSocket;
    m_pCommandSocket->setOnEvent(this, &FTPRequestHandler::onCommandSocketEvent);
    onCommandSocketEvent(TCPSOCKET_ACCEPT);
}

void FTPRequestHandler::acceptDataSocket(TCPSocket* pDataSocket)
{
    m_pDataSocket = pDataSocket;
    m_pDataSocket->setOnEvent(this, &FTPRequestHandler::onDataSocketEvent);
    onDataSocketEvent(TCPSOCKET_ACCEPT);
}

bool FTPRequestHandler::openDataSocket()
{
    if(m_pDataSocket != NULL)
    {
        sendReply(125, "Data connection already open; transfer starting.");
        return true;
    }

    m_pDataSocket = new TCPSocket;
    Host h(IpAddr(127,0,0,1), 20, "localhost");
    m_pDataSocket->bind(h);
    sendReply(150, "About to open data connection.");
    TCPSocketErr bindErr = m_pDataSocket->connect(*m_pDataClient);
    if(bindErr == TCPSOCKET_OK)
    {
        m_pDataSocket->setOnEvent(this, &FTPRequestHandler::onDataSocketEvent);
        return true;
    }
    else
    {
        closeDataSocket();
        sendReply(425, "Can't open data connection.");
        DBG("Error binding data socket: %d\r\n", bindErr);
        return false;
    }
}


void FTPRequestHandler::onCommandSocketEvent(TCPSocketEvent e)
{
    DBG("Command Event %d\r\n", e);
  
    if(m_closed)
    {
        DBG("\r\nWARN: Discarded\r\n");
        return;
    }

    switch(e)
    {
        case TCPSOCKET_ACCEPT:
            sendReply(220, m_pSvr->getWelcomeMessage().c_str());
            break;
        
        case TCPSOCKET_READABLE:
            DBG("Event TCPSOCKET_READABLE\r\n");
            handleRequest();
            break;
            
        case TCPSOCKET_WRITEABLE:
            DBG("Event TCPSOCKET_WRITEABLE\r\n");
            break;
            
        case TCPSOCKET_CONTIMEOUT:
        case TCPSOCKET_CONRST:
        case TCPSOCKET_CONABRT:
        case TCPSOCKET_ERROR:
        case TCPSOCKET_DISCONNECTED:
            close();
            break;
    }
}

void FTPRequestHandler::onDataSocketEvent(TCPSocketEvent e)
{
    DBG("Data Event %d\r\n", e);
  
    if(m_closed)
    {
        DBG("\r\nWARN: Discarded\r\n");
        return;
    }

    switch(e)
    {
        case TCPSOCKET_ACCEPT:
            sendReply(150, "Accepted data connection.");
            handleResponse();
            break;
    
        case TCPSOCKET_READABLE:
            DBG("Event TCPSOCKET_READABLE\r\n");
            break;
            
        case TCPSOCKET_WRITEABLE:
            DBG("Event TCPSOCKET_WRITEABLE\r\n");
            handleResponse();
            break;
            
        case TCPSOCKET_CONTIMEOUT:
        case TCPSOCKET_CONRST:
        case TCPSOCKET_CONABRT:
        case TCPSOCKET_ERROR:
        case TCPSOCKET_DISCONNECTED:
            closeDataSocket();
            break;
    }
}


void FTPRequestHandler::close() //Close socket and destroy data
{
    if(m_closed)
    {
        return;
    }
    
    closeDataSocket();
    
    m_closed = true; //Prevent recursive calling or calling on an object being destructed by someone else
    
    if(m_pCommandSocket) //m_pTCPSocket Should only be destroyed if ownership not passed to an handler
    {
        m_pCommandSocket->resetOnEvent();
        m_pCommandSocket->close();
        delete m_pCommandSocket; //This fn might have been called by this socket (through an event), so DO NOT DESTROY IT HERE
    }
    
    if(m_pCommandClient)
    {
        delete m_pCommandClient;
    }
    
    if(m_pDataClient)
    {
        delete m_pDataClient;
    }
    
    NetService::close();
}

void FTPRequestHandler::closeDataSocket()
{
    if(m_closed)
    {
        return;
    }
    
    if(m_pDataSocket)
    {
        sendReply(226, "Closing data connection.");
        m_pDataSocket->resetOnEvent();
        m_pDataSocket->close();
        delete m_pDataSocket;
        m_pDataSocket = NULL;
    }
}


int FTPRequestHandler::getRequest()
{
    char c[2];
    request = "";
    
    while(1)
    {
        int ret = m_pCommandSocket->recv(c, 1);
        
        if(!ret)
        {
            break;
        }
        
        if(request.length() > 0 && c[0] == '\n')
        {
            break;
        }
        else
        {
            request += c[0];
        }
    }
    
    request = trim(request);
    return request.length();
}


void FTPRequestHandler::sendReply(int replyCode, const char* format, ...)
{
    char buf[256], fmt[256];
    va_list args;
    int i;

    snprintf(fmt, 256, "%03u %s\r\n", replyCode, format);

    va_start(args, format);
    i = vsnprintf(buf, 256, fmt, args);
    va_end(args);

    buf[i+1] = 0;
    DBG("Response: %s", buf);

    m_pCommandSocket->send(buf, i);
}

void FTPRequestHandler::sendReply(const char* format, ...)
{
    char buf[256], fmt[256];
    va_list args;
    int i;

    snprintf(fmt, 256, "%s\r\n", format);

    va_start(args, format);
    i = vsnprintf(buf, 256, fmt, args);
    va_end(args);

    buf[i+1] = 0;
    DBG("Response: %s", buf);

    m_pCommandSocket->send(buf, i);
}

void FTPRequestHandler::sendData(const char* data, int length)
{
    if(m_pDataSocket)
    {
        m_pDataSocket->send(data, length);
    }
}


bool FTPRequestHandler::isAuthenticated()
{
    if(!m_authenticated)
    {
        sendReply(530, "Not logged in.");
    }
    
    return m_authenticated;
}

void FTPRequestHandler::sendSyntaxError()
{
    sendReply(501, "Syntax error in parameters");
}


void FTPRequestHandler::handleRequest()
{
    getRequest();
    DBG("Request: %s\r\n", request.c_str());

    vector<string> tokens = tokenize(request, " ");
    
    string& command = tokens[0];
    
    if(command.compare("USER") == 0)
    {
        if(tokens.size() == 2)
        {
            cmdUser(tokens[1]);
        }
        else
        {
            sendSyntaxError();
        }
    }
    else if(command.compare("PASS") == 0)
    {
        if(tokens.size() == 2)
        {
            cmdPass(tokens[1]);
        }
        else
        {
            sendSyntaxError();
        }
    }
    else if(command.compare("QUIT") == 0)
    {
        cmdQuit();
    }
    else if(command.compare("FEAT") == 0)
    {
        cmdFeat();
    }
    else if(command.compare("PORT") == 0)
    {
        if(tokens.size() == 2)
        {
            cmdPort(tokens[1]);
        }
        else
        {
            sendSyntaxError();
        }    
    }
    else if(command.compare("PASV") == 0)
    {
        cmdPasv();
    }
    else if(command.compare("TYPE") == 0)
    {
        if(tokens.size() == 2)
        {
            cmdType(tokens[1], "");
        }
        else if(tokens.size() == 3)
        {
            cmdType(tokens[1], tokens[2]);
        }        
        else
        {
            sendSyntaxError();
        }
        
    }
    else if(command.compare("PWD") == 0)
    {
        cmdPwd();
    }
    else if(command.compare("CWD") == 0)
    {
        if(tokens.size() == 2)
        {
            cmdCwd(tokens[1]);
        }
        else
        {
            sendSyntaxError();
        }
    }
    else if(command.compare("CDUP") == 0)
    {
        cmdCdup();
    }
    else if(command.compare("LIST") == 0)
    {
        cmdList();
    }
    else if(command.compare("MLSD") == 0)
    {
        cmdMlsd();
    }
    else
    {
        sendReply(202, "Command not implemented.");
    }
}


void FTPRequestHandler::handleResponse()
{   
    if(m_command == MLSD)
    {
        sendData(m_data_mlsd.c_str(), m_data_mlsd.length());
        closeDataSocket();
        m_data_mlsd = "";
    }
    else
    {
        
    }
}


void FTPRequestHandler::cmdUser(const string& username)
{
    m_user.setUsername(username);
    sendReply(331, "User name okay, need password.");
    //sendReply(530, "Not logged in.");
    //sendReply(230, "User logged in, proceed.");
}

void FTPRequestHandler::cmdPass(const string& password)
{
    FTPUser* user = m_pSvr->checkUser(m_user.getUsername(), password);
    
    if(user != NULL)
    {
        m_user = *user;
        m_authenticated = true;
        sendReply(230, "User logged in, proceed.");
    }
    else
    {
        sendReply(530, "Not logged in.");
        close();
    }
}
  
void FTPRequestHandler::cmdQuit()
{
    sendReply(221, "Service closing control connection.");
    close();
}

void FTPRequestHandler::cmdFeat()
{
    if(isAuthenticated())
    {
        //List extended features
        sendReply("211-Extensions supported:");
        sendReply(" MLSD");
        sendReply(211, "End");
    }
}

void FTPRequestHandler::cmdPort(const string& ip_port)
{
    if(isAuthenticated())
    {
        vector<string> ip_port_vector = tokenize(ip_port, ",");
        if(ip_port_vector.size() != 6)
        {
            sendSyntaxError();
            return;
        }
        
        if(m_pDataSocket)
        {
            sendReply(500, "Sorry, only one transfer at a time.");
            return;
        }
        
        uint8_t ip1 = atoi(ip_port_vector[0].c_str());
        uint8_t ip2 = atoi(ip_port_vector[1].c_str());
        uint8_t ip3 = atoi(ip_port_vector[2].c_str());
        uint8_t ip4 = atoi(ip_port_vector[3].c_str());
        uint8_t port1 = atoi(ip_port_vector[4].c_str());
        uint8_t port2 = atoi(ip_port_vector[5].c_str());
        int port = (port1 << 8) | port2;
    
        m_pDataClient = new Host(IpAddr(ip1, ip2, ip3, ip4), port);
        
        sendReply(200, "PORT Command okay.");   
    }
}

void FTPRequestHandler::cmdPasv()
{
    if(isAuthenticated())
    {
        if(m_pDataSocket)
        {
            sendReply(500, "Sorry, only one transfer at a time.");
            return;
        }
        
        int port = m_pSvr->getDataPort();
        const IpAddr& ip = m_pSvr->getIp();
        uint8_t port2 = (port >>  0) & 0xFF;
        uint8_t port1 = (port >>  8) & 0xFF;
        sendReply(227, "Entering passive mode (%u,%u,%u,%u,%u,%u)", ip[0], ip[1], ip[2], ip[3], port1, port2);
    }
}

void FTPRequestHandler::cmdType(const string& type, const string& arg)
{
    if(isAuthenticated())
    {
        sendReply(200, "TYPE ignored (always I)");
    }
}



void FTPRequestHandler::cmdPwd()
{
    if(isAuthenticated())
    {
        string workDir = fileSystem.getWorkingDirectory();
        sendReply(257, "\"%s\"", workDir.c_str());
    }
}

void FTPRequestHandler::cmdCwd(const string& dir)
{
    if(isAuthenticated())
    {
        if(fileSystem.changeWorkingDirectory(dir))
        {
            sendReply(250, "CWD successful.");
        }
        else
        {
            sendReply(550, "Directory not found.");
        }
    }
}

void FTPRequestHandler::cmdCdup()
{
    cmdCwd("..");
}

void FTPRequestHandler::cmdList()
{
    cmdMlsd();
}


void FTPRequestHandler::cmdMlsd()
{
    if(isAuthenticated())
    {
        m_command = MLSD;
        m_data_mlsd = "";
        
        m_data_mlsd += "Type=cdir; ";
        m_data_mlsd += fileSystem.getWorkingDirectory();
        m_data_mlsd += "\r\n";
        
        list<File> fileList = fileSystem.getFileList();
        for(list<File>::iterator it = fileList.begin(); it != fileList.end(); it++)
        {
            File& file = *it;
            
            if(file.getType() == TYPE_FILE)
            {
                m_data_mlsd += "Type=file;";
            }
            else if(file.getType() == TYPE_DIR)
            {
                m_data_mlsd += "Type=dir;";
            }
            
            m_data_mlsd += " ";
            m_data_mlsd += file.getName();
            m_data_mlsd += "\r\n";
        }
        
        DBG("\r\n%s", m_data_mlsd.c_str());
    }
}

