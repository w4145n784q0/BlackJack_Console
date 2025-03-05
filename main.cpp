#include <WS2tcpip.h>
#include <WinSock2.h>
#include<string>
#include<iostream>
#include <numeric>
#include <vector>
#include <ctime>
#include<limits>
#pragma comment( lib, "ws2_32.lib" )
using std::cout;
using std::endl;
using std::cin;
using std::string;
using std::vector;
std::numeric_limits;

// �|�[�g�ԍ�
const unsigned short SERVER_PORT = 8888;
// ����M���郁�b�Z�[�W�̍ő�l
const unsigned int MESSAGE_LENGTH = 1024;
// �T�[�o��IP�A�h���X
const char* SERVER_ADDRESS = "127.0.0.1";

const int connectPlayer = 3;


int main()
{
	//�v���C���[�̏��
	//id       : �v���C���[ID
	//MyCardNum: �����̎����Ă�J�[�h�̍��v�l
	//isHit    : hit���ǂ����@true�Ȃ�J�[�h�������i�b��j
	//isStand  : stand���ǂ����@�S��true�Ȃ珟�s������
	struct PLAYER
	{
		int id;//�v���C���[ID
		int MyCardNum;//�����̎����Ă�J�[�h�̍��v�l
		bool isHit; //hit���ǂ����@true�Ȃ�J�[�h�������i�b��j
		bool isStand;//stand���ǂ����@�S��true�Ȃ珟�s������
		//string name;//���O
	};
	PLAYER clientCard[connectPlayer];
	clientCard[0] = { 0,0,false,false };
	clientCard[1] = { 0,0,false,false };
	clientCard[2] = { 0,0,false,false };

	SOCKET clientSocks[connectPlayer];
	SOCKET listenSock;

	srand((unsigned int)time(NULL));

	//�f�B�[���[�̎�D
	vector<int> dealerCards;
	for (int i = 0; i < 2; i++) {
		dealerCards.push_back((rand() % 10) + 1);
	}
	int dealerCard = std::reduce(dealerCards.begin(), dealerCards.end(), 0);

	//�����̓T�[�o�[���N���C�A���g��
	bool IsServer;

	void InitServer(SOCKET sock);
	void InitClient(SOCKET sock);

	// WinSock2.2 ����������
	int ret = 0;
	WSADATA wsaData;
	ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		cout << "Winsock error" << endl;
		return 0;
	}
	cout << "Winsock success" << endl;

	// TCP���X���\�P�b�g�̍쐬
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
	{
		// �G���[����
		cout << "socket error" << endl;
		return 0;
	}
	cout << "socket success" << endl;

	// sock���m���u���b�L���O���[�h��
	u_long arg = 0x01;
	ret = ioctlsocket(listenSock, FIONBIO, &arg);

	if (ret == SOCKET_ERROR)
	{
		cout << "ioctlsocket error" << endl;
		return 0;
	}
	else
	{
		cout << "ioctlsocket success" << endl;
	}

	//�T�[�o�[���N���C�A���g�����߂�
	while (true) {
		int key = 0;
		cout << "1:�T�[�o�[�𗧂Ă�" << endl;
		cout << "2:�T�[�o�[�ɎQ������" << endl;

		cin >> key;
		if (key == 1) {
			IsServer = true;
			InitServer(listenSock);
			break;
		}
		else if (key == 2) {
			IsServer = false;
			InitClient(listenSock);
			break;
		}
		else {
			cout << "��蒼���Ă�������" << endl;
		}
	}

	//�ڑ���
	int clientCount = 0;

	//�T�[�o�̑ҋ@
	if (IsServer) {


		while (true) {
			char buff[MESSAGE_LENGTH];

			if (clientCount < connectPlayer)
			{
				// �ڑ��v����t��
				SOCKADDR_IN fromAddr;
				int fromlen = sizeof(fromAddr);
				SOCKET sock = accept(listenSock, (SOCKADDR*)&fromAddr, &fromlen);
				// �R�l�N�V�����m������
				if (sock != INVALID_SOCKET)
				{
					// �m���u���b�L���O�ɂ���
					u_long arg = 0x01;
					ioctlsocket(sock, FIONBIO, &arg);
					clientSocks[clientCount] = sock;
					clientCard[clientCount].id = clientCount;
					clientCard[clientCount].MyCardNum = 0;
					clientCard[clientCount].isHit = false;
					clientCard[clientCount].isStand = false;
					clientCount++;
				}
				else
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						// �ڑ��v���Ȃ�
						//cout << "waiting connect..." << endl;
					}
					else
					{
						// �G���[
						cout << "connect error" << endl;
						return 0;
					}
				}
			}

			// �R�l�N�V�����m���ς݂̑S�N���C�A���g����̎�M��
			for (int i = 0; i < clientCount; i++)
			{
				//��M�p
				PLAYER player;
				int ret = recv(clientSocks[i], (char*)&player, sizeof(player), 0);
				// ��M����������
				if (ret != SOCKET_ERROR)
				{
					// �o�C�g�I�[�_�[�ϊ�

					clientCard[i].id = ntohl(player.id);
					clientCard[i].MyCardNum = ntohl(player.MyCardNum);
					clientCard[i].isHit = ntohl(player.isHit);
					clientCard[i].isStand = ntohl(player.isStand);


				}
			}

			// ���M�f�[�^�̍쐬
			//CIRCLE sendPackets[3];
			PLAYER Packets[connectPlayer];

			for (int i = 0; i < connectPlayer; i++)
			{
				Packets[i].id = htonl(clientCard[i].id);
				Packets[i].MyCardNum = htonl(clientCard[i].MyCardNum);
				Packets[i].isHit = htonl(clientCard[i].isHit);
				Packets[i].isStand = htonl(clientCard[i].isStand);
			}

			// �R�l�N�V�����m���ς݂̑S�N���C�A���g�֑��M
			for (int i = 0; i < clientCount; i++)
			{
				//int ret = send(clientSocks[i], (char*)sendPackets, sizeof(sendPackets), 0);
				int ret = send(clientSocks[i], (char*)Packets, sizeof(Packets), 0);
				if (ret != SOCKET_ERROR)
				{
					// ���M����
					//cout << "send: Player" << i + 1 << endl;
					//getchar();
				}
				else
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						// �����M
					}
					else
					{
						// �G���[
					}
				}
			}

			if (clientCount == connectPlayer)
			{
				const char* message = "Server has 3 connected clients!";
				for (int i = 0; i < connectPlayer; i++)
				{
					send(clientSocks[i], message, strlen(message), 0);
				}
				//cout << " 3 connected clients!'" << endl;
			}

		}
	}

	//�N���C�A���g�̑ҋ@
	else if (!IsServer)
	{
		while (true) {
			char buff[MESSAGE_LENGTH];

			PLAYER player = { 0,0,false,false };
			PLAYER sendbuff = { htonl(player.id),htonl(player.MyCardNum),htonl(player.isHit),
						   htonl(player.isStand) };

			int ret = send(listenSock, (char*)&sendbuff, sizeof(sendbuff), 0);

			if (ret != SOCKET_ERROR)
			{
				// ���M�ł���
				cout << "���M�ł���" << endl;
			}
			else
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					// �����M
					cout << "�����M" << endl;
				}
				else
				{
					// �G���[
					cout << "���M�G���[" << endl;
				}
			}

			// �T�[�o�����M
			PLAYER recvPacket[connectPlayer];
			ret = recv(listenSock, (char*)recvPacket, sizeof(recvPacket), 0);

			int message = recv(listenSock, buff, sizeof(buff) - 1, 0);	// ��������strlen()�ɂ��Ȃ�
			if (message == SOCKET_ERROR)
			{
				// �G���[����
			}
			else
			{
				// �Ō�ɏI�[�L����\0������
				buff[message] = '\0';
			}
			std::string str = buff;
			if (str.compare("Server has 3 connected clients!") == 0)
			{
				std::cout << "connect" << std::endl;
				//getchar();
				break;
			}



			if (ret != SOCKET_ERROR)
			{
				for (int i = 0; i < connectPlayer; i++)
				{
					clientCard[i].id = ntohl(recvPacket[i].id);
					clientCard[i].MyCardNum = ntohl(recvPacket[i].MyCardNum);
					clientCard[i].isHit = ntohl(recvPacket[i].isHit);
					clientCard[i].isStand = ntohl(recvPacket[i].isStand);
				}
			}
			else
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					// ����M
					//cout << "����M" << endl;
				}
				else
				{
					//cout << "����M���G���[" << endl;
				}
			}
		}
	}
	//�ʐM�ڑ��I���

	//��������Q�[��

	//�T�[�o�[���̃Q�[������
	if (IsServer)
	{
		cout << "game start ���Ȃ��̓f�B�[���[�ł�" << endl;
		//�f�B�[���[�̎�D������
		vector<int> dealerCards_;
		for (int i = 0; i < 2; i++) {
			dealerCards_.push_back((rand() % 10) + 1);
		}
		int dealer = std::reduce(dealerCards_.begin(), dealerCards_.end(), 0);
		int Stand = 0;
		bool allStand[3] = { false,false,false };

		cout << "�N���C�A���g�ҋ@��";
		while (true) {
			for (int i = 0; i < connectPlayer; i++)
			{
				// �R�l�N�V�����m���ς݂̑S�N���C�A���g����̎�M��
				for (int i = 0; i < clientCount; i++)
				{
					//��M�p
					PLAYER player;
					int ret = recv(clientSocks[i], (char*)&player, sizeof(player), 0);
					// ��M����������
					if (ret != SOCKET_ERROR)
					{
						// �o�C�g�I�[�_�[�ϊ�
						clientCard[i].id = ntohl(player.id);
						clientCard[i].MyCardNum = ntohl(player.MyCardNum);
						clientCard[i].isHit = ntohl(player.isHit);
						clientCard[i].isStand = ntohl(player.isStand);

						if (clientCard[i].isStand)
						{
							allStand[i] = true;
							cout << "player" << i + 1 << "���X�^���h���܂���" << endl;
						}

					}
				}

				if (allStand[0] && allStand[1] && allStand[0])
				{
					cout << "�S��stand" << endl;
					break;
				}

			}
		}
	}

	//�N���C�A���g���̃Q�[��
	if (!IsServer)
	{
		PLAYER myData = { 0,0,false,false };

		cout << "game start ���Ȃ��̓v���C���[�ł�" << endl;
		int card = 0;
		vector<int> mycards = {};
		for (int i = 0; i < 2; i++) {
			card = (rand() % 10) + 1;
			mycards.push_back(card);
		}
		int mycardsNum = std::reduce(mycards.begin(), mycards.end(), 0);
		cout << "1���ڂ̃J�[�h: " << mycards[0] << endl;
		cout << "2���ڂ̃J�[�h: " << mycards[1] << endl;
		cout << "�J�[�h�̍��v: " << mycardsNum << endl;

		myData.MyCardNum = mycardsNum;

		while (true)
		{
			if (!myData.isStand)
			{
				int choice = -1;
				cout << "�q�b�g���܂����H (1:�q�b�g 2:�X�^���h) " << endl;
				cin >> choice;
				if (choice == 1) {
					card = (rand() % 10) + 1;
					mycards.push_back(card);
					mycardsNum = std::reduce(mycards.begin(), mycards.end(), 0);

					cout << "�V�����J�[�h: " << card << endl;
					cout << "�J�[�h�̍��v: " << mycardsNum << endl;
					myData.MyCardNum = mycardsNum;

				}
				else if (choice == 2) {
					myData.isStand = true;
					myData.MyCardNum = mycardsNum;

				}
				else if (choice != 1 && choice != 2) {
					cout << "1��2�����" << endl;
				}
				else
				{
					cout << "�G���[" << endl;
					break;
				}

				if (mycardsNum >= 22) {
					cout << "�o�[�X�g���܂���" << endl;
					myData.isStand = true;
				}

			}
			else
			{
				break;
			}
		}

		cout << "����" << endl;
		cout << "�f�B�[���[�̎�D:" << dealerCard << endl;
		cout << "���Ȃ��̎�D:" << myData.MyCardNum << endl;

		if (dealerCard > myData.MyCardNum) {
			cout << "�f�B�[���[�̏��� ";
		}
		else if (dealerCard < myData.MyCardNum) {
			cout << "���Ȃ��̏��� ";
		}
		else {
			cout << "�������� " << endl;
		}
		cout << "�Q�[�����I�����܂�" << endl;
	}

	closesocket(clientSocks[2]);
	closesocket(clientSocks[1]);
	closesocket(clientSocks[0]);
	closesocket(listenSock);
	WSACleanup();
	return 0;

}

void InitServer(SOCKET sock)
{
	// �Œ�A�h���X�̊��蓖��
	SOCKADDR_IN bindAddr;
	memset(&bindAddr, 0, sizeof(bindAddr));		// 0�N���A�ŏ�����
	bindAddr.sin_family = AF_INET;					// IPv4�A�h���X�g�p
	bindAddr.sin_port = htons(SERVER_PORT);			// �|�[�g�ԍ�10000�w��	�}�W�b�N�i���o�[��NG
	bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);	// ���ׂĂ̎�����NIC���Ώ�INADDR_ANY ����̂��̂����󂯕t����Ȃ璼��IP�A�h���X

	if (bind(sock, (SOCKADDR*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
	{
		cout << "bind error" << endl;
		return;
	}

	// ���X����Ԃ�
	if (listen(sock, connectPlayer) == SOCKET_ERROR)
	{
		// �G���[
		cout << "listen error" << endl;
		return;
	}
}

void InitClient(SOCKET sock)
{
	// �T�[�o�A�h���X�̎w��
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDRESS, &serverAddr.sin_addr.s_addr);
	//DrawString(0, 60, "�T�[�o�A�h���X�w�萬��", GetColor(255, 255, 255));

	if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			// �ڑ��v�����s
			cout << "�ڑ��v���Ɏ��s���܂���" << endl;
		}
	}

	cout << "�ڑ�����" << endl;
}