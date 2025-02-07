#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <sstream>
#include <signal.h>
#include <cassert>
#include <zmq.hpp>
#include <chrono>
#include <thread>
#include <exception>
#include <map>
#include <cstdlib>
#include <ctime>


using namespace std::chrono_literals;

const int TIMER = 2000;
const int SMOL_TIMER = 500;
int n = 2;

const int BOARD_SIZE = 10;

zmq::context_t context(3);
zmq::socket_t main_socket(context, ZMQ_REP);



// общая функция для отправки сообщения в дочерний процесс
bool send_message(zmq::socket_t &socket, const std::string &message_string) {
    zmq::message_t message(message_string.size());
    memcpy(message.data(), message_string.c_str(), message_string.size());
    return socket.send(message); 
}


std::string receive_message(zmq::socket_t &socket) {
    zmq::message_t message;
    bool ok = false;
    try {
        ok = socket.recv(&message);
    }
    catch (...) {
        ok = false;
    }
    std::string recieved_message(static_cast<char*>(message.data()), message.size());
    if (recieved_message.empty() || !ok) {
        return "Root is dead";
    }
    return recieved_message;
}

// меняем созданый fork процесс на дочерний, передавая туда нужные нам аргументы (клиент)
void create_node(int id, int port) {
    char* arg0 = strdup("./client");
    char* arg1 = strdup((std::to_string(id)).c_str());
    char* arg2 = strdup((std::to_string(port)).c_str());
    char* args[] = {arg0, arg1, arg2, NULL};
    execv("./client", args);
}

// функция, собирающая полный адрес до дочернего процесса
std::string get_port_name(const int port) {
    return "tcp://127.0.0.1:" + std::to_string(port);
}

bool is_number(std::string val) {
    try {
        int tmp = std::stoi(val);
        return true;
    }
    catch(std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return false;
    }
}


class Player {
public:
    std::vector< std::vector<char> > board;
	int num;

    Player() {
        board = std::vector< std::vector<char> >(BOARD_SIZE, std::vector<char>(BOARD_SIZE, ' '));
    }

    void placeShips(zmq::socket_t &player_socket) {
		// Расставляем корабли
		std::string msg;
		std::string tmp;
		// msg = "Расставьте ваши корабли (формат: x, y и ориентация (H или V) через пробелы):\n";
		// std::cout << "Расставьте ваши корабли (формат: x, y и ориентация (H или V) через пробелы):" << std::endl;
		int ships_count = 5;
		for (int i = 1; i <= 1; ++i) {
			ships_count -= 1;
			for (int j = 0; j < ships_count; j++) {
				char orientation;
				int x, y;
				msg = "Разместите корабль " + std::to_string(i) + " (1x" + std::to_string(i) + "): ";
				// std::cout << msg << std::endl;
				// std::cout << "Разместите корабль " << i << " (1x" << i << "): ";
				send_message(player_socket, msg);
				std::string received_message = receive_message(player_socket);
				std::cout << "Получил запрос: " + received_message << std::endl;

				std::stringstream strs(received_message);
				std::getline(strs, tmp, ':');
				if (tmp == "coords") {
					std::getline(strs, tmp, ':');
					x = std::stoi(tmp);
					std::getline(strs, tmp, ':');
					y = std::stoi(tmp);
					std::getline(strs, tmp, ':');
					orientation = tmp[0];

					// std::cin >> y >> x >> orientation;
					if (!((orientation == 'V') or (orientation == 'H'))) {
						// std::cout << "Ты по-моему перепутал" << std::endl;
						send_message(player_socket, "Error : Ты по-моему перепутал");
						received_message = receive_message(player_socket);
						--j;
						continue;
					}
					if (isValidPlacement(x, y, i, orientation)) {
						placeShip(x, y, i, orientation);
					} else {
						send_message(player_socket, "Error : Неверное местоположение! Попробуйте еще раз.");
						received_message = receive_message(player_socket);
						// std::cout << "Неверное местоположение! Попробуйте еще раз." << std::endl;
						--j;
						continue;
					}

					std::string boardState = "board";
					boardState += getBoard();
					// std::cout << boardState.substr(5, boardState.size()) << std::endl;
					send_message(player_socket, boardState);
					received_message = receive_message(player_socket);
				}
			}
		}
    }

    bool isValidPlacement(int x, int y, int size, char orientation) const {
		if ((x > 9) or (x < 0) or (y > 9) or (y < 0)) {
			return false;
		}
        if (orientation == 'V') {
            if (x + size - 1 >= BOARD_SIZE) {
                return false;
            }

            for (int i = x; i < x + size; ++i) {
                if (board[i][y] != ' ') {
                    return false;
                }
            }
        } 
		else if (orientation == 'H') {
            if (y + size - 1 >= BOARD_SIZE) {
                return false;
            }

            for (int j = y; j < y + size; ++j) {
                if (board[x][j] != ' ') {
                    return false;
                }
            }
        }
		for (int i = 0; i < size; i++) {
			if (orientation == 'H') {
				if (!isEmptyAround(x + i, y)) {
					return false;
				}
			}
			else {
				if (!isEmptyAround(x, y + i)) {
					return false;
				}
			}
		}

        return true;
    }

	bool isEmptyAround(int x, int y) const {
		for (int i = x - 1; i <= x + 1; ++i) {
			for (int j = y - 1; j <= y + 1; ++j) {
				if (i >= 0 && i < BOARD_SIZE && j >= 0 && j < BOARD_SIZE && board[i][j] != ' ') {
					return false;
				}
			}
		}
		return true;
	}

    void placeShip(int x, int y, int size, char orientation) {
        if (orientation == 'V') {
            for (int i = x; i < x + size; ++i) {
                board[i][y] = 'O';
            }
        } else if (orientation == 'H') {
            for (int j = y; j < y + size; ++j) {
                board[x][j] = 'O';
            }
        }
    }

    std::string getBoard() const {
		std::string result;
		std::string probel(1, ' ');
        result = "  0 1 2 3 4 5 6 7 8 9\n";
        for (int i = 0; i < BOARD_SIZE; ++i) {
            result += std::to_string(i) + " ";
            for (int j = 0; j < BOARD_SIZE; ++j) {
				std::string brd(1, board[i][j]);
                result += brd + probel;
            }
            result += '\n';
        }
        result += '\n';
		return result;
    }

    std::string getClearBoard() const {
		std::string result;
		std::string space(1, ' ');
        result = "  0 1 2 3 4 5 6 7 8 9\n";
        for (int i = 0; i < BOARD_SIZE; ++i) {
            result += std::to_string(i) + " ";
            for (int j = 0; j < BOARD_SIZE; ++j) {
				std::string brd(1, board[i][j]);
				if (brd == "O") {
                	result += space + space;	
				}
				else {
					result += brd + space;
				}
            }
            result += '\n';
        }
        result += '\n';
		return result;
    }
};

class Game {
public:
    Player player1;
	Player player2;
	

    void play(zmq::socket_t &player1_socket, zmq::socket_t &player2_socket) {
        std::cout << "Игра \"Морской бой\" началась!" << std::endl;

		std::string msg1;
		std::string msg2;

        // Расставляем корабли для каждого игрока
		// pthread_t tid[2];
		// pthread_create(&tid[0], NULL, player_placement, &player1);
		// pthread_create(&tid[1], NULL, player_placement, &player2);

		// pthread_join(tid[0], NULL);
		// pthread_join(tid[1], NULL);
		player1.num = 1;
		player2.num = 2;
        player1.placeShips(player1_socket);
        player2.placeShips(player2_socket);

        // Начинаем игру
		int turn = 0;
        while (!gameOver()) {
			if (turn % 2 == 0) {
				msg1 = "your_turn";
				msg2 = "not_your_turn";
				send_message(player1_socket, msg1);
				receive_message(player1_socket);
				send_message(player2_socket, msg2);
				receive_message(player2_socket);
				std::cout << "Ход игрока 1:" << std::endl;
				if (playerTurn(player1, player2, player1_socket, player2_socket)) {
					if (gameOver())  {
						std::cout << "Победил игрок 1" << std::endl;
						send_message(player1_socket, "win");
						receive_message(player1_socket);
						send_message(player2_socket, "lose");
						receive_message(player2_socket);
						break;
					}
					continue;
				}
				else {
					turn += 1;
				}
			}
			else {
				std::cout << "Ход игрока 2:" << std::endl;
				msg1 = "your_turn";
				msg2 = "not_your_turn";
				send_message(player2_socket, msg1);
				receive_message(player2_socket);
				send_message(player1_socket, msg2);
				receive_message(player1_socket);
				if (playerTurn(player2, player1, player2_socket, player1_socket)) {
					if (gameOver()) {
						std::cout << "Победил игрок 2" << std::endl;
						send_message(player2_socket, "win");
						receive_message(player2_socket);
						send_message(player1_socket, "lose");
						receive_message(player1_socket);
						break;
					}
					continue;
				}
				else {
					turn += 1;
				}
			}
        }
        std::cout << "Игра завершена!" << std::endl;
    }

    bool gameOver() const {
        return allShipsSunk(player1) || allShipsSunk(player2);
    }

    bool allShipsSunk(const Player& player) const {
        for (const auto& row : player.board) {
            for (char cell : row) {
                if (cell == 'O') {
                    return false;
                }
            }
        }
        return true;
    }


    bool playerTurn(Player& attacker, Player& defender, zmq::socket_t &attacker_socket, zmq::socket_t &defender_socket) {
		bool shoot = false;
		std::string received_message;
		std::string tmp;
        int x, y;
		send_message(attacker_socket, "shoot");
		received_message = receive_message(attacker_socket);
		std::stringstream strs(received_message);
		std::getline(strs, tmp, ':');
		std::getline(strs, tmp, ':');
		x = std::stoi(tmp);
		std::getline(strs, tmp, ':');
		y = std::stoi(tmp);

        if (isValidMove(x, y, defender)) {
            if (defender.board[x][y] == 'O') {
				send_message(attacker_socket, "shooted");
				received_message = receive_message(attacker_socket);
				send_message(defender_socket, "shooted");
				received_message = receive_message(defender_socket);

                std::cout << "Попадание!" << std::endl;
                defender.board[x][y] = 'X';
				shoot = true;
				// std::cout << defender.getBoard();
				return shoot;
            } else {
				send_message(attacker_socket, "miss");
				received_message = receive_message(attacker_socket);
				send_message(defender_socket, "miss");
				received_message = receive_message(defender_socket);

                std::cout << "Промах!" << std::endl;
                defender.board[x][y] = '*';
				std::string board = defender.getBoard();
				std::string clearBoard = defender.getClearBoard();
				
				send_message(attacker_socket, "board" + clearBoard);
				received_message = receive_message(attacker_socket);
				send_message(defender_socket, "board" + board);
				received_message = receive_message(defender_socket);
				return shoot;
            }

        } else {
            std::cout << "Неверные координаты! Попробуйте еще раз." << std::endl;
            return playerTurn(attacker, defender, attacker_socket, defender_socket);
        }
    }

    bool isValidMove(int x, int y, const Player& defender) const {
        return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE &&
               (defender.board[x][y] == ' ' || defender.board[x][y] == 'O');
    }
};

// void* player_placement(void *param) {
// 	Player *player = (Player*) param;
//     player->placeShips();
// 	pthread_exit(0);
// }




int main() {
    // zmq::context_t context(3);
    // zmq::socket_t main_socket(context, ZMQ_REP);
    zmq::socket_t  first_player_socket(context, ZMQ_REQ);
    zmq::socket_t second_player_socket(context, ZMQ_REQ);
    main_socket.bind("tcp://*:5555");
    first_player_socket.bind("tcp://*:5556");
    second_player_socket.bind("tcp://*:5557");
    std::cout << "Сервер начал работу" << std::endl;
    std::map<int, std::string> login_map;
    int port_iter = 1;
    while (true) {
        std::string received_message = receive_message(main_socket);
        std::cout << "На сервер поступил запрос: '" + received_message + "'" << std::endl;
        std::stringstream ss(received_message);
        std::string tmp;
        std::getline(ss, tmp, ':');
        if (tmp == "login") {
            if (port_iter > 2) {
                send_message(main_socket, "Error:TwoPlayersAlreadyExist");
            }
            else {
                std::getline(ss, tmp, ':');
                std::getline(ss, tmp, ':');
				if (login_map[1] == tmp) {
					std::cout << "Игрок ввел занятое имя" << std::endl;
					send_message(main_socket, "Error:NameAlreadyExist");
				}
				else {
					std::cout << "Логин игрока номер " + std::to_string(port_iter) + ": " + tmp << std::endl;
					std::string login = tmp;
					login_map[port_iter] = login;
					
					send_message(main_socket, "Ok:" + std::to_string(port_iter));
					port_iter += 1;
				}
            }
        }
        // получили от клиента запрос на игру другому клиенту
        else if (tmp == "invite") {
			std::cout << "Обрабатываю инвайт" << std::endl;
			std::this_thread::sleep_for(100ms);
            std::string invite_login;
            std::getline(ss, tmp, ':');
            int sender_id = std::stoi(tmp);
            std::getline(ss, invite_login, ':');
            
            // если клиент 1 отправил запрос клиенту 2
			if (invite_login == login_map[sender_id]) {
				std::cout << "Игрок пригласил сам себя" << std::endl;
				send_message(main_socket, "Error:SelfInvite");
			}
            else if (invite_login == login_map[2]) {
				std::cout << "Игрок " + login_map[1] + " пригласил в игру " + login_map[2] << std::endl;
                send_message(second_player_socket, "invite:" + login_map[1]);
				std::string invite_message = receive_message(second_player_socket);

				second_player_socket.set(zmq::sockopt::rcvtimeo, -1);
                if (invite_message == "accept") {
					std::cout << "Игрок " + login_map[2] + " принял запрос " << std::endl;
                    send_message(main_socket, invite_message);
                    break;
                }
                else if (invite_message == "reject") {
					std::cout << "Игрок " + login_map[2] + " отклонил запрос " << std::endl;
                    send_message(main_socket, invite_message);
                }
                else {
                    std::cout << "Что-то пошло не так во время обработки запроса на игру" << std::endl;
                }

            }
            // если клиент 2 отправил запрос клиенту 1
            else if (invite_login == login_map[1]){
				std::cout << "Игрок " + login_map[2] + " пригласил в игру " + login_map[1] << std::endl;
                send_message(first_player_socket, "invite:" + login_map[2]);
				std::string invite_message = receive_message(first_player_socket);

                if (invite_message == "accept") {
					std::cout << "Игрок " + login_map[1] + " принял запрос " << std::endl;
                    send_message(main_socket, invite_message);
                    break;
                }
                else if (invite_message == "reject") {
					std::cout << "Игрок " + login_map[1] + " отклонил запрос " << std::endl;
                    send_message(main_socket, invite_message);
                }
                else {
                    std::cout << "Что-то пошло не так во время обработки запроса на игру" << std::endl;
                }
            }
            else {
				std::cout << "Ника " + invite_login + " нет в базе" << std::endl;
				std::cout << "Отправляю ошибку игроку" << std::endl;
				std::this_thread::sleep_for(100ms);
				
				send_message(main_socket, "Error:LoginNotExist");
            }
            std::getline(ss, tmp, ':');
        }
    }
	
    std::cout << "Опрашиваю игроков" << std::endl;
	send_message(first_player_socket, "ping");
	send_message(second_player_socket, "ping");

	std::string received_message1 = receive_message(first_player_socket);
	std::string received_message2 = receive_message(second_player_socket);

	if (received_message1 == "pong") {
		std::cout << "Игрок " + login_map[1] + " готов!" << std::endl;
	}
	else {
		std::cout << "Игрок " + login_map[1] + " откзался от игры" << std::endl;
		return 0;
	}

	if (received_message2 == "pong") {
		std::cout << "Игрок " + login_map[2] + " готов!" << std::endl;
	}
	else {
		std::cout << "Игрок " + login_map[2] + " откзался от игры" << std::endl;
		return 0;
	}


    std::cout << "Начинаю игру!" << std::endl;
    Game game;
    game.play(first_player_socket, second_player_socket);
}
