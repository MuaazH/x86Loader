#include <windows.h>
#include <iostream>
#include "../x86assm32/x86_assm.h"
#include <RedBlackBST.h>
#include <io.h>
#include <memory.h>
#include <c_strings.h>

using namespace std;

static char *pScript = 0;
static unsigned int scriptLen;

bool loadScript(char *pFileName) {
	bool success = true;
	unsigned int size;
	IOHANDLE pFile = io_open(pFileName, (int) IOFLAG_READ);
	if (pFile) {
		size =  (unsigned int) io_length(pFile);
        if (size < 1) {
            success = false;
		} else {
			char *pBuf = new char[size];
			unsigned int  read = 0;
			while (read < size) {
				unsigned int count = io_read(pFile, (void *)(pBuf + read), size - read);
				if (count < 1) { // EOF
					success = false;
					break;
				}
				read += count;
			}
			pScript = pBuf;
			scriptLen  = size;
		}
		io_close(pFile);
	} else {
		success = false;
	}
	return success;
}

void releaseScript() {
	if (pScript) {
		delete [] pScript;
		pScript = 0;
	}
}

void deleteNode(char *b, unsigned int x) {
	delete [] b;
}

int nextLine(unsigned int from, int maxLen, char *pOut, unsigned int *pOutLen) {
	unsigned int i = from;
	int len = 0;
	bool spaceFound = false;
	for (; i < scriptLen; i++) {
		if (pScript[i] == '\r' || pScript[i] == '\n') {
			if (len) { // end of line found
				break;
			} else { // skip this crap at the begining of the file
				continue;
			}
		}
		if (pScript[i] <= ' ') {
			spaceFound = true;
		} else {
			// remove comments
			if (pScript[i] == ';') { // skip to the end of the line
				while (i < scriptLen && !(pScript[i] == '\r' || pScript[i] == '\n')) {
					i++;
				}
				if (len)  // stop only if some text was already found
					break;
				else
					continue;
			}
			if (spaceFound && len) {
				pOut[len++] = ' ';
				if (len >= maxLen) break;
			}
			spaceFound = false;
			pOut[len++] = pScript[i];
			if (len >= maxLen) break;
		}
	}
	*pOutLen = len;
	if (len < maxLen) {
		pOut[len] = 0;
	} else {
		while (i < scriptLen && !(pScript[i] == '\r' || pScript[i] == '\n')) {
			i++;
		}
	}
	return i;
}

bool startsWith(char *pText, const char *pPerfix) {
	for(int i = 0; ; i++)
	{
		if (!pPerfix[i]) return true;
		if (pText[i] != pPerfix[i]) break;
	}
	return false;
}

char *trimBuffer(char *buf, unsigned int size) {
	char *s = new char[size];
	mem_copy((void *) s, (void *) buf, size);
	delete [] buf;
	return s;
}

int strCmp(char *a, char *b) {
	
	for(int i = 0; ; i++)
	{
		if (a[i] < b[i]) return -1;
		if (a[i] > b[i]) return  1;
		if (!a[i]) break;
	}
	return 0;
}

void strCopy(char *dst, char *src) {
	for (int i = 0; ; i++) {
		dst[i] = src[i];
		if (!src[i]) break;
	}
}

/* return how many times shit got replaced, or -1 for error */
int strReplace(const char *txt, const char *oldTxt, const char *newTxt, char *pOut, unsigned int outSize) {
	if (!oldTxt[0]) return false; // too short, what the shit man?
	int count = 0;
	unsigned int j = 0; // out length
	for (unsigned int i = 0; txt[i]; i++) {
		bool match = false;
		
		for(unsigned int n = 0; ; n++)
		{
			if (!oldTxt[n]) { match = true; i += n - 1; count++; break; }
			if (txt[i + n] != oldTxt[n]) break;
		}
		
		if (match) {
			int m = 0;
			while (newTxt[m]) {
				if (j < outSize) {
					pOut[j++] = newTxt[m];
					m++;
				} else {
					return -1;
				}
			}
			continue;
		}
		if (j < outSize) {
			pOut[j++] = txt[i];
		} else {
			return -1;
		}
	}
	if (j < outSize) {
		pOut[j] = 0;
	} else {
		return -1;
	}
	return count;
}

void replaceLabels(char *line, char *out, unsigned int outSize, RBNode<char *, unsigned int> *pSymbols) {
	if (!pSymbols) {
		strCopy(out, line);
		return;
	}
	char val[9];
	val[8] = 0;
	x86::writeHexInt(val, pSymbols->m_Val);
	int c = strReplace(line, pSymbols->m_Key, val, out, outSize);
	if (c > 0) {
		strCopy(line, out);
	} else {
		strCopy(out, line);
	}
	replaceLabels(line, out, outSize, pSymbols->m_pRight);
	replaceLabels(line, out, outSize, pSymbols->m_pLeft);
}

unsigned int parseHexByte(char *str) {
	int mem = 0;
	for(int i = 0; i < 2; i++)
	{
		char c = str[i];
		if ('0' <= str[i] && str[i] <= '9') {
			mem *= 16;
			mem += str[i] - '0';
			continue;
		} else {
			c |= 0x20;
			if ('a' <= c && c <= 'z') {
				c += 10 - 'a';
				mem *= 16;
				mem += c;
				continue;
			}
		}
		return -1;
	}
	if (str[2] <= ' ') {
		return mem;
	}
	return -1;
}

unsigned int parseHexMemAddress(char *str) {
	int mem = 0;
	for(int i = 0; i < 8; i++)
	{
		char c = str[i];
		if ('0' <= str[i] && str[i] <= '9') {
			mem *= 16;
			mem += str[i] - '0';
			continue;
		} else {
			c |= 0x20;
			if ('a' <= c && c <= 'z') {
				c += 10 - 'a';
				mem *= 16;
				mem += c;
				continue;
			}
		}
		return 0;
	}
	if (str[8] <= ' ') {
		return mem;
	}
	return 0;
}

void execScript(HANDLE processHld) {
	const unsigned int maxLineLen = 128;
	char line[maxLineLen];
	char fixedLine[maxLineLen];
	unsigned int i = 0;
	unsigned int len;

	char *assmCode = 0; // used to save assembly shit
	unsigned int assmCodeMemory;
	unsigned int assmCodeLength;
	bool assmStartFound;

	RedBlackBST<char *, unsigned int> symbols(strCmp);
	symbols.SetCleaner(deleteNode);

	RedBlackBST<char *, unsigned int> globalLabels(strCmp);
	globalLabels.SetCleaner(deleteNode);

	
	while (true) {
		i = nextLine(i, maxLineLen, line, &len);
		if (i >= scriptLen) { break; }
		if (len >= maxLineLen) {
			cout << "Script Execution Failed: Line Too Long (max line length is " << maxLineLen << " characters)" << endl;
			return;
		}
		replaceLabels(line, fixedLine, maxLineLen, symbols.GetRoot());
		if (assmCode) {
			if (!assmStartFound) {
				if (fixedLine[0] == '{' && !fixedLine[1]) {
					assmStartFound = true;
					continue;
				} else {
					cout << "Script Execution Failed: Expected '{' Before '" << line << "'" << endl;
					return;
				}
			} else {
				if (fixedLine[0] == '}' && fixedLine[1] == 0) {
					assmCode = trimBuffer(assmCode, assmCodeLength + 1);
					assmCode[assmCodeLength] = 0;
					// Assmble Code then Write To Memory
					int mcSize = assmCodeLength * 3;
					unsigned char *mcBuf = new unsigned char[mcSize];
					mcSize = x86::assm32Program(assmCode, mcBuf, mcSize, assmCodeMemory, &globalLabels);
					bool asmFail = false;
					if (mcSize > 0) {
						cout << "Assmebly Successful" << endl;
						cout << "Writing Code To Memory" << endl;
						SIZE_T bytesWritten = 0;
						if (!WriteProcessMemory(processHld, (void *) assmCodeMemory, (void *) mcBuf, mcSize, &bytesWritten) || (int) bytesWritten != mcSize) {
							cout << "Failed To Write To Memory (error " << GetLastError() << ")" << endl;
							asmFail = true;
						}
					} else {
						asmFail = true;
						cout << "Failed To assmeble code" << endl;
					}
					delete [] mcBuf;
					delete [] assmCode;
					assmCode = 0;
					if (asmFail) {
						return;
					}
				} else {
					unsigned int assmLineLen = str_length(fixedLine);
					mem_copy((void *)(assmCode + assmCodeLength), fixedLine, assmLineLen);
					assmCodeLength += assmLineLen;
					assmCode[assmCodeLength++] = '\n';
				}
				continue;
			}
		} else {
			if (startsWith(fixedLine, "new ")) {
				char memName[64];
				unsigned int memorySize;
				if (sscanf(fixedLine, "new %s %d", memName, &memorySize) == 2) {
					cout << "Creating Memory <" << memName << "> size " << memorySize << endl;
					void *allocatedMem = VirtualAllocEx(processHld, 0, memorySize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
					if (allocatedMem) {
						unsigned int iAllocatedMem = (unsigned int) allocatedMem;
						cout << "Memory Allocated @ " << iAllocatedMem << endl;
						unsigned int newLableLength = str_length(memName);
						char *newLabel = new char[newLableLength + 3];
						strCopy(newLabel + 1, memName);
						newLabel[0] = '<';
						newLabel[newLableLength + 1] = '>';
						newLabel[newLableLength + 2] = 0;
						symbols.Put(newLabel, iAllocatedMem);
						continue;
					}
					cout << "Failed To Create Memory (error " << GetLastError() << ")" << endl;
					return;
				} else {
					cout << "Script Execution Failed: new syntax is 'new <label> <size>'" << endl;
					return;
				}
			} else if (startsWith(fixedLine, "set ")) {
				int memoryLocation = parseHexMemAddress(&fixedLine[4]);
				if (assmCodeMemory) {
					char *setLine = &fixedLine[13];
					if (startsWith(setLine, "bytes ")) {
						setLine += 6;
						char bytes[8];
						int count = 0;
						while (*setLine) {
							if (*setLine == '\t' || *setLine == ' ') {
								setLine++;
								continue;
							}
							int b = parseHexByte(setLine);
							if (b < 0) {
								count = 0; // invalidate everything and return to zero to prevent writing anything to process memory
								break;
							}
							setLine += 2;
							bytes[count] = (char) b;
							count++;
						}
						if (count > 0) {
							cout << "Writing Value To Memory" << endl;
							SIZE_T bytesWritten = 0;
							if (!WriteProcessMemory(processHld, (void *) memoryLocation, (void *) bytes, count, &bytesWritten) || (int) bytesWritten != count) {
								cout << "Failed To Write To Memory (error " << GetLastError() << ")" << endl;
								return;
							}
							continue;
						}
					}
				}
				cout << "Script Execution Failed: set Syntax 'set <address in hex | label surrounded by <> marks > bytes <byte 1 in hex> [byte 2 in hex] ... [byte 8 in hex]'" << endl;
				return;
			} else if (startsWith(fixedLine, "asm ")) {
				assmCodeMemory = parseHexMemAddress(&fixedLine[4]);
				if (assmCodeMemory) {
					assmCode = new char[scriptLen];
					assmCodeLength = 0;
					assmStartFound = false;
					continue;
				}
				cout << "Script Execution Failed: asm Syntax 'asm <address in hex | label surrounded by <> marks>'" << endl;
				return;
			}
		}
		cout << "Script Execution Failed: Invalid line '" << line << "'" << endl;
		return;
	}

	if (assmCode) {
		delete [] assmCode;
		cout << "Script Execution Failed: End Of File Error" << endl;
		return;
	}
	cout << "Script Execution Successful" << endl;
}

int main(int argc, char *argv[]) {

	const int exeIdx = 1;
	const int scriptIdx = exeIdx + 1;
	if (argc != (scriptIdx + 1)) {
		cout << "Usage: " << argv[0] << " <exe> <script>" << endl; 
		return 1;
	}
	cout << "Assembler version " << x86::assm32version << endl;
	cout << "Exe: " << argv[exeIdx] << endl;
	cout << "Loading Script" << endl;
	if (!loadScript(argv[scriptIdx])) {
		cout << "Failed To Load Script" << endl;
		return -1;
	}
	cout << "Creating Proccess" << endl;

	char appName[256];
	for (int i = 0;; i++) {
		appName[i] = argv[exeIdx][i];
		if (!argv[exeIdx][i]) {
			break;
		}
	}

	STARTUPINFO info = {0};
	PROCESS_INFORMATION processInfo = {0};
	if (!CreateProcess(NULL, appName, NULL, NULL, false, 0, NULL, NULL, &info, &processInfo))
	{
		cout << "Could Not Create Proccess" << endl;
		return -2;
	}
	cout << "Proccess Created" << endl;
	Sleep(5000);
	execScript(processInfo.hProcess);
	
	releaseScript();

	return 0;
}
