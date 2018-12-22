#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "function.h"
#include "cuddInt.h"

class Rule{
private :
  char action;
  int ruleNum;
  std::string cond;
public :
  static int ruleLength;
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
  void setRuleLength() { Rule::ruleLength = cond.size(); }
  static int getRuleLength() { return ruleLength; }
};

int Rule::ruleLength = -1;

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


DdNode* makeBDD(DdManager* gbm, std::string cond){
  // std::cout << cond << std::endl;
  DdNode *bdd=Cudd_ReadOne(gbm);
  DdNode *var, *tmp1, *tmp2, *tmp3;
  int w = cond.size();

  bdd = Cudd_ReadOne(gbm);
  Cudd_Ref(bdd);
  for (int i = w-1; i >= 0; --i) {
    std::cout << cond[i];
    if ('*' == cond[i])
      continue;
    else {
      var = Cudd_bddIthVar(gbm, i);
      Cudd_Ref(var); /*Increases the reference count of a node*/
      if ('0' == cond[i]) {
        tmp1 = Cudd_Not(var);
        Cudd_Ref(tmp1);
        Cudd_RecursiveDeref(gbm, var); var = NULL;
      }
      else
        tmp1 = var;
      tmp2 = Cudd_bddAnd(gbm, tmp1, bdd);
      Cudd_Ref(tmp2);
      tmp3 = bdd;
      bdd = tmp2;
      Cudd_RecursiveDeref(gbm, tmp1);
      Cudd_RecursiveDeref(gbm, tmp3);
      tmp1 = tmp2 = tmp3 = NULL;
    }
  }
  std::cout << std::endl;
  return bdd;
