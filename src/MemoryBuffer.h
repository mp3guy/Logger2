/*
 * MemoryBuffer.h
 *
 *  Created on: 21 May 2013
 *      Author: thomas
 */

#ifndef MEMORYBUFFER_H_
#define MEMORYBUFFER_H_

#include <QProgressDialog>
#include <QApplication>

#include <string>
#include <list>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <cassert>

#include "ThreadMutexObject.h"

#ifndef OS_WINDOWS
#ifndef __APPLE__
#include <sys/types.h>
#include <sys/sysinfo.h>
#else
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <mach/task_info.h>
#include <mach/task.h>
#endif
#else
#include <windows.h>
#include <psapi.h>
#endif

class MemoryBuffer
{
    public:
        MemoryBuffer()
         : totalBytes(0)
        {
            memoryFull.assignValue(false);
        }

        virtual ~MemoryBuffer()
        {
            clear();
        }

        class DataChunk
        {
            public:
                DataChunk(unsigned char * data,
                          int dataSize)
                 : dataSize(dataSize)
                {
                    this->data = (unsigned char *)malloc(dataSize);
                    memcpy(this->data, data, dataSize);
                }

                virtual ~DataChunk()
                {
                    free(data);
                }

                unsigned char * data;
                int dataSize;
        };

        void addData(unsigned char * data, int dataSize)
        {
            dataChunks.push_back(new DataChunk(data, dataSize));

            totalBytes += dataSize;

            if(getTotalSystemMemory() - getUsedSystemMemory() <= 419430400ll)
            {
                memoryFull.assignValue(true);
            }
        }

        void writeOutAndClear(std::string file, int32_t numFrames, QWidget * parent)
        {
            int32_t * frameCount = (int32_t *)dataChunks.front()->data;

            *frameCount = numFrames;

            FILE * logFile = fopen(file.c_str(), "wb+");

            QProgressDialog progress("Writing log to disk...", "Cancel", 0, 100, parent);
            progress.setWindowModality(Qt::WindowModal);
            progress.show();
            progress.setMinimumDuration(1000);

            int64_t writtenBytes = 0;

            for(std::list<DataChunk *>::iterator i = dataChunks.begin(); i != dataChunks.end(); ++i)
            {
                fwrite((*i)->data, (*i)->dataSize, 1, logFile);

                writtenBytes += (*i)->dataSize;

                double amountWritten = ((double)writtenBytes / (double)totalBytes) * 100.0;

                progress.setValue(amountWritten);

                QApplication::processEvents();

                if(progress.wasCanceled())
                {
                    break;
                }
            }

            fflush(logFile);
            fclose(logFile);

            progress.setValue(100);

            progress.close();

            clear();
        }

        void clear()
        {
            for(std::list<DataChunk *>::iterator i = dataChunks.begin(); i != dataChunks.end(); ++i)
            {
                delete *i;
            }

            dataChunks.clear();

            totalBytes = 0;

            memoryFull.assignValue(false);
        }

        ThreadMutexObject<bool> memoryFull;

#ifndef OS_WINDOWS
#ifndef __APPLE__
        static int parseLine(char* line)
        {
            int i = strlen(line);
            while(*line < '0' || *line > '9')
            {
                line++;
            }
            line[i - 3] = '\0';
            i = atoi(line);
            return i;
        }

        static long long getTotalSystemMemory()
        {
            struct sysinfo memInfo;
            sysinfo(&memInfo);

            long long totalPhysMem = memInfo.totalram;
            totalPhysMem *= memInfo.mem_unit;

            return totalPhysMem;
        }

        static long long getUsedSystemMemory()
        {
            struct sysinfo memInfo;
            sysinfo(&memInfo);

            long long physMemUsed = memInfo.totalram - memInfo.freeram - memInfo.sharedram - memInfo.bufferram;
            physMemUsed *= memInfo.mem_unit;

            return physMemUsed;
        }

        static long long getProcessMemory()
        {
            FILE* file = fopen("/proc/self/status", "r");
            int result = -1;
            char line[128];

            while(fgets(line, 128, file) != NULL)
            {
                if(strncmp(line, "VmRSS:", 6) == 0)
                {
                    result = parseLine(line);
                    break;
                }
            }
            fclose(file);
            return result;
        }
#else
        static long long getTotalSystemMemory()
        {
            int mib[2];
            int64_t physical_memory;
            mib[0] = CTL_HW;
            mib[1] = HW_MEMSIZE;
            size_t length = sizeof(int64_t);
            sysctl(mib, 2, &physical_memory, &length, NULL, 0);
            return physical_memory;
        }

        static long long getUsedSystemMemory()
        {
            vm_size_t page_size;
            mach_port_t mach_port;
            mach_msg_type_number_t count;
            vm_statistics_data_t vm_stats;

            mach_port = mach_host_self();

            count = sizeof(vm_stats) / sizeof(natural_t);

            if(KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
               KERN_SUCCESS == host_statistics(mach_port, HOST_VM_INFO, (host_info_t)&vm_stats, &count))
            {
                return ((int64_t)vm_stats.active_count +
                        (int64_t)vm_stats.inactive_count +
                        (int64_t)vm_stats.wire_count) * (int64_t)page_size;
            }
            
            return 0;
        }

        static long long getProcessMemory()
        {
            struct task_basic_info t_info;
            mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

            if(KERN_SUCCESS != task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&t_info, &t_info_count))
            {
                return -1;
            }

            return t_info.resident_size;
        }
#endif
#else
        static long long getTotalSystemMemory()
        {
            MEMORYSTATUSEX memInfo;
            memInfo.dwLength = sizeof(MEMORYSTATUSEX);
            GlobalMemoryStatusEx(&memInfo);
            return memInfo.ullTotalPhys;
        }

        static long long getUsedSystemMemory()
        {
            MEMORYSTATUSEX memInfo;
            memInfo.dwLength = sizeof(MEMORYSTATUSEX);
            GlobalMemoryStatusEx(&memInfo);
            return memInfo.ullTotalPhys - memInfo.ullAvailPhys;
        }

        static long long getProcessMemory()
        {
            PROCESS_MEMORY_COUNTERS_EX pmc;
            GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
            return pmc.WorkingSetSize;
        }
#endif

    private:
        std::list<DataChunk *> dataChunks;
        int64_t totalBytes;
};


#endif /* MEMORYBUFFER_H_ */
