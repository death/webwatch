// WebWatch Util - by DEATH, 2005
//
// $Workfile: stdafx.h $ - Precompiled headers
//
// $Author: Death $
// $Revision: 3 $
// $Date: 5/07/05 20:03 $
// $NoKeywords: $
//
#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wininet.h>
#include <shellapi.h>
#include <winsock2.h>
#include <comdef.h>
#include <olectl.h>
#include <process.h>

#include <list>
#include <algorithm>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include <stdexcept>
#include <sstream>
#include <cctype>
#include <cstddef>
#include <ctime>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/shared_ptr.hpp>

#include "xml/Xml.h"

// Define Loki's default threading model
#define SINGLETON_DEFAULT_THREADING ::Loki::ClassLevelLockable
#define DEFAULT_THREADING ::Loki::ObjectLevelLockable
