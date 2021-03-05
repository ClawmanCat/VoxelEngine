# What is this?
This file contains some basic information about what components make up the engine and where to find them.  
It is meant as a help for people new to this codebase, who wish to understand and/or make changes to the code.  

### Renderer
The renderer consists of two sections: a common part and a platform-dependent part.  
The platform-dependent part has a different implementation, depending on the used graphics API (OpenGL / Vulkan).
It can be found in `VoxelEngine/platform/graphics/<current_platform>`.  
The common part consists of the parts of the renderer which do not change with the graphics API. 
It can be found in. `VoxelEngine/graphics`. Here you will also find the file `graphics.hpp`, 
which includes the entire public facing part of the renderer, including the platform dependent parts
for the currently selected platform.  
This file also exposes the macro `graphics_include`,  which can be used to include files from the currently active graphics API.  
e.g. `#include graphics_include(window/window.hpp)` will either include the window class for Vulkan
or for OpenGL, depending on the currently active platform.  
To set the current platform, change the `VE_GRAPHICS_API` macro to either `vulkan` or `opengl`.
This macro is set in `VoxelEngine/CMakeLists.txt`.


The renderer consists of several components: a *target* (for example a layer on a window or a texture) is drawn to by a *pipeline*.
Each pipeline has some *buffers*, each drawn with a *shader*. Both the pipeline and the buffer can have associated *uniforms*,
which are values that remain constant during the rendering of that object.


### Events


### Entity Component System
The ECS, as the name suggests, is made up of three distinct parts: *entities*, *components* and *systems*.  
An entity is any object existing within the game world. It can have properties, so called components, associated with it.  
A system is a functor which acts on all entities with a certain component or a set of components.  
For example: some entities may have a *renderable* component. The *renderer* (a system) takes all these components and uses them
to render the associated entities to whatever its render target is.  

Many game engines force developers to either use a fully-ECS focussed system, where an entity is just a number with some components associated to it,
or a fully OO-based system, where each entity is an instance of some class derived from a common base.  
VoxelEngine allows its users to pick a point anywhere between these two extremes: while it is possible to create new 
entity types purely by obtaining a new entity ID and adding some components to it, it is also possible to create a class to serve
as a template for many identical entities. Such a class can have so-called *static components*: components associated with each entity of that class,
that cannot be removed and can be accessed as if they were member variables and functions.


### Client & Server


### Voxel Spaces


### Actors & Plugins