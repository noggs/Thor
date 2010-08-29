#include <assimp.hpp>      // C++ importer interface
#include <aiScene.h>       // Output data structure
#include <aiPostProcess.h> // Post processing flags
#include <iostream>
#include <fstream>


namespace bb 
{

	typedef enum VertexComponentBit
	{
		BIT_UV1			= 0x1,
		BIT_NRM			= 0x2,
		BIT_TAN			= 0x4,
	};

}


void DoTheSceneProcessing( const aiScene* scene )
{

	std::fstream out( "out.bbg", std::ios::out | std::ios::binary );

	unsigned int iMesh=0;
	unsigned int iNumMesh=0;

	// count number of valid meshes
	for( iMesh=0; iMesh<scene->mNumMeshes; ++iMesh )
		if( aiPrimitiveType_TRIANGLE & scene->mMeshes[iMesh]->mPrimitiveTypes )
			++iNumMesh;		

	// write number of meshes
	out.write( (char*)&iNumMesh, sizeof(unsigned int) );

	// for each mesh...
	for( iMesh=0; iMesh<scene->mNumMeshes; ++iMesh )
	{
		const aiMesh* mesh = scene->mMeshes[iMesh];
		if( aiPrimitiveType_TRIANGLE & mesh->mPrimitiveTypes )
		{
			int fmt = 0;		// always has position
			if( mesh->HasTextureCoords(0) )
				fmt |= bb::BIT_UV1;
			if( mesh->HasNormals() )
				fmt |= bb::BIT_NRM;
			if( mesh->HasTangentsAndBitangents() )
				fmt |= bb::BIT_TAN;

			// write the format def
			out.write( (char*)&fmt, sizeof(int) );

			// write number of vertices
			out.write( (char*)&mesh->mNumVertices, sizeof(unsigned int) );

			// write vertex stream out
			for( unsigned int iVert=0; iVert < mesh->mNumVertices; ++iVert )
			{
				// position
				out.write( (char*)&mesh->mVertices[iVert], sizeof(float) * 3 );

				// normal
				if( fmt & bb::BIT_NRM )	{
					out.write( (char*)&mesh->mNormals[iVert], sizeof(float) * 3 );
				}

				// tangent
				if( fmt & bb::BIT_TAN ) {
					out.write( (char*)&mesh->mTangents[iVert], sizeof(float) * 3 );
					out.write( (char*)&mesh->mBitangents[iVert], sizeof(float) * 3 );
				}

				// tex coords (1)
				if( fmt & bb::BIT_UV1 )	{
					out.write( (char*)&mesh->mTextureCoords[0][iVert], sizeof(float) * 2 );
				}

			}

			// now write the face information
			out.write( (char*)&mesh->mNumFaces, sizeof(unsigned int) );
			for( unsigned int iFace = 0; iFace < mesh->mNumFaces; ++iFace )
			{
				//assert(mesh->mFaces[iFace].mNumIndices == 3);

				// memory could be saved by using short or even char...
				short indices[3];
				indices[0] = (short)mesh->mFaces[iFace].mIndices[0];
				indices[1] = (short)mesh->mFaces[iFace].mIndices[1];
				indices[2] = (short)mesh->mFaces[iFace].mIndices[2];

				out.write( (char*)&indices[0], sizeof(short) * 3 );
			}
		}
	}

	// done!
	out.flush();
	out.close();
}


bool DoTheImportThing( const std::string& pFile)
{
  // Create an instance of the Importer class
  Assimp::Importer importer;

  // And have it read the given file with some example postprocessing
  // Usually - if speed is not the most important aspect for you - you'll 
  // propably to request more postprocessing than we do in this example.
  const aiScene* scene = importer.ReadFile( pFile, 
        aiProcess_CalcTangentSpace       | 
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);
  
  // If the import failed, report it
  if( !scene)
  {
    //DoTheErrorLogging( importer.GetErrorString());
    return false;
  }

  // Now we can access the file's contents. 
  DoTheSceneProcessing( scene);

  // We're done. Everything will be cleaned up by the importer destructor
  return true;
}



int main(int argc, const char* argv[])
{
	// open the file

	std::string filename( argv[1] );
	DoTheImportThing( filename );


	return 0;
}


