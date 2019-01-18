#pragma once

#include <sdkddkver.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/functional/hash.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <boost/signals2.hpp>

#include <boost/log/trivial.hpp>
#include <boost/log/sources/channel_feature.hpp>
#include <boost/log/sources/channel_logger.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/expressions/attr_fwd.hpp>
#include <boost/log/expressions/attr.hpp>
#include <boost/log/utility/setup/from_stream.hpp>

#include <boost/log/utility/type_dispatch/type_dispatcher.hpp>
#include <boost/log/utility/type_dispatch/static_type_dispatcher.hpp>
#include <boost/log/utility/type_dispatch/dynamic_type_dispatcher.hpp>
#include <boost/log/utility/type_dispatch/standard_types.hpp>
#include <boost/log/utility/type_dispatch/date_time_types.hpp>
