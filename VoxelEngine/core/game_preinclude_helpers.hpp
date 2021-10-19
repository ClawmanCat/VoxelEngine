// This tag can be used in the game pre-include header to overload settings structs within the engine.
// This is done by forward declaring the settings template, and specializing it for the tag.
// For example, to overload the struct ve::voxel::voxel_settings_t:
//
// template <typename T> struct ve::voxel::voxel_settings_t;
//
// template <> struct ve::voxel::voxel_settings_t<ve::overloadable_settings_tag> {
//     [Settings go here.]
// }
//
// To overload only some of the settings, simply inherit from a different specialization of the template,
// e.g. ve::voxel::voxel_settings_t<void> for the above example, so that base-class settings are used where no derived value is defined.
namespace ve {
    struct overloadable_settings_tag {};
}