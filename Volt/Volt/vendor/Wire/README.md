# Wire
![](https://raw.githubusercontent.com/ChunkTreasure1/Wire/main/resource/WireLogo.png)
Wire is a small, simple, fast and generic ECS made in C++.
## Features
* Easy interface
* Cache friendly
* Built in serialization and deserialization
* Built in entity child support
## Usage
The entire ECS is based on the `Wire::Registry`class, here you will create/remove entities and handle their components. A simple example:

    Wire::Registry registry;
    Wire::EntityId entity = registry.CreateEntity();
    ExampleComponent& component = registry.AddComponent<ExampleComponent>(entity);
Components are simple POD structs which are registered and given a GUID. Example component:

    struct ExampleComponent
    {
	    float x;
	    float y;
	    SERIALIZE_COMPONENT(ExampleComponent, "{4709522E-FB7B-4B85-8FDD-C31853A89FF3}"_guid);
    }
    REGISTER_COMPONENT(ExampleComponent);
Serialization and deserialization is done using the `Wire::Serializer` class and is quite easy to use. Example:
 

    Wire::Serializer::SerializeEntityToFile(ent, registry, "Scene");
	Wire::Serializer::DeserializeEntityToRegistry("Entity.ent", registry);



## TODO:
* Remove name from serialization
* Implement backwards compatability
* Implement vector serialization support
