/*
 * Copyright 2004-2017 Cray Inc.
 * Other additional copyright holders may be indicated within.
 *
 * The entirety of this work is licensed under the Apache License,
 * Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/************************************* | **************************************
*                                                                             *
* A ResolveScope supports the first phase in mapping a name to a lexically    *
* scoped symbol. A scope is a set of bindings where a binding is a mapping    *
* from a name to a single symbol.  The name must be unique within a given     *
* scope.                                                                      *
*                                                                             *
*                                                                             *
* May 8, 2017:                                                                *
*   Until recently this state was implemented using a std::map and a handful  *
* of file static function.  This first implementation simply wraps that code. *
*                                                                             *
* This representation is able to detect disallowed reuse of a name within a   *
* scope but does not fully support function overloading; a name can only be   *
* mapped to a single AST node.                                                *
*                                                                             *
* Scopes are created for 4 kinds of AST nodes                                 *
*    1) A  BlockStmt  with tag BLOCK_NORMAL                                   *
*    2) An FnSymbol   defines a scope for its formals and query variables     *
*    3) A  TypeSymbol for an enum type                                        *
*    4) A  TypeSymbol for an aggregate type                                   *
*                                                                             *
************************************** | *************************************/

#include "ResolveScope.h"

#include "scopeResolve.h"
#include "stmt.h"
#include "symbol.h"
#include "type.h"

static std::map<BaseAST*, ResolveScope*> sScopeMap;

ResolveScope* ResolveScope::findOrCreateScopeFor(DefExpr* def) {
  BaseAST*      ast    = getScope(def);
  ResolveScope* retval = NULL;

  if (ast == rootModule->block) {
    ast = theProgram->block;
  }

  retval = getScopeFor(ast);

  if (retval == NULL) {
    if (BlockStmt* blockStmt = toBlockStmt(ast)) {
      retval = new ResolveScope(blockStmt);

    } else if (FnSymbol*  fnSymbol = toFnSymbol(ast)) {
      retval = new ResolveScope(fnSymbol);

    } else if (TypeSymbol* typeSymbol = toTypeSymbol(ast)) {
      retval = new ResolveScope(typeSymbol);

    } else {
      INT_ASSERT(false);
    }

    sScopeMap[ast] = retval;
  }

  return retval;
}

ResolveScope* ResolveScope::getScopeFor(BaseAST* ast) {
  std::map<BaseAST*, ResolveScope*>::iterator it;
  ResolveScope*                               retval = NULL;

  it = sScopeMap.find(ast);

  if (it != sScopeMap.end()) {
    retval = it->second;
  }

  return retval;
}

void ResolveScope::destroyAstMap() {
  std::map<BaseAST*, ResolveScope*>::iterator it;

  for (it = sScopeMap.begin(); it != sScopeMap.end(); it++) {
    delete it->second;
  }

  sScopeMap.clear();
}

/************************************* | **************************************
*                                                                             *
* Historically, definitions have been mapped to scopes by                     *
*   1) Walking gDefExprs                                                      *
*   2) Determining the "scope" for a given DefExpr by walking upwards         *
*      through the AST.                                                       *
*   3) Validating that the definition is valid.                               *
*   4) Extending the scope with the definition                                *
*                                                                             *
* As a special case the built-in symbols, which are defined in rootModule,    *
* are mapped as if they were defined in chpl_Program.                         *
*                                                                             *
* 2017/05/23:                                                                 *
*   Begin to modify this process so that scopes and definitions are managed   *
* using a traditional top-down traversal of the AST starting at chpl_Program. *
*                                                                             *
* This process will overlook the compiler defined built-ins.  This function   *
* is responsible for pre-allocating the scope for chpl_Program and then       *
* inserting the built-ins.
*                                                                             *
************************************** | *************************************/

void ResolveScope::initializeScopeForChplProgram() {
  ResolveScope* program = new ResolveScope(theProgram->block);

  program->extend(dtObject->symbol);
  program->extend(dtVoid->symbol);
  program->extend(dtStringC->symbol);

  program->extend(gFalse);
  program->extend(gTrue);

  program->extend(gTryToken);

  program->extend(dtNil->symbol);
  program->extend(gNil);

  program->extend(gNoInit);

  program->extend(dtUnknown->symbol);
  program->extend(dtValue->symbol);

  program->extend(gUnknown);
  program->extend(gVoid);

  program->extend(dtBools[BOOL_SIZE_SYS]->symbol);
  program->extend(dtBools[BOOL_SIZE_1]->symbol);
  program->extend(dtBools[BOOL_SIZE_8]->symbol);
  program->extend(dtBools[BOOL_SIZE_16]->symbol);
  program->extend(dtBools[BOOL_SIZE_32]->symbol);
  program->extend(dtBools[BOOL_SIZE_64]->symbol);

  program->extend(dtInt[INT_SIZE_8]->symbol);
  program->extend(dtInt[INT_SIZE_16]->symbol);
  program->extend(dtInt[INT_SIZE_32]->symbol);
  program->extend(dtInt[INT_SIZE_64]->symbol);

  program->extend(dtUInt[INT_SIZE_8]->symbol);
  program->extend(dtUInt[INT_SIZE_16]->symbol);
  program->extend(dtUInt[INT_SIZE_32]->symbol);
  program->extend(dtUInt[INT_SIZE_64]->symbol);

  program->extend(dtReal[FLOAT_SIZE_32]->symbol);
  program->extend(dtReal[FLOAT_SIZE_64]->symbol);

  program->extend(dtImag[FLOAT_SIZE_32]->symbol);
  program->extend(dtImag[FLOAT_SIZE_64]->symbol);

  program->extend(dtComplex[COMPLEX_SIZE_64]->symbol);
  program->extend(dtComplex[COMPLEX_SIZE_128]->symbol);

  program->extend(dtStringCopy->symbol);
  program->extend(gStringCopy);

  program->extend(dtCVoidPtr->symbol);
  program->extend(dtCFnPtr->symbol);
  program->extend(gCVoidPtr);
  program->extend(dtSymbol->symbol);

  program->extend(dtFile->symbol);
  program->extend(gFile);

  program->extend(dtOpaque->symbol);
  program->extend(gOpaque);

  program->extend(dtTaskID->symbol);
  program->extend(gTaskID);

  program->extend(dtSyncVarAuxFields->symbol);
  program->extend(gSyncVarAuxFields);

  program->extend(dtSingleVarAuxFields->symbol);
  program->extend(gSingleVarAuxFields);

  program->extend(dtAny->symbol);
  program->extend(dtIntegral->symbol);
  program->extend(dtAnyComplex->symbol);
  program->extend(dtNumeric->symbol);

  program->extend(dtIteratorRecord->symbol);
  program->extend(dtIteratorClass->symbol);

  program->extend(dtMethodToken->symbol);
  program->extend(gMethodToken);

  program->extend(dtTypeDefaultToken->symbol);
  program->extend(gTypeDefaultToken);

  program->extend(dtModuleToken->symbol);
  program->extend(gModuleToken);

  program->extend(dtAnyEnumerated->symbol);

  program->extend(gBoundsChecking);
  program->extend(gCastChecking);
  program->extend(gDivZeroChecking);
  program->extend(gPrivatization);
  program->extend(gLocal);
  program->extend(gNodeID);

  sScopeMap[theProgram->block] = program;
}

/************************************* | **************************************
*                                                                             *
*                                                                             *
*                                                                             *
************************************** | *************************************/

ResolveScope::ResolveScope(BlockStmt* blockStmt) {
  mAstRef = blockStmt;
}

ResolveScope::ResolveScope(FnSymbol*  fnSymbol)  {
  mAstRef = fnSymbol;
}

ResolveScope::ResolveScope(TypeSymbol* typeSymbol) {
  Type* type = typeSymbol->type;

  INT_ASSERT(isEnumType(type) || isAggregateType(type));

  mAstRef = typeSymbol;
}

bool ResolveScope::extend(Symbol* newSym) {
  const char* name   = newSym->name;
  bool        retval = false;

  if (Symbol* oldSym = lookup(name)) {
    FnSymbol* oldFn = toFnSymbol(oldSym);
    FnSymbol* newFn = toFnSymbol(newSym);

    // Do not complain if they are both functions
    if (oldFn != NULL && newFn != NULL) {
      retval = true;

    // e.g. record String and proc String(...)
    } else if (isAggregateTypeAndConstructor(oldSym, newSym) == true ||
               isAggregateTypeAndConstructor(newSym, oldSym) == true) {
      retval = true;

    // Methods currently leak their scope and can conflict with variables
    } else if (isSymbolAndMethod(oldSym, newSym)             == true ||
               isSymbolAndMethod(newSym, oldSym)             == true) {
      retval = true;

    } else {
      USR_FATAL(oldSym,
                "'%s' has multiple definitions, redefined at:\n  %s",
                name,
                newSym->stringLoc());
    }

    // If oldSym is a constructor and newSym is a type, update with the type
    if (newFn == NULL || newFn->_this == NULL) {
      mBindings[name] = newSym;
    }

  } else {
    mBindings[name] = newSym;
    retval          = true;
  }

  return retval;
}

bool ResolveScope::isAggregateTypeAndConstructor(Symbol* sym0, Symbol* sym1) {
  TypeSymbol* typeSym = toTypeSymbol(sym0);
  FnSymbol*   funcSym = toFnSymbol(sym1);
  bool        retval  = false;

  if (typeSym != NULL && funcSym != NULL && funcSym->_this != NULL) {
    AggregateType* type0 = toAggregateType(typeSym->type);
    AggregateType* type1 = toAggregateType(funcSym->_this->type);

    retval = (type0 == type1) ? true : false;
  }

  return retval;
}

bool ResolveScope::isSymbolAndMethod(Symbol* sym0, Symbol* sym1) {
  FnSymbol* fun0   = toFnSymbol(sym0);
  FnSymbol* fun1   = toFnSymbol(sym1);
  bool      retval = false;

  if (fun0 == NULL && fun1 != NULL && fun1->_this != NULL) {
    retval = true;
  }

  return retval;
}

Symbol* ResolveScope::lookup(const char* name) const {
  std::map<const char*, Symbol*>::const_iterator it     = mBindings.find(name);
  Symbol*                                        retval = NULL;

  if (it != mBindings.end()) {
    retval = it->second;
  }

  return retval;
}
