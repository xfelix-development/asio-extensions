/// @copyright Copyright (c) 2016 Tim Niederhausen (tim@rnc-ag.de)
/// Distributed under the Boost Software License, Version 1.0.
/// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "asioext/open.hpp"

#include "asioext/detail/error_code.hpp"
#include "asioext/detail/throw_error.hpp"

#if defined(ASIOEXT_WINDOWS)
# include "asioext/detail/win_file_ops.hpp"
#else
# include "asioext/detail/posix_file_ops.hpp"
#endif

ASIOEXT_NS_BEGIN

file_handle open(const char* filename, open_flags flags,
                 file_perms perms, file_attrs attrs)
{
  error_code ec;
  const file_handle h = open(filename, flags, perms, attrs, ec);
  detail::throw_error(ec);
  return h;
}

file_handle open(const char* filename, open_flags flags,
                 file_perms perms, file_attrs attrs,
                 error_code& ec) ASIOEXT_NOEXCEPT
{
#if defined(ASIOEXT_WINDOWS)
  return detail::win_file_ops::open(filename, flags, perms, attrs, ec);
#else
  return detail::posix_file_ops::open(filename, flags, perms, attrs, ec);
#endif
}

#if defined(ASIOEXT_WINDOWS)
file_handle open(const wchar_t* filename, open_flags flags,
                 file_perms perms, file_attrs attrs)
{
  error_code ec;
  const file_handle h = open(filename, flags, perms, attrs, ec);
  detail::throw_error(ec);
  return h;
}

file_handle open(const wchar_t* filename, open_flags flags,
                 file_perms perms, file_attrs attrs,
                 error_code& ec) ASIOEXT_NOEXCEPT
{
  return detail::win_file_ops::open(filename, flags, perms, attrs, ec);
}
#endif

#if defined(ASIOEXT_HAS_BOOST_FILESYSTEM)
file_handle open(const boost::filesystem::path& filename, open_flags flags,
                 file_perms perms, file_attrs attrs)
{
  error_code ec;
  const file_handle h = open(filename, flags, perms, attrs, ec);
  detail::throw_error(ec);
  return h;
}

file_handle open(const boost::filesystem::path& filename, open_flags flags,
                 file_perms perms, file_attrs attrs,
                 error_code& ec) ASIOEXT_NOEXCEPT
{
#if defined(ASIOEXT_WINDOWS)
  return detail::win_file_ops::open(filename.c_str(), flags, perms, attrs, ec);
#else
  return detail::posix_file_ops::open(filename.c_str(), flags, perms, attrs, ec);
#endif
}
#endif

ASIOEXT_NS_END
