#pragma once

#include "LuaBase.hpp"
#include "RangeMatcherMethods4Lua.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <memory>

struct StreamPair
{
 std::istream & is;
 std::ostream & os;
};

class Console
{
private:
 void init();

public:
 explicit Console(StreamPair streamPair);

 explicit Console(StreamPair streamPair, std::shared_ptr<LuaBase> luaBase, RangeMatcherMethods4Lua & rangeMatcherMethods4Lua);

private:
 StreamPair m_StreamPair;

 std::shared_ptr<LuaBase> m_LuaBase;

 bool m_ConsoleQuit{false};

 RangeMatcherMethods4Lua m_RangeMatcherMethods4LuaDefault{};

 RangeMatcherMethods4Lua & m_RangeMatcherMethods4Lua; // TODO : distribute both variations in subclasses

 struct Token : std::string { using std::string::string; };

 struct StringRange
 {
  using iterator = std::string::iterator;
  iterator begin;
  iterator end;
 };
 
 struct Menu;

 using MenuMap = std::map<Token,Menu>;

 struct Menu
 {
  std::string helpString;
  MenuMap menuMap;
  std::function<void(StringRange)> invoke;
 };

 MenuMap m_MenuMap{};

 void emplaceContextEntries(MenuMap &menuMap);

 void interpretMenu(StringRange stringRange);
 void interpretLua(StringRange stringRange);
 void interpretLuaP(StringRange stringRange);
 void interpretQuit(StringRange stringRange);

 void interpretQuestionMark(StringRange stringRange);

 void interpretTreeRek( int depth, Console::MenuMap & menuMap, int maxdepth);
 void interpretTree(StringRange stringRange);

 void interpretHelp(StringRange stringRange);

 void interpretRoot(StringRange stringRange);
 void interpretNode(MenuMap & menuMap, StringRange stringRange);

 MenuMap * m_LastChoosedMenuMap{nullptr};
 Menu * m_LastChoosedMenu{nullptr};
 Token m_LastToken{};
 
 bool m_DryRun{false};
 
 std::string m_PreLine{};
};
