#include "../../CZetScript.h"


CUndefined *CScriptVariable::UndefinedSymbol=NULL;
//CVoid *CScriptVariable::VoidSymbol=NULL;
CNull *CScriptVariable::NullSymbol=NULL;



void CScriptVariable::createSingletons(){
	NullSymbol = NEW_NULL_VAR;
	UndefinedSymbol = NEW_UNDEFINED_VAR;
	//VoidSymbol = NEW_VOID_VAR;

}


void CScriptVariable::destroySingletons(){
	delete UndefinedSymbol;
	//delete VoidSymbol;
	delete NullSymbol;
}

CScriptVariable::tSymbolInfo *CScriptVariable::addFunctionSymbol(const string & symbol_value,int _idxAstNode,tScriptFunctionObject *irv){
	tSymbolInfo si;
	si.proxy_ptr=0;
	si.object = irv;

	// get super function ...
	si.super_function = getFunctionSymbol(symbol_value);

	si.symbol_value = symbol_value;
	si.idxAstNode = _idxAstNode;
	m_functionSymbol.push_back(si);


	return &m_functionSymbol[m_functionSymbol.size()-1];
}

void CScriptVariable::createSymbols(tInfoRegisteredClass *ir_class){
	tSymbolInfo *si;

	//if(ir_class == this->m_infoRegisteredClass)
	{ // only register vars from higher class because the heredited symbols were created at CCompiler::gacClass

		for ( unsigned i = 0; i < ir_class->metadata_info.object_info.local_symbols.m_registeredVariable.size(); i++){


			tInfoRegisteredVariableSymbol * ir_var = &ir_class->metadata_info.object_info.local_symbols.m_registeredVariable[i];

			si = addVariableSymbol(ir_var->symbol_name,ir_var->idxAstNode);

			// Warning if you put any var for primitives (i.e CInteger, CNumber, etc will crash in recursive manner)
			if(IS_CLASS_C){ // we know the type object so we allocate new var symbol ...
				// check if primitive type (only 4 no more no else)...
				void *ptr_variable = (void*) ((unsigned long long) c_object + ir_var->ref_ptr);

				if(CScriptClass::valid_C_PrimitiveType[CScriptClass::INT_PTR_TYPE].type_str==ir_var->c_type.c_str()){//={typeid(int *).name(),"int *",INT_PTR_TYPE};
					//si->object = new CInteger(CScriptClass::getInstance()->getRegisteredClassInteger(),(int *)ptr_variable);
					CInteger *i= new CInteger();
					i->m_intValue=*((int *)ptr_variable);
					si->object =i;

				}else if(CScriptClass::valid_C_PrimitiveType[CScriptClass::FLOAT_PTR_TYPE].type_str==ir_var->c_type.c_str()){//={typeid(float *).name(),"float *",FLOAT_PTR_TYPE};
					//si->object = new CNumber(CScriptClass::getInstance()->getRegisteredClassNumber(),(float *)ptr_variable);
					CNumber *n= new CNumber();
					n->m_floatValue=*((float *)ptr_variable);
					si->object =n;

				}else if(CScriptClass::valid_C_PrimitiveType[CScriptClass::STRING_PTR_TYPE].type_str==ir_var->c_type.c_str()){//={typeid(string *).name(),"string *",STRING_PTR_TYPE};
					//si->object = new CString(CScriptClass::getInstance()->getRegisteredClassString(),(string *)ptr_variable);
					CString *s= new CString();
					s->m_strValue=*((string *)ptr_variable);
					si->object =s;

				}else if(CScriptClass::valid_C_PrimitiveType[CScriptClass::BOOL_PTR_TYPE].type_str==ir_var->c_type.c_str()){//={typeid(bool *).name(),"bool *",BOOL_PTR_TYPE};
					CBoolean *b= new CBoolean();
					b->m_boolValue=*((bool *)ptr_variable);
					si->object =b;
				}else{
					tInfoRegisteredClass *info_registered_class = CScriptClass::getInstance()->getRegisteredClassBy_C_ClassPtr(ir_var->c_type);

					if(info_registered_class){
						CScriptVariable *var = new CScriptVariable();
						var->init(info_registered_class,ptr_variable);
						si->object = var;
					}

				}
				//si->
				//si->object = CScriptClass::getInstance()->get(ir_var->c_type);
			}

		}

	}


	// Register even for primitives (if appropiate)
	for ( unsigned i = 0; i < ir_class->metadata_info.object_info.local_symbols.vec_idx_registeredFunction.size(); i++){
		tScriptFunctionObject * ir_fun  = &ir_class->metadata_info.object_info.local_symbols.vec_idx_registeredFunction[i];
		//print_info_cr("=========================================");
		//print_info_cr("- Create function %s...",irv->object_info.local_symbols.vec_idx_registeredFunction[i].object_info.symbol_info.symbol_name.c_str());
		 si =addFunctionSymbol(
				ir_class->metadata_info.object_info.local_symbols.vec_idx_registeredFunction[i].object_info.symbol_info.symbol_name,
				ir_class->metadata_info.object_info.local_symbols.vec_idx_registeredFunction[i].object_info.symbol_info.idxAstNode,
				ir_fun

				);
		 if((ir_fun->object_info.symbol_info.properties & SYMBOL_INFO_PROPERTIES::PROPERTY_C_OBJECT_REF) == SYMBOL_INFO_PROPERTIES::PROPERTY_C_OBJECT_REF){ // create proxy function ...
			si->proxy_ptr = (*((std::function<void *(void *,PROXY_CREATOR)> *)ir_fun->object_info.symbol_info.ref_ptr))(c_object,PROXY_CREATOR::CREATE_FUNCTION);
		}
		//addSymbol(m_infoRegisteredClass->object_info.local_symbols.m_registeredVariable[i].ast);
	}

}


void CScriptVariable::fun1(int *i){
	print_info_cr("FUN1 (I): %i",*i);
}

void CScriptVariable::fun1(string *s){
	print_info_cr("FUN1 (S): %s",s->c_str());
}

void CScriptVariable::setup(){

	idx_shared_ptr=-1;
	m_infoRegisteredClass = NULL;
	c_object = NULL;
	created_object = NULL;
	m_value = NULL;

}

CScriptVariable::CScriptVariable(){

	setup();



}
/*
CScriptVariable::CScriptVariable(tInfoRegisteredClass *irv, void *_c_object){

	init(irv,_c_object);

}*/

void CScriptVariable::init(tInfoRegisteredClass *irv, void *_c_object){
	setup();




	this->m_infoRegisteredClass = irv;
	//m_rootAst=NULL;
	//m_registeredVariable = NULL;
	//m_type = TYPE::SCRIPT_FUNCTION_TYPE;
	//pointer_function = NULL;
	//m_scope = scope;
	//m_parentFunction = _parentFunction;
	//print_info_cr("creating var type %s",irv->object_info.symbol_info.symbol_name.c_str());

	if(IS_CLASS_C){
		if(_c_object == NULL){
			created_object = (*m_infoRegisteredClass->c_constructor)();
			c_object = created_object;
		}else{
			c_object = _c_object;
		}
	}else{
		c_object = this;
	}


	createSymbols(irv);

	/*if(_parentFunction == NULL){ // this is the main function ...
		setName("Main");
		m_registeredVariable=new vector<tRegisteredSymbolInfo>();
	}
	else {
		_parentFunction->addFunction(this);
	}*/

	// reserve symbolsand its extended classes...

	// create object functions ...
	/*for ( unsigned i = 0; i < m_infoRegisteredClass->object_info.local_symbols.vec_idx_registeredFunction.size(); i++){
		print_info_cr("=========================================");
		print_info_cr("- Create function %s...",m_infoRegisteredClass->object_info.local_symbols.vec_idx_registeredFunction[i].object_info.symbol_info.info_var_scope->name.c_str());
		addFunctionSymbol(
				m_infoRegisteredClass->object_info.local_symbols.vec_idx_registeredFunction[i].object_info.symbol_info.ast,
				&m_infoRegisteredClass->object_info.local_symbols.vec_idx_registeredFunction[i]

				);
		//addSymbol(m_infoRegisteredClass->object_info.local_symbols.m_registeredVariable[i].ast);
	}*/

}

tScriptFunctionObject *CScriptVariable::getConstructorFunction(){

	if(m_infoRegisteredClass->idx_function_script_constructor != -1){
		return &m_infoRegisteredClass->metadata_info.object_info.local_symbols.vec_idx_registeredFunction[m_infoRegisteredClass->idx_function_script_constructor];
	}

	return NULL;
}

int CScriptVariable::getIdxClass(){
	return m_infoRegisteredClass->idxClass;
}

bool CScriptVariable::setIdxClass(int idx){
	tInfoRegisteredClass *_info_registered_class = CScriptClass::getInstance()->getRegisteredClassByIdx(idx);

	if(_info_registered_class == NULL){
		return false;
	}

	m_infoRegisteredClass = _info_registered_class;
	return true;
}


CScriptVariable::tSymbolInfo * CScriptVariable::addVariableSymbol(const string & symbol_value, int _idxAstNode){
	tSymbolInfo si;
	si.proxy_ptr=0;
	si.object = CScriptVariable::UndefinedSymbol;
	si.idxAstNode = _idxAstNode;
	si.symbol_value = symbol_value;
	m_variableSymbol.push_back(si);

	return &m_variableSymbol[m_variableSymbol.size()-1];
}





/*
void CScriptVariable::addArgSymbol(const string & arg_name){
	tSymbolInfo si;
	si.ast = NULL;
	m_arg_symbol.push_back(si);
}
*/
CScriptVariable::tSymbolInfo * CScriptVariable::getVariableSymbol(const string & varname){
	for(unsigned int i = 0; i < this->m_variableSymbol.size(); i++){
		if(varname == this->m_variableSymbol[i].symbol_value){
			return &m_variableSymbol[i];
		}
	}

	return NULL;
}


CScriptVariable::tSymbolInfo * CScriptVariable::getVariableSymbolByIndex(unsigned idx){
	if(idx >= m_variableSymbol.size()){
		print_error_cr("idx symbol index out of bounds (%i)",idx);
		return NULL;
	}

	return &m_variableSymbol[idx];
}

/*
CScriptVariable::tSymbolInfo *getFunctionSymbolRecursive(const string & varname){

}
*/
CScriptVariable::tSymbolInfo * CScriptVariable::getFunctionSymbol(const string & varname){

	// from lat value to first to get last override function...
	for(int i = this->m_functionSymbol.size()-1; i >= 0; i--){
		if(varname == this->m_functionSymbol[i].symbol_value){
			return &m_functionSymbol[i];
		}
	}

	return NULL;
}


int CScriptVariable::getIdxFunctionSymbolWithMatchArgs(const string & varname, vector<CScriptVariable *> *argv, bool match_signature){

	// from lat value to first to get last override function...
	/*for(int i = this->m_functionSymbol.size()-1; i >= 0; i--){
		if(varname == this->m_functionSymbol[i].symbol_value){
			return &m_functionSymbol[i];
		}
	}*/

	bool all_check=false;
	//bool found = false;

	//for(int h=0; h < 2 && !found; h++){

	for(int i = this->m_functionSymbol.size()-1; i>=0; i--){

		tScriptFunctionObject *irfs = (tScriptFunctionObject *)m_functionSymbol[i].object;

		if(this->m_functionSymbol[i].symbol_value == varname && (irfs->m_arg.size() == argv->size())){


			all_check=true;
			// convert parameters script to c...
			for( int k = 0; k < (int)argv->size() && all_check;k++){
				//converted_param[i]= (int)(argv->at(i));
				bool is_script_var = argv->at(k)->getPointer_C_ClassName() == TYPE_SCRIPT_VARIABLE; // if C_ClassName is void means that is a ScriptClass...
				if(match_signature){
					all_check = (argv->at(k)->getPointer_C_ClassName()==irfs->m_arg[k]) || (is_script_var);
				}
				else{
					if((argv->at(k))->getPointer_C_ClassName()!=irfs->m_arg[k] && !(is_script_var)){
						all_check =(CScriptClass::getInstance()->getConversionType((argv->at(k))->getPointer_C_ClassName(),irfs->m_arg[k], false)!=NULL);
					}
				}
			}

			if(all_check){ // we found the right function ...
				return i;
			}
		}
	}
	//}



	return -1;
}

const string & CScriptVariable::getClassName(){
		return m_infoRegisteredClass->metadata_info.object_info.symbol_info.symbol_name;
	}

	const string & CScriptVariable::getPointer_C_ClassName(){
		return m_infoRegisteredClass->classPtrType;
	}


    string * CScriptVariable::toString(){
    	return &m_strValue;
    }

    bool CScriptVariable::refSharedPtr(){

    	if(idx_shared_ptr == -1){

			if((idx_shared_ptr = CSharedPointerManager::getInstance()->newSharedPointer(this)) != -1){
				return true;
			}
    	}
    	else{
    		print_error_cr("idx shared ptr alrady registered");
    	}

		return false;
    }

    bool CScriptVariable::unrefSharedPtr(){
    	if(idx_shared_ptr!=-1){
    		if(CSharedPointerManager::getInstance()->getNumShares(idx_shared_ptr) > 1){
    			print_error_cr("shared pointer more than once");
    			return false;
    		}

    		int index_0_share_idx = CSharedPointerManager::getInstance()->getIdx0Shares(idx_shared_ptr);
    		if(index_0_share_idx!=-1){
    			CSharedPointerManager::getInstance()->remove0Shares(index_0_share_idx);
    		}
    		idx_shared_ptr = -1;
    	}

    	return true;
    }

	int CScriptVariable::get_C_StructPtr(){
		return (int)c_object;
	}


CScriptVariable::tSymbolInfo *CScriptVariable::getFunctionSymbolByIndex(unsigned idx){
	if(idx >= m_functionSymbol.size()){
		print_error_cr("idx symbol index out of bounds");
		return NULL;
	}
	return &m_functionSymbol[idx];

}

void * CScriptVariable::get_C_Object(){
	return c_object;
}

vector<tInfoStatementOp> * CScriptVariable::getCompiledCode(int idx_function){
	tSymbolInfo *si=CScriptVariable::getFunctionSymbolByIndex(idx_function);
	if(si){
		return &((tScriptFunctionObject *)si->object)->object_info.statment_op;
	}

	return NULL;
}

CScriptVariable::~CScriptVariable(){

	if(created_object != NULL){
		 (*m_infoRegisteredClass->c_destructor)(created_object);
	}


	// remove vars & fundtions if class is C...
	tSymbolInfo *si;

	if((this->m_infoRegisteredClass->metadata_info.object_info.symbol_info.properties & SYMBOL_INFO_PROPERTIES::PROPERTY_C_OBJECT_REF) == 0){ // script class
		for ( unsigned i = 0; i < m_variableSymbol.size(); i++){
			si = &m_variableSymbol[i];
			CScriptVariable * sc  = (CScriptVariable *)(m_variableSymbol[i].object);

			if(sc!=CScriptVariable::UndefinedSymbol && sc!=CScriptVariable::NullSymbol){
				delete sc;
			}
		}
	}


	// Register even for primitives (if appropiate)
	for ( unsigned i = 0; i < m_functionSymbol.size(); i++){
		si = &m_functionSymbol[i];
		tScriptFunctionObject * ir_fun  = (tScriptFunctionObject *)(m_functionSymbol[i].object);
		 if((ir_fun->object_info.symbol_info.properties & SYMBOL_INFO_PROPERTIES::PROPERTY_C_OBJECT_REF) == SYMBOL_INFO_PROPERTIES::PROPERTY_C_OBJECT_REF){ // create proxy function ...
			 (*((std::function<void *(void *,PROXY_CREATOR)> *)ir_fun->object_info.symbol_info.ref_ptr))(si->proxy_ptr,PROXY_CREATOR::DESTROY_FUNCTION);

		}
		//addSymbol(m_infoRegisteredClass->object_info.local_symbols.m_registeredVariable[i].ast);
	}

}
