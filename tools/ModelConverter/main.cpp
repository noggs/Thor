#include <assimp.hpp>      // C++ importer interface
#include <aiScene.h>       // Output data structure
#include <aiPostProcess.h> // Post processing flags
#include <iostream>
#include <fstream>

#include <vector>

namespace bb 
{

	typedef enum VertexComponentBit
	{
		BIT_UV1			= 0x1,
		BIT_NRM			= 0x2,
		BIT_TAN			= 0x4,


		BIT_SKIN		= 0x8,	// skinned mesh (contains BoneIndex and BoneWeight components (x4)

	};

}


struct Node
{
	//Node() : mChildren(NULL), mSiblings(NULL) {}

	// tree information
	std::vector<Node*>	mChildren;

	// node information
	const aiNode*	mNode;

};

struct HierarchicalSkeleton
{
	Node* mRoot;
};


Node* RecurseChildren( const aiNode* node, const std::vector<aiString>& validnodes, Node* skeletonNode)
{
	if( find(validnodes.begin(), validnodes.end(), node->mName) != validnodes.end() )
	{
		// if this is a valid node, add to skeleton
		Node* skelnode = new Node;
		skelnode->mNode = node;
		skeletonNode->mChildren.push_back( skelnode );

		// check all children
		for(unsigned int i=0; i<node->mNumChildren; ++i)
		{
			RecurseChildren( node->mChildren[i], validnodes, skelnode );
		}
	}

	return skeletonNode;
}


//////////////////////////////////////////////////////////////////////////
// Given a mesh, will iterate over all the bones and find the aiNode that 
// corresponds to each bone, and also all the parents up till we reach
// the mesh node. This ensures nodes which have no bone in the mesh but 
// are part of the hierarchy get added as well.
void CreateSkeleton(const aiMesh* mesh, const aiScene* scene)
{
	std::vector<aiString> nodes;

	unsigned int meshRootID;
	for(meshRootID=0; meshRootID<scene->mNumMeshes; ++meshRootID)
		if( scene->mMeshes[ meshRootID ] == mesh )
			break;

	for( unsigned int iBone = 0; iBone < mesh->mNumBones; ++iBone )
	{
		aiBone* bone = mesh->mBones[iBone];
		aiNode* node = scene->mRootNode->FindNode( bone->mName );

		while( node )
		{
			if(find(nodes.begin(), nodes.end(), node->mName)==nodes.end())
				nodes.push_back( node->mName );

			// is the mesh root node?
			bool isMeshNode = std::find( &node->mMeshes[0], &node->mMeshes[node->mNumMeshes], meshRootID ) != &node->mMeshes[node->mNumMeshes];

			// is the parent of the mesh root node?
			//bool isMeshNodeParent = node->mNumChildren==1 && node->mChildren[0]->mNumMeshes==1 && node->mChildren[0]->mMeshes[0]==meshRootID;
			bool isMeshNodeParent = node->mNumChildren==1 && std::find( &node->mChildren[0]->mMeshes[0], &node->mChildren[0]->mMeshes[ node->mChildren[0]->mNumMeshes ], meshRootID ) != &node->mChildren[0]->mMeshes[ node->mChildren[0]->mNumMeshes ];

			if( isMeshNode || isMeshNodeParent )
				break;
			else
				node = node->mParent;
		}
	}

	// should have a list of nodes to include in the hierarchy so now recurse through the hierarchy adding nodes we find in our list
	Node* ourRootNode = new Node;
	RecurseChildren( scene->mRootNode, nodes, ourRootNode );

	// now we have the full hierarchy in the skeleton, collapse the nodes that have
	// no bones and a single child associated with them.



}


void PrintNodesRecursive(aiNode* node, int depth, int& nodeCount)
{
	char buf[256] = {0};
	int i=0;
	for(i=0; i<depth; ++i)
		buf[i] = ' ';
	printf( "NODE %02d: %s %s\n", ++nodeCount, buf, node->mName.data);

	for(i=0; i<(int)node->mNumChildren; ++i)
		PrintNodesRecursive(node->mChildren[i], depth+1, nodeCount);
}


void DoTheSceneProcessing( const aiScene* scene )
{

	// print out skeleton information
	int nodeCount=0;
	PrintNodesRecursive( scene->mRootNode, 0, nodeCount );


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
			// temporary arrays for skinning information
			int* numWeights = new int[ mesh->mNumVertices ];		// number of bone weights per vertex
			unsigned char* boneIDs = new unsigned char[ mesh->mNumVertices * 4 ];	// max of 4 bones per vertex
			unsigned char* weights = new unsigned char[ mesh->mNumVertices * 4 ];	// max of 4 weights per vertex

			memset( numWeights, 0, mesh->mNumVertices * sizeof(int) );
			memset( boneIDs, 0, mesh->mNumVertices * 4 );
			memset( weights, 0, mesh->mNumVertices * 4 );

			// iterate over all the bones and create a map of vertex->boneIDs
			for( unsigned int iBone = 0; iBone < mesh->mNumBones; ++iBone )
			{
				const aiBone* bone = mesh->mBones[iBone];
				for( unsigned int iWeight = 0; iWeight < bone->mNumWeights; ++iWeight )
				{
					const aiVertexWeight* weight = &bone->mWeights[iWeight];

					if( numWeights[ weight->mVertexId ] < 4 )
					{
						int nw = numWeights[ weight->mVertexId ];
						boneIDs[ weight->mVertexId * 4 + nw ] = (unsigned char) iBone;
						weights[ weight->mVertexId * 4 + nw ] = (unsigned char)( weight->mWeight * 255.0f );

						++numWeights[ weight->mVertexId ];
					}
					//else
					//	error - more than 4 weights per bone!

				}
			}

			int fmt = 0;		// always has position
			if( mesh->HasTextureCoords(0) )
				fmt |= bb::BIT_UV1;
			if( mesh->HasNormals() )
				fmt |= bb::BIT_NRM;
			if( mesh->HasTangentsAndBitangents() )
				fmt |= bb::BIT_TAN;
			if( mesh->HasBones() )
				fmt |= bb::BIT_SKIN;


			// write the format def
			out.write( (char*)&fmt, sizeof(int) );

			// write number of vertices
			out.write( (char*)&mesh->mNumVertices, sizeof(unsigned int) );

			// write vertex stream out
			for( unsigned int iVert=0; iVert < mesh->mNumVertices; ++iVert )
			{
				// position
				out.write( (char*)&mesh->mVertices[iVert], sizeof(float) * 3 );

				// bone indexes / weights
				if( fmt & bb::BIT_SKIN ) {
					out.write( (char*)&boneIDs[iVert * 4], 4 );
					out.write( (char*)&weights[iVert * 4], 4 );
				}

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

			// now write out bone matrices
			if( fmt & bb::BIT_SKIN ) {

				//////////////////////////////////////////////////////////////////////////
				// a) Create a map or a similar container to store which nodes are necessary for the skeleton. Pre-initialise it for all nodes with a "no". 
				// b) For each bone in the mesh: 
				// b1) Find the corresponding node in the scene's hierarchy by comparing their names. 
				// b2) Mark this node as "yes" in the necessityMap. 
				// b3) Mark all of its parents the same way until you 1) find the mesh's node or 2) the parent of the mesh's node. 
				// c) Recursively iterate over the node hierarchy 
				// c1) If the node is marked as necessary, copy it into the skeleton and check its children 
				// c2) If the node is marked as not necessary, skip it and do not iterate over its children. 
				// 

				CreateSkeleton( mesh, scene );


				out.write( (char*)&mesh->mNumBones, sizeof(unsigned int) );




				for(unsigned int iBone=0; iBone<mesh->mNumBones; ++iBone)
				{
					out.write( (char*)&mesh->mBones[iBone]->mOffsetMatrix, sizeof(float) * 16 );	// write out 4x3 matrix to save space
				}
			}

			delete numWeights;
			delete boneIDs;
			delete weights;
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
        aiProcess_SortByPType            |
		aiProcess_LimitBoneWeights         );
  
  // If the import failed, report it
  if( !scene)
  {
	  const char* error = importer.GetErrorString();
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


