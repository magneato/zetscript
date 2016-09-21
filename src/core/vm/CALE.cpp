#include "core/zg_core.h"


CALE::tAleObjectInfo  CALE::stack[VM_MAX_STACK];
CALE::tAleObjectInfo *CALE::currentBaseStack=NULL;
int CALE::idxCurrentStack=0;


CALE::tAleObjectInfo *CALE::allocStack(unsigned n_vars){

	if((idxCurrentStack+n_vars) >=  VM_MAX_STACK){
		print_error_cr("Error MAXIMUM stack size reached");
		exit(EXIT_FAILURE);
	}


	currentBaseStack=&stack[CALE::idxCurrentStack];

	// init vars ...
	for(unsigned i = 0; i < n_vars; i++){
		currentBaseStack[i].stkObject=CScopeInfo::UndefinedSymbol;
		currentBaseStack[i].type = CVariable::VAR_TYPE::UNDEFINED;
		currentBaseStack[i].ptrAssignableVar = NULL;
	}


	CALE::idxCurrentStack+=n_vars;

	return currentBaseStack;
}

CALE::tAleObjectInfo *CALE::freeStack(unsigned n_vars){

	if((idxCurrentStack-n_vars) <  0){
		print_error_cr("Error MINIMUM stack size reached");
		exit(EXIT_FAILURE);
	}

	CALE::idxCurrentStack-=n_vars;
	currentBaseStack=&stack[CALE::idxCurrentStack];


	return currentBaseStack;
}


// general
#define CHECK_VALID_INDEXES \
if(!(index_op1 >= 0 && index_op1 <=current_asm_instruction)) { print_error_cr("instruction 1 out of bounds"); return false;} \
if(!(index_op2 >= 0 && index_op2 <=current_asm_instruction)) { print_error_cr("instruction 2 out of bounds"); return false;} \
if(!(index_op2 >= index_op1 )) { print_error_cr("invalid indexes"); return false;}


#define LOAD_NUMBER_OP(idx) \
		(((CNumber *)(result_object_instruction[idx].stkObject))->m_value)

#define LOAD_INT_OP(idx) \
		(((CInteger *)(result_object_instruction[idx].stkObject))->m_value)




#define LOAD_BOOL_OP(idx) \
		(((CBoolean *)(result_object_instruction[(idx)].stkObject))->m_value)


#define LOAD_STRING_OP(idx) \
		(((CString *)(result_object_instruction[idx].stkObject))->m_value)



// Check types
#define IS_NUMBER(idx) \
(result_object_instruction[idx].type == CVariable::VAR_TYPE::NUMBER)


#define IS_INT(idx) \
(result_object_instruction[idx].type == CVariable::VAR_TYPE::INTEGER)


#define IS_STRING(idx) \
(result_object_instruction[idx].type == CVariable::VAR_TYPE::STRING)

#define IS_BOOLEAN(idx) \
(result_object_instruction[idx].type == CVariable::VAR_TYPE::BOOLEAN)

#define IS_UNDEFINED(idx) \
(result_object_instruction[idx].type == CVariable::VAR_TYPE::UNDEFINED)

#define IS_OBJECT(idx) \
(result_object_instruction[idx].type == CVariable::VAR_TYPE::OBJECT)

#define IS_VECTOR(idx) \
(result_object_instruction[idx].type == CVariable::VAR_TYPE::VECTOR)

#define IS_GENERIC_NUMBER(idx) \
((result_object_instruction[idx].type == CVariable::VAR_TYPE::INTEGER) ||\
(result_object_instruction[idx].type == CVariable::VAR_TYPE::NUMBER))


#define OP1_AND_OP2_ARE_NUMBERS \
(IS_GENERIC_NUMBER(index_op1) && IS_GENERIC_NUMBER(index_op2))

#define OP1_IS_STRING_AND_OP2_IS_NUMBER \
(result_object_instruction[index_op1].type == CVariable::VAR_TYPE::STRING) && \
IS_GENERIC_NUMBER(index_op2)

#define OP1_IS_STRING_AND_OP2_IS_BOOLEAN \
(result_object_instruction[index_op1].type == CVariable::VAR_TYPE::STRING) && \
(result_object_instruction[index_op2].type == CVariable::VAR_TYPE::BOOLEAN)


#define OP1_AND_OP2_ARE_BOOLEANS \
(result_object_instruction[index_op1].type == CVariable::VAR_TYPE::BOOLEAN) && \
(result_object_instruction[index_op2].type == CVariable::VAR_TYPE::BOOLEAN)

#define OP1_AND_OP2_ARE_STRINGS \
(result_object_instruction[index_op1].type == CVariable::VAR_TYPE::STRING) && \
(result_object_instruction[index_op2].type == CVariable::VAR_TYPE::STRING)

#define PROCESS_NUM_OPERATION(__OVERR_OP__)\
					if (IS_INT(index_op1) && IS_INT(index_op2)){\
						if(!pushInteger(LOAD_INT_OP(index_op1) __OVERR_OP__ LOAD_INT_OP(index_op2))){\
							return false;\
						}\
					}else if (IS_INT(index_op1) && IS_NUMBER(index_op2)){\
						if(!pushInteger(LOAD_INT_OP(index_op1) __OVERR_OP__ LOAD_NUMBER_OP(index_op2))) {\
							return false;\
						}\
					}else if (IS_NUMBER(index_op1) && IS_INT(index_op2)){\
						if(!pushNumber(LOAD_NUMBER_OP(index_op1) __OVERR_OP__ LOAD_INT_OP(index_op2))) {\
							return false;\
						}\
					}else {\
						if(!pushNumber(LOAD_NUMBER_OP(index_op1) __OVERR_OP__ LOAD_NUMBER_OP(index_op2))) {\
							return false;\
						}\
					}

// NUMBER result behaviour.
// this is the combination for number operations:
//
// op1 | op2 |  R
// ----+-----+----
//  i  |  i  |  i
//  i  |  f  |  i
//  f  |  i  |  f
//  f  |  f  |  f



CALE::CALE(){

	n_totalIntegerPointers=
	n_totalNumberPointers=
	n_totalBooleanPointers=
	n_totalStringPointers=0;


	reset();

}

bool CALE::pushInteger(int  init_value){
	if(n_stkInteger ==MAX_PER_TYPE_OPERATIONS){
		print_error_cr("Max int operands");
		return false;
	}

	if(n_stkInteger >= n_totalIntegerPointers){
		stkInteger[n_stkInteger] = new CInteger();
		n_totalIntegerPointers++;
	}

	stkInteger[n_stkInteger]->m_value=init_value;
	result_object_instruction[current_asm_instruction]={CVariable::VAR_TYPE::INTEGER,stkInteger[n_stkInteger],NULL};
	n_stkInteger++;

	return true;
}

bool CALE::pushBoolean(bool init_value, int n_stk){
	if(n_stkBoolean ==MAX_PER_TYPE_OPERATIONS){
		print_error_cr("Reached max bool operations");
		return false;
	}




	if(n_stkBoolean >= n_totalBooleanPointers){
		stkBoolean[n_stkBoolean] = new CBoolean();
		n_totalBooleanPointers++;
	}

	stkBoolean[n_stkBoolean]->m_value=init_value;
	result_object_instruction[current_asm_instruction]={CVariable::VAR_TYPE::BOOLEAN,stkBoolean[n_stkBoolean],NULL};
	n_stkBoolean++;

	return true;
}

bool CALE::pushNumber(float init_value){
	if(n_stkNumber ==MAX_PER_TYPE_OPERATIONS){
		print_error_cr("Reached max number operations");
		return false;
	}


	if(n_stkNumber >= n_totalNumberPointers){
		stkNumber[n_stkNumber] = new CNumber();
		n_totalNumberPointers++;
	}

	stkNumber[n_stkNumber]->m_value=init_value;
	result_object_instruction[current_asm_instruction]={CVariable::VAR_TYPE::NUMBER,stkNumber[n_stkNumber],NULL};
	n_stkNumber++;

	return true;
}

bool CALE::pushString(const string & init_value){
	if(n_stkString ==MAX_PER_TYPE_OPERATIONS){
		print_error_cr("Reached max string operations");
		return false;
	}

	if(n_stkString >= n_totalStringPointers){
		stkString[n_stkString] = new CString();
		n_totalStringPointers++;
	}

	stkString[n_stkString]->m_value=init_value;
	result_object_instruction[current_asm_instruction]={CVariable::VAR_TYPE::STRING,stkString[n_stkString],NULL};
	n_stkString++;

	return true;

}

bool CALE::pushObject(CVariable * init_value, CVariable ** ptrAssignableVar=NULL){
	if(n_stkObject ==MAX_PER_TYPE_OPERATIONS){
		print_error_cr("Reached max object operations");
		return false;
	}

	if(init_value == NULL){
		print_error_cr("NULL value");
		return false;
	}

	// try to deduce object type ...
	CVariable::VAR_TYPE	var_type = (init_value)->getVariableType();


	result_object_instruction[current_asm_instruction]={var_type,init_value, ptrAssignableVar};

	return true;
}

bool CALE::pushVector(CVariable * init_value){
	if(n_stkVector ==MAX_PER_TYPE_OPERATIONS){
		print_error_cr("Reached max vector operations");
		return false;
	}

	// this vector will be revised to deallocate vectors which wasn't moved to variables
	stkVector[n_stkVector]=((CVector *)init_value);

	result_object_instruction[current_asm_instruction]={CVariable::VAR_TYPE::VECTOR,stkVector[n_stkVector],NULL};
	n_stkVector++;
	return true;
}

bool CALE::pushFunction(tInfoRegisteredFunctionSymbol * init_value){
	if(n_stkFunction ==MAX_PER_TYPE_OPERATIONS){
		print_error_cr("Reached max function operations");
		return false;
	}

	result_object_instruction[current_asm_instruction]={CVariable::VAR_TYPE::FUNCTION,(CVariable *)init_value,NULL};
	return true;
}

CVariable * CALE::getObjectFromIndex(int index){
	CVariable *obj=NULL;

	if(index >= 0 && index < this->current_asm_instruction){
		return result_object_instruction[index].stkObject;
	}else{
		print_error_cr("index out of bounds!");
	}

	return obj;
}

CVariable * CALE::createObjectFromIndex(int index){
	CVariable *obj = NULL;


	// check second operand valid object..
	switch(result_object_instruction[index].type){
	case CVariable::VAR_TYPE::INTEGER:
		obj=new CInteger();
		((CInteger *)obj)->m_value = ((CInteger *)(result_object_instruction[index].stkObject))->m_value;
		break;
	case CVariable::VAR_TYPE::NUMBER:
		obj = new CNumber();
		((CNumber *)obj)->m_value = ((CNumber *)(result_object_instruction[index].stkObject))->m_value;
		break;
	case CVariable::VAR_TYPE::STRING:
		obj = new CString();
		((CString *)obj)->m_value = ((CString *)(result_object_instruction[index].stkObject))->m_value;
		break;
	case CVariable::VAR_TYPE::BOOLEAN:
		obj = new CBoolean();
		((CBoolean *)obj)->m_value = ((CBoolean *)(result_object_instruction[index].stkObject))->m_value;
		break;
	default:
		obj = result_object_instruction[index].stkObject;
		break;
	}

	return obj;
}

bool CALE::performPreOperator(ASM_PRE_POST_OPERATORS pre_post_operator_type, CVariable *obj){
	// ok from here, let's check preoperator ...
	CVariable *var = (CVariable *)obj;
	switch(var->getVariableType()){
	case CVariable::INTEGER:
		if(pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_INC)
			((CInteger *)var)->m_value++;
		else //dec
			((CInteger *)var)->m_value--;
		break;
	case CVariable::NUMBER:
		if(pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_INC)
			((CNumber *)var)->m_value++;
		else // dec
			((CNumber *)var)->m_value--;

		break;
	default:
		print_error_cr("Cannot perform preoperator ?? because is not number");
		return false;
		break;

	}

	return true;


}

bool CALE::performPostOperator(ASM_PRE_POST_OPERATORS pre_post_operator_type, CVariable *obj){
	// ok from here, let's check preoperator ...
	CVariable *var = (CVariable *)obj;
	switch(var->getVariableType()){
	case CVariable::INTEGER:
		if(pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_INC)
			((CInteger *)var)->m_value++;
		else //dec
			((CInteger *)var)->m_value--;
		break;
	case CVariable::NUMBER:
		if(pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_INC)
			((CNumber *)var)->m_value++;
		else // dec
			((CNumber *)var)->m_value--;

		break;
	default:
		print_error_cr("Cannot perform postoperator ?? because is not number");
		return false;
		break;

	}

	return true;
}


bool CALE::loadVariableValue(tInfoAsmOp *iao, tInfoRegisteredFunctionSymbol *info_function,CScriptClass *this_object, int n_stk){

	if(iao->index_op1 != LOAD_TYPE_VARIABLE){
		print_error_cr("expected load type variable.");
		return false;
	}

	//CScriptClass *this_object = function_object->getThisObject();
	CScriptClass::tSymbolInfo *si;
	CVariable **ptr_var_object=NULL;
	CVariable *var_object = NULL;

	switch(iao->scope_type){
	default:
		print_error_cr("unknow scope type");
		break;
	case SCOPE_TYPE::THIS_SCOPE:

		// get var from object ...
		if((si = this_object->getVariableSymbolByIndex(iao->index_op2))==NULL){
			print_error_cr("cannot find symbol \"%s\"",iao->ast_node->value_symbol.c_str());
			return false;
		}

		ptr_var_object = (CVariable **)(&si->object);
		var_object = (CVariable *)(si->object);

		break;

	case SCOPE_TYPE::LOCAL_SCOPE:

		// get var from base stack ...
		ptr_var_object = (CVariable **)(&CALE::currentBaseStack[iao->index_op2].stkObject);
		var_object = (CVariable *)(CALE::currentBaseStack[iao->index_op2].stkObject);


		/*if((si = this_object->getVariableSymbolByIndex(iao->index_op2))==NULL){
			print_error_cr("cannot find symbol \"%s\"",iao->ast_node->value_symbol.c_str());

			return false;
		}*/

		break;


	}

	//CVariable **ptr_var_object = (CVariable **)(&si->object);
	//CVariable *var_object = (CVariable *)(si->object);
	if(iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_DEC || iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::PRE_INC){

		if(!performPreOperator(iao->pre_post_operator_type, var_object)){
			return false;
		}
	}

	if(iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_DEC || iao->pre_post_operator_type == ASM_PRE_POST_OPERATORS::POST_INC){
		// 1. Load value as constant value
		if(!loadConstantValue(var_object,n_stk)){
			return false;
		}

		// 2. then perform post operation ...
		if(!performPostOperator(iao->pre_post_operator_type, var_object)){
			return false;
		}

	}
	else{
	// generic object pushed ...
		if(!pushObject(var_object,ptr_var_object)) {
			return false;
		}
	}

	return true;
}

bool CALE::loadFunctionValue(tInfoAsmOp *iao,tInfoRegisteredFunctionSymbol *local_function, CScriptClass *this_object, int n_stk){

	if(iao->index_op1 != LOAD_TYPE_FUNCTION){
		print_error_cr("expected load type function.");
		return false;
	}


	tInfoRegisteredFunctionSymbol *info_function=NULL;

	CScriptClass::tSymbolInfo *si;

	//CVariable *var_object = NULL;
	//tInfoRegisteredFunctionSymbol *info_function = (tInfoRegisteredFunctionSymbol *)(si->object);
	//CScriptClass *this_object = function_object->getThisObject();
	//tInfoRegisteredFunctionSymbol *si;

	/*if((si = this_object->getFunctionSymbol(iao->index_op2))==NULL){
		print_error_cr("cannot find function info \"%s\"",iao->ast_node->value_symbol.c_str());
		return false;
	}*/

	switch(iao->scope_type){
	default:
		print_error_cr("unknow scope type");
		break;
	case SCOPE_TYPE::THIS_SCOPE:

		// get var from object ...
		if((si = this_object->getFunctionSymbolByIndex(iao->index_op2))==NULL){
			print_error_cr("cannot find symbol \"%s\"",iao->ast_node->value_symbol.c_str());
			return false;
		}

		info_function =(tInfoRegisteredFunctionSymbol *)si->object;

		break;

	case SCOPE_TYPE::LOCAL_SCOPE:
		info_function = &local_function->object_info.local_symbols.m_registeredFunction[iao->index_op2];
		break;


	}

	// generic object pushed ...
	if(!pushFunction(info_function)) {
		return false;
	}
	//result_object_instruction[current_asm_instruction]={CVariable::FUNCTION,(CVariable **)si, false};


	return true;
}

bool CALE::loadConstantValue(CVariable *var, int n_stk){

	if(var != NULL){

		switch(var->getVariableType()){
			default:
				print_error_cr("Invalid load constant value as %i",var->getVariableType());
				return false;
				break;
			case CVariable::VAR_TYPE::UNDEFINED:

				result_object_instruction[current_asm_instruction]={CVariable::VAR_TYPE::UNDEFINED,CScopeInfo::UndefinedSymbol,NULL};

				break;
			case CVariable::VAR_TYPE::INTEGER:
				if(!pushInteger(((CInteger *)var)->m_value)) return false;
				break;
			case CVariable::VAR_TYPE::BOOLEAN:
				if(!pushBoolean(((CBoolean *)var)->m_value,n_stk)) return false;
				break;
			case CVariable::VAR_TYPE::STRING:
				if(!pushString(((CString *)var)->m_value)) return false;
				break;
			case CVariable::VAR_TYPE::NUMBER:
				if(!pushNumber(((CNumber *)var)->m_value)) return false;
				break;
			}

		return true;

	}

	print_error_cr("Null value");
	return false;

}





bool CALE::assignObjectFromIndex(CVariable **var, int index){

	CVariable *aux_var=NULL;


	// if undefined, create new by default ...
	if(*var == CScopeInfo::UndefinedSymbol){

		if((*var = createObjectFromIndex(index)) == NULL){
			return false;
		}
	}

	tInfoRegisteredFunctionSymbol * init_value;

	// finally assign the value ...
	switch(result_object_instruction[index].type){

		case CVariable::VAR_TYPE::UNDEFINED:
			*var = CScopeInfo::UndefinedSymbol;
			break;

		case CVariable::VAR_TYPE::INTEGER:
			if((*var) == NULL) {
				print_error_cr("Expected variable type!");
				return false;
			}
			if((*var)->getVariableType() == CVariable::VAR_TYPE::INTEGER){
				((CInteger *)(*var))->m_value=((CInteger *)(result_object_instruction[index].stkObject))->m_value;
			}else if((*var)->getVariableType() == CVariable::VAR_TYPE::NUMBER){
				((CNumber *)(*var))->m_value=((CInteger *)(result_object_instruction[index].stkObject))->m_value;
			}else
			{
				print_error_cr("var is not type integer!");
				return false;
			}
			break;
		case CVariable::VAR_TYPE::NUMBER:
			if((*var) == NULL) {print_error_cr("Expected variable type!");return false;}
			if((*var)->getVariableType() == CVariable::VAR_TYPE::INTEGER){
				((CInteger *)aux_var)->m_value=((CNumber *)(result_object_instruction[index].stkObject))->m_value;
			}else if((*var)->getVariableType() == CVariable::VAR_TYPE::NUMBER){
				((CNumber *)(*var))->m_value=((CNumber *)(result_object_instruction[index].stkObject))->m_value;
			}else
			{
				print_error_cr("var is not type number!");
				return false;
			}
			break;
		case CVariable::VAR_TYPE::STRING:
			if((*var) == NULL) {print_error_cr("Expected variable type!");return false;}
			if((*var)->getVariableType() == CVariable::VAR_TYPE::STRING){
				((CString  *)(*var))->m_value= ((CString *)(result_object_instruction[index].stkObject))->m_value;
			}else
			{
				print_error_cr("var is not type string!");
				return false;
			}

			break;
		case CVariable::VAR_TYPE::BOOLEAN:
			if((*var) == NULL) {print_error_cr("Expected variable type!");return false;}
			if((*var)->getVariableType() == CVariable::VAR_TYPE::BOOLEAN){
				((CBoolean  *)aux_var)->m_value= ((CBoolean *)(result_object_instruction[index].stkObject))->m_value;
			}else
			{
				print_error_cr("var is not type boolean!");
				return false;
			}
			break;

		// pointer assigment ...
		case CVariable::VAR_TYPE::VECTOR: // vector object ...
			*var = ((CVector *)(result_object_instruction[index].stkObject));
			break;
		case CVariable::VAR_TYPE::FUNCTION: // function object
			*var = result_object_instruction[index].stkObject;
			break;
		case CVariable::VAR_TYPE::OBJECT: // generic object
			*var = (result_object_instruction[index].stkObject);
			break;

		default:
				print_error_cr("Unknow assignment 2!");
				return false;
				break;
		}


	return true;
}

bool CALE::performInstruction(int idx_instruction, tInfoAsmOp * instruction, int & jmp_to_statment,tInfoRegisteredFunctionSymbol *info_function,CScriptClass *this_object,vector<CVariable *> * argv, int n_stk){


	string 	aux_string;
	bool	aux_boolean;
	string symbol;
	CVariable **obj=NULL;


	jmp_to_statment=-1;

	current_asm_instruction = idx_instruction;

	//CScopeInfo *_lc = instruction->ast_node->scope_info_ptr;

	int index_op1 = instruction->index_op1;
	int index_op2 = instruction->index_op2;





	switch(instruction->operator_type){
	default:
		print_error_cr("operator type(%s) not implemented",CCompiler::def_operator[instruction->operator_type].op_str);
		break;
	case NOP: // ignore ...
		break;
	case LOAD: // load value in function of value/constant ...
		/*if(!loadValue(instruction, n_stk)){
			return false;
		}*/
		//sprintf(print_aux_load_value,"UNDEFINED");
		switch(instruction->index_op1){
		case LOAD_TYPE::LOAD_TYPE_CONSTANT:

			if(!loadConstantValue((CVariable *)instruction->index_op2, n_stk)){
				return false;
			}

			//sprintf(print_aux_load_value,"CONST(%s)",value_symbol.c_str());
			break;
		case LOAD_TYPE::LOAD_TYPE_VARIABLE:

			if(!loadVariableValue(instruction, info_function,this_object, n_stk)){
				return false;
			}

			break;
		case LOAD_TYPE::LOAD_TYPE_FUNCTION:
			if(!loadFunctionValue(instruction,info_function, this_object, n_stk)){
				return false;
			}

			break;
		case LOAD_TYPE::LOAD_TYPE_ARGUMENT:

			if(argv!=NULL){
				if(index_op2<(int)argv->size()){
					CVariable *var=(*argv)[index_op2];

					pushObject(var,NULL);
				}else{
					print_error_cr("index out of bounds");
					return false;
				}
			}
			else{
				print_error_cr("argv null");
				return false;
			}

			//sprintf(print_aux_load_value,"ARG(%s)",value_symbol.c_str());
			break;
		default:
			print_error_cr("no load defined type");
			return false;
			break;

		}

		break;
		case MOV: // mov value expression to var

			// ok load object pointer ...
			if((obj = result_object_instruction[index_op1].ptrAssignableVar) != NULL) {// == CVariable::VAR_TYPE::OBJECT){

				// get pointer object (can be assigned)
				//obj = result_object_instruction[index_op1].stkObject;



				if(!assignObjectFromIndex(obj,index_op2))
						return false;

			}else{
				print_error_cr("Expected object l-value mov");
				return false;
			}

			break;
		case EQU:  // == --> boolean && boolean or string && string or number && number

			if(OP1_AND_OP2_ARE_BOOLEANS) {
				if(!pushBoolean(LOAD_BOOL_OP(index_op1) == LOAD_BOOL_OP(index_op2))) return false;
			}else if(OP1_AND_OP2_ARE_STRINGS){
				if(!pushBoolean(LOAD_STRING_OP(index_op1) == LOAD_STRING_OP(index_op2))) return false;
			}else if (IS_INT(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) == LOAD_INT_OP(index_op2))) return false;
			}else if (IS_INT(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) == LOAD_NUMBER_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) == LOAD_INT_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) == LOAD_NUMBER_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as string, number or boolean!");
				return false;
			}

			break;

		case NOT_EQU:  // == --> boolean && boolean or string && string or number && number

			if(OP1_AND_OP2_ARE_BOOLEANS) {
				if(!pushBoolean(LOAD_BOOL_OP(index_op1) != LOAD_BOOL_OP(index_op2))) return false;
			}else if(OP1_AND_OP2_ARE_STRINGS){
				if(!pushBoolean(LOAD_STRING_OP(index_op1) != LOAD_STRING_OP(index_op2))) return false;
			}else if (IS_INT(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) != LOAD_INT_OP(index_op2))) return false;
			}else if (IS_INT(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) != LOAD_NUMBER_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) != LOAD_INT_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) != LOAD_NUMBER_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as string, number or boolean!");
				return false;
			}

			break;
		case LT:  // <
			if (IS_INT(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) < LOAD_INT_OP(index_op2))) return false;
			}else if (IS_INT(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) < LOAD_NUMBER_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) < LOAD_INT_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) < LOAD_NUMBER_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}
			break;
		case LTE:  // <=

			if (IS_INT(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) <= LOAD_INT_OP(index_op2))) return false;
			}else if (IS_INT(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) <= LOAD_NUMBER_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) <= LOAD_INT_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) <= LOAD_NUMBER_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}

			break;
		case NOT: // !
			if (result_object_instruction[index_op1].type == CVariable::VAR_TYPE::BOOLEAN){
				if(!pushBoolean(!LOAD_BOOL_OP(index_op1))) return false;
			}else{
				print_error_cr("Expected operands 1 as boolean!");
				return false;
			}
			break;
		case NEG: // !
			if (IS_GENERIC_NUMBER(index_op1)){
				if(result_object_instruction[index_op1].type == CVariable::VAR_TYPE::INTEGER){ // operation will result as integer.
					if(!pushInteger(-LOAD_INT_OP(index_op1))) {
						return false;
					}
				}
				else{
					if(!pushNumber(-LOAD_NUMBER_OP(index_op2))){
						return false;
					}
				}

			}else{
					print_error_cr("Expected operands 1 as number or integer!");
					return false;
			}
			break;

		case GT:  // >
			if (IS_INT(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) > LOAD_INT_OP(index_op2))) return false;
			}else if (IS_INT(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) > LOAD_NUMBER_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) > LOAD_INT_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) > LOAD_NUMBER_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}
			break;
		case GTE: // >=
			if (IS_INT(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) >= LOAD_INT_OP(index_op2))) return false;
			}else if (IS_INT(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_INT_OP(index_op1) >= LOAD_NUMBER_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_INT(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) >= LOAD_INT_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_NUMBER(index_op2)){
				if(!pushBoolean(LOAD_NUMBER_OP(index_op1) >= LOAD_NUMBER_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}
			break;

		case ADD: // +

			// get indexes and check whether is possible or not its calculation.
			// check indexes
			CHECK_VALID_INDEXES;

			// check types ...
			if (IS_STRING(index_op1) && (IS_UNDEFINED(index_op2) || IS_VECTOR(index_op2) || IS_OBJECT(index_op2))){
				if(!pushString(LOAD_STRING_OP(index_op1)+(result_object_instruction[index_op2].stkObject)->getPointerClassStr())) return false;
			}else if (IS_INT(index_op1) && IS_INT(index_op2)){
				if(!pushInteger(LOAD_INT_OP(index_op1) + LOAD_INT_OP(index_op2))) return false;
			}else if (IS_INT(index_op1) && IS_NUMBER(index_op2)){
				if(!pushInteger(LOAD_INT_OP(index_op1) + LOAD_NUMBER_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_INT(index_op2)){
				if(!pushNumber(LOAD_NUMBER_OP(index_op1) + LOAD_INT_OP(index_op2))) return false;
			}else if (IS_NUMBER(index_op1) && IS_NUMBER(index_op2)){
				if(!pushNumber(LOAD_NUMBER_OP(index_op1) + LOAD_NUMBER_OP(index_op2))) return false;
			}else if(OP1_IS_STRING_AND_OP2_IS_NUMBER){ // concatenate string + number

				aux_string =  LOAD_STRING_OP(index_op1);

				if(result_object_instruction[index_op2].type == CVariable::VAR_TYPE::INTEGER)
					aux_string = aux_string + CStringUtils::intToString(LOAD_INT_OP(index_op2));
				else
					aux_string = aux_string + CStringUtils::intToString(LOAD_NUMBER_OP(index_op2));

				if(!pushString(aux_string)) return false;
			}else if(OP1_IS_STRING_AND_OP2_IS_BOOLEAN){ // concatenate string + boolean

				aux_string =  LOAD_STRING_OP(index_op1);
				aux_boolean =  LOAD_BOOL_OP(index_op2);

				if(aux_boolean)
					aux_string = aux_string + "true";
				else
					aux_string = aux_string + "false";

				if(!pushString(aux_string)) return false;

			}else if(OP1_AND_OP2_ARE_STRINGS){ // concatenate string + boolean

				if(!pushString(LOAD_STRING_OP(index_op1)+LOAD_STRING_OP(index_op2))) return false;

			}else{

				// full error description ...

				string var_type1=(result_object_instruction[index_op1].stkObject)->getClassStr(),
					   var_type2=(result_object_instruction[index_op2].stkObject)->getClassStr();


				//print_error_cr("Expected operands as number+number, string+string, string+number or string + boolean!");


				print_error_cr("Error at line %i cannot perform operator \"%s\" +  \"%s\"",
						instruction->ast_node->definedValueline,
						var_type1.c_str(),
						var_type2.c_str());
				return false;
			}

			break;

		case LOGIC_AND: // &&
			if(OP1_AND_OP2_ARE_BOOLEANS) {
				if(!pushBoolean(LOAD_BOOL_OP(index_op1) && LOAD_BOOL_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands boolean!");
				return false;
			}
			break;
		case LOGIC_OR:  // ||
			if(OP1_AND_OP2_ARE_BOOLEANS) {
				if(!pushBoolean(LOAD_BOOL_OP(index_op1) || LOAD_BOOL_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands boolean!");
				return false;
			}
			break;
		case DIV: // /
			if(OP1_AND_OP2_ARE_NUMBERS) {

				if(IS_INT(index_op2)){
					if(LOAD_INT_OP(index_op2) == 0) {
						print_error_cr("Divide by 0 at line %i.",instruction->ast_node->definedValueline);
						return false;
					}
				}else{
					if(LOAD_NUMBER_OP(index_op2) == 0) {
						print_error_cr("Divide by 0 at line %i.",instruction->ast_node->definedValueline);
						return false;
					}
				}

				PROCESS_NUM_OPERATION(/);
			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}

			break;
		case MUL: // *
			if(OP1_AND_OP2_ARE_NUMBERS) {
					PROCESS_NUM_OPERATION(*);

			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}
			break;
		case MOD:  // %
			if(OP1_AND_OP2_ARE_NUMBERS) {

				if(IS_INT(index_op2)){
					if(LOAD_INT_OP(index_op2) == 0) {
						print_error_cr("Divide by 0 at line %i.",instruction->ast_node->definedValueline);
						return false;
					}
				}else{
					if(LOAD_NUMBER_OP(index_op2) == 0) {
						print_error_cr("Divide by 0 at line %i.",instruction->ast_node->definedValueline);
						return false;
					}
				}
				//PROCESS_NUM_OPERATION(%);

				if (IS_INT(index_op1) && IS_INT(index_op2)){
					if(!pushInteger(LOAD_INT_OP(index_op1) % LOAD_INT_OP(index_op2))){
						return false;\
					}
				}else if (IS_INT(index_op1) && IS_NUMBER(index_op2)){
					if(!pushInteger(LOAD_INT_OP(index_op1) % ((int) LOAD_NUMBER_OP(index_op2)))) {
						return false;\
					}
				}else if (IS_NUMBER(index_op1) && IS_INT(index_op2)){
					if(!pushNumber(fmod(LOAD_NUMBER_OP(index_op1), LOAD_INT_OP(index_op2)))) {
						return false;
					}
				}else {
					if(!pushNumber(fmod(LOAD_NUMBER_OP(index_op1) , LOAD_NUMBER_OP(index_op2)))) {
						return false;
					}
				}

			}else{
				print_error_cr("Expected both operands as number!");
				return false;
			}

			break;
		case AND: // bitwise logic and
			if((result_object_instruction[index_op1].type == CVariable::INTEGER) && (result_object_instruction[index_op2].type == CVariable::INTEGER)){
				if(!pushInteger(LOAD_INT_OP(index_op1) & LOAD_INT_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}
			break;
		case OR: // bitwise logic or
			if((result_object_instruction[index_op1].type == CVariable::INTEGER) && (result_object_instruction[index_op2].type == CVariable::INTEGER)){
				if(!pushInteger(LOAD_INT_OP(index_op1) | LOAD_INT_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}

			break;
		case XOR: // logic xor
			if((result_object_instruction[index_op1].type == CVariable::INTEGER) && (result_object_instruction[index_op2].type == CVariable::INTEGER)){
				if(!pushInteger(LOAD_INT_OP(index_op1) ^ LOAD_INT_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}
			break;
		case SHL: // shift left
			if((result_object_instruction[index_op1].type == CVariable::INTEGER) && (result_object_instruction[index_op2].type == CVariable::INTEGER)){
				if(!pushInteger(LOAD_INT_OP(index_op1) << LOAD_INT_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}
			break;
		case SHR: // shift right
			if((result_object_instruction[index_op1].type == CVariable::INTEGER) && (result_object_instruction[index_op2].type == CVariable::INTEGER)){
				if(!pushInteger(LOAD_INT_OP(index_op1) >> LOAD_INT_OP(index_op2))) return false;
			}else{
				print_error_cr("Expected both operands as integer types!");
				return false;
			}
			break;
		// special internal ops...
		case JMP:
			jmp_to_statment = index_op1;
			break;
		case JNT: // goto if not true ... goes end to conditional.

			// load boolean var and jmp if true...
			if(current_asm_instruction > 0){

				if(result_object_instruction[current_asm_instruction-1].type == CVariable::VAR_TYPE::BOOLEAN){

					if(!(*((CBoolean **)result_object_instruction[current_asm_instruction-1].stkObject))->m_value){
						jmp_to_statment = index_op1;
					}
				}
			}else{
				print_error_cr("No boolean elements");
				return false;
			}
			break;
		case JT: // goto if true ... goes end to conditional.
			if(current_asm_instruction > 0){

				if(result_object_instruction[current_asm_instruction-1].type == CVariable::VAR_TYPE::BOOLEAN){

					if((*((CBoolean **)result_object_instruction[current_asm_instruction-1].stkObject))->m_value){
						jmp_to_statment = index_op1;
					}
				}
			}else{
				print_error_cr("No boolean elements");
				return false;
			}
			break;
		case CALL: // calling function after all of args are processed...

			// check whether signatures matches or not ...
			// 1. get function object ...
			if(result_object_instruction[index_op1].type == CVariable::FUNCTION){
				CVariable *ret_obj;
				tInfoRegisteredFunctionSymbol * function_info = (tInfoRegisteredFunctionSymbol *)result_object_instruction[index_op1].stkObject;

				// by default virtual machine gets main object class in order to run functions ...
				if((ret_obj=CVirtualMachine::execute(function_info,this_object,&m_functionArgs, n_stk+1))==NULL){
					return false;
				}

				// finally set result value into stkObject...
				if(!pushObject(ret_obj)){
					return false;
				}
			}
			else{
				print_error_cr("object \"%s\" is not function at line %i",instruction->ast_node->value_symbol.c_str(), instruction->ast_node->definedValueline);
				return false;
			}
			break;
		case PUSH: // push arg instruction...
			m_functionArgs.push_back(getObjectFromIndex(index_op1));
			break;
		case CLR: // clear args
			m_functionArgs.clear();
			break;
		case VGET: // vector access after each index is processed...
			// index_op1 is vector, index op2 is index...
			if(result_object_instruction[index_op1].type == CVariable::VECTOR){
				if(result_object_instruction[index_op2].type == CVariable::INTEGER){
					// determine object ...
					CVector * vec = (CVector *)(result_object_instruction[index_op1].stkObject);//[stkInteger[result_object_instruction[index_op2].index]];
					int v_index = LOAD_INT_OP(index_op2);

					print_info_cr("%i",v_index);

					// check indexes ...
					if(v_index < 0 || v_index >= (int)vec->m_value.size()){
						print_error_cr("Line %i. Index vector out of bounds!",instruction->ast_node->definedValueline);
						return false;
					}

					if(!pushObject(vec->m_value[v_index],&vec->m_value[v_index])){
						return false;
					}

				}else{
					print_error_cr("Expected vector-index as integer");
					return false;
				}
			}
			else{
				print_error_cr("Expected operand 1 as vector");
				return false;
			}

			break;
		case VPUSH: // Value push for vector
			if(result_object_instruction[index_op1].type == CVariable::VECTOR){
				CVector * vec = (CVector *)(result_object_instruction[index_op1].stkObject);
				vec->m_value.push_back(createObjectFromIndex(index_op2));
			}else{
				print_error_cr("Expected operand 1 as vector");
				return false;
			}
			break;
		case VEC: // Create new vector object...

			pushVector(new CVector());

			break;

		case RET:

			/*if(!assignObjectFromIndex(function_object->getReturnObjectPtr(),instruction->index_op1)){
				return false;
			}*/

			break;

	}

	return true;
}

void CALE::reset(){

	// deallocate allocated aux vectors...

	n_stkInteger=
	n_stkNumber=
	n_stkBoolean=
	n_stkString=
	n_stkObject=
	n_stkVector=
	n_stkFunction=
	current_asm_instruction=0;

	//memset(result_object_instruction,0,sizeof(result_object_instruction));
	m_functionArgs.clear();

}

CALE::~CALE(){

	reset();

	for(int i = 0 ; i < n_totalIntegerPointers; i++){
		delete 	stkInteger[i];
	}

	for(int i = 0 ; i < n_totalNumberPointers; i++){
		delete stkNumber[i];
	}

	for(int i = 0 ; i < n_totalBooleanPointers; i++){
		delete stkBoolean[i];
	}

	for(int i = 0 ; i < n_totalStringPointers; i++){
		delete stkString[i];
	}


	n_totalIntegerPointers=
	n_totalNumberPointers=
	n_totalBooleanPointers=
	n_totalStringPointers=0;



}



