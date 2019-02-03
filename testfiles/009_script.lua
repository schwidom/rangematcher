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

expectedResults = { "initial-element-jc3jvchtrz", 0, 0, 51, 51, "comment2", 0, 2, 9, 10, "string2", 10, 11, 12, 14, "string1", 14, 15, 26, 28, "comment1", 28, 30, 38, 40, "string1", 40, 41, 41, 42, "string2", 42, 43, 43, 44, "string1", 44, 45, 51, 51 }

gotResults = { rmMatchRanges2Lua(result) }

for k, v in pairs(gotResults)
do
 -- print( k)
 -- print( gotResults[k])
 -- print( expectedResults[k])
 -- print( gotResults[k] == expectedResults[k])
 print( v == expectedResults[k])
end

