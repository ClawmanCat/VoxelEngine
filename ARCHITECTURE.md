# What is this?
This file contains some basic information about what components make up the engine and where to find them.  
It is meant as a help for people new to this codebase, who wish to understand and/or make changes to the code.  

### Renderer
The main component used when rendering is the **pipeline**. Each pipeline combines a **shader program** and some
**pipeline settings** with a **render target**. The pipeline can be used to render **vertex buffers** to that target.
Examples of targets are the **swapchain** of a window or an off-screen **framebuffer**.

There are different **pipeline categories** (**Rasterization Pipeline**, **Compute Pipeline**, etc.).
The type a pipeline will have depends on what shader types are contained within the associated shader program.  
For example, a shader program created from a `.vert.glsl` and a `.frag.glsl` shader file will always create a rasterization pipeline.

**Uniforms** can be bound to a pipeline, or to a single vertex buffer. These are values used by the shader that do not change
on a per-vertex basis, like the camera matrix.  
**Textures** are a type of uniform. Typically, many textures will be combined into a single **texture atlas** which will them
be used during rendering.


### Events
TODO

### Entity Component System
TODO
- Basic functionality
- Provided component types
- Component composition with hierarchy

### Client & Server
TODO

### Voxel Spaces
TODO

### Actors & Plugins
TODO