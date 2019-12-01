#pragma once

class ReplicationManagerClient
{
public:
	void read(const InputMemoryStream &packet);

	void Spawn(GameObject* go);
};