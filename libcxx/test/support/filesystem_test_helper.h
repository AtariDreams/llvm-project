#ifndef FILESYSTEM_TEST_HELPER_H
#define FILESYSTEM_TEST_HELPER_H

#include "filesystem_include.h"

#include <sys/stat.h> // for stat, mkdir, mkfifo
#ifndef _WIN32
#include <unistd.h> // for ftruncate, link, symlink, getcwd, chdir
#else
#include <io.h>
#include <direct.h>
#include <windows.h> // for CreateSymbolicLink, CreateHardLink
#endif

#include <cassert>
#include <cstdio> // for printf
#include <string>
#include <chrono>
#include <vector>
#include <regex>

#include "test_macros.h"
#include "rapid-cxx-test.h"
#include "format_string.h"

// For creating socket files
#if !defined(__FreeBSD__) && !defined(__APPLE__) && !defined(_WIN32)
# include <sys/socket.h>
# include <sys/un.h>
#endif

namespace utils {
#ifdef _WIN32
    inline int mkdir(const char* path, int mode) { (void)mode; return ::_mkdir(path); }
    inline int ftruncate(int fd, off_t length) { return ::_chsize(fd, length); }
    inline int symlink(const char* oldname, const char* newname, bool is_dir) {
        DWORD flags = is_dir ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;
        if (CreateSymbolicLinkA(newname, oldname,
                                flags | SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE))
          return 0;
        if (GetLastError() != ERROR_INVALID_PARAMETER)
          return 1;
        return !CreateSymbolicLinkA(newname, oldname, flags);
    }
    inline int link(const char *oldname, const char* newname) {
        return !CreateHardLinkA(newname, oldname, NULL);
    }
#else
    using ::mkdir;
    using ::ftruncate;
    inline int symlink(const char* oldname, const char* newname, bool is_dir) { (void)is_dir; return ::symlink(oldname, newname); }
    using ::link;
#endif

    inline std::string getcwd() {
        // Assume that path lengths are not greater than this.
        // This should be fine for testing purposes.
        char buf[4096];
        char* ret = ::getcwd(buf, sizeof(buf));
        assert(ret && "getcwd failed");
        return std::string(ret);
    }

    inline bool exists(std::string const& path) {
        struct ::stat tmp;
        return ::stat(path.c_str(), &tmp) == 0;
    }
} // end namespace utils

struct scoped_test_env
{
    scoped_test_env() : test_root(available_cwd_path()) {
#ifdef _WIN32
        // Windows mkdir can create multiple recursive directories
        // if needed.
        std::string cmd = "mkdir " + test_root.string();
#else
        std::string cmd = "mkdir -p " + test_root.string();
#endif
        int ret = std::system(cmd.c_str());
        assert(ret == 0);

        // Ensure that the root_path is fully resolved, i.e. it contains no
        // symlinks. The filesystem tests depend on that. We do this after
        // creating the root_path, because `fs::canonical` requires the
        // path to exist.
        test_root = fs::canonical(test_root);
    }

    ~scoped_test_env() {
#ifdef _WIN32
        std::string cmd = "rmdir /s /q " + test_root.string();
        int ret;
#else
        std::string cmd = "chmod -R 777 " + test_root.string();
        int ret = std::system(cmd.c_str());
        assert(ret == 0);

        cmd = "rm -r " + test_root.string();
#endif
        ret = std::system(cmd.c_str());
        assert(ret == 0);
    }

    scoped_test_env(scoped_test_env const &) = delete;
    scoped_test_env & operator=(scoped_test_env const &) = delete;

    fs::path make_env_path(std::string p) { return sanitize_path(p); }

    std::string sanitize_path(std::string raw) {
        assert(raw.find("..") == std::string::npos);
        std::string root = test_root.string();
        if (root.compare(0, root.size(), raw, 0, root.size()) != 0) {
            assert(raw.front() != '\\');
            fs::path tmp(test_root);
            tmp /= raw;
            return tmp.string();
        }
        return raw;
    }

    // Purposefully using a size potentially larger than off_t here so we can
    // test the behavior of libc++fs when it is built with _FILE_OFFSET_BITS=64
    // but the caller is not (std::filesystem also uses uintmax_t rather than
    // off_t). On a 32-bit system this allows us to create a file larger than
    // 2GB.
    std::string create_file(fs::path filename_path, uintmax_t size = 0) {
        std::string filename = filename_path.string();
#if defined(__LP64__) || defined(_WIN32)
        auto large_file_fopen = fopen;
        auto large_file_ftruncate = utils::ftruncate;
        using large_file_offset_t = off_t;
#else
        auto large_file_fopen = fopen64;
        auto large_file_ftruncate = ftruncate64;
        using large_file_offset_t = off64_t;
#endif

        filename = sanitize_path(std::move(filename));

        if (size > std::numeric_limits<large_file_offset_t>::max()) {
            fprintf(stderr, "create_file(%s, %ju) too large\n",
                    filename.c_str(), size);
            abort();
        }

#ifndef _WIN32
#define FOPEN_CLOEXEC_FLAG "e"
#else
#define FOPEN_CLOEXEC_FLAG ""
#endif
        FILE* file = large_file_fopen(filename.c_str(), "w" FOPEN_CLOEXEC_FLAG);
        if (file == nullptr) {
            fprintf(stderr, "fopen %s failed: %s\n", filename.c_str(),
                    strerror(errno));
            abort();
        }

        if (large_file_ftruncate(
                fileno(file), static_cast<large_file_offset_t>(size)) == -1) {
            fprintf(stderr, "ftruncate %s %ju failed: %s\n", filename.c_str(),
                    size, strerror(errno));
            fclose(file);
            abort();
        }

        fclose(file);
        return filename;
    }

    std::string create_dir(fs::path filename_path) {
        std::string filename = filename_path.string();
        filename = sanitize_path(std::move(filename));
        int ret = utils::mkdir(filename.c_str(), 0777); // rwxrwxrwx mode
        assert(ret == 0);
        return filename;
    }

    std::string create_symlink(fs::path source_path,
                               fs::path to_path,
                               bool sanitize_source = true,
                               bool is_dir = false) {
        std::string source = source_path.string();
        std::string to = to_path.string();
        if (sanitize_source)
            source = sanitize_path(std::move(source));
        to = sanitize_path(std::move(to));
        int ret = utils::symlink(source.c_str(), to.c_str(), is_dir);
        assert(ret == 0);
        return to;
    }

    std::string create_hardlink(fs::path source_path, fs::path to_path) {
        std::string source = source_path.string();
        std::string to = to_path.string();
        source = sanitize_path(std::move(source));
        to = sanitize_path(std::move(to));
        int ret = utils::link(source.c_str(), to.c_str());
        assert(ret == 0);
        return to;
    }

#ifndef _WIN32
    std::string create_fifo(std::string file) {
        file = sanitize_path(std::move(file));
        int ret = ::mkfifo(file.c_str(), 0666); // rw-rw-rw- mode
        assert(ret == 0);
        return file;
    }
#endif

  // Some platforms doesn't support socket files so we shouldn't even
  // allow tests to call this unguarded.
#if !defined(__FreeBSD__) && !defined(__APPLE__) && !defined(_WIN32)
    std::string create_socket(std::string file) {
        file = sanitize_path(std::move(file));

        ::sockaddr_un address;
        address.sun_family = AF_UNIX;
        assert(file.size() <= sizeof(address.sun_path));
        ::strncpy(address.sun_path, file.c_str(), sizeof(address.sun_path));
        int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
        ::bind(fd, reinterpret_cast<::sockaddr*>(&address), sizeof(address));
        return file;
    }
#endif

    fs::path test_root;

private:
    // This could potentially introduce a filesystem race if multiple
    // scoped_test_envs were created concurrently in the same test (hence
    // sharing the same cwd). However, it is fairly unlikely to happen as
    // we generally don't use scoped_test_env from multiple threads, so
    // this is deemed acceptable.
    static inline fs::path available_cwd_path() {
        fs::path const cwd = utils::getcwd();
        fs::path const tmp = fs::temp_directory_path();
        fs::path const base = tmp / cwd.filename();
        int i = 0;
        fs::path p = base / ("static_env." + std::to_string(i));
        while (utils::exists(p.string())) {
            p = fs::path(base) / ("static_env." + std::to_string(++i));
        }
        return p;
    }
};

/// This class generates the following tree:
///
///     static_test_env
///     ├── bad_symlink -> dne
///     ├── dir1
///     │   ├── dir2
///     │   │   ├── afile3
///     │   │   ├── dir3
///     │   │   │   └── file5
///     │   │   ├── file4
///     │   │   └── symlink_to_dir3 -> dir3
///     │   ├── file1
///     │   └── file2
///     ├── empty_file
///     ├── non_empty_file
///     ├── symlink_to_dir -> dir1
///     └── symlink_to_empty_file -> empty_file
///
class static_test_env {
    scoped_test_env env_;
public:
    static_test_env() {
        env_.create_symlink("dne", "bad_symlink", false);
        env_.create_dir("dir1");
        env_.create_dir("dir1/dir2");
        env_.create_file("dir1/dir2/afile3");
        env_.create_dir("dir1/dir2/dir3");
        env_.create_file("dir1/dir2/dir3/file5");
        env_.create_file("dir1/dir2/file4");
        env_.create_symlink("dir3", "dir1/dir2/symlink_to_dir3", false, true);
        env_.create_file("dir1/file1");
        env_.create_file("dir1/file2", 42);
        env_.create_file("empty_file");
        env_.create_file("non_empty_file", 42);
        env_.create_symlink("dir1", "symlink_to_dir", false);
        env_.create_symlink("empty_file", "symlink_to_empty_file", false);
    }

    const fs::path Root = env_.test_root;

    fs::path makePath(fs::path const& p) const {
        // env_path is expected not to contain symlinks.
        fs::path const& env_path = Root;
        return env_path / p;
    }

    const std::vector<fs::path> TestFileList = {
        makePath("empty_file"),
        makePath("non_empty_file"),
        makePath("dir1/file1"),
        makePath("dir1/file2")
    };

    const std::vector<fs::path> TestDirList = {
        makePath("dir1"),
        makePath("dir1/dir2"),
        makePath("dir1/dir2/dir3")
    };

    const fs::path File          = TestFileList[0];
    const fs::path Dir           = TestDirList[0];
    const fs::path Dir2          = TestDirList[1];
    const fs::path Dir3          = TestDirList[2];
    const fs::path SymlinkToFile = makePath("symlink_to_empty_file");
    const fs::path SymlinkToDir  = makePath("symlink_to_dir");
    const fs::path BadSymlink    = makePath("bad_symlink");
    const fs::path DNE           = makePath("DNE");
    const fs::path EmptyFile     = TestFileList[0];
    const fs::path NonEmptyFile  = TestFileList[1];
    const fs::path CharFile      = "/dev/null"; // Hopefully this exists

    const std::vector<fs::path> DirIterationList = {
        makePath("dir1/dir2"),
        makePath("dir1/file1"),
        makePath("dir1/file2")
    };

    const std::vector<fs::path> DirIterationListDepth1 = {
        makePath("dir1/dir2/afile3"),
        makePath("dir1/dir2/dir3"),
        makePath("dir1/dir2/symlink_to_dir3"),
        makePath("dir1/dir2/file4"),
    };

    const std::vector<fs::path> RecDirIterationList = {
        makePath("dir1/dir2"),
        makePath("dir1/file1"),
        makePath("dir1/file2"),
        makePath("dir1/dir2/afile3"),
        makePath("dir1/dir2/dir3"),
        makePath("dir1/dir2/symlink_to_dir3"),
        makePath("dir1/dir2/file4"),
        makePath("dir1/dir2/dir3/file5")
    };

    const std::vector<fs::path> RecDirFollowSymlinksIterationList = {
        makePath("dir1/dir2"),
        makePath("dir1/file1"),
        makePath("dir1/file2"),
        makePath("dir1/dir2/afile3"),
        makePath("dir1/dir2/dir3"),
        makePath("dir1/dir2/file4"),
        makePath("dir1/dir2/dir3/file5"),
        makePath("dir1/dir2/symlink_to_dir3"),
        makePath("dir1/dir2/symlink_to_dir3/file5"),
    };
};

struct CWDGuard {
  std::string oldCwd_;
  CWDGuard() : oldCwd_(utils::getcwd()) { }
  ~CWDGuard() {
    int ret = ::chdir(oldCwd_.c_str());
    assert(ret == 0 && "chdir failed");
  }

  CWDGuard(CWDGuard const&) = delete;
  CWDGuard& operator=(CWDGuard const&) = delete;
};

// Misc test types

#define MKSTR(Str) {Str, TEST_CONCAT(L, Str), TEST_CONCAT(u, Str), TEST_CONCAT(U, Str)}

struct MultiStringType {
  const char* s;
  const wchar_t* w;
  const char16_t* u16;
  const char32_t* u32;

  operator const char* () const { return s; }
  operator const wchar_t* () const { return w; }
  operator const char16_t* () const { return u16; }
  operator const char32_t* () const { return u32; }
};

const MultiStringType PathList[] = {
        MKSTR(""),
        MKSTR(" "),
        MKSTR("//"),
        MKSTR("."),
        MKSTR(".."),
        MKSTR("foo"),
        MKSTR("/"),
        MKSTR("/foo"),
        MKSTR("foo/"),
        MKSTR("/foo/"),
        MKSTR("foo/bar"),
        MKSTR("/foo/bar"),
        MKSTR("//net"),
        MKSTR("//net/foo"),
        MKSTR("///foo///"),
        MKSTR("///foo///bar"),
        MKSTR("/."),
        MKSTR("./"),
        MKSTR("/.."),
        MKSTR("../"),
        MKSTR("foo/."),
        MKSTR("foo/.."),
        MKSTR("foo/./"),
        MKSTR("foo/./bar"),
        MKSTR("foo/../"),
        MKSTR("foo/../bar"),
        MKSTR("c:"),
        MKSTR("c:/"),
        MKSTR("c:foo"),
        MKSTR("c:/foo"),
        MKSTR("c:foo/"),
        MKSTR("c:/foo/"),
        MKSTR("c:/foo/bar"),
        MKSTR("prn:"),
        MKSTR("c:\\"),
        MKSTR("c:\\foo"),
        MKSTR("c:foo\\"),
        MKSTR("c:\\foo\\"),
        MKSTR("c:\\foo/"),
        MKSTR("c:/foo\\bar"),
        MKSTR("//"),
        MKSTR("/finally/we/need/one/really/really/really/really/really/really/really/long/string")
};
const unsigned PathListSize = sizeof(PathList) / sizeof(MultiStringType);

template <class Iter>
Iter IterEnd(Iter B) {
  using VT = typename std::iterator_traits<Iter>::value_type;
  for (; *B != VT{}; ++B)
    ;
  return B;
}

template <class CharT>
const CharT* StrEnd(CharT const* P) {
    return IterEnd(P);
}

template <class CharT>
std::size_t StrLen(CharT const* P) {
    return StrEnd(P) - P;
}

// Testing the allocation behavior of the code_cvt functions requires
// *knowing* that the allocation was not done by "path::__str_".
// This hack forces path to allocate enough memory.
inline void PathReserve(fs::path& p, std::size_t N) {
  auto const& native_ref = p.native();
  const_cast<fs::path::string_type&>(native_ref).reserve(N);
}

template <class Iter1, class Iter2>
bool checkCollectionsEqual(
    Iter1 start1, Iter1 const end1
  , Iter2 start2, Iter2 const end2
  )
{
    while (start1 != end1 && start2 != end2) {
        if (*start1 != *start2) {
            return false;
        }
        ++start1; ++start2;
    }
    return (start1 == end1 && start2 == end2);
}


template <class Iter1, class Iter2>
bool checkCollectionsEqualBackwards(
    Iter1 const start1, Iter1 end1
  , Iter2 const start2, Iter2 end2
  )
{
    while (start1 != end1 && start2 != end2) {
        --end1; --end2;
        if (*end1 != *end2) {
            return false;
        }
    }
    return (start1 == end1 && start2 == end2);
}

// We often need to test that the error_code was cleared if no error occurs
// this function returns an error_code which is set to an error that will
// never be returned by the filesystem functions.
inline std::error_code GetTestEC(unsigned Idx = 0) {
  using std::errc;
  auto GetErrc = [&]() {
    switch (Idx) {
    case 0:
      return errc::address_family_not_supported;
    case 1:
      return errc::address_not_available;
    case 2:
      return errc::address_in_use;
    case 3:
      return errc::argument_list_too_long;
    default:
      assert(false && "Idx out of range");
      std::abort();
    }
  };
  return std::make_error_code(GetErrc());
}

inline bool ErrorIsImp(const std::error_code& ec,
                       std::vector<std::errc> const& errors) {
  for (auto errc : errors) {
    if (ec == std::make_error_code(errc))
      return true;
  }
  return false;
}

template <class... ErrcT>
inline bool ErrorIs(const std::error_code& ec, std::errc First, ErrcT... Rest) {
  std::vector<std::errc> errors = {First, Rest...};
  return ErrorIsImp(ec, errors);
}

// Provide our own Sleep routine since std::this_thread::sleep_for is not
// available in single-threaded mode.
void SleepFor(std::chrono::seconds dur) {
    using namespace std::chrono;
#if defined(_LIBCPP_HAS_NO_MONOTONIC_CLOCK)
    using Clock = system_clock;
#else
    using Clock = steady_clock;
#endif
    const auto wake_time = Clock::now() + dur;
    while (Clock::now() < wake_time)
        ;
}

inline bool PathEq(fs::path const& LHS, fs::path const& RHS) {
  return LHS.native() == RHS.native();
}

struct ExceptionChecker {
  std::errc expected_err;
  fs::path expected_path1;
  fs::path expected_path2;
  unsigned num_paths;
  const char* func_name;
  std::string opt_message;

  explicit ExceptionChecker(std::errc first_err, const char* func_name,
                            std::string opt_msg = {})
      : expected_err{first_err}, num_paths(0), func_name(func_name),
        opt_message(opt_msg) {}
  explicit ExceptionChecker(fs::path p, std::errc first_err,
                            const char* func_name, std::string opt_msg = {})
      : expected_err(first_err), expected_path1(p), num_paths(1),
        func_name(func_name), opt_message(opt_msg) {}

  explicit ExceptionChecker(fs::path p1, fs::path p2, std::errc first_err,
                            const char* func_name, std::string opt_msg = {})
      : expected_err(first_err), expected_path1(p1), expected_path2(p2),
        num_paths(2), func_name(func_name), opt_message(opt_msg) {}

  void operator()(fs::filesystem_error const& Err) {
    TEST_CHECK(ErrorIsImp(Err.code(), {expected_err}));
    TEST_CHECK(Err.path1() == expected_path1);
    TEST_CHECK(Err.path2() == expected_path2);
    LIBCPP_ONLY(check_libcxx_string(Err));
  }

  void check_libcxx_string(fs::filesystem_error const& Err) {
    std::string message = std::make_error_code(expected_err).message();

    std::string additional_msg = "";
    if (!opt_message.empty()) {
      additional_msg = opt_message + ": ";
    }
    auto transform_path = [](const fs::path& p) {
      if (p.native().empty())
        return std::string("\"\"");
      return p.string();
    };
    std::string format = [&]() -> std::string {
      switch (num_paths) {
      case 0:
        return format_string("filesystem error: in %s: %s%s", func_name,
                             additional_msg, message);
      case 1:
        return format_string("filesystem error: in %s: %s%s [%s]", func_name,
                             additional_msg, message,
                             transform_path(expected_path1).c_str());
      case 2:
        return format_string("filesystem error: in %s: %s%s [%s] [%s]",
                             func_name, additional_msg, message,
                             transform_path(expected_path1).c_str(),
                             transform_path(expected_path2).c_str());
      default:
        TEST_CHECK(false && "unexpected case");
        return "";
      }
    }();
    TEST_CHECK(format == Err.what());
    if (format != Err.what()) {
      fprintf(stderr,
              "filesystem_error::what() does not match expected output:\n");
      fprintf(stderr, "  expected: \"%s\"\n", format.c_str());
      fprintf(stderr, "  actual:   \"%s\"\n\n", Err.what());
    }
  }

  ExceptionChecker(ExceptionChecker const&) = delete;
  ExceptionChecker& operator=(ExceptionChecker const&) = delete;

};

#endif /* FILESYSTEM_TEST_HELPER_HPP */
