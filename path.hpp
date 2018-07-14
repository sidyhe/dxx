//////////////////////////////////////////////////////////////////////////
// all code from: https://github.com/dlecocq/apathy

#ifndef _APATHY_PATH_HPP_
#define _APATHY_PATH_HPP_

/* C++ includes */

#ifndef _EASTL
#include <vector>
#include <string>
#else
#include <eastl/vector.h>
#include <eastl/string.h>
#endif // _EASTL

/* A class for path manipulation */
namespace apathy {

	namespace stlx {
#ifndef _EASTL
		using string = std::string;
		template<typename T>
		using vector = std::vector<T>;
#else
		using string = eastl::string;
		template<typename T>
		using vector = eastl::vector<T>;
#endif // _EASTL
	}

	class Path {
	public:
		/* This is the separator used on this particular system */
#ifdef __MSDOS__
#error "Platforms using backslashes not yet supported"
#else
		static const char separator = '/';
#endif
		/* A class meant to contain path segments */
		struct Segment {
			/* The actual string segment */
			stlx::string segment;

			Segment(stlx::string s = "") : segment(s) {}
		};

		/**********************************************************************
		* Constructors
		*********************************************************************/

		/* Default constructor
		*
		* Points to current directory */
		Path(const stlx::string& path = "") : path(path) {}

		/* Our generalized constructor.
		*
		* This enables all sorts of type promotion (like int -> Path) for
		* arguments into all the functions below. Anything that
		* std::stringstream can support is implicitly supported as well
		*
		* @param p - path to construct */
		Path(const char* str);

		/**********************************************************************
		* Operators
		*********************************************************************/
		/* Checks if the paths are exactly the same */
		bool operator==(const Path& other) { return path == other.path; }

		/* Check if the paths are not exactly the same */
		bool operator!=(const Path& other) { return !(*this == other); }

		/* Append the provided segment to the path as a directory. This is the
		* same as append(segment)
		*
		* @param segment - path segment to add to this path */
		Path& operator<<(const Path& segment);

		/* Append the provided segment to the path as a directory. This is the
		* same as append(segment). Returns a /new/ path object rather than a
		* reference.
		*
		* @param segment - path segment to add to this path */
		Path operator+(const Path& segment) const;

		/* Check if the two paths are equivalent
		*
		* Two paths are equivalent if they point to the same resource, even if
		* they are not exact string matches
		*
		* @param other - path to compare to */
		bool equivalent(const Path& other);

		/* Return a string version of this path */
		stlx::string string() const { return path; }

		/* Return the name of the file */
		stlx::string filename() const;

		/* Return the extension of the file */
		stlx::string extension() const;

		/* Return a path object without the extension */
		Path stem() const;

		/**********************************************************************
		* Manipulations
		*********************************************************************/

		/* Append the provided segment to the path as a directory. Alias for
		* `operator<<`
		*
		* @param segment - path segment to add to this path */
		Path& append(const Path& segment);

		/* Evaluate the provided path relative to this path. If the second path
		* is absolute, then return the second path.
		*
		* @param rel - path relative to this path to evaluate */
		Path& relative(const Path& rel);

		/* Move up one level in the directory structure */
		Path& up();

		/* Turn this into an absolute path
		*
		* If the path is already absolute, it has no effect. Otherwise, it is
		* evaluated relative to the current working directory */
		Path& absolute();

		/* Sanitize this path
		*
		* This...
		*
		* 1) replaces runs of consecutive separators with a single separator
		* 2) evaluates '..' to refer to the parent directory, and
		* 3) strips out '/./' as referring to the current directory
		*
		* If the path was absolute to begin with, it will be absolute
		* afterwards. If it was a relative path to begin with, it will only be
		* converted to an absolute path if it uses enough '..'s to refer to
		* directories above the current working directory */
		Path& sanitize();

		/* Make this path a directory
		*
		* If this path does not have a trailing directory separator, add one.
		* If it already does, this does not affect the path */
		Path& directory();

		/* Trim this path of trailing separators, up to the leading separator.
		* For example, on *nix systems:
		*
		*   assert(Path("///").trim() == "/");
		*   assert(Path("/foo//").trim() == "/foo");
		*/
		Path& trim();

		/**********************************************************************
		* Copiers
		*********************************************************************/

		/* Return parent path
		*
		* Returns a new Path object referring to the parent directory. To
		* move _this_ path to the parent directory, use the `up` function */
		Path parent() const { return Path(Path(*this).up()); }

		/**********************************************************************
		* Member Utility Methods
		*********************************************************************/

		/* Returns a vector of each of the path segments in this path */
		stlx::vector<Segment> split() const;

		/**********************************************************************
		* Type Tests
		*********************************************************************/
		/* Is the path an absolute path? */
		bool is_absolute() const;

		/* Does the path have a trailing slash? */
		bool trailing_slash() const;

		/**********************************************************************
		* Static Utility Methods
		*********************************************************************/

		/* Return a brand new path as the concatenation of the two provided
		* paths
		*
		* @param a - first part of the path to join
		* @param b - second part of the path to join
		*/
		static Path join(const Path& a, const Path& b);

		/* Return a branch new path as the concatenation of each segments
		*
		* @param segments - the path segments to concatenate
		*/
		static Path join(const stlx::vector<Segment>& segments);

		/* Current working directory */
		static Path cwd();

	private:
		/* Our current path */
		stlx::string path;
	};

	/* Constructor */
	inline Path::Path(const char* str) : path(str) {

	}

	/**************************************************************************
	* Operators
	*************************************************************************/
	inline Path& Path::operator<<(const Path& segment) {
		return append(segment);
	}

	inline Path Path::operator+(const Path& segment) const {
		Path result(path);
		result.append(segment);
		return result;
	}

	inline bool Path::equivalent(const Path& other) {
		/* Make copies of both paths, sanitize, and ensure they're equal */
		return Path(path).absolute().sanitize() ==
			Path(other).absolute().sanitize();
	}

	inline stlx::string Path::filename() const {
		size_t pos = path.rfind(separator);
		if (pos != stlx::string::npos) {
			return path.substr(pos + 1);
		}
		return "";
	}

	inline stlx::string Path::extension() const {
		/* Make sure we only look in the filename, and not the path */
		stlx::string name = filename();
		size_t pos = name.rfind('.');
		if (pos != stlx::string::npos) {
			return name.substr(pos + 1);
		}
		return "";
	}

	inline Path Path::stem() const {
		size_t sep_pos = path.rfind(separator);
		size_t dot_pos = path.rfind('.');
		if (dot_pos == stlx::string::npos) {
			return Path(*this);
		}

		if (sep_pos == stlx::string::npos || sep_pos < dot_pos) {
			return Path(path.substr(0, dot_pos));
		}
		else {
			return Path(*this);
		}
	}

	/**************************************************************************
	* Manipulators
	*************************************************************************/
	inline Path& Path::append(const Path& segment) {
		/* First, check if the last character is the separator character.
		* If not, then append one and then the segment. Otherwise, just
		* the segment */
		if (!trailing_slash()) {
			path.push_back(separator);
		}
		path.append(segment.path);
		return *this;
	}

	inline Path& Path::relative(const Path& rel) {
		if (!rel.is_absolute()) {
			return append(rel);
		}
		else {
			operator=(rel);
			return *this;
		}
	}

	inline Path& Path::up() {
		/* Make sure we turn this into an absolute url if it's not already
		* one */
		if (path.size() == 0) {
			path = "..";
			return directory();
		}

		append("..").sanitize();
		if (path.size() == 0) {
			return *this;
		}
		return directory();
	}

	inline Path& Path::absolute() {
		/* If the path doesn't begin with our separator, then it's not an
		* absolute path, and should be appended to the current working
		* directory */
		if (!is_absolute()) {
			/* Join our current working directory with the path */
			operator=(join(cwd(), path));
		}
		return *this;
	}

	inline Path& Path::sanitize() {
		/* Split the path up into segments */
		stlx::vector<Segment> segments(split());
		/* We may have to test this repeatedly, so let's check once */
		bool is_relative = !is_absolute();

		/* Now, we'll create a new set of segments */
		stlx::vector<Segment> pruned;
		for (size_t pos = 0; pos < segments.size(); ++pos) {
			/* Skip over empty segments and '.' */
			if (segments[pos].segment.size() == 0 ||
				segments[pos].segment == ".") {
				continue;
			}

			/* If there is a '..', then pop off a parent directory. However, if
			* the path was relative to begin with, if the '..'s exceed the
			* stack depth, then they should be appended to our path. If it was
			* absolute to begin with, and we reach root, then '..' has no
			* effect */
			if (segments[pos].segment == "..") {
				if (is_relative) {
					if (pruned.size() && pruned.back().segment != "..") {
						pruned.pop_back();
					}
					else {
						pruned.push_back(segments[pos]);
					}
				}
				else if (pruned.size()) {
					pruned.pop_back();
				}
				continue;
			}

			pruned.push_back(segments[pos]);
		}

		bool was_directory = trailing_slash();
		if (!is_relative) {
			path = stlx::string(1, separator) + Path::join(pruned).path;
			if (was_directory) {
				return directory();
			}
			return *this;
		}

		/* It was a relative path */
		path = Path::join(pruned).path;
		if (path.length() && was_directory) {
			return directory();
		}
		return *this;
	}

	inline Path& Path::directory() {
		trim();
		path.push_back(separator);
		return *this;
	}

	inline Path& Path::trim() {
		if (path.length() == 0) { return *this; }

		size_t p = path.find_last_not_of(separator);
		if (p != stlx::string::npos) {
			path.erase(p + 1, path.size());
		}
		else {
			path = "";
		}
		return *this;
	}

	/**************************************************************************
	* Member Utility Methods
	*************************************************************************/

	/* Returns a vector of each of the path segments in this path */
	inline stlx::vector<Path::Segment> Path::split() const {

		stlx::string t;
		stlx::vector<Path::Segment> results;

		for (char c : path)
		{
			if (c == separator)
			{
				results.push_back(t);
				t.clear();
			}
			else
			{
				t.append(1, c);
			}
		}
		if (!t.empty())
			results.push_back(t);

		if (trailing_slash()) {
			results.push_back(Path::Segment(""));
		}
		return results;
	}

	/**************************************************************************
	* Tests
	*************************************************************************/
	inline bool Path::is_absolute() const {
		return path.size() && path[0] == separator;
	}

	inline bool Path::trailing_slash() const {
		return path.size() && path[path.length() - 1] == separator;
	}

	/**************************************************************************
	* Static Utility Methods
	*************************************************************************/
	inline Path Path::join(const Path& a, const Path& b) {
		Path p(a);
		p.append(b);
		return p;
	}

	inline Path Path::join(const stlx::vector<Segment>& segments) {
		stlx::string path;
		/* Now, we'll go through the segments, and join them with
		* separator */
		stlx::vector<Segment>::const_iterator it(segments.begin());
		for (; it != segments.end(); ++it) {
			path += it->segment;
			if (it + 1 != segments.end()) {
				path += stlx::string(1, separator);
			}
		}
		return Path(path);
	}

	inline Path Path::cwd() {
		Path p("/");

		/* Ensure this is a directory */
		p.directory();
		return p;
	}
}

#endif
