#ifndef DBINTERFACE_H_INCLUDED
#define DBINTERFACE_H_INCLUDED

#include <math/Matrix.h>
#include <core/Types.h>

///
// Forward define some sqlite types
struct sqlite3;
struct sqlite3_stmt;

namespace Thor
{
	typedef Int32 ModelID;

	class DbInterface
	{
	public:
		DbInterface(const char* dbName);
		~DbInterface();

		// create tables and populate with test data
		void CreateAndPopulate();

		ModelID AddModel(const char* fileName, const Thor::Mtx44& mat);

		// grabs all the LOCAL transforms and puts them in the array provided
		// returns the number actually found
		Int32 GetLocalMatrices( Thor::Matrix* matArray, const Int32 maxMats );

		// updates the WORLD transforms in the DB
		Int32 SetWorldMatrices( const Thor::Matrix* const matArray, const Int32 numMats );

	private:

		void CreateStatements();


		sqlite3* mDB;

		// cache queries and insert statements
		sqlite3_stmt*	mStmt_AddMatrix;
		sqlite3_stmt*	mStmt_AddModel;
		sqlite3_stmt*	mStmt_SelectLocalMatrices;
		sqlite3_stmt*	mStmt_UpdateWorldMatrices;

	};

}

#endif
