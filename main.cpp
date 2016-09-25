#include <stdio.h>
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

string data; //will hold the api's responses contents

vector<string> jsonArrParse(string json) {
   vector<string> aVector;

   string word;
   bool writing = false;

   for (int i = 0; i < json.size(); i++) {
      if (json[i] == '"') {
         if (writing) {
            writing = false;

            aVector.push_back(word);
            word = "";
         } else {
            writing = true;
         }

         continue;
      }

      if (writing) {
         word = word + json[i];
      }
   }

   return aVector;
}

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up) { 
    //buf is a pointer to the data that curl has for us
    //size*nmemb is the size of the buffer

    for (int c = 0; c<size*nmemb; c++)
    {
        data.push_back(buf[c]);
    }
    return size * nmemb; //tell curl how many bytes we handled
}

string httpRequest(string url) {
   CURL *curl;
   CURLcode res;

   curl = curl_easy_init();

   if(curl) {
      curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);

      /* example.com is redirected, so we tell libcurl to follow redirection */ 
      curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

      /* Perform the request, res will get the return code */ 
      res = curl_easy_perform(curl);

      /* Check for errors */ 
      if(res != CURLE_OK)
         fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));

      curl_easy_cleanup(curl);

      string ret = data;
      data = "";
      return ret;
   } 
}

string getCourseInfo(string department, string courseNumber, string section) {
   string url = "http://api.lib.sfu.ca/courses/course?term=&department=" + department + 
                  "&number=" + courseNumber + "&section=" + section;

   json info = json::parse(httpRequest(url));

   string infoStr;

   for (int i = 0; i < info["schedules"].size(); i++) {
      json curr = info["schedules"][i];

      string days = curr["days"];

      if (curr["exam"]) {
         infoStr += "EXAM: ";
      } else {
         infoStr += info["sectionCode"];
         infoStr += ": ";
      }

      for (int j = 0; j < days.size(); j++) {
         switch (days[j]) {
            case 'M':
               infoStr += "Monday ";
               break;
            case 'T':
               infoStr += "Tuesday ";
               break;
            case 'W':
               infoStr += "Wednesday ";
               break;
            case 'R':
               infoStr += "Thursday ";
               break;
            case 'F':
               infoStr += "Friday ";
               break;
            default:
               infoStr += days[j];
         }
      }

      infoStr += " in ";
      infoStr += curr["campus"];
      infoStr += "   Time: ";
      infoStr += curr["startTime"];
      infoStr += "-";
      infoStr += curr["endTime"];

      infoStr += "\n";
   }

   return infoStr;
}

vector<string> getSections(string department, string courseNumber) {
   string url = "http://api.lib.sfu.ca/courses/sections?term=&department=" + department + "&number=" + courseNumber;
      
   return jsonArrParse(httpRequest(url));
}

int main(void) {
   string input;
   string department;
   string courseNumber;

   getline(cin, input);


   for (int i = 0; i < input.size(); i++) {
      if (isdigit(input[i])) {
         courseNumber = courseNumber + input[i];
      } else {
         if (isalpha(input[i])) {
            department = department + input[i];
         }
      }
   }

   vector<string> sections = getSections(department, courseNumber);

   for (int i = 0; i < sections.size(); i++) {
      cout << sections[i] << endl;
   }

   cin >> input;

   cout << getCourseInfo(department, courseNumber, input);


   return 0;
}
