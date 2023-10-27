#include <exception>
#include <filesystem>

#include "Debug/Debug.h"
#include "MemReader/MemReader.h"

#define OFFSET 0xAEE9CE
#define OUT_FILENAME L".//DarkSoulsII_patched.exe"

using namespace std;

bool isExeValid(BYTE* pBytes)
{
	if (pBytes[0] == 0x48 && pBytes[1] == 0xC7 && pBytes[2] == 0x45 && pBytes[3] == 0xEC && pBytes[4] == 0x00 && pBytes[5] == 0x00 && pBytes[6] == 0x02 && pBytes[7] == 0x00)
		return true;

	return false;
}

//This program patches DarkSoulsII.exe so that it allocates more memory to the DebugGUI heap
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		Debug::Alert(Debug::LVL_ERROR, "DebugManagerPatcher.cpp", "Invalid argument count (got %d, expected 2)\n", argc);

		return -1;
	}

	Debug::DebuggerMessage(Debug::LVL_DEBUG, "argv={%s, %s}", argv[0], argv[1]);

	ifstream input(argv[1], ios::binary | ios::in);

	if (!input.good())
	{
		Debug::Alert(Debug::LVL_ERROR, "DebugManagerPatcher.cpp", "Bad input %s\n", argv[1]);

		return -1;
	}

	input.seekg(OFFSET);

	BYTE pPatchBytes[8] = { 0 };
	MemReader::ReadByteArray(&input, pPatchBytes, 7);

	if (!isExeValid(pPatchBytes))
	{
		Debug::Alert(Debug::LVL_ERROR, "DebugManagerPatcher.cpp", "This executable is either invalid or already patched\n");

		return -1;
	}

	input.close();

	//Patch the instruction at OFFSET with mov qword ptr[rbp - 0x14], 0xc80000
	pPatchBytes[6] = 0xC8;
	pPatchBytes[7] = 0x00;

	try
	{
		std::filesystem::copy_file(argv[1], OUT_FILENAME, filesystem::copy_options::overwrite_existing);
	}
	catch (const std::exception& e)
	{
		Debug::Alert(Debug::LVL_ERROR, "DebugManagerPatcher.cpp", e.what());
	}

	ofstream out(OUT_FILENAME, ios::in | ios::out | ios::binary);

	out.seekp(OFFSET);

	out.write((const char*)pPatchBytes, 8);

	out.close();
}
