#pragma once

// Class CHtml
// Created: 2013.04.29, by Zhang Aimin

#include <stdio.h>
#include <vector>
using namespace std;

// font colors
#define COLOR_BLACK		0x000000
#define COLOR_WHITE		0xFFFFFF
#define COLOR_RED			0xFF0000
#define COLOR_GREEN		0x00FF00
#define COLOR_BLUE		0x0000FF

// font styles
#define FONT_CAMBRIA				"Cambria"
#define FONT_COURIER_NEW			"Courier New"
#define FONT_SEGOE_UI_SYMBOL		"Segoe UI Symbol"

// font directions
#define DIRECT_LTR	0
#define DIRECT_RTL	1
#define DIRECT(dir)	(dir == DIRECT_LTR ? "ltr" : "rtl")

// font languages
// Note : previous definition in 'winnt.h' 
//#define LANG_CHINESE	0
//#define LANG_ENGLISH	1

class CHtml
{
public:
	// Create a HTML file named (filename), whose title is (title)
	CHtml(const char* filename, const char* title = "Untitled");
	~CHtml(void);

	void SetFont(const int color = COLOR_BLACK, 
		const int size = 5, 
		const char* face = FONT_CAMBRIA, 
		const int dir = DIRECT_LTR, 
		const int lang = LANG_ENGLISH);
	void ShowText(const char* text, int headlevel = 0);
	void ShowImage(const char *imagefile, const int width, const int height, const char* title = "");
	void ShowImage(const char *imagefile, const int width, const int height, 
		const int hspace, const int vspace, const char* title = "");
	void ShowImageTable(const vector<string>& picnames, const int width, const int height, const int cols);
	void ShowImageTable(const vector<string>& picnames, const vector<string>& titles, 
		const int width, const int height, const int cols);
	void EndLine();

private:
	CHtml(const CHtml& h);
	CHtml& operator=(const CHtml& h);

	const char* LANGUAGE(int lang) const;

private:
	FILE* fp; // HTML file pointer
}; // end class CHtml
