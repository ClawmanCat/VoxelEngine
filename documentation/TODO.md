### Renderer
- Verify shader IO with attributes and targets.
- Implement render to framebuffer.
- Rewrite API in Vulkan.
- Model Loading
- Phong Lighting

### ECS
- Basic ECS structure

### Events
- IO Event handling by layerstack
- Localized events

### Voxel World
- Nested Entities

### Plugin System
- Test ResourceOwner system
- Prioritized resource owners to minimize `on_actor_destroyed` calls.

### General
- Review usage of VE_ASSERT and VE_DEBUG_ONLY, consider leaving some asserts enabled in release mode.

### Serializer
- [X] If T extends BinarySerializable, it is serializable by calling the interface methods.
- [X] If T is a class, it is serializable if all its members are serializable, by serializing all its members.
- [X] If T is trivial, it is serializable by casting it to a byte[].
- [ ] If T has begin(), end() and insert() methods, it is serializable if T::value_type is serializable, by serializing all its element.