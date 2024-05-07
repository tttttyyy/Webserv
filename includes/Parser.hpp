#ifndef PARSER_HPP
#define PARSER_HPP
#include "Libs.hpp"
#include "ServerManager.hpp"

class Location;
class HTTPServer;
class HTTPRequest;
class ServerManager;

class Parser
{
    private:
    enum p_type
    {
        WORD, // general type of token for CONTEXT && DIRECTIVE
        CONTEXT, // for server and location contexts
        OPENBRACE, // {
        CLOSEBRACE, // }
        DIRECTIVE, // property for server
        DIRECTIVE_VALUE, // value for property
        SEMICOLON, // ;
        CONTEXT_PREFIX, //For location context
        SEPARATOR // \a
    };
    public:
        Parser(const char *confFile);
        ~Parser();
    public:
        void start(ServerManager &mgn);
    private:
        std::string context_keyword(std::string const &context_token);
    private:
        size_t contextWord(std::string const &config, size_t p);
        void removeUnprintables(std::vector<std::string> &tmp_ctx);
        bool isWord(char s);
        std::string remove_extraSpace(std::string const &d_val);
        int context_wrap(std::string const &server);
        void tolower(std::string &s);
        void addWord(std::string config, size_t *i);
        void addToken(char s, p_type type);
        void clean( void );
        void scheme( void );
        void semantic_analysis( void );
        void intermediate_code_generation( void );
        void syntax_analysis( void );
        void fill_servers(ServerManager &mgn);
        void remove_spaces(std::string &tmp_text);
    private:
        std::fstream IO;
        std::map<std::string, void (Parser::*)(std::string &, HTTPServer &)>directives;
        std::map<std::string, void (Parser::*)(std::string &, Location &)>location_directives;
        struct Token
        {
            p_type type;
            std::string token;
        };
        std::list<Token> tokens;
        std::string config;
        std::vector<std::string> server_ctx;
    private:
        void create_server(ServerManager &mgn, std::list<Token>::iterator& ch);
        void location(std::list<Token>::iterator& next, HTTPServer& srv);
        void l_directive(std::list<Token>::iterator& next, Location& loc); //Location directives
        void s_directive(std::list<Token>::iterator& next, HTTPServer& srv); // Server directives
        void make_pair(size_t i, std::list<Token>::iterator& node, HTTPServer &srv);
    private:
        void d_listen(std::string &d_val, HTTPServer &srv);
        void d_server_name(std::string &d_val, HTTPServer &srv);
        void d_root(std::string &d_val, HTTPServer &srv);
        void d_index(std::string &d_val, HTTPServer &srv);
        void d_autoindex(std::string &d_val, HTTPServer &srv);
        void d_methods(std::string &d_val, HTTPServer &srv);
        void d_err_page(std::string &d_val, HTTPServer &srv);
        void d_body_size(std::string &d_val, HTTPServer &srv);
        void d_redirect(std::string &d_val, HTTPServer &srv);
        void d_cgi(std::string &d_val, HTTPServer &srv);
        void d_upload_dir(std::string &d_val, HTTPServer &srv);
    private:
        void l_root(std::string &d_val, Location &srv);
        void l_index(std::string &d_val, Location &srv);
        void l_autoindex(std::string &d_val, Location &srv);
        void l_methods(std::string &d_val, Location &srv);
        void l_err_page(std::string &d_val, Location &srv);
        void l_body_size(std::string &d_val, Location &loc);
        void l_redirect(std::string &d_val, Location &loc);
        void l_cgi(std::string &d_val, Location &loc);
        void l_upload_dir(std::string &d_val, Location &loc);
};
typedef std::map<std::string, void (Parser::*)(std::string &, HTTPServer &)>::iterator FuncDir;
typedef std::map<std::string, void (Parser::*)(std::string &, Location &)>::iterator LocDir;
#endif