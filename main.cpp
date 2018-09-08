/* 
 * Program which interprets a fake language.
 * 
 * File:   main.cpp
 * Author: Gavin
 *
 * Created on September 1, 2018, 8:11 PM
 * Copyright 2018
 */

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include <typeinfo>
#include <limits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cctype>

std::string quotes(const std::string& s);

enum L_datatype{INTEGER, BOOLEAN, STRING};

struct L_var{
    // All data represented in strings
    std::string s;
    L_datatype datatype;
    
    L_var() {}
    L_var(int i) : s(std::to_string(i)), datatype(L_datatype::INTEGER) {}
    L_var(bool b) : s(std::to_string(b)), datatype(L_datatype::BOOLEAN) {}
    L_var(const std::string& s) : s(s), datatype(L_datatype::STRING) {}
};

void interpret_file(const std::string& file);
void interpret_line(const std::string& line);
std::vector<std::string> tokenize_string(const std::string& s);
void parse_line_tokens(const std::vector<std::string>& tokens,
        std::unordered_map<std::string, L_var>& vars);
void parse_operator_tokens(const std::vector<std::string>& tokens,
        std::unordered_map<std::string, L_var>& vars);
void parse_reserved_words(const std::vector<std::string>& tokens,
        std::unordered_map<std::string, L_var>& vars);
bool stob(const std::string& s);
bool in_quotes(const std::string& s);
std::string remove_quotes(const std::string& s);
bool is_alpha(char c);
std::string bool_to_string(bool b);

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: interpret <file>";
        return -1;
    }
    
    interpret_file(argv[1]);
    
    return 0;
}

void interpret_file(const std::string& file) {
    std::ifstream fin(file);
    std::string line;
    while (std::getline(fin, line)) {
        interpret_line(line);
    }
}

void interpret_line(const std::string& line) {
    static std::unordered_map<std::string, L_var> variables;
    std::vector<std::string> tokens = tokenize_string(line);
    parse_line_tokens(tokens, variables);
}

void parse_line_tokens(const std::vector<std::string>& tokens,
        std::unordered_map<std::string, L_var>& vars) {
    parse_operator_tokens(tokens, vars);
    parse_reserved_words(tokens, vars);
}

void parse_operator_tokens(const std::vector<std::string>& tokens,
        std::unordered_map<std::string, L_var>& vars) {
    if (tokens.size() < 3) {
        return;
    }
    if (tokens[1] == "=" && vars.find(tokens[2]) == vars.end()) {
        // Assignment
        if (in_quotes(tokens[2])) {
            vars[tokens[0]] = L_var(tokens[2]);
        } else if (tokens[2] == "TRUE" || tokens[2] == "FALSE") {
            vars[tokens[0]] = L_var(stob(tokens[2]));
        } else {
            vars[tokens[0]] = L_var(std::stoi(tokens[2]));
        }
    } else if (tokens[1] == "=") {
        vars[tokens[0]] = vars[tokens[2]];
    } else if (tokens[1] == "+="
      && vars[tokens[0]].datatype == L_datatype::STRING
      && (in_quotes(tokens[2]) || vars.find(tokens[2]) != vars.end())) {
        // String concatenation
        std::string concat = (vars.find(tokens[2]) == vars.end())
            ? remove_quotes(vars[tokens[0]].s) + remove_quotes(tokens[2]) :
            remove_quotes(vars[tokens[0]].s) + remove_quotes(vars[tokens[2]].s);
        vars[tokens[0]].s = quotes(concat);
    } else if (tokens[1] == "+=" && !in_quotes(tokens[2]) 
      && vars[tokens[0]].datatype == L_datatype::INTEGER) {
        // Integer addition
        int sum = (vars.find(tokens[2]) == vars.end()) 
            ? std::stoi(vars[tokens[0]].s) + std::stoi(tokens[2]) : 
                std::stoi(vars[tokens[0]].s) + std::stoi(vars[tokens[2]].s);
        vars[tokens[0]].s = std::to_string(sum);
    } else if (tokens[1] == "*=" 
      && vars[tokens[0]].datatype == L_datatype::INTEGER) {
        // Integer multiplication
        int product = (vars.find(tokens[2]) == vars.end()) 
            ? std::stoi(vars[tokens[0]].s) * std::stoi(tokens[2]) : 
                std::stoi(vars[tokens[0]].s) * std::stoi(vars[tokens[2]].s);
        vars[tokens[0]].s = std::to_string(product);
    } else if (tokens[1] == "&=" 
      && vars[tokens[0]].datatype == L_datatype::BOOLEAN) {
        // And-Equals
        bool other = (tokens[2] == "TRUE");
        bool result = stob(vars[tokens[0]].s) && other;
        vars[tokens[0]].s = std::to_string(result);
    }
}

void parse_reserved_words(const std::vector<std::string>& tokens,
        std::unordered_map<std::string, L_var>& vars) {
    if (tokens.size() < 3) {
        return;
    }
    if (tokens[0] == "PRINT" && vars.find(tokens[1]) != vars.end()) {
        if (vars[tokens[1]].datatype == L_datatype::BOOLEAN) {
            std::cout << tokens[1] << "=" << bool_to_string(stob(vars[tokens[1]].s)) << std::endl;
        } else {
            std::cout << tokens[1] << "=" << vars[tokens[1]].s << std::endl;
        }
    }
}

std::string bool_to_string(bool b) {
    std::string s;
    b ? s = "TRUE" : s = "FALSE";
    
    return s;
}

bool in_quotes(const std::string& s) {
    auto pos1 = s.find("\"");
    auto pos2 = s.find_last_of("\"");
    
    return pos1 != pos2 && pos1 != std::string::npos;
}

bool stob(const std::string& s) {
    // Due to conversions to strings, we check for the legacy values of true and false
    bool b;
    s == "0" ? b = false : b = true;
    
    return b;
}

std::string remove_quotes(const std::string& s) {
    size_t pos;
    std::string s1 = const_cast<std::string&>(s);
    while ((pos = s1.find("\"")) != std::string::npos) {
        s1.erase(pos, 1);
    }
    
    return s1;
}

std::string quotes(const std::string& s) {
    std::string s1 = "\"";
    s1 += const_cast<std::string&>(s);
    s1 += "\"";
    return s1;
}

bool is_alpha(char c) {
    return std::isalpha(c);
}

std::vector<std::string> tokenize_string(const std::string& s) {
    // Custom tokenize to retain quotes around quoted items
    std::vector<std::string> tokens;
    size_t lastpos = 0, pos = s.find(' ');
    while (pos != std::string::npos) {
        tokens.push_back(s.substr(lastpos, pos - lastpos));
        lastpos = pos + 1;
        if (s[pos + 1] == '\"') {
            // Skip over quotes
            pos = s.find('\"');
            pos = s.find('\"', pos + 1) + 1;
        } else {
            pos = s.find(' ', pos + 1);
        }
    }
    
    tokens.push_back(s.substr(lastpos, s.size() - lastpos));
    return tokens; 
}
