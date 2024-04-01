#pragma once
#include <map>
#include <functional>
/// <summary>
/// quick and dirty delegate implementation
/// </summary>
/// <typeparam name="T"></typeparam>

typedef unsigned int DelegateHandle;
template<typename T>
class Delegate
{
public:
	Delegate() {}
	virtual ~Delegate() {}

	template<typename ...DelegateParams> 
	void Broadcast(DelegateParams... params)
	{
		for (auto& pair : m_Functions)
		{
			pair.second(params...);
		}
	}

	[[nodiscard]] DelegateHandle Add(std::function<T> function);
	void Remove(DelegateHandle handle);

private:
	std::map<unsigned int, std::function<T>> m_Functions;
};

template<typename T>
inline DelegateHandle Delegate<T>::Add(std::function<T> function)
{
	static unsigned int id = 0;
	m_Functions[id] = function;
	return id++;
}

template<typename T>
inline void Delegate<T>::Remove(DelegateHandle handle)
{
	m_Functions.erase(handle);
}
