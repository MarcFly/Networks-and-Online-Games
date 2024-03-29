#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	// TODO(jesus): TCP listen socket stuff
	// - Create the listenSocket
	int err_ret;
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	// - Set address reuse

	int enable = 1;
	err_ret = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(int));
	if (err_ret == SOCKET_ERROR)
	{
		reportError("Error setting up Server Socket.");
		return false;
	}
	// - Bind the socket to a local interface

	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(port); // Port
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address

	err_ret = bind(listenSocket, (const sockaddr*)&bindAddr, sizeof(bindAddr));
	if (err_ret == SOCKET_ERROR)
	{
		reportError("Error Binding Server Address.");
		return false;
	}

	// - Enter in listen mode

	int backlog = 8; // Max allowed connections
	err_ret = listen(listenSocket, backlog);
	if (err_ret == SOCKET_ERROR)
	{
		reportError("Error Setting Max Connections.");
		return false;
	}
	// - Add the listenSocket to the managed list of sockets using addSocket()

	addSocket(listenSocket);

	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}



//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ClientMessage clientMessage;
	packet >> clientMessage;

	if (clientMessage == ClientMessage::Hello)
	{
		std::string playerName;
		packet >> playerName;

		// Set the player name of the corresponding connected socket proxy
		for (auto &connectedSocket : connectedSockets)
		{
			if (connectedSocket.playerName == playerName)
			{
				OutputMemoryStream packet;
				packet << ClientMessage::Error;
				packet << "PlayerName already exists.\0";
				sendPacket(packet, socket);
				onSocketDisconnected(socket);
				break;
			}
			else if (connectedSocket.socket == socket)
			{
				connectedSocket.playerName = playerName;
				break;
			}
		}
	}
	else if (clientMessage == ClientMessage::Data)
	{
		std::string message;
		packet >> message;
		OutputMemoryStream outPacket;
		outPacket << ServerMessage::Data;
		outPacket << message;
		for (auto& connectedSocket : connectedSockets)
		{
			sendPacket(outPacket, connectedSocket.socket);
		}
	}
	else if (clientMessage == ClientMessage::Command)
	{
		HandleCommands(socket, packet);
	}
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto &connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			connectedSockets.erase(it);
			break;
		}
	}
}

void ModuleNetworkingServer::HandleCommands(SOCKET socket, const InputMemoryStream& packet)
{
	Commands type;
	packet >> type;
	std::string msg;
	packet >> msg;
	const char* check = strpbrk(msg.c_str(), " ");
	int pos = (check == nullptr) ? 0 : msg.find_first_of(' ',0);
	std::string base_val(msg.c_str(), pos);

	if (type == Commands::Whisper)
	{		
		for (auto it : connectedSockets)
		{
			if (it.playerName.compare(base_val) == 0)
			{
				OutputMemoryStream packet_send;
				packet_send << ServerMessage::Data;
				msg.erase(0, pos);
				packet_send << msg;
				sendPacket(packet_send, it.socket);
				sendPacket(packet_send, socket);
				break;
			}
		}
	}
	else if (type == Commands::Kick)
	{
		OutputMemoryStream packet_send;
		packet_send << ServerMessage::Command;
		packet_send << Commands::Kick;

		for (auto it : connectedSockets)
		{
			if (it.playerName.compare(base_val))
			{
				sendPacket(packet_send, it.socket);
				break;
			}
		}
		
	}
	else if (type == Commands::Ban)
	{
		
	}
}


