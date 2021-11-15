//tt_error.cpp - Error handling
//Copyright (C) 2021 Ayman Wagih
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include		<stdio.h>
#include		<stdarg.h>
#include		<string.h>
#include		"tt.h"
extern const int e_msg_size=2048;
char			first_error_msg[e_msg_size]={}, latest_error_msg[e_msg_size]={};
bool 			log_error(const char *file, int line, const char *format, ...)
{
	bool firsttime=first_error_msg[0]=='\0';
	char *buf=first_error_msg[0]?latest_error_msg:first_error_msg;
	va_list args;
	va_start(args, format);
	vsprintf_s(g_buf, e_msg_size, format, args);
	va_end(args);
	int size=strlen(file), start=size-1;
	for(;start>=0&&file[start]!='/'&&file[start]!='\\';--start);
	start+=start==-1||file[start]=='/'||file[start]=='\\';
//	int length=snprintf(buf, e_msg_size, "%s (%d)%s", g_buf, line, file+start);
//	int length=snprintf(buf, e_msg_size, "%s\n%s(%d)", g_buf, file, line);
//	int length=snprintf(buf, e_msg_size, "%s(%d):\n\t%s", file, line, g_buf);
	int length=sprintf_s(buf, e_msg_size, "%s(%d): %s", file+start, line, g_buf);
	if(firsttime)
	{
		memcpy(latest_error_msg, first_error_msg, length);
		messagebox("Error", latest_error_msg);//redundant, since report_error/emergencyPrint prints both
	}
	return firsttime;
}
void			report_errors()
{
	if(latest_error_msg[0])
	{
#ifdef _WIN32
		console_show();
#endif
		printf("%s\n", latest_error_msg);
	}
}