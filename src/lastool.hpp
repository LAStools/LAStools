/*
===============================================================================

  FILE:  lastool.hpp

  CONTENTS:

    common lastool content

  PROGRAMMERS:

    info@rapidlasso.de

  COPYRIGHT:

    (c) 2009-2024, rapidlasso GmbH - fast tools to catch reality

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

    01 Mai 2024 - initial

===============================================================================
*/
#ifndef LASTOOL_HPP
#define LASTOOL_HPP

#include "lasdefinitions.hpp"
#include "lasmessage.hpp"
#include "mydefs.hpp"

#include <cstdio>
#include <cstring>
#include <string>
#include <filesystem>

class LasTool
{
   private:
    bool header_printed_once = false;

   public:
    virtual ~LasTool() = default;
    int argc = 0;
    char** argv = 0;
    bool force = false;  // force continuation on serious_warnings
    bool blast = false;
#ifdef COMPILE_WITH_GUI
    bool gui = false;
#endif
#ifdef COMPILE_WITH_MULTI_CORE
    I32 cores = 1;
    BOOL cpu64 = FALSE;
#endif
    std::string name;
    void init(int argc, char** argv, std::string name)
    {
        this->argc = argc;
        this->argv = argv;
        this->name = name;
        // optional: set default dir to exe
        // std::filesystem::current_path(exe_path().c_str());
    }

    virtual std::string sBlast()
    {
        return (blast ? " BLAST" : "");
    }

    virtual void lastitle(LAS_MESSAGE_TYPE type, std::string info = "")
    {
        if (info.empty())
        {
            info = laslicinfo();
        }
        if (!header_printed_once)
        {
            LASMessage(type, "LAStools%s %s (by info@rapidlasso.de) version %d%s\n", sBlast().c_str(), name.c_str(), LAS_TOOLS_VERSION, info.c_str());
            header_printed_once = type >= get_message_log_level();
        }
    }

    virtual std::string laslicinfo()
    {
        return "";
    }

    void laserrorusage(LAS_FORMAT_STRING(const char*) fmt, ...)
    {
        laserror(fmt);
        usage();
        byebye();
    }

    void laswarnforce(LAS_FORMAT_STRING(const char*) fmt, ...) const
    {
        LASMessage(LAS_SERIOUS_WARNING, fmt);
        if (!force)
        {
            byebye();
        }
    }

    virtual void usage()
    {
    }

    /// <summary>
    /// handles the argument[i], first step (arguments which maybe influences other arguments or general behaviour)
    /// optional passive mode: just check, if the argument is valid
    /// </summary>
    /// <param name="i">argument index</param>
    /// <returns>true: argument[i] valid; false: argument[i] not defined</returns>
    virtual bool parse_pre(int& i, bool active)
    {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-verbose") == 0)
        {
            if ((active) && (get_message_log_level() > LAS_VERBOSE))
            {
                set_message_log_level(LAS_VERBOSE);
            }
        }
        else if (strcmp(argv[i], "-vv") == 0 || strcmp(argv[i], "-very_verbose") == 0)
        {
            if (active)
                set_message_log_level(LAS_VERY_VERBOSE);
        }
        else if (strcmp(argv[i], "-quiet") == 0)
        {
            if (active)
                set_message_log_level(LAS_QUIET);
        }
        else if (strcmp(argv[i], "-silent") == 0)
        {
            if (active)
                set_message_log_level(LAS_WARNING);
        }
        else if (strcmp(argv[i], "-errors_ignore") == 0)
        {
            if (active)
                halt_on_error(false);
        }
        else if (strcmp(argv[i], "-force") == 0)
        {
            if (active)
                force = true;
        }
        else if (strcmp(argv[i], "-print_log_stats") == 0)
        {
            if (active)
                print_log_stats();
        }
        else
        {
            return false;
        }
        return true;
    }
    /// <summary>
    /// handles the argument[i]
    /// </summary>
    /// <param name="i">argument index</param>
    /// <returns>true: argument[i] valid; false: argument[i] not defined</returns>
    virtual bool parse_base(int& i)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-hh") == 0 || strcmp(argv[i], "-help") == 0)
        {
            lastitle(LAS_INFO);
            usage();
            if (strcmp(argv[i], "-hh") != 0)
            {
                LASMessage(LAS_INFO, "use '-hh' or see %s_README.md to get extended help\n", name.c_str());
            }
            byebye();
        }
        else if (strcmp(argv[i], "-version") == 0)
        {
            lastitle(LAS_INFO);
            byebye();
        }
        else if (strcmp(argv[i], "-gui") == 0)
        {
#ifdef COMPILE_WITH_GUI
            gui = true;
#else
            LASMessage(LAS_WARNING, "not compiled with GUI support. ignoring '-gui' ...");
#endif
        }
        else if (strcmp(argv[i], "-cores") == 0)
        {
#ifdef COMPILE_WITH_MULTI_CORE
            if ((i + 1) >= argc)
            {
                laserrorusage("'%s' needs 1 argument: number", argv[i]);
            }
            if (sscanf(argv[i + 1], "%u", &cores) != 1)
            {
                laserror("cannot understand argument '%s' for '%s'", argv[i + 1], argv[i]);
                usage();
                byebye();
            }
            argv[i][0] = '\0';
            i++;
            argv[i][0] = '\0';
#else
            LASMessage(LAS_WARNING, "not compiled with multi-core batching. ignoring '-cores' ...");
            i++;
#endif
        }
        else if (strcmp(argv[i], "-cpu64") == 0)
        {
#ifdef COMPILE_WITH_MULTI_CORE
            cpu64 = TRUE;
#else
            LASMessage(LAS_WARNING, "not compiled with 64 bit support. ignoring '-cpu64' ...");
#endif
            argv[i][0] = '\0';
        }
        else
        {
            return false;
        }
        return true;
    }
    virtual void after_parse() {}
    template <typename parseitm>
    void parse(parseitm x)
    {
        // 1st parse
        int i = 1;
        while (i < argc)
        {
            if (argv[i][0] != '\0')
            {
                parse_pre(i, true);
            }
            i++;
        }
        // print title in verbose mode
        lastitle(LAS_VERBOSE);
        // 2nd parse
        i = 1;
        while (i < argc)
        {
            if (argv[i][0] != '\0')
            {
                if (!x(i) && !parse_base(i) && !parse_pre(i, false))
                {
                    laserror("cannot understand argument '%s'. use '-h' or see %s_README.md to get help.", argv[i], name.c_str());
                }
            }
            i++;
        }
        //
        after_parse();
        force_check();
    }
    void laserrorinfo(std::string pre) const
    {
        laserror("%s. use '-h' or see %s_README.md to get help.", pre.c_str(), name.c_str());
    }
    void parse_arg_invalid(char* argv) const
    {
        std::ostringstream s;
        s << "cannot understand argument '" << std::string(argv) << "'";
        laserrorinfo(s.str());
    }
    void parse_arg_invalid_n(int n) const
    {
        std::ostringstream s;
        s << "cannot understand argument [" << n << "]='" << std::string(argv[n]) << "'";
        laserrorinfo(s.str());
    }
    void error_parse_arg_n_invalid(int i, int n) const
    {
        std::ostringstream s;
        s << "cannot understand argument [" << n << "]=[" << std::string(argv[i + n]) << "] for [" << std::string(argv[i]) << "]";
        laserrorinfo(s.str());
    }
    void parse_arg_cnt_check(int i, int cnt, const char* desc) const
    {
        if ((i + cnt) >= argc)
        {
            laserror("'%s' needs [%d] argument%s%s%s", argv[i], cnt, (cnt > 1 ? "s" : ""), (std::strcmp(desc, "") == 0 ? "" : ": "), desc);
        }
    }
    void force_check() const
    {
        if (force)
            return;
        if (lasmessage_cnt[LAS_SERIOUS_WARNING] > 0)
        {
            byebye();
        }
    }
};

#endif
