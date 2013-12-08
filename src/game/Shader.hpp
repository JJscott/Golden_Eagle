/*
 * Initial3D Shader Helper (GL2)
 *
 * @author Ben Allen
 *
 * TODO - suppose I link multiple frag shaders together, how do i tell them apart in program info log?
 * TODO - better unload functions
 *
 */

#ifndef INITIAL3D_SHADER_H
#define INITIAL3D_SHADER_H

#include "GLee.h"
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace initial3d {

	class shader_error : public std::runtime_error {
	public:
		explicit inline shader_error(const std::string &what_ = "Generic shader error.") : std::runtime_error(what_) { }
	};

	class shader_compile_error : public shader_error {
	public:
		explicit inline shader_compile_error(const std::string &what_ = "Shader compilation failed.") : shader_error(what_) { }
	};

	class shader_link_error : public shader_error {
	public:
		explicit inline shader_link_error(const std::string &what_ = "Shader program linking failed.") : shader_error(what_) { }
	};

	inline void printShaderInfoLog(GLuint obj) {
		int infologLength = 0;
		int charsWritten = 0;
		glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
		if (infologLength > 1) {
			char *infoLog = new char[infologLength];
			glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
			std::cout << infoLog << std::endl;
			delete[] infoLog;
		}
	}

	inline void printProgramInfoLog(GLuint obj) {
		int infologLength = 0;
		int charsWritten  = 0;
		glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
		if (infologLength > 1) {
			char *infoLog = new char[infologLength];
			glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
			std::cout << infoLog << std::endl;
			delete[] infoLog;
		}
	}

	inline GLuint compileShader(GLenum type, const std::string &text) {
		GLuint shader = glCreateShader(type);
		const char *text_c = text.c_str();
		glShaderSource(shader, 1, &text_c, NULL);
		glCompileShader(shader);
		// always print, so we can see warnings
		printShaderInfoLog(shader);
		GLint compile_status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
		if (!compile_status) {
			throw shader_compile_error();
		}
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
			throw shader_compile_error(msg);
		}
		std::cout << "Compiling '" << path << "'..." << std::endl;
		return compileShader(type, ifs);
	}

	inline void linkShaderProgram(GLuint prog) {
		glLinkProgram(prog);
		// always print, so we can see warnings
		printProgramInfoLog(prog);
		GLint link_status;
		glGetProgramiv(prog, GL_LINK_STATUS, &link_status);
		if (!link_status) {
			throw shader_link_error();
		}
	}

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

	public:
		inline static void disassembleProgramName(const std::string &name, std::vector<std::string> &out) {
			int i = 0;
			while (i != std::string::npos) {
				int j = name.find(';', i);
				out.push_back(name.substr(i, j - i));
				if (j != std::string::npos) j++;
				i = j;
			}
		}

		// resolve duplicates, strip whitespace, ordering etc. does not account for auto-linking.
		inline static std::string canonicalProgramName(const std::string &name) {
			// remove duplicates and blanks, sort
			std::vector<std::string> names, names2;
			disassembleProgramName(name, names);
			for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); it++) {
				if (it->find_first_not_of(' ') != std::string::npos) {
					// has non-whitespace (and isnt empty)
					if (std::find(names2.begin(), names2.end(), *it) == names2.end()) {
						// not a dup
						int offset = it->find_first_not_of(" \t\r\n");
						int count = it->find_last_not_of(" \t\r\n") + 1 - offset;
						names2.push_back(it->substr(offset, count));
					}
				}
			}
			// need to sort to avoid permutations
			std::sort(names2.begin(), names2.end());
			std::string cname;
			for (std::vector<std::string>::const_iterator it = names2.begin(); it != names2.end(); it++) {
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

		inline void autoLinkShader(const std::string &name) {
			m_auto_link.push_back(name);
		}

		std::string resolveSourcePath(const std::string &name) {
			// nope, find file
			for (std::vector<std::string>::const_iterator it = m_shader_dirs.begin(); it != m_shader_dirs.end(); it++) {
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
			throw shader_compile_error(msg);
		}

		// process #include directives, and strip #version directives
		// returns the version number from this file
		// "" style includes are handled relative to the directory containing the shader being compiled
		// <> style includes are handled relative to source directories known to the ShaderManager
		inline int preprocessShader(const std::string &path, std::ostream &text_os, std::vector<std::string> &source_names) {
			char cbuf[1024];
			int version = 110;
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
				throw shader_compile_error(msg);
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

			// init line numbers
			text_os << "#line 0 " << source_id << '\n';

			while (ifs.good()) {
				// TODO like this, borked #includes will be passed to the compiler
				std::string line;
				std::getline(ifs, line);
				int v;
				if (sscanf(line.c_str(), " #version %d", &v) > 0 && version == 110) {
					// deal with #version
					// this strips all #version directives, and only remembers the first
					version = v;
					text_os << '\n';
				} else if (sscanf(line.c_str(), " #include \"%[^\"]\"", cbuf) > 0) {
					// deal with #include "..."
					// TODO ? the negated charset is C99
					std::string path_inc = cwd + cbuf;
					int ver_inc = preprocessShader(path_inc, text_os, source_names);
					if (version != ver_inc) {
						std::cerr << "WARNING: #version of '" << path_inc << "' (" << ver_inc
							<< ") != #version of includer '" << path << "' (" << version << ")" << std::endl; 
					}
					text_os << "#line " << line_number << " " << source_id << '\n';
				} else if (sscanf(line.c_str(), " #include <%[^>]>", cbuf) > 0) {
					// deal with #include <...>
					// TODO ? the negated charset is C99
					std::string path_inc = resolveSourcePath(cbuf);
					int ver_inc = preprocessShader(path_inc, text_os, source_names);
					if (version != ver_inc) {
						std::cerr << "WARNING: #version of '" << path_inc << "' (" << ver_inc
							<< ") != #version of includer '" << path << "' (" << version << ")" << std::endl; 
					}
					text_os << "#line " << line_number << " " << source_id << '\n';
				} else {
					// ... its a normal glsl line
					text_os << line << '\n';
				}
				line_number++;
			}
			return version;
		}

		inline GLuint getShader(GLenum type, const std::string &name) {
			// is it already compiled?
			for (std::vector<shader_t>::const_iterator it = m_shaders.begin(); it != m_shaders.end(); it++) {
				if (it->type == type && it->name == name) {
					// yes, return id
					return it->id;
				}
			}
			// resolve path
			std::string path = resolveSourcePath(name);
			std::cout << "Compiling '" << path << "'..." << std::endl;
			// preprocess
			std::ostringstream text_os_main;
			std::vector<std::string> source_names;
			int version = preprocessShader(path, text_os_main, source_names);
			// prepend version and define type
			std::ostringstream text_os_head;
			text_os_head << "#version " << version << '\n';
			switch(type) {
			case GL_VERTEX_SHADER:
				text_os_head << "#define _VERTEX_\n";
				break;
			case GL_FRAGMENT_SHADER:
				text_os_head << "#define _FRAGMENT_\n";
				break;
			case GL_GEOMETRY_SHADER:
				text_os_head << "#define _GEOMETRY_\n";
				break;
			// TODO other shader types
			default:
				// wat do?
				break;
			}
			// append preprocessed source
			text_os_head << text_os_main.str();
			// display source string info
			for (int i = 0; i < source_names.size(); i++) {
				std::cout << i << "\t: " << source_names[i] << std::endl;
			}
			// now, lets compile!
			GLuint id = compileShader(type, text_os_head.str());
			// add to cache
			shader_t shader;
			shader.type = type;
			shader.name = name;
			shader.id = id;
			m_shaders.push_back(shader);
			return id;
		}

		// name is a semicolon separated list of shader names
		inline GLuint getProgram(const std::string &name) {
			std::string full_name = name;
			// assemble name including auto-links
			for (std::vector<std::string>::const_iterator it = m_auto_link.begin(); it != m_auto_link.end(); it++) {
				(full_name += ';') += *it;
			}
			return getProgramNoAutoLink(full_name);
		}

		// name is a semicolon separated list of shader names, no shaders will be automagically linked
		inline GLuint getProgramNoAutoLink(const std::string & name) {
			std::string cname = canonicalProgramName(name);
			// is it already linked?
			for (std::vector<program_t>::const_iterator it = m_programs.begin(); it != m_programs.end(); it++) {
				if (it->cname == cname) {
					// yes, return id
					return it->id;
				}
			}
			// nope, compile as necessary then link
			GLuint id = glCreateProgram();
			std::vector<std::string> shader_names;
			disassembleProgramName(cname, shader_names);
			for (std::vector<std::string>::const_iterator it = shader_names.begin(); it != shader_names.end(); it++) {
				int i = it->find_last_of(".");
				std::string ext = it->substr(i);
				if (ext == ".vert" || ext == ".vertex" || ext == ".vp") {
					glAttachShader(id, getShader(GL_VERTEX_SHADER, *it));
				} else if (ext == ".frag" || ext == ".fragment" || ext == ".fp") {
					glAttachShader(id, getShader(GL_FRAGMENT_SHADER, *it));
				} else if (ext == ".geom" || ext == ".geometry" || ext == ".gp") {
					glAttachShader(id, getShader(GL_GEOMETRY_SHADER, *it));
				}
				// TODO other shader types
				else {
					// unable to determine type from extension, try everything
					glAttachShader(id, getShader(GL_VERTEX_SHADER, *it));
					glAttachShader(id, getShader(GL_FRAGMENT_SHADER, *it));
					if (GLEE_ARB_geometry_shader4) {
						// only try geometry shader if supported
						glAttachShader(id, getShader(GL_GEOMETRY_SHADER, *it));
					}
					// TODO other shader types
				}
			}
			std::cout << "Linking shader program '" << cname << "'..." << std::endl;
			linkShaderProgram(id);
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
			for (std::vector<program_t>::const_iterator it = m_programs.begin(); it != m_programs.end(); it++) {
				glDeleteProgram(it->id);
			}
			// delete shaders
			for (std::vector<shader_t>::const_iterator it = m_shaders.begin(); it != m_shaders.end(); it++) {
				glDeleteShader(it->id);
			}
			// clear cache
			m_shaders.clear();
			m_programs.clear();
		}

		inline ShaderManager(const std::string &dir) {
			addSourceDirectory(dir);
		}
	};

}

#endif // INITIAL3D_SHADER_H
