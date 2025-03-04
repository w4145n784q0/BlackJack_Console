#include <WS2tcpip.h>
#include <WinSock2.h>
#include<string>
#include<iostream>
#include <numeric>
#include <vector>
#include <ctime>
#pragma comment( lib, "ws2_32.lib" )
using std::cout;
using std::endl;
using std::cin;
using std::string;
using std::vector;

// ポート番号
const unsigned short SERVER_PORT = 8888;
// 送受信するメッセージの最大値
const unsigned int MESSAGE_LENGTH = 1024;
// サーバのIPアドレス
const char* SERVER_ADDRESS = "192.168.43.5";

const int connectPlayer = 3;


int main()
{
	//プレイヤーの情報
	//id       : プレイヤーID
	//MyCardNum: 自分の持ってるカードの合計値
	//isHit    : hitかどうか　trueならカードを引く（暫定）
	//isStand  : standかどうか　全員trueなら勝敗処理へ
	//name     : プレイヤーの名前
	struct PLAYER
	{
		int id;//プレイヤーID
		int MyCardNum;//自分の持ってるカードの合計値
		bool isHit; //hitかどうか　trueならカードを引く（暫定）
		bool isStand;//standかどうか　全員trueなら勝敗処理へ
		string name;
	};
	PLAYER clientCard[connectPlayer];

	SOCKET clientSocks[connectPlayer];
	SOCKET listenSock;
	
	//自分はサーバーかクライアントか
	bool IsServer;

	void InitServer(SOCKET sock);
	void InitClient(SOCKET sock);
	srand((unsigned int)time(NULL));

	// WinSock2.2 初期化処理
	int ret = 0;
	WSADATA wsaData;
	ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret != 0)
	{
		cout << "Winsock error" << endl;
		return 0;
	}
	cout << "Winsock success" << endl;

	// TCPリスンソケットの作成
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
	{
		// エラー処理
		cout << "socket error" << endl;
		return 0;
	}
	cout << "socket success" << endl;

	// sockをノンブロッキングモードに
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

	//サーバーかクライアントか決める
	while (true) {
		int key = 0;
		cout << "1:サーバーを立てる" << endl;
		cout << "2:サーバーに参加する" << endl;

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
			cout << "やり直してください" << endl;
		}
	}

	//接続数
	int clientCount = 0;

	//サーバの待機
	if (IsServer) {

		
		while (true) {
			char buff[MESSAGE_LENGTH];

			if (clientCount < connectPlayer)
			{
				// 接続要求受付部
				SOCKADDR_IN fromAddr;
				int fromlen = sizeof(fromAddr);
				SOCKET sock = accept(listenSock, (SOCKADDR*)&fromAddr, &fromlen);
				// コネクション確率成功
				if (sock != INVALID_SOCKET)
				{
					// ノンブロッキングにして
					u_long arg = 0x01;
					ioctlsocket(sock, FIONBIO, &arg);
					clientSocks[clientCount] = sock;
					clientCard[clientCount].id = clientCount;
					clientCard[clientCount].MyCardNum = 0;
					clientCard[clientCount].isHit = false;
					clientCard[clientCount].isStand = false;
					clientCard[clientCount].name = "";
					clientCount++;
				}
				else
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						// 接続要求なし
						//cout << "waiting connect..." << endl;
					}
					else
					{
						// エラー
						cout << "connect error" << endl;
						return 0;
					}
				}
			}

			// コネクション確立済みの全クライアントからの受信部
			for (int i = 0; i < clientCount; i++)
			{
				//受信用
				PLAYER player;
				int ret = recv(clientSocks[i], (char*)&player, sizeof(player), 0);
				// 受信があったら
				if (ret != SOCKET_ERROR)
				{
					// バイトオーダー変換

					clientCard[i].id = ntohl(player.id);
					clientCard[i].MyCardNum = ntohl(player.MyCardNum);
					clientCard[i].isHit = ntohl(player.isHit);
					clientCard[i].isStand = ntohl(player.isStand);
					clientCard[i].name = player.name;
				}
			}

			// 送信データの作成
			//CIRCLE sendPackets[3];
			PLAYER Packets[connectPlayer];

			for (int i = 0; i < connectPlayer; i++)
			{
				Packets[i].id = htonl(clientCard[clientCount].id);
				Packets[i].MyCardNum = htonl(clientCard[clientCount].MyCardNum);
				Packets[i].isHit = htonl(clientCard[clientCount].isHit);
				Packets[i].isStand = htonl(clientCard[clientCount].isStand);
				Packets[i].name = clientCard[clientCount].name;
			}

			// コネクション確立済みの全クライアントへ送信
			for (int i = 0; i < clientCount; i++)
			{
				//int ret = send(clientSocks[i], (char*)sendPackets, sizeof(sendPackets), 0);
				int ret = send(clientSocks[i], (char*)Packets, sizeof(Packets), 0);
				if (ret != SOCKET_ERROR)
				{
					// 送信成功
					//cout << "send: Player" << i + 1 << endl;
					//getchar();
				}
				else
				{
					if (WSAGetLastError() == WSAEWOULDBLOCK)
					{
						// 未送信
					}
					else
					{
						// エラー
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
				cout << " 3 connected clients!'" << endl;
				break;
			}

		}
	}

	//クライアントの待機
	else if (!IsServer)
	{
		while (true) {
			char buff[MESSAGE_LENGTH];

			PLAYER player = { 0,0,false,false,""};
			PLAYER sendbuff = { htonl(player.id),htonl(player.MyCardNum),htonl(player.isHit),
						   htonl(player.isStand),player.name };

			int ret = send(listenSock, (char*)&sendbuff, sizeof(sendbuff), 0);

			if (ret != SOCKET_ERROR)
			{
				// 送信できた
				cout << "送信できた" << endl;
			}
			else
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					// 未送信
					cout << "未送信" << endl;
				}
				else
				{
					// エラー
					cout << "送信エラー" << endl;
				}
			}

			// サーバから受信
			PLAYER recvPacket[connectPlayer];
			ret = recv(listenSock, (char*)recvPacket, sizeof(recvPacket), 0);

			//ゲーム開始メッセージを受け取る用
			int message = recv(listenSock, buff, sizeof(buff) - 1, 0);	// こっちはstrlen()にしない
			if (message == SOCKET_ERROR)
			{
				// エラー処理
			}
			else
			{
				// 最後に終端記号の\0をつける
				buff[message] = '\0';
			}
			//std::cout << "受信した情報 :" << buff << std::endl;
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
					clientCard[i].name = recvPacket[i].name;
				}
			}
			else
			{
				if (WSAGetLastError() == WSAEWOULDBLOCK)
				{
					// 未受信
					//cout << "未受信" << endl;
				}
				else
				{
					//cout << "未受信かつエラー" << endl;
				}
			}

			

		}
	}
	//通信接続終わり

	//ここからゲーム

	//サーバー側のゲーム処理
	if (IsServer) 
	{
		char buff[MESSAGE_LENGTH];
		cout << "あなたはディーラーです" << endl;
		//ディーラーの手札を決定
		vector<int> dealerCards;
		for (int i = 0; i < 2; i++) {
			dealerCards.push_back((rand() % 10) + 1);
		}
		int dealer = std::reduce(dealerCards.begin(), dealerCards.end(), 0);
		int Stand = 0;
		bool allStand[3] = {false,false,false};

		cout << "クライアント待機中";

		while (true)
		{
			// コネクション確立済みの全クライアントからの受信部
			for (int i = 0; i < clientCount; i++)
			{
				//受信用
				//プレイヤー情報の構造体
				PLAYER player;
				int ret = recv(clientSocks[i], (char*)&player, sizeof(player), 0);

				//プレイヤーの名前
				//int name = recv(listenSock, buff, sizeof(buff) - 1, 0);

				// Playerからの受信があったら
				if (ret != SOCKET_ERROR)
				{
					// バイトオーダー変換
					clientCard[i].id = ntohl(player.id);
					clientCard[i].MyCardNum = ntohl(player.MyCardNum);
					clientCard[i].isHit = ntohl(player.isHit);
					clientCard[i].isStand = ntohl(player.isStand);
					clientCard[i].name = player.name;
					
					if (clientCard[i].isStand) {
						allStand[i] = true;
					}
				}

				//std::cout << "受信した情報 :" << buff << std::endl;

			}

			if (allStand[0] && allStand[1] && allStand[0])
			{
				break;
			}
	}
}

	//クライアント側のゲーム
	if (!IsServer)
	{
		PLAYER myData = {0,0,false,false,""};
		
		char Namebuff[MESSAGE_LENGTH];

		cout << "名前を入力してください" << endl;
		cin >> myData.name;

		cout << "ようこそ"<< myData.name << " " << " あなたはプレイヤーです" << endl;
		int card = 0;
		vector<int> mycards = {};
		for (int i = 0; i < 2; i++) {
			card = (rand() % 10) + 1;
			mycards.push_back(card);
		}
		int mycardsNum = std::reduce(mycards.begin(), mycards.end(), 0);
		cout << "1枚目のカード: " << mycards[0] << endl;
		cout << "2枚目のカード: " << mycards[1] << endl;
		cout << "カードの合計: " << mycardsNum << endl;

		//myData.isHit = 
		//myData.isStand = 
		myData.MyCardNum = mycardsNum;


		int choice = -1;
		cout << "ヒットしますか？ (1:ヒット 2:スタンド) " << endl;
		cin >> choice;
		if (choice == 1) {
			card = (rand() % 10) + 1;
			mycards.push_back(card);
			mycardsNum = std::reduce(mycards.begin(), mycards.end(), 0);

			cout << "新しいカード: " << card << endl;
			cout << "カードの合計: " << mycardsNum << endl;
			myData.MyCardNum = mycardsNum;
			
		}
		else if (choice == 2) {
			myData.isStand = true;
			
		}
		else {
			cout << "1か2を入力" << endl;
		}

		//データ送信
		PLAYER sendbuff = { htonl(myData.id),htonl(myData.isHit)//エラー
			,htonl(myData.isStand), htonl(myData.MyCardNum),myData.name};

		//1ターン目の情報を送る
		int sendData = send(listenSock, (char*)&sendbuff, sizeof(sendbuff), 0);
		//名前を送る
		//int sendName = send(listenSock, (char*)&Namebuff, strlen(Namebuff), 0);
		
		

		//一旦終わる
		

		
	}
}

void InitServer(SOCKET sock)
{
	// 固定アドレスの割り当て
	SOCKADDR_IN bindAddr;
	memset(&bindAddr, 0, sizeof(bindAddr));		// 0クリアで初期化
	bindAddr.sin_family = AF_INET;					// IPv4アドレス使用
	bindAddr.sin_port = htons(SERVER_PORT);			// ポート番号10000指定	マジックナンバーはNG
	bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);	// すべての自分のNICが対象INADDR_ANY 特定のものだけ受け付けるなら直接IPアドレス

	if (bind(sock, (SOCKADDR*)&bindAddr, sizeof(bindAddr)) == SOCKET_ERROR)
	{
		cout << "bind error" << endl;
		return;
	}

	// リスン状態へ
	if (listen(sock, connectPlayer) == SOCKET_ERROR)
	{
		// エラー
		cout << "listen error" << endl;
		return;
	}
}

void InitClient(SOCKET sock)
{
	// サーバアドレスの指定
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDRESS, &serverAddr.sin_addr.s_addr);
	//DrawString(0, 60, "サーバアドレス指定成功", GetColor(255, 255, 255));

	if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			// 接続要求失敗
			cout << "接続要求に失敗しました" << endl;
		}
	}

	cout << "接続成功" << endl;
}

void Dealer()
{
	
}

void Player()
{

}