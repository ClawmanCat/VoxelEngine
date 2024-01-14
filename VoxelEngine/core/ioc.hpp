#pragma once

#include <kangaru/kangaru.hpp>

#include <tuple>


/** Insert within a class to mark it as a service. i.e. an object which is globally accessible through the IOC container. */
#define VE_SERVICE(Cls) friend auto service_map(const Cls&) -> kgr::autowire_single
/** Insert within a class to mark it as a multi-service, i.e. an object which is automatically constructed every time it is used by the IOC container. */
#define VE_MULTI_SERVICE(Cls) friend auto service_map(const Cls&) -> kgr::autowire


namespace ve {
    /** Provides the generated service-definition class for autowired services. */
    template <typename T> using service_of = kgr::mapped_service_t<T>;


    /** Returns the global instance of the IOC service container. */
    [[nodiscard]] extern kgr::container& services(void);


    /** Returns the instance of the service of the given type. */
    template <typename Service> inline Service& get_service(void) {
        return services().service<service_of<Service>>();
    }

    /** Returns a tuple of instances of services of the given types. */
    template <typename... Services> inline std::tuple<Services&...> get_services(void) {
        return std::tuple<Services&...> { get_service<Services>()... };
    }
}