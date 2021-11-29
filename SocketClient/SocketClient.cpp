// lab1_Client.cpp : ���� ���� �������� ������� "main". ����� ���������� � ������������� ���������� ���������.
//

#include "pch.h"
#include "framework.h"
#include "Message.h"
#include <mutex>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// ������������ ������ ����������

CWinApp theApp;

int ClientId;
mutex hMutex;
bool connection;

void canalStart(CSocket& hS) {
    hS.Create();
    hS.Connect(_T("127.0.0.1"), 12345);
}

void canalStop(CSocket& hS) {
    hS.Close();
}

void getData() {
    while (true) {
        CSocket client;
        canalStart(client);
        Message m;
        Message::SendMessage(client, 0, ClientId, M_GETDATA);
        MsgHeader h_message;
        h_message = m.Receive(client);
        canalStop(client);
        if (h_message.m_Type == M_TEXT) 
        {
            hMutex.lock();
            cout << "Message from client " << m.getM_Header().m_From << ": " << m.getM_Data() << endl;
            hMutex.unlock();
        }

        if (h_message.m_Type == M_EXIT1) 
        {
            hMutex.lock();
            cout << "You have been disconnected due to long inactivity" << endl;
            hMutex.unlock();
            connection = false;
            cout << "Want to connect to a server again? (0/1)" << endl;
        }

        Sleep(1000);
    }
}

void ConnectToServer(Message& m, MsgHeader& h_message, CSocket& client) {

    AfxSocketInit();
    canalStart(client);
    Message::SendMessage(client, 0, 0, M_INIT);
    h_message = m.Receive(client);
    canalStop(client);

    if (h_message.m_Type == M_CONFIRM) {
        ClientId = h_message.m_To;
        hMutex.lock();
        cout << "Your ID is " << ClientId << endl;
        hMutex.unlock();
        thread t(getData);
        t.detach();
        connection = true;
    }
    else {
        cout << "ERROR::CLIENT NOT CONNECTED" << endl;
        return;
    }
}

void start() {
    cout << "Want to connect to a server? (0/1)" << endl;
    int answer;
    cin >> answer;
    if (answer == 1)
        connection = true;
    if (answer == 0)
        return;
    MsgHeader h_message;
    CSocket client;
    Message m;
    ConnectToServer(m, h_message, client);
    while (connection) {

        cout << "Press 0 for send Message \n Press 1 for Exit" << endl;
        cin >> answer;

        switch (answer)
        {
        case 0: {
            if (connection) {
                string str;
                cout << "Enter ID of client" << endl;
                int ID;
                cin >> ID;
                cout << "Enter your Message" << endl;
                cin.ignore();
                getline(cin, str);
                canalStart(client);
                if (ID == 100) {
                    Message::SendMessage(client, A_ALL, ClientId, M_TEXT, str);
                }
                else {
                    Message::SendMessage(client, ID, ClientId, M_TEXT, str);
                }

                h_message = m.Receive(client);
                canalStop(client);
                hMutex.lock();
                if (h_message.m_Type == M_CONFIRM) {
                    cout << "SUCCESS::MESSAGE WAS SEND" << endl;
                }
                else {
                    cout << "ERROR::MESSAGE WAS NOT SEND" << endl;
                }
                hMutex.unlock();
                break;
            }
            else {
                return;
            }

        }

        case 1: {
            if (connection) {
                canalStart(client);
                Message::SendMessage(client, 0, ClientId, M_EXIT0);
                h_message = m.Receive(client);
                hMutex.lock();
                if (h_message.m_Type == M_CONFIRM)
                {
                    cout << "SUCCESS!" << endl;
                }
                else cout << "ERROR" << endl;
                hMutex.unlock();
                connection = false;
                return;
            }
            else {
                connection = true;
                ConnectToServer(m, h_message, client);
            }
            break;
        }
        default:
            cout << "ERROR::PLEASE, PRESS 0 OR 1" << endl;
            break;
        }
    }

}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // ���������������� MFC, � ����� ������ � ��������� �� ������� ��� ����
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: �������� ���� ��� ��� ����������.
            wprintf(L"����������� ������: ���� ��� ������������� MFC\n");
            nRetCode = 1;
        }
        else
        {
            HWND hwnd = GetConsoleWindow();
            HMENU hmenu = GetSystemMenu(hwnd, FALSE);
            EnableMenuItem(hmenu, SC_CLOSE, MF_GRAYED);
            start();
            // TODO: �������� ���� ��� ��� ����������.
        }
    }
    else
    {
        // TODO: �������� ��� ������ � ������������ � �������������
        wprintf(L"����������� ������: ���� GetModuleHandle\n");
        nRetCode = 1;
    }

    return nRetCode;
}
