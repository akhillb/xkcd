#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
 
#include <curl/curl.h>
 
struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}
 
int main(void)
{
  CURL *curl_handle;
  CURLcode res;
  struct json_object *jobj;
  struct json_object *jobj1;

  static const char *pagefilename = "xkcd.png";
  FILE *pagefile;
 
  struct MemoryStruct chunk;
 
  chunk.memory = malloc(1);
  chunk.size = 0;
 
  curl_global_init(CURL_GLOBAL_ALL);
 
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, "https://xkcd.com/info.0.json");
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
 
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
 
  res = curl_easy_perform(curl_handle);
 
  if(res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
  }
  else {
    jobj = json_tokener_parse(chunk.memory);
    json_object_object_get_ex(jobj, "img", &jobj1);
  }
  const char *json_string = json_object_to_json_string_ext(jobj1, JSON_C_TO_STRING_SPACED);
  char new_json_string[strlen(json_string)];
  int i = 0;
  int j = 0;
  while(json_string[i] != '\0') {
    i++;
    if(json_string[i] == '"' || json_string[i] == '\\') continue;
    new_json_string[j] = json_string[i];
    j++;
  }
  new_json_string[j] = '\0';

  curl_easy_setopt(curl_handle, CURLOPT_URL, new_json_string);
  curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
	pagefile = fopen(pagefilename, "wb");
  if(pagefile) {
 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
 
    curl_easy_perform(curl_handle);
 
    fclose(pagefile);
  }
 
  curl_easy_cleanup(curl_handle);
 
  free(chunk.memory);
 
  curl_global_cleanup();
 
  return 0;
}
