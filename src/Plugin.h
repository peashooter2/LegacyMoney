#pragma once

#include <ll/api/base/Concepts.h>
#include <ll/api/plugin/NativePlugin.h>


namespace legacymoney {

class Plugin {
public:
    Plugin(ll::plugin::NativePlugin& self);

    Plugin(Plugin&&)                 = delete;
    Plugin(const Plugin&)            = delete;
    Plugin& operator=(Plugin&&)      = delete;
    Plugin& operator=(const Plugin&) = delete;

    ~Plugin() = default;

    /// @return True if the plugin is enabled successfully.
    bool enable();

    /// @return True if the plugin is disabled successfully.
    bool disable();

private:
    ll::plugin::NativePlugin& mSelf;
};

} // namespace legacymoney
