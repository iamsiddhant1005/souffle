/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2017, The Souffle Developers. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Explain.h
 *
 * Provenance interface for Souffle; works for compiler and interpreter
 *
 ***********************************************************************/

#pragma once

#include "ExplainProvenanceImpl.h"

#include <csignal>
#include <iostream>
#include <regex>
#include <string>
#include <unistd.h>

#ifdef USE_NCURSES
#include <ncurses.h>
#endif

#include "SouffleInterface.h"
#include "WriteStreamCSV.h"

#define MAX_TREE_HEIGHT 500
#define MAX_TREE_WIDTH 500

namespace souffle {

class ExplainConfig {
public:
    /* Deleted copy constructor */
    ExplainConfig(const ExplainConfig&) = delete;

    /* Deleted assignment operator */
    ExplainConfig& operator=(const ExplainConfig&) = delete;

    /* Obtain the global ExplainConfiguration */
    static ExplainConfig& getExplainConfig() {
        static ExplainConfig _config;
        return _config;
    }

    /* Configuration variables */
    std::ostream* outputStream = nullptr;
    bool json = false;
    int depthLimit = 4;

private:
    ExplainConfig() {}
};

class Explain {
public:
    ExplainProvenance& prov;

    Explain(ExplainProvenance& p) : prov(p) {}
    ~Explain() {
        // close file streama
        if (ExplainConfig::getExplainConfig().outputStream != nullptr) {
            delete ExplainConfig::getExplainConfig().outputStream;
            ExplainConfig::getExplainConfig().outputStream = nullptr;
        }
    }

    /* Process a command, a return value of true indicates to continue, returning false indicates to break (if
     * the command is q/exit) */
    bool processCommand(std::string& commandLine) {
        std::vector<std::string> command = split(commandLine, ' ', 1);

        if (command.empty()) {
            return true;
        }

        if (command[0] == "setdepth") {
            if (command.size() != 2) {
                printError("Usage: setdepth <depth>\n");
                return true;
            }
            try {
                ExplainConfig::getExplainConfig().depthLimit = std::stoi(command[1]);
            } catch (std::exception& e) {
                printError("<" + command[1] + "> is not a valid depth\n");
                return true;
            }
            printInfo("Depth is now " + std::to_string(ExplainConfig::getExplainConfig().depthLimit) + "\n");
        } else if (command[0] == "explain") {
            std::pair<std::string, std::vector<std::string>> query;
            if (command.size() == 2) {
                query = parseTuple(command[1]);
            } else {
                printError("Usage: explain relation_name(\"<string element1>\", <number element2>, ...)\n");
                return true;
            }
            std::unique_ptr<TreeNode> t =
                    prov.explain(query.first, query.second, ExplainConfig::getExplainConfig().depthLimit);
            printTree(std::move(t));
        } else if (command[0] == "subproof") {
            std::pair<std::string, std::vector<std::string>> query;
            int label = -1;
            if (command.size() > 1) {
                query = parseTuple(command[1]);
                label = std::stoi(query.second[0]);
            } else {
                printError("Usage: subproof relation_name(<label>)\n");
                return true;
            }
            std::unique_ptr<TreeNode> t =
                    prov.explainSubproof(query.first, label, ExplainConfig::getExplainConfig().depthLimit);
            printTree(std::move(t));
        } else if (command[0] == "explainnegation") {
            std::pair<std::string, std::vector<std::string>> query;
            if (command.size() == 2) {
                query = parseTuple(command[1]);
            } else {
                printError(
                        "Usage: explainnegation relation_name(\"<string element1>\", <number element2>, "
                        "...)\n");
                return true;
            }

            size_t i = 1;
            std::string rules;
            for (auto rule : prov.getRules(query.first)) {
                rules += std::to_string(i) + ": ";
                rules += rule;
                rules += "\n\n";
                i++;
            }
            printInfo(rules);

            printPrompt("Pick a rule number: ");

            std::string ruleNum = getInput();
            auto variables = prov.explainNegationGetVariables(query.first, query.second, std::stoi(ruleNum));

            if (variables.size() == 1 && variables[0] == "@") {
                printInfo("The tuple exists, cannot explain negation of it!\n");
                return true;
            } else if (variables.size() == 1 && variables[0] == "@non_matching") {
                printInfo("The variable bindings don't match, cannot explain!\n");
                return true;
            }

            std::map<std::string, std::string> varValues;
            for (auto var : variables) {
                printPrompt("Pick a value for " + var + ": ");
                std::string varValue = getInput();
                varValues[var] = varValue;
            }

            printTree(prov.explainNegation(query.first, std::stoi(ruleNum), query.second, varValues));
        } else if (command[0] == "rule" && command.size() == 2) {
            auto query = split(command[1], ' ');
            if (query.size() != 2) {
                printError("Usage: rule <relation name> <rule number>\n");
                return true;
            }
            try {
                printInfo(prov.getRule(query[0], std::stoi(query[1])) + "\n");
            } catch (std::exception& e) {
                printError("Usage: rule <relation name> <rule number>\n");
            }
        } else if (command[0] == "measure") {
            try {
                printInfo(prov.measureRelation(command[1]));
            } catch (std::exception& e) {
                printError("Usage: measure <relation name>\n");
            }
        } else if (command[0] == "output") {
            if (command.size() == 2) {
                ExplainConfig::getExplainConfig().outputStream = new std::ofstream(command[1]);
            } else if (command.size() == 1) {
                delete ExplainConfig::getExplainConfig().outputStream;
                ExplainConfig::getExplainConfig().outputStream = nullptr;
            } else {
                printError("Usage: output  [<filename>]\n");
            }
        } else if (command[0] == "format") {
            if (command.size() == 2 && command[1] == "json") {
                ExplainConfig::getExplainConfig().json = true;
            } else if (command.size() == 2 && command[1] == "proof") {
                ExplainConfig::getExplainConfig().json = false;
            } else {
                printError("Usage: format <json|proof>\n");
            }
        } else if (command[0] == "exit" || command[0] == "q" || command[0] == "quit") {
            printPrompt("Exiting explain\n");
            return false;
        } else {
            printError(
                    "\n----------\n"
                    "Commands:\n"
                    "----------\n"
                    "setdepth <depth>: Set a limit for printed derivation tree height\n"
                    "explain <relation>(<element1>, <element2>, ...): Prints derivation tree\n"
                    "explainnegation <relation>(<element1>, <element2>, ...): Enters an interactive\n"
                    "    interface where the non-existence of a tuple can be explained\n"
                    "subproof <relation>(<label>): Prints derivation tree for a subproof, label is\n"
                    "    generated if a derivation tree exceeds height limit\n"
                    "rule <relation name> <rule number>: Prints a rule\n"
                    "output <filename>: Write output into a file/disable output\n"
                    "format <json|proof>: switch format between json and proof-trees\n"
                    "exit: Exits this interface\n\n");
        }

        return true;
    }

    /* The main explain call */
    virtual void explain() = 0;

private:
    /* Get input */
    virtual std::string getInput() = 0;

    /* Print a command prompt */
    virtual void printPrompt(const std::string& prompt) = 0;

    /* Print a tree */
    virtual void printTree(std::unique_ptr<TreeNode> tree) = 0;

    /* Print any other information */
    virtual void printInfo(const std::string& info) = 0;

    /* Print an error, such as a wrong command */
    virtual void printError(const std::string& error) = 0;

    /* Parse tuple, split into relation name and values */
    std::pair<std::string, std::vector<std::string>> parseTuple(const std::string& str) {
        std::string relName;
        std::vector<std::string> args;

        // regex for matching tuples
        // values matches numbers or strings enclosed in quotation marks
        std::regex relRegex(
                "([a-zA-Z0-9_.-]*)[[:blank:]]*\\(([[:blank:]]*([0-9]+|\"[^\"]*\")([[:blank:]]*,[[:blank:]]*(["
                "0-"
                "9]+|\"[^\"]*\"))*)?\\)",
                std::regex_constants::extended);
        std::smatch relMatch;

        // first check that format matches correctly
        // and extract relation name
        if (!std::regex_match(str, relMatch, relRegex) || relMatch.size() < 3) {
            return std::make_pair(relName, args);
        }

        // set relation name
        relName = relMatch[1];

        // extract each argument
        std::string argsList = relMatch[2];
        std::smatch argsMatcher;
        std::regex argRegex(R"([0-9]+|"[^"]*")", std::regex_constants::extended);

        while (std::regex_search(argsList, argsMatcher, argRegex)) {
            // match the start of the arguments
            std::string currentArg = argsMatcher[0];
            args.push_back(currentArg);

            // use the rest of the arguments
            argsList = argsMatcher.suffix().str();
        }

        return std::make_pair(relName, args);
    }
};

class ExplainConsole : public Explain {
public:
    ExplainConsole(ExplainProvenance& provenance) : Explain(provenance) {}

    /* The main explain call */
    void explain() override {
        printPrompt("Explain is invoked.\n");

        while (true) {
            printPrompt("Enter command > ");
            std::string line = getInput();

            // a return value of false indicates that an exit/q command has been processed
            if (processCommand(line) == false) {
                break;
            }
        }
    }

private:
    /* Get input */
    std::string getInput() override {
        std::string line;

        if (!getline(std::cin, line)) {
            // if EOF has been reached, quit
            line = "q";
        }

        return line;
    }

    /* Print a command prompt */
    void printPrompt(const std::string& prompt) override {
        if (isatty(fileno(stdin))) {
            std::cout << prompt;
        }
    }

    /* Print a tree */
    void printTree(std::unique_ptr<TreeNode> tree) override {
        if (tree) {
            // handle a file ostream output
            std::ostream* output;
            if (ExplainConfig::getExplainConfig().outputStream == nullptr) {
                output = &std::cout;
            } else {
                output = ExplainConfig::getExplainConfig().outputStream;
            }

            if (!ExplainConfig::getExplainConfig().json) {
                tree->place(0, 0);
                ScreenBuffer screenBuffer(tree->getWidth(), tree->getHeight());
                tree->render(screenBuffer);
                *output << screenBuffer.getString();
            } else {
                *output << "{ \"proof\":\n";
                tree->printJSON(*output, 1);
                *output << ",";
                prov.printRulesJSON(*output);
                *output << "}\n";
            }
        }
    }

    /* Print any other information */
    void printInfo(const std::string& info) override {
        if (isatty(fileno(stdin))) {
            std::cout << info;
        }
    }

    /* Print an error, such as a wrong command */
    void printError(const std::string& error) override {
        std::cout << error;
    }
};

#ifdef USE_NCURSES
class ExplainNcurses : public Explain {
public:
    ExplainNcurses(ExplainProvenance& provenance) : Explain(provenance) {}

    /* The main explain call */
    void explain() override {
        if (ExplainConfig::getExplainConfig().outputStream == nullptr) {
            initialiseWindow();
            std::signal(SIGWINCH, nullptr);
        }

        printPrompt("Explain is invoked.\n");

        while (true) {
            clearDisplay();
            printPrompt("Enter command > ");
            std::string line = getInput();

            // a return value of false indicates that an exit/q command has been processed
            if (processCommand(line) == false) {
                break;
            }

            // refresh treePad and allow scrolling
            prefresh(treePad, 0, 0, 0, 0, maxy - 3, maxx - 1);
            scrollTree(maxx, maxy);
        }

        // clean up
        endwin();
    }

private:
    WINDOW* treePad = nullptr;
    WINDOW* queryWindow = nullptr;
    int maxx, maxy;

    /* Get input */
    std::string getInput() override {
        char buf[100];

        curs_set(1);
        echo();

        // get next command
        wgetnstr(queryWindow, buf, 100);
        noecho();
        curs_set(0);
        std::string line = buf;

        return line;
    }

    /* Print a command prompt */
    void printPrompt(const std::string& prompt) override {
        if (isatty(fileno(stdin))) {
            std::cout << prompt;
        }
        werase(queryWindow);
        wrefresh(queryWindow);
        mvwprintw(queryWindow, 1, 0, prompt.c_str());
    }

    /* Print a tree */
    void printTree(std::unique_ptr<TreeNode> tree) override {
        if (tree) {
            if (!ExplainConfig::getExplainConfig().json) {
                tree->place(0, 0);
                ScreenBuffer screenBuffer(tree->getWidth(), tree->getHeight());
                tree->render(screenBuffer);
                wprintw(treePad, screenBuffer.getString().c_str());
            } else {
                if (ExplainConfig::getExplainConfig().outputStream == nullptr) {
                    std::stringstream ss;
                    ss << "{ \"proof\":\n";
                    tree->printJSON(ss, 1);
                    ss << ",";
                    prov.printRulesJSON(ss);
                    ss << "}\n";

                    wprintw(treePad, ss.str().c_str());
                } else {
                    std::ostream* output = ExplainConfig::getExplainConfig().outputStream;
                    *output << "{ \"proof\":\n";
                    tree->printJSON(*output, 1);
                    *output << ",";
                    prov.printRulesJSON(*output);
                    *output << "}\n";
                }
            }
        }
    }

    /* Print any other information */
    void printInfo(const std::string& info) override {
        if (isatty(fileno(stdin))) {
            wprintw(treePad, info.c_str());
            prefresh(treePad, 0, 0, 0, 0, maxy - 3, maxx - 1);
        }
    }

    /* Print an error, such as a wrong command */
    void printError(const std::string& error) override {
        wprintw(treePad, error.c_str());
        prefresh(treePad, 0, 0, 0, 0, maxy - 3, maxx - 1);
    }

    /* Initialise ncurses window */
    WINDOW* makeQueryWindow() {
        WINDOW* w = newwin(3, maxx, maxy - 2, 0);
        wrefresh(w);
        return w;
    }

    // initialise ncurses window
    void initialiseWindow() {
        initscr();

        // get size of window
        getmaxyx(stdscr, maxy, maxx);

        // create windows for query and tree display
        queryWindow = makeQueryWindow();
        treePad = newpad(MAX_TREE_HEIGHT, MAX_TREE_WIDTH);

        keypad(treePad, true);
    }

    // allow scrolling of provenance tree
    void scrollTree(int maxx, int maxy) {
        int x = 0;
        int y = 0;

        while (true) {
            int ch = wgetch(treePad);

            if (ch == KEY_LEFT) {
                if (x > 2) x -= 3;
            } else if (ch == KEY_RIGHT) {
                if (x < MAX_TREE_WIDTH - 3) x += 3;
            } else if (ch == KEY_UP) {
                if (y > 0) y -= 1;
            } else if (ch == KEY_DOWN) {
                if (y < MAX_TREE_HEIGHT - 1) y += 1;
            } else {
                ungetch(ch);
                break;
            }

            prefresh(treePad, y, x, 0, 0, maxy - 3, maxx - 1);
        }
    }

    void clearDisplay() {
        // reset tree display on each loop
        werase(treePad);
        prefresh(treePad, 0, 0, 0, 0, maxy - 3, maxx - 1);
    }
};
#endif

inline void explain(SouffleProgram& prog, bool ncurses = false) {
    ExplainProvenanceImpl prov(prog);

    if (ncurses) {
#ifdef USE_NCURSES
        ExplainNcurses exp(prov);
        exp.explain();
#else
        std::cout << "The ncurses-based interface is not enabled\n";
#endif
    } else {
        ExplainConsole exp(prov);
        exp.explain();
    }
}

}  // end of namespace souffle
