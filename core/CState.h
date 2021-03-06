/*
 *  This file is distributed under the MIT License.
 *  See LICENSE file for details.
 */

#pragma once


#define SAVE_STATE				CState::saveState()
#define RESTORE_LAST_STATE		CState::restoreLastState()


namespace zetscript{

	class CScope;


	class  CState{



		static vector<CState *> 						* vec_saved_state;
		static CState 									* current_state;


		//static vector<tScopeVar *> 					* current_vec_scope_var_node;
		static string * aux_str;

		void destroyASTNodes();
		void destroyScopeNodes();
		void destroyScriptFunctionObjectNodes();
		void destroyScriptClassNodes();
		void destroyInfoParsedSourceNode();

		vector<CASTNode *> 							*vec_ast_node; // ast collection register...
		vector<CScope *> 							*vec_scope_node;
		//vector<tScopeVar *> 						vec_scope_var_node;
		vector<CScriptClass *> 						*vec_script_class_node;
		vector<CScriptFunctionObject *> 			*vec_script_function_object_node;
		vector<tInfoParsedSource> 					*vec_info_parsed_source_node;

	public:

		static void init();
		ZETSCRIPT_MODULE_EXPORT static bool  setState(int idx);

		/**
		 * Save current state
		 */
		ZETSCRIPT_MODULE_EXPORT static int   saveState();

		static bool   restoreLastState();

		static CState 			* currentState();
		static void clearCurrentCompileInformation();





		/**
		 * Get tScopeVar Node by its idx, regarding current state.
		 */
		/*static vector<tScopeVar *> 				*	getCurrentVecScopeVarNode();
		static int 				  registerScopeVar(tScopeVar *info_scope_var);
		static tScopeVar 	* getScopeVarNodeByIdx(int idx);*/







		static void destroySingletons();

		CState();
		void clearCompileInformation();

		vector<CASTNode *> 					*	getVectorASTNodeNode(); // ast collection register...
		vector<CScope *> 					*	getVectorScopeNode();
		//vector<tScopeVar *> 				*	getVecScopeVarNode();
		vector<CScriptClass *> 				*	getVectorScriptClassNode();
		vector<CScriptFunctionObject *> 	*	getVectorScriptFunctionObjectNode();
		vector<tInfoParsedSource> 			*	getVectorInfoParsedSourceNode();


		void destroy();
		~CState();

	};

}
