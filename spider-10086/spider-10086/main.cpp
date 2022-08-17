#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <regex>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")


using namespace std;
//ȫ�ֱ���

string g_host;		//����
string g_object;	//��Դ·��
SOCKET g_socket;	//�ͻ����׽���

//����

//��ʼץȡ
bool StartCatch(string url);
//����URL
bool AnalyseURL(string url);
//��������
bool Connect();
//������ҳ
bool GetHtml(string url, string& html);
//����ͼƬ
bool Download(string url, string filename);

//������
int main()
{
	cout << "****************************************************" << endl;
	cout << "*           ��ӭʹ�ó����޵�����ϵͳ               *" << endl;
	cout << "*           �����ߣ�����ţ��                       *" << endl;
	cout << "*           ��ǰ�汾��1.0086                       *" << endl;
	cout << "****************************************************" << endl;
	cout << "������Ҫץȡ��ͼƬURL���ӣ�" << endl;

	//�洢��ʼURL
	string startURL;
	cin >> startURL;

	//����һ���ļ���
	CreateDirectory(L"images", NULL);
	CreateDirectory(L"videos", NULL);

	//��ʼץȡ
	StartCatch(startURL);

	return 0;
}


//��ʼץȡ

bool StartCatch(string url)
{
	queue<string> q;
	q.push(url);

	while (!q.empty())
	{
		string currentUrl = q.front();
		q.pop();
		string html;
		if (false == GetHtml(currentUrl, html))
		{
			continue;
		}
		vector<string> vImageURL;
		smatch mat;
		regex reg("http://[^\\s'\"<>()]+");
		string::const_iterator start = html.begin();
		string::const_iterator end = html.end();
		while (regex_search(start, end, mat, reg))
		{
			string per(mat[0].first, mat[0].second);
			vImageURL.push_back(per);

			start = mat[0].second;
		}

		for (int i = 0; i < vImageURL.size(); i++)
		{
			string imageUrl = vImageURL[i];

			if (imageUrl.find(".jpg") != string::npos || imageUrl.find(".png") != string::npos)
			{
				string filename = "./images/" + imageUrl.substr(imageUrl.find_last_of('/') + 1);
				if (false == Download(imageUrl, filename))
				{
					cout << "����ʧ��" << endl;
				}
				else
				{
					cout << "���سɹ�" << endl;
				}
			}
			else
			{
				q.push(imageUrl);
			}
		}

	}
	return true;
}

//����URL

bool AnalyseURL(string url)
{
	string newurl;
	for (int i = 0; i < url.length(); i++)
	{
		newurl += tolower(url[i]);
	}

	if (string::npos == newurl.find("http://"))
		return false;

	if (newurl.length() <= 7)
		return false;

	int pos = newurl.find('/', 7);
	if (string::npos == pos)
	{
		g_host = newurl.substr(7);
		g_object = "/";
	}
	else
	{
		g_host = newurl.substr(7, pos - 7);
		g_object = newurl.substr(pos);
	}

	if (g_host.empty())
		return false;

	return true;
}

//��������

bool Connect()
{
	WSADATA wd;
	WSAStartup(MAKEWORD(2, 2), &wd);
	g_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (g_socket == INVALID_SOCKET)
		return false;
	hostent* host = gethostbyname(g_host.c_str());
	if (host == NULL)
		return false;
	sockaddr_in sa;
	sa.sin_family = AF_INET;
	memcpy(&sa.sin_addr, host->h_addr, 4);
	sa.sin_port = htons(80);

	if (SOCKET_ERROR == connect(g_socket, (sockaddr*)&sa, sizeof(sa)))
		return false;
	return true;
}

//������ҳ

bool GetHtml(string url, string& html)
{
	if (false == AnalyseURL(url))
		return false;
	if (false == Connect())
		return false;
	string info;
	info = info + "GET " + g_object + " HTTP/1.1\r\n";
	info = info + "Host: " + g_host + "\r\n";
	info = info + "Connection: Close\r\n";
	info = info + "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36\r\n";
	info = info + "\r\n";
	if (SOCKET_ERROR == send(g_socket, info.c_str(), info.length(), 0))
		return false;
	char ch = 0;
	int nRealSize = 0;
	while (recv(g_socket, &ch, sizeof(ch), 0))
	{
		html = html + ch;
	}
	closesocket(g_socket);
	WSACleanup();

	return true;
}

//����ͼƬ

bool Download(string url, string filename)
{
	if (false == AnalyseURL(url))
		return false;
	if (false == Connect())
		return false;
	string info;
	info = info + "GET " + g_object + " HTTP/1.1\r\n";
	info = info + "Host: " + g_host + "\r\n";
	info = info + "Connection: Close\r\n";
	info = info + "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36\r\n";
	info = info + "\r\n";

	if (SOCKET_ERROR == send(g_socket, info.c_str(), info.length(), 0))
		return false;
	char ch = 0;
	int nRealSize = 0;	
	int nRecvNum = 0;
	while ((nRecvNum = recv(g_socket, &ch, sizeof(ch), 0)) > 0)
	{
		if (ch == '\r')
		{
			nRecvNum = recv(g_socket, &ch, sizeof(ch), 0);
			if (nRecvNum > 0 && ch == '\n')
			{
				nRecvNum = recv(g_socket, &ch, sizeof(ch), 0);
				if (nRecvNum > 0 && ch == '\r')
				{
					nRecvNum = recv(g_socket, &ch, sizeof(ch), 0);
					if (nRecvNum > 0 && ch == '\n')
					{
						break;
					}
				}
			}
		}
	}
	FILE* fp = fopen(filename.c_str(), "wb");
	if (fp == NULL)
		return false;
	ch = 0;
	nRealSize = 0;	
	nRecvNum = 0;
	while ((nRecvNum = recv(g_socket, &ch, sizeof(ch), 0)) > 0)
	{
		fwrite(&ch, 1, nRecvNum, fp);
	}
	fclose(fp);
	closesocket(g_socket);
	WSACleanup();

	return true;

	return true;
}
