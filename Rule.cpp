#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

class Rule{
private :
  char action;
  int ruleNum;
  std::string cond;
public :
  Rule() : action('D'), ruleNum(-1), cond("") {}
  Rule(char a, int i, std::string s) : action(a), ruleNum(i), cond(s) {}
  Rule(const Rule& r) {
    action = r.action;
    ruleNum = r.ruleNum;
    cond = r.cond;
  }
  ~Rule() { }
  char getAction() { return action; }
  int getRuleNum() { return ruleNum; }
  std::string getCond() { return cond; }
};

std::vector<Rule > readRulelist(char* rulelistname){
  FILE* fp;
  if((fp = fopen(rulelistname, "r")) ==NULL){
    fprintf(stderr, "ERROR!");
  }
  std::vector<Rule > R;
  char s[255];
  char a;

  for(int k=0; EOF!=fscanf(fp,"%c %s\n",&a,s);++k){
    std::string tmp(s);
    Rule r(a,k,tmp);
    R.push_back(r);
  }
  fclose(fp);

  return R;
}
