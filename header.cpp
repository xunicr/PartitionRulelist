#include <string>
#include <stdio.h>
#include <iostream>
#include <map>
#include <fstream>

std::map<std::string,int> readHeaderList(char* filename){
  std::map<std::string, int> HI;
  FILE* fp;

  if((fp = fopen(filename, "r")) ==NULL){
    fprintf(stderr, "ERROR!");
  }
  char s[255];
  int x=1;

  while(fscanf(fp, "%s\n", s)!=EOF){
    std::string tmp(s);
    auto itr = HI.find(tmp);
    if(itr !=HI.end()){
      x = itr->second;
      ++x;
      HI.erase(tmp);
      HI.emplace(tmp,x);
      //     std::cout << itr->first << ":" << itr->second << "\n";
      //    std::cout << HI[tmp] << "\n";
      
    }

    else{
      // HI.insert(std::make_pair(tmp,x));
      HI.emplace(tmp,x);
      //     std::cout << HI[tmp] << "\n";
      //    std::cout << itr->first << itr->second << "\n";  
    }

    x=1;
  }
  fclose(fp);
  return HI;
}
