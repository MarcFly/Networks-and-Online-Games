#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	Data,
	Command,
	Error
};

enum class ServerMessage
{
	Welcome,
	Data,
	Command,
	Error
};

