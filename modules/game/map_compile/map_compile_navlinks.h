#ifndef MAP_COMPILE_NAVLINKS_H
#define MAP_COMPILE_NAVLINKS_H

#include "modules/tbloader/src/tb_loader_singleton.h"
class HBNavlinkCompiler : public TBLoaderHook {

    enum NavlinkType {
        
    };

    virtual void post_compile_hook(TBLoader *p_loader, TBLoaderBuildInfo *p_build_info) {};
};

#endif // MAP_COMPILE_NAVLINKS_H
