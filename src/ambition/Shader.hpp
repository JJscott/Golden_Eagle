/*
 * Ambition Shader Manager
 *
 * Extra preprocessor directives:
 *		#include "..."		: include file relative to directory containing current file
 *		#include <...>		: include file relative to directories known to the shader manager
 *								#include resolves #version directives
 *
 *		#shader shader-type	: specify shader type(s) a file should be compiled as
 *								valid values for shader-type are:
 *								- vertex
 *								- fragment
 *								- geometry
 *								- tess_control
 *								- tess_evaluation
 *
 * The line numbers reported in compiler messages should be correct provided the compiler
 * follows the GLSL spec for the version in question regarding the #line directive.
 * The spec changed regarding this with GLSL 330 (to the best of my knowledge).
 * If the compiler uses the pre-330 behaviour for 330 or later code, line numbers will
 * be reported as 1 greater than they should be.
 *
 * @author Ben Allen
 *
 * TODO supply GLSL preprocessor definitions somehow
 * TODO suppose I link multiple frag shaders together, how do i tell them apart in program info log?
 * TODO better unload functions?
 * TODO #include, #shader and #version are processed regardless of #if etc
 *
 */

//
// If (eg with an AMD GPU) shader compilation fails regarding #line
// #define AMBITION_SHADER_NO_LINE_DIRECTIVES
// before including this file to prevent #line directives.
// This will mean line numbers will not be correct.
//

#ifndef AMBITION_SHADER_HPP
#define AMBITION_SHADER_HPP

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "Window.hpp"
#include "Log.hpp"

namespace ambition {

	class shader_error : public std::runtime_error {
	public:
		explicit inline shader_error(const std::string &what_ = "Generic shader error.") : std::runtime_error(what_) { }
	};

	class shader_type_error : public shader_error {
	public:
		explicit inline shader_type_error(const std::string &what_ = "Bad shader type.") : shader_error(what_) { }
	};

	class shader_compile_error : public shader_error {
	public:
		explicit inline shader_compile_error(const std::string &what_ = "Shader compilation failed.") : shader_error(what_) { }
	};

	class shader_link_error : public shader_error {
	public:
		explicit inline shader_link_error(const std::string &what_ = "Shader program linking failed.") : shader_error(what_) { }
	};

	inline void printShaderInfoLog(GLuint obj, bool error = false) {
		int infologLength = 0;
		int charsWritten = 0;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
		if (infologLength > 1) {
			std::vector<char> infoLog(infologLength);
			glGetShaderInfoLog(obj, infologLength, &charsWritten, &infoLog[0]);
			if (error) {
				log("ShaderMan").error() << &infoLog[0];
			} else {
				// TODO might not be warnings
				log("ShaderMan").warning() << &infoLog[0];
			}
		}
	}

	inline void printProgramInfoLog(GLuint obj, bool error = false) {
		int infologLength = 0;
		int charsWritten  = 0;
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
		if (infologLength > 1) {
			std::vector<char> infoLog(infologLength);
			glGetProgramInfoLog(obj, infologLength, &charsWritten, &infoLog[0]);
			if (error) {
				log("ShaderMan").error() << &infoLog[0];
			} else {
				// TODO might not be warnings
				log("ShaderMan").warning() << &infoLog[0];
			}
		}
	}

	inline GLuint compileShader(GLenum type, const std::string &text) {
		GLuint shader = glCreateShader(type);
		const char *text_c = text.c_str();
		glShaderSource(shader, 1, &text_c, nullptr);
		glCompileShader(shader);
		GLint compile_status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
		if (!compile_status) {
			printShaderInfoLog(shader, true);
			throw shader_compile_error();
		}
		// always print, so we can see warnings
		printShaderInfoLog(shader, false);
		return shader;
	}

	inline GLuint compileShader(GLenum type, std::istream &text_is) {
		std::string text;
		while (text_is.good()) {
			std::string line;
			std::getline(text_is, line);
			(text += line) += '\n';
		}
		return compileShader(type, text);
	}

	inline GLuint compileShaderFromFile(GLenum type, const std::string &path) {
		std::ifstream ifs(path.c_str());
		if (!ifs.good()) {
			std::string msg = "Error opening shader source '";
			msg += path;
			msg += "'.";
			throw shader_error(msg);
		}
		log("ShaderMan") << "Compiling '" << path << "'...";
		return compileShader(type, ifs);
	}

	inline void linkShaderProgram(GLuint prog) {
		glLinkProgram(prog);
		GLint link_status;
		glGetProgramiv(prog, GL_LINK_STATUS, &link_status);
		if (!link_status) {
			printProgramInfoLog(prog, true);
			throw shader_link_error();
		}
		// always print, so we can see warnings
		printProgramInfoLog(prog, false);
	}

	struct shader_version {
		unsigned id;
		std::string profile;
		
		inline explicit shader_version(unsigned id_, std::string profile_ = "") : id(id_), profile(profile_) { }
		
		inline bool operator==(const shader_version &rhs) const {
			return id == rhs.id && profile == rhs.profile;
		}
		
		inline bool operator!=(const shader_version &rhs) const {
			return !(*this == rhs);
		}
		
		inline friend std::ostream & operator<<(std::ostream &out, const shader_version &ver) {
			out << ver.id;
			if (!ver.profile.empty()) out << " " << ver.profile;
			return out;
		}
	};

	class ShaderManager {
	private:
		// not copyable etc
		ShaderManager(const ShaderManager &);
		ShaderManager & operator=(const ShaderManager &);

		struct shader_t {
			GLenum type;
			std::string name;
			GLuint id;
		};

		struct program_t {
			std::string cname;
			GLuint id;
		};

		// TODO use more intelligent data structures (maps)
		std::vector<std::string> m_shader_dirs;
		std::vector<std::string> m_auto_link;
		std::vector<shader_t> m_shaders;
		std::vector<program_t> m_programs;

		inline static void disassembleProgramName(const std::string &name, std::vector<std::string> &out) {
			size_t i = 0;
			while (i != std::string::npos) {
				size_t j = name.find(';', i);
				out.push_back(name.substr(i, j - i));
				if (j != std::string::npos) j++;
				i = j;
			}
		}

		inline static std::string strip(const std::string &str) {
			size_t offset = str.find_first_not_of(" \t\r\n");
			if (offset == std::string::npos) return "";
			size_t count = str.find_last_not_of(" \t\r\n") + 1 - offset;
			return str.substr(offset, count);
		}

		inline static std::string shaderTypeString(GLenum type) {
			switch (type) {
			case GL_VERTEX_SHADER:
				return "GL_VERTEX_SHADER";
			case GL_FRAGMENT_SHADER:
				return "GL_FRAGMENT_SHADER";
			case GL_GEOMETRY_SHADER:
				return "GL_GEOMETRY_SHADER";
			case GL_TESS_CONTROL_SHADER:
				return "GL_TESS_CONTROL_SHADER";
			case GL_TESS_EVALUATION_SHADER:
				return "GL_TESS_EVALUATION_SHADER";
			default:
				return "UNKNOWN";
			}
		}
		
		// make a line directive string, using pre-330 behaviour
		inline static std::string lineDirective(const shader_version &ver, unsigned line, unsigned source) {
			// to set next line to 1
			// by glsl-spec-1.30.8: #line 0
			// by glsl-spec-4.20.8: #line 1
			// changeover seems to be at version 330
			std::ostringstream oss;
#ifndef AMBITION_SHADER_NO_LINE_DIRECTIVES
			if (ver.id < 330) {
				oss << "#line " << line << " " << source;
			} else {
				oss << "#line " << (line + 1) << " " << source;
			}
#endif
			return oss.str();
		}

	public:
		// resolve duplicates, strip whitespace, ordering etc. does not account for auto-linking.
		inline static std::string canonicalProgramName(const std::string &name) {
			// remove duplicates and blanks
			std::vector<std::string> names, names2;
			disassembleProgramName(name, names);
			for (auto it = names.cbegin(); it != names.cend(); it++) {
				std::string name_stripped = strip(*it);
				if (!name_stripped.empty()) {
					// has non-whitespace
					if (std::find(names2.cbegin(), names2.cend(), name_stripped) == names2.cend()) {
						// not a dup
						names2.push_back(name_stripped);
					}
				}
			}
			// need to sort to avoid permutations
			std::sort(names2.begin(), names2.end());
			std::string cname;
			for (auto it = names2.cbegin(); it != names2.cend(); it++) {
				(cname += *it) += ';';
			}
			if (cname.length() > 0) cname = cname.substr(0, cname.length() - 1);
			return cname;
		}

		// assumed to always use '/' as separator
		inline void addSourceDirectory(const std::string &dir) {
			// FIXME more robust
			if (dir[dir.length() - 1] == '/') {
				m_shader_dirs.push_back(dir);
			} else {
				m_shader_dirs.push_back(dir + '/');
			}
		}

		// dont use this
		inline void autoLinkShader(const std::string &name) {
			m_auto_link.push_back(strip(name));
		}

		std::string resolveSourcePath(const std::string &name) {
			// find file
			for (auto it = m_shader_dirs.cbegin(); it != m_shader_dirs.cend(); it++) {
				std::string path = *it + name;
				std::ifstream ifs(path.c_str());
				if (ifs.good()) {
					// found it
					ifs.close();
					return path;
				}
			}
			// didnt find the file
			std::string msg = "Unable to find shader file '";
			msg += name;
			msg += "'.";
			throw shader_error(msg);
		}

		// process #include and #shader directives, and strip #version directives
		// returns the version number from this file
		// "" style includes are handled relative to the directory containing the shader being compiled
		// <> style includes are handled relative to source directories known to the ShaderManager
		inline shader_version preprocessShader(
			const std::string &path,
			std::ostream &text_os,
			std::vector<std::string> &source_names,
			std::vector<GLenum> &shader_types,
			std::ostream &log_os
		) {
			char cbuf[1024];
			shader_version ver = shader_version(110);
			bool have_ver = false;
			int source_id = source_names.size();
			source_names.push_back(path);
			int line_number = 1;

			// open file
			std::ifstream ifs(path.c_str());
			if (!ifs.good()) {
				std::string msg;
				msg += "Error opening file '";
				msg += path;
				msg += "'.";
				throw shader_error(msg);
			}

			// get cwd
			std::string cwd;
			size_t index = path.find_last_of('/');
			if (index != std::string::npos) {
				cwd = path.substr(0, index + 1);
			} else {
				// assume actual current directory
				cwd = "./";
			}

			// don't init line numbers till we have a #version

			while (ifs.good()) {
				// TODO like this, borked #includes will be passed to the compiler
				std::string line;
				std::getline(ifs, line);
				int v;

				if (std::sscanf(line.c_str(), " #version %d %s", &v, cbuf) == 1 && !have_ver) {
					// deal with #version version-id
					// this strips all #version directives, and only remembers the first
					ver.id = v;
					ver.profile = "";
					have_ver = true;
					// init line numbers
					text_os << lineDirective(ver, line_number, source_id) << '\n';

				} else if (std::sscanf(line.c_str(), " #version %d %s", &v, cbuf) == 2 && !have_ver) {
					// deal with #version version-id profile-name
					// this strips all #version directives, and only remembers the first
					ver.id = v;
					ver.profile = cbuf;
					have_ver = true;
					// init line numbers
					text_os << lineDirective(ver, line_number, source_id) << '\n';

				} else if (std::sscanf(line.c_str(), " #include \"%[^\"]\"", cbuf) > 0) {
					// deal with #include "..."
					// the negated charset is C99
					std::string path_inc = cwd + strip(cbuf);
					shader_version ver_inc = preprocessShader(path_inc, text_os, source_names, shader_types, log_os);
					if (ver != ver_inc) {
						log_os << "WARNING: \n'" << path_inc << "' (version " << ver_inc
							<< ") included by\n'" << path << "' (version " << ver << ")" << std::endl;
					}
					text_os << lineDirective(ver, line_number, source_id) << '\n';

				} else if (std::sscanf(line.c_str(), " #include <%[^>]>", cbuf) > 0) {
					// deal with #include <...>
					// the negated charset is C99
					std::string path_inc = resolveSourcePath(cbuf);
					shader_version ver_inc = preprocessShader(path_inc, text_os, source_names, shader_types, log_os);
					if (ver != ver_inc) {
						log_os << "WARNING: \n'" << path_inc << "' (version " << ver_inc
							<< ") included by\n'" << path << "' (version " << ver << ")" << std::endl;
					}
					text_os << lineDirective(ver, line_number, source_id) << '\n';

				} else if (std::sscanf(line.c_str(), " #shader %s", cbuf) > 0) {
					// deal with #shader - specifies shader type to compile as
					// TODO this silently ignores bad #shader directives
					std::string type_str = cbuf;
					if (type_str == "vertex") shader_types.push_back(GL_VERTEX_SHADER);
					if (type_str == "fragment") shader_types.push_back(GL_FRAGMENT_SHADER);
					if (type_str == "geometry") shader_types.push_back(GL_GEOMETRY_SHADER);
					if (type_str == "tess_control") shader_types.push_back(GL_TESS_CONTROL_SHADER);
					if (type_str == "tess_evaluation") shader_types.push_back(GL_TESS_EVALUATION_SHADER);
					text_os << '\n';
					
				} else {
					// ... its a normal glsl line
					text_os << line << '\n';
				}

				line_number++;
			}
			return ver;
		}

		inline GLuint getShader(GLenum type, const std::string &name, bool force_type = false) {
			std::string name_stripped = strip(name);

			// is it already compiled?
			for (auto it = m_shaders.cbegin(); it != m_shaders.cend(); it++) {
				if (it->type == type && it->name == name_stripped) {
					// yes, return id
					return it->id;
				}
			}

			// resolve path
			std::string path = resolveSourcePath(name_stripped);

			// preprocess
			std::ostringstream text_os_main;
			std::vector<std::string> source_names;
			std::vector<GLenum> shader_types;
			std::ostringstream log_os;
			shader_version version = preprocessShader(path, text_os_main, source_names, shader_types, log_os);
			
			if (!force_type && std::find(shader_types.cbegin(), shader_types.cend(), type) == shader_types.cend()) {
				// type being compiled was not declared (and the type isnt being forced)
				throw shader_type_error("Shader type being compiled was not declared.");
			}
			
			{
				auto compile_log = log("ShaderMan");

				if (!log_os.str().empty()) {
					// pre-pre-processor log isnt empty - warning
					compile_log.warning();
				}

				compile_log << "Compiling " << shaderTypeString(type) << " (" << version << ") '" << path << "'..." << std::endl;

				// display source string info
				for (size_t i = 0; i < source_names.size(); i++) {
					compile_log << i << "\t: " << source_names[i] << std::endl;
				}
			
				// display any output from pre-pre-processor
				compile_log << log_os.str();
			}
			
			// specify version and define type
			std::ostringstream text_os;
			text_os << "#version " << version << '\n';
			switch(type) {
			case GL_VERTEX_SHADER:
				text_os << "#define _VERTEX_\n";
				break;
			case GL_FRAGMENT_SHADER:
				text_os << "#define _FRAGMENT_\n";
				break;
			case GL_GEOMETRY_SHADER:
				text_os << "#define _GEOMETRY_\n";
				break;
			case GL_TESS_CONTROL_SHADER:
				text_os << "#define _TESS_CONTROL_\n";
				break;
			case GL_TESS_EVALUATION_SHADER:
				text_os << "#define _TESS_EVALUATION_\n";
				break;
			default:
				throw shader_error("Unknown shader type.");
			}

			// append preprocessed source
			text_os << text_os_main.str();
			
			// now, lets compile!
			GLuint id = compileShader(type, text_os.str());
			
			// add to cache
			shader_t shader;
			shader.type = type;
			shader.name = name_stripped;
			shader.id = id;
			m_shaders.push_back(shader);
			return id;
		}

		// name is a semicolon separated list of shader names
		inline GLuint getProgram(const std::string &name) {
			std::string full_name = name;
			// assemble name including auto-links
			for (auto it = m_auto_link.cbegin(); it != m_auto_link.cend(); it++) {
				(full_name += ';') += *it;
			}
			return getProgramNoAutoLink(full_name);
		}

		// name is a semicolon separated list of shader names, no shaders will be automagically linked
		inline GLuint getProgramNoAutoLink(const std::string & name) {
			std::string cname = canonicalProgramName(name);

			// is it already linked?
			for (auto it = m_programs.cbegin(); it != m_programs.cend(); it++) {
				if (it->cname == cname) {
					// yes, return id
					return it->id;
				}
			}

			// nope, compile as necessary then link
			GLuint id = glCreateProgram();
			std::vector<std::string> shader_names;
			disassembleProgramName(cname, shader_names);
			for (auto it = shader_names.cbegin(); it != shader_names.cend(); it++) {
				size_t i = it->find_last_of(".");
				std::string ext = it->substr(i);
				// if the ext is for a specific type, only compile as that type
				if (ext == ".vert") {
					glAttachShader(id, getShader(GL_VERTEX_SHADER, *it, true));
				} else if (ext == ".frag") {
					glAttachShader(id, getShader(GL_FRAGMENT_SHADER, *it, true));
				} else if (ext == ".geom") {
					glAttachShader(id, getShader(GL_GEOMETRY_SHADER, *it, true));
				} else {
					// unable to determine type from extension, try everything
					static const std::vector<GLenum> types {
						GL_VERTEX_SHADER,
						GL_FRAGMENT_SHADER,
						GL_GEOMETRY_SHADER,
						GL_TESS_CONTROL_SHADER,
						GL_TESS_EVALUATION_SHADER
					};
					for (GLenum type : types) {
						try {
							glAttachShader(id, getShader(type, *it, false));
						} catch (shader_type_error &e) {
							// type wasnt declared in source, skip
						}
					}
				}
			}

			log("ShaderMan") << "Linking shader program '" << cname << "'...";
			linkShaderProgram(id);
			log("ShaderMan") << "Shader program compiled and linked successfully.";

			// cache it
			program_t program;
			program.cname = cname;
			program.id = id;
			m_programs.push_back(program);
			return id;
		}

		// unload / delete all shaders and programs
		inline void unloadAll() {
			glUseProgram(0);
			// delete programs
			for (auto it = m_programs.cbegin(); it != m_programs.cend(); it++) {
				glDeleteProgram(it->id);
			}
			// delete shaders
			for (auto it = m_shaders.cbegin(); it != m_shaders.cend(); it++) {
				glDeleteShader(it->id);
			}
			// clear cache
			m_shaders.clear();
			m_programs.clear();
		}
		
		// get a vector of all loaded program names
		std::vector<std::string> getLoadedProgramNames() {
			std::vector<std::string> prog_names;
			for (const program_t & prog : m_programs) {
				prog_names.push_back(prog.cname);
			}
			return prog_names;
		}

		// ctor takes a source directory
		inline ShaderManager(const std::string &dir) {
			addSourceDirectory(dir);
		}
	};

}

#endif // AMBITION_SHADER_HPP
