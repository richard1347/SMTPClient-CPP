#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <fstream>

#ifndef WIN32
#include<netdb.h>
#endif

using namespace std;

class SMTPBase
{
protected:
	struct EmailInfo
	{
		std::string smtpServer;      //SMTP server
		std::string serverPort;      //SMTP server port
		std::string charset;         //email character set
		std::string sender;          //sender's name
		std::string senderEmail;     //sender's email
		std::string password;        //password of sender
		std::string recipient;       //recipient's name
		std::string recipientEmail;  //recipient's email

		std::map<std::string, std::string> recvList; //recipient's list<email, name>

		std::string subject;         //the email message's subject
		std::string message;         //the email message body

		std::map<std::string, std::string> ccEmail;         //cc list
		std::vector<std::string> attachment;		//attachments
	};
public:
	virtual ~SMTPBase() {}
	virtual int SendEmail(const std::string& from, const std::string& passs, const std::string& to, const std::string& subject, const std::string& strMessage, const vector<string>& attachment) = 0;
	virtual int SendEmail(const std::string& from, const std::string& passs, const std::vector<std::string>& vecTo,
		const std::string& subject, const std::string& strMessage, const std::vector<std::string>& attachment, const std::vector<std::string>& ccList) = 0;
	std::string SMTPLastError()
	{
		return m_lastErrorMsg;
	}
	virtual int Read(void* buf, int num) = 0;
	virtual int Write(const void* buf, int num) = 0;
	virtual int Connect() = 0;
	virtual int Disconnect() = 0;
protected:
	std::string m_lastErrorMsg;
};


class SMTPEmail : public SMTPBase
{

public:
	SMTPEmail(const std::string& smtpserver, const std::string& port);
	~SMTPEmail();
	int SendEmail(const std::string& from, const std::string& passs, const std::string& to, const std::string& subject, const std::string& strMessage, const vector<string>& attachment);
	int SendEmail(const std::string& from, const std::string& passs, const std::vector<std::string>& vecTo,
		const std::string& subject, const std::string& strMessage, const std::vector<std::string>& attachment, const std::vector<std::string>& ccList);
protected:
	int Read(void* buf, int num);
	int Write(const void* buf, int num);
	int Connect();
	int Disconnect();
	virtual std::string GetEmailBody(const EmailInfo& info);
private:
	int SMTPCommunicate(const EmailInfo& info);
protected:
	addrinfo* m_addrinfo;
	int m_socketfd;
	std::string m_host;
	std::string m_port;
	bool m_isConnected;
};