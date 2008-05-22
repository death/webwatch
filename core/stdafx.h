// WebWatch Core - by DEATH, 2004
//
// $Workfile: stdafx.h $ - Precompiled headers
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/20/05 3:12 $
// $NoKeywords: $
//
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wininet.h>

#include <sstream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <hash_map>
#include <functional>
#include <iterator>
#include <cctype>

#include "openssl/ssl.h"
#include "boost/regex.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string.hpp"

#include "xml/Xml.h"

#ifndef BOOST_HAS_THREADS
#error Must compile with multithreading support for Boost
#endif // BOOST_HAS_THREADS

// Define Loki's default threading model
#define SINGLETON_DEFAULT_THREADING ::Loki::ClassLevelLockable
#define DEFAULT_THREADING ::Loki::ObjectLevelLockable