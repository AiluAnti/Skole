#!/usr/bin/python3
import os

def getFiles():
    return [x for x in os.listdir(os.getcwd()) if x.endswith(".py")]

def countLines(fileList):
    commentLines = 0
    emptyLines = 0
    codeLines = 0
    inComment = False
    inSingle = False
    inDouble = False
    for fileName in fileList:
        for line in open(fileName):
            line = line.strip()
            
            if inComment:
                commentLines += 1
                if inDouble and line.startswith('"""'):
                    inComment = False
                    inDouble = False
                elif inSingle and line.startswith("'''"):
                    inComment = False
                    inSingle = False
            else:
                if line == '':
                    emptyLines += 1
                elif line.startswith('#'):
                    commentLines += 1
                elif line.startswith('"""') and not inDouble:
                    inComment = True
                    inDouble = True
                    commentLines += 1
                elif line.startswith("'''") and not inSingle:
                    inComment = True
                    inSingle = True
                    commentLines += 1
                else:
                    codeLines += 1

    return (emptyLines, commentLines, codeLines)
        

def main():
    files = getFiles()
    emptyLines, commentLines, codeLines = countLines(files)

    print("Number of empty lines:    {}".format(emptyLines))
    print("Number of comment lines:  {}".format(commentLines))
    print("Number of code lines:     {}".format(codeLines))
    print("Number of lines in total: {}".format(emptyLines + commentLines + codeLines))
    
main()
