#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff
	// - Create the socket

	own_socket = socket(AF_INET, SOCK_STREAM, 0);

	// - Create the remote address object
	
	struct sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET; // IPv4
	remoteAddr.sin_port = htons(serverPort); // Port
	const char *remoteAddrStr = serverAddressStr; // Not so remote� :-P
	inet_pton(AF_INET, remoteAddrStr, &remoteAddr.sin_addr);

	// - Connect to the remote address

	if (connect(own_socket, (const sockaddr*)&remoteAddr, sizeof(remoteAddr)) != -1)
	{
		// - Add the created socket to the managed list of sockets using addSocket()
		addSocket(own_socket);
		// If everything was ok... change the state
		state = ClientState::Start;
	}
	else
	{
		state = ClientState::Stopped;
	}

	

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		// TODO(jesus): Send the player name to the server
		send(own_socket, playerName.c_str(), playerName.size(), 0);
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("%s connected to the server...", playerName.c_str());

		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, byte * data)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

