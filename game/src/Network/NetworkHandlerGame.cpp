#include "NetworkHandlerGame.h"

void NetworkHandlerGame::handleTCPEventClient(sf::Packet& tcpPacket, int event)
{
	int h;
	switch ((GameEvent)event)
	{
	case GameEvent::SAY_HELLO:
		tcpPacket >> h;
		Log::write("Hello " + std::to_string(h));
		break;
	case GameEvent::SAY_BYE:
		Log::write("Bye");
		break;
	case GameEvent::SPAM:
		Log::write("Client: Spam from server");
		break;
	default:
		break;
	}
}

void NetworkHandlerGame::handleUDPEventClient(sf::Packet& udpPacket, int event)
{
	if (event == (int)GameEvent::SPAM)
	{
		Log::write("Client: Spam from server");
		//this->getClient()->getUDPPacket() << (int)GameEvent::SPAM;
	}
}

void NetworkHandlerGame::handleTCPEventServer(Server* server, int clientID, sf::Packet& tcpPacket, int event)
{
	int h;
	sf::Packet packet;
	switch ((GameEvent)event)
	{
	case GameEvent::SAY_HELLO:
		tcpPacket >> h;
		packet << event;
		packet << h;
		server->sendToAllClientsTCP(packet);
		break;
	default:
		packet << event;
		server->sendToAllClientsTCP(packet);
		break;
	}
}

void NetworkHandlerGame::handleUDPEventServer(Server* server, int clientID, sf::Packet& udpPacket, int event)
{
	sf::Packet packet;
	packet << (int)GameEvent::SPAM;
	server->sendToAllClientsUDP(packet);
	Log::write("Server: Sent spam to clients");
}