#include "DbInterface.h"

#include <sqlite3.h>

#include <core/Core.h>


int callbackFN(void* pv, int i, char** c1, char** c2)
{
	return 0;
}



namespace Thor
{

	DbInterface::DbInterface(const char *dbName)
		: mDB(NULL)
		, mStmt_BeginTransaction(NULL)
		, mStmt_CommitTransaction(NULL)
		, mStmt_AddMatrix(NULL)
		, mStmt_AddModel(NULL)
		, mStmt_SelectLocalMatrices(NULL)
		, mStmt_UpdateWorldMatrices(NULL)
	{
		int ret;
		
		ret = sqlite3_open(dbName, &mDB);

		if(ret == SQLITE_OK)
		{
			// add tables if they don't already exist?
			// then create cached statements
		}
	}

	DbInterface::~DbInterface()
	{
		sqlite3_finalize( mStmt_BeginTransaction );
		sqlite3_finalize( mStmt_CommitTransaction );
		sqlite3_finalize( mStmt_AddMatrix );
		sqlite3_finalize( mStmt_AddModel );
		sqlite3_finalize( mStmt_SelectLocalMatrices );
		sqlite3_finalize( mStmt_UpdateWorldMatrices );
		sqlite3_close(mDB);
	}


	void DbInterface::CreateStatements()
	{
		int ret;

		{
			const char* sql[] = { "BEGIN TRANSACTION;", "COMMIT TRANSACTION;" };
			ret = sqlite3_prepare_v2(mDB, sql[0], -1, &mStmt_BeginTransaction, NULL);
			ret = sqlite3_prepare_v2(mDB, sql[1], -1, &mStmt_CommitTransaction, NULL);
		}

		{
			///
			// Begin transaction
			// Insert 2 rows into matrix table
			// Obtain the 2 matrix IDs (or just first one if both consecutive?)
			// Insert 1 row into Models table
			// Commit transaction

			const char* sql[] =	{
				 "INSERT INTO Matrices (xx,xy,xz,xw,yx,yy,yz,yw,zx,zy,zz,zw,px,py,pz,pw) "
				  "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?); " 
			,	 "INSERT INTO Models (modelName, localMatrix, worldMatrix) "
				  "VALUES (?, ?, ?); "	
			};

			ret = sqlite3_prepare_v2(mDB, sql[0], -1, &mStmt_AddMatrix, NULL);
			ret = sqlite3_prepare_v2(mDB, sql[1], -1, &mStmt_AddModel, NULL);
		}

		{
			const char* sql = 
				"SELECT xx,xy,xz,xw,yx,yy,yz,yw,zx,zy,zz,zw,px,py,pz,pw "
				"FROM Matrices JOIN Models ON Matrices.id == Models.localMatrix";
			ret = sqlite3_prepare_v2(mDB, sql, -1, &mStmt_SelectLocalMatrices, NULL);
		}
		{
			const char* sql = 
				"UPDATE Matrices SET "
				"xx=?,xy=?,xz=?,xw=?,yx=?,yy=?,yz=?,yw=?,zx=?,zy=?,zz=?,zw=?,px=?,py=?,pz=?,pw=? "
				"WHERE ROWID = ?"
				;

			ret = sqlite3_prepare_v2(mDB, sql, -1, &mStmt_UpdateWorldMatrices, NULL);
		}

	}


	void DbInterface::CreateAndPopulate()
	{
		int ret;
		char* errmsg=0;

		// matrices table
		{
			const char* sql = 
				"CREATE TABLE Matrices("
				"   id INTEGER PRIMARY KEY"
				",  xx REAL, xy REAL, xz REAL, xw REAL"
				",  yx REAL, yy REAL, yz REAL, yw REAL"
				",  zx REAL, zy REAL, zz REAL, zw REAL"
				",  px REAL, py REAL, pz REAL, pw REAL"
				")";
			ret = sqlite3_exec( mDB, sql, NULL, NULL, &errmsg );
			if(errmsg) {
				sqlite3_free(errmsg);
				errmsg=0;
			}
		}

		// Models table
		{
			const char* sql = 
				"CREATE TABLE Models("
				"   id INTEGER PRIMARY KEY"
				",  modelName TEXT"
				",  localMatrix INTEGER REFERENCES Models(id)"
				",  worldMatrix INTEGER REFERENCES Models(id)"
				")";
			ret = sqlite3_exec( mDB, sql, NULL, NULL, &errmsg );
			if(errmsg) {
				sqlite3_free(errmsg);
				errmsg=0;
			}
		}

		{
			// create view
			const char* sql = 
				"CREATE TEMPORARY VIEW ModelWorldMatrices AS "
				"SELECT Matrices.id,xx,xy,xz,xw,yx,yy,yz,yw,zx,zy,zz,zw,px,py,pz,pw "
				"FROM Matrices JOIN Models ON Matrices.id == Models.worldMatrix";
				;
			ret = sqlite3_exec( mDB, sql, NULL, NULL, &errmsg );
			if(errmsg) {
				sqlite3_free(errmsg);
				errmsg=0;
			}

			// create trigger to handle update of a matrix
			const char* sqlTrigger = 
				"CREATE TEMPORARY TRIGGER update_ModelWorldMatrix "
				"INSTEAD OF UPDATE ON ModelWorldMatrices "
				"  BEGIN "
				"    UPDATE Matrices SET "
				"        xx=new.xx,xy=new.xy,xz=new.xz,xw=new.xw,"
				"        yx=new.yz,yy=new.yy,yz=new.yz,yw=new.yw,"
				"        zx=new.zx,zy=new.zy,zz=new.zz,zw=new.zw,"
				"        px=new.px,py=new.py,pz=new.pz,pw=new.pw "
				"      WHERE id = new.id;"
				"  END;"
				;
			ret = sqlite3_exec( mDB, sqlTrigger, NULL, NULL, &errmsg );
			if(errmsg) {
				sqlite3_free(errmsg);
				errmsg=0;
			}
		}

		// do this after the tables have been created
		CreateStatements();


		// Add some models
		Thor::Mtx44 matIdentity;

		//matIdentity.xx =  1.0f;	matIdentity.xy =  2.0f;	matIdentity.xz =  3.0f;	matIdentity.xw =  4.0f;
		//matIdentity.yx =  5.0f;	matIdentity.yy =  6.0f;	matIdentity.yz =  7.0f;	matIdentity.yw =  8.0f;
		//matIdentity.zx =  9.0f;	matIdentity.zy = 10.0f;	matIdentity.zz = 11.0f;	matIdentity.zw = 12.0f;
		//matIdentity.px = 13.0f;	matIdentity.py = 14.0f;	matIdentity.pz = 15.0f;	matIdentity.pw = 16.0f;

		Thor::MtxIdentity( matIdentity );

		//matIdentity.px = 2.0f;
		//for(int i=0; i<800; ++i)
		AddModel( "duck", matIdentity );
		AddModel( "plane", matIdentity );
		AddModel( "tiny_bones", matIdentity );


	}


	ModelID DbInterface::AddModel(const char* fileName, const Thor::Mtx44& mat)
	{
		int ret;

		// naughty!!!? index as an array...
		const float* matF = &mat.xx;

		// params 1-16 - matrix
		for(int i=0; i<16; ++i)
		{
			ret = sqlite3_bind_double( mStmt_AddMatrix, i+1, matF[i] );
		}

		// add 2 matrices
		ret = sqlite3_step( mStmt_AddMatrix );
		thorAssertNoMessage( ret == SQLITE_DONE );
		ret = sqlite3_reset( mStmt_AddMatrix );

		ret = sqlite3_step( mStmt_AddMatrix );
		thorAssertNoMessage( ret == SQLITE_DONE );
		ret = sqlite3_reset( mStmt_AddMatrix );

		// get the matrix rowID of the last inserted matrix
		sqlite3_int64 rowID = sqlite3_last_insert_rowid(mDB);

		// param 1 - model filename, 2+3 matrixIDs
		ret = sqlite3_bind_text ( mStmt_AddModel, 1, fileName, -1, SQLITE_TRANSIENT );
		ret = sqlite3_bind_int64( mStmt_AddModel, 2, rowID-1 );
		ret = sqlite3_bind_int64( mStmt_AddModel, 3, rowID );

		// add the model
		ret = sqlite3_step( mStmt_AddModel );
		thorAssertNoMessage( ret == SQLITE_DONE );
		ret = sqlite3_reset( mStmt_AddModel );

		sqlite3_int64 modelID = sqlite3_last_insert_rowid(mDB);

		return (ModelID)modelID;
	}


	Int32 DbInterface::GetLocalMatrices(Thor::Matrix *matArray, const Thor::Int32 maxMats)
	{
		int ret;
		float mat[16];
		int matCount = 0;
		sqlite3_stmt* stmt = mStmt_SelectLocalMatrices;

		do
		{
			ret = sqlite3_step( stmt );
			if(ret==SQLITE_ROW)
			{
				thorAssertNoMessage( sqlite3_column_count(stmt) == 16 );
				for(int i=0; i<16; ++i)
				{
					thorAssertNoMessage( sqlite3_column_type(stmt,i) == SQLITE_FLOAT );
					mat[i] = (float)sqlite3_column_double( stmt, i );
				}
				matArray[matCount] = Thor::Matrix( &mat[0] );
				++matCount;
			}
		}
		while( ret == SQLITE_ROW && matCount < maxMats );

		ret = sqlite3_reset( stmt );

		return matCount;
	}


	Int32 DbInterface::SetWorldMatrices( const Thor::Matrix* const matArray, const Int32 numMats )
	{
		///
		// Makes the assumption that matArray is a list of matrices for consecutive entries
		// in the Models table starting from 0 (like is returned by GetLocalMatrices)

		int ret;
		int matCount = 0;
		sqlite3_stmt* stmt = mStmt_UpdateWorldMatrices;

		// BEGIN TRANSACTION
		ret = sqlite3_step( mStmt_BeginTransaction );

		for( int i=0; i<numMats; ++i )
		{
			// naughty!!?
			const float* const matF = &matArray[i]._11;

			// set matrix parameters
			for( int c=0; c<16; ++c )
				ret = sqlite3_bind_double( stmt, c+1, matF[c] );

			// set modelID
			ret = sqlite3_bind_int64( stmt, 17, (Int64) i+1 );

			// execute it
			ret = sqlite3_step( stmt );

			// check if rows were affected
			int changes = sqlite3_changes( mDB );
			if( changes==1 )
				++matCount;

			ret = sqlite3_reset( stmt );
		}

		// END TRANSACTION
		ret = sqlite3_step( mStmt_CommitTransaction );


		return matCount;
	};


}

