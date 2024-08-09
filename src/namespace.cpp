#include <iterator>
#include <ranges>
#include <sdkgenny/class.hpp>
#include <sdkgenny/enum.hpp>
#include <sdkgenny/enum_class.hpp>
#include <sdkgenny/generic_type.hpp>
#include <sdkgenny/struct.hpp>

#include <sdkgenny/namespace.hpp>

namespace sdkgenny {

Namespace::Namespace(std::string_view name) : Typename{name} {
}

Type* Namespace::type(std::string_view name) {
    return find_in_owners_or_add<Type>(name);
}
GenericType* Namespace::generic_type(std::string_view name) {
    return find_in_owners_or_add<GenericType>(name);
}
Struct* Namespace::struct_(std::string_view name) {
    return find_or_add<Struct>(name);
}
Class* Namespace::class_(std::string_view name) {
    return find_or_add<Class>(name);
}
Enum* Namespace::enum_(std::string_view name) {
    return find_or_add<Enum>(name);
}
EnumClass* Namespace::enum_class(std::string_view name) {
    return find_or_add<EnumClass>(name);
}
Namespace* Namespace::namespace_(std::string_view name) {
    return find_or_add<Namespace>(name);
}

Type* Namespace::resolve_type(std::string_view ns) {
    auto nssplit = std::views::split(ns, std::string_view("::"));
    auto nssplitsize = std::ranges::distance(nssplit);
    auto last = *std::ranges::next(nssplit.begin(), nssplitsize - 1);
    auto type = std::string_view(last.begin(), last.end());
    auto path = nssplit | std::views::take(nssplitsize - 1);
    Namespace* nss = topmost_owner<Namespace>();
    if(nss == nullptr)
        nss = this;

    for(auto const& nspath : path) {
        nss = nss->find_in_owners<Namespace>(std::string_view(nspath.begin(), nspath.end()), true);
    }
    
    return nss->find_in_owners<Type>(type, true);
}
} // namespace sdkgenny