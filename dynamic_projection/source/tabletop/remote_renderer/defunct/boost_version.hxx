#pragma once

#if BOOST_VERSION / 100000 <= 1 and BOOST_VERSION / 100 % 1000 <= 34
#warning "Your version of Boost is over 2 years old. Please upgrade."
#define BOOST_DEPRECATED_THREAD_MODEL
#endif
