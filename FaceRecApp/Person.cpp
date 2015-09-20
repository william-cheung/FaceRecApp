#include "StdAfx.h"
#include "Person.h"

CPerson::CPerson(const CString& name)
{
#ifdef UNICODE
	const wchar_t* wstr = name.GetString();
	int n = name.GetLength();
	char* str = new char[n + 1];
	for (int i = 0; i < n; i++)
		str[i] = (char)wstr[i];
	str[n] = '\0';
	this->name = str;
	delete[] str;
#else
	this->name = name.GetString();
#endif
}