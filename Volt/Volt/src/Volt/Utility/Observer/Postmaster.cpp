#include "vtpch.h"
#include "Observer.h"
#include <cassert>

namespace Volt{

	void Postmaster::Invoke(const eEvent& aEvent, const EventMessage& aMessage){
		for(Observer* o:mySubscriberMap[aEvent])
			o->OnEvent(aEvent, aMessage);
	}

	bool Postmaster::Subscribe(Observer* ptrObserver, const eEvent& aEvent){
		assert(ptrObserver&&"ptrObserver was nullptr");
		if(!ptrObserver)
			return false;

		std::vector<eEvent>::iterator it = find(ptrObserver->mySubscriptions.begin(), ptrObserver->mySubscriptions.end(), aEvent);
		assert(it==ptrObserver->mySubscriptions.end()&&"ptrObserver is already subscribed to aEvent");
		if(it!=ptrObserver->mySubscriptions.end())
			return false;

		ptrObserver->mySubscriptions.push_back(aEvent);
		mySubscriberMap[aEvent].push_back(ptrObserver);
		return true;
	}

	bool Postmaster::UnSubscribe(Observer* ptrObserver, const eEvent& aEvent){
		assert(ptrObserver&&"ptrObserver was nullptr");
		if(!ptrObserver)
			return false;

		std::vector<eEvent>::iterator it = find(ptrObserver->mySubscriptions.begin(), ptrObserver->mySubscriptions.end(), aEvent);
		assert(it!=ptrObserver->mySubscriptions.end()&&"ptrObserver isn't subscribed to aEvent");
		if(it==ptrObserver->mySubscriptions.end())
			return false;

		ptrObserver->mySubscriptions.erase(find(ptrObserver->mySubscriptions.begin(), ptrObserver->mySubscriptions.end(), aEvent));
		mySubscriberMap[aEvent].erase(find(mySubscriberMap[aEvent].begin(), mySubscriberMap[aEvent].end(), ptrObserver));
		return true;
	}
}