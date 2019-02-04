tokenStarts4 = rmNextObjectIndex()
rmPatternString( "/*", "//", "\"", "'")
tokenEnds4 = rmNextObjectIndex()
rmPatternString( "*/", "\n")
rmPatternRegex( "([^\\\\]|^)\"", "([^\\\\]|^)'")
ranges4 = rmNextObjectIndex()
rmNamedPatternRange( "comment1", tokenStarts4 + 0, tokenEnds4 + 0)
rmNamedPatternRange( "comment2", tokenStarts4 + 1, tokenEnds4 + 1)
rmNamedPatternRange( "string1", tokenStarts4 + 2, tokenEnds4 + 2)
rmNamedPatternRange( "string2", tokenStarts4 + 3, tokenEnds4 + 3)
nonOverlappingMatcher = rmNonOverlappingMatcher( ranges4 + 0, ranges4 + 1, ranges4 + 2, ranges4 + 3)
file1 = rmFileRead("testfiles/006.txt")
result = rmMatchRanges(nonOverlappingMatcher, file1)
-- print( rmMatchRanges2Lua(result))

expectedResults = { 
 { ["name"] = "initial-element-jc3jvchtrz",  ["begin.begin"]=0, ["begin.end"]=0, ["end.begin"]=51, ["end.end"]=51} ,
 { ["name"] = "comment2",  ["begin.begin"]=0, ["begin.end"]=2, ["end.begin"]=9, ["end.end"]=10 } ,
 { ["name"] = "string2",  ["begin.begin"]=10, ["begin.end"]=11, ["end.begin"]=12, ["end.end"]=14 } ,
 { ["name"] = "string1",  ["begin.begin"]=14, ["begin.end"]=15, ["end.begin"]=26, ["end.end"]=28} ,
 { ["name"] = "comment1",  ["begin.begin"]=28, ["begin.end"]=30, ["end.begin"]=38, ["end.end"]=40} ,
 { ["name"] = "string1",  ["begin.begin"]=40, ["begin.end"]=41, ["end.begin"]=41, ["end.end"]=42} ,
 { ["name"] = "string2",  ["begin.begin"]=42, ["begin.end"]=43, ["end.begin"]=43, ["end.end"]=44 } ,
 { ["name"] = "string1",  ["begin.begin"]=44, ["begin.end"]=45, ["end.begin"]=51, ["end.end"]=51 }}

gotResults = { rmMatchRanges2Lua(result) }

print( 8 == #gotResults)

for k, v in pairs(gotResults)
do
 -- print( k)
 -- print( gotResults[k])
 -- print( expectedResults[k])
 -- print( gotResults[k] == expectedResults[k])
 -- print( "#v", #v)

 -- print( "+++")
 -- print( k)
 -- print( v[k][1])
 -- print( expectedResults[k][1])

 gotT = v
 expectedT = expectedResults[k]
 
 for k2, v2 in pairs(gotT)
 do
  -- print( "---")
  -- print( "k2", k2)
  -- print( "v2", v2)
  -- print( expectedT[k2])
  print( v2 == expectedT[k2])
 end
end

