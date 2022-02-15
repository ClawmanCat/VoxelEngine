# ARCHITECTURE.md
This file contains some basic information about what components make up the engine and where to find them.  
It is meant as a help for people new to this codebase, who wish to understand and/or make changes to the code.  


### Entity Component System
At the core of the engine is the Entity Component System (ECS). An entity component system is a design pattern meant to replace the use of inheritance for objects used within the game world.  
Instead of objects, there are *entities*, which are simply numerical identifiers that are are associated with a set of components.  
*Components* serve a similar purpose to interfaces: they can be used to add functionality to certain entities.
The main difference is that components can be assigned on a per-entity basis and can be added and removed at runtime.  
Finally, systems can be used to iterate over all instances of a component. For example the `system_renderer` iterates over every `mesh_component` from every entity to render said entity.  

The ECS used in the engine is based on [ENTT](https://github.com/skypjack/entt). It does however extend on the functionality offered by this library in several ways:

###### Static Entities
While it is nice to be able to add and remove components from entities arbitrarily, it is often useful to define a template that can be used to create many identical entities, like a class would be used in a traditional OO-based approach.  
To facilitate this, the engine offers the concept of *static entities*, which form a kind of hybrid between a pure ECS-entity and a class-based one.  
By extending from `static_entity`, a class gains the ability to use the `VE_COMPONENT` and `VE_COMPONENT_FN` macros. Member variables and functions declared using these macros appear as normal members within the context of the class, but are actually managed by the ECS internally.  
For example, the `howlee` entity in `VEDemoGame` defines a member function `update`, with a signature corresponding to that of the `update_component`. When an entity is instantiated using this class, the corresponding update component is automatically added to the ECS and will thus run when `system_updater` is invoked.  

Entities created using this `static_entity` base class also contain a `this_component`, which can be used to obtain a pointer to the instance of the class associated with the entity at any time.

###### Storage Groups
While components can be used to define groups of entities at compile time, it is often useful to group entities by some runtime-defined metric as well.  
For example it may be desired to group all entities in a given chunk in a voxel space together. With just components, this would be impossible, as one would have to define a separate component type for every chunk an entity could be in.  
The engine provides a different system for such scenarios: *storage groups*.  
By inheriting from `storage_group`, an object can associate some group of entities with itself. A view of the group can then be created, which functions exactly like a normal ENTT component view.  
For example, to get all the transforms of the entities within a single chunk, one could write:
```c++
auto view = chunk.view() | storage.view<transform_component>();

for (auto entity : view) {
    const auto& transform = view.get<transform_component>(entity);
}
```
By using `tracked_storage_group` instead of a normal storage group, it is also possible to get the storage group an entity belongs to from that group itself: by fetching the `tracked_storage_group::tracker` component of an entity, a pointer to the storage group can be obtained.


### Renderer
Code for the renderer is split into two sections: the API-dependent section can be found in `VoxelEngine/platform/graphics`, and the common section can be found in `VoxelEngine/graphics`.  
The API-dependent section consists of code that depends on the underlying graphics API, and is rewritten for each API that is supported. The common section works with all graphics APIs.  

###### Pipelines
The main primitive of the renderer is the *pipeline*. A pipeline takes some settings and one or more buffers, and renders them to some target, like a window or a framebuffer.
For multi-pass rendering, a pipeline can itself be made up of different pipelines, one for each render pass. An example of such a setup can be found in `pipeline/multipass_pbr_pipeline.hpp` in the API-dependent graphics section.  
The most simple pipeline is the `simple_pipeline`, found in `pipeline/pipeline.hpp`, which performs a single render pass using the provided shaders.  

###### Shaders & Inputs
*Shaders* are managed by the `shader_cache` in `graphics/shader/cache.hpp`. Simply provide the name of a shader, or a set of shader file paths, and the engine will either compile the shader or load it from cache.  
The returned shader will also contain additional info, like reflection information and what type of graphics pipeline it belongs to.

Other than vertex buffers, shaders may accept *uniforms* as additional inputs when performing rendering.  
Various classes in the rendering system, like the pipeline, and the vertex buffer inherit from `uniform_storage`. Objects stored in said storage will be automatically used when the object is used for rendering.
Values can be stored either directly, or in the form of producers, which are queried for a new value every time a render is performed.  

Each uniform can be assigned a `combine_function`. The combine function dictates how uniforms are merged if different layers in the rendering system provide different values for a uniform. 
For example, if both a mesh and a submesh provide a transform matrix, the desired behaviour will probably be to multiply these matrices, rather than for one to overwrite the other.

Currently, the engine only supports Uniform Buffer Objects (UBOs) as shader inputs. The engine automatically converts any provided uniform to the STD140 layout, (See `graphics/shader/glsl_layout.hpp`) so this need not be done manually.

###### GLSL Extensions
The engine supports several extensions to standard GLSL. For example, shaders may include other files using `#include` directives, like in C++, and may use preprocessor macros defined when compiling the shader.  
Said definitions can be set in the shader cache using `set_compile_options`.  
The engine will automatically generate bindings and locations for shader inputs that do not have them. Do note that there is currently no functionality to synchronize bindings between different shaders, so in that case manual binding is still required.

###### Textures
Internally, the engine uses texture atlases to combine many textures into one. This means many objects with different textures, like the different tiles of a chunk in a voxel space, can be rendered in a single draw call.  
Managing the textures and atlases involved in this process can be done using the `texture_manager`. Textures can be provided in the form of either a file path, an image object or a generator function, and are associated with a name that can be used to fetch them later.  
The UV and index information provided by returned textures can be used to directly initialize the vertices that need to have said texture applied to them.  
For rendering, the texture atlas can be obtained from the texture manager and simply be bound as a uniform. It will automatically map to the sampler of the same name, if it exists.  
To accomplish this, texture atlases implement the `uniform_sampler` interface. Other classes may implement this interface as well to achieve similar functionality.

###### Lighting
Since the way in which lights need to be managed depends on the type of rendering performed (For example, when doing shadow mapping, it is necessary to perform one render pass per light), they cannot be handled as simple uniforms.  
Instead, the engine provides the `light_component` which can be added to any entity to turn it into a light source.  
The way in which this light source is handled is then decided by the pipeline.  

###### ECS Integration
For most simple scenarios, direct interaction with the rendering system is not necessary. If one simply wants to render an object. it is enough to add a `system_renderer` to the ECS and provide the desired entity with a `mesh_component`.  
The render system is templatized in such a way that it can be almost fully customized. By providing a list of components to include or exclude, entities can be split across different renderers. The renderer also accepts a set of required components which will be converted to uniforms for rendering.  
Said conversions can be managed through the `uniform_convertible` interface, which allows arbitrary components to be used as uniforms in the renderer.


### Events & Input
The engine provides several classes for working with events. These can be found in `VoxelEngine/event`. The `simple_event_dispatcher` simply dispatches events to handles as they are received.
The `delayed_event_dispatcher` collects events, then dispatches them all at once.  
`subscribe_only_view` can be used to hide the event dispatching section of the handler in derived classes. This can be used to create classes that can be subscribed to from anywhere, but can only dispatch events themselves.  

A notable user of the event system is the `input_manager` in `VoxelEngine/input`. The input manager dispatches events for all user input that is received, like keyboard presses and mouse movement.  
The `input_binder` is a wrapper around the `input_manager` that can be used to bind certain inputs to names, and then listen for those names instead.  
Using the input binder is preferred over directly interacting with the input_manager, since it allows easy rebinding of inputs.


### Client & Server
The engine can be used in both a client-only as well as in a client-server configuration. Each side, client or server, is known internally  as an `instance`.  

Each instance contains its own ECS. Two instances can be connected together to exchange data.  
The engine separates each instance from how it is connecting to its partner on the other side. Whether the other instance is running locally or on the other side of the world, the client and server classes themselves do not care.
Instead this is handled by classes extending the `message_handler` class, which is responsible for sending messages between the two.  
Different types of messages can be created. Each type is assigned a numerical identifier by the Message Type Registry (MTR).
The engine ensures that both sides agree on these identifiers and what type of data the message associated with them contains.  

Core messages are message types that are predefined by the engine in order for it to function. These are automatically added to the MTR upon creation and cannot be removed.  
In most cases, you should never need to send a message of a core type manually. They are mainly for internal use.


---
**NOTE**

This feature is currently unfinished. Notably:
- The two instances of the ECS are not yet synchronized automatically. Messages must be manually sent.
- The remote connection is currently not in a state where it can be reliably used. It can be found on a separate branch.

---


### Voxel Spaces
The `voxel` module contains facilities for creating voxel spaces. Voxel spaces consists of `chunks`: parts of the voxel space which can be unloaded and loaded separately.
The use of chunks allows voxel spaces to effectively have infinite size, since only the parts of the space near a player need to be loaded.  
Chunks are generated using a class implementing the `chunk_generator` interface. Each voxel space has exactly one chunk generator associated with it.  
Chunk loaders can be added to the space to load parts of it. Chunk loaders can be created by deriving from the `chunk_loader` interface.  

Chunks are made up of tiles. Different types of tile can be created by extending the `tile` class. Note that chunks do not store instances of tiles: there is exactly one tile object for every type of tile.
Since this means that tiles effectively cannot store any state inside of them, each tile is assigned one or more bytes of metadata that it may use to store per-tile state.  
The `tile_registry` keeps track of what types of tile exist in the game and allows conversions between tile objects and the data objects stored in chunks.  
New tiles must always be added to the registry before they can be used in the engine.  

Meshing of the chunks within a voxel space happens automatically. To obtain the mesh of the voxel space, simply call `get_vertex_buffer()`. The resulting buffer can be used directly in a mesh component in the ECS.


### Plugins
The engine supports the ability to dynamically load and unload external plugins. Different plugins may have dependencies upon one another, which are automatically resolved by the engine.  
Whenever a plugin stores some object within a structure it does not own, it receives a RAII-handle to that resource, which will cause the resource to be removed from the structure when the plugin is unloaded.  
This prevents the engine from keeping references to objects from plugins that are no longer loaded.

---
**NOTE**

This feature is currently unfinished. Notably:
- The system for assigning resource tokens to plugins is currently being refactored. In previous versions, resource-managing objects were responsible for deleting plugin resources manually. The new system will provide plugins with RAII-handles to these resources instead.  
  This new system is not yet implemented. The old version of this system can be found on a separate branch.

---

### Game
A demonstrator game is provided with the engine to provide code examples of how to perform basic tasks. The game can be found in `VEDemoGame`.  
This game also serves to demonstrate some of the performance capabilities of the engine, notably:
- Having a very large number of entities active at one time (thousands of tens of thousands)
- Using deferred shading to render a large number of light sources (several hundred).