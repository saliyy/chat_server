#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>

#pragma warning(disable: 4996)


SOCKET Connections[100];
int Counter = 0;

void GetRequest(int index) {
	int msg_size;

	int msg_name_size;

	char* msg;

	std::string name_of_sender;
	while (true) {

		recv(Connections[index], (char*)&msg_size, sizeof(int), NULL); // получаем размер сообщения

		msg = new char[msg_size + 1]; // выделяем память 

		msg[msg_size] = '\0'; // по классике

		recv(Connections[index], msg, msg_size, NULL); // само сообщение

		name_of_sender = ("User: " + std::to_string(index) + " send: " + msg); // форматируем для других пользователей
        
		msg_name_size = name_of_sender.size();
		
		for (int i = 0; i < Counter; i++) {

			if (i == index) {

				continue;
			}

			send(Connections[i], (char*)&msg_name_size, sizeof(int), NULL); // размер
			send(Connections[i], name_of_sender.c_str(), msg_name_size, NULL); // само сообщение
		}
		delete[] msg; // очищаем от утечки памяти
	}
}

void UserDisconnect(int index)
{
	while (true)
	{
		int r = recv(Connections[index], NULL, 0, 0);
		if (r == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET) {
			std::cout << "client: " << index << "disconnect from server" << std::endl;
		}
	}

}



int main(int argc, char* argv[]) {

	
	// загржуаем нужную версию  .. 
	// создадим структуру

	WSAData wsaData;
	WORD DLLVersion = MAKEWORD(2, 1);
	// проверяем как загрузилась библиотека
	if (WSAStartup(DLLVersion, &wsaData) != 0) {
		std::cout << "Error" << std::endl;
		exit(1);
	}


	// собираем всю необходимую информацию для структуры
	SOCKADDR_IN addr;
	int sizeofaddr = sizeof(addr);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // local
	addr.sin_port = htons(1111); // port
	addr.sin_family = AF_INET; // семейство протоколов (TCP/UPD)

	// создаем слушателя
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr)); // связываем адрр с listenerom
	listen(sListen, SOMAXCONN); //  режим ожидания включен

	SOCKET newConnection; // сокет для работы с текущим клиентом
	for (int i = 0; i < 100; i++) {

		newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

		if (newConnection == 0) {
			std::cout << "Error #2\n";
		}
		else {
			std::cout << "Client: " << i << " connected to server" << std::endl;
			Connections[i] = newConnection;
			Counter++;
			
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)GetRequest, (LPVOID)(i), NULL, NULL); // поток для принятия сообщений
			CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)UserDisconnect, (LPVOID)(i), NULL, NULL); // поток для принятия сообщений
			
		}
	}

	system("pause");
	return 0;
}