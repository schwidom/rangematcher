#include "Console.hpp"

#include <string> // getline

#include <sstream> // istringstream
#include <stdexcept> // logic_error
#include <utility>

namespace
{
 std::string shortHelp{ R"help(
input prompt is : <<
mh ... help
? ... menu context help
q ... quit)help"+1};
}

#define BIND(X) std::bind(&Console::X, this, std::placeholders::_1)

void Console::emplaceContextEntries(MenuMap &menuMap)
{
 menuMap.emplace( Token{"?"}, Menu{ "current menu keys", {}, BIND(interpretQuestionMark)});
 menuMap.emplace( Token{"t"}, Menu{ "current menu tree with optional depth", {}, BIND(interpretTree)});
}

Console::Console(StreamPair streamPair)
: m_StreamPair(streamPair)
{
 
 auto & sp = m_StreamPair;

 luaL_openlibs( m_LuaBase.getLua());
 m_RangeMatcherMethods4Lua.propagate(m_LuaBase);

 emplaceContextEntries( m_MenuMap);

 m_MenuMap.emplace( Token{"q"}, Menu{ "quit", {}, BIND( interpretQuit)});
 m_MenuMap.emplace( Token{"m"}, Menu{ "menu", {}, BIND( interpretMenu)});
 m_MenuMap.emplace( Token{"l"}, Menu{ "execute lua", {}, BIND( interpretLua)});
 m_MenuMap.emplace( Token{"p"}, Menu{ "execute lua inside print()", {}, BIND( interpretLuaP)});

 std::string prompt{"<< "};

 sp.os << shortHelp << std::endl;

 while(true)
 {
  std::string line;
  
  sp.os << prompt;
  std::getline( sp.is, line);

  if( sp.is.eof()){ break;}

  interpretRoot(StringRange{line.begin(), line.end()});

  if( m_ConsoleQuit)
  {
   sp.os << "exiting by " << m_LastToken << std::endl;
   break;
  }
 }
}

void Console::interpretMenu(StringRange stringRange)
{
 if(!m_LastChoosedMenu)
 {
  throw std::logic_error("!m_LastChoosedMenuMap");
 }

 if( m_LastChoosedMenu->menuMap.empty())
 {
  auto & menuMap = m_LastChoosedMenu->menuMap;
 
  emplaceContextEntries(menuMap);
  menuMap.emplace( Token{"h"}, Menu{ "help", {}, BIND( interpretHelp)});
 }

 if( m_DryRun){ return; }

 interpretNode(m_LastChoosedMenu->menuMap, stringRange);
}

void Console::interpretLua(StringRange stringRange)
{
 if( m_DryRun){ return; }

 auto & sp = m_StreamPair;

 std::string luaCode{stringRange.begin, stringRange.end};

 sp.os << "executing lua: " << luaCode << std::endl;

 int res = luaL_dostring( m_LuaBase.getLua(), luaCode.c_str());

 sp.os << ( ( 0 == res ) ? "success" : "error" ) << std::endl;
 if( 0 != res)
 {
  sp.os << m_RangeMatcherMethods4Lua.getLastErrorMessage() << std::endl;
 }
}

void Console::interpretLuaP(StringRange stringRange)
{
 if( m_DryRun){ return; }

 std::string luaCode{stringRange.begin, stringRange.end};

 std::string luaCodeExtended{"print( " + luaCode + ")"};
 
 interpretLua(StringRange{luaCodeExtended.begin(), luaCodeExtended.end()});
}

void Console::interpretQuit(StringRange stringRange)
{
 if( m_DryRun){ return; }

 auto & sp = m_StreamPair;

 std::string quitString{stringRange.begin, stringRange.end};
 if(!quitString.empty())
 {
  sp.os << "trailing chars after " << m_LastToken << std::endl;
  return;
 }

 m_ConsoleQuit = true;
}

void Console::interpretQuestionMark(StringRange stringRange)
{
 if( m_DryRun){ return; }

 auto & sp = m_StreamPair;

 std::string quitString{stringRange.begin, stringRange.end};
 if(!quitString.empty())
 {
  sp.os << "trailing chars after " << m_LastToken << std::endl;
  return;
 }

 if( !m_LastChoosedMenuMap)
 {
  throw std::logic_error("!m_LastChoosedMenuMap");
 }

 for( auto & kvp : *m_LastChoosedMenuMap)
 {
  m_StreamPair.os << kvp.first << ": " << kvp.second.helpString << std::endl;
 }
}

void Console::interpretTreeRek( int depth, MenuMap & menuMap, int maxdepth)
{
 if( depth == maxdepth){ return; }

 for( auto & kvp : menuMap)
 {
  if(kvp.first == Token{"t"}){ continue;}
  if(kvp.first == Token{"?"}){ continue;}
  m_StreamPair.os << std::string(" ", depth) << kvp.first << ": " << kvp.second.helpString << std::endl;
  std::string dummy;
  m_LastChoosedMenu= &kvp.second;
  m_LastChoosedMenuMap = &kvp.second.menuMap;
  kvp.second.invoke(StringRange{dummy.begin(),dummy.end()});
  interpretTreeRek( depth + 1, kvp.second.menuMap, maxdepth);
 }
}

void Console::interpretTree(StringRange stringRange)
{
 if( m_DryRun){ return; }

 std::string parameterInt{stringRange.begin, stringRange.end};

 // TODO depth

 if( !m_LastChoosedMenuMap)
 {
  throw std::logic_error("!m_LastChoosedMenuMap");
 }

 m_DryRun = true;

 int maxdepth = -1;

 std::istringstream iss(parameterInt);
 
 iss >> maxdepth;

 interpretTreeRek(0, *m_LastChoosedMenuMap, maxdepth);
}

void Console::interpretHelp(StringRange stringRange)
{
 if( m_DryRun){ return; }

 m_StreamPair.os << (R"help(
the rangematcher console:

)help" + 1) <<  shortHelp << (R"help(

input is interpreted linewise
if a line begins or ends with a hash sign (#) then the input is aborted
if a line begins and ends with a hash sign (#) then the input is accepted as without beginning and ending hash signs
the rangematcher datastructures are defined via lua
all lines beginning with l or p precedes lua commands
a question mark (?) at any menupoint including the root menu shows the menu contents

 )help") << std::endl;
}

void Console::interpretRoot(StringRange stringRange)
{
 m_DryRun = false;
 m_LastChoosedMenuMap = nullptr;

 if( stringRange.begin == stringRange.end){ return;}

 bool aborted{false};

 if( '#' == *stringRange.begin)
 {
  aborted = true;
  stringRange.begin +=1;
 }

 if( '#' == *(stringRange.end - 1))
 {
  aborted = !aborted;
  stringRange.end -=1;
 }

 if( stringRange.begin >= stringRange.end){ return;}

 if( aborted)
 {
  m_StreamPair.os << "aborted" << std::endl;
  return;
 }

 interpretNode( m_MenuMap, stringRange);
}

void Console::interpretNode(MenuMap & menuMap, StringRange stringRange)
{
 if( m_DryRun){ return; }

 m_LastChoosedMenuMap = &menuMap;

 Token currentToken = Token{*stringRange.begin};

 m_LastToken = currentToken;

 auto it = menuMap.find(currentToken);

 if( it == menuMap.end())
 {
  m_LastChoosedMenu = nullptr;
  m_StreamPair.os << "not implemented " << currentToken << std::endl;
  return;
 }

 m_LastChoosedMenu = &it->second;

 m_LastChoosedMenu->invoke(StringRange{stringRange.begin + 1, stringRange.end});
}

