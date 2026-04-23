export module teinject.globals;

import teinject.common;
import teinject.handlers;

namespace teinject::globals {
    
    export auto custom_flags = impl::CustomTilesetFlagsManager();
    export auto fast_restarter = impl::FastRestarter();
    export auto state = impl::GlobalState();
    
    export auto mod_registry = ModRegistry();
    export auto handler_registry = TeinjectHandlerRegistry();
    export auto user_defined_patches = UserDefinedPatchRegistry();
    
}
