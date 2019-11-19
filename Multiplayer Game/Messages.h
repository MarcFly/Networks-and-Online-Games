#pragma once

enum class ClientMessage
{
	Hello,
	Input,
	Ping,
	Bye
};

enum class ServerMessage
{
	Welcome,
	Unwelcome,
	Replication,
	Ping,
	Disconnected
};
