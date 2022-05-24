#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve::gfx::opengl {
    // Base class for pipelines that support mixins.
    template <
        template <template <typename> typename...> typename Pipeline,
        template <typename Pipeline> typename... Mixins
    > class pipeline_mixin_base : public Mixins<Pipeline<Mixins...>>... {
    public:
        using mixin_list = meta::pack<Mixins<Pipeline<Mixins...>>...>;
        ve_shared_only_then(pipeline_mixin_base, init_mixins) {}
    protected:
        void init_mixins(void) {
            mixin_list::foreach([&] <typename Mixin> {
                ((Mixin*) this)->on_construct();
            });
        }
    };
}