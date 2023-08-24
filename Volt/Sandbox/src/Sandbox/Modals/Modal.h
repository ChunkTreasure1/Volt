#pragma once

#include <Volt/Core/UUID.h>

#include <string>

class Modal
{
public:
	Modal(const std::string& strId);
	virtual ~Modal() = default;

	void Open();
	void Close();
	bool Update();

	inline const Volt::UUID GetID() const { return m_id; }

protected:
	virtual void DrawModalContent() = 0;
	virtual void OnOpen() {};
	virtual void OnClose() {};

private:
	friend class ModalSystem;

	bool m_wasOpenLastFrame = false;

	Volt::UUID m_id;
	std::string m_strId;
};
