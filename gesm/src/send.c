
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "send.h"

#define UPLOAD_FILE_AS "while-uploading.txt"
#define REMOTE_URL "ftp://ftp.borza.ro" UPLOAD_FILE_AS
#define RENAME_FILE_TO "renamed-and-fine.txt"

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t retcode = fread(ptr, size, nmemb, stream);
	return retcode;
}

unsigned char upload(char *filename)
{
	CURL *curl;
	CURLcode res = 1;
	FILE *hd_src;
	struct stat file_info;

	struct curl_slist *headerlist = 0;
	static const char buf_1 [] = "RNFR " UPLOAD_FILE_AS;
	static const char buf_2 [] = "RNTO " RENAME_FILE_TO;

	/* get the file size of the local file */
	if (stat(filename, &file_info)) {
		printf("Couldn't open '%s': %s\n", filename, strerror(errno));
		return 0;
	}
	printf("Local file size: %1d bytes.\n", file_info.st_size);

	/* get a FILE * of the same file */
	hd_src = fopen(filename, "rb");

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if (curl)
	{
		/* build a list of commands to pass to libcurl */	
		headerlist = curl_slist_append(headerlist, buf_1);
		headerlist = curl_slist_append(headerlist, buf_2);
		
		/* we want to use our own read function */
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);

		/* enable uploading */
		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

		/* specify target */
		curl_easy_setopt(curl, CURLOPT_URL, REMOTE_URL);

		/* pass in that last FTP commands to run after transfer */
		curl_easy_setopt(curl, CURLOPT_POSTQUOTE, headerlist);

		/* now specify which file to upload */
		curl_easy_setopt(curl, CURLOPT_READDATA, hd_src);

		/* Set the size of the file to upload (optional).  If you give a *_LARGE
		 * option you MUST make sure that the type of the passed-in argument is a
		 * curl_off_t. If you use CURLOPT_INFILESIZE (without _LARGE) you must
		 * make sure that to pass in a type 'long' argument. */
		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE,
			(curl_off_t)file_info.st_size);

		/* now run off and do what you've been told */
		res = curl_easy_perform(curl);

		/* clean up the FTP commands list */
		curl_slist_free_all(headerlist);

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	/* close local file */
	fclose(hd_src);

	curl_global_cleanup();

	return (res == 0);
}

