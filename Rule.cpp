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


DdNode* makeBDDforRule(DdManager* gbm, std::string cond){
  // std::cout << cond << std::endl;
  DdNode *bdd=Cudd_ReadOne(gbm);
  DdNode *var, *tmp1, *tmp2, *tmp3;
  int w = cond.size();

  bdd = Cudd_ReadOne(gbm);
  Cudd_Ref(bdd);
  for (int i = w-1; i >= 0; --i) {
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
  return bdd;
}



DdNode* makeZDDforRule(DdManager* gzm, std::string cond){
  // std::cout << cond << std::endl;
  DdNode *zdd=Cudd_ReadOne(gzm);
  DdNode *var, *tmp1, *tmp2, *tmp3;
  int w = cond.size();

  zdd = Cudd_ReadOne(gzm);
  Cudd_Ref(zdd);
  for (int i = w-1; i >= 0; --i) {
    if ('0' == cond[i])
      continue;
    else if ('1'== cond[i]) {
      var = Cudd_zddChange(gzm, zdd, i);
      Cudd_Ref(var); /*Increases the reference count of a node*/
      Cudd_RecursiveDeref(gzm, zdd);
      zdd = var;
      var = NULL;
    }
    else{
      tmp1 = Cudd_zddChange(gzm, zdd, i);
      tmp2 = Cudd_zddUnion(gzm, zdd, tmp1);
      Cudd_RecursiveDeref(gzm, zdd);
      Cudd_RecursiveDeref(gzm, tmp1);
      zdd = tmp2;
      tmp1 = tmp2 = NULL;
    }
  }
  return zdd;
}

std::vector<DdNode*> makeBddsForRuleList(DdManager* gbm, std::vector<Rule>& R) {
  std::vector<DdNode*> bdds;
  for (int i = 0; i < R.size(); ++i)
    bdds.push_back(makeBDD(gbm, R[i].getCond()));
}

void partitionRulelistViaBDD(DdManager* gbm, std::vector<Rule>& R, std::vector<DdNode*>* gamma, std::vector<std::set<int> >* S) {
  // std::vector<DdNode*> bdd = makeBddsForRuleList(gbm, R);

  for (int i = 0; i < R.size(); i++) {
    // printf("r[%d]\n", i);
    DdNode* X = makeBDD(gbm, R[i].getCond()); // xにpacket(ri)を入れる;
    std::set<int> T; T.insert(i);
    Cudd_Ref(X);
    const DdNode* ONE = Cudd_ReadOne(gbm);
    const DdNode* ZERO = Cudd_ReadLogicZero(gbm);

    for(int j = 0; j < gamma->size(); j++) {
      DdNode* XandGamma = Cudd_bddAnd(gbm, X, (*gamma)[j]);
      Cudd_Ref(XandGamma);
      if(ZERO == XandGamma) {
	Cudd_RecursiveDeref(gbm, XandGamma); XandGamma = NULL;
	//	printf("[%d, %d] Empty\n", i, j);
	continue; //line 6
      }
      else {
	DdNode* NotX = Cudd_Not(X) ;
	Cudd_Ref(NotX);
	DdNode* notXorGamma =Cudd_bddOr(gbm,NotX, (*gamma)[j]) ;
	Cudd_Ref(notXorGamma);
	DdNode* GammaAndNotX = Cudd_bddAnd(gbm,NotX, (*gamma)[j]);
	Cudd_Ref(GammaAndNotX); 

	if(ONE == notXorGamma){
	  gamma->push_back(GammaAndNotX); //line8 
	  Cudd_RecursiveDeref(gbm, (*gamma)[j]);

	  (*gamma)[j] = X ; //line 9
	  Cudd_Ref((*gamma)[j]);
	  Cudd_RecursiveDeref(gbm, XandGamma);
	  Cudd_RecursiveDeref(gbm, notXorGamma);
	  Cudd_RecursiveDeref(gbm, NotX);
	  Cudd_RecursiveDeref(gbm, GammaAndNotX);
	  //	  printf("[%d,%d] x is a subset of gamma\n", i, j);
	  S->push_back((*S)[j]);
	  break; //line10
	}
	else {
	  DdNode* NotGamma = Cudd_Not((*gamma)[j]);
	  Cudd_Ref(NotGamma);
	  DdNode* NotGammaOrX = Cudd_bddOr(gbm, NotGamma, X);
	  Cudd_Ref(NotGammaOrX);
	  DdNode* NotGammaAndX = Cudd_bddAnd(gbm, NotGamma, X);
	  Cudd_Ref(NotGammaAndX);

	  if(ONE == NotGammaOrX){
	    Cudd_RecursiveDeref(gbm, X); //line12
	    X = NotGammaAndX;
	    Cudd_Ref(X);
	    Cudd_RecursiveDeref(gbm, XandGamma);
	    Cudd_RecursiveDeref(gbm, notXorGamma);
	    Cudd_RecursiveDeref(gbm, NotX);
	    Cudd_RecursiveDeref(gbm, GammaAndNotX);
	    Cudd_RecursiveDeref(gbm, NotGamma);
	    Cudd_RecursiveDeref(gbm, NotGammaOrX); 
	    Cudd_RecursiveDeref(gbm, NotGammaAndX); 
	    //	    printf("[%d,%d] gamma is a subset of x\n", i, j);
	    (*S)[j].insert(i);
	  }
	  else{
	    gamma->push_back(GammaAndNotX); //line14
	    Cudd_RecursiveDeref(gbm, (*gamma)[j]); //line15
	    (*gamma)[j] = XandGamma;
	    Cudd_Ref((*gamma)[j]);
	    Cudd_RecursiveDeref(gbm, X); //line16
	    X =  NotGammaAndX;
	    Cudd_Ref(X);
	    Cudd_RecursiveDeref(gbm, XandGamma);
	    Cudd_RecursiveDeref(gbm, notXorGamma);
	    Cudd_RecursiveDeref(gbm, NotX);
	    Cudd_RecursiveDeref(gbm, GammaAndNotX);
	    Cudd_RecursiveDeref(gbm, NotGamma);
	    Cudd_RecursiveDeref(gbm, NotGammaOrX); 
	    Cudd_RecursiveDeref(gbm, NotGammaAndX); 
	    //	    printf("[%d,%d] otherwise\n", i, j);    
	    S->push_back((*S)[j]);
	    (*S)[j].insert(i);
	  }	    
	}
      }
      //      Cudd_RecursiveDeref(gbm, XandGamma);
      //     Cudd_RecursiveDeref(gbm, notXorGamma);
      //     Cudd_RecursiveDeref(gbm, NotX);
      //     Cudd_RecursiveDeref(gbm, GammaAndNotX);
      //     Cudd_RecursiveDeref(gbm, NotGamma);
      //     Cudd_RecursiveDeref(gbm, NotGammaOrX); 
      //     Cudd_RecursiveDeref(gbm, NotGammaAndX); 
    }
    gamma->push_back(X);
    S->push_back(T);
    Cudd_RecursiveDeref(gbm,X);
    // printf("r[%d]\n", i);
    //    std::cout << "the size of gamma = " << gamma->size() << std::endl;
  }
}
