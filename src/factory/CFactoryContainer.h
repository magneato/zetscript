#pragma once

class CObject;
class CFactoryContainer{

protected:

	typedef struct{
		string   m_type;
		string   m_typePtr;
		CObject * (* newObject)();
		CObject * (* newObjectWithID)(const string & _id);
		bool (* deleteObjectByID)(const string & _id);
		bool (* deleteObject)(CObject *obj);
		CObject * (* getObject)(const string & _id);
		vector<string> * (* getLocalResourceTypesToLoad)();
		void (* registerScriptFunctions)();

		void (* unloadResources)();
		void (* destroyFactory)();
	}tInfoFactoryContainer;

	vector<tInfoFactoryContainer>							*m_vecTypeContainer;

	static CFactoryContainer * m_registerFactory;

	tInfoFactoryContainer * getFactoryInfo(const string & _type);

public:



	static void createSingletons();
	static void destroySingletons();


	static  CFactoryContainer * getInstance();
	void registerScriptFunctions();

	CFactoryContainer();

	bool registerFactory(
			CObject * (* _pfNewObject)(),
			CObject * (* _pfNewObjectWithID)(const string & _id),
			bool (* _pfDeleteObject)(CObject *),
			bool (* _pfDeleteObjectByID)(const string & _id),
			CObject * (* _pfGetObject)(const string & _id),
			vector<string> * (* getLocalResourceTypesToLoad)(),
			void (* _registerScriptFunctions)(),

			void (* unloadResources)(),
			void (* destroyFactory)(),
			const string & m_type,
			const string & m_typePtr
			);

	bool classPtrIsRegistered(const string & s);

	vector<string>							getFactoryTypeList();

	// GLOBAL
	CObject *newObject(const string & _type);
	CObject * newObjectByClassPtr(const string & _ptr_type);
	CObject *newObject(const string & _type,const string & _id);
	CObject *getObject(const string & _type,const string & _id);




	/**
	 * Get the right type passing the insensitive one.
	 */
	string getUserTypeFromInsensitive(const string & insensitive_type);

	/**
	 * buildMapGroupTypes: returns a set of map collection that matches with specified group and object.
	 */
	//map<string,map<string,CObject *> *>	buildMapGroupTypes(CObject *_obj,const string & _group);

	void unload();

	void unLoadResourcesFactories();
	void destroyFactories();

	tInfoFactoryContainer	* typeObjectFactoryExists(const string & _type);
	bool unRegisterFactory(const string & _type);



	~CFactoryContainer();



};
