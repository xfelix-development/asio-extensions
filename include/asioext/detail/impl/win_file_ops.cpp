/// @copyright Copyright (c) 2015 Tim Niederhausen (tim@rnc-ag.de)
/// Distributed under the Boost Software License, Version 1.0.
/// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "asioext/open_flags.hpp"

#include "asioext/detail/win_file_ops.hpp"
#include "asioext/detail/error.hpp"

#if defined(ASIOEXT_WINDOWS_USE_UTF8_FILENAMES) || defined(ASIOEXT_WINDOWS_APP)
# include "asioext/detail/win_path.hpp"
#endif

#include <windows.h>

ASIOEXT_NS_BEGIN

namespace detail {
namespace win_file_ops {

struct create_file_args
{
  DWORD creation_disposition;
  DWORD desired_access;
  DWORD share_mode;
  DWORD attrs;
  DWORD flags;
};

void set_error(error_code& ec)
{
  ec = error_code(::GetLastError(), asio::error::get_system_category());
}

bool parse_open_flags(create_file_args& args, open_flags flags,
                      file_perms perms, file_attrs attrs)
{
  if (!is_valid(flags))
    return false;

  args.creation_disposition = 0;
  if ((flags & open_flags::create_new) != open_flags::none)
    args.creation_disposition = CREATE_NEW;
  else if ((flags & open_flags::create_always) != open_flags::none)
    args.creation_disposition = CREATE_ALWAYS;
  else if ((flags & open_flags::open_existing) != open_flags::none)
    args.creation_disposition = OPEN_EXISTING;
  else if ((flags & open_flags::open_always) != open_flags::none)
    args.creation_disposition = OPEN_ALWAYS;
  else if ((flags & open_flags::truncate_existing) != open_flags::none)
    args.creation_disposition = TRUNCATE_EXISTING;

  args.desired_access = 0;
  if ((flags & open_flags::access_read) != open_flags::none)
    args.desired_access |= GENERIC_READ;
  if ((flags & open_flags::access_write) != open_flags::none)
    args.desired_access |= GENERIC_WRITE;

  args.attrs = 0;
  if ((attrs & file_attrs::hidden) != file_attrs::none)
    args.attrs |= FILE_ATTRIBUTE_HIDDEN;
  if ((attrs & file_attrs::system) != file_attrs::none)
    args.attrs |= FILE_ATTRIBUTE_SYSTEM;
  if ((attrs & file_attrs::not_indexed) != file_attrs::none)
    args.attrs |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;

  args.flags = 0;
  // TODO: Add support
  args.share_mode = 0;
  return true;
}

handle_type open(const char* filename, open_flags flags,
                 file_perms perms, file_attrs attrs, error_code& ec)
{
  create_file_args args;
  if (!parse_open_flags(args, flags, perms, attrs)) {
    ec = asio::error::invalid_argument;
    return INVALID_HANDLE_VALUE;
  }

#if defined(ASIOEXT_WINDOWS_USE_UTF8_FILENAMES) || defined(ASIOEXT_WINDOWS_APP)
  detail::win_path p(filename, std::strlen(filename), ec);
  if (ec) return INVALID_HANDLE_VALUE;
#endif

#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h =
# if defined(ASIOEXT_WINDOWS_USE_UTF8_FILENAMES)
      ::CreateFileW(p.c_str(),
                    args.desired_access, args.share_mode, NULL,
                    args.creation_disposition, args.attrs | args.flags, NULL);
# else
      ::CreateFileA(filename,
                    args.desired_access, args.share_mode, NULL,
                    args.creation_disposition, args.attrs | args.flags, NULL);
# endif
#else
  CREATEFILE2_EXTENDED_PARAMETERS params = {};
  params.dwSize = sizeof(params);
  params.dwFileAttributes = args.attrs;
  params.dwFileFlags = args.flags;
  const handle_type h =
      ::CreateFile2(p.c_str(), args.desired_access, args.share_mode,
                    args.creation_disposition, &params);
#endif

  if (h == INVALID_HANDLE_VALUE)
    set_error(ec);
  else
    ec = error_code();

  return h;
}

handle_type open(const wchar_t* filename, open_flags flags,
                 file_perms perms, file_attrs attrs, error_code& ec)
{
  create_file_args args;
  if (!parse_open_flags(args, flags, perms, attrs)) {
    ec = asio::error::invalid_argument;
    return INVALID_HANDLE_VALUE;
  }

#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h =
      ::CreateFileW(filename, args.desired_access, args.share_mode, NULL,
                    args.creation_disposition, args.attrs | args.flags, NULL);
#else
  CREATEFILE2_EXTENDED_PARAMETERS params = {};
  params.dwSize = sizeof(params);
  params.dwFileAttributes = args.attrs;
  params.dwFileFlags = args.flags;
  const handle_type h =
      ::CreateFile2(filename, args.desired_access, args.share_mode,
                    args.creation_disposition, &params);
#endif

  if (h != INVALID_HANDLE_VALUE)
    ec = error_code();
  else
    set_error(ec);

  return h;
}

void close(handle_type fd, error_code& ec)
{
  if (::CloseHandle(fd))
    ec = error_code();
  else
    set_error(ec);
}

handle_type duplicate(handle_type fd, error_code& ec)
{
  const handle_type current_process = ::GetCurrentProcess();
  handle_type new_fd = INVALID_HANDLE_VALUE;

  if (::DuplicateHandle(current_process, fd, current_process, &new_fd,
                        0, FALSE, DUPLICATE_SAME_ACCESS))
    ec = error_code();
  else
    set_error(ec);

  return new_fd;
}

handle_type get_stdin(error_code& ec)
{
#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h = ::GetStdHandle(STD_INPUT_HANDLE);
  if (h != INVALID_HANDLE_VALUE)
    ec = error_code();
  else
    set_error(ec);

  return h;
#else
  ec = asio::error::operation_not_supported;
  return INVALID_HANDLE_VALUE;
#endif
}

handle_type get_stdout(error_code& ec)
{
#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h = ::GetStdHandle(STD_OUTPUT_HANDLE);
  if (h != INVALID_HANDLE_VALUE)
    ec = error_code();
  else
    set_error(ec);

  return h;
#else
  ec = asio::error::operation_not_supported;
  return INVALID_HANDLE_VALUE;
#endif
}

handle_type get_stderr(error_code& ec)
{
#if !defined(ASIOEXT_WINDOWS_APP)
  const handle_type h = ::GetStdHandle(STD_ERROR_HANDLE);
  if (h != INVALID_HANDLE_VALUE)
    ec = error_code();
  else
    set_error(ec);

  return h;
#else
  ec = asio::error::operation_not_supported;
  return INVALID_HANDLE_VALUE;
#endif
}

uint64_t size(handle_type fd, error_code& ec)
{
  LARGE_INTEGER size;
  if (::GetFileSizeEx(fd, &size)) {
    ec = error_code();
    return static_cast<uint64_t>(size.QuadPart);
  }

  set_error(ec);
  return 0;
}

// Make sure our origin mappings match the system headers.
static_assert(static_cast<DWORD>(seek_origin::from_begin) == FILE_BEGIN &&
              static_cast<DWORD>(seek_origin::from_current) == FILE_CURRENT &&
              static_cast<DWORD>(seek_origin::from_end) == FILE_END,
              "whence mapping must match the system headers");

uint64_t seek(handle_type fd, seek_origin origin, int64_t offset,
              error_code& ec)
{
  LARGE_INTEGER pos, res;
  pos.QuadPart = offset;

  if (::SetFilePointerEx(fd, pos, &res, static_cast<DWORD>(origin)))
    return res.QuadPart;

  set_error(ec);
  return 0;
}

uint32_t read(handle_type fd, void* buffer, uint32_t size, error_code& ec)
{
  DWORD bytesRead = 0;
  if (!::ReadFile(fd, buffer, size, &bytesRead, NULL)) {
    set_error(ec);
    return 0;
  }

  if (bytesRead == 0 && size != 0)
    ec = asio::error::eof;

  return bytesRead;
}

uint32_t write(handle_type fd,
               const void* buffer,
               uint32_t size,
               error_code& ec)
{
  DWORD bytesWritten = 0;
  if (!::WriteFile(fd, buffer, size, &bytesWritten, NULL)) {
    set_error(ec);
    return 0;
  }

  return bytesWritten;
}

uint32_t pread(handle_type fd,
               void* buffer,
               uint32_t size,
               uint64_t offset,
               error_code& ec)
{
  LARGE_INTEGER offset2;
  offset2.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset2.LowPart;
  overlapped.OffsetHigh = offset2.HighPart;

  DWORD bytesRead = 0;
  if (!::ReadFile(fd, buffer, size, &bytesRead, &overlapped)) {
    set_error(ec);
    return 0;
  }

  if (bytesRead == 0 && size != 0)
    ec = asio::error::eof;

  return bytesRead;
}

uint32_t pwrite(handle_type fd,
                const void* buffer,
                uint32_t size,
                uint64_t offset,
                error_code& ec)
{
  LARGE_INTEGER offset2;
  offset2.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset2.LowPart;
  overlapped.OffsetHigh = offset2.HighPart;

  DWORD bytesWritten = 0;
  if (!::WriteFile(fd, buffer, size, &bytesWritten, &overlapped)) {
    set_error(ec);
    return 0;
  }

  return bytesWritten;
}

}
}

ASIOEXT_NS_END
