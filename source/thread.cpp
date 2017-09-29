/*  This file is part of Checkpoint
>	Copyright (C) 2017 Bernardo Giordano
>
>   This program is free software: you can redistribute it and/or modify
>   it under the terms of the GNU General Public License as published by
>   the Free Software Foundation, either version 3 of the License, or
>   (at your option) any later version.
>
>   This program is distributed in the hope that it will be useful,
>   but WITHOUT ANY WARRANTY; without even the implied warranty of
>   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
>   GNU General Public License for more details.
>
>   You should have received a copy of the GNU General Public License
>   along with this program.  If not, see <http://www.gnu.org/licenses/>.
>   See LICENSE for information.
*/

#include "thread.h"

static std::vector<Thread> threads;

void createThread(ThreadFunc entrypoint)
{
	s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	Thread thread = threadCreate((ThreadFunc)entrypoint, NULL, 4*1024, prio-1, -2, false);
	threads.push_back(thread);
}

void destroyThreads(void)
{
	for (u32 i = 0; i < threads.size(); i++)
	{
		threadJoin(threads.at(i), U64_MAX);
		threadFree(threads.at(i));
	}
}

void threadDownloadFilter(void)
{
	std::string json;
	Result res = callTitleDB(json);
	if (R_FAILED(res))
	{
		createError(res, "Filter update failed. Please check your internet connection...");
	}
	else
	{	
		const char* cstr = json.c_str();
		std::string tofile = "";
		
		for (size_t oldpos = 0, size = json.size(); oldpos < size;)
		{
			const size_t off = json.find("0004", oldpos);
			if (off != std::string::npos)
			{
				char titleid[17] = {0};
				memcpy(titleid, cstr + off, 16);
				titleid[16] = '\0';
				std::string titlestring(titleid);
				
				if (tofile.find(titlestring, 0) == std::string::npos)
				{
					tofile += titlestring;
				}
				
				oldpos = off + 16;				
			}
			else
			{
				break;
			}
		}
		
		res = writeFile(getArchiveSDMC(), (u8*)tofile.c_str(), u8tou16(PATH_FILTER), tofile.size());
		if (R_FAILED(res))
		{
			createError(res, "Filter update failed. Couldn't write the new version to disk.");
		}
	}
}

void threadLoadTitles(void)
{
	if (!fileExist(getArchiveSDMC(), u8tou16(PATH_FILTER)))
	{
		copyFilter();
	}
	
	loadFilter();
	loadTitles();
}