#pragma once

#include <boost/preprocessor.hpp>

#include <mutex>


#define ve_threadsafe_function                                          \
static std::mutex BOOST_PP_CAT(ve_auto_mtx_, __LINE__) {};              \
std::lock_guard<std::mutex> BOOST_PP_CAT(ve_auto_lock_, __LINE__)       \
{ BOOST_PP_CAT(ve_auto_mtx_, __LINE__) };