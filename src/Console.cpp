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

void Console::init()
{
 
 auto & sp = m_StreamPair;

 emplaceContextEntries( m_MenuMap);

 m_MenuMap.emplace( Token{"q"}, Menu{ "quit", {}, BIND( interpretQuit)});
 m_MenuMap.emplace( Token{"m"}, Menu{ "menu", {}, BIND( interpretMenu)});
 m_MenuMap.emplace( Token{"l"}, Menu{ "execute lua", {}, BIND( interpretLua)});
 m_MenuMap.emplace( Token{"p"}, Menu{ "execute lua inside print()", {}, BIND( interpretLuaP)});

 std::string prompt1{"<< "};
 std::string prompt2{"++ "};

 sp.os << shortHelp << std::endl;

 while(true)
 {
  std::string line;
  
  sp.os << ( m_PreLine.empty() ? prompt1 : prompt2 );
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

Console::Console(StreamPair streamPair)
: m_StreamPair(streamPair)
, m_LuaBase{std::make_shared<LuaBase>()}
, m_RangeMatcherMethods4Lua{m_RangeMatcherMethods4LuaDefault}
{
 m_RangeMatcherMethods4Lua.registerMethods2LuaBase(m_LuaBase);
 init();
}

Console::Console(StreamPair streamPair, std::shared_ptr<LuaBase> luaBase, RangeMatcherMethods4Lua & rangeMatcherMethods4Lua)
: m_StreamPair(streamPair)
, m_LuaBase{std::move(luaBase)}
, m_RangeMatcherMethods4Lua{rangeMatcherMethods4Lua}
{
 init();
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

 int res = luaL_dostring( m_LuaBase->getLua(), luaCode.c_str());

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
if a line ends with a backslash (\) then the line gets extended by the end of line sign (\n) and the second line
the rangematcher is controlled by lua commands
all lines beginning with l or p precedes lua commands, p sets the command inside 'print( ... )'
a question mark (?) at any menupoint including the root menu shows the menu contents
l rmHelp() ... shows the help for the lua interface

 )help") << std::endl;
}

void Console::interpretRoot(StringRange stringRangeNext)
{
 m_DryRun = false;
 m_LastChoosedMenuMap = nullptr;

 m_PreLine += std::string(stringRangeNext.begin, stringRangeNext.end);

 StringRange stringRange{ m_PreLine.begin(), m_PreLine.end()};

 if( stringRange.begin == stringRange.end){ return;}

 if( '\\' == *(stringRange.end - 1)) {
  *(stringRange.end - 1) = '\n';
  return; 
 }

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
  m_PreLine.clear();
  m_StreamPair.os << "aborted" << std::endl;
  return;
 }

 interpretNode( m_MenuMap, stringRange);
 m_PreLine.clear();
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

