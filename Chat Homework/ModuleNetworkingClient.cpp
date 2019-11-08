#include "ModuleNetworkingClient.h"


bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	int err_ret;
	// TODO(jesus): TCP connection stuff
	// - Create the socket
	
	err_ret = own_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (err_ret == SOCKET_ERROR)
	{
		reportError("Error Creating Client Socket.");
		return false;
	}
	// - Create the remote address object
	
	struct sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET; // IPv4
	remoteAddr.sin_port = htons(serverPort); // Port
	const char *remoteAddrStr = serverAddressStr; // Not so remote� :-P
	inet_pton(AF_INET, remoteAddrStr, &remoteAddr.sin_addr);
	if (err_ret == SOCKET_ERROR)
	{
		reportError("Error Setting Client Addres.");
		return false;
	}
	// - Connect to the remote address

	err_ret = connect(own_socket, (const sockaddr*)&remoteAddr, sizeof(remoteAddr));
	if ( err_ret == 0)
	{
		// - Add the created socket to the managed list of sockets using addSocket()
		addSocket(own_socket);
		// If everything was ok... change the state
		state = ClientState::Start;
	}
	else
	{
		if(err_ret == -1) reportError("Error Connecting to Server.");
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
		OutputMemoryStream packet;
		packet << ClientMessage::Hello;
		packet << playerName;
		if (sendPacket(packet, own_socket))
		{
			state = ClientState::Logging;
		}
		else
		{
			disconnect();
			state = ClientState::Stopped;
		}
	}
	if (state == ClientState::Logging)
	{
		
	}

	return true;
}

bool ModuleNetworkingClient::gui()
{
	bool sdw = true;
	ImGui::ShowDemoWindow(&sdw);
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		if (ImGui::Button("Disconnect"))
		{
			disconnect();
			state = ClientState::Stopped;
		}
		ImGui::SameLine();

		ImGui::Text("%s connected to the server...", playerName.c_str());
		ImGui::Separator();
		ImGui::BeginGroup();
		{
			for (int i = 0; i < messages.size(); ++i)
				ImGui::Text(messages[i].c_str());
			
		}ImGui::EndGroup();

		ImGui::Separator();
		ImGui::InputText("", curr_msg, 512);
		ImGui::SameLine();
		if (ImGui::Button("Send"))
		{
			OutputMemoryStream packet;
			std::string data(curr_msg);
			packet << ClientMessage::Data;
			packet << data;
			sendPacket(packet, own_socket);
		}
		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{

	ClientMessage type;
	packet >> type;
	std::string message;
	switch (type) 
	{
	case ClientMessage::Error:
		packet >> message;
		reportError(message.c_str());
		state = ClientState::Stopped;
		break;

	case ClientMessage::Data:
		packet >> message;
		messages.push_back(message);
		break;

	case ClientMessage::Command:
		break;
	default:
		state = ClientState::Stopped;
	}
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

