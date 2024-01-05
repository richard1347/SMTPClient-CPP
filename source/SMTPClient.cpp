#ifdef WIN32
#include <WinSock2.h>
#endif
#include <fstream>
#include <sstream>
#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#define INVALID_SOCKET -1
#endif

#include "SMTPClient.h"

std::string join(vector<string>& vecData, const std::string& delim)
{
	if (vecData.size() <= 0)
	{
		return std::string();
	}
	std::stringstream ss;
	for (auto& item : vecData)
	{
		ss << delim << item;
	}

	return ss.str().substr(delim.length());
}

const char MIMETypes[][2][128] =
{
	{ "***",    "application/octet-stream" },
	{ "csv",    "text/csv" },
	{ "tsv",    "text/tab-separated-values" },
	{ "tab",    "text/tab-separated-values" },
	{ "html",    "text/html" },
	{ "htm",    "text/html" },
	{ "doc",    "application/msword" },
	{ "docx",    "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
	{ "ods",    "application/x-vnd.oasis.opendocument.spreadsheet" },
	{ "odt",    "application/vnd.oasis.opendocument.text" },
	{ "rtf",    "application/rtf" },
	{ "sxw",    "application/vnd.sun.xml.writer" },
	{ "txt",    "text/plain" },
	{ "xls",    "application/vnd.ms-excel" },
	{ "xlsx",    "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
	{ "pdf",    "application/pdf" },
	{ "ppt",    "application/vnd.ms-powerpoint" },
	{ "pps",    "application/vnd.ms-powerpoint" },
	{ "pptx",    "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
	{ "wmf",    "image/x-wmf" },
	{ "atom",    "application/atom+xml" },
	{ "xml",    "application/xml" },
	{ "json",    "application/json" },
	{ "js",    "application/javascript" },
	{ "ogg",    "application/ogg" },
	{ "ps",    "application/postscript" },
	{ "woff",    "application/x-woff" },
	{ "xhtml","application/xhtml+xml" },
	{ "xht",    "application/xhtml+xml" },
	{ "zip",    "application/zip" },
	{ "gz",    "application/x-gzip" },
	{ "rar",    "application/rar" },
	{ "rm",    "application/vnd.rn-realmedia" },
	{ "rmvb",    "application/vnd.rn-realmedia-vbr" },
	{ "swf",    "application/x-shockwave-flash" },
	{ "au",        "audio/basic" },
	{ "snd",    "audio/basic" },
	{ "mid",    "audio/mid" },
	{ "rmi",        "audio/mid" },
	{ "mp3",    "audio/mpeg" },
	{ "aif",    "audio/x-aiff" },
	{ "aifc",    "audio/x-aiff" },
	{ "aiff",    "audio/x-aiff" },
	{ "m3u",    "audio/x-mpegurl" },
	{ "ra",    "audio/vnd.rn-realaudio" },
	{ "ram",    "audio/vnd.rn-realaudio" },
	{ "wav",    "audio/x-wave" },
	{ "wma",    "audio/x-ms-wma" },
	{ "m4a",    "audio/x-m4a" },
	{ "bmp",    "image/bmp" },
	{ "gif",    "image/gif" },
	{ "jpe",    "image/jpeg" },
	{ "jpeg",    "image/jpeg" },
	{ "jpg",    "image/jpeg" },
	{ "jfif",    "image/jpeg" },
	{ "png",    "image/png" },
	{ "svg",    "image/svg+xml" },
	{ "tif",    "image/tiff" },
	{ "tiff",    "image/tiff" },
	{ "ico",    "image/vnd.microsoft.icon" },
	{ "css",    "text/css" },
	{ "bas",    "text/plain" },
	{ "c",        "text/plain" },
	{ "h",        "text/plain" },
	{ "rtx",    "text/richtext" },
	{ "mp2",    "video/mpeg" },
	{ "mpa",    "video/mpeg" },
	{ "mpe",    "video/mpeg" },
	{ "mpeg",    "video/mpeg" },
	{ "mpg",    "video/mpeg" },
	{ "mpv2",    "video/mpeg" },
	{ "mov",    "video/quicktime" },
	{ "qt",    "video/quicktime" },
	{ "lsf",    "video/x-la-asf" },
	{ "lsx",    "video/x-la-asf" },
	{ "asf",    "video/x-ms-asf" },
	{ "asr",    "video/x-ms-asf" },
	{ "asx",    "video/x-ms-asf" },
	{ "avi",    "video/x-msvideo" },
	{ "3gp",    "video/3gpp" },
	{ "3gpp",    "video/3gpp" },
	{ "3g2",    "video/3gpp2" },
	{ "movie","video/x-sgi-movie" },
	{ "mp4",    "video/mp4" },
	{ "wmv",    "video/x-ms-wmv" },
	{ "webm","video/webm" },
	{ "m4v",    "video/x-m4v" },
	{ "flv",    "video/x-flv" }
};


std::string fileBasename(const std::string path)
{
	std::string filename = path.substr(path.find_last_of("/\\") + 1);
	return filename;
}

std::string getFileContents(const char* filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);
	if (in.good())
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	throw(errno);
}

std::string GetFileExtension(const std::string& FileName)
{
	if (FileName.find_last_of(".") != std::string::npos)
		return FileName.substr(FileName.find_last_of(".") + 1);
	return "";
}

const char* GetMIMETypeFromFileName(char* szFileExt)
{
	for (unsigned int i = 0; i < sizeof(MIMETypes) / sizeof(MIMETypes[0]); i++)
	{
		if (strcmp(MIMETypes[i][0], szFileExt) == 0)
		{
			return MIMETypes[i][1];
		}
	}
	return MIMETypes[0][1];   //if does not match any,  "application/octet-stream" is returned
}

char* base64Encode(char const* sourceSigned, unsigned sourceLength)
{
	return nullptr;
}

string quotedprintableEncode(string sourceSigned, unsigned int sourceLength) {
	return "";
}

int SMTPEmail::SMTPCommunicate(const EmailInfo& info)
{
	char* buffer = new char[1000];
	memset(buffer, 0, 1000);

	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "220", 3) != 0) // not equal to 220
	{
		m_lastErrorMsg = buffer;
		cout << "Connetion error happened, server returns: " << buffer << endl;
		return -1;
	}

	//Send EHLO to servers
	std::string command = "EHLO test\r\n";
	Write(command.c_str(), command.length());

	memset(buffer, 0, 1000);
	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "250", 3) != 0) // ehlo failed
	{
		m_lastErrorMsg = buffer;
		cout << "EHLO failed! Server returns: " << buffer << endl;
		return -1;
	}

	//Login authentication: AUTH PLAIN method
	command = "AUTH PLAIN ";
	std::string auth = '\0' + info.senderEmail + '\0' + info.password;
	command += base64Encode(auth.data(), auth.size());
	command += "\r\n";
	Write(command.c_str(), command.length());

	memset(buffer, 0, 1000);
	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "235", 3) != 0) // login failed
	{
		m_lastErrorMsg = buffer;
		cout << "AUTH PLAIN failed! Server returns: " << buffer << endl;
		return -1;
	}

	/*
	//Login authentication: AUTH LOGIN method
	command = "AUTH LOGIN\r\n";
	Write(command.c_str(), command.length());
	memset(buffer, 0, 1000);
	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "334", 3) != 0) // login failed
	{
		m_lastErrorMsg = buffer;
		cout << "AUTH LOGIN failed! Server returns: " << buffer << endl;
		return -1;
	}
	command = base64Encode(info.senderEmail.c_str(), info.senderEmail.length());
	command += "\r\n";
	Write(command.c_str(), command.length());
	memset(buffer, 0, 1000);
	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "334", 3) != 0) // login failed
	{
		m_lastErrorMsg = buffer;
		cout << "Authentication failed! Server returns: " << buffer << endl;
		return -1;
	}
	command = base64Encode(info.password.c_str(), info.password.length());
	command += "\r\n";
	Write(command.c_str(), command.length());
	memset(buffer, 0, 1000);
	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "235", 3) != 0) // login failed
	{
		m_lastErrorMsg = buffer;
		cout << "Authentication failed! Server returns: " << buffer << endl;
		return -1;
	}
	*/

	//Configure senders
	command = "MAIL FROM:<" + info.senderEmail + ">\r\n";
	Write(command.c_str(), command.length());

	memset(buffer, 0, 1000);
	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "250", 3) != 0) // not ok
	{
		m_lastErrorMsg = buffer;
		cout << "MAIL FROM: failed! Server returns: " << buffer << endl;
		return -1;
	}

	//Configure receivers
	command = "RCPT TO:<" + info.recipientEmail + ">\r\n";
	Write(command.c_str(), command.length());

	memset(buffer, 0, 1000);
	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "250", 3) != 0) // not ok
	{
		m_lastErrorMsg = buffer;
		cout << "RCPT TO: failed! Server returns: " << buffer << endl;
		return -1;
	}

	//Ready to send
	command = "DATA\r\n";
	Write(command.c_str(), command.length());

	memset(buffer, 0, 1000);
	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "354", 3) != 0) // not ready to receive message
	{
		m_lastErrorMsg = buffer;
		cout << "DATA failed! Server returns: " << buffer << endl;
		return -1;
	}

	command = std::move(GetEmailBody(info));
	cout << command << endl;
	Write(command.c_str(), command.length());

	memset(buffer, 0, 1000);
	Read(buffer, 999);
	cout << buffer << endl;
	if (strncmp(buffer, "250", 3) != 0) // not ok
	{
		m_lastErrorMsg = buffer;
		cout << "Email body sent failed! Server returns: " << buffer << endl;
		return -1;
	}

	delete buffer;
	Write("QUIT\r\n", 6);

	Disconnect();
	return 0;
}

std::string SMTPEmail::GetEmailBody(const EmailInfo& info)
{
	std::ostringstream message;
	message << "From: \"" << info.sender << "\" <" << info.senderEmail << ">\r\n";

	std::vector<std::string> vecToList;
	for (auto item : info.recvList)
	{
		std::string to = "\"" + item.first + "\" <" + item.first + ">";
		vecToList.push_back(to);
	}

	message << "To: " << join(vecToList, ",") << "\r\n";
	message << "Subject: " << info.subject << "\r\n";
	message << "X-Priority: 3" << "\r\n";
	message << "X-Has-Attach: " << (info.attachment.size() ? "yes" : "no") << "\r\n";
	message << "X-Mailer: Mail 6.0" << "\r\n";
	message << "MIME-Version: 1.0\r\n";

	if (info.ccEmail.size() > 0)
	{
		std::vector<std::string> vecCcList;
		for (auto item : info.ccEmail)
		{
			std::string cc = "\"" + item.first + "\" <" + item.first + ">";
			vecCcList.push_back(cc);
		}
		message << "Cc:" << join(vecCcList, ",") << "\r\n";
	}

	message << "Content-Type: multipart/mixed; boundary=\"---eqouri98qqoiupoua8---\"\r\n\r\n";
	//message << "Content-Type: multipart/related; boundary=\"---eqouri98qqoiupoua8---\"\r\n\r\n";							//当邮件中有内嵌图片时，使用multipart/related结构
	message << "Content-Type: multipart/alternative; boundary=\"---eqouri98qqoiupoua8---\"\r\n\r\n";
	message << "Content-Type: " << "text/plain" << "; charset=\"" << info.charset << "\"\r\n";
	message << "Content-Transfer-Encoding: base64\r\n";
	message << base64Encode(info.message.c_str(), info.message.length());
	message << "\r\n\r\n";
	message << "\"---eqouri98qqoiupoua8---\"\r\n\r\n";
	message << "Content-Type: " << "text/html" << "; charset=\"" << info.charset << "\"\r\n";
	message << "Content-Transfer-Encoding: quoted-printable\r\n";
	message << R"(<html><head><meta http-equiv=3D"content-type" content=3D"text/html; charset = 3DGB2312"><style>body { line-height: 1.5; }body { font-size: 14px; font-family: "Microsoft YaHei UI"; color: rgb(0, 0, 0); line-height: 1.5;}</style></head><body>=0A<div><span></span>)";
	message << quotedprintableEncode(info.message, info.message.length());
	message << R"(</div>=0A<div><br></div><hr style=3D"width: 210px; height: 1px;" color=3D"#b5c4df" size=3D"1" align=3D"left">=0A<div><span><div style=3D"MARGIN: 10px; FONT-FAMILY: verdana; FONT-SIZE: 10pt"><div>)";
	message << info.sender;
	message << R"(</div></div></span></div>=0A</body></html>)";
	message << "\r\n\r\n";
	message << "\"---eqouri98qqoiupoua8---\"\r\n\r\n";

	//Attachment files are added and decoded one by one
	for (auto item : info.attachment)
	{
		std::string filename = fileBasename(item);
		std::string fileContent = getFileContents(item.c_str());
		std::string fileBase64Context = base64Encode(fileContent.c_str(), fileContent.length());
		std::string extension = GetFileExtension(filename);
		std::string mimetype = GetMIMETypeFromFileName((char*)extension.c_str());
		message << "Content-Type: " << mimetype << "; name=\"" << filename << "\"\r\n";
		message << "Content-Transfer-Encoding: base64\r\n";
		message << "Content-Disposition: attachment; filename=\"" << filename << "\"\r\n\r\n";
		message << fileBase64Context + "\r\n\r\n";
	}
	message << "\r\n.\r\n";
	return message.str();
}

SMTPEmail::SMTPEmail(const std::string& smtpserver, const std::string& port) :m_host(smtpserver), m_port(port)
{
	if (Connect() != 0) {
		cout << SMTPLastError() << endl;
		cout << "Do you want to test again?" << endl;
		exit(EXIT_FAILURE);
	}
	else {
		cout << "SMTP server <" << m_host << "> connected." << endl;
	}
}

SMTPEmail::~SMTPEmail()
{

}

int SMTPEmail::Read(void* buf, int num)
{
	return recv(m_socketfd, (char*)buf, num, 0);
}
int SMTPEmail::Write(const void* buf, int num)
{
	return send(m_socketfd, (char*)buf, num, 0);
}

int SMTPEmail::Connect()
{
#ifdef WIN32
	//start socket connection
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
#endif
	m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socketfd == INVALID_SOCKET)
	{
		m_lastErrorMsg = "Error on creating socket fd.";
		return -1;
	}

	addrinfo inAddrInfo = { 0 };
	inAddrInfo.ai_family = AF_INET;
	inAddrInfo.ai_socktype = SOCK_STREAM;

	int errorCode;
	if ((errorCode = getaddrinfo(m_host.c_str(), m_port.c_str(), &inAddrInfo, &m_addrinfo)) != 0) // error occurs
	{
		cout << "getaddrinfo() error code: " << errorCode << ", error string: " << gai_strerror(errorCode) << " system error code: " << GetLastError() << ", system WSA error code: " << WSAGetLastError() << endl;
		m_lastErrorMsg = "Error on calling getadrrinfo().";
		return -2;
	}
	char IPAddress[16];
	inet_ntop(m_addrinfo->ai_family, m_addrinfo->ai_addr, IPAddress, 16);
	cout << "SMTP server address family: " << m_addrinfo->ai_protocol << ", address: " << IPAddress << endl;

	if (errorCode = connect(m_socketfd, m_addrinfo->ai_addr, m_addrinfo->ai_addrlen))
	{
		cout << "connect() error code: " << errorCode << ", error string: " << gai_strerror(GetLastError()) << " system error code: " << GetLastError() << ", system WSA error code: " << WSAGetLastError() << endl;
		m_lastErrorMsg = "Error on calling connect().";
		return -3;
	}
	return 0;
}

int SMTPEmail::Disconnect()
{
	freeaddrinfo(m_addrinfo);
#ifdef WIN32
	closesocket(m_socketfd);
#else
	close(m_socketfd);
#endif
	return 0;
}

int SMTPEmail::SendEmail(const std::string& from, const std::string& passs, const std::string& to, const std::string& subject, const std::string& strMessage, const vector<string>& attachment)
{
	EmailInfo info;
	info.charset = "utf-8";
	info.sender = from;
	info.password = passs;
	info.senderEmail = from;
	info.recipientEmail = to;

	info.recvList[to] = "";

	info.subject = subject;
	info.message = strMessage;
	info.attachment = attachment;
	return SMTPCommunicate(info);
}

int SMTPEmail::SendEmail(const std::string& from, const std::string& passs, const std::vector<std::string>& vecTo,
	const std::string& subject, const std::string& strMessage, const std::vector<std::string>& attachment, const std::vector<std::string>& ccList)
{
	std::vector<std::string> recvList;
	recvList.insert(recvList.end(), vecTo.begin(), vecTo.end());
	recvList.insert(recvList.end(), ccList.begin(), ccList.end());

	for (auto& item : recvList)
	{
		EmailInfo info;
		info.charset = "UTF-8";
		info.sender = from;
		info.password = passs;
		info.senderEmail = from;;
		info.recipientEmail = item;

		for (auto item : vecTo)
		{
			info.recvList[item] = "";
		}

		info.subject = subject;
		info.message = strMessage;

		for (auto& item : ccList)
		{
			info.ccEmail[item] = item;
		}

		info.attachment = attachment;
		if (SMTPCommunicate(info) != 0)
		{
			return -1;
		}
	}
	return 0;
}

int main(int argc, char** argv) {
	string email, smtp_server, tmp, dest;
	cout << "Input the address of your email server to start (port defaults to 25), press ENTER to confirm:" << endl;
	cin >> email;
	cout << "Input the destination email addresses to send, press ENTER to confirm:" << endl;
	cin >> dest;
	if (!email.compare("localhost")) {
		smtp_server = email;
	}
	else {
		int count = 0;
		for (auto c : email) {
			if (c == '.')count++;
		}
		if (count == 3 && isdigit(*email.cbegin())) {							//input is IP address
			smtp_server = email;
		}
		else {
			tmp = email;
			smtp_server = "smtp." + tmp.substr(tmp.find('@') + 1);
		}
	}
	cout << "SMTP server is: " << smtp_server << endl;
	SMTPEmail testSMTP(smtp_server, "25");
	vector<string> attachments;
	testSMTP.SendEmail("you@sender.com", "password", dest, "Hello", "        ", attachments);
	return 0;
}