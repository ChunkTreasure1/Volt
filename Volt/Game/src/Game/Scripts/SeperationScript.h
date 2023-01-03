#pragma once

#include <Volt/Scripting/Script.h>
#include <Volt/Events/KeyEvent.h>

class SeperationScript : public Volt::Script
{
public:
	SeperationScript(Volt::Entity entity);
	~SeperationScript() override = default;

	void OnStart() override;
	void OnEvent(Volt::Event& e) override;
	void OnUpdate(float aDeltaTime) override;

	static Ref<Script> Create(Volt::Entity aEntity) { return CreateRef<SeperationScript>(aEntity); }
	static WireGUID GetStaticGUID() { return "{B007B94F-1A98-474C-A285-F687CF777496}"_guid; };
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	gem::vec2 myViewportMousePos;
	gem::vec2 myViewportSize;

	gem::vec3 myTarget;
	gem::vec3 myDirection;
};