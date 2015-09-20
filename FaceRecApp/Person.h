#pragma once

#include <string>

class CPerson
{
private:
	std::string name;

public:
	CPerson(const char* name = "") { this->name = name; }
	CPerson(const CString& name);
	~CPerson(void) {}

	bool operator == (const CPerson& person) const { return this->name == person.name; }
	bool operator != (const CPerson& person) const { return !(*this == person); }
	const char* getName() const { return name.c_str(); } 
};

