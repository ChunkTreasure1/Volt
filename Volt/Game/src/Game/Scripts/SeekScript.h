#pragma once

#include <Volt/Scripting/Script.h>
#include <Volt/Events/KeyEvent.h>

class SeekScript : public Volt::Script
{
public:
	SeekScript(Volt::Entity entity);
	~SeekScript() override = default;

	void OnStart() override;
	void OnEvent(Volt::Event& e) override;
	void OnUpdate(float aDeltaTime) override;

	static Ref<Script> Create(Volt::Entity aEntity) { return CreateRef<SeekScript>(aEntity); }
	static WireGUID GetStaticGUID() { return "{9A9D8F76-7E27-4F12-95BC-FED74B22A18B}"_guid; };
	WireGUID GetGUID() override { return GetStaticGUID(); }

private:
	gem::vec2 myViewportMousePos;
	gem::vec2 myViewportSize;

	gem::vec3 myTarget;
	gem::vec3 myDirection;
};