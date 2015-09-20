#include "stdafx.h"
#include "Html.h"

CHtml::CHtml(const char* filename, const char* title)
{ 
	if ((fp = fopen(filename, "w")) != NULL)
	{
		fprintf(fp,
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
			"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
			"<head>\n"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
			"<title>%s</title>\n"
			"</head>\n"
			"<body>\n",
			title);
	}
	else
		printf("error: class CHtml: failed to create a object!\n");
}


CHtml::~CHtml(void)
{
	if (fp != NULL)
	{
		fprintf(fp,
			"</body>\n"
			"</html>\n"
			);
		fclose(fp);
	}
}

void CHtml::SetFont(const int color, const int size, const char* face, const int dir, const int lang)
{
	if (fp != NULL)
	{
		fprintf(fp, 
			"<font color=\"#%6X\" size=\"%d\" face=\"%s\" dir=\"%s\" lang=\"%s\"></font>\n", 
			color, size, face, DIRECT(dir), LANGUAGE(lang));
	}
}

const char* CHtml::LANGUAGE(int lang) const
{
	switch(lang)
	{
	case LANG_CHINESE:
		return "zh";
	case LANG_ENGLISH:
	default:
		return "en";
	}
}

void CHtml::ShowText(const char* text, int headlevel)
{
	if (fp != NULL)
	{
		if (headlevel == 0)
			fprintf(fp, text);
		else if (headlevel > 0 && headlevel < 4)
			fprintf(fp, "<h%d>%s</h%d>\n", headlevel, text, headlevel);
	}
}

void CHtml::ShowImage(const char *imagefile, const int width, const int height, const char* title)
{
	if (fp != NULL)
	{
		fprintf(fp,
			"<img src=\"%s\" title=\"%s\" width=\"%d\" height=\"%d\"/>\n",
			imagefile, title, width, height);
	}
}

void CHtml::ShowImage(const char *imagefile, const int width, const int height, 
	const int hspace, const int vspace, const char* title)
{
	if (fp != NULL)
	{
		fprintf(fp,
			"<img src=\"%s\" title=\"%s\" width=\"%d\" height=\"%d\" hspace=\"%d\" vspace=\"%d\"/>\n",
			imagefile, title, width, height, hspace, vspace);
	}
}

void CHtml::EndLine()
{
	if (fp != NULL)
		fprintf(fp, "<p></p>\n");
}

void CHtml::ShowImageTable(const vector<string>& picnames, const int width, const int height, const int tablecols)
{
	if (fp == NULL || tablecols < 1)
		return;

	int hspace = 5, vspace = 0;

	int npics = picnames.size();
	int rows = npics / tablecols;

	int k = 0;
	fprintf(fp, "<table>\n");
	for (int i = 0; i < rows; i++)
	{
		fprintf(fp, "<tr>\n");
		for (int j = 0; j < tablecols; j++)
			ShowImage(picnames[k++].c_str(), width, height, hspace, vspace);
		EndLine();
		fprintf(fp, "</tr>\n");
	}
	npics = npics % tablecols;
	if (npics > 0)
	{
		fprintf(fp, "<tr>\n");
		for (int j = 0; j < npics; j++)
			ShowImage(picnames[k++].c_str(), width, height, hspace, vspace);
		EndLine();
		fprintf(fp, "</tr>\n");
	}
	fprintf(fp, "</table>\n");
}

void CHtml::ShowImageTable(const vector<string>& picnames, const vector<string>& titles, 
		const int width, const int height, const int tablecols)
{
	if (fp == NULL || tablecols < 1)
		return;

	int hspace = 5, vspace = 0;

	int npics = picnames.size();
	int rows = npics / tablecols;

	int k = 0;
	fprintf(fp, "<table>\n");
	for (int i = 0; i < rows; i++)
	{
		fprintf(fp, "<tr>\n");
		for (int j = 0; j < tablecols; j++, k++)
			ShowImage(picnames[k].c_str(), width, height, hspace, vspace, titles[k].c_str());
		EndLine();
		fprintf(fp, "</tr>\n");
	}
	npics = npics % tablecols;
	if (npics > 0)
	{
		fprintf(fp, "<tr>\n");
		for (int j = 0; j < npics; j++, k++)
			ShowImage(picnames[k].c_str(), width, height, hspace, vspace, titles[k].c_str());
		EndLine();
		fprintf(fp, "</tr>\n");
	}
	fprintf(fp, "</table>\n");
}