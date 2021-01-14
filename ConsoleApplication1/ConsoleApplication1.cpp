// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>

#include <unicode/ucnv.h>
#include <unicode/utypes.h>
#include <unicode/ucsdet.h>

#define BUF_MAX     (1024 * 128)

#include <iostream>

/*
 * data，    传入参数， 需要探测的字符串
 * len，     传入参数， 探测字符串长度
 * detected  传出参数， 探测的最有可能的字符编码名称, 调用者需要释放该字段
**/
bool detectTextEncoding(const char* data, int32_t len, char** detected) {
        UCharsetDetector* csd;
        const UCharsetMatch** csm;
        int32_t matchCount = 0;

        UErrorCode status = U_ZERO_ERROR;

        csd = ucsdet_open(&status);
        if (status != U_ZERO_ERROR)
                return false;

        ucsdet_setText(csd, data, len, &status);
        if (status != U_ZERO_ERROR)
                return false;

        csm = ucsdet_detectAll(csd, &matchCount, &status);
        if (status != U_ZERO_ERROR)
                return false;

#if 0 //打印出探测的可能的编码
        int32_t match;
        for (match = 0; match < matchCount; match += 1)
        {
                const char* name = ucsdet_getName(csm[match], &status);
                const char* lang = ucsdet_getLanguage(csm[match], &status);
                int32_t confidence = ucsdet_getConfidence(csm[match], &status);

                if (lang == NULL || strlen(lang) == 0)
                        lang = "**";

                printf("%s (%s) %d\n", name, lang, confidence);
        }
#endif

        if (matchCount > 0) {
                *detected = _strdup(ucsdet_getName(csm[0], &status)); //分配了内存， 需要释放
                if (status != U_ZERO_ERROR)
                        return false;
        }

        printf("charset = %s\n", *detected);

        ucsdet_close(csd);
        return true;
}


/*
 * toConverterName,      转换后的字符编码
 * fromConverterName,    转换前的字符编码
 * target,               存储转换后的字符串， 传出参数
 * targetCapacity,       存储容量，target的大小
 * source，              需要转换的字符串
 * sourceLength,         source的大小
**/
int convert(const char* toConverterName, const char* fromConverterName,
        char* target, int32_t targetCapacity, const char* source, int32_t sourceLength) {
        UErrorCode error = U_ZERO_ERROR;
        ucnv_convert(toConverterName, fromConverterName, target, targetCapacity, source, sourceLength, &error);

        return error;
}


int main(int argc, char** argv)
{
	if (argc <= 1) {
		printf("Usage: %s [filename]...\n", argv[0]);
		return -1;
	}

	FILE* file;
	char* filename = argv[1];

	file = fopen(filename, "rb");
	if (file == NULL) {
		printf("Cannot open file \"%s\"\n\n", filename);
		return -1;
	}

	int len = 0;
	char* detected = NULL;

	char* buffer = new char[BUF_MAX];
	char* target = new char[BUF_MAX * 2];

	while (true) {
		memset(buffer, 0, BUF_MAX);
		memset(target, 0, BUF_MAX * 2);

		len = (int32_t)fread(buffer, sizeof(char), BUF_MAX, file);

		if (detected == NULL) {
			if (!detectTextEncoding(buffer, len, &detected)) //编码探测
				break;
		}
#if 0
		//转换为utf8字符编码
		if (convert("UTF-8", detected, target, BUF_MAX * 2, (const char*)buffer, len) != U_ZERO_ERROR) {
			printf("ucnv_convert error");
			break;
		}
#endif
		printf("%s", target); //打印出转换的文件的字符串

		if (len < BUF_MAX)
			break;
	}

	delete[] buffer;
	delete[] target;
	delete[] detected;
	fclose(file);

}
